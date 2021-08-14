#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"

void game_sv_freemp::SavePlayers()
{

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
