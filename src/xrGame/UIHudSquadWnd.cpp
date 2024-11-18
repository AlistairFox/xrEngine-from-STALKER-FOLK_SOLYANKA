#include "stdafx.h"
#include "UIHudSquadWnd.h"
#include "ui/UIXmlInit.h"
#include "ui/UIHelper.h"
#include "ui/UIProgressBar.h"
#include "actor.h"
#include "level.h"
#include "game_cl_freemp.h"

CUIHudSquadWnd::CUIHudSquadWnd() {}

CUIHudSquadWnd::~CUIHudSquadWnd() {}

void CUIHudSquadWnd::Init(CUIXml& xml, LPCSTR path)
{
	CUIXmlInit::InitWindow(xml, path, 0, this);

	m_game = smart_cast<game_cl_freemp*>(&Game());
	R_ASSERT(m_game);

	string64 buf;
	for (int i = 0; i < squad_size - 1; ++i)
	{
		xr_sprintf(buf, "hud_squad:member_%d", i);

		m_pSquadMembers[i] = xr_new<CUIHudSquadMember>();
		m_pSquadMembers[i]->SetAutoDelete(true);
		m_pSquadMembers[i]->Init(xml, buf);

		AttachChild(m_pSquadMembers[i]);
		m_pSquadMembers[i]->Show(false);
	}
}

void CUIHudSquadWnd::Update()
{
	inherited::Update();
}

void CUIHudSquadWnd::UpdateMembers()
{
	if (!m_game->local_player) return;

	for (int i = 0; i < squad_size - 1; ++i) m_pSquadMembers[i]->Clear();

	if (!m_game->local_squad->players.size()) return;

	xr_vector<game_PlayerState*>::const_iterator it = m_game->local_squad->players.begin();
	xr_vector<game_PlayerState*>::const_iterator it_e = m_game->local_squad->players.end();

	int j = 0;

	for (; it != it_e; ++it)
	{
		if ((*it) && (*it)->GameID != m_game->local_player->GameID)
		{
			if (j < squad_size - 1)
			{
				m_pSquadMembers[j]->Add(*it);
				j++;
			}
		}
	}
}

void CUIHudSquadMember::Init(CUIXml& xml_doc, LPCSTR path)
{
	CUIXmlInit::InitWindow(xml_doc, path, 0, this);

	CUIXmlInit::InitAutoStaticGroup(xml_doc, "squad_member", 0, this);

	m_pName = UIHelper::CreateStatic(xml_doc, "squad_member:name", this);
	m_pDistance = UIHelper::CreateStatic(xml_doc, "squad_member:distance", this);
	m_pVoiceIcon = UIHelper::CreateStatic(xml_doc, "squad_member:voice_icon", this);
	m_pStatusIcon = UIHelper::CreateStatic(xml_doc, "squad_member:status_icon", this);
	m_pHealthBar = UIHelper::CreateProgressBar(xml_doc, "squad_member:progress_bar_health", this);

	m_name_color = m_pName->TextItemControl()->GetTextColor();
	m_distance_color = m_pDistance->TextItemControl()->GetTextColor();

	m_wounded_color = CUIXmlInit::GetColor(xml_doc, "squad_member:color_wounded", 0, color_rgba(255, 0, 0, 255));
	m_dead_color = CUIXmlInit::GetColor(xml_doc, "squad_member:color_dead", 0, color_rgba(80, 80, 80, 255));

	m_health_blink = pSettings->r_float("actor_condition", "hud_health_blink");
	clamp(m_health_blink, 0.0f, 1.0f);
}

void CUIHudSquadMember::Update()
{
	inherited::Update();

	//------Name
	if (m_ps)
	{
		m_pName->TextItemControl()->SetText(m_ps->getName());

		// m_ps->isSpeaking ? ShowVoiceIcon(true) : ShowVoiceIcon(false);

		m_actor = smart_cast<CActor*>(Level().Objects.net_Find(m_ps->GameID));
	}
	else
		return;

	if (m_actor)
	{
		//------Distance
		if (Actor())
		{
			string16 buf;
			xr_sprintf(buf, "%.0f m", Actor()->Position().distance_to(m_actor->Position()));

			m_pDistance->TextItemControl()->SetText(buf);
		}

		//------StatusIcon
		if (!m_actor->g_Alive())
		{
			m_pStatusIcon->Show(true);

			if (m_status_icon_state != e_dead_state)
			{
				m_status_icon_state = e_dead_state;
				m_pStatusIcon->InitTexture("ui_ingame_hud_squad_dead");

				m_pName->TextItemControl()->SetTextColor(m_dead_color);
				m_pDistance->TextItemControl()->SetTextColor(m_dead_color);
			}
		}
		//else if (m_actor->is_actor_wounded())
		//{
		//	m_pStatusIcon->Show(true);
		//
		//	if (m_status_icon_state != e_wounded_state)
		//	{
		//		m_status_icon_state = e_wounded_state;
		//		m_pStatusIcon->InitTexture("ui_ingame_hud_squad_wounded");
		//
		//		m_pName->TextItemControl()->SetTextColor(m_wounded_color);
		//	}
		//}
		else
		{
			m_pStatusIcon->Show(false);

			m_pName->TextItemControl()->SetTextColor(m_name_color);
			m_pDistance->TextItemControl()->SetTextColor(m_distance_color);
		}

		//------Health
		float cur_health = m_actor->GetfHealth();
		m_pHealthBar->SetProgressPos(iCeil(cur_health * 100.0f * 35.f) / 35.f);
		if (_abs(cur_health - m_last_health) > m_health_blink)
		{
			m_last_health = cur_health;
			m_pHealthBar->m_UIProgressItem.ResetColorAnimation();
		}
	}
	else
	{
		m_pDistance->TextItemControl()->SetText("--");
	}
}

void CUIHudSquadMember::Add(game_PlayerState* ps)
{
	m_ps = ps;

	m_pName->TextItemControl()->SetText("");

	Show(true);
}

void CUIHudSquadMember::Clear()
{
	m_pName->TextItemControl()->Reset();
	m_pDistance->TextItemControl()->Reset();
	ShowVoiceIcon(false);
	m_ps = NULL;
	m_actor = NULL;

	Show(false);
}

void CUIHudSquadMember::ShowVoiceIcon(bool status)
{
	m_pVoiceIcon->Show(status);
}