#include "StdAfx.h"
#include "Weapon.h"
#include "player_hud.h"
#include "Actor.h"
// #include "ActorNightVision.h"
#include "Inventory.h"

// CAN KILLS CHECK (FOR START FIRE)

bool CWeapon::can_kill() const
{
	if (GetSuitableAmmoTotal(true) || m_ammoTypes.empty())
		return				(true);

	return					(false);
}

CInventoryItem* CWeapon::can_kill(CInventory* inventory) const
{
	if (GetAmmoElapsed() || m_ammoTypes.empty())
		return				(const_cast<CWeapon*>(this));

	TIItemContainer::iterator I = inventory->m_all.begin();
	TIItemContainer::iterator E = inventory->m_all.end();
	for (; I != E; ++I) {
		CInventoryItem* inventory_item = smart_cast<CInventoryItem*>(*I);
		if (!inventory_item)
			continue;

		xr_vector<shared_str>::const_iterator	i = std::find(m_ammoTypes.begin(), m_ammoTypes.end(), inventory_item->object().cNameSect());
		if (i != m_ammoTypes.end())
			return			(inventory_item);
	}

	return					(0);
}

const CInventoryItem* CWeapon::can_kill(const xr_vector<const CGameObject*>& items) const
{
	if (m_ammoTypes.empty())
		return				(this);

	xr_vector<const CGameObject*>::const_iterator I = items.begin();
	xr_vector<const CGameObject*>::const_iterator E = items.end();
	for (; I != E; ++I) {
		const CInventoryItem* inventory_item = smart_cast<const CInventoryItem*>(*I);
		if (!inventory_item)
			continue;

		xr_vector<shared_str>::const_iterator	i = std::find(m_ammoTypes.begin(), m_ammoTypes.end(), inventory_item->object().cNameSect());
		if (i != m_ammoTypes.end())
			return			(inventory_item);
	}

	return					(0);
}

bool CWeapon::ready_to_kill() const
{
	return					(
		!IsMisfire() &&
		((GetState() == eIdle) || (GetState() == eFire) || (GetState() == eFire2)) &&
		GetAmmoElapsed()
		);
}

// IS STATE


BOOL CWeapon::IsUpdating()
{
	bool bIsActiveItem = m_pInventory && m_pInventory->ActiveItem() == this;
	return bIsActiveItem || bWorking;// || IsPending() || getVisible();
}


bool CWeapon::IsNecessaryItem(const shared_str& item_sect)
{
	return (std::find(m_ammoTypes.begin(), m_ammoTypes.end(), item_sect) != m_ammoTypes.end());
}

bool CWeapon::IsNecessaryItem(const shared_str& item_sect, xr_vector<shared_str> item)
{
	return (std::find(item.begin(), item.end(), item_sect) != item.end());
}


bool CWeapon::IsPartlyReloading()
{
	return (m_set_next_ammoType_on_reload == undefined_ammo_type && GetAmmoElapsed() > 0 && !IsMisfire());
}

bool CWeapon::IsMisfireNow()
{
	return IsMisfire();
}

bool CWeapon::IsMagazineEmpty()
{
	return IsEmptyMagazine();
}

BOOL CWeapon::IsMisfire() const
{
	return bMisfire;
}

BOOL CWeapon::IsEmptyMagazine() const
{
	return (iAmmoElapsed == 0);
}

bool CWeapon::IsHudModeNow()
{
	return (HudItemData() != NULL);
}


bool CWeapon::MovingAnimAllowedNow()
{
	return !IsZoomed();
}

bool CWeapon::AllowBore()
{
	return true;
}



// SWITCH STATE


void CWeapon::SwitchLaser(bool on)
{
	if (!has_laser || !IsLaserAttached())
		return;

	if (on)
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonLaserOn;

		if (isHUDAnimationExist("anm_laser_on"))
			SwitchState(eLaserSwitch);
	}
	else
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonLaserOn;

		if (isHUDAnimationExist("anm_laser_off"))
			SwitchState(eLaserSwitch);
	}
}

void CWeapon::SwitchFlashlight(bool on)
{
	if (!has_flashlight || !IsTacticalTorchAttached())
		return;

	if (on)
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;

		if (isHUDAnimationExist("anm_torch_on"))
			SwitchState(eFlashlightSwitch);
	}
	else
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;

		if (isHUDAnimationExist("anm_torch_off"))
			SwitchState(eFlashlightSwitch);
	}
}
void CWeapon::SwitchZoomMode()
{
	!m_bAltZoomActive ? m_bAltZoomActive = true : m_bAltZoomActive = false;
}


// Switch NightVision NIGHT VISION
void CWeapon::EnableActorNVisnAfterZoom()
{
	CActor* pA = smart_cast<CActor*>(H_Parent());
	if (IsGameTypeSingle() && !pA)
		pA = g_actor;

	//if (pA)
	//{
	//	pA->SwitchNightVision(true, false, false);
	//	pA->GetNightVision()->PlaySounds(CNightVisionEffector::eIdleSound);
	//}
}

// UPDATE METHODS FOR UPDATE_CL

#include "WeaponBinocularsVision.h"
void CWeapon::UpdateNightVision()
{
	// if (m_zoom_params.m_pNight_vision && !need_renderable())
	// {
	// 	if (!m_zoom_params.m_pNight_vision->IsActiveWeapon())
	// 	{
	// 		CActor* pA = smart_cast<CActor*>(H_Parent());
	// 		R_ASSERT(pA);
	// 		if (pA->GetNightVisionStatus())
	// 		{
	// 			m_bRememberActorNVisnStatus = pA->GetNightVisionStatus();
	// 			pA->SwitchNightVision(false, false, false);
	// 		}
	// 		m_zoom_params.m_pNight_vision->StartForScope(m_zoom_params.m_sUseZoomPostprocess, pA, false);
	// 	}
	// 
	// }
	// else if (m_bRememberActorNVisnStatus)
	// {
	// 	m_bRememberActorNVisnStatus = false;
	// 	EnableActorNVisnAfterZoom();
	// }
	// 
	// if (m_zoom_params.m_pVision)
	// 	m_zoom_params.m_pVision->Update();
}

extern u32 hud_adj_mode;

void CWeapon::UpdateBoreState()
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (pActor && !pActor->AnyMove() && this == pActor->inventory().ActiveItem())
	{
		if (hud_adj_mode == 0 &&
			GetState() == eIdle &&
			(Device.dwTimeGlobal - m_dw_curr_substate_time > 20000) &&
			!IsZoomed() &&
			g_player_hud->attached_item(1) == NULL)
		{
			if (AllowBore())
				SwitchState(eBore);

			ResetSubStateTime();
		}
	}
}


// UPDATE HUD ATTACHES (IN XFORM FOR WORLD MODEL)

void CWeapon::render_hud_mode()
{
	RenderLight();

	for (auto mesh : m_weapon_attaches)
		mesh->RenderAttach();
}

