// xrServer.cpp: implementation of the xrServer class.
//
//////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "xrServer.h"
#include "xrMessages.h"
#include "xrServer_Objects_ALife_All.h"
#include "level.h"
#include "game_cl_base.h"
#include "game_sv_mp.h"
#include "game_cl_base_weapon_usage_statistic.h"
#include "ai_space.h"
#include "../xrEngine/IGame_Persistent.h"
#include "string_table.h"
#include "object_broker.h"

#include "../xrEngine/XR_IOConsole.h"
#include "ui/UIInventoryUtilities.h"
#include "file_transfer.h"
#include "screenshot_server.h"
#include "xrServer_info.h"
#include <functional>

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

u32 g_sv_traffic_optimization_level = eto_none;

xrClientData::xrClientData	() : IClient(Device.GetTimerGlobal())
{
	ps = NULL;
	Clear		();
}

void	xrClientData::Clear()
{
	owner									= NULL;
	net_Ready								= FALSE;
	net_Accepted							= FALSE;
	net_PassUpdates							= TRUE;
	m_ping_warn.m_maxPingWarnings			= 0;
	m_ping_warn.m_dwLastMaxPingWarningTime	= 0;
	m_admin_rights.m_has_admin_rights		= FALSE;

	m_last_update_time_15 = 0;
	m_last_update_time_10 = 0;
	m_last_update_time_5 = 0;
	m_last_update_time_1 = 0;
	m_last_update_time_05 = 0;
};


xrClientData::~xrClientData()
{
	xr_delete(ps);
}


xrServer::xrServer() : inherited(Device.GetTimerGlobal(), g_dedicated_server)
{
	m_file_transfers	= NULL;
	m_aDelayedPackets.clear();
	m_server_logo		= NULL;
	m_server_rules		= NULL;
	m_last_updates_size	= 0;
	m_last_update_time	= 0;
}

xrServer::~xrServer()
{
	struct ClientDestroyer
	{
		static bool true_generator(IClient*)
		{
			return true;
		}
	};
	IClient* tmp_client = net_players.GetFoundClient(&ClientDestroyer::true_generator);
	while (tmp_client)
	{
		client_Destroy(tmp_client);
		tmp_client = net_players.GetFoundClient(&ClientDestroyer::true_generator);
	}
	m_aDelayedPackets.clear();
	entities.clear();
	delete_data(m_info_uploaders);
	xr_delete(m_server_logo);
	xr_delete(m_server_rules);
}

//--------------------------------------------------------------------

CSE_Abstract*	xrServer::ID_to_entity		(u16 ID)
{
	// #pragma todo("??? to all : ID_to_entity - must be replaced to 'game->entity_from_eid()'")	
	if (0xffff==ID)				return 0;
	xrS_entities::iterator	I	= entities.find	(ID);
	if (entities.end()!=I)		return I->second;
	else						return 0;
}

//--------------------------------------------------------------------
IClient*	xrServer::client_Create		()
{
	return xr_new<xrClientData> ();
}
void		xrServer::client_Replicate	()
{
}

IClient*	xrServer::client_Find_Get	(ClientID ID)
{
	DWORD dwPort			= 0;
	ip_address tmp_ip_address;


	if ( !psNET_direct_connect )
		GetClientAddress(ID, tmp_ip_address, &dwPort );
	else
		tmp_ip_address.set( "127.0.0.1" );

	IClient* newCL = client_Create();
	newCL->ID = ID;
	
	if(!psNET_direct_connect)
	{
		newCL->m_cAddress	= tmp_ip_address;	
		newCL->m_dwPort		= dwPort;
	}

	newCL->server			= this;
	net_players.AddNewClient(newCL);

#ifndef MASTER_GOLD
	Msg		("# New player created.");
#endif // #ifndef MASTER_GOLD
	return newCL;
};

u32	g_sv_Client_Reconnect_Time = 3;

void		xrServer::client_Destroy	(IClient* C)
{
	// Delete assosiated entity
	// xrClientData*	D = (xrClientData*)C;
	// CSE_Abstract* E = D->owner;
	IClient* alife_client = net_players.FindAndEraseClient(std::bind(std::equal_to<IClient*>(), std::placeholders::_1, C));
	//VERIFY(alife_client);
	if (alife_client)
	{
		CSE_Abstract* pOwner	= static_cast<xrClientData*>(alife_client)->owner;
		CSE_Spectator* pS		= smart_cast<CSE_Spectator*>(pOwner);
		if (pS)
		{
			NET_Packet			P;
			P.w_begin			(M_EVENT);
			P.w_u32				(Level().timeServer());//Device.TimerAsync());
			P.w_u16				(GE_DESTROY);
			P.w_u16				(pS->ID);
			SendBroadcast		(C->ID,P,net_flags(TRUE,TRUE));
		};

		DelayedPacket pp;
		pp.SenderID = alife_client->ID;
		xr_deque<DelayedPacket>::iterator it;
		do{
			it						=std::find(m_aDelayedPackets.begin(),m_aDelayedPackets.end(),pp);
			if(it!=m_aDelayedPackets.end())
			{
				m_aDelayedPackets.erase	(it);
				Msg("removing packet from delayed event storage");
			}else
				break;
		}while(true);
		
		if (pOwner)
		{
			game->CleanDelayedEventFor(pOwner->ID);
		}
		
//.		if (!alife_client->flags.bVerified)
		xrClientData*	xr_client = static_cast<xrClientData*>(alife_client);
		m_disconnected_clients.Add(xr_client); //xr_delete(alife_client);				
	}
}

void xrServer::GetPooledState(xrClientData* xrCL)
{
	xrClientData* pooled_client = m_disconnected_clients.Get(xrCL);
	if (!pooled_client)
		return;

	NET_Packet	tmp_packet;
	u16			tmp_fake;
	tmp_packet.w_begin				(M_SPAWN);
	pooled_client->ps->net_Export	(tmp_packet, TRUE);
	tmp_packet.r_begin				(tmp_fake);
	xrCL->ps->net_Import			(tmp_packet);
	xrCL->ps->flags__ = 0; // reset flags
	xrCL->flags.bReconnect			= TRUE;
	xr_delete						(pooled_client);
}

//--------------------------------------------------------------------
int	g_Dump_Update_Write = 0;

#ifdef DEBUG
INT g_sv_SendUpdate = 0;
#endif

void xrServer::Update	()
{
	if (Level().IsDemoPlayStarted() || Level().IsDemoPlayFinished())
		return;								//diabling server when demo is playing

	NET_Packet		Packet;

	VERIFY						(verify_entities());

	ProceedDelayedPackets();
	// game update
	game->ProcessDelayedEvent();
	game->Update						();

	// spawn queue
	u32 svT								= Device.TimerAsync();
	while (!(q_respawn.empty() || (svT<q_respawn.begin()->timestamp)))
	{
		// get
		svs_respawn	R					= *q_respawn.begin();
		q_respawn.erase					(q_respawn.begin());

		// 
		CSE_Abstract* E					= ID_to_entity(R.phantom);
		E->Spawn_Write		(Packet,FALSE);
		u16								ID;
		Packet.r_begin		(ID);
		R_ASSERT(M_SPAWN==ID);
		ClientID						clientID; 
		clientID.set(0xffff);
		Process_spawn		(Packet,clientID);
	}


	SendUpdatesToAll();


	if (game->sv_force_sync)	Perform_game_export();

	VERIFY						(verify_entities());
	//-----------------------------------------------------
	
	PerformCheckClientsForMaxPing	();
	Flush_Clients_Buffers			();
	
	if( 0==(Device.dwFrame%100) )//once per 100 frames
	{
		UpdateBannedList();
	}
}

void _stdcall xrServer::SendGameUpdateTo(IClient* client)
{
	xrClientData*	xr_client = static_cast<xrClientData*>(client);
	VERIFY			(xr_client);
	if (!xr_client->net_Ready)
	{
		return;
	}

	if (!HasBandwidth(client)
#ifdef DEBUG 
			&& !g_sv_SendUpdate
#endif
		)
	{
		return;
	}
	
	NET_Packet				Packet;
	u16 PacketType			= M_UPDATE;
	Packet.w_begin			(PacketType);
	game->net_Export_Update	(Packet, xr_client->ID, xr_client->ID);
	SendTo					(xr_client->ID, Packet, net_flags(FALSE,TRUE));
}

void xrServer::MakeUpdatePackets()
{
	NET_Packet						tmpPacket;			
	u32								position;

#ifndef OLD_SYNC
	m_update_packets.clear();
#endif // !OLD_SYNC

	m_updator.begin_updates			();
	
	xrS_entities::iterator I	= entities.begin();
	xrS_entities::iterator E	= entities.end();
	for (; I!=E; ++I)
	{//all entities
		CSE_Abstract&	Test			= *(I->second);

		if (0==Test.owner)								continue;
		if (!Test.net_Ready)							continue;
		if (Test.s_flags.is(M_SPAWN_OBJECT_PHANTOM))	continue;	// Surely: phantom
		if (!Test.Net_Relevant() )						continue;

		tmpPacket.B.count				= 0;
		// write specific data
		{
			tmpPacket.w_u16					(Test.ID);
			tmpPacket.w_u32					(Level().timeServer());
			tmpPacket.w_chunk_open8			(position);
			Test.UPDATE_Write				(tmpPacket);
			u32 ObjectSize					= u32(tmpPacket.w_tell()-position)-sizeof(u8);
			tmpPacket.w_chunk_close8		(position);

			if (ObjectSize == 0)			continue;

#ifdef DEBUG
			if (g_Dump_Update_Write) Msg("* %s : %d", Test.name(), ObjectSize);
#endif

#ifdef OLD_SYNC
			m_updator.write_update_for(Test.ID, tmpPacket);
#else
			// save all packets
			m_update_packets.push_back(UpdatePacket());
			UpdatePacket* NewPacket = &(m_update_packets.back());
			NewPacket->Entity = I->second;
			CopyMemory(&(NewPacket->Packet), &tmpPacket, sizeof(NET_Packet));
#endif // OLD_SYNC
			}
	}//all entities

	m_updator.end_updates			(m_update_begin, m_update_end);
}

u32 ai_stalker, ai_mutants, items, actors, arts, physics, others;
u32 item_weapon;

void xrServer::SendUpdatePacketsToAll()
{
#ifdef OLD_SYNC
	m_last_updates_size = 0;
	for (update_iterator_t i = m_update_begin; i != m_update_end; ++i)
	{
		NET_Packet& to_send = **i;
		if (to_send.B.count > 2)
		{
			m_last_updates_size += to_send.B.count;
			SendBroadcast	(GetServerClient()->ID, to_send, net_flags(FALSE,TRUE));
			if (Level().IsDemoSave())
			{
				Level().SavePacket(to_send);
			}
		}
	}
#else

	struct ClientExcluderPredicate
	{
		ClientID id_to_exclude;
		ClientExcluderPredicate(ClientID exclude) :
			id_to_exclude(exclude)
		{}
		bool operator()(IClient* client)
		{
			xrClientData* tmp_client = static_cast<xrClientData*>(client);
			if (client->ID == id_to_exclude)
				return false;
			if (!client->flags.bConnected)
				return false;
			if (!tmp_client->net_Accepted)
				return false;
			return true;
		}
	};

	struct SenderFunctor
	{
		xrServer *m_owner;
		u32 m_dwFlags;
		xr_vector<UpdatePacket> &m_packets;

		server_updates_compressor*	m_updator;
		update_iterator_t			m_update_begin;
		update_iterator_t			m_update_end;

		SenderFunctor(xrServer* owner, xr_vector<UpdatePacket> &packets, server_updates_compressor* updator, u32 dwFlags) :
			m_owner(owner), m_packets(packets), m_updator(updator), m_dwFlags(dwFlags)
		{}
		void operator()(IClient* client)
		{
			auto I = m_packets.begin();
			auto E = m_packets.end();

			xrClientData* CL = static_cast<xrClientData*>(client);

			bool need_to_update_15 = Device.dwTimeGlobal - CL->m_last_update_time_15 >= u32(1000 / psNET_ServerUpdate); // 30 per sec
			bool need_to_update_10 = Device.dwTimeGlobal - CL->m_last_update_time_10 >= u32(1000 / psNET_ServerUpdate / 3); // 10 per sec
			bool need_to_update_5  = Device.dwTimeGlobal - CL->m_last_update_time_5 >= u32(1000 / psNET_ServerUpdate / 5);    // 5 per sec
			bool need_to_update_1  = Device.dwTimeGlobal - CL->m_last_update_time_1 >= u32(1000);        // 1 per sec
			bool need_to_update_05 = Device.dwTimeGlobal - CL->m_last_update_time_05 >= u32(2000);       // 1 per 2 sec
			
			constexpr float distance_30 = 30.f * 30.f;
			constexpr float distance_50 = 50.f * 50.f;
			constexpr float distance_60 = 60.f * 60.f;
			constexpr float distance_100 = 100.f * 100.f;
			constexpr float distance_200 = 200.f * 200.f;
			constexpr float distance_300 = 300.f * 300.f;

			// create big net packets & compress (if enabled)
			m_updator->begin_updates();
			for (; I != E; ++I)
			{
				CSE_Abstract* owner = CL->owner;
				if (!owner) continue;

				CSE_Abstract*	entity = I->Entity;
				NET_Packet& packet = I->Packet;

				float distance = 0.f;

				CSE_Abstract* parent = m_owner->ID_to_entity(entity->ID_Parent);

				bool has_parent = !!parent;

				if (!has_parent)
				{
					distance = owner->Position().distance_to_sqr(entity->Position());
				}
				else
				{
					distance = owner->Position().distance_to_sqr(parent->Position());
				}

				if (entity->cast_human_abstract() || entity->cast_monster_abstract()) 
				{
					// MONSTERS AND HUMANS
					// 0 - 50 : 30 per sec
					// 50 - 100 : 15 per sec
					// 100 - 200 : 10 per sec
					// 200 - 300 : 5 per sec
					// 300 and more : 1 per 2 sec

					if (distance <= distance_50)
					{
						m_updator->write_update_for(entity->ID, packet);
						if (entity->cast_human_abstract())
							ai_stalker += packet.B.count;

						if (smart_cast<CSE_ALifeMonsterBase*>(entity))
							ai_mutants += packet.B.count;
					}
					else if (need_to_update_15 && distance <= distance_100)
					{
						m_updator->write_update_for(entity->ID, packet);
						if (entity->cast_human_abstract())
							ai_stalker += packet.B.count;

						if (smart_cast<CSE_ALifeMonsterBase*>(entity))
							ai_mutants += packet.B.count;
					}
					else if (need_to_update_10 && distance <= distance_200)
					{
						m_updator->write_update_for(entity->ID, packet);
						if (entity->cast_human_abstract())
							ai_stalker += packet.B.count;

						if (smart_cast<CSE_ALifeMonsterBase*>(entity))
							ai_mutants += packet.B.count;
					}
					else if (need_to_update_5 && distance <= distance_300)
					{
						m_updator->write_update_for(entity->ID, packet);
						if (entity->cast_human_abstract())
							ai_stalker += packet.B.count;

						if (smart_cast<CSE_ALifeMonsterBase*>(entity))
							ai_mutants += packet.B.count;
					}
					else if (need_to_update_05)
					{
						m_updator->write_update_for(entity->ID, packet);
						if (entity->cast_human_abstract())
							ai_stalker += packet.B.count;

						if (smart_cast<CSE_ALifeMonsterBase*>(entity))
							ai_mutants += packet.B.count;
					}
				}
				else if (entity->cast_actor_mp()) 
				{
					// ACTORS
					// 0 - 200 : 30 per second
					// 200 - 300 : 10 per second
					// 300 and more : 1 per sec

					if (distance <= distance_200)
					{
						m_updator->write_update_for(entity->ID, packet);
						actors += packet.B.count;
					}
					else if (need_to_update_10 && distance <= distance_300)
					{
						m_updator->write_update_for(entity->ID, packet);
						actors += packet.B.count;
					}
					else if (need_to_update_1)
					{
						m_updator->write_update_for(entity->ID, packet);
						actors += packet.B.count;
					}


				}
				else if (entity->cast_item_artefact())
				{
					// ARTEFACTS
					// 0 - 30 : 10 per second
					// 30 - 60 : 5 per second
					// 60 and more : 1 per 2 sec

					if (need_to_update_10 && distance <= distance_30)
					{
						m_updator->write_update_for(entity->ID, packet);
						arts += packet.B.count;
					}
					else if (need_to_update_5 && distance <= distance_60)
					{
						m_updator->write_update_for(entity->ID, packet);
						arts += packet.B.count;
					}
					else if (need_to_update_05)
					{
						m_updator->write_update_for(entity->ID, packet);
						arts += packet.B.count;
					}
				}
				else if (entity->cast_inventory_item())
				{
					if (has_parent)
					{					
						// Inventory items with parent
						// 0 - 50	: 15 per second
						// 50 - 200 : 10 per second
						// 200 - 300 : 5 per second
						// 300 and more : 1 per sec

						if (distance <= distance_50 && need_to_update_15)
						{
							m_updator->write_update_for(entity->ID, packet);


							items += packet.B.count;

							if (entity->cast_item_weapon())
								item_weapon += packet.B.count;
						}						
						else if (distance <= distance_200 && need_to_update_10)
						{
							m_updator->write_update_for(entity->ID, packet);

							items += packet.B.count;

							if (entity->cast_item_weapon())
								item_weapon += packet.B.count;
						}
						else if (need_to_update_5 && distance <= distance_300)
						{
							m_updator->write_update_for(entity->ID, packet);

							items += packet.B.count;

							if (entity->cast_item_weapon())
								item_weapon += packet.B.count;
						}
						else if (need_to_update_1)
						{
							m_updator->write_update_for(entity->ID, packet);

							items += packet.B.count;

							if (entity->cast_item_weapon())
								item_weapon += packet.B.count;
						}
					}
					else
					{
						// ���� ������������ ������ �����������, ������ ��� ������ ������� ������� �� �����.
						// ����� ������� �� ����������� ��������� (�� ������� ���� �� ������).
						m_updator->write_update_for(entity->ID, packet);

						items += packet.B.count;

						if (entity->cast_item_weapon())
							item_weapon += packet.B.count;
					}
				}
				
				else if (smart_cast<CSE_ALifeObjectPhysic*>(entity))
				{
 					m_updator->write_update_for(entity->ID, packet);
					physics += packet.B.count;
				}
				else
				{
					m_updator->write_update_for(entity->ID, packet);
					others += packet.B.count;
				}
			} // end for

			if (need_to_update_15)
				CL->m_last_update_time_15 = Device.dwTimeGlobal;

			if (need_to_update_10)
				CL->m_last_update_time_10 = Device.dwTimeGlobal;

			if (need_to_update_5)
				CL->m_last_update_time_5 = Device.dwTimeGlobal;

			if (need_to_update_1)
				CL->m_last_update_time_1 = Device.dwTimeGlobal;

			if (need_to_update_05)
				CL->m_last_update_time_05 = Device.dwTimeGlobal;

			m_updator->end_updates(m_update_begin, m_update_end);

			// send packets to client

			for (update_iterator_t i = m_update_begin; i != m_update_end; ++i)
			{
				NET_Packet& P = **i;
				if (P.B.count > 2)
				{
					m_owner->SendTo_LL(client->ID, P.B.data, P.B.count, m_dwFlags);
				}
			}
		}
	};

	SenderFunctor temp_functor(this, m_update_packets, &m_updator, net_flags(FALSE, TRUE));
	net_players.ForFoundClientsDo(ClientExcluderPredicate(GetServerClient()->ID), temp_functor);

#endif // OLD_SYNC
}

u32 xrServerExportKBS = 0;
 
u32 ai_stalker_cons = 0;
u32 ai_monster_cons = 0;
u32 item_cons = 0;
u32 arts_cons = 0;
u32 actors_cons = 0;
u32 others_cons = 0;
u32 physic_cons = 0;

u32 item_weapon_cons = 0;


void xrServer::SendUpdatesToAll()
{
	if (IsGameTypeSingle())
		return;
	
	KickCheaters();


	//sending game_update 
	fastdelegate::FastDelegate1<IClient*,void> sendtofd;
	sendtofd.bind(this, &xrServer::SendGameUpdateTo);
	ForEachClientDoSender(sendtofd);

	if ((Device.dwTimeGlobal - m_last_update_time) >= u32(1000/psNET_ServerUpdate))
	{
		MakeUpdatePackets				();
		SendUpdatePacketsToAll			();

#ifdef DEBUG
		g_sv_SendUpdate = 0;
#endif			
		if (game->sv_force_sync)	Perform_game_export();
		VERIFY						(verify_entities());
		m_last_update_time			= Device.dwTimeGlobal;
	}

	if (Device.dwTimeGlobal - xrServerExportKBS > 1000)
	{
		//Msg("Ai_Stalker [%d]", ai_stalker);
		ai_stalker_cons = ai_stalker;
		ai_stalker = 0;
		
		ai_monster_cons = ai_mutants;
		ai_mutants = 0;

		item_cons = items;
		items = 0;

		arts_cons = arts;
		arts = 0;

		actors_cons = actors;
		actors = 0;

		others_cons = others;
		others = 0;

		physic_cons = physics;
		physics = 0;

		item_weapon_cons = item_weapon;
		item_weapon = 0;

		xrServerExportKBS = Device.dwTimeGlobal;
	}

	if (m_file_transfers)
	{
		m_file_transfers->update_transfer();
		m_file_transfers->stop_obsolete_receivers();
	}
}

xr_vector<shared_str>	_tmp_log;
void console_log_cb(LPCSTR text)
{
	_tmp_log.push_back	(text);
}

u32 xrServer::OnDelayedMessage	(NET_Packet& P, ClientID sender)			// Non-Zero means broadcasting with "flags" as returned
{
	u16						type;
	P.r_begin				(type);

	//csPlayers.Enter			();

	VERIFY							(verify_entities());
	xrClientData* CL				= ID_to_client(sender);
	//R_ASSERT2						(CL, make_string("packet type [%d]",type).c_str());

	switch (type)
	{
		case M_CLIENT_REQUEST_CONNECTION_DATA:
		{
			IClient* tmp_client = net_players.GetFoundClient(
				ClientIdSearchPredicate(sender));
			VERIFY(tmp_client);
			OnCL_Connected(tmp_client);
		}break;
		case M_REMOTE_CONTROL_CMD:
		{
			if(CL->m_admin_rights.m_has_admin_rights)
			{
				string1024			buff;
				P.r_stringZ			(buff);
				Msg("* Radmin [%s] is running command: %s", CL->ps->getName(), buff);
				SetLogCB			(console_log_cb);
				_tmp_log.clear		();
				LPSTR		result_command;
				string64	tmp_number_str;
				xr_sprintf(tmp_number_str, " raid:%u", CL->ID.value());
				STRCONCAT(result_command, buff, tmp_number_str);
				Console->Execute	(result_command);
				SetLogCB			(NULL);

				NET_Packet			P_answ;			
				for(u32 i=0; i<_tmp_log.size(); ++i)
				{
					P_answ.w_begin		(M_REMOTE_CONTROL_CMD);
					P_answ.w_stringZ	(_tmp_log[i]);
					SendTo				(sender,P_answ,net_flags(TRUE,TRUE));
				}
			}else
			{
				NET_Packet			P_answ;			
				P_answ.w_begin		(M_REMOTE_CONTROL_CMD);
				P_answ.w_stringZ	("you dont have admin rights");
				SendTo				(sender,P_answ,net_flags(TRUE,TRUE));
			}
		}break;
		case M_FILE_TRANSFER:
		{
			m_file_transfers->on_message(&P, sender);
		}break;
	}
	VERIFY							(verify_entities());

	//csPlayers.Leave					();
	return 0;
}

u32	xrServer::OnMessageSync(NET_Packet& P, ClientID sender)
{
	csMessage.Enter();
	u32 ret = OnMessage(P, sender);
	csMessage.Leave();
	return ret;
}

extern	float	g_fCatchObjectTime;
u32 xrServer::OnMessage	(NET_Packet& P, ClientID sender)			// Non-Zero means broadcasting with "flags" as returned
{
	u16			type;
	P.r_begin	(type);

	VERIFY							(verify_entities());
	xrClientData* CL				= ID_to_client(sender);

	switch (type)
	{
		case M_EVENT_CMD_SYNC:
		{
			SendBroadcast(sender, P, net_flags(true, true));
		}break;
		case M_UPDATE:	
			{
				Process_update			(P,sender);						// No broadcast
				VERIFY					(verify_entities());
			}break;
		case M_SPAWN:	
			{
				if (CL->flags.bLocal)
					Process_spawn		(P,sender);	

				VERIFY					(verify_entities());
			}break;
		case M_EVENT:	
			{
				Process_event			(P,sender);
				VERIFY					(verify_entities());
			}break;
		case M_EVENT_PACK:
			{
				NET_Packet	tmpP;
				while (!P.r_eof())
				{
					tmpP.B.count		= P.r_u8();
					P.r					(&tmpP.B.data, tmpP.B.count);

					OnMessage			(tmpP, sender);
				};			
			}break;
		case M_CL_UPDATE:
			{
				xrClientData* CL		= ID_to_client	(sender);
				if (!CL)				break;
				CL->net_Ready			= TRUE;

				if (!CL->net_PassUpdates)
					break;
				//-------------------------------------------------------------------
				u32 ClientPing = CL->stats.getPing();
				P.w_seek(P.r_tell()+2, &ClientPing, 4);
				//-------------------------------------------------------------------
				if (SV_Client) 
					SendTo	(SV_Client->ID, P, net_flags(TRUE, TRUE));
				VERIFY					(verify_entities());
			}break;
		case M_MOVE_PLAYERS_RESPOND:
			{
				xrClientData* CL		= ID_to_client	(sender);
				if (!CL)				break;
				CL->net_Ready			= TRUE;
				CL->net_PassUpdates		= TRUE;
			}break;
		//-------------------------------------------------------------------
		case M_CL_INPUT:
			{
				xrClientData* CL		= ID_to_client	(sender);
				if (CL)	CL->net_Ready	= TRUE;
				if (SV_Client) SendTo	(SV_Client->ID, P, net_flags(TRUE, TRUE));
				VERIFY					(verify_entities());
			}break;
		case M_GAMEMESSAGE:
			{
				SendBroadcast			(BroadcastCID,P,net_flags(TRUE,TRUE));
				VERIFY					(verify_entities());
			}break;
		case M_CLIENTREADY:
			{
				game->OnPlayerConnectFinished(sender);
				//game->signal_Syncronize	();
				VERIFY					(verify_entities());
			}break;
		case M_SWITCH_DISTANCE:
			{
				game->switch_distance	(P,sender);
				VERIFY					(verify_entities());
			}break;
		case M_CHANGE_LEVEL:
			{
				game->AddDelayedEvent(P, M_CHANGE_LEVEL, 0, sender);
			
				VERIFY					(verify_entities());
			}break;
		case M_SAVE_GAME:
			{
				game->save_game			(P,sender);
				VERIFY					(verify_entities());
			}break;
		case M_LOAD_GAME:
			{
				game->load_game			(P,sender);
				SendBroadcast			(BroadcastCID,P,net_flags(TRUE,TRUE));
				VERIFY					(verify_entities());
			}break;
		case M_RELOAD_GAME:
			{
				SendBroadcast			(BroadcastCID,P,net_flags(TRUE,TRUE));
				VERIFY					(verify_entities());
			}break;
		case M_SAVE_PACKET:
			{
				Process_save			(P,sender);
				VERIFY					(verify_entities());
			}break;
		case M_CLIENT_REQUEST_CONNECTION_DATA:
			{
				AddDelayedPacket(P, sender);
			}break;
		case M_CHAT_MESSAGE:
			{
				xrClientData *l_pC			= ID_to_client(sender);
				OnChatMessage				(&P, l_pC);
			}break;
		case M_SV_MAP_NAME:
			{
				xrClientData *l_pC			= ID_to_client(sender);
				OnProcessClientMapData		(P, l_pC->ID);
			}break;
		case M_SV_DIGEST:
			{
				R_ASSERT(CL);
				ProcessClientDigest			(CL, &P);
			}break;
		case M_CHANGE_LEVEL_GAME:
			{
				ClientID CID; CID.set		(0xffffffff);
				SendBroadcast				(CID,P,net_flags(TRUE,TRUE));
			}break;
		case M_CL_AUTH:
			{
				game->AddDelayedEvent		(P,GAME_EVENT_PLAYER_AUTH, 0, sender);
			}break;
		case M_CREATE_PLAYER_STATE:
			{
				game->AddDelayedEvent		(P,GAME_EVENT_CREATE_PLAYER_STATE, 0, sender);
			}break;
		case M_STATISTIC_UPDATE:
			{
				SendBroadcast			(BroadcastCID,P,net_flags(TRUE,TRUE));
			}break;
		case M_STATISTIC_UPDATE_RESPOND:
			{
				//client method for collecting statistics are called from two places : 1 - this, 2 - game_sv_mp::WritePlayerStats
				if (GameID() != eGameIDSingle)
				{
					game_sv_mp* my_game = static_cast<game_sv_mp*>(game);
					if (CL)
					{
						my_game->m_async_stats.set_responded(CL->ID);
						if (static_cast<IClient*>(CL) != GetServerClient())
						{
							game_PlayerState* tmp_ps = CL->ps;
							u32 tmp_pid = tmp_ps != NULL ? tmp_ps->m_account.profile_id() : 0;
							Game().m_WeaponUsageStatistic->OnUpdateRespond(&P, CL->m_cdkey_digest, tmp_pid);
						}
					} else
					{
						Msg("! ERROR: SV: update respond received from unknown sender");
					}
				}			
				//if (SV_Client) SendTo	(SV_Client->ID, P, net_flags(TRUE, TRUE));
			}break;
		case M_PLAYER_FIRE:
			{
				if (game)
					game->OnPlayerFire(sender, P);
			}break;
		case M_REMOTE_CONTROL_AUTH:
			{
				string512				reason;
				shared_str				user;
				shared_str				pass;
				P.r_stringZ				(user);
				if(0==stricmp(user.c_str(),"logoff"))
				{
					CL->m_admin_rights.m_has_admin_rights	= FALSE;
					if (CL->ps)
					{
						CL->ps->resetFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS);
						game->signal_Syncronize();
					}
					xr_strcpy				(reason,"logged off");
					Msg("# Remote administrator logged off.");
				}else
				{
					P.r_stringZ				(pass);
					bool res = CheckAdminRights(user, pass, reason);
					if(res){
						CL->m_admin_rights.m_has_admin_rights	= TRUE;
						CL->m_admin_rights.m_dwLoginTime		= Device.dwTimeGlobal;
						if (CL->ps)
						{
							CL->ps->setFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS);
							game->signal_Syncronize();
						}
						Msg("# User [%s] logged as remote administrator.", user.c_str());
					}else
						Msg("# User [%s] tried to login as remote administrator. Access denied.", user.c_str());

				}
				NET_Packet			P_answ;			
				P_answ.w_begin		(M_REMOTE_CONTROL_AUTH);
				P_answ.w_stringZ	(reason);
				SendTo				(CL->ID,P_answ,net_flags(TRUE,TRUE));
			}break;

		case M_REMOTE_CONTROL_CMD:
			{
				AddDelayedPacket(P, sender);
			}break;
		case M_BATTLEYE:
			{
			}break;
		case M_FILE_TRANSFER:
			{
				AddDelayedPacket(P, sender);
			}break;
		case M_SECURE_KEY_SYNC:
			{
				PerformSecretKeysSyncAck(CL, P);
			}break;
		case M_SECURE_MESSAGE:
			{
				OnSecureMessage(P, CL);
			}break;
		case M_SCRIPT_EVENT:
			{
				OnScriptEvent(P, sender);
			}break;
		
		case M_VOICE_MESSAGE:
		{
			OnVoiceMessage(P, sender);
		}break;

		case M_CL_UPDATE_ACTIVE_SLOT:
		{
			SendBroadcast(sender, P, net_flags(true, true));
		}break;

	}

	VERIFY							(verify_entities());

	return							inherited::OnMessage(P, sender);
}

bool xrServer::CheckAdminRights(const shared_str& user, const shared_str& pass, string512& reason)
{
	bool res			= false;
	string_path			fn;
	FS.update_path		(fn,"$app_data_root$","radmins.ltx");
	if(FS.exist(fn))
	{
		CInifile			ini(fn);
		if(ini.line_exist("radmins",user.c_str()))
		{
			if (ini.r_string ("radmins",user.c_str()) == pass)
			{
				xr_strcpy			(reason, sizeof(reason),"Access permitted.");
				res				= true;
			}else
			{
				xr_strcpy			(reason, sizeof(reason),"Access denied. Wrong password.");
			}
		}else
			xr_strcpy			(reason, sizeof(reason),"Access denied. No such user.");
	}else
		xr_strcpy				(reason, sizeof(reason),"Access denied.");

	return				res;
}

// Server sending data


xr_map<int, u32> dump_network;
xr_map<int, u32> dump_total_network;

void xrServer::GetDataNetwork(xr_vector<shared_str>& datavec)
{
	for (auto data : dump_network)
	{
		string128 tmp;
		sprintf(tmp, "Sended : EventID[%u] : Size: %u", data.first, data.second);
		datavec.push_back(tmp);
 	}

	for (auto data : dump_total_network)
	{
		string128 tmp;
		sprintf(tmp, "[TOTAL] Sended : EventID[%u] : Size: %u", data.first, data.second);
		datavec.push_back(tmp);
	}
	dump_network.clear();
}

void xrServer::SendTo(ClientID ID, NET_Packet& P, u32 dwFlags, u32 dwTimeout)
{
	u16 IDVal;
	P.r_begin(IDVal);
	dump_network[IDVal] += P.B.count;
	dump_total_network[IDVal] += P.B.count;
	SendTo_LL(ID, P.B.data, P.B.count, dwFlags, dwTimeout);
}
 
void xrServer::SendTo_LL(ClientID ID, void* data, u32 size, u32 dwFlags, u32 dwTimeout)
{
	if ((SV_Client && SV_Client->ID == ID) || (psNET_direct_connect))
	{
		// optimize local traffic
		Level().OnMessage(data, size);
	}
	else
	{
		IClient* pClient = ID_to_client(ID);
		VERIFY2(pClient && pClient->flags.bConnected, "trying to send packet to disconnected client");
		if (!pClient || !pClient->flags.bConnected)
			return;

		inherited::SendTo_Buf(ID, data, size, dwFlags, dwTimeout);
	}
}


void xrServer::SendBroadcast(ClientID exclude, NET_Packet& P, u32 dwFlags)
{
 	u16 IDVal;
	P.r_begin(IDVal);
	dump_network[IDVal] += P.B.count;
	dump_total_network[IDVal] += P.B.count;

	//if (IDVal == 9 && P.B.count == 40)
	//{
	//	Debug.Callstack();
	//	Msg("Strange Event: %u", P.B.count);
	//}

	struct ClientExcluderPredicate
	{
		ClientID id_to_exclude;
		ClientExcluderPredicate(ClientID exclude) :
			id_to_exclude(exclude)
		{
		}
		bool operator()(IClient* client)
		{
			xrClientData* tmp_client = static_cast<xrClientData*>(client);
			if (client->ID == id_to_exclude)
				return false;
			if (!client->flags.bConnected)
				return false;
			if (!tmp_client->net_Accepted)
				return false;
			return true;
		}
	};
	struct ClientSenderFunctor
	{
		xrServer* m_owner;
		void* m_data;
		u32				m_size;
		u32				m_dwFlags;
		ClientSenderFunctor(xrServer* owner, void* data, u32 size, u32 dwFlags) :
			m_owner(owner), m_data(data), m_size(size), m_dwFlags(dwFlags)
		{
		}
		void operator()(IClient* client)
		{
			m_owner->SendTo_LL(client->ID, m_data, m_size, m_dwFlags);
		}
	};
	ClientSenderFunctor temp_functor(this, P.B.data, P.B.count, dwFlags);
	net_players.ForFoundClientsDo(ClientExcluderPredicate(exclude), temp_functor);
}

//--------------------------------------------------------------------
CSE_Abstract*	xrServer::entity_Create		(LPCSTR name)
{
	return F_entity_Create(name);
}

void			xrServer::entity_Destroy	(CSE_Abstract *&P)
{
	//Msg("xrServer::entity_Destroy : [%d][%s][%s]", P->ID, P->name(), P->name_replace());
	
 	R_ASSERT					(P);

	entities.erase				(P->ID);

	m_tID_Generator.vfFreeID	(P->ID, Device.TimerAsync());

	if(P->owner && P->owner->owner==P)
		P->owner->owner		= NULL;

	if(P->owner)
		P->owner = NULL;

	if (!ai().get_alife() || !P->m_bALifeControl)
	{
		F_entity_Destroy		(P);
	}
 
}

//--------------------------------------------------------------------
void			xrServer::Server_Client_Check	( IClient* CL )
{
	if (SV_Client && SV_Client->ID == CL->ID)
	{
		if (!CL->flags.bConnected)
		{
			SV_Client = NULL;
		};
		return;
	};

	if (SV_Client && SV_Client->ID != CL->ID)
	{
		return;
	};


	if (!CL->flags.bConnected) 
	{
		return;
	};

	if( CL->process_id == GetCurrentProcessId() )
	{
		CL->flags.bLocal	= 1;
		SV_Client			= (xrClientData*)CL;
		Msg( "New SV client 0x%08x", SV_Client->ID.value());
	}else
	{
		CL->flags.bLocal	= 0;
	}
};

bool		xrServer::OnCL_QueryHost		() 
{
	if (game->Type() == eGameIDSingle) return false;
	return (GetClientsCount() != 0); 
};

CSE_Abstract*	xrServer::GetEntity			(u32 Num)
{
	xrS_entities::iterator	I=entities.begin(),E=entities.end();
	for (u32 C=0; I!=E; ++I,++C)
	{
		if (C == Num) return I->second;
	};
	return NULL;
}

xrS_entities* xrServer::GetEntitys()
{
	return &entities;
}

void		xrServer::OnChatMessage(NET_Packet* P, xrClientData* CL)
{
	if (!CL->net_Ready)
		return;

	struct MessageSenderController
	{
		xrServer*			m_owner;
		s16					m_team;
		game_PlayerState*	m_sender_ps;
		NET_Packet*			m_packet;
		MessageSenderController(xrServer* owner) :
			m_owner(owner)
		{}
		void operator()(IClient* client)
		{
			xrClientData* xr_client = static_cast<xrClientData*>(client);
			game_PlayerState* ps = xr_client->ps;
			if (!ps)
				return;
			if (!xr_client->net_Ready)
				return;
			if (m_team != -1 && ps->team != m_team)
				return;
			if (m_sender_ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD) &&
				!ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
			{
				return;
			}
			m_owner->SendTo(client->ID, *m_packet);
		}
	};
	MessageSenderController	mesenger(this);
	mesenger.m_team			= P->r_s16();
	mesenger.m_sender_ps	= CL->ps;
	mesenger.m_packet		= P;
	ForEachClientDoSender(mesenger);
};

#ifdef DEBUG

static	BOOL	_ve_initialized			= FALSE;
static	BOOL	_ve_use					= TRUE;

bool xrServer::verify_entities				() const
{
	if (!_ve_initialized)	{
		_ve_initialized					= TRUE;
		if (strstr(Core.Params,"-~ve"))	_ve_use=FALSE;
	}
	if (!_ve_use)						return true;

	xrS_entities::const_iterator		I = entities.begin();
	xrS_entities::const_iterator		E = entities.end();
	for ( ; I != E; ++I) {
		VERIFY2							((*I).first != 0xffff,"SERVER : Invalid entity id as a map key - 0xffff");
		VERIFY2							((*I).second,"SERVER : Null entity object in the map");
		VERIFY3							((*I).first == (*I).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*I).second->name_replace());
		verify_entity					((*I).second);
	}
	return								(true);
}

void xrServer::verify_entity				(const CSE_Abstract *entity) const
{
	VERIFY(entity->m_wVersion!=0);
	if (entity->ID_Parent != 0xffff) {
		xrS_entities::const_iterator	J = entities.find(entity->ID_Parent);
		VERIFY2							(J != entities.end(),
			make_string("SERVER : Cannot find parent in the map [%s][%s]",entity->name_replace(),
			entity->name()).c_str());
		VERIFY3							((*J).second,"SERVER : Null entity object in the map",entity->name_replace());
		VERIFY3							((*J).first == (*J).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*J).second->name_replace());
		VERIFY3							(std::find((*J).second->children.begin(),(*J).second->children.end(),entity->ID) != (*J).second->children.end(),"SERVER : Parent/Children relationship mismatch - Object has parent, but corresponding parent doesn't have children",(*J).second->name_replace());
	}

	xr_vector<u16>::const_iterator		I = entity->children.begin();
	xr_vector<u16>::const_iterator		E = entity->children.end();
	for ( ; I != E; ++I) {
		VERIFY3							(*I != 0xffff,"SERVER : Invalid entity children id - 0xffff",entity->name_replace());
		xrS_entities::const_iterator	J = entities.find(*I);
		VERIFY3							(J != entities.end(),"SERVER : Cannot find children in the map",entity->name_replace());
		VERIFY3							((*J).second,"SERVER : Null entity object in the map",entity->name_replace());
		VERIFY3							((*J).first == (*J).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*J).second->name_replace());
		VERIFY3							((*J).second->ID_Parent == entity->ID,"SERVER : Parent/Children relationship mismatch - Object has children, but children doesn't have parent",(*J).second->name_replace());
	}
}
#endif // DEBUG

void xrServer::OnVoiceMessage(NET_Packet& P, ClientID sender)
{
	xrClientData* pClient = (xrClientData*)ID_to_client(sender);

	if (!pClient || !pClient->net_Ready) return;
	game_PlayerState* ps = pClient->ps;
	if (!ps) return;
	if (!pClient->owner) return;
  
	struct send_voice_message
	{
		xrServer* m_server;
		NET_Packet* m_packet;
		xrClientData* m_from;
		float m_voiceDistanceSqr;

		void operator()(IClient* client)
		{
			if (client == m_server->GetServerClient()) return;

			xrClientData* CL = static_cast<xrClientData*>(client);
			if (!CL || !CL->net_Ready || !CL->owner || !m_from->owner || !m_from->ps) return;
 			if (CL->ID == m_from->ID) return;

			game_PlayerState* ps = CL->ps;
			if (!ps || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) return;

			// se7kills (���������� ���� �������)
			//float distanceSqr = CL->owner->Position().distance_to_sqr(m_from->owner->Position());
			//
			//if (distanceSqr <= m_voiceDistanceSqr)
			//{
			//	m_server->SendTo(CL->ID, *m_packet, net_flags(FALSE, TRUE, TRUE, TRUE));
			//}
 
			m_server->SendTo(CL->ID, *m_packet, net_flags(FALSE, TRUE, TRUE, TRUE));
		}
	};

	u8 distance = P.r_u8(); // distance byte
	float voiceDistanceSqr = (float)distance * (float)distance;

	send_voice_message tmp_functor;
	tmp_functor.m_server = this;
	tmp_functor.m_packet = &P;
	tmp_functor.m_from = pClient;
	tmp_functor.m_voiceDistanceSqr = voiceDistanceSqr;

	ForEachClientDo(tmp_functor);
};

shared_str xrServer::level_name				(const shared_str &server_options) const
{
	return	(game->level_name(server_options));
}
shared_str xrServer::level_version			(const shared_str &server_options) const
{
	return	(game_sv_GameState::parse_level_version(server_options));						
}

void xrServer::create_direct_client()
{
	SClientConnectData cl_data;
	cl_data.clientID.set(1);
	xr_strcpy( cl_data.name, "single_player" );
	cl_data.process_id = GetCurrentProcessId();
	
	new_client( &cl_data );
}


void xrServer::ProceedDelayedPackets()
{
	DelayedPackestCS.Enter();
	while (!m_aDelayedPackets.empty())
	{
		DelayedPacket& DPacket	= *m_aDelayedPackets.begin();
		OnDelayedMessage(DPacket.Packet, DPacket.SenderID);
//		OnMessage(DPacket.Packet, DPacket.SenderID);
		m_aDelayedPackets.pop_front();
	}
	DelayedPackestCS.Leave();
};

void xrServer::AddDelayedPacket	(NET_Packet& Packet, ClientID Sender)
{
	DelayedPackestCS.Enter();

	m_aDelayedPackets.push_back(DelayedPacket());
	DelayedPacket* NewPacket = &(m_aDelayedPackets.back());
	NewPacket->SenderID = Sender;
	CopyMemory	(&(NewPacket->Packet),&Packet,sizeof(NET_Packet));	

	DelayedPackestCS.Leave();
}

u32 g_sv_dwMaxClientPing		= 2000;
u32 g_sv_time_for_ping_check	= 15000;// 15 sec
u8	g_sv_maxPingWarningsCount	= 5;

void xrServer::PerformCheckClientsForMaxPing()
{
	struct MaxPingClientDisconnector
	{
		xrServer* m_owner;
		MaxPingClientDisconnector(xrServer* owner) :
			m_owner(owner)
		{}
		void operator()(IClient* client)
		{
			xrClientData*		Client = static_cast<xrClientData*>(client);
			game_PlayerState*	ps = Client->ps;
			if (!ps)
				return;
			
			if (client == m_owner->GetServerClient())
				return;

			if(	ps->ping > g_sv_dwMaxClientPing && 
				Client->m_ping_warn.m_dwLastMaxPingWarningTime+g_sv_time_for_ping_check < Device.dwTimeGlobal )
			{
				++Client->m_ping_warn.m_maxPingWarnings;
				Client->m_ping_warn.m_dwLastMaxPingWarningTime	= Device.dwTimeGlobal;
				
				if(Client->m_ping_warn.m_maxPingWarnings >= g_sv_maxPingWarningsCount)
				{  //kick
					LPSTR	reason;
					STRCONCAT( reason, CStringTable().translate("st_kicked_by_server").c_str() );
					Level().Server->DisconnectClient( Client, reason );
				}
				else
				{ //send warning
					NET_Packet		P;	
					P.w_begin		(M_CLIENT_WARN);
					P.w_u8			(1); // 1 means max-ping-warning
					P.w_u16			(ps->ping);
					P.w_u8			(Client->m_ping_warn.m_maxPingWarnings);
					P.w_u8			(g_sv_maxPingWarningsCount);
					m_owner->SendTo	(Client->ID,P,net_flags(FALSE,TRUE));
				}
			}
		}
	};
	MaxPingClientDisconnector temp_functor(this);
	ForEachClientDoSender(temp_functor);
}

extern	s32		g_sv_dm_dwFragLimit;
extern  s32		g_sv_ah_dwArtefactsNum;
extern	s32		g_sv_dm_dwTimeLimit;
extern	int		g_sv_ah_iReinforcementTime;
extern	int		g_sv_mp_iDumpStatsPeriod;
extern	BOOL	g_bCollectStatisticData;

//xr_token game_types[];
LPCSTR GameTypeToString(EGameIDs gt, bool bShort);

extern u64 UpdateCLStalkerTime = 0;
extern u64 UpdateCLTime = 0;

u64 UpdateCLMonster_temp = 0;
u64 UpdateCLStalker_temp = 0;


u32 clearTime = 0;
extern u32 SendedBytes;
extern u32 SyncAlifeObjects;
extern int SyncAlifeCount;

#include "game_sv_freemp.h"

extern int debuging_networking;

void xrServer::GetServerInfo( CServerInfo* si )
{
	if (debuging_networking)
	{
		xr_vector<shared_str> text_to_draw;
		GetDataNetwork(text_to_draw);
		for (auto text : text_to_draw)
		{
			si->AddItem("S", text.c_str(), RGB(128, 128, 255));
		}
		return;
	}

	string32  tmp;
	string256 tmp256;
	si->AddItem( "Server port", itoa( GetPort(), tmp, 10 ), RGB(128,128,255) );
	
	LPCSTR time = InventoryUtilities::GetTimeAsString( Device.dwTimeGlobal, InventoryUtilities::etpTimeToSecondsAndDay ).c_str();	
	si->AddItem( "Uptime", time, RGB(255,228,0) );

	xr_strcpy( tmp256, GameTypeToString( game->Type(), true ) );
	si->AddItem("Game type", tmp256, RGB(128, 255, 255));

	sprintf(tmp256, "%u kbs | AlifeObjets: %u | SendRateO: %u", SendedBytes / 1024, SyncAlifeObjects, SyncAlifeCount);
	si->AddItem("AlifeSync", tmp256, RGB(128, 255, 255));
	SendedBytes = 0;

	tmp256[0] = NULL;
 
 	sprintf_s(tmp256, "st[%u], m[%u], item[%u], arts[%u], wpn[%u], PH[%u], other[%u]", 
		ai_stalker_cons, ai_monster_cons, item_cons, arts_cons, item_weapon_cons, physic_cons, others_cons);
	si->AddItem("KBS", tmp256, RGB(128, 255, 255));

 	u32 stalkers = 0;
	u32 stalkersAlive = 0;
	u32 mosters = 0;
	u32 mostersAlive = 0;
	u32 items = 0;
	u32 arts = 0;
 
	for (xrS_entities::iterator iter = entities.begin(); iter != entities.end(); iter++)
	{
		CSE_Abstract* abs = (*iter).second;
		CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(abs);
		CSE_ALifeMonsterBase* monster = smart_cast<CSE_ALifeMonsterBase*>(abs);
		CSE_ALifeItem* item = smart_cast<CSE_ALifeItem*>(abs);
		CSE_ALifeItemArtefact* artefact = smart_cast<CSE_ALifeItemArtefact*>(abs);
  		if (stalker)
		{
			if (stalker->g_Alive())
				stalkersAlive += 1;
 			stalkers += 1;
		}
 		if (monster)
		{
			if (monster->g_Alive())
				mostersAlive += 1;
 			mosters += 1;
		}
 		if (item)
 			items += 1;
 		if (artefact)
 			arts += 1;
 	}
 
	string128 tmpINFO = { 0 };	
	sprintf(tmpINFO, "stalkers: %u/%u, monsters: %u/%u", stalkers, stalkersAlive, mosters, mostersAlive);
	si->AddItem("Info", tmpINFO, RGB(0, 255, 0));
	 	 
	if (clearTime + 1000 < Device.dwTimeGlobal)
	{
		UpdateCLMonster_temp = UpdateCLTime;
		UpdateCLStalker_temp = UpdateCLStalkerTime;

		UpdateCLTime = 0;
		UpdateCLStalkerTime = 0;
		clearTime = Device.dwTimeGlobal;
	}

	game_sv_freemp* freemp = smart_cast<game_sv_freemp*>(game);
	if (freemp)
		freemp->GetServerInfo(si);
}

void xrServer::AddCheater			(shared_str const & reason, ClientID const & cheaterID)
{
	CheaterToKick new_cheater;
	new_cheater.reason = reason;
	new_cheater.cheater_id = cheaterID;
	m_cheaters.push_back(new_cheater);
}

void xrServer::KickCheaters			()
{
	for (cheaters_t::iterator i = m_cheaters.begin(),
		ie = m_cheaters.end(); i != ie; ++i)
	{
		IClient* tmp_client = GetClientByID(i->cheater_id);
		if (!tmp_client)
		{
			Msg("! ERROR: KickCheaters: client [%u] not found", i->cheater_id);
			continue;
		}
		ClientID tmp_client_id = tmp_client->ID;
		DisconnectClient(tmp_client, i->reason.c_str());
		
		NET_Packet		P;
		P.w_begin		( M_GAMEMESSAGE ); 
		P.w_u32			( GAME_EVENT_SERVER_STRING_MESSAGE );
		P.w_stringZ		( i->reason.c_str() + 2 );
		Level().Server->SendBroadcast( tmp_client_id, P );
	}
	m_cheaters.clear();
}

void xrServer::MakeScreenshot(ClientID const & admin_id, ClientID const & cheater_id)
{
	if ((cheater_id == SV_Client->ID) && g_dedicated_server)
	{
		return;
	}
	for (int i = 0; i < sizeof(m_screenshot_proxies)/sizeof(clientdata_proxy*); ++i)
	{
		if (!m_screenshot_proxies[i]->is_active())
		{
			m_screenshot_proxies[i]->make_screenshot(admin_id, cheater_id);
			Msg("* admin [%d] is making screeshot of client [%d]", admin_id, cheater_id);
			return;
		}
	}
	Msg("! ERROR: SV: not enough file transfer proxies for downloading screenshot, please try later ...");
}
void xrServer::MakeConfigDump(ClientID const & admin_id, ClientID const & cheater_id)
{
	if ((cheater_id == SV_Client->ID) && g_dedicated_server)
	{
		return;
	}
	for (int i = 0; i < sizeof(m_screenshot_proxies)/sizeof(clientdata_proxy*); ++i)
	{
		if (!m_screenshot_proxies[i]->is_active())
		{
			m_screenshot_proxies[i]->make_config_dump(admin_id, cheater_id);
			Msg("* admin [%d] is making config dump of client [%d]", admin_id, cheater_id);
			return;
		}
	}
	Msg("! ERROR: SV: not enough file transfer proxies for downloading file, please try later ...");
}


void xrServer::initialize_screenshot_proxies()
{
	for (int i = 0; i < sizeof(m_screenshot_proxies)/sizeof(clientdata_proxy*); ++i)
	{
		m_screenshot_proxies[i] = xr_new<clientdata_proxy>(m_file_transfers);
	}
}
void xrServer::deinitialize_screenshot_proxies()
{
	for (int i = 0; i < sizeof(m_screenshot_proxies)/sizeof(clientdata_proxy*); ++i)
	{
		xr_delete(m_screenshot_proxies[i]);
	}
}

struct PlayerInfoWriter
{
	NET_Packet*	dest;
	void operator()(IClient* C)
	{
		xrClientData* tmp_client = smart_cast<xrClientData*>(C);
		if (!tmp_client)
			return;

		dest->w_clientID(tmp_client->ID);
		dest->w_stringZ(tmp_client->m_cAddress.to_string().c_str());
		dest->w_stringZ(tmp_client->m_cdkey_digest);
	}
};//struct PlayerInfoWriter

void xrServer::SendPlayersInfo(ClientID const & to_client)
{
	PlayerInfoWriter tmp_functor;
	NET_Packet tmp_packet;
	tmp_packet.w_begin	(M_GAMEMESSAGE); 
	tmp_packet.w_u32	(GAME_EVENT_PLAYERS_INFO_REPLY);
	tmp_functor.dest	= &tmp_packet;
	ForEachClientDo		(tmp_functor);
	SendTo				(to_client, tmp_packet, net_flags(TRUE, TRUE));
}

// script events

void xrServer::OnScriptEvent(NET_Packet & P, ClientID sender)
{
	script_server_events.push_back(ScriptEvent());
	ScriptEvent* pEvent = &(script_server_events.back());

	pEvent->SenderID = sender.value();
	CopyMemory(&(pEvent->Packet), &P, sizeof(NET_Packet));
}

void xrServer::KillStalkers()
{
	for (xrS_entities::iterator ent = entities.begin(); ent !=entities.end(); ent++ )
	{
		CSE_Abstract* entit = (*ent).second;
		CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(entit);

		if (stalker)
		{
			stalker->kill();
		}
	}
}

ScriptEvent * xrServer::GetLastServerScriptEvent()
{
	R_ASSERT2(script_server_events.size() > 0, "empty script server events");
	return &(script_server_events.back());
}

void xrServer::PopLastServerScriptEvent()
{
	script_server_events.pop_back();
}

u32 xrServer::GetSizeServerScriptEvent()
{
	return script_server_events.size();
}
