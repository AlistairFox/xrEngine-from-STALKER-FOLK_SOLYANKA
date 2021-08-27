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

	Array jsonWeapon;
	Array jsonAmmo;
	Array jsonOutfit;
	Object jsonOthers;


	for (; itemStart != itemEnd; itemStart++)
	{
		PIItem item = (*itemStart);

		Object table;

		string2048 updates;
		item->get_upgrades_str(updates);

		if (item->GetCondition() < 1)
			table << "condition" << Value(item->GetCondition());

		if (item->has_any_upgrades())
			table << "upgrades" << Value(updates);

		CWeapon* weap = item->cast_weapon();

		if (item->CurrSlot())
		{
			int slot_val = item->m_ItemCurrPlace.value;
			int slot_id = item->m_ItemCurrPlace.slot_id;
			int slot_base = item->m_ItemCurrPlace.base_slot_id;
			int slot_type = item->m_ItemCurrPlace.type;
						
			table << "slot_id" << Value(slot_id);
		}

		if (weap)
		{
			table << "ammo_count" << Value(weap->GetAmmoElapsed());
			table << "ammo_type" << Value(weap->m_ammoType);
			table << "cur_scope" << Value(weap->m_cur_scope);
			table << "addon_State" << Value(weap->GetAddonsState());
		}

		CWeaponAmmo* ammo = item->cast_weapon_ammo();

		if (item->cast_weapon())
		{
			table << "Section" << Value(item->m_section_id.c_str());
			jsonWeapon << table;
		}
		else
		if (ammo)
		{
			int box_size = ammo->m_boxCurr;
			//Msg("ammo_size[%d] [%s]", box_size, item->NameItem());
			Object tab;
			tab << "count" << Number(box_size);
			tab << "Section" << ammo->m_section_id.c_str();
			jsonAmmo << tab;
		}
		else
		if (smart_cast<CCustomOutfit*>(item) || smart_cast<CHelmet*>(item))
		{
			table << "Section" << Value(item->m_section_id.c_str());
			jsonOutfit << table;
		}
		else
		{
			if (jsonOthers.has<Number>(item->m_section_id.c_str()))
			{

				int count = jsonOthers.get<Number>(item->m_section_id.c_str());

				count += 1;

				jsonOthers << item->m_section_id.c_str() << Number(count);

			}
			else
				jsonOthers << item->m_section_id.c_str() << Number(1);
		}
	}

	/*
			for (auto i = jsonOthers.kv_map().begin(); i != jsonOthers.kv_map().end(); i++ )
			{
				LPCSTR name = (*i).first.c_str();
				Msg("Tab[%s]", name);
			}
	*/

	Object jsonInventory;

	jsonInventory << "WEAPON" << jsonWeapon;
	jsonInventory << "OUTFIT" << jsonOutfit;
	jsonInventory << "AMMO" << jsonAmmo;
	jsonInventory << "STUF" << jsonOthers;

	json << "Inventory" << jsonInventory;
	json << "Community:" << Value(pActor->Community());
	json << "Pos[X]" << Value(pActor->Position().x);
	json << "Pos[Y]" << Value(pActor->Position().y);
	json << "Pos[Z]" << Value(pActor->Position().z);

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
			Msg("SaveName %s", name);
		}
		ofile.close();
	}
	else
	{
		Msg("SaveName %s", ps->m_account.name_save().c_str());
	}

}