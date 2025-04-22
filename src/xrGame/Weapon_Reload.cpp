#include "StdAfx.h"
#include "Weapon.h"
#include "Level.h"
#include "Inventory.h"
#include "Actor.h"
#include "ai_object_location.h"
#include "InventoryOwner.h"
#include "inventory_item_impl.h"

// SPAWN AMMO 
void CWeapon::SpawnAmmo(u32 boxCurr, LPCSTR ammoSect, u32 ParentID)
{
	if (!m_ammoTypes.size())			return;
	if (OnClient())					return;
	m_bAmmoWasSpawned = true;

	int l_type = 0;
	l_type %= m_ammoTypes.size();

	if (!ammoSect) ammoSect = m_ammoTypes[l_type].c_str();

	++l_type;
	l_type %= m_ammoTypes.size();

	CSE_Abstract* D = F_entity_Create(ammoSect);

	{
		CSE_ALifeItemAmmo* l_pA = smart_cast<CSE_ALifeItemAmmo*>(D);
		R_ASSERT(l_pA);
		l_pA->m_boxSize = (u16)pSettings->r_s32(ammoSect, "box_size");
		D->s_name = ammoSect;
		D->set_name_replace("");
		//.		D->s_gameid					= u8(GameID());
		D->s_RP = 0xff;
		D->ID = 0xffff;
		if (ParentID == 0xffffffff)
			D->ID_Parent = (u16)H_Parent()->ID();
		else
			D->ID_Parent = (u16)ParentID;

		D->ID_Phantom = 0xffff;
		D->s_flags.assign(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime = 0;
		l_pA->m_tNodeID = g_dedicated_server ? u32(-1) : ai_location().level_vertex_id();

		if (boxCurr == 0xffffffff)
			boxCurr = l_pA->m_boxSize;

		while (boxCurr)
		{
			l_pA->a_elapsed = (u16)(boxCurr > l_pA->m_boxSize ? l_pA->m_boxSize : boxCurr);
			NET_Packet				P;
			D->Spawn_Write(P, TRUE);
			Level().Send(P, net_flags(TRUE));

			if (boxCurr > l_pA->m_boxSize)
				boxCurr -= l_pA->m_boxSize;
			else
				boxCurr = 0;
		}
	}
	F_entity_Destroy(D);
}


void CWeapon::SetAmmoElapsed(int ammo_count)
{
	iAmmoElapsed = ammo_count;

	u32 uAmmo = u32(iAmmoElapsed);

	if (uAmmo != m_magazine.size())
	{
		if (uAmmo > m_magazine.size())
		{
			CCartridge			l_cartridge;
			l_cartridge.Load(m_ammoTypes[m_ammoType].c_str(), m_ammoType);
			while (uAmmo > m_magazine.size())
				m_magazine.push_back(l_cartridge);
		}
		else
		{
			while (uAmmo < m_magazine.size())
			{
				if (!m_magazine.empty())
					m_magazine.pop_back();
			}
		};
	};
}

bool CWeapon::SwitchAmmoType(u32 flags)
{
	Msg("Switch Ammo Type");
	if (IsPending() || !(flags & CMD_START))
		return false;

	u8 l_newType = m_ammoType;
	bool b1, b2;
	do
	{
		l_newType = u8((u32(l_newType + 1)) % m_ammoTypes.size());
		b1 = (l_newType != m_ammoType);
		b2 = unlimited_ammo() ? false : (!m_pInventory->GetAny(m_ammoTypes[l_newType].c_str()));
	} while (b1 && b2);

	if (l_newType != m_ammoType)
	{
		m_set_next_ammoType_on_reload = l_newType;
  		Reload();
 	}
	return true;
}

void CWeapon::Reload()
{
	OnZoomOut();

	if (ParentIsActor() && !GameConstants::GetReloadIfSprint())
	{
		Actor()->StopSprint();
		// Actor()->m_iTrySprintCounter = 0;
	}
}


int CWeapon::GetSuitableAmmoTotal(bool use_item_to_spawn) const
{
	int ae_count = iAmmoElapsed;
	if (!m_pInventory)
	{
		return ae_count;
	}

	//чтоб не делать лишних пересчетов
	if (m_pInventory->ModifyFrame() <= m_BriefInfo_CalcFrame)
	{
		return ae_count + m_iAmmoCurrentTotal;
	}
	m_BriefInfo_CalcFrame = Device.dwFrame;

	m_iAmmoCurrentTotal = 0;
	for (u8 i = 0; i < u8(m_ammoTypes.size()); ++i)
	{
		m_iAmmoCurrentTotal += GetAmmoCount_forType(m_ammoTypes[i]);

		if (!use_item_to_spawn)
		{
			continue;
		}

		if (!inventory_owner().item_to_spawn())
		{
			continue;
		}
		m_iAmmoCurrentTotal += inventory_owner().ammo_in_box_to_spawn();
	}
	return ae_count + m_iAmmoCurrentTotal;
}

int CWeapon::GetAmmoCount(u8 ammo_type) const
{
	VERIFY(m_pInventory);
	R_ASSERT(ammo_type < m_ammoTypes.size());

	return GetAmmoCount_forType(m_ammoTypes[ammo_type]);
}

int CWeapon::GetAmmoCount_forType(shared_str const& ammo_type) const
{
	int res = 0;

	TIItemContainer::iterator itb = m_pInventory->m_belt.begin();
	TIItemContainer::iterator ite = m_pInventory->m_belt.end();
	for (; itb != ite; ++itb)
	{
		CWeaponAmmo* pAmmo = smart_cast<CWeaponAmmo*>(*itb);
		if (pAmmo && (pAmmo->cNameSect() == ammo_type))
		{
			res += pAmmo->m_boxCurr;
		}
	}

	itb = m_pInventory->m_ruck.begin();
	ite = m_pInventory->m_ruck.end();
	for (; itb != ite; ++itb)
	{
		CWeaponAmmo* pAmmo = smart_cast<CWeaponAmmo*>(*itb);
		if (pAmmo && (pAmmo->cNameSect() == ammo_type))
		{
			res += pAmmo->m_boxCurr;
		}
	}
	return res;
}

float CWeapon::GetConditionMisfireProbability() const
{
	// modified by Peacemaker [17.10.08]
	//	if(GetCondition() > 0.95f) 
	//		return 0.0f;
	if (GetCondition() > misfireStartCondition)
		return 0.0f;
	if (GetCondition() < misfireEndCondition)
		return misfireEndProbability;
	//	float mis = misfireProbability+powf(1.f-GetCondition(), 3.f)*misfireConditionK;
	float mis = misfireStartProbability + (
		(misfireStartCondition - GetCondition()) *				// condition goes from 1.f to 0.f
		(misfireEndProbability - misfireStartProbability) /		// probability goes from 0.f to 1.f
		((misfireStartCondition == misfireEndCondition) ?		// !!!say "No" to devision by zero
			misfireStartCondition :
			(misfireStartCondition - misfireEndCondition))
		);
	clamp(mis, 0.0f, 0.99f);
	return mis;
}

BOOL CWeapon::CheckForMisfire()
{
	CActor*  a = smart_cast<CActor*> (H_Parent());
	if (H_Parent() == Level().CurrentControlEntity() && a)
	{
		float rnd = ::Random.randF(0.f, 1.f);
		float mp = GetConditionMisfireProbability();
		if (rnd < mp)
		{
			FireEnd();

			bMisfire = true;
			SwitchState(eMisfire);

			return TRUE;
		}
		else
 			return FALSE;
 	}
 
	if (!a)
	{
		float rnd = ::Random.randF(0.f, 1.f);
		float mp = GetConditionMisfireProbability();
		if (rnd < mp)
		{
			FireEnd();

			bMisfire = true;
			SwitchState(eMisfire);

			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

void CWeapon::OnMagazineEmpty()
{
	VERIFY((u32)iAmmoElapsed == m_magazine.size());
}


bool CWeapon::unlimited_ammo()
{
	if (smart_cast<CActor*>(H_Parent()))
	{
		if (m_pInventory)
		{
			return inventory_owner().unlimited_ammo() && m_DefaultCartridge.m_flags.test(CCartridge::cfCanBeUnlimited);
		}
		else
			return false;
	}
	else
		return m_DefaultCartridge.m_flags.test(CCartridge::cfCanBeUnlimited);
};

float CWeapon::GetMagazineWeight(const decltype(CWeapon::m_magazine)& mag) const
{
	float res = 0;
	const char* last_type = nullptr;
	float last_ammo_weight = 0;
	for (auto& c : mag)
	{
		// Usually ammos in mag have same type, use this fact to improve performance
		if (last_type != c.m_ammoSect.c_str())
		{
			last_type = c.m_ammoSect.c_str();
			last_ammo_weight = c.Weight();
		}
		res += last_ammo_weight;
	}
	return res;
}

// WEIGHT CALCULATOR
float CWeapon::Weight() const
{
	float res = CInventoryItemObject::Weight();
	if (IsGrenadeLauncherAttached() && GetGrenadeLauncherName().size())
	{
		res += pSettings->r_float(GetGrenadeLauncherName(), "inv_weight");
	}
	if (IsScopeAttached() && m_scopes.size())
	{
		res += pSettings->r_float(GetScopeName(), "inv_weight");
	}
	if (IsSilencerAttached() && GetSilencerName().size()) {
		res += pSettings->r_float(GetSilencerName(), "inv_weight");
	}
	if (IsLaserAttached() && GetLaserName().size()) {
		res += pSettings->r_float(GetLaserName(), "inv_weight");
	}
	if (IsTacticalTorchAttached() && GetTacticalTorchName().size()) {
		res += pSettings->r_float(GetTacticalTorchName(), "inv_weight");
	}

	res += GetMagazineWeight(m_magazine);

	return res;
}