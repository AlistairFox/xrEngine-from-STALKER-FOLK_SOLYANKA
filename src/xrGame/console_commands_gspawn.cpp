#include "stdafx.h"
#include "../xrEngine/xr_ioconsole.h"
#include "../xrEngine/xr_ioc_cmd.h"
#include "Level.h"
#include "game_sv_mp.h"
#include "game_sv_freemp.h"
#include "HUDManager.h"
#include "InventoryBox.h" 
#include "InventoryOwner.h"

extern char const* exclude_raid_from_args(LPCSTR args, LPSTR dest, size_t dest_size);


/**

	G_Spawns classes

**/


class CCC_SpawnToInventory : public IConsole_Command {
public:
	CCC_SpawnToInventory(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void		Execute(LPCSTR arguments)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_mp* srv = smart_cast<game_sv_mp*>(Level().Server->game);
		if (!srv) return;

		ClientID client_id(0);
		u32 tmp_client_id;
		string256 section;
		u32 counts;


		string1024 buff;
		exclude_raid_from_args(arguments, buff, sizeof(buff));

		if (sscanf_s(buff, "%u %s %u", &tmp_client_id, &section, sizeof(section), &counts) != 3)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Spawn item to player. Format: \"sv_spawn_to_player_inv <player session id> <item section> <counts>\"");
			return;
		}

		client_id.set(tmp_client_id);

		xrClientData* CL = static_cast<xrClientData*>(Level().Server->GetClientByID(client_id));

		if (counts > 10)
			counts = 10;

		if (pSettings->section_exist(section))
			for (u32 i = 0; i < counts; i++)
			{
				if (CL && CL->owner)
				{
					srv->SpawnItem(section, CL->owner->ID);
				}
				else
				{
					Msg("! Can't spawn item to client %u", client_id.value());
				}
			}
	}
	virtual void		Save(IWriter* F) {};
};

class CCC_SpawnToObjWithId : public IConsole_Command {
public:
	CCC_SpawnToObjWithId(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void		Execute(LPCSTR arguments)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_mp* srv = smart_cast<game_sv_mp*>(Level().Server->game);
		if (!srv) return;

		u16 tmp_id;
		string256 section;
		u32 counts = 1;

		string1024 buff;
		exclude_raid_from_args(arguments, buff, sizeof(buff));

		if (sscanf_s(buff, "%u %s %u", &tmp_id, &section, &counts) != 3)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Spawn item by ID. Format: \"sv_spawn_to_obj_with_id <player id> <item section> <counts>\"");
			return;
		}

		if (!pSettings->section_exist(section))
		{
			return;
		}

		if (counts > 10)
			counts = 10;

		if (Level().Objects.net_Find(tmp_id))
		{
			for (int i = 0; i < counts; i++)
				srv->SpawnItem(section, tmp_id);
		}
		else
		{
			Msg("! Can't spawn item to parent %u", tmp_id);
		}
	}
	virtual void		Save(IWriter* F) {};
};
 
class CCC_SpawnOnPosition : public IConsole_Command {
public:
	CCC_SpawnOnPosition(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void		Execute(LPCSTR arguments)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_freemp* srv = smart_cast<game_sv_freemp*>(Level().Server->game);
		if (!srv) return;

		string256 section;
		Fvector3 vec;
		u32 counts;


		string1024 buff;
		exclude_raid_from_args(arguments, buff, sizeof(buff));

		u32 raid = sscanf_s(buff, "%s %u %f %f %f", &section, sizeof(section), &counts, &vec.x, &vec.y, &vec.z);

		if (raid != 5)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Spawn object. Format: \"sv_spawn_on_position <item section> <counts> <position>\"");
			return;
		}

		if (counts > 50)
		{
			counts = 50;
		}

		if (pSettings->section_exist(section))
			for (auto I = 0; I < counts; I++)
				srv->SpawnItemToPos(section, vec);

	}
	virtual void		Save(IWriter* F) {};
};

class CCC_GSpawnToInventorySelf : public IConsole_Command {
public:
	CCC_GSpawnToInventorySelf(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void		Execute(LPCSTR arguments)
	{

		string256 section;
		u32 counts;

		string1024 buff;
		exclude_raid_from_args(arguments, buff, sizeof(buff));
		sscanf_s(buff, "%s %u", &section, sizeof(section), &counts);

		if (!pSettings->section_exist(section))
		{
			Msg("Cant Find section [%s]", section);
			return;
		}

		if (counts > 10)
			counts = 1;

		NET_Packet		P;
		P.w_begin(M_REMOTE_CONTROL_CMD);
		string128 str;
		xr_sprintf(str, "sv_spawn_to_player_inv %u %s %u", Game().local_svdpnid.value(), section, counts);
		Msg("command %s", str);
		P.w_stringZ(str);
		Level().Send(P, net_flags(TRUE, TRUE));

	}

	virtual void		Save(IWriter* F) {};

	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		for (auto sect : pSettings->sections())
		{
			if (sect->line_exist("description")
				&& !sect->line_exist("value")
				&& !sect->line_exist("scheme_index")
				&& !sect->line_exist("ignore_spawn")
				&& !sect->line_exist("parent_section")
				)
				tips.push_back(sect->Name);

			//if (sect->line_exist("parent_section"))
			//{
			//	LPCSTR name = pSettings->r_string(sect->Name, "parent_section");
			//	if (xr_strcmp(name, sect->Name) == 0 )
			//	{
			//		tips.push_back(sect->Name);
			//	}
			//}
		}
	}
};

class CCC_GSpawn_Scopes
{

};

class CCC_GSpawn : public IConsole_Command {
public:
	CCC_GSpawn(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void		Execute(LPCSTR arguments)
	{
		Fvector3 pos, dir, madPos;
		float range;
		pos.set(Device.vCameraPosition);
		dir.set(Device.vCameraDirection);
		collide::rq_result& RQ = HUD().GetCurrentRayQuery();

		if (RQ.O)
		{
			Msg("! ERROR: Can spawn only on ground");
			return;
		}

		range = RQ.range;
		dir.normalize();
		madPos.mad(pos, dir, range);

		string256 section;
		u32 counts;

		string1024 buff;
		exclude_raid_from_args(arguments, buff, sizeof(buff));
		sscanf_s(buff, "%s %u", &section, sizeof(section), &counts);

		if (!pSettings->section_exist(section))
		{
			Msg("Cant Find section [%s]", section);
			return;
		}

		if (counts > 50)
			counts = 50;

		Msg("Spawn: %s, Size: %u", section, counts);

		NET_Packet		P;
		P.w_begin(M_REMOTE_CONTROL_CMD);
		string128 str;
		xr_sprintf(str, "sv_spawn_on_position %s %u %f %f %f", section, counts, madPos.x, madPos.y, madPos.z);
		Msg("command %s", str);

		P.w_stringZ(str);
		Level().Send(P, net_flags(TRUE, TRUE));

	}

	virtual void		Save(IWriter* F) {};

	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		for (auto sect : pSettings->sections())
		{
			if ((sect->line_exist("description")
				&& !sect->line_exist("value")
				&& !sect->line_exist("scheme_index")
				&& !sect->line_exist("ignore_spawn")
				)
				|| sect->line_exist("species") || sect->line_exist("script_binding")
				)
				tips.push_back(sect->Name);
		}
	}
};

class CCC_GSpawnToInventory : public IConsole_Command {
public:
	CCC_GSpawnToInventory(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void		Execute(LPCSTR arguments)
	{
		collide::rq_result& RQ = HUD().GetCurrentRayQuery();

		CInventoryOwner* invOwner = smart_cast<CInventoryOwner*>(RQ.O);
		CInventoryBox* invBox = smart_cast<CInventoryBox*>(RQ.O);

		string256 section;
		u32 counts;

		string1024 buff;
		exclude_raid_from_args(arguments, buff, sizeof(buff));
		sscanf_s(buff, "%s %u", &section, sizeof(section), &counts);

		if (!pSettings->section_exist(section))
		{
			Msg("Cant find section [%s]", section);
			return;
		}

		if (counts > 10)
			counts = 1;

		if (RQ.O && (invOwner || invBox))
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "sv_spawn_to_obj_with_id %hu %s %u", RQ.O->ID(), section, counts);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
			return;
		}

	}

	virtual void		Save(IWriter* F) {};

	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		for (auto sect : pSettings->sections())
		{
			if (sect->line_exist("description")
				&& !sect->line_exist("value")
				&& !sect->line_exist("scheme_index")
				&& !sect->line_exist("ignore_spawn")
				)
				tips.push_back(sect->Name);


		}
	}
};


void register_console_gspawn()
{

	// SV PROCESING
	CMD1(CCC_SpawnToInventory, "sv_spawn_to_player_inv");
	CMD1(CCC_SpawnToObjWithId, "sv_spawn_to_obj_with_id");
	CMD1(CCC_SpawnOnPosition, "sv_spawn_on_position");
	
	// CL PROCESING
	CMD1(CCC_GSpawn, "g_spawn");
	CMD1(CCC_GSpawnToInventorySelf, "g_spawn_to_self_inv");
	CMD1(CCC_GSpawnToInventory, "g_spawn_to_inv");

}