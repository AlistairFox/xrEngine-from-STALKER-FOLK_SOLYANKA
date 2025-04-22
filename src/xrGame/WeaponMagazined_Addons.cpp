#include "StdAfx.h"
#include "Weapon.h"

//Addons
#include "Level.h"
#include "Scope.h"
#include "Silencer.h"
#include "GrenadeLauncher.h"
#include <EffectorZoomInertion.h>
#include <string_table.h>
// #include "LaserDesignator.h"
// #include "TacticalTorch.h"

void CWeaponMagazined::ApplySilencerKoeffs()
{
	cur_silencer_koef = m_silencer_koef;
}

void CWeaponMagazined::ResetSilencerKoeffs()
{
	cur_silencer_koef.Reset();
}

bool CWeaponMagazined::CanAttach(PIItem pIItem)
{
	CScope* pScope = smart_cast<CScope*>(pIItem);
	CSilencer* pSilencer = smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher* pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);
	//CLaserDesignator* pLaser = smart_cast<CLaserDesignator*>(pIItem);
	//CTacticalTorch* pTacticalTorch = smart_cast<CTacticalTorch*>(pIItem);

	if (pScope &&
		m_eScopeStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 /*&&
		(m_scopes[cur_scope]->m_sScopeName == pIItem->object().cNameSect())*/)
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == pIItem->object().cNameSect())
					return true;
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == pIItem->object().cNameSect())
					return true;
			}
		}
		return false;
	}
	else if (pSilencer &&
		m_eSilencerStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
		(m_sSilencerName == pIItem->object().cNameSect()))
		return true;
	else if (pGrenadeLauncher &&
		m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
		(m_sGrenadeLauncherName == pIItem->object().cNameSect()))
		return true;
	//else if (pLaser &&
	//	m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
	//	(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) == 0 &&
	//	(m_sLaserName == pIItem->object().cNameSect()))
	//	return true;
	//else if (pTacticalTorch &&
	//	m_eTacticalTorchStatus == ALife::eAddonAttachable &&
	//	(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) == 0 &&
	//	(m_sTacticalTorchName == pIItem->object().cNameSect()))
	//	return true;
	else
		return inherited::CanAttach(pIItem);
}

bool CWeaponMagazined::CanDetach(const char* item_section_name)
{
	if (m_eScopeStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonScope))/* &&
		(m_scopes[cur_scope]->m_sScopeName	== item_section_name))*/
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == item_section_name)
					return true;
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == item_section_name)
					return true;
			}
		}
		return false;
	}
	//	   return true;
	else if (m_eSilencerStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer) &&
		(m_sSilencerName == item_section_name))
		return true;
	else if (m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
		(m_sGrenadeLauncherName == item_section_name))
		return true;
	else if (m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) &&
		(m_sLaserName == item_section_name))
		return true;
	else if (m_eTacticalTorchStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) &&
		(m_sTacticalTorchName == item_section_name))
		return true;
	else
		return inherited::CanDetach(item_section_name);
}

bool CWeaponMagazined::Attach(PIItem pIItem, bool b_send_event)
{
	bool result = false;

	CScope* pScope = smart_cast<CScope*>(pIItem);
	CSilencer* pSilencer = smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher* pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);
	// CLaserDesignator* pLaser = smart_cast<CLaserDesignator*>(pIItem);
	// CTacticalTorch* pTacticalTorch = smart_cast<CTacticalTorch*>(pIItem);

	if (pScope &&
		m_eScopeStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 /*&&
		(m_scopes[cur_scope]->m_sScopeName == pIItem->object().cNameSect())*/)
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == pIItem->object().cNameSect())
					m_cur_scope = u8(it - m_scopes.begin());
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == pIItem->object().cNameSect())
					m_cur_scope = u8(it - m_scopes.begin());
			}
		}
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonScope;
		result = true;
	}
	else if (pSilencer &&
		m_eSilencerStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
		(m_sSilencerName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonSilencer;
		result = true;
	}
	else if (pGrenadeLauncher &&
		m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
		(m_sGrenadeLauncherName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;
		result = true;
	}
	//	else if (pLaser &&
	//		m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
	//		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) == 0 &&
	//		(m_sLaserName == pIItem->object().cNameSect()))
	//	{
	//		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator;
	//		result = true;
	//	}
	//	else if (pTacticalTorch &&
	//		m_eTacticalTorchStatus == ALife::eAddonAttachable &&
	//		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) == 0 &&
	//		(m_sTacticalTorchName == pIItem->object().cNameSect()))
	//	{
	//		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch;
	//		result = true;
	//	}

	if (result)
	{
		if (pScope && bUseAltScope)
		{
			bNVsecondVPstatus = !!pSettings->line_exist(pIItem->object().cNameSect(), "scope_nightvision");
		}

		if (b_send_event && OnServer())
		{
			//уничтожить подсоединенную вещь из инвентаря
//.			pIItem->Drop					();
			pIItem->object().DestroyObject();
		};

		UpdateAltScope();
		UpdateAddonsVisibility();
		InitAddons();

		return true;
	}
	else
		return inherited::Attach(pIItem, b_send_event);
}

bool CWeaponMagazined::DetachScope(const char* item_section_name, bool b_spawn_item)
{
	bool detached = false;
	SCOPES_VECTOR_IT it = m_scopes.begin();
	shared_str iter_scope_name = "none";
	for (; it != m_scopes.end(); it++)
	{
		if (bUseAltScope)
		{
			iter_scope_name = (*it);
		}
		else
		{
			iter_scope_name = pSettings->r_string((*it), "scope_name");
		}
		if (!xr_strcmp(iter_scope_name, item_section_name))
		{
			m_cur_scope = NULL;
			m_cur_scope_bone = NULL;
			detached = true;
		}
	}
	return detached;
}

bool CWeaponMagazined::Detach(const char* item_section_name, bool b_spawn_item)
{
	if (m_eScopeStatus == ALife::eAddonAttachable &&
		DetachScope(item_section_name, b_spawn_item))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonScope) == 0)
		{
			Msg("ERROR: scope addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonScope;

		UpdateAltScope();
		UpdateAddonsVisibility();
		InitAddons();

		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eSilencerStatus == ALife::eAddonAttachable &&
		(m_sSilencerName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0)
		{
			Msg("ERROR: silencer addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonSilencer;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		(m_sGrenadeLauncherName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0)
		{
			Msg("ERROR: grenade launcher addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
		(m_sLaserName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) == 0)
		{
			Msg("ERROR: laser designator addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eTacticalTorchStatus == ALife::eAddonAttachable &&
		(m_sTacticalTorchName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) == 0)
		{
			Msg("ERROR: tactical torch addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else
		return inherited::Detach(item_section_name, b_spawn_item);;
}

void CWeaponMagazined::InitAddons()
{
	m_zoom_params.m_fIronSightZoomFactor = READ_IF_EXISTS(pSettings, r_float, cNameSect(), "ironsight_zoom_factor", 50.0f);

	SetAnimFlag(ANM_SHOT_AIM, "anm_shots_when_aim");
	SetAnimFlag(ANM_SHOT_AIM_GL, "anm_shots_w_gl_when_aim");

	m_weapon_attaches.clear();

	if (IsScopeAttached())
	{
		if (m_eScopeStatus == ALife::eAddonAttachable)
		{
			LoadCurrentScopeParams(GetScopeName().c_str());

			if (pSettings->line_exist(m_scopes[m_cur_scope], "bones"))
			{
				pcstr ScopeBone = pSettings->r_string(m_scopes[m_cur_scope], "bones");
				m_cur_scope_bone = ScopeBone;
			}

			if (m_sScopeAttachSection.size() && pSettings->line_exist(m_sScopeAttachSection, "attach_hud_visual"))
				WeaponAttach().CreateAttach(m_sScopeAttachSection, m_weapon_attaches);
		}
		else if (m_eScopeStatus == ALife::eAddonPermanent)
		{
			if (m_sScopeAttachSection.size() && pSettings->line_exist(m_sScopeAttachSection, "attach_hud_visual"))
				WeaponAttach().CreateAttach(m_sScopeAttachSection, m_weapon_attaches);
		}
	}
	else
	{
		if (m_sScopeAttachSection.size() && pSettings->line_exist(m_sScopeAttachSection, "attach_hud_visual"))
			WeaponAttach().RemoveAttach(m_sScopeAttachSection, m_weapon_attaches);

		if (m_UIScope)
			xr_delete(m_UIScope);

		if (bIsSecondVPZoomPresent())
			m_zoom_params.m_fSecondVPFovFactor = 0.0f;

		if (IsZoomEnabled())
			m_zoom_params.m_fIronSightZoomFactor = pSettings->r_float(cNameSect(), "scope_zoom_factor");
	}

	if (IsSilencerAttached()/* && SilencerAttachable() */)
	{
		m_sFlameParticlesCurrent = m_sSilencerFlameParticles;
		m_sSmokeParticlesCurrent = m_sSilencerSmokeParticles;
		m_sSndShotCurrent = "sndSilencerShot";

		//подсветка от выстрела
		LoadLights(*cNameSect(), "silencer_");
		ApplySilencerKoeffs();

		if (m_sSilencerAttachSection.size() && pSettings->line_exist(m_sSilencerAttachSection, "attach_hud_visual"))
			WeaponAttach().CreateAttach(m_sSilencerAttachSection, m_weapon_attaches);
	}
	else
	{
		if (m_sSilencerAttachSection.size() && pSettings->line_exist(m_sSilencerAttachSection, "attach_hud_visual"))
			WeaponAttach().RemoveAttach(m_sSilencerAttachSection, m_weapon_attaches);

		m_sFlameParticlesCurrent = m_sFlameParticles;
		m_sSmokeParticlesCurrent = m_sSmokeParticles;
		m_sSndShotCurrent = "sndShot";

		//подсветка от выстрела
		LoadLights(*cNameSect(), "");
		ResetSilencerKoeffs();
	}

	if (m_sGrenadeLauncherAttachSection.size() && pSettings->line_exist(m_sGrenadeLauncherAttachSection, "attach_hud_visual"))
	{
		if (IsGrenadeLauncherAttached())
			WeaponAttach().CreateAttach(m_sGrenadeLauncherAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sGrenadeLauncherAttachSection, m_weapon_attaches);
	}

	if (m_sLaserAttachSection.size() && pSettings->line_exist(m_sLaserAttachSection, "attach_hud_visual"))
	{
		if (IsLaserAttached())
			WeaponAttach().CreateAttach(m_sLaserAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sLaserAttachSection, m_weapon_attaches);
	}

	if (m_sTacticalTorchAttachSection.size() && pSettings->line_exist(m_sTacticalTorchAttachSection, "attach_hud_visual"))
	{
		if (IsTacticalTorchAttached())
			WeaponAttach().CreateAttach(m_sTacticalTorchAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sTacticalTorchAttachSection, m_weapon_attaches);
	}

	inherited::InitAddons();
}

void CWeaponMagazined::LoadSilencerKoeffs()
{
	if (m_eSilencerStatus == ALife::eAddonAttachable)
	{
		LPCSTR sect = m_sSilencerName.c_str();
		m_silencer_koef.hit_power = READ_IF_EXISTS(pSettings, r_float, sect, "bullet_hit_power_k", 1.0f);
		m_silencer_koef.hit_impulse = READ_IF_EXISTS(pSettings, r_float, sect, "bullet_hit_impulse_k", 1.0f);
		m_silencer_koef.bullet_speed = READ_IF_EXISTS(pSettings, r_float, sect, "bullet_speed_k", 1.0f);
		m_silencer_koef.fire_dispersion = READ_IF_EXISTS(pSettings, r_float, sect, "fire_dispersion_base_k", 1.0f);
		m_silencer_koef.cam_dispersion = READ_IF_EXISTS(pSettings, r_float, sect, "cam_dispersion_k", 1.0f);
		m_silencer_koef.cam_disper_inc = READ_IF_EXISTS(pSettings, r_float, sect, "cam_dispersion_inc_k", 1.0f);
	}

	clamp(m_silencer_koef.hit_power, 0.0f, 1.0f);
	clamp(m_silencer_koef.hit_impulse, 0.0f, 1.0f);
	clamp(m_silencer_koef.bullet_speed, 0.0f, 1.0f);
	clamp(m_silencer_koef.fire_dispersion, 0.0f, 3.0f);
	clamp(m_silencer_koef.cam_dispersion, 0.0f, 1.0f);
	clamp(m_silencer_koef.cam_disper_inc, 0.0f, 1.0f);
}