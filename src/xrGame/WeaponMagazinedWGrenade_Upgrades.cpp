#include "stdafx.h"
#include "WeaponMagazinedWGrenade.h"
#include "Inventory.h"

// Upgrades


bool CWeaponMagazinedWGrenade::install_upgrade_ammo_class(LPCSTR section, bool test)
{
	LPCSTR str;

	bool result = process_if_exists(section, "ammo_mag_size", &CInifile::r_s32, iMagazineSize2, test);
	iMagazineSize = m_bGrenadeMode ? 1 : iMagazineSize2;

	//	ammo_class = ammo_5.45x39_fmj, ammo_5.45x39_ap  // name of the ltx-section of used ammo
	bool result2 = process_if_exists_set(section, "ammo_class", &CInifile::r_string, str, test);
	if (result2 && !test)
	{
		xr_vector<shared_str>& ammo_types = m_bGrenadeMode ? m_ammoTypes2 : m_ammoTypes;
		ammo_types.clear();
		for (int i = 0, count = _GetItemCount(str); i < count; ++i)
		{
			string128						ammo_item;
			_GetItem(str, i, ammo_item);
			ammo_types.push_back(ammo_item);
		}

		m_ammoType = 0;
		m_ammoType2 = 0;
	}
	result |= result2;

	return result2;
}

bool CWeaponMagazinedWGrenade::install_upgrade_impl(LPCSTR section, bool test)
{
	LPCSTR str;
	bool result = inherited::install_upgrade_impl(section, test);

	//	grenade_class = ammo_vog-25, ammo_vog-25p          // name of the ltx-section of used grenades
	bool result2 = process_if_exists_set(section, "grenade_class", &CInifile::r_string, str, test);
	if (result2 && !test)
	{
		xr_vector<shared_str>& ammo_types = !m_bGrenadeMode ? m_ammoTypes2 : m_ammoTypes;
		ammo_types.clear();
		for (int i = 0, count = _GetItemCount(str); i < count; ++i)
		{
			string128						ammo_item;
			_GetItem(str, i, ammo_item);
			ammo_types.push_back(ammo_item);
		}

		m_ammoType = 0;
		m_ammoType2 = 0;
	}
	result |= result2;

	result |= process_if_exists(section, "launch_speed", &CInifile::r_float, m_fLaunchSpeed, test);

	result2 = process_if_exists_set(section, "snd_shoot_grenade", &CInifile::r_string, str, test);
	if (result2 && !test) { m_sounds.LoadSound(section, "snd_shoot_grenade", "sndShotG", false, m_eSoundShot); }
	result |= result2;

	result2 = process_if_exists_set(section, "snd_reload_grenade", &CInifile::r_string, str, test);
	if (result2 && !test) { m_sounds.LoadSound(section, "snd_reload_grenade", "sndReloadG", true, m_eSoundReload); }
	result |= result2;

	result2 = process_if_exists_set(section, "snd_switch", &CInifile::r_string, str, test);
	if (result2 && !test) { m_sounds.LoadSound(section, "snd_switch", "sndSwitch", true, m_eSoundReload); }
	result |= result2;

	return result;
}

void CWeaponMagazinedWGrenade::net_Spawn_install_upgrades(Upgrades_type saved_upgrades)
{
	// do not delete this
	// this is intended behaviour
}


#include "string_table.h"
bool CWeaponMagazinedWGrenade::GetBriefInfo(II_BriefInfo& info)
{
	VERIFY(m_pInventory);
	/*
		if(!inherited::GetBriefInfo(info))
			return false;
	*/
	string32	int_str, fire_mode, ammo = "";
	int	ae = GetAmmoElapsed();
	xr_sprintf(int_str, "%d", ae);
	info.cur_ammo._set(int_str);
	info.fire_mode._set("");

	if (bHasBulletsToHide && !m_bGrenadeMode)
	{
		last_hide_bullet = ae >= bullet_cnt ? bullet_cnt : bullet_cnt - ae - 1;
		if (ae == 0) last_hide_bullet = -1;
	}

	if (HasFireModes())
	{
		if (m_iQueueSize == WEAPON_ININITE_QUEUE)
			info.fire_mode._set("A");
		else
		{
			xr_sprintf(int_str, "%d", m_iQueueSize);
			info.fire_mode._set(int_str);
		}
	}
	if (m_pInventory->ModifyFrame() <= m_BriefInfo_CalcFrame)
		return false;

	GetSuitableAmmoTotal();

	u32 at_size = m_bGrenadeMode ? m_ammoTypes2.size() : m_ammoTypes.size();
	if (unlimited_ammo() || at_size == 0)
	{
		info.fmj_ammo._set("--");
		info.ap_ammo._set("--");
	}
	else
	{
		info.fmj_ammo._set("");
		info.ap_ammo._set("");

		if (at_size >= 1 && at_size < 3)
		{
			xr_sprintf(ammo, "%d", m_bGrenadeMode ? GetAmmoCount2(0) : GetAmmoCount(0));
			info.fmj_ammo._set(ammo);
		}
		if (at_size == 2)
		{
			xr_sprintf(ammo, "%d", m_bGrenadeMode ? GetAmmoCount2(1) : GetAmmoCount(1));
			info.ap_ammo._set(ammo);
		}
		if (at_size >= 3)
		{
			xr_sprintf(ammo, "%d", m_bGrenadeMode ? GetAmmoCount2(m_ammoType) : GetAmmoCount(m_ammoType));
			info.fmj_ammo._set(ammo);
			u8 m = 0;
			u64 ap = 0;
			while (m < at_size)
			{
				if (m != m_ammoType)
					ap += m_bGrenadeMode ? GetAmmoCount2(m) : GetAmmoCount(m);
				m++;
			}
			xr_sprintf(ammo, "%d", ap);
			info.ap_ammo._set(ammo);
		}
	}

	if (ae != 0 && m_magazine.size() != 0)
	{
		LPCSTR ammo_type = m_ammoTypes[m_magazine.back().m_LocalAmmoType].c_str();
		info.name._set(CStringTable().translate(pSettings->r_string(ammo_type, "inv_name_short")));
		info.icon._set(ammo_type);
	}
	else
	{
		LPCSTR ammo_type = m_ammoTypes[m_ammoType].c_str();
		info.name._set(CStringTable().translate(pSettings->r_string(ammo_type, "inv_name_short")));
		info.icon._set(ammo_type);
	}

	if (!IsGrenadeLauncherAttached())
	{
		info.grenade = "";
		return false;
	}

	int total2 = m_bGrenadeMode ? GetAmmoCount(0) : GetAmmoCount2(0);
	if (unlimited_ammo())
		xr_sprintf(int_str, "--");
	else
	{
		if (total2)
			xr_sprintf(int_str, "%d", total2);
		else
			xr_sprintf(int_str, "X");
	}
	info.grenade = int_str;

	return true;
}