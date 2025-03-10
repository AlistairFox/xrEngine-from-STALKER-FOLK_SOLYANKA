#include "stdafx.h"
#include "WeaponMagazinedWGrenade.h"

// ACTIONS
void CWeaponMagazinedWGrenade::OnAnimationEnd(u32 state)
{
	if (eSwitch == GetState())
		SwitchState(eIdle);

	switch (state)
	{
	case eSwitch:
	{
		SwitchState(eIdle);
	}break;
	case eFire:
	{
		if (m_bGrenadeMode)
			Reload();
	}break;
	}
	inherited::OnAnimationEnd(state);
}


void CWeaponMagazinedWGrenade::OnStateSwitch(u32 S)
{
	switch (S)
	{
	case eSwitch:
	{
		if (!SwitchMode()) {
			SwitchState(eIdle);
			return;
		}
	}break;
	}

	inherited::OnStateSwitch(S);
	UpdateGrenadeVisibility(!!iAmmoElapsed || S == eReload);
}

bool CWeaponMagazinedWGrenade::Action(u16 cmd, u32 flags)
{
	if (m_bGrenadeMode && (cmd == kWPN_FIREMODE_PREV || cmd == kWPN_FIREMODE_NEXT))
		return false;

	if (m_bGrenadeMode && cmd == kWPN_FIRE)
	{
		if (IsPending())
			return				false;

		if (flags & CMD_START)
		{
			if (iAmmoElapsed)
				LaunchGrenadeSend();
			else
				Reload();

			Msg("Ammo Elapsed: %d", iAmmoElapsed);

			if (GetState() == eIdle)
				OnEmptyClick();
		}
		return					true;
	}

	if (inherited::Action(cmd, flags))
		return true;

	switch (cmd)
	{
	case kWPN_FUNC:
	{
		if (flags & CMD_START && !IsPending())
			SwitchState(eSwitch);
		return true;
	}
	}
	return false;
}
#include "Level.h"
void CWeaponMagazinedWGrenade::OnEvent(NET_Packet& P, u16 type)
{
	inherited::OnEvent(P, type);
	u16 id;
	switch (type)
	{
	case GE_WPN_STARTGRENADE:
	{
		Fvector position, direction;
		P.r_vec3(position);
		P.r_vec3(direction);
		LaunchGrenadeRecived(direction, position);
	}break;

	case GE_WPN_SPAWNGRENADE:
	{
		if (OnServer())
		{
			shared_str section;
			P.r_stringZ(section);
			CRocketLauncher::SpawnRocket(section, this);
		}break;
	}break;

	case GE_OWNERSHIP_TAKE:
	{
		P.r_u16(id);
		CRocketLauncher::AttachRocket(id, this);
	}
	break;
	case GE_OWNERSHIP_REJECT:
	case GE_LAUNCH_ROCKET:
	{
		bool bLaunch = (type == GE_LAUNCH_ROCKET);
		P.r_u16(id);
		CRocketLauncher::DetachRocket(id, bLaunch);
		if (bLaunch)
		{
			PlayAnimShoot();

			PlaySound("sndShotG", get_LastFP());

			AddShotEffector();
			StartFlameParticles2();
		}
		break;
	}
	}
}


// PROCESSERS


void  CWeaponMagazinedWGrenade::PerformSwitchGL()
{
	m_bGrenadeMode = !m_bGrenadeMode;

	iMagazineSize = m_bGrenadeMode ? 1 : iMagazineSize2;

	if (m_bGrenadeMode)
		iAmmoElapsedMain = iAmmoElapsed;

	m_ammoTypes.swap(m_ammoTypes2);

	swap(m_ammoType, m_ammoType2);
	swap(m_DefaultCartridge, m_DefaultCartridge2);

	xr_vector<CCartridge> l_magazine;
	while (m_magazine.size()) { l_magazine.push_back(m_magazine.back()); m_magazine.pop_back(); }
	while (m_magazine2.size()) { m_magazine.push_back(m_magazine2.back()); m_magazine2.pop_back(); }
	while (l_magazine.size()) { m_magazine2.push_back(l_magazine.back()); l_magazine.pop_back(); }
	iAmmoElapsed = (int)m_magazine.size();

	m_BriefInfo_CalcFrame = 0;
}


void CWeaponMagazinedWGrenade::switch2_Reload()
{
	VERIFY(GetState() == eReload);
	if (m_bGrenadeMode)
	{
		PlaySound("sndReloadG", get_LastFP2());

		if (IsMisfire())
			PlayHUDMotionIfExists({ "anm_reload_g_jammed", "anm_reload_jammed_g", "anm_reload_g" }, true, GetState());
		else if (IsMainMagazineEmpty())
			PlayHUDMotionIfExists({ "anm_reload_g_empty", "anm_reload_empty_g", "anm_reload_g" }, true, GetState());
		else
			PlayHUDMotion("anm_reload_g", FALSE, this, GetState());

		SetPending(TRUE);
	}
	else
		inherited::switch2_Reload();
}


void CWeaponMagazinedWGrenade::switch2_Unmis()
{
	if (m_bGrenadeMode) return;

	VERIFY(GetState() == eUnMisfire);

	if (GrenadeLauncherAttachable() && IsGrenadeLauncherAttached())
	{
		if (m_sounds_enabled)
		{
			if (m_sounds.FindSoundItem("sndReloadMisfire", false) && (isHUDAnimationExist("anm_reload_w_gl_misfire") || isHUDAnimationExist("anm_reload_misfire_w_gl")))
				PlaySound("sndReloadMisfire", get_LastFP());
			else if (m_sounds.FindSoundItem("sndReloadEmpty", false) && (isHUDAnimationExist("anm_reload_w_gl_empty") || isHUDAnimationExist("anm_reload_empty_w_gl")))
				PlaySound("sndReloadEmpty", get_LastFP());
			else
				PlaySound("sndReload", get_LastFP());
		}

		if (isHUDAnimationExist("anm_reload_w_gl_misfire") || isHUDAnimationExist("anm_reload_misfire_w_gl"))
			PlayHUDMotionIfExists({ "anm_reload_w_gl_misfire", "anm_reload_misfire_w_gl" }, true, GetState());
		else if (isHUDAnimationExist("anm_reload_w_gl_empty") || isHUDAnimationExist("anm_reload_empty_w_gl"))
			PlayHUDMotionIfExists({ "anm_reload_w_gl_empty", "anm_reload_empty_w_gl" }, true, GetState());
		else
			PlayHUDMotion("anm_reload_w_gl", TRUE, this, GetState());
	}
	else
		inherited::switch2_Unmis();
}

void CWeaponMagazinedWGrenade::OnShot()
{
	if (m_bGrenadeMode)
	{
		PlayAnimShoot();

		PlaySound("sndShotG", get_LastFP());

		AddShotEffector();
		StartFlameParticles2();

		// CGameObject* object = smart_cast<CGameObject*>(H_Parent());
		// if (object)
		// 	object->callback(GameObject::eOnWeaponFired)(object->lua_game_object(), this->lua_game_object(), iAmmoElapsed);
	}
	else
		inherited::OnShot();
}

bool CWeaponMagazinedWGrenade::SwitchMode()
{
	bool bUsefulStateToSwitch = ((eSwitch == GetState()) || (eIdle == GetState()) || (eHidden == GetState()) || (eMisfire == GetState()) || (eMagEmpty == GetState())) && (!IsPending());

	if (!bUsefulStateToSwitch)
		return false;

	if (!IsGrenadeLauncherAttached())
		return false;

	OnZoomOut();

	SetPending(TRUE);

	PerformSwitchGL();

	PlaySound("sndSwitch", get_LastFP());

	PlayAnimModeSwitch();

	m_BriefInfo_CalcFrame = 0;

	return					true;
}

#include "inventory.h"
#include "inventoryOwner.h"
void CWeaponMagazinedWGrenade::state_Fire(float dt)
{
	VERIFY(fOneShotTime > 0.f);

	//режим стрельбы подствольника
	if (m_bGrenadeMode)
	{
	}
	//режим стрельбы очередями
	else
		inherited::state_Fire(dt);
}