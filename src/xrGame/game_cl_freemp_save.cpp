#include "stdafx.h"
#include "game_cl_freemp.h"
#include "../jsonxx/jsonxx.h"
using namespace jsonxx;
#include <fstream>;
#include <istream>;

#include "actor_mp_client.h"
#include "Inventory.h"
#include "inventory_item.h"
#include "Weapon.h"
#include "ActorHelmet.h"
#include "CustomOutfit.h"
#include "CustomDetector.h"

void game_cl_freemp::save_player(game_PlayerState* cl)
{
	game_PlayerState* ps = cl;
	if (!ps || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) return;

	CActor* pActor = smart_cast<CActor*>(Level().Objects.net_Find(ps->GameID));
	if (!pActor || !pActor->g_Alive()) return;

	TIItemContainer items;

	pActor->inventory().AddAvailableItems(items, false);

	xr_vector<PIItem>::const_iterator itemStart = items.begin();
	xr_vector<PIItem>::const_iterator itemEnd = items.end();

	Object json;
	
	Object jsonConsumable;
	Array jsonCondition;
	Array jsonInSlots;


	for (; itemStart != itemEnd; itemStart++)
	{
		PIItem item = (*itemStart);

		if (!xr_strcmp(item->m_section_id.c_str(), "mp_players_rukzak"))
			continue;

		Object table;

		string2048 updates = {0};
		
		int prop_count = 0;
		for (auto upgrade : item->upgardes() )
		{
			if (prop_count > 0)
				xr_strcat(updates, sizeof(updates), ", ");

			xr_strcat(updates, sizeof(updates), upgrade.c_str());
			prop_count += 1;
		}
 
		table << "Section" << String(item->m_section_id.c_str());

		if (item->GetCondition() < 1)
			table << "condition" << Value(item->GetCondition());

		if (item->has_any_upgrades())
			table << "upgrades" << String(updates);

		if (item->cast_weapon())
		{
			CWeapon* weap = item->cast_weapon();
			table << "ammo_count" << Value(weap->GetAmmoElapsed());
			table << "ammo_type" << Value(weap->m_ammoType);
			table << "cur_scope" << Value(weap->m_cur_scope);
			table << "addon_State" << Value(weap->GetAddonsState());
		}

		if (item->cast_weapon_ammo())
		{
			CWeaponAmmo* ammo = item->cast_weapon_ammo();
			table << "count" << Number(ammo->m_boxCurr);
		}		
		
		if (item->m_ItemCurrPlace.type == eItemPlaceSlot || item->m_ItemCurrPlace.type == eItemPlaceBelt)
		{
 			table << "slot_value" << Number(item->m_ItemCurrPlace.value);
			jsonInSlots << table;
		}
		else 
		if (item->cast_weapon() || smart_cast<CCustomOutfit*>(item) || smart_cast<CHelmet*>(item) || item->cast_weapon_ammo())
		{
			jsonCondition << table;
		}
		else
		{
			if (jsonConsumable.has<Number>(item->m_section_id.c_str()))
			{
				int count = jsonConsumable.get<Number>(item->m_section_id.c_str());
				count += 1;
				jsonConsumable << item->m_section_id.c_str() << Number(count);
			}
			else
				jsonConsumable << item->m_section_id.c_str() << Number(1);
		}
	}

	Object jsonInventory;


	CCustomDetector* det =  smart_cast<CCustomDetector*>(pActor->inventory().ItemFromSlot(DETECTOR_SLOT));
	
	if (det)
	{
		Object table;
		table << "Section" << det->m_section_id.c_str();
		table << "slot_value" << Number(det->cast_inventory_item()->m_ItemCurrPlace.value);
		jsonInSlots << table;
	}

	jsonInventory << "Slots" << jsonInSlots;
	jsonInventory << "Consumable" << jsonConsumable;
	jsonInventory << "Condition" << jsonCondition;

	json << "Inventory" << jsonInventory;
	
	json << "Pos[X]" << Value(pActor->Position().x);
	json << "Pos[Y]" << Value(pActor->Position().y);
	json << "Pos[Z]" << Value(pActor->Position().z);

	if (Level().name().size() > 0)
		json << "LevelID" << String(Level().name().c_str());

	if (ps->money_for_round != 0)
		json << "money" << Number(ps->money_for_round);

	bool noclip = ps->testFlag(GAME_PLAYER_MP_NO_CLIP);
	bool invis = ps->testFlag(GAME_PLAYER_MP_INVIS);
	bool godmode = ps->testFlag(GAME_PLAYER_MP_GOD_MODE);
	bool safemode = ps->testFlag(GAME_PLAYER_MP_SAFE_MODE);

	json << "noclip" << Value(noclip);
	json << "invis" << Value(invis);
	json << "god" << Value(godmode);
	json << "safemode" << Value(safemode);

	if (ps->m_account.name_save().size() != 0)
	{

		string_path name;

		if (FS.path_exist("$mp_saves_file$"))
		{
			FS.update_path(name, "$mp_saves_file$", ps->m_account.name_save().c_str());
		}
		else
		{
			return;
		}

		IWriter* file = FS.w_open(name);
		FS.w_close(file);

		std::ofstream ofile(name);

		if (ofile.is_open())
		{
			ofile << json.json().c_str();
			//Msg("SaveName %s", name);
		}
		ofile.close();
	}
	else
	{
		Msg("SaveName NULL %s", ps->m_account.name_save().c_str());
	}

}