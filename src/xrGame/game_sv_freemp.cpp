#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"
#include "alife_simulator.h"
bool loaded_inventory = false;

game_sv_freemp::game_sv_freemp()
	:pure_relcase(&game_sv_freemp::net_Relcase)
{
	m_type = eGameIDFreeMp;
	loaded_inventory = false;
}

game_sv_freemp::~game_sv_freemp()
{
	Msg("[game_sv_freemp] Destroy Server");
	loaded_inventory = false;
}

void game_sv_freemp::Create(shared_str & options)
{
	inherited::Create(options);
	R_ASSERT2(rpoints[0].size(), "rpoints for players not found");

	switch_Phase(GAME_PHASE_PENDING);

	::Random.seed(GetTickCount());
	m_CorpseList.clear();

	//if (Game().Type() == eGameIDFreeMp)
	LoadParamsDeffaultFMP();

}

// player connect #1
void game_sv_freemp::OnPlayerConnect(ClientID id_who)
{
	inherited::OnPlayerConnect(id_who);

	xrClientData* xrCData = m_server->ID_to_client(id_who);
	game_PlayerState*	ps_who = get_id(id_who);

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
	 
	//RespawnPlayer(id_who, true);
}

void game_sv_freemp::AddMoneyToPlayer(game_PlayerState * ps, s32 amount)
{
	if (!ps) return;

	Msg("- Add money to player: [%u]%s, %d amount", ps->GameID, ps->getName(), amount);

	s64 total_money = ps->money_for_round;
	total_money += amount;

	if (total_money < 0)
		total_money = 0;

	if (total_money > std::numeric_limits<s32>().max())
	{
		Msg("! The limit of the maximum amount of money has been exceeded.");
		total_money = std::numeric_limits<s32>().max() - 1;
	}

	ps->money_for_round = s32(total_money);
	signal_Syncronize();
}

void game_sv_freemp::SpawnItemToActor(u16 actorId, LPCSTR name)
{
	if (!name) return;

	CSE_Abstract *E = spawn_begin(name);
	E->ID_Parent = actorId;
	E->s_flags.assign(M_SPAWN_OBJECT_LOCAL);	// flags

	CSE_ALifeItemWeapon		*pWeapon = smart_cast<CSE_ALifeItemWeapon*>(E);
	if (pWeapon)
	{
		u16 ammo_magsize = pWeapon->get_ammo_magsize();
		pWeapon->a_elapsed = ammo_magsize;
	}

	CSE_ALifeItemPDA *pPda = smart_cast<CSE_ALifeItemPDA*>(E);
	if (pPda)
	{
		pPda->m_original_owner = actorId;
	}

	spawn_end(E, m_server->GetServerClient()->ID);
}

CSE_Abstract* game_sv_freemp::SpawnItemToActorReturn(u16 actorId, LPCSTR name)
{
	if (!name) return NULL;

	CSE_Abstract* E = spawn_begin(name);
	E->ID_Parent = actorId;
	E->s_flags.assign(M_SPAWN_OBJECT_LOCAL);	// flags

	/*
	CSE_ALifeItemWeapon* pWeapon = smart_cast<CSE_ALifeItemWeapon*>(E);
	
	if (pWeapon)
	{
		u16 ammo_magsize = pWeapon->get_ammo_magsize();
		pWeapon->a_elapsed = ammo_magsize;
	}
	*/
	/*
	CSE_ALifeItemPDA* pPda = smart_cast<CSE_ALifeItemPDA*>(E);
	
	if (pPda)
	{
		pPda->m_original_owner = actorId;
	}
	*/

//	spawn_end(E, m_server->GetServerClient()->ID);

	return E;
}

void game_sv_freemp::OnTransferMoney(NET_Packet & P, ClientID const & clientID)
{
	ClientID to;
	s32 money;

	P.r_clientID(to);
	P.r_s32(money);

	Msg("* Try to transfer money from %u to %u. Amount: %d", clientID.value(), to.value(), money);

	game_PlayerState* ps_from = get_id(clientID);
	if (!ps_from)
	{
		Msg("! Can't find player state with id=%u", clientID.value());
		return;
	}

	game_PlayerState* ps_to = get_id(to);
	if (!ps_to)
	{
		Msg("! Can't find player state with id=%u", to.value());
		return;
	}

	if (money <= 0 || ps_from->money_for_round < money) return;

	AddMoneyToPlayer(ps_from, -money);
	AddMoneyToPlayer(ps_to, money);
}

void game_sv_freemp::OnPlayerReady(ClientID id_who)
{
	switch (Phase())
	{
	case GAME_PHASE_INPROGRESS:
	{
		xrClientData*	xrCData = (xrClientData*)m_server->ID_to_client(id_who);
		game_PlayerState*	ps = get_id(id_who);

		if (ps->IsSkip())
			break;

		if (!(ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)))
			break;

		bool flag = ps->testFlag(GAME_PLAYER_MP_ON_CONNECTED);

		RespawnPlayer(id_who, true);
		
		if (!flag)
		{
			if (Game().Type() == eGameIDFreeMp)
			{
				for (auto& item : spawned_items.StartItems)
				{
					SpawnItemToActor(ps->GameID, item.c_str());
				}
				// set start money
				ps->money_for_round = spawned_items.StartMoney;
			}
		}
		else
			if (LoadPlayer(ps))
				return;

			if (Game().Type() == eGameIDFreeMp)
			{
				for (auto& item : spawned_items.StartItems)
				{
					SpawnItemToActor(ps->GameID, item.c_str());
				}
				// set start money
				ps->money_for_round = spawned_items.StartMoney;
			}

	} break;

	default:
		break;
	};
}

// player disconnect
void game_sv_freemp::OnPlayerDisconnect(ClientID id_who, LPSTR Name, u16 GameID)
{
	inherited::OnPlayerDisconnect(id_who, Name, GameID);		  

	for (auto table : teamPlayers)
	{
		for (auto pl : table.second.players)
		{
			if (pl.GameID == GameID)
			{
				pl.GameID = -1;
				pl.Client = -1;

				table.second.cur_players -= 1;
			}
			
			OnPlayerUIContactsRecvestUpdate(pl.Client, GameID);
			
		}
	}
}

struct real_sender
{
	xrServer* server_for_send;
	NET_Packet* P;
	u32	flags_to_send;

	real_sender(xrServer* server, NET_Packet* Packet, u32 flags = DPNSEND_GUARANTEED)
	{
		server_for_send = server;
		P = Packet;
		flags_to_send = flags;
	}
	void operator()(IClient* client)
	{
		xrClientData* tmp_client = static_cast<xrClientData*>(client);
		game_PlayerState* ps = tmp_client->ps;
		if (!ps || !tmp_client->net_Ready)
			return;

		server_for_send->SendTo(client->ID, *P, flags_to_send);
	}
};

void game_sv_freemp::OnPlayerKillPlayer(game_PlayerState * ps_killer, game_PlayerState * ps_killed, KILL_TYPE KillType, SPECIAL_KILL_TYPE SpecialKillType, CSE_Abstract * pWeaponA)
{
	if (ps_killed)
	{
		ps_killed->setFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD);
		ps_killed->DeathTime = Device.dwTimeGlobal;
	}

	signal_Syncronize();
}

void game_sv_freemp::OnEvent(NET_Packet &P, u16 type, u32 time, ClientID sender)
{
	switch (type)
	{
		case GAME_EVENT_PLAYER_KILL: // (g_kill)
		{
			u16 ID = P.r_u16();
			xrClientData *l_pC = (xrClientData*)get_client(ID);
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
			//Msg("M_CHANGE_LEVEL");
			if (change_level(P, sender))
			{
				server().SendBroadcast(BroadcastCID, P, net_flags(TRUE, TRUE));
			}
		}break;

		case GAME_EVENT_PLAYER_NAME_ACCAUNT:
		{
 			shared_str nick;
			P.r_stringZ(nick);
			u32 team;
			P.r_u32(team);

			xrClientData* data = server().ID_to_client(sender);

			if (data && data->ps)
			{			
				LPCSTR old_name = data->name.c_str();
				data->name = nick.c_str();
				data->ps->m_account.set_player_name(nick.c_str());
				set_account_nickname(data->ps->m_account.name_login().c_str(), data->ps->m_account.password().c_str(), nick.c_str(), team);
		
				NET_Packet			P;
				GenerateGameMessage(P);
				P.w_u32(GAME_EVENT_PLAYER_NAME);
				P.w_u16(data->owner->ID);
				P.w_s16(data->ps->team);
				P.w_stringZ(old_name);
				P.w_stringZ(data->ps->getName());
				//---------------------------------------------------
				real_sender tmp_functor(m_server, &P);
				m_server->ForEachClientDoSender(tmp_functor);
				//---------------------------------------------------
				data->owner->set_name_replace(data->name.c_str());

				signal_Syncronize();
			}
			
			
		}break;

		default:
			inherited::OnEvent(P, type, time, sender);
	};
}

u32 oldTime_saveServer = 0;
u32 oldTimeInventoryBoxSave = 0;

void game_sv_freemp::Update()
{
	inherited::Update();

	if (Phase() != GAME_PHASE_INPROGRESS)
	{
		OnRoundStart();
		oldTime_saveServer = Device.dwTimeGlobal;
	}

	if (!loaded_inventory && Phase() == GAME_PHASE_INPROGRESS)
	{
		xrS_entities* entitys = server().GetEntitys();
		auto begin = entitys->begin();
		auto end = entitys->end();

		for (auto iter = begin; iter != end; iter++)
		{
			CSE_Abstract* E = server().ID_to_entity((*iter).first);
			CSE_ALifeInventoryBox* box = smart_cast<CSE_ALifeInventoryBox*>(E);

			if (box)
			{
				load_inventoryBox(box);
				
				loaded_inventory = true;
			}
		}
	}		
  
 	if (false)
	if (Device.dwTimeGlobal - oldTime_saveServer > 1000 * 60)
	{
 
		if (ai().get_alife())
		{
			string_path	S;
			S[0] = 0;
			strconcat(sizeof(S), S, "dima", "", "");
			NET_Packet			net_packet;
			net_packet.w_begin(M_SAVE_GAME);
			net_packet.w_stringZ(S);
			net_packet.w_u8(0);
			Level().Send(net_packet, net_flags(TRUE));

		}
		
		oldTime_saveServer = Device.dwTimeGlobal;
	}

	if (Device.dwTimeGlobal % 500 == 0)
	{
 
		for (auto players : teamPlayers)
		{
			for (auto player : players.second.players)
			{
				if (player.GameID != -1) 
				{
					OnPlayerUIContactsRecvestUpdate(player.Client, players.second.LeaderGameID);
					Msg("Recvest Send %d / Leader %d", player.GameID, players.second.LeaderGameID);
				}
			}
		}
	}


	if (Device.dwTimeGlobal - oldTimeInventoryBoxSave > 2000 && loaded_inventory)
	{
		xrS_entities* entitys = server().GetEntitys();
		auto begin = entitys->begin();
		auto end = entitys->end();

		for (auto iter = begin; iter != end; iter++)
		{
			CSE_Abstract* E = server().ID_to_entity((*iter).first);
			CSE_ALifeInventoryBox* box = smart_cast<CSE_ALifeInventoryBox*>(E);

			if (box)
			{
				save_inventoryBox(box);
			}
		}
		oldTimeInventoryBoxSave = Device.dwTimeGlobal;
	}
}

BOOL game_sv_freemp::OnTouch(u16 eid_who, u16 eid_what, BOOL bForced)
{
	CSE_ActorMP *e_who = smart_cast<CSE_ActorMP*>(m_server->ID_to_entity(eid_who));
	if (!e_who)
		return TRUE;

	CSE_Abstract *e_entity = m_server->ID_to_entity(eid_what);
	if (!e_entity)
		return FALSE;

	return TRUE;
}

void game_sv_freemp::on_death(CSE_Abstract* e_dest, CSE_Abstract* e_src)
{
	inherited::on_death(e_dest, e_src);

	if (!ai().get_alife())
		return;

	alife().on_death(e_dest, e_src);
}
