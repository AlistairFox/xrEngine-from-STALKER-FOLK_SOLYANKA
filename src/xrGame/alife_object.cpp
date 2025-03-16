////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_object.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife object class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife.h"
#include "alife_simulator.h"
#include "xrServer_Objects_ALife_Items.h"
#include "Level.h"

void CSE_ALifeObject::spawn_supplies		()
{
	spawn_supplies(*m_ini_string);
}

void CSE_ALifeObject::spawn_supplies(LPCSTR ini_string)
{
	if (OnClient())
		return;

	if (!ini_string)
		return;

	if (!xr_strlen(ini_string))
		return;

#pragma warning(push)
#pragma warning(disable:4238)
	CInifile* ini = new CInifile(&IReader((void*)(ini_string), xr_strlen(ini_string)), FS.get_path("$game_config$")->m_Path);
#pragma warning(pop)

	string_path tmp = { 0 };
	string_path tt;

	if (smart_cast<CSE_ALifeInventoryBox*> (this))
	{
		sprintf_s(tt, "inventory_spawns\\%s.ltx", this->name_replace());
		FS.update_path(tmp, "$users_data_root$", tt);
 		if (FS.exist(tmp))
		{
			ini = new CInifile(tmp, true);
		}
	}
	 
	if (ini->section_exist("spawn"))
	{
		Msg("Spawn Reding from: %s : SelfID: %u", ini->fname(), this->ID );

		LPCSTR					N, V;
		float					p;
		for (u32 k = 0, j; ini->r_line("spawn", k, &N, &V); k++)
		{
			VERIFY(xr_strlen(N));

			if (!pSettings->section_exist(N))
			{
				Msg("[CGameObject] Cant spawn [spawn]: %s", N);
				continue;
			}
			/// Msg("Dynamic Object [%s] SpawnSupply [%s]", this->name_replace(), N);

			float f_cond = 1.0f;
			bool bScope = false;
			bool bSilencer = false;
			bool bLauncher = false;

			j = 1;
			p = 1.f;

			if (V && xr_strlen(V))
			{
				string64			buf;
				j = atoi(_GetItem(V, 0, buf));
				if (!j)		j = 1;

				bScope = (NULL != strstr(V, "scope"));
				bSilencer = (NULL != strstr(V, "silencer"));
				bLauncher = (NULL != strstr(V, "launcher"));
				//probability
				if (NULL != strstr(V, "prob="))
					p = (float)atof(strstr(V, "prob=") + 5);

				if (fis_zero(p))
					p = 1.0f;
				if (NULL != strstr(V, "cond="))
					f_cond = (float)atof(strstr(V, "cond=") + 5);
			}

			for (u32 i = 0; i < j; ++i)
			{
				if (randF(1.f) < p) {
					CSE_Abstract* E = alife().spawn_item(N, o_Position, m_tNodeID, m_tGraphID, ID);

					//подсоединить аддоны к оружию, если включены соответствующие флажки
					CSE_ALifeItemWeapon* W = smart_cast<CSE_ALifeItemWeapon*>(E);
					if (W)
					{
						if (W->m_scope_status == ALife::eAddonAttachable)
							W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonScope, bScope);
						if (W->m_silencer_status == ALife::eAddonAttachable)
							W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonSilencer, bSilencer);
						if (W->m_grenade_launcher_status == ALife::eAddonAttachable)
							W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher, bLauncher);
					}
					CSE_ALifeInventoryItem* IItem = smart_cast<CSE_ALifeInventoryItem*>(E);
					if (IItem)
						IItem->m_fCondition = f_cond;
				}
			}
		}
	}


	if (xr_strlen(tmp) > 4)
	{
		ini->save_as(tmp);
	}
}

bool CSE_ALifeObject::keep_saved_data_anyway() const
{
	return			(false);
}
