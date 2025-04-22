#include "stdafx.h"
#include "WeaponMagazined.h"

#include "actor.h"
#include "inventory.h"
#include "xr_level_controller.h"
#include "UIGameCustom.h"

#include "CustomDetector.h"

#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"

#include "ActorEffector.h"
#include "Level.h"
#include "object_saver.h"
#include "object_loader.h"

CWeaponMagazined::CWeaponMagazined(ESoundTypes eSoundType) : CWeapon()
{
	m_eSoundShow = ESoundTypes(SOUND_TYPE_ITEM_TAKING | eSoundType);
	m_eSoundHide = ESoundTypes(SOUND_TYPE_ITEM_HIDING | eSoundType);
	m_eSoundShot = ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING | eSoundType);
	m_eSoundEmptyClick = ESoundTypes(SOUND_TYPE_WEAPON_EMPTY_CLICKING | eSoundType);
	m_eSoundReload = ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING | eSoundType);
	m_eSoundClose = ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING);
	m_sounds_enabled = true;

	psWpnAnimsFlag = { 0 };

	m_sSilencerFlameParticles = m_sSilencerSmokeParticles = NULL;

	m_bFireSingleShot = false;
	m_iShotNum = 0;
	m_fOldBulletSpeed = 0;
	bullet_cnt = 0;
	m_iQueueSize = WEAPON_ININITE_QUEUE;
	m_bLockType = false;
	m_bAutoreloadEnabled = READ_IF_EXISTS(pSettingsSe7kills, r_bool, "gameplay", "autoreload_enabled", true);
	m_bNeedBulletInGun = false;
	m_bHasDifferentFireModes = false;
	m_opened = false;
	m_bUseFiremodeChangeAnim = true;
	bHasBulletsToHide = false;

	m_sSndShotCurrent = nullptr;
}

CWeaponMagazined::~CWeaponMagazined()
{
}
 
void CWeaponMagazined::net_Destroy()
{
	inherited::net_Destroy();
}

void CWeaponMagazined::SetAnimFlag(u32 flag, LPCSTR anim_name)
{
	if (pSettings->line_exist(hud_sect, anim_name))
		psWpnAnimsFlag.set(flag, TRUE);
	else
		psWpnAnimsFlag.set(flag, FALSE);
}

bool CWeaponMagazined::UseScopeTexture()
{
	return bScopeIsHasTexture;
}

void CWeaponMagazined::FireStart()
{
	if (!IsMisfire())
	{
		if (IsValid())
		{
			if (!IsWorking() || AllowFireWhileWorking())
			{
				if (GetState() == eReload)
					return;
				if (GetState() == eShowing)
					return;
				if (GetState() == eHiding)
					return;
				if (GetState() == eMisfire)
					return;
				if (GetState() == eUnMisfire)
					return;
				if (GetState() == eFiremodePrev)
					return;
				if (GetState() == eFiremodeNext)
					return;
				if (GetState() == eLaserSwitch)
					return;
				if (GetState() == eFlashlightSwitch)
					return;

				inherited::FireStart();

				if (iAmmoElapsed == 0)
					OnMagazineEmpty();
				else {
					R_ASSERT(H_Parent());
					SwitchState(eFire);
				}
			}
		}
		else
		{
			if (eReload != GetState())
				OnMagazineEmpty();
		}
	}
	else
	{
		//misfire
		//CGameObject* object = smart_cast<CGameObject*>(H_Parent());
		//if (object)
		//	object->callback(GameObject::eOnWeaponJammed)(object->lua_game_object(), this->lua_game_object());

		if (smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity() == H_Parent()))
			CurrentGameUI()->AddCustomStatic("gun_jammed", true);

		OnEmptyClick();
	}
}

void CWeaponMagazined::FireEnd()
{
	inherited::FireEnd();

	if (m_bAutoreloadEnabled)
	{
		CActor* actor = smart_cast<CActor*>(H_Parent());
		if (Actor()->get_state() & mcSprint && !GameConstants::GetReloadIfSprint())
			return;

		if (m_pInventory && !iAmmoElapsed && actor && GetState() != eReload)
			Reload();
	}
}

void CWeaponMagazined::UpdateCL()
{
	inherited::UpdateCL();
	float dt = Device.fTimeDelta;

	//когда происходит апдейт состояния оружия
	//ничего другого не делать
	if (GetNextState() == GetState())
	{
		switch (GetState())
		{
		case eShowing:
		case eHiding:
		case eReload:
		case eSprintStart:
		case eSprintEnd:
		case eIdle:
		{
			fShotTimeCounter -= dt;
			clamp(fShotTimeCounter, 0.0f, flt_max);
		}break;
		case eFire:
		{
			state_Fire(dt);
		}break;
		case eMisfire:		state_Misfire(dt);	break;
		case eMagEmpty:		state_MagEmpty(dt);	break;
		case eHidden:		break;
		}
	}

	UpdateSounds();
}

void CWeaponMagazined::UpdateSounds()
{
	if (Device.dwFrame == dwUpdateSounds_Frame)
		return;

	dwUpdateSounds_Frame = Device.dwFrame;

	Fvector P = get_LastFP();
	m_sounds.SetPosition("sndShow", P);
	m_sounds.SetPosition("sndHide", P);
	if (psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
		m_sounds.SetPosition("sndClose", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_changefiremode"))
		m_sounds.SetPosition("sndFireModes", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_laser_on"))
		m_sounds.SetPosition("sndLaserOn", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_laser_off"))
		m_sounds.SetPosition("sndLaserOff", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_torch_on"))
		m_sounds.SetPosition("sndFlashlightOn", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_torch_off"))
		m_sounds.SetPosition("sndFlashlightOff", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_change_zoom"))
		m_sounds.SetPosition("sndChangeZoom", P);

	m_sounds.SetPosition("sndReload", P);
}

void CWeaponMagazined::SetDefaults()
{
	CWeapon::SetDefaults();
}

bool CWeaponMagazined::Action(u16 cmd, u32 flags)
{
	if (inherited::Action(cmd, flags))
		return true;
	//если оружие чем-то занято, то ничего не делать
	if (IsPending())
		return false;
 
	switch (cmd)
	{
		case kWPN_RELOAD:
		{
			if (flags & CMD_START)
			{
				if (iAmmoElapsed < iMagazineSize || IsMisfire())
				{
					if (GetState() == eUnMisfire) // Rietmon: Запрещаем перезарядку, если играет анима передергивания затвора
						return false;

					Reload();

					/*PIItem Det = Actor()->inventory().ItemFromSlot(DETECTOR_SLOT);
					if (!Det)
						Reload(); // Rietmon: Если в слоте нету детектора, то он не может быть активен

					if (Det)
					{
						CCustomDetector* pDet = smart_cast<CCustomDetector*>(Det);
						if (!pDet->IsWorking())
							Reload();
					}
					*/
				}
				else
				{
					Msg("Ammo Elapsed: %d / Mag: %d", iAmmoElapsed, iMagazineSize);
				}
				return true;
			}
		}break;
	
		case kWPN_FIREMODE_PREV:
		{
			Msg("Firemode Prev Clicked");
			if (flags & CMD_START)
			{
				OnPrevFireMode();
				return true;
			};
		}
		break;

		case kWPN_FIREMODE_NEXT:
		{
			Msg("Firemode Next Clicked");

			if (flags & CMD_START)
			{
				OnNextFireMode();
				return true;
			};
		}
		break;
	}

	return false;
}

void CWeaponMagazined::OnH_A_Chield()
{
	if (m_bHasDifferentFireModes)
	{
		CActor* actor = smart_cast<CActor*>(H_Parent());
		if (!actor) SetQueueSize(-1);
		else SetQueueSize(GetCurrentFireMode());
	};
	inherited::OnH_A_Chield();
};
 
float CWeaponMagazined::GetWeaponDeterioration()
{
	return (m_iShotNum == 1) ? conditionDecreasePerShot : conditionDecreasePerQueueShot;
};

void CWeaponMagazined::net_Export(NET_Packet& P)
{
	inherited::net_Export(P);

	//	P.w_u8(u8(m_iCurFireMode & 0x00ff));
}

void CWeaponMagazined::net_Import(NET_Packet& P)
{
	inherited::net_Import(P);

	// 	m_iCurFireMode = P.r_u8();
	//	SetQueueSize(GetCurrentFireMode());
}
 
void CWeaponMagazined::OnEvent(NET_Packet& P, u16 type)
{
	switch (type)
	{
		case GE_WPN_UNLOAD_AMMO:
		{
			if (OnServer())
			{
				u16 amount = P.r_u16();
				shared_str sec;
				P.r_stringZ(sec);
				SpawnAmmo(amount, sec.c_str());
			}
		}break;

		case GE_WPN_UPDATE_AMMO:
		{
			u16 BoxID = P.r_u16();
			u16 m_boxCurr = P.r_u16();
 			CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*> (Level().Objects.net_Find(BoxID));
			if (ammo)
				ammo->m_boxCurr = m_boxCurr;
		}break;

		case GE_WPN_SWITCH_FIREMODE:
		{
			m_iCurFireMode = P.r_u8();
			SetQueueSize(GetCurrentFireMode());
		}break;

		default:
			inherited::OnEvent(P, type);
			break;
	}
}
 
void CWeaponMagazined::FireBullet(const Fvector& pos,
	const Fvector& shot_dir,
	float fire_disp,
	const CCartridge& cartridge,
	u16 parent_id,
	u16 weapon_id,
	bool send_hit)
{
	if (m_iBaseDispersionedBulletsCount)
	{
		if (m_iShotNum <= 1)
		{
			m_fOldBulletSpeed = GetBulletSpeed();
			SetBulletSpeed(m_fBaseDispersionedBulletsSpeed);
		}
		else if (m_iShotNum > m_iBaseDispersionedBulletsCount)
		{
			SetBulletSpeed(m_fOldBulletSpeed);
		}
	}
	inherited::FireBullet(pos, shot_dir, fire_disp, cartridge, parent_id, weapon_id, send_hit);
}

// AVO: for custom added sounds check if sound exists
bool CWeaponMagazined::WeaponSoundExist(LPCSTR section, LPCSTR sound_name, bool log) const
{
	pcstr str;
	bool sec_exist = process_if_exists_set(section, sound_name, &CInifile::r_string, str, true);
	if (sec_exist)
		return true;
#ifdef DEBUG
	if (log)
		Msg("~ [WARNING] ------ Sound [%s] does not exist in [%s]", sound_name, section);
#endif
	return false;
}
