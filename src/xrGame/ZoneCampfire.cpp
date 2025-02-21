#include "stdafx.h"
#include "ZoneCampfire.h"
#include "ParticlesObject.h"
#include "GamePersistent.h"
#include "../xrEngine/LightAnimLibrary.h"
#include "xrServer_Objects_ALife_Monsters.h"
std::vector<u16> CZoneCampfire::vCampfires;

/*
CZoneCampfire* g_zone = NULL;
void turn_zone()
{
	if(!g_zone) return;
	if(g_zone->is_on())
		g_zone->turn_off_script();
	else
		g_zone->turn_on_script();
}
*/
CZoneCampfire::CZoneCampfire()
:m_pDisabledParticles(NULL),m_pEnablingParticles(NULL),m_turned_on(true),m_turn_time(0)
{
//.	g_zone = this;
}

CZoneCampfire::~CZoneCampfire()
{
	CParticlesObject::Destroy	(m_pDisabledParticles);
	CParticlesObject::Destroy	(m_pEnablingParticles);
	m_disabled_sound.destroy	();
}

void CZoneCampfire::Load(LPCSTR section)
{
	inherited::Load(section);
}

void CZoneCampfire::GoEnabledState()
{
	inherited::GoEnabledState();
	
}

void CZoneCampfire::GoDisabledState()
{
	inherited::GoDisabledState		();
}

BOOL CZoneCampfire::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC))
		return FALSE;

	CZoneCampfire::vCampfires.push_back(this->ID());

	CSE_ALifeCustomZone* cse_Camp = smart_cast<CSE_ALifeCustomZone*>(DC);

	FireTimer = Device.dwTimeGlobal + 10000;
	OffTimerStart = true;

	return TRUE;
}

#define OVL_TIME 3000
void CZoneCampfire::turn_on_script()
{
	if( psDeviceFlags.test(rsR2|rsR3|rsR4) )
	{
		m_turn_time				= Device.dwTimeGlobal+OVL_TIME;
		m_turned_on				= true;
		GoEnabledState			();
	}
}

void CZoneCampfire::turn_off_script()
{
	if( psDeviceFlags.test(rsR2|rsR3|rsR4) )
	{
		m_turn_time				= Device.dwTimeGlobal+OVL_TIME;
		m_turned_on				= false;
		GoDisabledState			();
	}
}

bool CZoneCampfire::is_on()
{
	return m_turned_on;
}
#include "Level.h"
#include "Actor.h"
#include "game_cl_freemp.h"

void CZoneCampfire::shedule_Update(u32	dt)
{
	if (OnServer())
	{
		if (UpdateTimer <= Device.dwTimeGlobal)
		{
			UpdateTimer = Device.dwTimeGlobal + 10000;
			NET_Packet P;
			CGameObject::u_EventGen(P, GE_CAMPFIRES_UPDATE, ID());
			P.w_u8(u8(m_eZoneState));
			CGameObject::u_EventSend(P);
		}
	}


	if (!IsEnabled() && m_turn_time)
	{
		UpdateWorkload	(dt);
	}

	if (OnServer() && IsEnabled())
	{
		if (OffTimerStart)
		{
			if (FireTimer <= Device.dwTimeGlobal)
			{
				SwitchZoneState(eZoneStateDisabled);
				FireTimer = 0;
				OffTimerStart = false;
			}
		}
	}

	if(m_pIdleParticles)
	{
		Fvector vel;
		vel.mul(GamePersistent().Environment().wind_blast_direction,GamePersistent().Environment().wind_strength_factor);
		m_pIdleParticles->UpdateParent(XFORM(),vel);
	}

	inherited::shedule_Update(dt);
}

void CZoneCampfire::OnStateSwitch(EZoneState new_state)
{
	if (new_state == eZoneStateDisabled)
		Disable();
	else
		Enable();

	if (new_state == eZoneStateAccumulate)
		PlayAccumParticles();

	if (new_state == eZoneStateAwaking)
		PlayAwakingParticles();

	m_eZoneState = new_state;
	m_iPreviousStateTime = m_iStateTime = 0;
}

bool CZoneCampfire::Enable()
{
	if (IsEnabled())
		return false;

	m_turn_time = Device.dwTimeGlobal + OVL_TIME;
	m_turned_on = true;

	if (OnServer())
	{
		FireTimer = Device.dwTimeGlobal + 300000;
		OffTimerStart = true;
	}

	if (m_pDisabledParticles)
	{
		m_pDisabledParticles->Stop(FALSE);
		CParticlesObject::Destroy(m_pDisabledParticles);
	}

	m_disabled_sound.stop();
	m_disabled_sound.destroy();

	LPCSTR str = pSettings->r_string(cNameSect(), "enabling_particles");
	m_pEnablingParticles = CParticlesObject::Create(str, FALSE);
	m_pEnablingParticles->UpdateParent(XFORM(), zero_vel);
	m_pEnablingParticles->Play(false);

	return inherited::Enable();
}

bool CZoneCampfire::Disable()
{
	if (!IsEnabled())
		return false;

	m_turn_time = Device.dwTimeGlobal + OVL_TIME;
	m_turned_on = false;

	R_ASSERT(NULL == m_pDisabledParticles);
	LPCSTR str = pSettings->r_string(cNameSect(), "disabled_particles");
	m_pDisabledParticles = CParticlesObject::Create(str, FALSE);
	m_pDisabledParticles->UpdateParent(XFORM(), zero_vel);
	m_pDisabledParticles->Play(false);


	str = pSettings->r_string(cNameSect(), "disabled_sound");
	m_disabled_sound.create(str, st_Effect, sg_SourceType);
	m_disabled_sound.play_at_pos(0, Position(), true);

	return inherited::Disable();
}

void CZoneCampfire::UpdateCL()
{

	inherited::UpdateCL();

}

void CZoneCampfire::OnEvent(NET_Packet& P, u16 type)
{
	switch (type)
	{
		case GE_CAMPFIRES_UPDATE:
		{
			if (OnServer())
				break;

			u8 State = P.r_u8();
			OnStateSwitch(EZoneState(State));
		}break;
	}
	inherited::OnEvent(P, type);
}


void CZoneCampfire::PlayIdleParticles(bool bIdleLight)
{
	if(m_turn_time==0 || m_turn_time-Device.dwTimeGlobal<(OVL_TIME-2000))
	{
		inherited::PlayIdleParticles(bIdleLight);
		if(m_pEnablingParticles)
		{
			m_pEnablingParticles->Stop	(FALSE);
			CParticlesObject::Destroy	(m_pEnablingParticles);
		}
	}
}

void CZoneCampfire::StopIdleParticles(bool bIdleLight)
{
	if(m_turn_time==0 || m_turn_time-Device.dwTimeGlobal<(OVL_TIME-500))
		inherited::StopIdleParticles(bIdleLight);
}

BOOL CZoneCampfire::AlwaysTheCrow()
{
	if(m_turn_time)
		return TRUE;
	else
		return inherited::AlwaysTheCrow();
}

void CZoneCampfire::UpdateWorkload(u32 dt)
{
	inherited::UpdateWorkload(dt);
	if(m_turn_time>Device.dwTimeGlobal)
	{
		float k = float(m_turn_time-Device.dwTimeGlobal)/float(OVL_TIME);

		if(m_turned_on)
		{
			k = 1.0f-k;
			PlayIdleParticles	(true);
			StartIdleLight		();
		}else
		{
			StopIdleParticles	(false);
		}

		if(m_pIdleLight && m_pIdleLight->get_active())
		{
			VERIFY(m_pIdleLAnim);
			int frame = 0;
			u32 clr		= m_pIdleLAnim->CalculateBGR(Device.fTimeGlobal,frame);
			Fcolor		fclr;
			fclr.set	(	((float)color_get_B(clr)/255.f)*k,
							((float)color_get_G(clr)/255.f)*k,
							((float)color_get_R(clr)/255.f)*k,
							1.f);
			
			float range = m_fIdleLightRange + 0.25f*::Random.randF(-1.f,1.f);
			range	*= k;

			m_pIdleLight->set_range	(range);
			m_pIdleLight->set_color	(fclr);
		}


	}else
	if(m_turn_time)
	{
		m_turn_time = 0;
		if(m_turned_on)
		{
			PlayIdleParticles(true);
		}else
		{
			StopIdleParticles(true);
		}
	}
}