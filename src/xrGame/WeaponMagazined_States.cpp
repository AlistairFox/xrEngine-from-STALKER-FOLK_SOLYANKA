#include "StdAfx.h"
#include "WeaponMagazined.h"
#include "Actor.h"
#include "EffectorZoomInertion.h"
#include "ActorEffector.h"
#include "UIGameCustom.h"
// States


ENGINE_API  extern float psHUD_FOV;
ENGINE_API  extern float psHUD_FOV_def;

void CWeaponMagazined::OnStateSwitch(u32 S)
{
	HUD_VisualBulletUpdate();

	inherited::OnStateSwitch(S);
	CInventoryOwner* owner = smart_cast<CInventoryOwner*>(this->H_Parent());
	switch (S)
	{
	case eFiremodeNext:
	{
		PlaySound("sndFireModes", get_LastFP());
		switch2_ChangeFireMode();
	}break;
	case eFiremodePrev:
	{
		PlaySound("sndFireModes", get_LastFP());
		switch2_ChangeFireMode();
	}break;
	case eLaserSwitch:
	{
		if (IsLaserOn())
			PlaySound("sndLaserOn", get_LastFP());
		else
			PlaySound("sndLaserOff", get_LastFP());

		switch2_LaserSwitch();
	}break;
	case eFlashlightSwitch:
	{
		if (IsFlashlightOn())
			PlaySound("sndFlashlightOn", get_LastFP());
		else
			PlaySound("sndFlashlightOff", get_LastFP());

		switch2_FlashlightSwitch();
	}break;
	case eIdle:
		switch2_Idle();
		break;
	case eFire:
		switch2_Fire();
		break;
	case eUnMisfire:
		if (owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Unmis();
		break;
	case eMisfire:
		if (smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity() == H_Parent()))
			CurrentGameUI()->AddCustomStatic("gun_jammed", true);
		break;
	case eMagEmpty:
		switch2_Empty();
		break;
	case eReload:
		if (owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Reload();
		break;
	case eShowing:
		if (owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Showing();
		break;
	case eHiding:
		if (owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Hiding();
		break;
	case eHidden:
		switch2_Hidden();
		break;
	}
}

void CWeaponMagazined::switch2_Idle()
{
	m_iShotNum = 0;
	if (m_fOldBulletSpeed != 0.f)
		SetBulletSpeed(m_fOldBulletSpeed);

	SetPending(FALSE);
	PlayAnimIdle();
}

void CWeaponMagazined::switch2_ChangeFireMode()
{
	if (GetState() != eFiremodeNext && GetState() != eFiremodePrev)
		return;

	FireEnd();
	PlayAnimFireMode();
	SetPending(TRUE);
}

void CWeaponMagazined::switch2_LaserSwitch()
{
	if (GetState() != eLaserSwitch)
		return;

	FireEnd();
	PlayAnimLaserSwitch();
	SetPending(TRUE);
}

void CWeaponMagazined::switch2_FlashlightSwitch()
{
	if (GetState() != eFlashlightSwitch)
		return;

	FireEnd();
	PlayAnimFlashlightSwitch();
	SetPending(TRUE);
}

// BASE MODES

void CWeaponMagazined::switch2_Fire()
{
	CInventoryOwner* io = smart_cast<CInventoryOwner*>(H_Parent());
	CInventoryItem* ii = smart_cast<CInventoryItem*>(this);

	if (!io)
		return;

	m_bStopedAfterQueueFired = false;
	m_bFireSingleShot = true;
	m_iShotNum = 0;

	if ((OnClient() || Level().IsDemoPlay()) && !IsWorking())
		FireStart();

}

void CWeaponMagazined::switch2_Empty()
{
	OnZoomOut();

	if (m_bAutoreloadEnabled)
	{
		if (!TryReload())
		{
			OnEmptyClick();
		}
		else
		{
			inherited::FireEnd();
		}
	}
	else
	{
		OnEmptyClick();
	}
}



void CWeaponMagazined::switch2_Reload()
{
	CWeapon::FireEnd();

	PlayReloadSound();
	PlayAnimReload();
	SetPending(TRUE);
}
void CWeaponMagazined::switch2_Hiding()
{
	OnZoomOut();
	CWeapon::FireEnd();

	if (m_sounds_enabled)
	{
		if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
			PlaySound("sndClose", get_LastFP());
		else
			PlaySound("sndHide", get_LastFP());
	}

	PlayAnimHide();
	SetPending(TRUE);
}

void CWeaponMagazined::switch2_Unmis()
{
	VERIFY(GetState() == eUnMisfire);

	if (m_sounds_enabled)
	{
		if (m_sounds.FindSoundItem("sndReloadMisfire", false) && isHUDAnimationExist("anm_reload_misfire"))
			PlaySound("sndReloadMisfire", get_LastFP());
		else if (m_sounds.FindSoundItem("sndReloadJammed", false) && isHUDAnimationExist("anm_reload_jammed"))
			PlaySound("sndReloadJammed", get_LastFP());
		else
			PlayReloadSound();
	}

	if (isHUDAnimationExist("anm_reload_misfire"))
		PlayHUDMotionIfExists({ "anm_reload_misfire", "anm_reload" }, true, GetState());
	else if (isHUDAnimationExist("anm_reload_jammed"))
		PlayHUDMotionIfExists({ "anm_reload_jammed", "anm_reload" }, true, GetState());
	else
		PlayAnimReload();
}

void CWeaponMagazined::switch2_Hidden()
{
	CWeapon::FireEnd();

	StopCurrentAnimWithoutCallback();

	signal_HideComplete();
	RemoveShotEffector();

	if (pSettings->line_exist(item_sect, "hud_fov"))
		m_nearwall_last_hud_fov = m_base_fov;
	else
		m_nearwall_last_hud_fov = psHUD_FOV_def;
}
void CWeaponMagazined::switch2_Showing()
{
	if (m_sounds_enabled)
		PlaySound("sndShow", get_LastFP());

	SetPending(TRUE);
	PlayAnimShow();
}



// ONSTATE
void CWeaponMagazined::state_Fire(float dt)
{
	if (iAmmoElapsed > 0)
	{
		VERIFY(fOneShotTime > 0.f);

		Fvector					p1, d;
		p1.set(get_LastFP());
		d.set(get_LastFD());

		CEntity* E = smart_cast<CEntity*>(H_Parent());
		if (E != nullptr)
		{
			E->g_fireParams(this, p1, d);

			if (!E->g_stateFire())
				StopShooting();
		}
		else
		{
			SwitchState(eIdle);
			return;
		}

		if (m_iShotNum == 0)
		{
			m_vStartPos = p1;
			m_vStartDir = d;
		};

		while (!m_magazine.empty() &&
			fShotTimeCounter < 0 &&
			(IsWorking() || m_bFireSingleShot) &&
			(m_iQueueSize < 0 || m_iShotNum < m_iQueueSize)
			)
		{
			if (CheckForMisfire())
			{
				StopShooting();
				return;
			}

			m_bFireSingleShot = false;

			fShotTimeCounter += fOneShotTime;

			++m_iShotNum;

			OnShot();

			if (m_iShotNum > m_iBaseDispersionedBulletsCount)
				FireTrace(p1, d);
			else
				FireTrace(m_vStartPos, m_vStartDir);
		}

		if (m_iShotNum == m_iQueueSize)
			m_bStopedAfterQueueFired = true;

		UpdateSounds();
	}

	if (fShotTimeCounter < 0)
	{
		if (iAmmoElapsed == 0)
			OnMagazineEmpty();

		StopShooting();
	}
	else
	{
		fShotTimeCounter -= dt;
	}
}

void CWeaponMagazined::state_Misfire(float dt)
{
	OnEmptyClick();
	SwitchState(eIdle);

	bMisfire = true;

	UpdateSounds();
}

void CWeaponMagazined::state_MagEmpty(float dt)
{
}

void CWeaponMagazined::OnShot()
{
	// Если актор бежит - останавливаем его
	if (ParentIsActor() && GameConstants::GetStopActorIfShoot())
		Actor()->set_state_wishful(Actor()->get_state_wishful() & (~mcSprint));

	// Camera	
	AddShotEffector();

	// Animation
	PlayAnimShoot();

	HUD_VisualBulletUpdate();

	// Shell Drop
	Fvector vel;
	PHGetLinearVell(vel);
	OnShellDrop(get_LastSP(), vel);

	// Огонь из ствола
	StartFlameParticles();

	//дым из ствола
	ForceUpdateFireParticles();
	StartSmokeParticles(get_LastFP(), vel);

	// Проиграем звук помпы отдельно, если не будет работать то будем думать что делать и как быть
	if (m_sounds.FindSoundItem("sndPumpGun", false))
		PlaySound("sndPumpGun", get_LastFP());

	if (ParentIsActor())
	{
		string128 sndName;
		strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), "Actor");
		if (m_sounds.FindSoundItem(sndName, false))
		{
			m_sounds.PlaySound(sndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
			return;
		}
	}

	string128 sndName;
	strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), (iAmmoElapsed == 1) ? "Last" : "");

	if (m_sounds.FindSoundItem(sndName, false))
		m_sounds.PlaySound(sndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
	else
		m_sounds.PlaySound(m_sSndShotCurrent.c_str(), get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);

	// Эхо выстрела
	if (IsSilencerAttached() == false)
	{
		bool bIndoor = false;
		if (H_Parent() != nullptr)
		{
			bIndoor = H_Parent()->renderable_ROS()->get_luminocity_hemi() < WEAPON_INDOOR_HEMI_FACTOR;
		}

		if (bIndoor && m_sounds.FindSoundItem("sndReflect", false))
		{
			if (IsHudModeNow())
			{
				HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(WEAPON_SND_REFLECTION_HUD_FACTOR);
			}
			PlaySound("sndReflect", get_LastFP());
			HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(1.0f);
		}
	}

	// CGameObject* object = smart_cast<CGameObject*>(H_Parent());
	// if (object)
	// 	object->callback(GameObject::eOnWeaponFired)(object->lua_game_object(), this->lua_game_object(), iAmmoElapsed);
}



// Ready State if Animation End
void CWeaponMagazined::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eReload:
	{
		CheckMagazine(); // Основано на механизме из Lost Alpha: New Project
		// Авторы: rafa & Kondr48

		CCartridge FirstBulletInGun;

		bool bNeedputBullet = iAmmoElapsed > 0;

		if (m_bNeedBulletInGun && bNeedputBullet)
		{
			FirstBulletInGun = m_magazine.back();
			if (!m_magazine.empty())
				m_magazine.pop_back();
			iAmmoElapsed--;
		}

		ReloadMagazine();

		if (m_bNeedBulletInGun && bNeedputBullet)
		{
			m_magazine.push_back(FirstBulletInGun);
			iAmmoElapsed++;
		}

		SwitchState(eIdle);

	}break;// End of reload animation
	case eHiding:	SwitchState(eHidden);   break;	// End of Hide
	case eShowing:	SwitchState(eIdle);		break;	// End of Show
	case eIdle:		switch2_Idle();			break;  // Keep showing idle
	case eUnMisfire:
	{
		bMisfire = false;
		if (!m_magazine.empty())
			m_magazine.pop_back();
		iAmmoElapsed--;
		SwitchState(eIdle);
	}break; // End of UnMisfire animation

	case eFiremodePrev:
	{
		SwitchState(eIdle);

		m_iCurFireMode = (m_iCurFireMode - 1 + m_aFireModes.size()) % m_aFireModes.size();
		SetQueueSize(GetCurrentFireMode());

		Msg("[OnPrevFireMode] Change Fire Mode : %u | Queue: %u", m_iCurFireMode, GetCurrentFireMode());

		NET_Packet packet;
		Game().u_EventGen(packet, GE_WPN_SWITCH_FIREMODE, ID());
		packet.w_u8(m_iCurFireMode);
		Game().u_EventSend(packet);

		break;
	}
	case eFiremodeNext:
	{
		SwitchState(eIdle);
		m_iCurFireMode = (m_iCurFireMode + 1 + m_aFireModes.size()) % m_aFireModes.size();
		SetQueueSize(GetCurrentFireMode());

		Msg("[OnNextFireMode] Change Fire Mode : %u | Queue: %u", m_iCurFireMode, GetCurrentFireMode());

		NET_Packet packet;
		Game().u_EventGen(packet, GE_WPN_SWITCH_FIREMODE, ID());
		packet.w_u8(m_iCurFireMode);
		Game().u_EventSend(packet);
		break;
	}

	case eLaserSwitch:
	{
		SwitchState(eIdle);
		break;
	}
	case eFlashlightSwitch:
	{
		SwitchState(eIdle);
		break;
	}
	}
	inherited::OnAnimationEnd(state);
}



//переключение режимов стрельбы одиночными и очередями
bool CWeaponMagazined::SwitchMode()
{
	if (eIdle != GetState() || IsPending()) return false;

	if (SingleShotMode())
		m_iQueueSize = WEAPON_ININITE_QUEUE;
	else
		m_iQueueSize = 1;

	PlaySound("sndEmptyClick", get_LastFP());

	return true;
}

void	CWeaponMagazined::OnNextFireMode()
{
	if (!m_bHasDifferentFireModes) return;

	if (isHUDAnimationExist("anm_changefiremode_from_1_to_a") || isHUDAnimationExist("anm_changefiremode"))
		SwitchState(eFiremodeNext);
	if (GetState() != eIdle) return;

	// Ended in On AnimationEnd (exist anim) !!!
	m_iCurFireMode = (m_iCurFireMode + 1 + m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());

	Msg("OnNextFireMode");
	NET_Packet packet;
	Game().u_EventGen(packet, GE_WPN_SWITCH_FIREMODE, ID());
	packet.w_u8(m_iCurFireMode);
	Game().u_EventSend(packet);
};

void	CWeaponMagazined::OnPrevFireMode()
{
	if (!m_bHasDifferentFireModes)
		return;

	if (isHUDAnimationExist("anm_changefiremode_from_1_to_a") || isHUDAnimationExist("anm_changefiremode"))
		SwitchState(eFiremodePrev);

	if (GetState() != eIdle)
		return;

	Msg("OnPrevFireMode");
	// Ended in On AnimationEnd (exist anim) !!!
	m_iCurFireMode = (m_iCurFireMode - 1 + m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());

	NET_Packet packet;
	Game().u_EventGen(packet, GE_WPN_SWITCH_FIREMODE, ID());
	packet.w_u8(m_iCurFireMode);
	Game().u_EventSend(packet);
};

// Переключение Зума
void CWeaponMagazined::OnZoomIn()
{
	inherited::OnZoomIn();

	if (GetState() == eIdle)
		PlayAnimIdle();

	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (pActor)
	{
		CEffectorZoomInertion* S = smart_cast<CEffectorZoomInertion*>	(pActor->Cameras().GetCamEffector(eCEZoom));
		if (!S)
		{
			S = (CEffectorZoomInertion*)pActor->Cameras().AddCamEffector(xr_new<CEffectorZoomInertion>());
			S->Init(this);
		};
		S->SetRndSeed(pActor->GetZoomRndSeed());
		R_ASSERT(S);
	}
}

void CWeaponMagazined::OnZoomOut()
{
	if (!IsZoomed())
		return;

	inherited::OnZoomOut();

	if (GetState() == eIdle)
		PlayAnimIdle();

	CActor* pActor = smart_cast<CActor*>(H_Parent());

	if (pActor)
		pActor->Cameras().RemoveCamEffector(eCEZoom);

}

