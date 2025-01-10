#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"

#include "../jsonxx/jsonxx.h"

#include <fstream>
#include <iostream>

#include "actor_mp_client.h"
#include "Weapon.h"
#include "InventoryBox.h"
#include "Actor.h"	
#include "Inventory.h"
#include "CustomDetector.h"
#include "CustomOutfit.h"
#include "ActorHelmet.h"

using namespace jsonxx;

void game_sv_freemp::SavePlayer(game_PlayerState* ps)
{
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

		string2048 updates = { 0 };

		if (item->has_any_upgrades())
		{
			item->get_upgrades(updates);
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


	CCustomDetector* det = smart_cast<CCustomDetector*>(pActor->inventory().ItemFromSlot(DETECTOR_SLOT));

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
	//bool safemode = ps->testFlag(GAME_PLAYER_MP_SAFE_MODE);
	bool unlim_ammo = ps->testFlag(GAME_PLAYER_MP_UNLIMATED_AMMO);

	json << "noclip" << Boolean(noclip);
	json << "invis" << Boolean(invis);
	json << "god" << Boolean(godmode);
	//json << "safemode" << Boolean(safemode);
	json << "unlim_ammo" << Boolean(unlim_ammo);

	/*
	xrClientData* data = (xrClientData*) get_client(ps->GameID);
	
	Array tab;
	for (auto cl_AS : cl_slots)
	{
		if (cl_AS.cl_id == data->ID)
		{
			Object new_tab;
			new_tab << "slot_1" << String(cl_AS.slots[0]);
			new_tab << "slot_2" << String(cl_AS.slots[1]);
			new_tab << "slot_3" << String(cl_AS.slots[2]);
			new_tab << "slot_4" << String(cl_AS.slots[3]);
			tab << new_tab;
		}
	}
	json << "player_quick_slots" << tab;
	*/

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
 		}
		ofile.close();
	}
	else
	{
		Msg("SaveName NULL %s", ps->m_account.name_save().c_str());
	}

}

bool game_sv_freemp::LoadPlayer(game_PlayerState* id_who)
{
	game_PlayerState* ps =  id_who;
	
	if (!ps) return false;

	Object json;
 	
	string_path name;

	if (FS.path_exist("$mp_saves_file$"))
	{
		FS.update_path(name, "$mp_saves_file$", ps->m_account.name_save().c_str());
	}
	else
	{
		return false;
	}
	
	std::ifstream ifile(name);

	if (ifile.is_open())
	{
		std::string str( (std::istreambuf_iterator<char>(ifile) ), std::istreambuf_iterator<char>() );
		json.parse(str);
		Msg("Load SaveName %s", name);
	}

	ifile.close();

	Array jsonWeapon;
	Array jsonAmmo;
	Array jsonOutfit;
	Object jsonOthers;

	bool invis = json.has<Boolean>("invis");
	bool noclip = json.has<Boolean>("noclip");
	bool god = json.has<Boolean>("god");
	bool unlim_ammo = json.has<Boolean>("unlim_ammo");

	if (invis && json.get<Boolean>("invis"))
		ps->setFlag(GAME_PLAYER_MP_INVIS);
	if (noclip && json.get<Boolean>("noclip"))
		ps->setFlag(GAME_PLAYER_MP_NO_CLIP);
	if (god && json.get<Boolean>("god"))
		ps->setFlag(GAME_PLAYER_MP_GOD_MODE);
	if (unlim_ammo && json.get<Boolean>("unlim_ammo"))
		ps->setFlag(GAME_PLAYER_MP_UNLIMATED_AMMO);

	if (json.has<Number>("money"))
	{
		int money = json.get<Number>("money");
		ps->money_for_round = money;
	}

	signal_Syncronize();

  
	if (json.has<Object>("Inventory"))
	{
		Object inv = json.get<Object>("Inventory");
		
		if (inv.has<Array>("Slots"))
		{
			Array slots = inv.get<Array>("Slots");
			for (int i = 0; i != slots.size(); i++)
			{
				Object slots_tab = slots.get<Object>(i);
				if (slots_tab.has<String>("Section"))
				{
					LPCSTR name = slots_tab.get<String>("Section").c_str();
					CSE_Abstract* ent = SpawnItemToActorReturn(ps->GameID, name);
					CSE_ALifeInventoryItem* item = smart_cast<CSE_ALifeInventoryItem*>(ent);
					CSE_ALifeItemWeapon* ent_weapon = smart_cast<CSE_ALifeItemWeapon*>(ent);

					if (!item)
						continue;

					if (slots_tab.has<String>("upgrades"))
					{
						LPCSTR upgredes = slots_tab.get<String>("upgrades").c_str();
						int counts = _GetItemCount(upgredes);
						for (int i = 0; i != counts; i++)
						{
							string256 upgrede;
							_GetItem(upgredes, i, upgrede, ',');
							 
							item->add_upgrade(upgrede);
						}	
					}

					if (slots_tab.has<Number>("condition"))
						item->m_fCondition = slots_tab.get<Number>("condition");
					
					if (slots_tab.has<Number>("slot_value"))
					{
						u16 value = slots_tab.get<Number>("slot_value");
						item->m_slot_value = value;
					}

					if (ent_weapon)
					{
						if (slots_tab.has<Number>("ammo_count"))
							ent_weapon->a_elapsed = slots_tab.get<Number>("ammo_count");

						if (slots_tab.has<Number>("ammo_type"))
							ent_weapon->ammo_type = slots_tab.get<Number>("ammo_type");

						if (slots_tab.has<Number>("addon_State"))
							ent_weapon->m_addon_flags.flags = slots_tab.get<Number>("addon_State");

						if (slots_tab.has<Number>("cur_scope"))
							ent_weapon->m_cur_scope = slots_tab.get<Number>("cur_scope");
					}
					 

					spawn_end(ent, server().GetServerClient()->ID);
    
				}
			}
		}
	
		if (inv.has<Array>("Condition"))
		{
			Array weap = inv.get<Array>("Condition");

			for (int i = 0; i != weap.size(); i++)
			{
				Object condition = weap.get<Object>(i);

				if (condition.has<String>("Section"))
				{
					LPCSTR sec = condition.get<String>("Section").c_str();
					
					CSE_Abstract* ent = SpawnItemToActorReturn(ps->GameID, sec);
					CSE_ALifeInventoryItem* item = ent->cast_inventory_item();
					CSE_ALifeItemWeapon* ent_weapon = ent->cast_item_weapon();
					CSE_ALifeItemAmmo* ammo = ent->cast_item_ammo();
				
					if (!ent)
						continue;

					if (ammo && condition.has<Number>("count"))
						ammo->a_elapsed = condition.get<Number>("count");

					if (item && condition.has<Number>("condition"))
						item->m_fCondition = condition.get<Number>("condition");

					if (ent_weapon && condition.has<Number>("ammo_count"))
						ent_weapon->a_elapsed = condition.get<Number>("ammo_count");
					
					if (ent_weapon && condition.has<Number>("ammo_type"))
						ent_weapon->ammo_type = condition.get<Number>("ammo_type");

					if (ent_weapon && condition.has<Number>("addon_State"))
						ent_weapon->m_addon_flags.flags = condition.get<Number>("addon_State");

					if (ent_weapon && condition.has<Number>("cur_scope"))
						ent_weapon->m_cur_scope = condition.get<Number>("cur_scope");			

					if (condition.has<String>("upgrades"))
					{
						LPCSTR upgredes = condition.get<String>("upgrades").c_str();
						int counts = _GetItemCount(upgredes);
						for (int i = 0; i != counts; i++)
						{
							string256 upgrede;
							_GetItem(upgredes, i, upgrede, ',');

							item->add_upgrade(upgrede);
						}
					}

					spawn_end(ent, m_server->GetServerClient()->ID);

				}
			}
		}

		if (inv.has<Object>("Consumable"))
		{
			Object others = inv.get<Object>("Consumable");
			for (auto i = others.kv_map().begin(); i != others.kv_map().end(); i++)
			{
				LPCSTR name = (*i).first.c_str();
				int count = (*i).second->number_value_;
				for (int i = 0; i != count; i++)
					SpawnItemToActor(ps->GameID, name);
			}
		}
	}

	if (GameID() == eGameIDFreeMp)
	{
		SpawnItemToActor(id_who->GameID, "device_pda");
		SpawnItemToActor(id_who->GameID, "device_torch");
		SpawnItemToActor(id_who->GameID, "wpn_knife");
		SpawnItemToActor(id_who->GameID, "wpn_binoc");
	}
	return true;
}

bool game_sv_freemp::LoadPlayerPosition(game_PlayerState* ps, Fvector& position, Fvector& angle)
{

	string_path path_xray;

	if (FS.exist(path_xray, "$mp_saves_file$", ps->m_account.name_save().c_str()))
	{
		std::ifstream input(path_xray);

		Object jsonObj;

		if (input.is_open())
		{
			std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

			jsonObj.parse(str);
		}

		if (jsonObj.has<String>("LevelID"))
		{
			LPCSTR level = jsonObj.get<String>("LevelID").c_str();
			if (xr_strcmp(Level().name().c_str(), level))
			{
				return false;
			}
		}

		if (jsonObj.has<Number>("Pos[X]") && jsonObj.has<Number>("Pos[Y]") && jsonObj.has<Number>("Pos[Z]"))
		{
			float x = jsonObj.get<Number>("Pos[X]");
			float y = jsonObj.get<Number>("Pos[Y]");
			float z = jsonObj.get<Number>("Pos[Z]");

			//Msg("Actor Postion READ [%.3f][%.3f][%.3f]", x, y, z);
			position.set(x, y, z);
			angle.set(0, 0, 0);
			return true;
		}

	}

	return false;
}

/*
bool game_sv_freemp::GetPosAngleFromActor(ClientID id, Fvector& Pos, Fvector& Angle)
{
	xrClientData* xrData = server().ID_to_client(id);

	if (xrData)
	if (xrData->ps)	
	if (!surge_started)
	if (LoadPlayerPosition(xrData->ps, Pos, Angle) )
	{
		Msg("Position Set [%.0f][%.0f][%.0f]", Pos.x, Pos.y, Pos.z);
		return true;
	}
 	
	return inherited::GetPosAngleFromActor(id, Pos, Angle);
 
}
*/

void game_sv_freemp::assign_RP(CSE_Abstract* E, game_PlayerState* ps_who)
{
	if (ps_who->testFlag(GAME_PLAYER_MP_ON_CONNECTED) && !surge_started)
	{
		Fvector Pos, Angle;

		if (LoadPlayerPosition(ps_who, Pos, Angle) )
		{
			E->o_Position.set(Pos);
			E->o_Angle.set(Angle);
			ps_who->resetFlag(GAME_PLAYER_MP_ON_CONNECTED);
			return;
		}
	}

	ps_who->resetFlag(GAME_PLAYER_MP_ON_CONNECTED);

	inherited::assign_RP(E, ps_who);
}

void game_sv_freemp::set_account_nickname(LPCSTR login, LPCSTR password, LPCSTR new_nickname, u32 team)
{
	Object ret;

	if (FS.path_exist("$mp_saves$"));
	{
		string_path path_xray;

		FS.update_path(path_xray, "$mp_saves$", "players.json");

		std::ifstream input(path_xray);

		Object jsonObj;

		if (input.is_open())
		{
			std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

			jsonObj.parse(str);
		}

		input.close();
 
		Array jsonArray;

		if (jsonObj.has<Array>("USERS"))
			jsonArray = jsonObj.get<Array>("USERS");

		for (int i = 0; i < jsonArray.size(); i++)
		{
			Object tab = jsonArray.get<Object>(i);

			if (tab.has<String>("login:"))
			{
				std::string login_str = tab.get<String>("login:");
				std::string password_str = tab.get<String>("password:");
				
				bool log = xr_strcmp(login_str.c_str(), login);
				bool pass = xr_strcmp(password_str.c_str(), password);

				if (!log && !pass)
				{
					//tab << "nick" << String(new_nickname);
					jsonObj.get<Array>("USERS").get<Object>(i) << "nick:" << String(new_nickname);
					//Msg("Nick[%s] login[%s] password[%s]", new_nickname, login, password);

					if (team)
					{
						jsonObj.get<Array>("USERS").get<Object>(i) << "team:" << Number(team);
					}
				}
			}


		}

		std::ofstream outfile(path_xray);

		if (outfile.is_open())
		{
			outfile.write(jsonObj.json().c_str(), jsonObj.json().size());
		}
		else
		{

		}

		outfile.close();

 	};

 
}

int game_sv_freemp::get_account_team(LPCSTR login, LPCSTR password)
{
	int team = 0;
	if (FS.path_exist("$mp_saves$"));
	{
		string_path path_xray;

		FS.update_path(path_xray, "$mp_saves$", "players.json");

		std::ifstream input(path_xray);

		Object jsonObj;

		if (input.is_open())
		{
			std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

			jsonObj.parse(str);
		}

		input.close();

		Array jsonArray;

		if (jsonObj.has<Array>("USERS"))
			jsonArray = jsonObj.get<Array>("USERS");

		for (int i = 0; i < jsonArray.size(); i++)
		{
			Object tab = jsonArray.get<Object>(i);
			
			if (tab.has<String>("login:"))
			if (!xr_strcmp(tab.get<String>("login:").c_str(), login))
			{
				if (tab.has<String>("password:"))
				if (!xr_strcmp(tab.get<String>("password:").c_str(), password))
				{
					if (tab.has<Number>("team:"))
						team = tab.get<Number>("team:");
				}
			}		
		}
	}
	 
	return team;
}

void game_sv_freemp::save_inventoryBox(CSE_Abstract* ent)
{
	CSE_ALifeInventoryBox* boxs = smart_cast<CSE_ALifeInventoryBox*>(ent);

	if (!boxs)
		return;

	CObject* obj = Level().Objects.net_Find(boxs->ID);
	CInventoryBox* box = smart_cast<CInventoryBox*>(obj);
	
	if (box)
	{
		TIItemContainer items;
		box->AddAvailableItems(items);
		
		Array listInventory;
		
		Object Main;

		for (auto item : items)
		{
			Object table;

			CWeapon* wpn = smart_cast<CWeapon*>(item);
			CWeaponAmmo* wpn_ammo = smart_cast<CWeaponAmmo*>(item);

			string2048 updates;
			item->get_upgrades(updates);
			
			table << "Section" << String(item->m_section_id.c_str());

			if (item->GetCondition() < 1)
				table << "condition" << Value(item->GetCondition());

			if (item->has_any_upgrades())
				table << "upgrades" << String(updates);

			if (item->cast_weapon())
			{
				CWeapon* weap = item->cast_weapon();
				table << "ammo_count" << Number(weap->GetAmmoElapsed());
				table << "ammo_type" << Number(weap->m_ammoType);
				table << "cur_scope" << Number(weap->m_cur_scope);
				table << "addon_State" << Number(weap->GetAddonsState());
			}

			if (item->cast_weapon_ammo())
				table << "count" << Number(item->cast_weapon_ammo()->m_boxCurr);

			if (box->personal_safe)
			{
				if (box->m_safe_items[item->object_id()].size() > 0)
					table << "player" << String(box->m_safe_items[item->object_id()].c_str());
			}

			listInventory << table;
		}

		Main << "ListItems" << listInventory;
		

		if (FS.path_exist("$mp_saves_inventory$"))
		{
			string128 pathLevel = { 0 };
			xr_strcat(pathLevel, Level().name().c_str());
			
			string128 name = { 0 };
			xr_strcat(name, box->Name());
			xr_strcat(name, ".json");

			xr_strcat(pathLevel, "\\");
			xr_strcat(pathLevel, name);
			
			string_path Level_path;
			FS.update_path(Level_path, "$mp_saves_inventory$", pathLevel);
						 
			IWriter* file = FS.w_open(Level_path);
			FS.w_close(file);

			std::ofstream ofile(Level_path);

			if (ofile.is_open())
			{
				ofile << Main.json().c_str();
				//Msg("SaveName %s", Level_path);
			}

			ofile.close();
		}
	}

}

void game_sv_freemp::load_inventoryBox(CSE_Abstract* ent)
{
	CSE_ALifeInventoryBox* boxs = smart_cast<CSE_ALifeInventoryBox*>(ent);

	if (!boxs)
		return;

	CObject* obj = Level().Objects.net_Find(boxs->ID);
	CInventoryBox* box = smart_cast<CInventoryBox*>(obj);

	//Msg("InitLoad inv_box [%s] id [%d]", boxs->name_replace(), boxs->ID);

	Object Main;
	Array listInventory;

	if (box && box->Name())
	if (FS.path_exist("$mp_saves_inventory$"))
	{
		string128 pathLevel = {0};
		xr_strcat(pathLevel, Level().name().c_str());

		string128 name = {0};
		xr_strcat(name, box->Name());
		xr_strcat(name, ".json");

		xr_strcat(pathLevel, "\\");
		xr_strcat(pathLevel, name);

		string_path Level_path;
		FS.update_path(Level_path, "$mp_saves_inventory$", pathLevel);

		std::ifstream InputFile(Level_path);

		//Msg("FileName %s", Level_path);

		if (InputFile.is_open())
		{
			std::string str((std::istreambuf_iterator<char>(InputFile)), std::istreambuf_iterator<char>());

			Main.parse(str);

			//Msg("LoadFile Name %s", Level_path);
		}

		InputFile.close();
	}
	
	if (Main.has<Array>("ListItems"))
	{
		listInventory = Main.get<Array>("ListItems");
	}
	else
		return;
	
	if (box && box->Name())
	for (int i = 0; i != listInventory.size(); i++)
	{
		Object table = listInventory.get<Object>(i);
		 	
		LPCSTR sec = {0};
		
		if (table.has<String>("Section"))
			sec = table.get<String>("Section").c_str();

		if (!sec)
			return;

		CSE_Abstract* abs = spawn_begin(sec);		
		if (!abs)
			continue;

		abs->ID_Parent = boxs->ID;
		abs->o_Position = boxs->position();

		CSE_ALifeInventoryItem* item = smart_cast<CSE_ALifeInventoryItem*>(abs);
		CSE_ALifeItemWeapon* wpn = smart_cast<CSE_ALifeItemWeapon*>(abs);
		CSE_ALifeItemAmmo* wpn_ammo = smart_cast<CSE_ALifeItemAmmo*>(abs);
		
		if (table.has<String>("upgrades"))
		{
			LPCSTR upgredes = table.get<String>("upgrades").c_str();
			int counts = _GetItemCount(upgredes);
			for (int i = 0; i != counts; i++)
			{
				string256 upgrede;
				_GetItem(upgredes, i, upgrede, ',');

				item->add_upgrade(upgrede);
			}
		}
		
		if (table.has<Number>("condition"))
			item->m_fCondition = table.get<Number>("condition");

		if (wpn)
		{
			u16 a_elapsed, ammo_type, cur_scope, addon_State;

			if (table.has<Number>("ammo_count"))
				a_elapsed = table.get<Number>("ammo_count");
			if (table.has<Number>("ammo_type"))
				ammo_type = table.get<Number>("ammo_type");
			if (table.has<Number>("cur_scope"))
				cur_scope = table.get<Number>("cur_scope");
			if (table.has<Number>("addon_State"))
				addon_State = table.get<Number>("addon_State");

			//Msg("Load Weapon [%d], [%d], [%d], [%d]", a_elapsed, ammo_type, cur_scope, addon_State);
			wpn->a_elapsed = a_elapsed;
			wpn->ammo_type = ammo_type;
			wpn->m_cur_scope = cur_scope;
			wpn->m_addon_flags.flags = addon_State;
		}

		if (wpn_ammo)
		{
			//Msg("Load Weapon ammo");
			if (table.has<Number>("count"))
				wpn_ammo->a_elapsed = table.get<Number>("count");
		}

		CSE_Abstract* item_sapwn_end = spawn_end(abs, server().GetServerClient()->ID);

		if (box->personal_safe)
		{
			if (table.has<String>("player"))
			{
				box->m_safe_items[item_sapwn_end->ID] = table.get<String>("player").c_str();
			}

		}

		//Msg("SpawnInventoryBoxItem [%s] in box [%s]", sec, boxs->name_replace() );

	}
	 
}

void game_sv_freemp::LoadParamsDeffaultFMP()
{
	string4096 items_str;
	string256 item_name;
 
	s32 money = pSettings->r_s32("freemp_default", "start_money");
	spawned_items.StartMoney = money;

	xr_strcpy(items_str, pSettings->r_string("freemp_default", "start_items"));

	u32 count = _GetItemCount(items_str);
	for (u32 t = 0; t < count; ++t)
	{
		_GetItem(items_str, t, item_name);
		
		// Msg("read_fmp_section [%s]",item_name);
		
		spawned_items.StartItems.push_back(item_name);
	};
}
