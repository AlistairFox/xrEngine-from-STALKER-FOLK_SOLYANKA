#include "stdafx.h"
#include "UIGameFMP.h"
#include "game_cl_freemp.h"
#include "../xrEngine/xr_input.h"

#include "Actor.h"
#include "level.h"
#include "xr_level_controller.h"

#include "ui/UIStatic.h"
#include "ui/UIXmlInit.h"
#include "UIAnimMode.h"
#include "ui/UIHelper.h"

BOOL g_cl_draw_mp_statistic = FALSE;

void CUIGameFMP::UpdateHudSquad()
{
	m_hud_squad->UpdateMembers();
}

CUIGameFMP::CUIGameFMP()
{
	m_game = NULL;
	m_animation = NULL;
	m_hud_squad = nullptr;
}

CUIGameFMP::~CUIGameFMP()
{
	xr_delete(m_animation);
	xr_delete(m_hud_squad);
	
}

u32 oldTimer;

void CUIGameFMP::Init(int stage)
{
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "ui_game_fmp.xml");

	if (stage == 0)
	{
		//shared
		m_stats = xr_new<CUITextWnd>();
		m_stats->SetAutoDelete(true);
 
		inherited::Init(stage);

		CUIXmlInit::InitWindow(uiXml, "global", 0, m_window);
		CUIXmlInit::InitTextWnd(uiXml, "stats", 0, m_stats);

		//surge_background = UIHelper::CreateStatic(uiXml, "surge", 0);
		//surge_cap = UIHelper::CreateTextWnd(uiXml, "surge_cap", 0);

		m_hud_squad = xr_new<CUIHudSquadWnd>();
		m_hud_squad->Init();
		m_hud_squad->SetAutoDelete(false);
		 
	}
	else if (stage == 1)
	{
		//unique

	}
	else if (stage == 2)
	{
		//after
		inherited::Init(stage);
		m_window->AttachChild(m_stats);
	}
	m_animation = xr_new<CUIAMode>();
	m_animation->Init();

	m_window->AttachChild(m_animation);
	m_window->AttachChild(m_hud_squad);
}

void CUIGameFMP::SetClGame(game_cl_GameState * g)
{
	inherited::SetClGame(g);
	m_game = smart_cast<game_cl_freemp*>(g);
	R_ASSERT(m_game);
}

void CUIGameFMP::HideShownDialogs()
{
	inherited::HideShownDialogs();
}

#include "ui/UIInventoryUtilities.h"
#include "Actor.h"
#include "Inventory.h"
#include "UI_UpgradesQuick.h"
#include "game_cl_freemp.h"

void _BCL CUIGameFMP::OnFrame()
{
	OPTICK_EVENT("CUIGameFMP::OnFrame");


	inherited::OnFrame();
 
	game_cl_freemp* fmp = smart_cast<game_cl_freemp*>(Level().game);

	if (g_cl_draw_mp_statistic && Level().game->local_player)
	{
		IClientStatistic& stats = Level().GetStatistic();

		string1024 outstr;
		if (UseDirectPlay())
		{
			xr_sprintf(
				outstr,
				"ping: %u/%u\\n"
				"in/out: %.1f/%.2f KB/s\\n"
				"packets dropped: %u\\n"
				"packets retried: %u\\n",
				Level().game->local_player->ping,
				stats.getPing(),
				stats.getReceivedPerSec() / 1000.0f,
				stats.getSendedPerSec() / 1000.0f,
				stats.getDroppedCount(),
				stats.getRetriedCount()
			);
		}
		else 
		{
			xr_sprintf(
				outstr,
				"FPS: %.0f \\n"
				"ping: %u/%u\\n"
				"in/out: %.1f/%.2f KB/s\\n"
				"packets in/out: %.0f/%.0f\\n"
				"quality local: %.2f\\n"
				"quality remote: %.2f\\n",
				Device.Statistic->fFPS,
 				Level().game->local_player->ping,
				stats.getPing(),
				stats.getReceivedPerSec() / 1000.0f,
				stats.getSendedPerSec() / 1000.0f,
				stats.getPacketsInPerSec(),
				stats.getPacketsOutPerSec(),
				stats.getQualityLocal(),
				stats.getQualityRemote()
			);
		}

		m_stats->SetTextST(outstr);
		m_stats->Enable(true);
	}
	else 
	if (m_stats->IsEnabled())
	{
		m_stats->SetTextST("");
		m_stats->Enable(false);
	}

 
}


extern bool caps_lock;


bool CUIGameFMP::IR_UIOnKeyboardPress(int dik)
{
	if (inherited::IR_UIOnKeyboardPress(dik)) return true;
	if (Device.Paused()) return false;

	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentEntity());
	if (!pInvOwner)			return false;

	CEntityAlive* EA = smart_cast<CEntityAlive*>(Level().CurrentEntity());
	if (!EA || !EA->g_Alive())	return false;

	CActor *pActor = smart_cast<CActor*>(pInvOwner);
	if (!pActor)
		return false;

	if (!pActor->g_Alive())
		return false;

	if (dik == DIK_CAPSLOCK)
	{
 		if (caps_lock)
			caps_lock = false;
		else
			caps_lock = true;
	} 

	if (dik == DIK_F1)
	{
		if (!upgrades_activated)
			upgrades_activated = true;
		else
			upgrades_activated = false;
	}

	switch (get_binded_action(dik))
	{
		case kACTIVE_JOBS:
		{
			if (!pActor->inventory_disabled())
				ShowPdaMenu();			
		} break;

		case kINVENTORY:
		{
			if (!pActor->inventory_disabled())
				ShowActorMenu();			
		} break;
  	
		case kAnimMode:
		{
			if (!m_animation->IsShown())
				m_animation->ShowDialog(false);
		}break;
 
		default:
			break;
	}
	return false;
}
