#include "StdAfx.h"
#include "xrServer_Objects_ALife.h"
#include "xrServer_Objects_ALife_Items.h"
#include "xrServer_Objects_ALife_Monsters.h"

#include "string_table.h"

#pragma warning(disable: 4995)
#include "..\jsonxx\jsonxx.h"
#pragma warning(default: 4995)
using namespace jsonxx;

void w_stringsafe(CInifile* file, LPCSTR section, LPCSTR line, LPCSTR data)
{
	if (file->line_exist(section, line))
		Msg("line exist: %s, %s", section, line);
	else
		file->w_string(section, line, data);
}

LPCSTR r_stringsafe(CInifile* file, LPCSTR section, LPCSTR line)
{
	if (file->line_exist(section, line))
		file->r_string(section, line);
	else
		return "default";
}

Object GenerateItemList(LPCSTR section_name, bool description, shared_str& des_text_ru, shared_str& name_text_ru_)
{
	Object insert_table;
 	insert_table << "description" << String(description ? des_text_ru.c_str() : "");
	insert_table << "cost" << Number(pSettings->r_u32(section_name, "cost"));
	insert_table << "inv_name" << String(name_text_ru_!= nullptr ? name_text_ru_.c_str() : "null name");
	insert_table << "section" << String(section_name);
	return insert_table;
}

void ExportSectionsItems()
{
	// FAST INI Sections Only

	string_path pooooo;
	FS.update_path(pooooo, "$game_config$", "alife\\gspawn_lock.ltx");
	CInifile* locked_items = xr_new<CInifile>(pooooo);


	string_path p;
	FS.update_path(p, "$game_config$", "alife\\trade\\trade_section.ltx");
	CInifile* file = xr_new<CInifile>(p, false, false, false);

	file->set_override_names(true);

	if (file)
		for (auto sect : pSettings->sections())
		{
			if (!locked_items->line_exist("list", sect->Name.c_str()))
				if (sect->line_exist("class") && !sect->line_exist("ignore_spawn") && !sect->line_exist("value") && !sect->line_exist("scheme_index"))  //sect->line_exist("cost") && 
				{
					Msg("OBJECT: %s", sect->Name.c_str());
					CSE_Abstract* E = F_entity_Create(sect->Name.c_str());
					CSE_ALifeInventoryItem* item = smart_cast<CSE_ALifeInventoryItem*>(E);

					CSE_ALifeItemAmmo* ammo = smart_cast<CSE_ALifeItemAmmo*>(E);
					CSE_ALifeItemArtefact* art = smart_cast<CSE_ALifeItemArtefact*>(E);
					CSE_ALifeItemDetector* det = smart_cast<CSE_ALifeItemDetector*>(E);
					CSE_ALifeItemDocument* doc = smart_cast<CSE_ALifeItemDocument*>(E);
					CSE_ALifeItemHelmet* helmet = smart_cast<CSE_ALifeItemHelmet*>(E);
					CSE_ALifeItemCustomOutfit* outfit = smart_cast<CSE_ALifeItemCustomOutfit*>(E);

					CSE_ALifeItemWeapon* wpn = smart_cast<CSE_ALifeItemWeapon*>(E);

					CSE_ALifeItemWeaponMagazined* wpn_magazined = smart_cast<CSE_ALifeItemWeaponMagazined*>(E);
					CSE_ALifeItemWeaponShotGun* wpn_ShotGun = smart_cast<CSE_ALifeItemWeaponShotGun*>(E);
					CSE_ALifeItemWeaponAutoShotGun* wpn_AutoShotGun = smart_cast<CSE_ALifeItemWeaponAutoShotGun*>(E);

					CSE_ALifeItemGrenade* grnd = smart_cast<CSE_ALifeItemGrenade*>(E);
					CSE_ALifeItemExplosive* explosive = smart_cast<CSE_ALifeItemExplosive*>(E);
					CSE_ALifeItemPDA* PDA = smart_cast<CSE_ALifeItemPDA*>(E);

					CSE_ALifeHelicopter* helis = smart_cast<CSE_ALifeHelicopter*>(E);
					CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(E);
					CSE_ALifeCreatureActor* actors = smart_cast<CSE_ALifeCreatureActor*>(E);
					CSE_ALifeMonsterAbstract* monsters = smart_cast<CSE_ALifeMonsterAbstract*>(E);
					CSE_ALifeHumanStalker* stalkers = smart_cast<CSE_ALifeHumanStalker*>(E);
					CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(E);
					CSE_ALifeCustomZone* czone = smart_cast<CSE_ALifeCustomZone*>(E);

					bool description = false;// pSettings->line_exist(sect->Name, "description");
					LPCSTR des_text_ru = 0;
					if (description)
						des_text_ru = (*CStringTable().translate(pSettings->r_string(sect->Name, "description")));

					if (wpn_AutoShotGun)
					{
						w_stringsafe(file, "Weapons_AutoShotguns", wpn_AutoShotGun->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (wpn_ShotGun)
					{
						w_stringsafe(file, "Weapons_Shotguns", wpn_ShotGun->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (wpn_magazined)
					{
						w_stringsafe(file, "Weapons_Magazined", wpn_magazined->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (wpn)
					{
						w_stringsafe(file, "Weapons", wpn->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (ammo)
					{
						w_stringsafe(file, "Ammos", ammo->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (art)
					{

						u8 Tier = 0;
						if (sect->line_exist("af_rank"))
						{
							Tier = pSettings->r_u8(sect->Name.c_str(), "af_rank");
						}
						string128 aftier;
						xr_sprintf(aftier, "Artefacts_%d", Tier);
						w_stringsafe(file, aftier, art->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (det)
					{
						w_stringsafe(file, "Detectors", det->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (doc)
					{
						w_stringsafe(file, "Documents", doc->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (helmet)
					{
						w_stringsafe(file, "Helmets", helmet->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (outfit)
					{
						string128 tmp_community;
						xr_sprintf(tmp_community, "Outfits_%s", sect->line_exist("use_community") ? pSettings->r_string(sect->Name, "use_community") : "NOLINE");

						w_stringsafe(file, tmp_community, outfit->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (PDA)
					{
						w_stringsafe(file, "PDA", PDA->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (grnd)
					{
						w_stringsafe(file, "Grenades", grnd->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (explosive)
					{
						w_stringsafe(file, "Explosive", explosive->s_name.c_str(), description ? des_text_ru : "");
					}

					else if (item)
					{
						string128 t;
						xr_sprintf(t, "Items_%s", pSettings->r_string(sect->Name, "class"));
						w_stringsafe(file, t, item->m_self->s_name.c_str(), description ? des_text_ru : "");
					}
					/*
					CSE_ALifeHelicopter* helis = smart_cast<CSE_ALifeHelicopter*>(E);
					CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(E);
					CSE_ALifeCreatureActor* actors = smart_cast<CSE_ALifeCreatureActor*>(E);
					CSE_ALifeMonsterAbstract* monsters = smart_cast<CSE_ALifeMonsterAbstract*>(E);
					CSE_ALifeHumanStalker* stalkers = smart_cast<CSE_ALifeHumanStalker*>(E);
					CSE_ALifeGroupAbstract* group = smart_cast<CSE_ALifeGroupAbstract*>(E);
					*/
					else if (helis)
					{
						w_stringsafe(file, "Helicopters", helis->s_name.c_str(), description ? des_text_ru : "");
					}

					else if (actors)
					{
						w_stringsafe(file, "Actors", actors->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (stalkers)
					{
						w_stringsafe(file, "Stalkers", stalkers->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (monsters)
					{
						w_stringsafe(file, "Monsters", monsters->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (group)
					{
						string128 sec_prefix;
						if (pSettings->line_exist(sect->Name.c_str(), "faction"))
							xr_strcpy(sec_prefix, pSettings->r_string(sect->Name.c_str(), "faction"));
						else
							xr_strcpy(sec_prefix, "default");

						string128 tmp;
						xr_sprintf(tmp, "sim_squads_%s", sec_prefix);

						w_stringsafe(file, tmp, group->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (czone)
					{
						w_stringsafe(file, "CustomZone", czone->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (dynamic)
					{
						w_stringsafe(file, "DynamicObjects", dynamic->s_name.c_str(), description ? des_text_ru : "");
					}
					else if (E)
					{
						string128 t;
						xr_sprintf(t, "Others_%s", pSettings->r_string(sect->Name, "class"));
						w_stringsafe(file, t, E->s_name.c_str(), description ? des_text_ru : "");
					}

					if (E)
						F_entity_Destroy(E);
				}
		}

	file->save_as(p);

	// JSON

	{
		FS.update_path(p, "$game_config$", "alife\\trade\\trade_section.json");

		Object table_json;
		Array array_Weapon, arrayWeaponShotguns, arrayWeaponAutoShotguns, arrayWeaponMag, arrayAmmos, arrayArtefacts, arrayDetectors, arrayDocuments,
			arrayHelmets, arrayOutfits, arrayPDA, arrayGrenades, arrayExplosive; //, arraysItems, arraysOthers;

		xr_map<LPCSTR, Array> inventory_classes;
		xr_map<LPCSTR, Array> outfits_factions;
		xr_map<LPCSTR, Array> others_classes;

		for (auto sect : pSettings->sections())
		{
			//sect->line_exist("description") && !sect->line_exist("value") && !sect->line_exist("scheme_index") && !sect->line_exist("ignore_spawn") ) || sect->line_exist("species") || sect->line_exist("script_binding")

			if (sect->line_exist("class") && sect->line_exist("cost") && !sect->line_exist("ignore_spawn"))
			{
				//	Msg("Read Section: %s", sect->Name.c_str());
				CSE_Abstract* E = F_entity_Create(sect->Name.c_str());
				CSE_ALifeInventoryItem* item = smart_cast<CSE_ALifeInventoryItem*>(E);

				CSE_ALifeItemAmmo* ammo = smart_cast<CSE_ALifeItemAmmo*>(E);
				CSE_ALifeItemArtefact* art = smart_cast<CSE_ALifeItemArtefact*>(E);
				CSE_ALifeItemDetector* det = smart_cast<CSE_ALifeItemDetector*>(E);
				CSE_ALifeItemDocument* doc = smart_cast<CSE_ALifeItemDocument*>(E);
				CSE_ALifeItemHelmet* helmet = smart_cast<CSE_ALifeItemHelmet*>(E);
				CSE_ALifeItemCustomOutfit* outfit = smart_cast<CSE_ALifeItemCustomOutfit*>(E);

				CSE_ALifeItemWeapon* wpn = smart_cast<CSE_ALifeItemWeapon*>(E);

				CSE_ALifeItemWeaponMagazined* wpn_magazined = smart_cast<CSE_ALifeItemWeaponMagazined*>(E);
				CSE_ALifeItemWeaponShotGun* wpn_ShotGun = smart_cast<CSE_ALifeItemWeaponShotGun*>(E);
				CSE_ALifeItemWeaponAutoShotGun* wpn_AutoShotGun = smart_cast<CSE_ALifeItemWeaponAutoShotGun*>(E);

				CSE_ALifeItemGrenade* grnd = smart_cast<CSE_ALifeItemGrenade*>(E);
				CSE_ALifeItemExplosive* explosive = smart_cast<CSE_ALifeItemExplosive*>(E);
				CSE_ALifeItemPDA* PDA = smart_cast<CSE_ALifeItemPDA*>(E);

				bool description = pSettings->line_exist(sect->Name, "description");
				bool name = pSettings->line_exist(sect->Name, "inv_name");

				shared_str des_text_ru;
				shared_str name_text_ru_;
				if (description)
				{
					des_text_ru._set(*CStringTable().translate(pSettings->r_string(sect->Name, "description")));
				}

				if (name)
				{
					name_text_ru_._set(*CStringTable().translate(pSettings->r_string(sect->Name, "inv_name")));
				}
				else
				{
					name_text_ru_._set("dont_have: inv_name");
				}

				if (wpn_AutoShotGun)
				{
					arrayWeaponAutoShotguns << GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (wpn_ShotGun)
				{
 					arrayWeaponShotguns		<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (wpn_magazined)
				{
					arrayWeaponMag			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
 				}
				else if (wpn)
				{
					array_Weapon			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (ammo)
				{
					arrayAmmos				<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (art)
				{
					arrayArtefacts			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (det)
				{
					arrayDetectors			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (doc)
				{
					arrayDocuments			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (helmet)
				{
					arrayHelmets			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (outfit)
				{
					Object insert_table;
					insert_table << "description" << String(description ? des_text_ru.c_str() : "");
					insert_table << "cost" << Number(pSettings->r_u32(sect->Name, "cost"));
					insert_table << "inv_name" << String(name_text_ru_.c_str());
					insert_table << "section" << String(sect->Name.c_str());
 
					string128 tmp_community;
					xr_sprintf(tmp_community, "Outfits_%s", sect->line_exist("use_community") ? pSettings->r_string(sect->Name, "use_community") : "NOLINE");

					outfits_factions[tmp_community] << insert_table;
				}
				else if (PDA)
				{
 					arrayPDA				<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (grnd)
				{
					arrayGrenades			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
				else if (explosive)
				{
					arrayExplosive			<< GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);
				}
 				else if (item)
				{
					inventory_classes[pSettings->r_string(sect->Name, "class")] << GenerateItemList(sect->Name.c_str(), description, des_text_ru, name_text_ru_);;
				}
				else if (E)
				{
					Object insert_table;
					insert_table << "section" << String(sect->Name.c_str());
					others_classes[pSettings->r_string(sect->Name, "class")] << insert_table;
				}

				if (E)
					F_entity_Destroy(E);
			}

		}

		table_json << "WPN" << array_Weapon;
		table_json << "WPN_SHOTGUNS" << arrayWeaponShotguns;
		table_json << "WPN_AUTO_SHOTGUNS" << arrayWeaponAutoShotguns;
		table_json << "WPN_MAGAZINED" << arrayWeaponMag;

		table_json << "WPN_AMMO" << arrayAmmos;
		table_json << "ARTEFACTS" << arrayArtefacts;
		table_json << "DETECTORS" << arrayDetectors;
		table_json << "HELMETS" << arrayHelmets;
		//table_json << "OUTFITS" << arrayOutfits;
		table_json << "PDA" << arrayPDA;
		table_json << "GRENADES" << arrayGrenades;
		table_json << "EXPLOSIVES" << arrayExplosive;

		Object ARRAY_inventory_classes;
		for (auto object : inventory_classes)
		{
			string128 tmp;
			xr_sprintf(tmp, "table_items_%s", object.first);
			ARRAY_inventory_classes << tmp << object.second;
		}
		table_json << "INV_CLASSES" << ARRAY_inventory_classes;

		Object ARRAY_others_classes;
		for (auto object : others_classes)
		{
			string128 tmp;
			xr_sprintf(tmp, "table_others_%s", object.first);
			ARRAY_others_classes << tmp << object.second;
		}
		table_json << "OTHERS_CLASSES" << ARRAY_others_classes;

		Object ARRAY_OUTFIT;
		for (auto item : outfits_factions)
		{
			string128 tmp;
			xr_sprintf(tmp, "%s", item.first);
			ARRAY_OUTFIT << tmp << item.second;
		}

		table_json << "OUTFITS" << ARRAY_OUTFIT;

		IWriter* w = FS.w_open(p);

		if (w)
		{
			w->w_string(table_json.json().c_str());
		}

		FS.w_close(w);

	}
}
