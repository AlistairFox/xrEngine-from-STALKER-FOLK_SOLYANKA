#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"
#include "alife_simulator.h"
#include "xr_time.h"
#include "alife_object_registry.h"
 
void game_sv_freemp::OnAlifeCreate(CSE_Abstract* E)
{

	CSE_ALifeInventoryBox* CSEBOX = smart_cast<CSE_ALifeInventoryBox*>(E);
	if (CSEBOX)
	{
		InvBoxEntityS SBox;
		SBox.E = E;

		CInifile IniFile(&IReader((void*)(CSEBOX->m_ini_string.c_str()), xr_strlen(CSEBOX->m_ini_string.c_str())), FS.get_path("$game_config$")->m_Path);


		if (IniFile.section_exist("saving_box"))
		{
			SBox.NeedToSave = true;
		}

		ServerInventoryBoxes[E->ID] = SBox;
	}
}

game_sv_freemp::game_sv_freemp() : pure_relcase(&game_sv_freemp::net_Relcase)
{
	if (g_dedicated_server)
	{
		m_alife_simulator = NULL;
		m_type = eGameIDFreeMp;
		loaded_inventory = false;
		loaded_gametime = false;
	}
	else
		R_ASSERT("НЕ Возвможно запустить сервер с клиента");
}

game_sv_freemp::~game_sv_freemp()
{
	Msg("[game_sv_freemp] Destroy Server");
	loaded_inventory = false;
	loaded_gametime = false;
	delete_data(m_alife_simulator);
	ServerInventoryBoxes.clear();
}

void game_sv_freemp::Create(shared_str & options)
{
	if (!Level().ClientData_AlifeOff())
		m_alife_simulator = xr_new<CALifeSimulator>(&server(), &options);

	inherited::Create(options);
	R_ASSERT2(rpoints[0].size(), "rpoints for players not found");

	switch_Phase(GAME_PHASE_PENDING);

	::Random.seed(GetTickCount());
	m_CorpseList.clear();

	//if (Game().Type() == eGameIDFreeMp)
	LoadParamsDeffaultFMP();

}


void game_sv_freemp::AddMoneyToPlayer(game_PlayerState * ps, s32 amount)
{
	if (!ps) return;

	Msg("--- Add money to player: [%u] [%s], %d amount", ps->GameID, ps->getName(), amount);

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

void game_sv_freemp::SetMoneyToPlayer(game_PlayerState* ps, s32 amount)
{
	if (!ps) return;

	Msg("--- Set money to player: [%u] [%s], %d money", ps->GameID, ps->getName(), amount);

	if (amount < 0)
		amount = 0;

	if (amount > std::numeric_limits<s32>().max())
	{
		Msg("! The limit of the maximum amount of money has been exceeded.");
		amount = std::numeric_limits<s32>().max() - 1;
	}

	ps->money_for_round = amount;
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
		

			/* (REGISTER IN ALIFE OBJECTS MP ACTOR)	
				!!!(OFF NOT FULL WORK GOOD)
			CSE_ALifeDynamicObject* player = smart_cast<CSE_ALifeDynamicObject*>(server().ID_to_entity(ps->GameID));
			if (player && ai().get_alife())
			{
				Msg("Register Object in alife():objects(%d), GRAPH (%d) FOR SHOW ONLY (crashed if alife():register(ent) ) ", player->ID, player->m_tGraphID);
				alife().register_in_objects(player);
			}
			*/

			if (flag)
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

u32 old_update_alife = 0;
u32 InvBoxesSaveTimer = 0;

void game_sv_freemp::Update()
{
	inherited::Update();

	if (Phase() != GAME_PHASE_INPROGRESS)
	{
		OnRoundStart();
		return;
	}

	if (!Level().game)
		return;
 
	// Update Alife
	UpdateAlifeData(); // Send netwroking Packets to AlifeObjects

	 
	if (Device.dwTimeGlobal - old_update_alife > 1000)
	{
		old_update_alife = Device.dwTimeGlobal;

		for (auto cl : Game().players)
			SavePlayer(cl.second);
	} 


	if (Device.dwTimeGlobal >= InvBoxesSaveTimer)
	{
		InvBoxesSaveTimer = Device.dwTimeGlobal + 5000;
		for (const auto& SBox : ServerInventoryBoxes)
		{
			CSE_Abstract* AbstractBox = SBox.second.E;
			CSE_ALifeInventoryBox* CSEBox = smart_cast<CSE_ALifeInventoryBox*>(AbstractBox);

			if (!SBox.second.BeLoaded)
			{
				load_inventoryBox(CSEBox);
				ServerInventoryBoxes[SBox.first].BeLoaded = true;
			}
			else
			{
				if (SBox.second.NeedToSave)
				{
					save_inventoryBox(CSEBox);
				}
				else
					continue;
			}
		}
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
