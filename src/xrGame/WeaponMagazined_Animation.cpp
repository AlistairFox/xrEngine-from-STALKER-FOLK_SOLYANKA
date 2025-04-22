#include "StdAfx.h"
#include "WeaponMagazined.h"



// ANIMS
void CWeaponMagazined::PlayReloadSound()
{
	if (m_sounds_enabled)
	{
		if (iAmmoElapsed == 0)
			if (m_sounds.FindSoundItem("sndReloadEmpty", false) && psWpnAnimsFlag.test(ANM_RELOAD_EMPTY))
				PlaySound("sndReloadEmpty", get_LastFP());
			else
				PlaySound("sndReload", get_LastFP());
		else
			PlaySound("sndReload", get_LastFP());
	}
}

// ANIMS

void CWeaponMagazined::PlayAnimFireMode()
{
	string_path guns_firemode_anm{};

	if (isHUDAnimationExist("anm_changefiremode"))
	{
		strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

		PlayHUDMotionIfExists({ guns_firemode_anm, "anm_changefiremode" }, true, GetState());
		return;
	}

	strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_from_", (m_iCurFireMode == 0) ? "a_to_1" : (m_iCurFireMode == 1) ? (m_aFireModes.size() == 3) ? "1_to_2" : "1_to_a" : (m_iCurFireMode == 2) ? "2_to_a" : "a_to_1");

	string64 guns_aim_anm_full;
	strconcat(sizeof(guns_aim_anm_full), guns_aim_anm_full, guns_firemode_anm, (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_aim_anm_full))
	{
		PlayHUDMotionNew(guns_aim_anm_full, true, GetState());
		return;
	}
	else if (strstr(guns_aim_anm_full, "_jammed"))
	{
		char new_guns_aim_anm[256];
		xr_strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (strstr(guns_aim_anm_full, "_empty"))
	{
		char new_guns_aim_anm[256];
		xr_strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}


void CWeaponMagazined::PlayAnimLaserSwitch()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_laser", IsLaserOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		xr_strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		xr_strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimFlashlightSwitch()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_torch", !IsFlashlightOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		xr_strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		xr_strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}


// PLAY ANIMS

void CWeaponMagazined::PlayAnimShow()
{
	VERIFY(GetState() == eShowing);

	if (iAmmoElapsed >= 1)
		m_opened = false;
	else
		m_opened = true;

	HUD_VisualBulletUpdate();

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SHOW_EMPTY))
		PlayHUDMotion("anm_show_empty", FALSE, this, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_show_jammed"))
		PlayHUDMotion("anm_show_jammed", false, this, GetState());
	else
		PlayHUDMotion("anm_show", FALSE, this, GetState());
}

void CWeaponMagazined::PlayAnimHide()
{
	VERIFY(GetState() == eHiding);

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY))
		PlayHUDMotion("anm_hide_empty", TRUE, this, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_hide_jammed"))
		PlayHUDMotion("anm_hide_jammed", true, this, GetState());
	else
		PlayHUDMotion("anm_hide", TRUE, this, GetState());
}

void CWeaponMagazined::PlayAnimBore()
{
	inherited::PlayAnimBore();
}

void CWeaponMagazined::PlayAnimIdleSprint()
{
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SPRINT_EMPTY))
		PlayHUDMotion("anm_idle_sprint_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_sprint_jammed"))
		PlayHUDMotion("anm_idle_sprint_jammed", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleSprint();
}

void CWeaponMagazined::PlayAnimIdleMoving()
{
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_MOVING_EMPTY))
		PlayHUDMotion("anm_idle_moving_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_moving_jammed"))
		PlayHUDMotion("anm_idle_moving_jammed", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleMoving();
}

void CWeaponMagazined::PlayAnimReload()
{
	VERIFY(GetState() == eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_reload_empty", "anm_reload" }, true, GetState());
	else
		PlayHUDMotion("anm_reload", TRUE, this, GetState());
}

void CWeaponMagazined::PlayAnimAim()
{
	if (IsRotatingToZoom())
	{
		if (isHUDAnimationExist("anm_idle_aim_start"))
		{
			PlayHUDMotionNew("anm_idle_aim_start", true, GetState());
			return;
		}
	}

	if (const char* guns_aim_anm = GetAnimAimName())
	{
		if (isHUDAnimationExist(guns_aim_anm))
		{
			PlayHUDMotionNew(guns_aim_anm, true, GetState());
			return;
		}
		else if (strstr(guns_aim_anm, "_jammed"))
		{
			char new_guns_aim_anm[256];
			xr_strcpy(new_guns_aim_anm, guns_aim_anm);
			new_guns_aim_anm[strlen(guns_aim_anm) - strlen("_jammed")] = '\0';

			if (isHUDAnimationExist(new_guns_aim_anm))
			{
				PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
				return;
			}
		}
		else if (strstr(guns_aim_anm, "_empty"))
		{
			char new_guns_aim_anm[256];
			xr_strcpy(new_guns_aim_anm, guns_aim_anm);
			new_guns_aim_anm[strlen(guns_aim_anm) - strlen("_empty")] = '\0';

			if (isHUDAnimationExist(new_guns_aim_anm))
			{
				PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
				return;
			}
		}
	}

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_AIM_EMPTY))
		PlayHUDMotion("anm_idle_aim_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_aim_jammed"))
		PlayHUDMotion("anm_idle_aim_jammed", true, nullptr, GetState());
	else
		PlayHUDMotion("anm_idle_aim", TRUE, NULL, GetState());
}

void CWeaponMagazined::PlayAnimIdle()
{
	if (GetState() != eIdle)	return;

	if (TryPlayAnimIdle()) return;

	if (IsZoomed())
		PlayAnimAim();
	else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_IDLE_EMPTY))
		PlayHUDMotion("anm_idle_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_jammed") && !TryPlayAnimIdle())
		PlayHUDMotion("anm_idle_jammed", true, nullptr, GetState());
	else
	{
		if (IsRotatingFromZoom())
		{
			if (isHUDAnimationExist("anm_idle_aim_end"))
			{
				PlayHUDMotionNew("anm_idle_aim_end", true, GetState());
				return;
			}
		}
		inherited::PlayAnimIdle();
	}
}

void CWeaponMagazined::PlayAnimShoot()
{
	VERIFY(GetState() == eFire);

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, (isHUDAnimationExist("anm_shoot") ? "anm_shoot" : "anm_shots"), (iAmmoElapsed == 1) ? "_last" : "", (IsZoomed() && !IsRotatingToZoom()) ? (IsScopeAttached() ? "_aim_scope" : "_aim") : "", (IsSilencerAttached() && m_bUseAimSilShotAnim) ? "_sil" : "");

	//HUD_VisualBulletUpdate();

	if (iAmmoElapsed == 1)
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shot_l", "anm_shots" }, false, GetState());
	else
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots" }, false, GetState());
}


