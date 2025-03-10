////////////////////////////////////////////////////////////////////////////////
//	Module		:	cta_game_artefact.cpp
//	Created		:	19.12.2007
//	Modified	:	19.12.2007
//	Autor		:	Alexander Maniluk
//	Description	:	Artefact object for Capture The Artefact game mode
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cta_game_artefact.h"
#include "cta_game_artefact_activation.h"
#include "game_cl_capture_the_artefact.h"
#include "xrServer_Objects_Alife_Items.h"
#include "xr_level_controller.h"

CtaGameArtefact::CtaGameArtefact()
{
	// game object must present...
	m_game				= smart_cast<game_cl_CaptureTheArtefact*>(&Game());
	m_artefact_rpoint	= nullptr;
	m_my_team			= etSpectatorsTeam;
}

CtaGameArtefact::~CtaGameArtefact()
{
}

bool CtaGameArtefact::IsMyTeamArtefact()
{
	if(!m_game)	
		return true;

	R_ASSERT			(H_Parent());
	game_PlayerState*	ps = m_game->GetPlayerByGameID(H_Parent()->ID());
	R_ASSERT			(ps != NULL);
	if (ps->team == etGreenTeam)
	{
		if (m_game->GetGreenArtefactID() == this->ID())
		{
			return true;
		}
	} else if (ps->team == etBlueTeam)
	{
		if (m_game->GetBlueArtefactID() == this->ID())
		{
			return true;
		}
	}
	return false;
}

bool CtaGameArtefact::Action(s32 cmd, u32 flags)
{
	if (m_game && (cmd == kWPN_FIRE) && (flags & CMD_START))
	{
		if (!m_game->CanActivateArtefact() || !IsMyTeamArtefact())
			return true;
	}
	
	return inherited::Action((u16)cmd, flags);
}

void CtaGameArtefact::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch(S);
}

void CtaGameArtefact::OnAnimationEnd(u32 state)
{
	if (!H_Parent())
	{
#ifndef MASTER_GOLD
		Msg("! ERROR: enemy artefact activation, H_Parent is NULL.");
#endif // #ifndef MASTER_GOLD
		return;
	}
	inherited::OnAnimationEnd(state);
}

void CtaGameArtefact::UpdateCLChild()
{
	inherited::UpdateCLChild();
	if(H_Parent())
		XFORM().set(H_Parent()->XFORM());

	if (!m_artefact_rpoint)
		InitializeArtefactRPoint();
	
	if (!m_artefact_rpoint)
	{
#ifdef DEBUG
		Msg("--- Waiting for sync packet, for artefact rpoint.");
#endif // #ifdef DEBUG
		return;
	}

	VERIFY(m_artefact_rpoint);
	
	if (m_game && m_artefact_rpoint->similar(XFORM().c, m_game->GetBaseRadius()) &&  PPhysicsShell())
	{
		MoveTo(*m_artefact_rpoint);
		deactivate_physics_shell();
	}
}

void CtaGameArtefact::InitializeArtefactRPoint()
{
	if (!m_game)
		return;

	if (ID() == m_game->GetGreenArtefactID())
	{
		m_artefact_rpoint = &m_game->GetGreenArtefactRPoint();
		m_my_team = etGreenTeam;
	}
	else if (ID() == m_game->GetBlueArtefactID())
	{
		m_artefact_rpoint = &m_game->GetBlueArtefactRPoint();
		m_my_team = etBlueTeam;
	}
}

void CtaGameArtefact::CreateArtefactActivation()
{
	if (OnServer())
	{
		NET_Packet						P;
		CGameObject::u_EventGen			(P,GE_OWNERSHIP_REJECT, H_Parent()->ID());
		P.w_u16							(ID());
		P.w_u8(0);
		if (!m_artefact_rpoint)
		{
			m_artefact_rpoint = &Position();
		}
		P.w_vec3(*m_artefact_rpoint);
		CGameObject::u_EventSend	(P);
	
		MoveTo(*m_artefact_rpoint);	//for server net_Import
	}
}

bool CtaGameArtefact::CanTake() const
{
	if (!inherited::CanTake())
		return false;
	
	return true;
};

void CtaGameArtefact::PH_A_CrPr()
{
}