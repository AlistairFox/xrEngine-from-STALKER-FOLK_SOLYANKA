#include "stdafx.h"
#include "game_cl_roleplay.h"
#include "clsid_game.h"
#include "UIGameRP.h"
#include "ui/UISpawnMenuRP.h"
#include "game_base_menu_events.h"
#include "actor_mp_client.h"

game_cl_roleplay::game_cl_roleplay()
{
	m_uTeamCount = (u8)READ_IF_EXISTS(pSettings, r_u32, "roleplay_settings", "team_count", 0);
}

game_cl_roleplay::~game_cl_roleplay()
{
}

CUIGameCustom * game_cl_roleplay::createGameUI()
{
	if (g_dedicated_server)
		return NULL;

	CLASS_ID clsid = CLSID_GAME_UI_ROLEPLAY;
	m_game_ui = smart_cast<CUIGameRP*> (NEW_INSTANCE(clsid));
	R_ASSERT(m_game_ui);
	m_game_ui->Load();
	m_game_ui->SetClGame(this);
	return m_game_ui;
}

void game_cl_roleplay::SetGameUI(CUIGameCustom *uigame)
{
	inherited::SetGameUI(uigame);
	m_game_ui = smart_cast<CUIGameRP*>(uigame);
	R_ASSERT(m_game_ui);
}

void game_cl_roleplay::OnConnected()
{
	inherited::OnConnected();
	m_game_ui = smart_cast<CUIGameRP*>(CurrentGameUI());
}

bool game_cl_roleplay::CanRespawn()
{
	CGameObject *pObject = smart_cast<CGameObject*>(Level().CurrentEntity());
	if (!pObject) return false;
	if (!local_player) return false;

	if (local_player->testFlag(GAME_PLAYER_FLAG_SPECTATOR) 
	&& local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
		return true;
 
	// If we are an actor and we are dead
	return !!smart_cast<CActor*>(pObject) && local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD);
}

void game_cl_roleplay::TryShowSpawnMenu()
{
	if (g_dedicated_server)
		return;
 
	if (!m_bTeamSelected && !m_game_ui->SpawnMenu()->IsShown())
		m_game_ui->SpawnMenu()->ShowDialog(true); 
 	
}

void game_cl_roleplay::TrySwitchJumpCaption()
{
	if (g_dedicated_server)
		return;

	if (!m_game_ui->SpawnMenu()->IsShown() && CanRespawn())
	{
		m_game_ui->SetPressJumpMsgCaption("mp_press_jump2start");
	}
	else
	{
		m_game_ui->SetPressJumpMsgCaption(nullptr);
	}
}

void game_cl_roleplay::shedule_Update(u32 dt)
{
	inherited::shedule_Update(dt);

	bool ready = local_player->testFlag(GAME_PLAYER_FLAG_READY);

	bool savefile = local_player->testFlag(GAME_PLAYER_MP_SAVETEAM);
	
	//Msg("Ready [%s], saveFile [%s]", ready ? "true" : "false", savefile ? "true" : "false");
	
	if (savefile)
		m_bTeamSelected = true;
	
	if (!m_bTeamSelected && ready && !savefile)
		TryShowSpawnMenu();

	TrySwitchJumpCaption();
}

void game_cl_roleplay::OnTeamSelect(int team)
{
	CGameObject *pObject = smart_cast<CGameObject*>(Level().CurrentEntity());
	if (!pObject) return;

	NET_Packet P;
	pObject->u_EventGen(P, GE_GAME_EVENT, pObject->ID());
	P.w_u16(GAME_EVENT_PLAYER_GAME_MENU);
	P.w_u8(PLAYER_CHANGE_TEAM);
	P.w_s16(static_cast<s16>(team));

	pObject->u_EventSend(P);
	m_bTeamSelected = true;
}

bool game_cl_roleplay::OnKeyboardPress(int key)
{
	if (kTEAM == key)
	{
		if (
			local_player->testFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS) &&
			!m_game_ui->SpawnMenu()->IsShown()
		)
		{
			m_game_ui->HideShownDialogs();
			m_game_ui->SpawnMenu()->ShowDialog(true);
		}
		return true;
	}
	else if (kJUMP == key)
	{
		if (CanRespawn())
		{
			CGameObject* GO = smart_cast<CGameObject*>(Level().CurrentControlEntity());
			NET_Packet P;
			GO->u_EventGen(P, GE_GAME_EVENT, GO->ID());
			P.w_u16(GAME_EVENT_PLAYER_READY);
			GO->u_EventSend(P);
			return true;
		}
		return false;
	}
	return inherited::OnKeyboardPress(key);
}

void game_cl_roleplay::OnSetCurrentControlEntity(CObject * O)
{

}

/*
#define PLAYER_NAME_COLOR_1 0xff40ff40
#define PLAYER_NAME_COLOR_2 0xff40ff40
#define PLAYER_NAME_COLOR_3 0xff40ff40
#define PLAYER_NAME_COLOR_4 0xff40ff40
#define PLAYER_NAME_COLOR_5 0xff40ff40
#define PLAYER_NAME_COLOR_6 0xff40ff40
#define PLAYER_NAME_COLOR_7 0xff40ff40
#define PLAYER_NAME_COLOR_8 0xff40ff40
#define PLAYER_NAME_COLOR_9 0xff40ff40

void game_cl_roleplay::OnRender()
{
	//Msg("Render");
	for (auto player : Game().players)
	{
		game_PlayerState* ps = player.second;
		u16 id = ps->GameID;
		if (ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) continue;
		//if (!ps->testFlag(GAME_PLAYER_FLAG_INVINCIBLE)) continue;

		CObject* pObject = Level().Objects.net_Find(id);
		if (!pObject) continue;
		if (!pObject || !smart_cast<CActor*>(pObject)) continue;
		if (ps == local_player) continue;
		//if (!IsEnemy(ps)) continue;
 
		VERIFY(pObject);
		CActor* pActor = smart_cast<CActor*>(pObject);
		VERIFY(pActor);
		
		Fvector IPos;
		IPos.set(0.0f, 0.5f, 0.0f);
		IPos.y -= 0.2f;
		//Msg("RenderText");

		float dup = 0.0f;
		if (ps->team == 1)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_1);
		if (ps->team == 2)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_2);
		if (ps->team == 3)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_3);
		if (ps->team == 4)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_4);
		if (ps->team == 5)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_5);
		if (ps->team == 6)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_6);
		if (ps->team == 7)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_7);
		if (ps->team == 8)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_8);
		if (ps->team == 9)
			pActor->RenderText(ps->getName(), IPos, &dup, PLAYER_NAME_COLOR_9);
	}

	inherited::OnRender();
}
*/