#include "StdAfx.h"
#include "../xrEngine/XR_IOConsole.h"
#include "../xrEngine/xr_ioc_cmd.h"

#include "Level.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "game_sv_freemp.h"

extern char const* exclude_raid_from_args(LPCSTR args, LPSTR dest, size_t dest_size);

//MASTER SERVER CMDS

class CCC_MasterServerPrint : public IConsole_Command
{
public:
	CCC_MasterServerPrint(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (!Level().Server)
			return;

		Level().Server->PrintMasterServerCLS();
	}
};

class CCC_MasterServerStartWORK : public IConsole_Command
{
public:
	CCC_MasterServerStartWORK(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (!Level().Server)
			return;

		Level().Server->StartServerCLS();
	}
};

class CCC_MasterServerSendMsg : public IConsole_Command
{
public:
	CCC_MasterServerSendMsg(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		LPCSTR msg;
		sscanf_s(args, "%s", &msg);

		if (!Level().Server)
			return;

		if (Level().Server->isMaster())
		{
			for (auto ms : Level().Server->getMasterCLS())
				Level().Server->SendMsgToMasterServerCL(msg, ms.id);
		}
		else
		{
			Level().SendMSG(msg, Level().MasterServerClient);
		}
	}
};

class CCC_MasterSendDataType : public IConsole_Command
{
public:
	CCC_MasterSendDataType(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		int id = 0;
		sscanf_s(args, "%d", &id);

		if (!Level().Server)
			return;

		if (!Level().Server->isMaster())
		{
			Msg("Send Data: %d", id);
			Level().MasterClientSend(id);
		}
	}
};

class CCC_MasterServerSendCMD : public IConsole_Command
{
public:
	CCC_MasterServerSendCMD(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		int id = 0;
		sscanf(args, "%d", &id);
		if (!Level().Server)
			return;

		MasterServerID mid; mid.set(id);

		if (Level().Server->isMaster())
		{
			Level().Server->SendReciveMsgsFromCLient(mid);
		}
	}
};

class CCC_MasterServerKillWorker : public IConsole_Command
{
public:
	CCC_MasterServerKillWorker(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		int id = 0;
		sscanf(args, "%d", &id);
		if (!Level().Server)
			return;

		MasterServerID mid; mid.set(id);

		if (Level().Server->isMaster())
		{
			Level().Server->SendKillProcess(mid);
		}
	}
};



class CCC_GetStatMemory : public IConsole_Command
{
public:
	CCC_GetStatMemory(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		int id = 0;
		sscanf(args, "%d", &id);
		if (!Level().Server)
			return;

		MasterServerID mid; mid.set(id);

		if (Level().Server->isMaster())
		{
			Level().Server->MasterServer_StatsMemory(mid);
		}
	}
};

struct castCSE
{
	CSE_Abstract* abs;
	CSE_ALifeCreatureActor* actor;
	CSE_ALifeInventoryItem* item;
	CSE_ALifeHumanStalker* stalker;
	CSE_ALifeOnlineOfflineGroup* group;
	CSE_ALifeItemWeapon* wpn;
	CSE_ALifeItemAmmo* ammo;

	CSE_ALifeHumanStalker s;
	CSE_ALifeMonsterAbstract m;


};


class CCC_ServerSize : public IConsole_Command
{
public:
	CCC_ServerSize(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (!Level().Server)
			return;

		if (Level().Server)
		{
			for (auto e : *Level().Server->GetEntitys())
			{
				Msg("Entity: %d, name: %s, _msize(%d)", e.first, e.second->name_replace(), sizeof(*e.second));
			}

		}

	}
};

class CCC_SetRankForID : public IConsole_Command
{
public:
	CCC_SetRankForID(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		if (!OnServer())
		{
			int rank = 0, id = 0;
			sscanf(args, "%d %d", &id, &rank);

			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "set_rank %d %d", id, rank);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
			return;
		}
		else
		{
			int rank = 0, id = 0;
			sscanf(args, "%d %d", &id, &rank);

			game_sv_freemp* freemp = smart_cast<game_sv_freemp*>(Level().Server->game);

			if (freemp)
			{
				ClientID clID;
				clID.set(id);

				xrClientData* data = (xrClientData*)Level().Server->GetClientByID(clID);
				if (data && data->ps)
					data->ps->rank = rank;

				freemp->signal_Syncronize();
			}
		}
	}
};

class CCC_PrintAllSmarts : public IConsole_Command
{
public:
	CCC_PrintAllSmarts(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			luabind::functor<void> function;
			ai().script_engine().functor("smart_terrain.print_all_smarts", function);
			function();
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "g_list_smarts");
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
};

class CCC_SpawnSquadToSmartName : public IConsole_Command
{
public:
	CCC_SpawnSquadToSmartName(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			string256 arg;
			exclude_raid_from_args(args, arg, sizeof(arg));
			string256 squad, smart;
			sscanf_s(arg, "%s %s", &squad, sizeof(squad), &smart, sizeof(smart));
			luabind::functor<void> function;
			ai().script_engine().functor("smart_terrain.sv_spawn_squad_name", function);
			function(squad, smart);
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "g_spawn_squad_name %s", args);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}

	virtual void		Save(IWriter* F) {};
	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		for (auto sec : pSettings->sections())
		{
			if (sec->line_exist("faction"))
				tips.push_back(sec->Name);
		}
	}
};

class CCC_SpawnSquadToSmartID : public IConsole_Command
{
public:
	CCC_SpawnSquadToSmartID(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			string256 arg;
			exclude_raid_from_args(args, arg, sizeof(arg));
			string256 squad;
			int smart;
			sscanf_s(arg, "%s %d", &squad, sizeof(squad), &smart);
			luabind::functor<void> function;
			ai().script_engine().functor("smart_terrain.sv_spawn_squad_id", function);
			function(squad, smart);
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "g_spawn_squad %s", args);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}

	virtual void	Save(IWriter* F) {};
	virtual void	fill_tips(vecTips& tips, u32 mode)
	{
		for (auto sec : pSettings->sections())
		{
			if (sec->line_exist("faction"))
				tips.push_back(sec->Name);
		}
	}

};

// Master Server (NOT ENDED CODE)
void register_console_masterserver()
{
 	//TEST	
	CMD1(CCC_ServerSize, "server_memory_entity");
	CMD1(CCC_PrintAllSmarts, "g_list_smarts");
	CMD1(CCC_SpawnSquadToSmartID, "g_spawn_squad");

	//Master Server Connsole Commands

	CMD1(CCC_MasterServerPrint, "ms_print_cls");
	CMD1(CCC_MasterServerStartWORK, "ms_start_workers")

	//CMD1(CCC_MasterServerSendMsg, "ms_send");
	//CMD1(CCC_MasterSendDataType, "ms_send_data");

	CMD1(CCC_MasterServerSendCMD, "ms_send_cmd");
	CMD1(CCC_MasterServerKillWorker, "ms_kill_worker");
	CMD1(CCC_GetStatMemory, "ms_stats");
}