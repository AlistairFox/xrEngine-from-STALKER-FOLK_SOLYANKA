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

#pragma warning(disable:4238)

void CSE_ALifeObject::spawn_supplies(LPCSTR ini_string)
{
	if (OnClient())
		return;

	if (!ini_string)
		return;

	if (!xr_strlen(ini_string))
		return;

	CInifile* ini = new CInifile(&IReader((void*)(ini_string), xr_strlen(ini_string)), FS.get_path("$game_config$")->m_Path);  
	if ( ini->section_exist("box_items_type") && pSettings->section_exist("mp_spawn_boxes") )
	{
		u32 type = ini->r_u32("box_items_type", "type");

		string128 type_id;
		sprintf(type_id, "type_%u", type);
 
		if (pSettings->line_exist("mp_spawn_boxes", type_id))
		{
 			shared_str SECT = pSettings->r_string("mp_spawn_boxes", type_id);
			Msg("Read Section to Spawn[%s] : %s", type_id, *SECT);

			if (pSettings->section_exist(SECT.c_str()))
			{
				for (int IDLine = 0; IDLine < pSettings->line_count(SECT); IDLine++)
				{
					
					LPCSTR m_spawn, mV;
					pSettings->r_line(SECT, IDLine, &m_spawn, &mV);

					float m_chance = 1;
					float m_cond = 1;

					if (_GetItemCount(mV) == 2)
						sscanf(mV, "%f, %f", &m_chance, &m_cond);
					else
						sscanf(mV, "%f", &m_chance);
					
					Msg("Read Spawn Item : %s, %f, %f", m_spawn, m_chance, m_cond);
					if (pSettings->section_exist(m_spawn))
					{
						if (randF(1.f) < m_chance)
						{
							CSE_Abstract* E = alife().spawn_item(m_spawn, o_Position, m_tNodeID, m_tGraphID, ID);
							CSE_ALifeInventoryItem* IItem = smart_cast<CSE_ALifeInventoryItem*>(E);
							if (IItem)
								IItem->m_fCondition = m_cond;
						}
					}
				}
			}
			else
			{
				Msg("Reading Unknown Section : %s", *SECT);
			}
			
		}
		else
		{
			Msg("cant find line_exist : %s", type_id);
		}
	}
	else
	{
		// Msg("Section not Exist: (box_items_type) %u, (mp_spawn_boxes) %u", ini->section_exist("box_items_type"), pSettings->section_exist("mp_spawn_boxes"));
	}

	if (ini->section_exist("spawn"))
	{
		// Msg("Spawn Reding from: %s : SelfID: %u", ini->fname(), this->ID );

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
				if (!j)	
					j = 1;
			
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
				if (randF(1.f) < p) 
				{
					CSE_Abstract* E = alife().spawn_item(N, o_Position, m_tNodeID, m_tGraphID, ID);
					// Msg("Inventory[%s] Spawn Supply [%s] alife_item[%p]", this->name_replace(), N, E);
 
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
}

bool CSE_ALifeObject::keep_saved_data_anyway() const
{
	return			(false);
}
