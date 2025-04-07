#include "StdAfx.h"
#include "WeaponMagazined.h"
#include "Inventory.h"
#include "Level.h"
#include "Actor.h"

// RELOADING MAGAZINE


void CWeaponMagazined::Reload()
{
	inherited::Reload();
	TryReload();
}


void CWeaponMagazined::OnEmptyClick()
{
	PlaySound("sndEmptyClick", get_LastFP());
}


void CWeaponMagazined::CheckMagazine()
{
	if (!ParentIsActor())
	{
		m_bNeedBulletInGun = false;
		return;
	}

	if (psWpnAnimsFlag.test(ANM_RELOAD_EMPTY) && iAmmoElapsed >= 1 && m_bNeedBulletInGun == false)
	{
		m_bNeedBulletInGun = true;
	}
	else if (psWpnAnimsFlag.test(ANM_RELOAD_EMPTY) && iAmmoElapsed == 0 && m_bNeedBulletInGun == true)
	{
		m_bNeedBulletInGun = false;
	}
}


// »спользуетс€ только при перезар€дке
void CWeaponMagazined::OnMotionMark(u32 state, const motion_marks& M)
{
	inherited::OnMotionMark(state, M);
	if (state == eReload)
	{
		u8 ammo_type = m_ammoType;
		int ae = CheckAmmoBeforeReload(ammo_type);

		if (ammo_type == m_ammoType)
		{
			//			Msg("Ammo elapsed: %d", iAmmoElapsed);
			ae += iAmmoElapsed;
		}

		last_hide_bullet = ae >= bullet_cnt ? bullet_cnt : bullet_cnt - ae - 1;

		//		Msg("Next reload: count %d with type %d", ae, ammo_type);

		HUD_VisualBulletUpdate();
	}
}

// ѕопыткаа перезар€дитьс€ 
bool CWeaponMagazined::TryReload()
{
	if (m_pInventory)
	{
		m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(m_ammoTypes[m_ammoType].c_str()));

		if (IsMisfire() && iAmmoElapsed)
		{
			SetPending(TRUE);
			SwitchState(eUnMisfire);
			return				true;
		}

		if (m_pCurrentAmmo || unlimited_ammo())
		{
			SetPending(TRUE);
			SwitchState(eReload);
			return				true;
		}
		else for (u8 i = 0; i < u8(m_ammoTypes.size()); ++i)
		{
			for (u32 i = 0; i < m_ammoTypes.size(); ++i)
			{
				m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if (m_pCurrentAmmo)
				{
					m_set_next_ammoType_on_reload = i;
					SetPending(TRUE);
					SwitchState(eReload);
					return				true;
				}
			}
		}

	}

	if (GetState() != eIdle)
		SwitchState(eIdle);

	return false;
}

// ћагазин пуст пытаемс€ перезагрузить 
void CWeaponMagazined::OnMagazineEmpty()
{
	if (GetState() == eIdle)
	{
		OnEmptyClick();
		return;
	}

	if (GetNextState() != eMagEmpty && GetNextState() != eReload)
	{
		SwitchState(eMagEmpty);
	}

	inherited::OnMagazineEmpty();
}

// ≈сть ли патроны какие небудь под текущий тип 
bool CWeaponMagazined::IsAmmoAvailable()
{
	if (smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(m_ammoTypes[m_ammoType].c_str())))
	{
		return true;
	}
	else
	{
		for (u32 i = 0; i < m_ammoTypes.size(); ++i)
		{
			if (smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(m_ammoTypes[i].c_str())))
				return true;
		}
	}
	return false;
}


// –азгружаем
void CWeaponMagazined::UnloadMagazine(bool spawn_ammo)
{
	last_hide_bullet = -1;
	HUD_VisualBulletUpdate();

	xr_map<LPCSTR, u16> l_ammo;

	while (!m_magazine.empty())
	{
		CCartridge& l_cartridge = m_magazine.back();
		xr_map<LPCSTR, u16>::iterator l_it;
		for (l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it)
		{
			if (!xr_strcmp(*l_cartridge.m_ammoSect, l_it->first))
			{
				++(l_it->second);
				break;
			}
		}

		if (l_it == l_ammo.end())
			l_ammo[*l_cartridge.m_ammoSect] = 1;

		m_magazine.pop_back();
		--iAmmoElapsed;
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	if (iAmmoElapsed < 0)
		iAmmoElapsed = 0;

	if (!spawn_ammo)
		return;

	xr_map<LPCSTR, u16>::iterator l_it;

	for (l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it)
	{
		if (m_pInventory)
		{
			CWeaponAmmo* l_pA = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(l_it->first));
			if (l_pA)
			{
				u16 l_free = l_pA->m_boxSize - l_pA->m_boxCurr;
				l_pA->m_boxCurr = l_pA->m_boxCurr + (l_free < l_it->second ? l_free : l_it->second);
				l_it->second = l_it->second - (l_free < l_it->second ? l_free : l_it->second);
			}
		}
		if (l_it->second && !unlimited_ammo())
		{
			if (OnServer())
				SpawnAmmo(l_it->second, l_it->first);
			else
			{
				NET_Packet	P;
				CGameObject::u_EventGen(P, GE_WPN_UNLOAD_AMMO, ID());
				P.w_u16(l_it->second);
				P.w_stringZ(l_it->first);
				CGameObject::u_EventSend(P);
			}
		}
	}

	if (iAmmoElapsed < 0)
		iAmmoElapsed = 0;

	SwitchState(eIdle);
}

//ѕровер€ем перед перезар€дкой
int CWeaponMagazined::CheckAmmoBeforeReload(u8& v_ammoType)
{
	if (m_set_next_ammoType_on_reload != undefined_ammo_type)
		v_ammoType = m_set_next_ammoType_on_reload;

	if (m_ammoTypes.size() <= v_ammoType)
	{
		return 0;
	}

	LPCSTR tmp_sect_name = m_ammoTypes[v_ammoType].c_str();

	if (!tmp_sect_name)
	{
		//		Msg("Sect name is wrong");
		return 0;
	}

	CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(tmp_sect_name));

	if (!ammo && !m_bLockType)
	{
		for (u8 i = 0; i < u8(m_ammoTypes.size()); ++i)
		{
			//проверить патроны всех подход€щих типов
			ammo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(m_ammoTypes[i].c_str()));
			if (ammo)
			{
				v_ammoType = i;
				break;
			}
		}
	}

	//	Msg("Ammo type %d", v_ammoType);

	return GetAmmoCount(v_ammoType);

}

// ѕерезар€жаем
void CWeaponMagazined::ReloadMagazine()
{
	if (IsMisfire())
		bMisfire = false;
	m_BriefInfo_CalcFrame = 0;

#ifdef USE_CLIENT_SIDE_WEAPONS
	if (smart_cast<CActor*>(H_Parent()) && Level().CurrentControlEntity() != H_Parent())
		return;

	// —кипаем другие клиенты от перезар€дки (Ќам прелет€т патроны и так)
	if (OnClient() && !smart_cast<CActor*>(H_Parent()))
		return;
	//устранить осечку при перезар€дке
// 	Msg("Start Reloading ");
#else
	if (OnClient())
		return;

	if (IsMisfire())
		bMisfire = false;
#endif	

	{
		if (!m_bLockType)
			m_pCurrentAmmo = NULL;
		if (!m_pInventory)
			return;
		if (m_set_next_ammoType_on_reload != undefined_ammo_type)
		{
			m_ammoType = m_set_next_ammoType_on_reload;
			m_set_next_ammoType_on_reload = undefined_ammo_type;
		}


		if (!unlimited_ammo())
		{
			if (m_ammoTypes.size() <= m_ammoType)
				return;
			LPCSTR tmp_sect_name = m_ammoTypes[m_ammoType].c_str();
			if (!tmp_sect_name)
				return;
			//попытатьс€ найти в инвентаре патроны текущего типа 
			m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(tmp_sect_name));

			if (!m_pCurrentAmmo && !m_bLockType)
			{
				for (u8 i = 0; i < u8(m_ammoTypes.size()); ++i)
				{
					//проверить патроны всех подход€щих типов
					m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(m_ammoTypes[i].c_str()));
					if (m_pCurrentAmmo)
					{
						m_ammoType = i;
						break;
					}
				}
			}


			//нет патронов дл€ перезар€дки
			if (!m_pCurrentAmmo)
				return;
		}


		//ћодернизируем проверку на соотвествие патронов, будем провер€ть так же последний патрон
		//разр€дить магазин, если загружаем патронами другого типа
		if (!m_bLockType && !m_magazine.empty() && (!m_pCurrentAmmo || xr_strcmp(m_pCurrentAmmo->cNameSect(), *m_magazine.front().m_ammoSect)))
			UnloadMagazine();

		VERIFY((u32)iAmmoElapsed == m_magazine.size());

		if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
			m_DefaultCartridge.Load(m_ammoTypes[m_ammoType].c_str(), m_ammoType);

		CCartridge l_cartridge = m_DefaultCartridge;
		while (iAmmoElapsed < iMagazineSize)
		{
			if (!unlimited_ammo())
			{
				if (!m_pCurrentAmmo->Get(l_cartridge))
					break;
			}
			++iAmmoElapsed;
			l_cartridge.m_LocalAmmoType = m_ammoType;

			m_magazine.push_back(l_cartridge);
		}

		VERIFY((u32)iAmmoElapsed == m_magazine.size());

		if (m_pCurrentAmmo)
		{
			// Msg("Send AmmoSize: %d", m_pCurrentAmmo->m_boxCurr);
			NET_Packet					P;
			Game().u_EventGen(P, GE_WPN_UPDATE_AMMO, ID());
			P.w_u16(m_pCurrentAmmo->ID());
			P.w_u16(m_pCurrentAmmo->m_boxCurr);
			Game().u_EventSend(P);
		}

		if (iMagazineSize > iAmmoElapsed)
		{
			m_bLockType = true;
			ReloadMagazine();
			m_bLockType = false;
		}
	}

}
