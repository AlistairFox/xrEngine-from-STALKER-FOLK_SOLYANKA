#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"

#include <fstream>;
#include "../jsonxx/jsonxx.h"

#include <fstream>
#include <iostream>

#include "actor_mp_client.h"
#include "Weapon.h"

using namespace jsonxx;

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
		std::string str( (std::istreambuf_iterator<char>(ifile) ), std::istreambuf_iterator<char>());

		json.parse(str);
		Msg("Load SaveName %s", name);
	}

	ifile.close();

	Array jsonWeapon;
	Array jsonAmmo;
	Array jsonOutfit;
	Object jsonOthers;
	
	if (json.has<Object>("Inventory"))
	{
		Object inv = json.get<Object>("Inventory");
		
		if (inv.has<Array>("AMMO"))
		{
			Array ammo = inv.get<Array>("AMMO");
			for (int i = 0; i != ammo.size(); i++)
			{
				Object ammo_obj = ammo.get<Object>(i);
				CSE_ALifeItemAmmo* ammo;

				if (ammo_obj.has<String>("Section"))
				{
					LPCSTR ammo_sec = ammo_obj.get<String>("Section").c_str();
					CSE_Abstract* ent = SpawnItemToActorReturn(ps->GameID, ammo_sec);
					ammo = smart_cast<CSE_ALifeItemAmmo*>(ent);

					if (ammo_obj.has<Number>("count"))
					{
						if (ammo)
							ammo->a_elapsed = ammo_obj.get<Number>("count");
					}
				
					spawn_end(ammo, m_server->GetServerClient()->ID);

					Msg("Section Ammo [%s]", ammo_sec);

				}
			}
		}

		if (inv.has<Array>("OUTFIT"))
		{
			Array outfit = inv.get<Array>("OUTFIT");
			for (int i = 0; i != outfit.size(); i++)
			{
				Object out = outfit.get<Object>(i);

			

				if (out.has<String>("Section"))
				{
					LPCSTR section = out.get<String>("Section").c_str();

					CSE_Abstract* ent = SpawnItemToActorReturn(ps->GameID, section);

					CSE_ALifeItemCustomOutfit* itemOutfit = smart_cast<CSE_ALifeItemCustomOutfit*>(ent);

					if (out.has<Number>("condition"))
					{
						if (itemOutfit)
							itemOutfit->m_fCondition = out.get<Number>("condition");
					}

					if (out.has<Number>("slot_id"))
					{
						if (itemOutfit)
							itemOutfit->m_in_slot = out.get<Number>("slot_id");
					}

					spawn_end(ent, m_server->GetServerClient()->ID);


					Msg("Section Outfit [%s]", section);
				}
			}
		}
	
		if (inv.has<Array>("WEAPON"))
		{
			Array weap = inv.get<Array>("WEAPON");

			for (int i = 0; i != weap.size(); i++)
			{
				Object weapon = weap.get<Object>(i);

				if (weapon.has<String>("Section"))
				{
					LPCSTR sec = weapon.get<String>("Section").c_str();
					CSE_Abstract* ent = SpawnItemToActorReturn(ps->GameID, sec);
					CSE_ALifeItemWeapon* ent_weapon = smart_cast<CSE_ALifeItemWeapon*>(ent);
					
					if (!ent_weapon)
						continue;

					if (weapon.has<Number>("condition")) 
						ent_weapon->m_fCondition = weapon.get<Number>("condition"); 

					if (weapon.has<Number>("ammo_count"))
						ent_weapon->a_elapsed = weapon.get<Number>("ammo_count");
					
					if (weapon.has<Number>("ammo_type"))
						ent_weapon->ammo_type = weapon.get<Number>("ammo_type");

					if (weapon.has<Number>("addon_State"))
						ent_weapon->m_addon_flags.flags = weapon.get<Number>("addon_State");

					if (weapon.has<Number>("cur_scope"))
						ent_weapon->m_cur_scope = weapon.get<Number>("cur_scope");

					spawn_end(ent, m_server->GetServerClient()->ID);
					
					Msg("Section [%s],con[%d],ammo[%d],type[%d], addon[%d], scope[%d]", 
						sec,
						ent_weapon->m_fCondition,
						ent_weapon->a_elapsed,
						ent_weapon->ammo_type,
						ent_weapon->m_addon_flags.flags,
						ent_weapon->m_cur_scope
						);
					/*
					if (weapon.has<Number>("slot_id"))
					{
						CObject* local_obj = Level().Objects.net_Find(ent_weapon->ID);
						
						int slot_id = weapon.get<Number>("slot_id");
						
						CWeapon* weapon_item = smart_cast<CWeapon*>(local_obj);

						if (weapon_item)
							weapon_item->m_ItemCurrPlace.value = slot_id;
					}
					*/

				}
			}
		}

		if (inv.has<Object>("STUF"))
		{
			Object others = inv.get<Object>("STUF");

			for (auto i = others.kv_map().begin(); i != others.kv_map().end(); i++)
			{
				LPCSTR name = (*i).first.c_str();
				int count = (*i).second->number_value_;
			
			//	Msg("Spawn Item[%s] [%d]", count);

				//if (inv.has<Number>(name))
				{
				//	int count = inv.get<Number>(name);
					Msg("Spawn Item[%s] [%d]", name, count);

					for (int i = 0; i != count; i++)
					{
						SpawnItemToActor(ps->GameID, name);
					}
				}
			}

			
		}
	}

	SpawnItemToActor(id_who->GameID, "device_pda");
	SpawnItemToActor(id_who->GameID, "device_torch");
	SpawnItemToActor(id_who->GameID, "wpn_knife");
	SpawnItemToActor(id_who->GameID, "wpn_binoc");


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

		if (jsonObj.has<Number>("money"))
		{
			int money = jsonObj.get<Number>("money");

			ps->money_for_round = money;
			signal_Syncronize();
		}

		if (jsonObj.has<Number>("Pos[X]") && jsonObj.has<Number>("Pos[Y]") && jsonObj.has<Number>("Pos[Z]"))
		{
			float x = jsonObj.get<Number>("Pos[X]");
			float y = jsonObj.get<Number>("Pos[Y]");
			float z = jsonObj.get<Number>("Pos[Z]");

			Msg("Actor Postion READ [%.3f][%.3f][%.3f]", x, y, z);
			position.set(x, y, z);
			angle.set(0, 0, 0);
			return true;
		}

	}

	return false;
}

bool game_sv_freemp::GetPosAngleFromActor(ClientID id, Fvector& Pos, Fvector& Angle)
{
	xrClientData* xrData = server().ID_to_client(id);
	if (!xrData)
		return false;

	if (!xrData->ps)
		return false;
	
	if (LoadPlayerPosition(xrData->ps, Pos, Angle) )
	{
		Msg("Position Set [%.0f][%.0f][%.0f]", Pos.x, Pos.y, Pos.z);
		return true;
	}
	 
		
	return inherited::GetPosAngleFromActor(id, Pos, Angle);
 
}

void game_sv_freemp::assign_RP(CSE_Abstract* E, game_PlayerState* ps_who)
{
	if (ps_who->testFlag(GAME_PLAYER_MP_ON_CONNECTED))
	{
		Fvector Pos, Angle;

		if (LoadPlayerPosition(ps_who, Pos, Angle))
		{
			E->o_Position.set(Pos);
			E->o_Angle.set(Angle);

			//Msg("Position Set [%.0f][%.0f][%.0f]", Pos.x, Pos.y, Pos.z);
			ps_who->resetFlag(GAME_PLAYER_MP_ON_CONNECTED);
			return;
		}
	}
	 
 
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
					Msg("Nick[%s] login[%s] password[%s]", new_nickname, login, password);

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
			
			if (!xr_strcmp(tab.get<String>("login:").c_str(), login))
			{
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
		
		Msg("read_fmp_section [%s]",item_name);
		
		spawned_items.StartItems.push_back(item_name);
	};
}
