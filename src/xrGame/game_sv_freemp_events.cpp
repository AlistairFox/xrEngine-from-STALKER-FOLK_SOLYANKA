#include "stdafx.h"
#include "game_sv_freemp.h"

#include "Actor.h"
#include "actor_mp_client.h"

/* base class accessors
* 
	// Main accessors
	// PLAYER STATE GETTERS
	virtual		game_PlayerState*	get_eid					(u16 id);
	virtual		game_PlayerState*	get_id					(ClientID id);

	// PLAYER CLIENT GETTER
	virtual		void* get_client(u16 id); //if exist

	// PLAYER NAME
	virtual		LPCSTR				get_name_id				(ClientID id);
				LPCSTR				get_player_name_id		(ClientID id);

	// ID TO GAMEID
	virtual		u16					get_id_2_eid			(ClientID id);
	virtual		u32					get_players_count		();

	// CSE ENTITY FROM ID
	CSE_Abstract*		get_entity_from_eid		(u16 id);
*/


// player disconnect
void game_sv_freemp::OnPlayerDisconnect(ClientID id_who, LPSTR Name, u16 GameID)
{
	delete_player_from_squad(GameID);


	game_PlayerState* ps = get_eid(GameID);
	
	if (ps != nullptr)
	{
 		delete_player_from_player_list( find_squad_by_squadid(ps->MPSquadID), ps);
		Msg("Remove Player[%d] By PlayerState from Squad[%d]", ps->GameID, ps->MPSquadID);
	}


 	inherited::OnPlayerDisconnect(id_who, Name, GameID);
	CActorMP* pActor = smart_cast<CActorMP*>(Level().Objects.net_Find(GameID));

	if (pActor)
		pActor->DestroyObject();
}

void game_sv_freemp::OnPlayerKillPlayer(game_PlayerState* ps_killer, game_PlayerState* ps_killed, KILL_TYPE KillType, SPECIAL_KILL_TYPE SpecialKillType, CSE_Abstract* pWeaponA)
{
	if (ps_killed)
	{
		ps_killed->setFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD);
		ps_killed->DeathTime = Device.dwTimeGlobal;
	}

	signal_Syncronize();
}


// player connect #1
void game_sv_freemp::OnPlayerConnect(ClientID id_who)
{
	inherited::OnPlayerConnect(id_who);

	xrClientData* xrCData = m_server->ID_to_client(id_who);
	game_PlayerState* ps_who = get_id(id_who);

	if (!xrCData->flags.bReconnect)
	{
		ps_who->clear();
		ps_who->team = 0;
		ps_who->skin = -1;
	};

	ps_who->setFlag(GAME_PLAYER_FLAG_SPECTATOR);

	ps_who->resetFlag(GAME_PLAYER_FLAG_SKIP);

	if (g_dedicated_server && (xrCData == m_server->GetServerClient()))
	{
		ps_who->setFlag(GAME_PLAYER_FLAG_SKIP);
		return;
	}

	if (id_who != server().GetServerClient()->ID)
		map_alife_sended[id_who] = Device.dwTimeGlobal;
}

// player connect #2
void game_sv_freemp::OnPlayerConnectFinished(ClientID id_who)
{
	xrClientData* xrCData = m_server->ID_to_client(id_who);
	SpawnPlayer(id_who, "spectator");

	if (xrCData)
	{
		R_ASSERT2(xrCData->ps, "Player state not created yet");
		NET_Packet					P;
		GenerateGameMessage(P);
		P.w_u32(GAME_EVENT_PLAYER_CONNECTED);
		P.w_clientID(id_who);

		int team = get_account_team(xrCData->ps->m_account.name_login().c_str(), xrCData->ps->m_account.password().c_str());

		if (team > 0)
		{
			xrCData->ps->setFlag(GAME_PLAYER_MP_SAVETEAM);
			xrCData->ps->team = team;
		}
		else
		{
			xrCData->ps->team = 0;
		}

		xrCData->ps->setFlag(GAME_PLAYER_FLAG_SPECTATOR);
		xrCData->ps->setFlag(GAME_PLAYER_FLAG_READY);
		xrCData->ps->setFlag(GAME_PLAYER_MP_ON_CONNECTED);

		xrCData->ps->net_Export(P, TRUE);
		u_EventSend(P);
		xrCData->net_Ready = TRUE;
	};
}


void game_sv_freemp::OnEvent(NET_Packet& P, u16 type, u32 time, ClientID sender)
{
	switch (type)
	{
	case GAME_EVENT_PLAYER_KILL: // (g_kill)
	{
		u16 ID = P.r_u16();
		xrClientData* l_pC = (xrClientData*)get_client(ID);
		if (!l_pC) break;
		KillPlayer(l_pC->ID, l_pC->ps->GameID);
	}
	break;

	case GAME_EVENT_MP_TRADE:
	{
		OnPlayerTrade(P, sender);
	}
	break;

	case GAME_EVENT_TRANSFER_MONEY:
	{
		OnTransferMoney(P, sender);
	}
	break;

	case M_CHANGE_LEVEL:
	{
		if (change_level(P, sender))
		{
			server().SendBroadcast(BroadcastCID, P, net_flags(TRUE, TRUE));
		}
	}break;

	case GAME_EVENT_PLAYER_NAME_ACCAUNT:
	{
		shared_str nick;  P.r_stringZ(nick);
		u32 team;		  P.r_u32(team);

		xrClientData* data = server().ID_to_client(sender);
		if (data && data->ps)
		{
			data->name = nick.c_str();
			data->ps->m_account.set_player_name(nick.c_str());
			data->owner->set_name_replace(data->name.c_str());

			set_account_nickname
			(
				data->ps->m_account.name_login().c_str(),
				data->ps->m_account.password().c_str(),
				nick.c_str(),
				team
			);

			signal_Syncronize();
		}

	}break;

	// GAME SPAWNER, SKIN SELECTOR
	case GAME_EVENT_SPAWNER_SPAWN_ITEM:
	{
		xrClientData* CL = m_server->ID_to_client(sender);
		CActor* pActor = smart_cast<CActor*>(Level().Objects.net_Find(CL->ps->GameID)); R_ASSERT(pActor);

		if (CL && CL->owner)
		{
			shared_str sect;
			P.r_stringZ(sect);

			SpawnItem(sect.c_str(), CL->owner->ID);
		}
		else
			Msg("! Can't spawn item to player: %s", pActor->Name());
	}break;

	case GAME_EVENT_CHANGE_VISUAL_FROM_SKIN_SELECTOR:
	{
		xrClientData* CL = m_server->ID_to_client(sender);
		if (!CL) return;

		CActor* pActor = smart_cast<CActor*>(Level().Objects.net_Find(CL->ps->GameID)); R_ASSERT(pActor);

		string256 visual_name;
		P.r_stringZ(visual_name);

		pActor->u_EventGen(P, GE_CHANGE_VISUAL_SS, pActor->ID());
		P.w_stringZ(visual_name);
		pActor->u_EventSend(P);

		pActor->u_EventGen(P, GE_CHANGE_VISUAL, pActor->ID());
		P.w_stringZ(visual_name);
		pActor->u_EventSend(P);
	}break;

	case GAME_EVENT_SPEEAKING:
	{
		ClientID client;
		P.r_clientID(client);
		
		u8 isActive =P.r_u8();
		game_PlayerState* PS = get_id(client);
		if (PS)
		{
			PS->is_speaking = isActive;
		}
		signal_Syncronize();
	}break;

	case GAME_EVENT_ADMIN_RIGHTS:
	{
		bool Give = P.r_u8();
		ClientID id; P.r_clientID(id);

		xrClientData * data = (xrClientData*) server().ID_to_client(id);
		if (data)
		{
			if (Give)
			{
				string128 tmp;
				sprintf(tmp, "Give Admin Rights For PS: %s", data->ps->getName());
				SvSendChatMessage(tmp);
				data->ps->setFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS);
				data->m_admin_rights.m_has_admin_rights = true;
			}
			else
			{
				string128 tmp;
				sprintf(tmp, "Remove Admin Rights For PS: %s", data->ps->getName());
				SvSendChatMessage(tmp);
				data->ps->resetFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS);
				data->m_admin_rights.m_has_admin_rights = false;
			}
			signal_Syncronize();
		}
	}break;

	default:
		inherited::OnEvent(P, type, time, sender);
	};
}