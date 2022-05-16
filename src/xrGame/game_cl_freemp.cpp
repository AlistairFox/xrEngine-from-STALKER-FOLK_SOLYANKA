#include "stdafx.h"
#include "game_cl_freemp.h"
#include "clsid_game.h"
#include "xr_level_controller.h"
#include "UIGameFMP.h"
#include "actor_mp_client.h"

#include "VoiceChat.h"
#include "ui/UIMainIngameWnd.h"

game_cl_freemp::game_cl_freemp()
{
	LPCSTR sec_name = "freemp_team_indicator";
	Indicator_render1 = pSettings->r_float(sec_name, "indicator_1");
	Indicator_render2 = pSettings->r_float(sec_name, "indicator_2");

	IndicatorPosition.x = pSettings->r_float(sec_name, "indicator_x");
	IndicatorPosition.y = pSettings->r_float(sec_name, "indicator_y");
	IndicatorPosition.z = pSettings->r_float(sec_name, "indicator_z");

	IndicatorPositionText.y = pSettings->r_float(sec_name, "indicator_y_text");


	LPCSTR		ShaderType = pSettings->r_string(sec_name, "indicator_shader");
	LPCSTR		ShaderTexture = pSettings->r_string(sec_name, "indicator_texture");
	IndicatorShaderFreemp->create(ShaderType, ShaderTexture);

	LPCSTR		ShaderTypeLeader = pSettings->r_string(sec_name, "leader_shader");
	LPCSTR		ShaderTextureLeader = pSettings->r_string(sec_name, "leader_texture");
	IndicatorShaderFreempLeader->create(ShaderTypeLeader, ShaderTextureLeader);

	if (!g_dedicated_server)
		m_pVoiceChat = xr_new<CVoiceChat>();
	else
		m_pVoiceChat = NULL;

	load_game_tasks = false;

}

game_cl_freemp::~game_cl_freemp()
{
	xr_delete(m_pVoiceChat);
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


	if (m_pVoiceChat)
	{
		m_game_ui->UIMainIngameWnd->SetVoiceDistance(m_pVoiceChat->GetDistance());
	}
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

#include "GametaskManager.h"
#include "GameTask.h"

void game_cl_freemp::shedule_Update(u32 dt)
{
	game_cl_GameState::shedule_Update(dt);

	if (!local_player)
		return;

	if (!g_dedicated_server && m_pVoiceChat)
	{
		const bool started = m_pVoiceChat->IsStarted();
		const bool is_dead = !local_player || local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD);
		const bool has_shown_dialogs = CurrentGameUI()->HasShownDialogs();
		if (started && (is_dead || has_shown_dialogs))
		{
			m_pVoiceChat->Stop();
			CurrentGameUI()->UIMainIngameWnd->SetActiveVoiceIcon(false);
		}
		m_pVoiceChat->Update();
	}

	if (OnClient() && Device.dwTimeGlobal - old_time > 5000 && load_game_tasks)
	{
		string_path filename;
		FS.update_path(filename, "$mp_saves$", "task\\tasks.ltx");

		CInifile* file = xr_new<CInifile>("filename", false, false, false);

		for (auto task : Level().GameTaskManager().GetGameTasks())
		{
			task.save_ltx(*file, task.task_id);
		}

		file->save_as(filename);
	}
	else if (!load_game_tasks && Actor() && Device.dwTimeGlobal - old_time > 5000)
	{
		string_path filename;
		FS.update_path(filename, "$mp_saves$", "task\\tasks.ltx");

		CInifile* file = xr_new<CInifile>(filename, true, true);

		for (auto sec : file->sections())
		{
			CGameTask* task = xr_new<CGameTask>();
			task->load_task_ltx(*file, sec->Name);
			task->m_ID = sec->Name.c_str();

			Level().GameTaskManager().LoadGameTask(task);
		}						  
		load_game_tasks = true;
	}
	
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

		//Msg("Rank %d, Current %d", ps->rank, pActor->Rank());

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

			//	Msg("Slot[%s][%s][%s][%s]", Slot1, Slot2, Slot3, Slot4);

				xr_strcpy(g_quick_use_slots[0], Slot1);
				xr_strcpy(g_quick_use_slots[1], Slot2);
				xr_strcpy(g_quick_use_slots[2], Slot3);
				xr_strcpy(g_quick_use_slots[3], Slot4);

				connected_spawn = true;
			}
		}
	}

	if (local_player->GameID)
	if (OnClient() && Device.dwTimeGlobal - old_time > 5000 && connected_spawn)
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
	switch (key)
	{
		case kVOICE_CHAT:
		{
			if (local_player && !local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
			{
				if (!m_pVoiceChat->IsStarted())
				{
					m_pVoiceChat->Start();
					CurrentGameUI()->UIMainIngameWnd->SetActiveVoiceIcon(true);
				}
			}
			return true;
		}break;

		case kVOICE_DISTANCE:
		{
			if (local_player && !local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
			{
				u8 distance = m_pVoiceChat->SwitchDistance();
				CurrentGameUI()->UIMainIngameWnd->SetVoiceDistance(distance);
			}
			return true;
		}break;

		case kJUMP:
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
		}break;

		default:
			break;
	}

	return inherited::OnKeyboardPress(key);

}
 

bool game_cl_freemp::OnKeyboardRelease(int key)
{
	switch (key)
	{
	case kVOICE_CHAT:
	{
		m_pVoiceChat->Stop();
		CurrentGameUI()->UIMainIngameWnd->SetActiveVoiceIcon(false);
		return true;
	}break;

	default:
		break;
	}

	return inherited::OnKeyboardRelease(key);
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

	if (m_game_ui)
		Game().OnScreenResolutionChanged();
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

#include "ui/UIPdaWnd.h"
#include "UIPda_Contacts.h"
#include "UIPda_Chat.h"

void game_cl_freemp::TranslateGameMessage(u32 msg, NET_Packet& P)
{
	switch (msg)
	{
		case (GAME_EVENT_UI_PDA):
		{
 			m_game_ui->PdaMenu().pUIContacts->EventRecive(P);
		}break;

		case (GAME_EVENT_PDA_CHAT):
		{
			m_game_ui->PdaMenu().pUIChatWnd->RecivePacket(P);
		}break;

		case (M_ALIFE_OBJECTS_SPAWN):
		{
			ReadSpawnAlife(&P);
		}break;

		case (M_ALIFE_OBJECTS_UPDATE):
		{
			ReadUpdateAlife(&P);
		}break;

		default:
			inherited::TranslateGameMessage(msg, P);
			break;
	}

}

#include "UIPda_Squad.h";

extern bool caps_lock = false;

void game_cl_freemp::OnRender()
{
	inherited::OnRender();
	
	if (m_pVoiceChat)
		m_pVoiceChat->OnRender();

	Team teamPL = m_game_ui->PdaMenu().pUIContacts->squad_UI->team_players;

	if (teamPL.cur_players > 0)
	for (auto pl : teamPL.players)
	{
		if (pl == 0)
			continue;

		game_PlayerState* ps = GetPlayerByGameID(GetPlayerByClientID(pl));

		if (!ps)
			continue;

		if (ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) continue;
		if (ps == local_player) continue;

		CActor* pActor = smart_cast<CActor*>(Level().Objects.net_Find(ps->GameID));
		if (!pActor) continue;

		float pos = 0.0f;
	
		Fvector posH = IndicatorPosition;
 		pActor->RenderIndicatorNew(posH, Indicator_render1, Indicator_render2, IndicatorShaderFreemp);
			
	}


	if (caps_lock && local_player->testFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS))
	{
		for (auto pl : players)
		{
			game_PlayerState* ps = GetPlayerByGameID(pl.second->GameID);

			if (!ps)
				continue;

			//if (ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) continue;
			if (ps == local_player) continue;

			CActor* pActor = smart_cast<CActor*>(Level().Objects.net_Find(ps->GameID));
			if (!pActor) continue;
			if (!pActor->cast_actor_mp()) continue;

			float pos = 0.0f;
			Fvector posPOINT = IndicatorPositionText;
			posPOINT.y -= 0.5f;

			string256 str = { 0 };
			xr_strcpy(str, pActor->Name());
			xr_strcat(str, "(");
			xr_strcat(str, ps->m_account.name_login().c_str());
			xr_strcat(str, ")");

			pActor->RenderText(str, posPOINT, &pos, color_rgba(255, 255, 0, 255));
		}

	}
 
}

void game_cl_freemp::ReadSpawnAlife(NET_Packet* packet)
{
	if (OnServer())
		return;

	shared_str name;
	packet->r_stringZ(name);

	//Msg("AlifeOBJECT: %s", name.c_str());

	if (!pSettings->section_exist(name.c_str()))
	{
		Msg("Cant Find Sec(%s)", name.c_str());
		return;
	}
	 
	CSE_Abstract* entity = F_entity_Create(name.c_str());
	entity->Spawn_ReadNoBeginPacket(*packet);
	entity->UPDATE_Read(*packet);

	u16 id = entity->ID;
 
	CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(entity);

	if (dynamic)
	{
		u32 level = 0;
		shared_str name_level ;

		if (&ai().game_graph())
		{
			level = ai().game_graph().vertex(dynamic->m_tGraphID)->level_id();
			name_level = ai().game_graph().header().level(level).name();
		}
 	
		//Msg("Recived id[%d], level[%d][%s], alife_object[%s], name_replace (%s), POS[%.0f][%.0f][%.0f]", id, level, name_level, name.c_str(),  entity->name_replace(), entity->o_Position.x, entity->o_Position.y, entity->o_Position.z);

		if ((*alife_objects.find(id)).second != dynamic)
		{
			alife_objects[id] = dynamic;
			CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(dynamic);
			CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(dynamic);
			CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(dynamic);

			if (stalker || changer || monster)
			{
				//Msg("Name %s, spawn_name %s", dynamic->name_replace(), dynamic->s_name.c_str());
				dynamic->on_register_client();
			}
		}
		
	}
	
		
}

void game_cl_freemp::ReadUpdateAlife(NET_Packet* packet)
{
	u16 id = packet->r_u16();

	if (alife_objects[id])
		alife_objects[id]->UPDATE_Read(*packet);
}

void game_cl_freemp::OnScreenResolutionChanged()
{
	if (m_game_ui && m_pVoiceChat)
	{
		m_game_ui->UIMainIngameWnd->SetVoiceDistance(m_pVoiceChat->GetDistance());
	}
}

void game_cl_freemp::OnVoiceMessage(NET_Packet* P)
{
	m_pVoiceChat->ReceiveMessage(P);
}
 