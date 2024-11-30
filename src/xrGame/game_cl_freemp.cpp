#include "stdafx.h"
#include "game_cl_freemp.h"
#include "clsid_game.h"
#include "xr_level_controller.h"
#include "UIGameFMP.h"
#include "actor_mp_client.h"

#include "VoiceChat.h"
#include "ui/UIMainIngameWnd.h"

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

	m_SquadLeaderShader->create("hud\\default", "ui\\ui_squad_leader");
	m_SquadMemberShader->create("hud\\default", "ui\\ui_squad_member");

	local_squad = xr_new<MP_SquadCL>();
	local_squad->need_update = false;

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

void game_cl_freemp::shedule_Update(u32 dt)
{
	game_cl_GameState::shedule_Update(dt);

	if (!local_player)
		return;

	// Update Войса
	shedule_voice();
	// синхронизация имени и денег игроков для InventoryOwner
	shedule_InventoryOwner();
	// Shedule Save QUESTS
	shedule_Quests();

}

void game_cl_freemp::shedule_voice()
{
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
}

void game_cl_freemp::shedule_InventoryOwner()
{
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

		if (ps->rank != pActor->Rank())
		{
			pActor->SetRankClient(ps->rank);
		}
	}
}

#include "GameTask.h"
#include "GametaskManager.h"

u32 old_task_save_update = 0;

void game_cl_freemp::shedule_Quests()
{	   
	if (old_task_save_update < Device.dwTimeGlobal)
		return;

	old_task_save_update = Device.dwTimeGlobal + 1000;
   
	json_ex.reset();

	for (auto task : Level().GameTaskManager().GetGameTasks())
		this->save_task(task.game_task);  

	json_ex.save("tasks.save", "$mp_client_player$");

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
		/*
		case kVOICE_DISTANCE:
		{
			if (local_player && !local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
			{
				u8 distance = m_pVoiceChat->SwitchDistance();
				CurrentGameUI()->UIMainIngameWnd->SetVoiceDistance(distance);
			}
			return true;
		}break;
		*/
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


void game_cl_freemp::GiveNews(LPCSTR caption, LPCSTR text, LPCSTR texture_name, int delay, int show_time, int type = 0)
{
	if (!Actor()) return;

	GAME_NEWS_DATA				news_data;
	news_data.m_type = (GAME_NEWS_DATA::eNewsType)type;
	news_data.news_caption = caption;
	news_data.news_text = text;
	if (show_time != 0)
		news_data.show_time = show_time;

	VERIFY(xr_strlen(texture_name) > 0);

	news_data.texture_name = texture_name;

	if (delay == 0)
	{
		Actor()->AddGameNews(news_data);
		m_sndPDAmsg.play(NULL, sm_2D);
	}
	else
	{
		Actor()->AddGameNews_deffered(news_data, delay);
		m_sndPDAmsg.play(NULL, sm_2D, (float)delay);
	}
}
#define SQUAD_MEMBER_LOCATION	"friend_location"
#define SQUAD_LEADER_LOCATION	"alife_presentation_squad_friend"
void game_cl_freemp::GiveNews(LPCSTR caption, LPCSTR text, LPCSTR texture_name, int delay, int show_time, int type, bool noSound)
{
	if (!Actor()) return;
	Msg("Give News For Actor: %s, cap: %s, text: %s, texture: %s", Actor()->Name(), caption, text, texture_name);
	GAME_NEWS_DATA				news_data;
	news_data.m_type = (GAME_NEWS_DATA::eNewsType)type;
	news_data.news_caption = caption;
	news_data.news_text = text;
	if (show_time != 0)
		news_data.show_time = show_time;

	VERIFY(xr_strlen(texture_name) > 0);

	news_data.texture_name = texture_name;


	if (delay == 0)
	{
		Actor()->AddGameNews(news_data);
		if (!noSound)
			m_sndPDAmsg.play(NULL, sm_2D);
	}
	else
	{
		Actor()->AddGameNews_deffered(news_data, delay);
		if (!noSound)
			m_sndPDAmsg.play(NULL, sm_2D, (float)delay);
	}
}

#include "map_manager.h"
	
void game_cl_freemp::TranslateGameMessage(u32 msg, NET_Packet& P)
{
	switch (msg)
	{
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

		case M_INVENTORY_OWNER_RANK:
		{
			u32 id, rank;
			P.r_u32(id);
			P.r_u32(rank);

			CObject* o = Level().Objects.net_Find(id);
			CInventoryOwner* owner = smart_cast<CInventoryOwner*>(o);
			if (owner)
			{
				owner->SetRankClient(rank);
			}

		}break;


		/// xrMPE PDA events

		case GE_PDA_SQUAD_RESPOND_INVITE:
		{
			Msg("Recive GE_PDA_SQUAD_RESPOND_INVITE");
			u8 capacity;
			u16 id;
			P.r_clientID(local_squad->squad_leader_cid); //squad leader id
			P.r_u16(local_squad->id); //squad id
			P.r_u16(local_squad->current_map_point); //current map point
			P.r_stringZ(local_squad->current_quest); //current quest
			P.r_u8(capacity);
			shared_str message;
			P.r_stringZ(message);

			if (xr_strlen(message))
 				GiveNews("Отряд", message.c_str(), "ui_inGame2_PD_Lider", 0, 4000, 0, false);
 
 			if (local_squad->players.size())
				local_squad->players.clear(); //clear all playerstates because we refill this list
			
			if (m_pVoiceChat->players_in_squad.size())
				m_pVoiceChat->players_in_squad.clear();

			for (u32 o_it = 0; o_it < capacity; o_it++) 
			{
				id = P.r_u16();
  				auto PlayerState = GetPlayerByGameID(id);
			

				// if (PlayerState == nullptr)
				// {
				// 	CVoiceChat::VoicePlayer player_data;
				// 	player_data.name = PlayerState->getName();
				// 	player_data.distance = 0;
				// 	player_data.PlayerState = PlayerState;
				// 	player_data.SquadID = PlayerState->MPSquadID;
				// 	m_pVoiceChat->players_in_squad.push_back(player_data);
				// }
				// else
				// {
				// 	Msg("--- PlayerID: %d dont Has PlayerState is Strage !!!", id);
				// }

				local_squad->players.push_back(PlayerState);
			};

			m_game_ui->UpdateHudSquad();
			local_squad->need_update = true;

		}break;

		case GE_PDA_SQUAD_KICK_PLAYER:
		{
			Msg("Recive GE_PDA_SQUAD_KICK_PLAYER");

			local_squad->id = 0;
			local_squad->squad_leader_cid = 0;
			local_squad->current_map_point = 0;
			local_squad->current_quest = 0;

			for (u32 o_it = 0; o_it < local_squad->players.size(); o_it++)
			{
				if (Level().MapManager().HasMapLocation(SQUAD_MEMBER_LOCATION, local_squad->players[o_it]->GameID))
					Level().MapManager().RemoveMapLocation(SQUAD_MEMBER_LOCATION, local_squad->players[o_it]->GameID);
				else if (Level().MapManager().HasMapLocation(SQUAD_LEADER_LOCATION, local_squad->players[o_it]->GameID))
					Level().MapManager().RemoveMapLocation(SQUAD_LEADER_LOCATION, local_squad->players[o_it]->GameID);
			}

			if (local_squad->players.size())
				local_squad->players.clear();

			local_squad->need_update = true;
			shared_str message;
			P.r_stringZ(message);

			if (xr_strlen(message))
				GiveNews("Отряд", message.c_str(), "ui_inGame2_PD_Lider", 0, 4000, 0, false);
 		 
			m_game_ui->UpdateHudSquad();

			//ChatDestination update
		//	if (m_game_ui->m_pMessagesWnd && m_game_ui->m_pMessagesWnd->GetChatWnd())
		//		m_game_ui->m_pMessagesWnd->GetChatWnd()->OnSquadLeave();
		}break;
		case GE_PDA_SQUAD_MAKE_LEADER:
		{
			Msg("Recive GE_PDA_SQUAD_MAKE_LEADER");
			shared_str message;
			P.r_stringZ(message);

			if (xr_strlen(message))
				GiveNews("Отряд", message.c_str(), "ui_inGame2_PD_Lider", 0, 4000, 0, false);
		}break;
		 
		case GE_PDA_SQUAD_SEND_INVITE:
		{
 			OnSquadInvniteReceived(P);
		}break;

		case GE_PDA_SQUAD_CANCEL_INVITE:
		{
			OnSquadCancelReceived(P);
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
 
	//if (m_bSquadIndicators)
	if (local_squad)
	{
		for (u32 o_it = 0; o_it < local_squad->players.size(); o_it++)
		{
			CObject* pObject = Level().Objects.net_Find(local_squad->players[o_it]->GameID);
			if (!pObject) continue;

			bool isFinded = false;
			for (auto PL : Game().players)
			{
				if (!isFinded)
				{
					isFinded = PL.second->GameID == local_squad->players[o_it]->GameID;
					break;
				}
			}

			if (!isFinded)
				continue;

			CActor* pActor = smart_cast<CActor*>(pObject);
 			if (!pActor) 
				continue;
 			if (!pActor->g_Alive())
				continue;
 			if (!local_squad->players[o_it]) 
				continue;
 			if (local_player == local_squad->players[o_it])
				continue;

 			if (players[local_squad->squad_leader_cid]->GameID == local_squad->players[o_it]->GameID)
				pActor->RenderSquadIndicator(local_squad->players[o_it]->getName(), color_argb(225, 255, 241, 150), Fvector().set(0.0f, 0.35f, 0.0f), 32.0f, 32.0f, m_SquadLeaderShader);
			else
				pActor->RenderSquadIndicator(local_squad->players[o_it]->getName(), color_argb(225, 255, 241, 150), Fvector().set(0.0f, 0.35f, 0.0f), 32.0f, 32.0f, m_SquadMemberShader);
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


game_cl_freemp::MP_SquadInvite* game_cl_freemp::FindInviteByInviterID(u16 ID)
{
	xr_vector<MP_SquadInvite*>::const_iterator it = mp_squad_invites.begin();
	xr_vector<MP_SquadInvite*>::const_iterator it_e = mp_squad_invites.end();

	for (; it != it_e; ++it)
	{
		if ((*it)->InviterID == ID)
			return (*it);
	}
	return 0;
}

void game_cl_freemp::RemoveInviteByInviterID(u16 ID)
{
	xr_vector<MP_SquadInvite*>::const_iterator it = mp_squad_invites.begin();
	xr_vector<MP_SquadInvite*>::const_iterator it_e = mp_squad_invites.end();

	for (; it != it_e; ++it)
	{
		if ((*it)->InviterID == ID)
			mp_squad_invites.erase(it);
	}

	return;
}


#include "string_table.h"
#define MAX_INVITE_SIZE 30
void game_cl_freemp::OnSquadInvniteReceived(NET_Packet& P)
{
	u16 SenderGameID = P.r_u16();
	game_PlayerState* ps = Game().GetPlayerByGameID(SenderGameID);
	if (!ps) return;

 	u32 size = mp_squad_invites.size();
	if (size >= MAX_INVITE_SIZE)
		return;

#pragma todo("We must create CMPGameTask instance in game_cl_mp class")

	string128 InviteMessage;
	LPCSTR PlayerName;
	PlayerName = ps->getName();

	xr_sprintf(InviteMessage, "%s %s", PlayerName, CStringTable().translate("mp_squad_invite").c_str()); //приглашает Вас в отряд

	// tasks->GiveNews(CStringTable().translate("mp_squad_title").c_str(), InviteMessage, "ui_inGame2_PD_Lider", 0, 4000, 0, false);

	MP_SquadInvite* invite = xr_new<MP_SquadInvite>();
	invite->InviteMessage = InviteMessage;
	invite->InviterID = SenderGameID;
	invite->ReceivedTime = Level().timeServer();

	mp_squad_invites.push_back(invite);

	m_bSwitchToNextInvite = true;
}

void game_cl_freemp::OnSquadCancelReceived(NET_Packet& P)
{
	u16 SenderGameID = P.r_u16();
	game_PlayerState* ps = Game().GetPlayerByGameID(SenderGameID);
	if (!ps) return;

	MP_SquadInvite* invite = FindInviteByInviterID(SenderGameID);
	if (!invite)
		return;

	RemoveInviteByInviterID(SenderGameID);
	m_bSwitchToNextInvite = true;

	string128 InviteMessage;
	LPCSTR PlayerName;
	PlayerName = ps->getName();
	xr_sprintf(InviteMessage, "%s %s %s", CStringTable().translate("mp_squad_cancel_0").c_str(), PlayerName, CStringTable().translate("mp_squad_cancel_1").c_str()); //"Приглашение от игрока %s отменено, либо истекло"

	// tasks->GiveNews(CStringTable().translate("mp_squad_title").c_str(), InviteMessage, "ui_inGame2_PD_Lider", 0, 4000, 0, false);
}