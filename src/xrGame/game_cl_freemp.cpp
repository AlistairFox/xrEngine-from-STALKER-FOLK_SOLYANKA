#include "stdafx.h"
#include "game_cl_freemp.h"
#include "clsid_game.h"
#include "xr_level_controller.h"
#include "UIGameFMP.h"
#include "actor_mp_client.h"

game_cl_freemp::game_cl_freemp()
{
}

game_cl_freemp::~game_cl_freemp()
{
}

bool connected_spawn = false;
extern bool just_Connected;

CUIGameCustom* game_cl_freemp::createGameUI()
{
	if (g_dedicated_server)
		return NULL;

	CLASS_ID clsid = CLSID_GAME_UI_FREEMP;
	m_game_ui = smart_cast<CUIGameFMP*> (NEW_INSTANCE(clsid));
	R_ASSERT(m_game_ui);
	m_game_ui->Load();
	m_game_ui->SetClGame(this);
	return					m_game_ui;
}

void game_cl_freemp::SetGameUI(CUIGameCustom* uigame)
{
	inherited::SetGameUI(uigame);
	m_game_ui = smart_cast<CUIGameFMP*>(uigame);
	R_ASSERT(m_game_ui);
}


void game_cl_freemp::net_import_state(NET_Packet & P)
{
	inherited::net_import_state(P);
}

void game_cl_freemp::net_import_update(NET_Packet & P)
{
	inherited::net_import_update(P);
}
u32 old_time = 0;

void game_cl_freemp::shedule_Update(u32 dt)
{
	game_cl_GameState::shedule_Update(dt);

	if (!local_player)
		return;
	
	// синхронизация имени и денег игроков для InventoryOwner
	for (auto cl : players)
	{
		game_PlayerState* ps = cl.second;
		if (!ps || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) continue;

		CActor* pActor = smart_cast<CActor*>(Level().Objects.net_Find(ps->GameID));
		if (!pActor || !pActor->g_Alive()) continue;

		pActor->SetName(ps->getName());
		pActor->cName_set(ps->getName());

		if (ps->team != pActor->Community())
		{
			CHARACTER_COMMUNITY	community;
			community.set(ps->team);
			pActor->SetCommunity(community.index());
			pActor->ChangeTeam(community.team(), 0, 0);
		}

		if (local_player->GameID == ps->GameID)
		{
			pActor->set_money((u32)ps->money_for_round, false);
		}

		if (!connected_spawn && local_player->GameID == ps->GameID)
		{
			string_path filepath;

			FS.update_path(filepath, "$mp_saves$", "actor.ltx");

			CInifile* file = xr_new<CInifile>(filepath, true);

			if (file && file->section_exist("QuickSlots"))
			{
				string64 Slot1, Slot2, Slot3, Slot4;
				xr_strcpy(Slot1, file->r_string("QuickSlots", "Slot1"));
				xr_strcpy(Slot2, file->r_string("QuickSlots", "Slot2"));
				xr_strcpy(Slot3, file->r_string("QuickSlots", "Slot3"));
				xr_strcpy(Slot4, file->r_string("QuickSlots", "Slot4"));

				Msg("Slot[%s][%s][%s][%s]", Slot1, Slot2, Slot3, Slot4);

				xr_strcpy(g_quick_use_slots[0], Slot1);
				xr_strcpy(g_quick_use_slots[1], Slot2);
				xr_strcpy(g_quick_use_slots[2], Slot3);
				xr_strcpy(g_quick_use_slots[3], Slot4);

				connected_spawn = true;
			}
		}
	}

	if (local_player->GameID)
	

	if (OnClient() && Device.dwTimeGlobal - old_time > 1000 && connected_spawn)
	{
		string_path filepath;

		FS.update_path(filepath, "$mp_saves$", "actor.ltx");

		CInifile* file = xr_new<CInifile>(filepath, false);

		if (file)
		{
			file->w_string("QuickSlots", "Slot1", g_quick_use_slots[0]);
			file->w_string("QuickSlots", "Slot2", g_quick_use_slots[1]);
			file->w_string("QuickSlots", "Slot3", g_quick_use_slots[2]);
			file->w_string("QuickSlots", "Slot4", g_quick_use_slots[3]);
		}

		file->save_as(filepath);

		old_time = Device.dwTimeGlobal;
	}

	if (OnServer() && Device.dwTimeGlobal - old_time > 1000)
	{
		for (auto cl : players)
			save_player(cl.second);

		old_time = Device.dwTimeGlobal;
	}

}

bool game_cl_freemp::OnKeyboardPress(int key)
{
	if (kJUMP == key)
	{
		bool b_need_to_send_ready = false;

		CObject* curr = Level().CurrentControlEntity();
		if (!curr) return(false);

		bool is_actor = !!smart_cast<CActor*>(curr);
		bool is_spectator = !!smart_cast<CSpectator*>(curr);

		game_PlayerState* ps = local_player;
				
		if (is_spectator || (is_actor && ps && ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)))
		{
			b_need_to_send_ready = true;
		}

		if (b_need_to_send_ready)
		{
			CGameObject* GO = smart_cast<CGameObject*>(curr);
			NET_Packet			P;
			GO->u_EventGen(P, GE_GAME_EVENT, GO->ID());
			P.w_u16(GAME_EVENT_PLAYER_READY);
			GO->u_EventSend(P);
			return				true;
		}
		else
		{
			return false;
		}
	};

	return inherited::OnKeyboardPress(key);
}

LPCSTR game_cl_freemp::GetGameScore(string32&	score_dest)
{
	s32 frags = local_player ? local_player->frags() : 0;
	xr_sprintf(score_dest, "[%d]", frags);
	return score_dest;
}

void game_cl_freemp::OnConnected()
{
	inherited::OnConnected();
	
	if (m_game_ui)
	{
		R_ASSERT(!g_dedicated_server);
		m_game_ui = smart_cast<CUIGameFMP*>	(CurrentGameUI());
		m_game_ui->SetClGame(this);
	}
	 
	luabind::functor<void>	funct;
	R_ASSERT(ai().script_engine().functor("mp_game_cl.on_connected", funct));
	funct();

	just_Connected = true;
	
}

bool game_cl_freemp::OnConnectedSpawnPlayer()
{
	bool b_need_to_send_ready = false;

	CObject* curr = Level().CurrentControlEntity();

	if (!curr) 
		return false;

	bool is_actor = !!smart_cast<CActor*>(curr);
	bool is_spectator = !!smart_cast<CSpectator*>(curr);

	game_PlayerState* ps = local_player;

	if (is_spectator || (is_actor && ps && ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)))
	{
		b_need_to_send_ready = true;
	}

	if (b_need_to_send_ready)
	{
		CGameObject* GO = smart_cast<CGameObject*>(curr);
		NET_Packet			P;
		GO->u_EventGen(P, GE_GAME_EVENT, GO->ID());
		P.w_u16(GAME_EVENT_PLAYER_READY);
		GO->u_EventSend(P);

		return true;
 	}
}
