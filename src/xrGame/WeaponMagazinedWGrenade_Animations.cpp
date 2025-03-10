// Animations 
#include "StdAfx.h"
#include "WeaponMagazinedWGrenade.h"
#include "Actor.h"

#pragma warning(disable:4995)


//виртуальные функции для проигрывания анимации HUD
void CWeaponMagazinedWGrenade::PlayAnimShow()
{
	VERIFY(GetState() == eShowing);

	if (IsGrenadeLauncherAttached())
	{
		if (!m_bGrenadeMode)
			HUD_VisualBulletUpdate();

		if (!m_bGrenadeMode)
		{
			if (IsMisfire())
				PlayHUDMotionIfExists({ "anm_show_w_gl_jammed", "anm_show_jammed_w_gl", "anm_show_w_gl" }, true, GetState());
			else if (IsMagazineEmpty())
				PlayHUDMotionIfExists({ "anm_show_w_gl_empty", "anm_show_empty_w_gl", "anm_show_w_gl" }, true, GetState());
			else
				PlayHUDMotion("anm_show_w_gl", FALSE, this, GetState());
		}
		else
		{
			if (IsMisfire())
				PlayHUDMotionIfExists({ "anm_show_g_jammed", "anm_show_jammed_g", "anm_show_g" }, true, GetState());
			else if (IsMainMagazineEmpty())
				PlayHUDMotionIfExists({ "anm_show_g_empty", "anm_show_empty_g", "anm_show_g" }, true, GetState());
			else
				PlayHUDMotion("anm_show_g", FALSE, this, GetState());
		}
	}
	else
		inherited::PlayAnimShow();
}

void CWeaponMagazinedWGrenade::PlayAnimHide()
{
	VERIFY(GetState() == eHiding);

	if (IsGrenadeLauncherAttached())
	{
		if (!m_bGrenadeMode)
		{
			if (IsMisfire())
				PlayHUDMotionIfExists({ "anm_hide_w_gl_jammed", "anm_hide_jammed_w_gl", "anm_hide_w_gl" }, true, GetState());
			else if (IsEmptyMagazine())
				PlayHUDMotionIfExists({ "anm_hide_w_gl_empty", "anm_hide_empty_w_gl", "anm_hide_w_gl" }, true, GetState());
			else
				PlayHUDMotion("anm_hide_w_gl", TRUE, this, GetState());
		}
		else
		{
			if (IsMisfire())
				PlayHUDMotionIfExists({ "anm_hide_g_jammed", "anm_hide_jammed_g", "anm_hide_g" }, true, GetState());
			else if (IsEmptyMagazine())
				PlayHUDMotionIfExists({ "anm_hide_g_empty", "anm_hide_empty_g", "anm_hide_g" }, true, GetState());
			else
				PlayHUDMotion("anm_hide_g", TRUE, this, GetState());
		}
	}
	else
		inherited::PlayAnimHide();
}

void CWeaponMagazinedWGrenade::PlayAnimReload()
{
	VERIFY(GetState() == eReload);

	if (IsGrenadeLauncherAttached())
	{
		if (iAmmoElapsed == 0)
			PlayHUDMotionIfExists({ "anm_reload_w_gl_empty", "anm_reload_empty_w_gl", "anm_reload_w_gl" }, true, GetState());
		else
			PlayHUDMotionIfExists({ "anm_reload_w_gl", "anm_reload_w_gl_empty", "anm_reload_empty_w_gl" }, true, GetState());
	}
	else
		inherited::PlayAnimReload();
}

void CWeaponMagazinedWGrenade::PlayAnimIdle()
{
	if (GetState() == eSwitch)
		return;

	if (IsGrenadeLauncherAttached())
	{
		if (IsZoomed())
		{
			if (IsRotatingToZoom())
			{
				string32 guns_aim_anm;
				strconcat(sizeof(guns_aim_anm), guns_aim_anm, "anm_idle_aim_start", m_bGrenadeMode ? "_g" : "_w_gl");
				if (isHUDAnimationExist(guns_aim_anm))
				{
					PlayHUDMotionNew(guns_aim_anm, true, GetState());
					return;
				}
			}

			if (const char* guns_aim_anm = GetAnimAimName())
			{
				string64 guns_aim_anm_full;
				strconcat(sizeof(guns_aim_anm_full), guns_aim_anm_full, guns_aim_anm, m_bGrenadeMode ? "_g" : "_w_gl");

				if (isHUDAnimationExist(guns_aim_anm_full))
				{
					PlayHUDMotionNew(guns_aim_anm_full, true, GetState());
					return;
				}
				else if (strstr(guns_aim_anm_full, "_jammed"))
				{
					char* jammed_position = strstr(guns_aim_anm_full, "_jammed");
					int jammed_length = strlen("_jammed");

					char new_guns_aim_anm[100];
					strncpy(new_guns_aim_anm, guns_aim_anm_full, jammed_position - guns_aim_anm_full);
					strcpy(new_guns_aim_anm + (jammed_position - guns_aim_anm_full), guns_aim_anm_full + (jammed_position - guns_aim_anm_full) + jammed_length);

					if (isHUDAnimationExist(new_guns_aim_anm))
					{
						PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
						return;
					}
				}
				else if (strstr(guns_aim_anm_full, "_empty"))
				{
					char* empty_position = strstr(guns_aim_anm_full, "_empty");
					int empty_length = strlen("_empty");

					char new_guns_aim_anm[100];
					strncpy(new_guns_aim_anm, guns_aim_anm_full, empty_position - guns_aim_anm_full);
					strcpy(new_guns_aim_anm + (empty_position - guns_aim_anm_full), guns_aim_anm_full + (empty_position - guns_aim_anm_full) + empty_length);

					if (isHUDAnimationExist(new_guns_aim_anm))
					{
						PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
						return;
					}
				}
			}

			if (m_bGrenadeMode)
			{
				if (IsMisfire())
					PlayHUDMotionIfExists({ "anm_idle_aim_g_jammed", "anm_idle_aim_jammed_g", "anm_idle_aim_g", "anm_idle_g_aim", "anm_idle_aim" }, true, GetState());
				else if (IsMainMagazineEmpty())
					PlayHUDMotionIfExists({ "anm_idle_aim_g_empty", "anm_idle_aim_empty_g", "anm_idle_aim_g", "anm_idle_g_aim", "anm_idle_aim" }, true, GetState());
				else
					PlayHUDMotionIfExists({ "anm_idle_aim_g", "anm_idle_g_aim", "anm_idle_aim" }, true, GetState());
			}
			else
			{
				if (IsMisfire())
					PlayHUDMotionIfExists({ "anm_idle_aim_w_gl_jammed", "anm_idle_aim_jammed_w_gl", "anm_idle_aim_w_gl", "anm_idle_w_gl_aim", "anm_idle_aim" }, true, GetState());
				else if (IsMagazineEmpty())
					PlayHUDMotionIfExists({ "anm_idle_aim_w_gl_empty", "anm_idle_aim_empty_w_gl", "anm_idle_aim_w_gl", "anm_idle_w_gl_aim", "anm_idle_aim" }, true, GetState());
				else
					PlayHUDMotionIfExists({ "anm_idle_aim_w_gl", "anm_idle_w_gl_aim", "anm_idle_aim" }, true, GetState());
			}
		}
		else
		{
			if (IsRotatingFromZoom())
			{
				string32 guns_aim_anm;
				strconcat(sizeof(guns_aim_anm), guns_aim_anm, "anm_idle_aim_end", m_bGrenadeMode ? "_g" : "_w_gl");
				if (isHUDAnimationExist(guns_aim_anm)) {
					PlayHUDMotionNew(guns_aim_anm, true, GetState());
					return;
				}
			}

			int act_state = 0;
			CActor* pActor = smart_cast<CActor*>(H_Parent());

			if (pActor)
			{
				const u32 State = pActor->get_state();
				if (State & mcSprint)
				{
					if (!m_bSprintType)
					{
						SwitchState(eSprintStart);
						return;
					}

					act_state = 1;
				}
				else if (m_bSprintType)
				{
					SwitchState(eSprintEnd);
					return;
				}
				else if (State & mcAnyMove)
				{
					if (!(State & mcCrouch))
					{
						if (State & mcAccel) //Ходьба медленная (SHIFT)
							act_state = 5;
						else
							act_state = 2;
					}
					else if (State & mcAccel) //Ходьба в присяде (CTRL+SHIFT)
						act_state = 4;
					else
						act_state = 3;
				}
			}

			if (m_bGrenadeMode)
			{
				switch (act_state)
				{
				case 0:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_jammed_g", "anm_idle_g_jammed", "anm_idle_g" }, true, GetState());
					else if (IsMainMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_empty_g", "anm_idle_g_empty", "anm_idle_g" }, true, GetState());
					else
						PlayHUDMotionNew({ "anm_idle_g" }, true, GetState());
					break;
				case 1:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_sprint_jammed_g", "anm_idle_sprint_g_jammed", "anm_idle_sprint_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else if (IsMainMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_sprint_empty_g", "anm_idle_sprint_g_empty", "anm_idle_sprint_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_sprint_g", "anm_idle_g" }, true, GetState());
					break;
				case 2:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_jammed_g", "anm_idle_moving_g_jammed", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else if (IsMainMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_empty_g", "anm_idle_moving_g_empty", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					break;
				case 3:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_jammed_g", "anm_idle_moving_crouch_g_jammed", "anm_idle_moving_crouch_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else if (IsMainMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_empty_g", "anm_idle_moving_crouch_g_empty", "anm_idle_moving_crouch_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					break;
				case 4:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_jammed_g", "anm_idle_moving_crouch_slow_g_jammed", "anm_idle_moving_crouch_slow_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else if (IsMainMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_empty_g", "anm_idle_moving_crouch_slow_g_empty", "anm_idle_moving_crouch_slow_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_g", "anm_idle_moving_crouch_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					break;
				case 5:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_slow_jammed_g", "anm_idle_moving_slow_g_jammed", "anm_idle_moving_slow_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else if (IsMainMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_slow_empty_g", "anm_idle_moving_slow_g_empty", "anm_idle_moving_slow_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_slow_g", "anm_idle_moving_g", "anm_idle_g" }, true, GetState());
					break;
				}
			}
			else
			{
				switch (act_state)
				{
				case 0:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_jammed_w_gl", "anm_idle_w_gl_jammed","anm_idle_w_gl_jammed", "anm_idle_w_gl" }, true, GetState());
					else if (IsMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_empty_w_gl", "anm_idle_w_gl_empty","anm_idle_w_gl_empty", "anm_idle_w_gl" }, true, GetState());
					else
						PlayHUDMotionNew({ "anm_idle_w_gl" }, true, GetState());
					break;
				case 1:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_sprint_jammed_w_gl", "anm_idle_sprint_w_gl_jammed", "anm_idle_sprint_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else if (IsMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_sprint_empty_w_gl", "anm_idle_sprint_w_gl_empty", "anm_idle_sprint_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_sprint_w_gl", "anm_idle_w_gl" }, true, GetState());
					break;
				case 2:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_jammed_w_gl", "anm_idle_moving_w_gl_jammed", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else if (IsMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_empty_w_gl", "anm_idle_moving_w_gl_empty", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					break;
				case 3:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_jammed_w_gl", "anm_idle_moving_crouch_w_gl_jammed", "anm_idle_moving_crouch_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else if (IsMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_empty_w_gl", "anm_idle_moving_crouch_w_gl_empty", "anm_idle_moving_crouch_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					break;
				case 4:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_jammed_w_gl", "anm_idle_moving_crouch_slow_w_gl_jammed", "anm_idle_moving_crouch_slow_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else if (IsMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_empty_w_gl", "anm_idle_moving_crouch_slow_w_gl_empty","anm_idle_moving_crouch_slow_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_w_gl", "anm_idle_moving_crouch_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					break;
				case 5:
					if (IsMisfire())
						PlayHUDMotionIfExists({ "anm_idle_moving_slow_jammed_w_gl", "anm_idle_moving_slow_w_gl_jammed","anm_idle_moving_slow_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else if (IsMagazineEmpty())
						PlayHUDMotionIfExists({ "anm_idle_moving_slow_empty_w_gl", "anm_idle_moving_slow_w_gl_empty", "anm_idle_moving_slow_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					else
						PlayHUDMotionIfExists({ "anm_idle_moving_slow_w_gl", "anm_idle_moving_w_gl", "anm_idle_w_gl" }, true, GetState());
					break;
				}
			}
		}
	}
	else
		inherited::PlayAnimIdle();
}

void CWeaponMagazinedWGrenade::PlayAnimShoot()
{
	if (m_bGrenadeMode)
	{
		string_path guns_shoot_anm{};
		strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, (isHUDAnimationExist("anm_shoot") ? "anm_shoot" : "anm_shots"), (this->IsZoomed() && !this->IsRotatingToZoom()) ? "_aim" : "", "_g", (IsMisfire() ? "_jammed" : IsMainMagazineEmpty() ? "_empty" : ""));

		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots_g" }, false, GetState());
	}
	else
	{
		//HUD_VisualBulletUpdate();

		VERIFY(GetState() == eFire);

		if (IsGrenadeLauncherAttached())
		{
			string_path guns_shoot_anm{};
			strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, (isHUDAnimationExist("anm_shoot") ? "anm_shoot" : "anm_shots"), (iAmmoElapsed == 1) ? "_last" : "", (this->IsZoomed() && !this->IsRotatingToZoom()) ? (this->IsScopeAttached() ? "_aim_scope" : "_aim") : "", this->IsSilencerAttached() ? "_sil" : "", "_w_gl");

			if (iAmmoElapsed == 1)
				PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shot_l_w_gl", "anm_shots_w_gl" }, false, GetState());
			else
				PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots_w_gl" }, false, GetState());
		}
		else
		{
			inherited::PlayAnimShoot();
		}
	}
}

void  CWeaponMagazinedWGrenade::PlayAnimModeSwitch()
{
	string_path guns_switch_anm{};
	strconcat(sizeof(guns_switch_anm), guns_switch_anm, "anm_switch", m_bGrenadeMode ? "_g" : isHUDAnimationExist("anm_switch_w_gl") ? "_w_gl" : "", (IsMisfire() ? "_jammed" : IsMainMagazineEmpty() ? "_empty" : ""));

	if (isHUDAnimationExist(guns_switch_anm))
	{
		PlayHUDMotionNew(guns_switch_anm, true, GetState());
		return;
	}
	else if (strstr(guns_switch_anm, "_jammed"))
	{
		char new_guns_switch_anm[256];
		strcpy(new_guns_switch_anm, guns_switch_anm);
		new_guns_switch_anm[strlen(guns_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_switch_anm))
		{
			PlayHUDMotionNew(new_guns_switch_anm, true, GetState());
			return;
		}
		else
			return;
	}
	else if (strstr(guns_switch_anm, "_empty"))
	{
		char new_guns_switch_anm[256];
		strcpy(new_guns_switch_anm, guns_switch_anm);
		new_guns_switch_anm[strlen(guns_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_switch_anm))
			PlayHUDMotionNew(new_guns_switch_anm, true, GetState());
	}
}

void CWeaponMagazinedWGrenade::PlayAnimBore()
{
	if (IsGrenadeLauncherAttached())
	{
		if (m_bGrenadeMode)
		{
			if (IsMisfireNow())
				PlayHUDMotionIfExists({ "anm_bore_g_jammed", "anm_bore_jammed_g", "anm_bore_g", "anm_bore" }, true, GetState());
			else if (IsMainMagazineEmpty())
				PlayHUDMotionIfExists({ "anm_bore_g_empty", "anm_bore_empty_g", "anm_bore_g", "anm_bore" }, true, GetState());
			else
				PlayHUDMotion("anm_bore_g", TRUE, this, GetState());
		}
		else
		{
			if (IsMisfireNow())
				PlayHUDMotionIfExists({ "anm_bore_w_gl_jammed", "anm_bore_jammed_w_gl", "anm_bore_w_gl", "anm_bore" }, true, GetState());
			else if (IsMagazineEmpty())
				PlayHUDMotionIfExists({ "anm_bore_w_gl_empty", "anm_bore_empty_w_gl", "anm_bore_w_gl", "anm_bore" }, true, GetState());
			else
				PlayHUDMotion("anm_bore_w_gl", TRUE, this, GetState());
		}
	}
	else
		inherited::PlayAnimBore();
}

void CWeaponMagazinedWGrenade::PlayAnimFireMode()
{
	if (!IsGrenadeLauncherAttached())
	{
		inherited::PlayAnimFireMode();
		return;
	}

	string_path guns_firemode_anm{};

	if (isHUDAnimationExist("anm_changefiremode"))
	{
		strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode", m_bGrenadeMode ? "_g" : "_w_gl", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

		if (isHUDAnimationExist(guns_firemode_anm))
		{
			PlayHUDMotionNew(guns_firemode_anm, true, GetState());
			return;
		}
		else if (strstr(guns_firemode_anm, "_jammed"))
		{
			char new_guns_firemode_anm[256];
			strcpy(new_guns_firemode_anm, guns_firemode_anm);
			new_guns_firemode_anm[strlen(guns_firemode_anm) - strlen("_jammed")] = '\0';

			if (isHUDAnimationExist(new_guns_firemode_anm))
			{
				PlayHUDMotionNew(new_guns_firemode_anm, true, GetState());
				return;
			}
		}
		else if (strstr(guns_firemode_anm, "_empty"))
		{
			char new_guns_firemode_anm[256];
			strcpy(new_guns_firemode_anm, guns_firemode_anm);
			new_guns_firemode_anm[strlen(guns_firemode_anm) - strlen("_empty")] = '\0';

			if (isHUDAnimationExist(new_guns_firemode_anm))
			{
				PlayHUDMotionNew(new_guns_firemode_anm, true, GetState());
				return;
			}
		}
	}

	strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_from_", (m_iCurFireMode == 0) ? "a_to_1" : (m_iCurFireMode == 1) ? (m_aFireModes.size() == 3) ? "1_to_2" : "1_to_a" : (m_iCurFireMode == 2) ? "2_to_a" : "a_to_1");

	string64 guns_aim_anm_full;
	strconcat(sizeof(guns_aim_anm_full), guns_aim_anm_full, guns_firemode_anm, m_bGrenadeMode ? "_g" : "_w_gl", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_aim_anm_full))
	{
		PlayHUDMotionNew(guns_aim_anm_full, true, GetState());
		return;
	}
	else if (strstr(guns_aim_anm_full, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
			inherited::PlayAnimFireMode();
	}
	else if (strstr(guns_aim_anm_full, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
			inherited::PlayAnimFireMode();
	}
	else
		inherited::PlayAnimFireMode();
}

void CWeaponMagazinedWGrenade::PlayAnimLaserSwitch()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_laser", IsLaserOn() ? "_on" : "_off", m_bGrenadeMode ? "_g" : "_w_gl", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
			inherited::PlayAnimLaserSwitch();
	}
	else if (strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
			inherited::PlayAnimLaserSwitch();
	}
	else
		inherited::PlayAnimLaserSwitch();
}

void CWeaponMagazinedWGrenade::PlayAnimFlashlightSwitch()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_torch", IsFlashlightOn() ? "_on" : "_off", m_bGrenadeMode ? "_g" : "_w_gl", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
			inherited::PlayAnimFlashlightSwitch();
	}
	else if (strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
			inherited::PlayAnimFlashlightSwitch();
	}
	else
		inherited::PlayAnimFlashlightSwitch();
}
