#include "stdafx.h"
#include "game_cl_freemp.h"
#include "clsid_game.h"
#include "xr_level_controller.h"
#include "UIGameFMP.h"
#include "actor_mp_client.h"

#include "VoiceChat.h"
#include "ui/UIMainIngameWnd.h"

#include "GametaskManager.h"
#include "GameTask.h"

#include "ui/UIPdaWnd.h"
#include "UIPda_Contacts.h"
#include "UIPda_Chat.h"	  
#include "UIPda_Squad.h";		//#include "game_sv_freemp.h" (здесь тоже используется)
	
//NEWS
#include "game_news.h"
#include "ui/UIMessagesWindow.h"
#include "ui/UITalkWnd.h"
 
game_cl_freemp::game_cl_freemp()
{
	l_events = xr_new<level_events>();


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

	alife_objects_synchronized = false;
}

game_cl_freemp::~game_cl_freemp()
{
	xr_delete(l_events);
	xr_delete(m_pVoiceChat);
	alife_objects_synchronized = false;		 
 	
	for (auto obj : alife_objects)
	{
 		alife_objects[obj.first] = 0;
	}

	Level().event_functors.clear_and_free();
}

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
//sync slots
string32 slots[4];

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

	/*
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
	else if (!load_game_tasks && Actor() && Actor()->Setuped_callbacks() && Device.dwTimeGlobal - old_time > 5000)
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
			Msg("LoadTask: %s", task->m_ID.c_str());
		}


		load_game_tasks = true;
	}	*/
	
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
	}	

	if (slots[0] != g_quick_use_slots[0] ||
		slots[1] != g_quick_use_slots[1] ||
		slots[2] != g_quick_use_slots[2] ||
		slots[3] != g_quick_use_slots[3]   )
	{
		xr_strcpy(slots[0], g_quick_use_slots[0]);
		xr_strcpy(slots[1], g_quick_use_slots[1]);	
		xr_strcpy(slots[2], g_quick_use_slots[2]);
		xr_strcpy(slots[3], g_quick_use_slots[3]);

		NET_Packet packet;
		Game().u_EventGen(packet, GE_ACTOR_SLOTS_SYNC, -1);
		packet.w_clientID(Level().game->local_svdpnid);
		packet.w_stringZ(slots[0]);
		packet.w_stringZ(slots[1]);
		packet.w_stringZ(slots[2]);
		packet.w_stringZ(slots[3]);
		Game().u_EventSend(packet);

	}


	
}

float game_cl_freemp::shedule_Scale()
{
	return 1.0f;
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

	if (m_game_ui)
		Game().OnScreenResolutionChanged();


}
	
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

		case (GAME_EVENT_NEWS_MESSAGE):
		{
			shared_str name, text, icon;
			P.r_stringZ(name);
			P.r_stringZ(text);
			P.r_stringZ(icon);
			GAME_NEWS_DATA data;
			data.m_type = data.eNews;
			data.news_caption = name;
			data.news_text = text;
			data.texture_name = icon;
			data.receive_time = Level().GetGameTime();
	
			if (CurrentGameUI())
			{
				bool talk = CurrentGameUI()->TalkMenu && CurrentGameUI()->TalkMenu->IsShown();
					
				if (CurrentGameUI()->UIMainIngameWnd && !talk)
					CurrentGameUI()->m_pMessagesWnd->AddIconedPdaMessage(&data);
				else 
					if (talk)
						CurrentGameUI()->TalkMenu->AddIconedMessage(name.c_str(), text.c_str(), icon.c_str(), "iconed_answer_item");
			}
			
		}break;

	 

		default:
			inherited::TranslateGameMessage(msg, P);
			break;
	}

}


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

void game_cl_freemp::OnScreenResolutionChanged()
{
	if (m_game_ui && m_pVoiceChat)
	{
		m_game_ui->UIMainIngameWnd->SetVoiceDistance(m_pVoiceChat->GetDistance());
	}
}

void game_cl_freemp::CreateParticle(LPCSTR name, Fvector3 pos)
{
	pobjec = CParticlesObject::Create(name);
	pobjec->play_at_pos(pos);
}

void game_cl_freemp::OnVoiceMessage(NET_Packet* P)
{
	m_pVoiceChat->ReceiveMessage(P);
}
 