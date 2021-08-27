#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"

#include <fstream>;
#include "../jsonxx/jsonxx.h"

#include <fstream>
#include <iostream>

using namespace jsonxx;

bool game_sv_freemp::LoadPlayer(ClientID id_who)
{
	
	return false;
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
