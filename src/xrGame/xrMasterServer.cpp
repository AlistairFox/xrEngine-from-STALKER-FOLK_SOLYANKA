#include "StdAfx.h"
#include "xrServer.h"
#include "MasterServerEvents.h"
#include "Level.h"

//Master Server
xr_map<int, client_DATA> master_server_data;

int last_port_client = 5000;
int last_port_svcl = 6000;

void StartDedicatedServer(string256& Level, string256& workFolder, string256& exe_file, string256& cmd_c, string256& cmd_s, string256& master_server)
{
	string_path app, params;
 
	string128 port_str, port_cl;
 
	sprintf(port_str, "%d", last_port_client);
	sprintf(port_cl, "%d", last_port_svcl);

	//Msg("Working folder is:%s", workFolder);
	//Msg("Level Name: %s", Level);

	xr_strcpy(app, exe_file);
	xr_strcat(app, "xrEngine.exe");
	sprintf(params, "%s  -i -start server(%s/%s/portsv=%s) client(%s) -fsltx ..\\..\\fsgame.ltx -master_server(%s) -hide", app, Level, cmd_s, port_str, cmd_c,  master_server);

	//Msg("app   = %s", app);
	Msg("params: %s", params);
 
	//CreateProcess need to return results to next two structures
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	//We use CreateProcess to setup working folder
	//char const* temp_wf = (xr_strlen(workFolder) > 0) ? workFolder : NULL;
	bool created = CreateProcess(app, params, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	//Msg("RUNNED: %s", created ? "true" : "false");

	last_port_client += 10;
	last_port_svcl   += 10;
}

xr_vector<int> need_send_data;
struct START_PARAMS
{
	string256 level;
	string256 wf;
	string256 process;
	string256 cmd_c;
	string256 cmd_s;
	string256 master_server;

	START_PARAMS(LPCSTR l, string256 w, string256 p, string256 c, string256 s, string256 m)
	{
		xr_strcpy(level, l);
		xr_strcpy(wf, w);
		xr_strcpy(process, p);
		xr_strcpy(cmd_c, c);
		xr_strcpy(cmd_s, s);
		xr_strcpy(master_server, m);
	}
};

xr_vector<START_PARAMS> sparams;

void ThreadUpdate(void* t)
{
 
}


void xrServer::PrintMasterServerCLS()
{	
	v_MS_servers vec = inherited::getMasterCLS();
 	for (auto ms : vec)
	{
		client_DATA data = master_server_data[ms.id.value()];

 		Msg("Master Server ID[%d] connection[%d], ip[%s]", ms.id, ms.connection, ms.IP_V4);
		Msg("TableData: id: %d, map: %s, ip: %s", data.master_client_id, data.map, data.IP_V4);
	}
}

void xrServer::StartServerCLS()
{
	string256 wf, process;
	string_path ini_path;

	FS.update_path(ini_path, "$app_data_root$", "MasterServer.ltx");
	FS.rescan_path(ini_path, true);
	CInifile* file = xr_new<CInifile>(ini_path, true, true);
	if (file)
	{
		xr_vector<LPCSTR> levels;

		xr_strcpy(process, file->r_string("setup", "run_process"));

		string256 cmd_s, cmd_c, master_server;
		xr_strcpy(cmd_s, file->r_string("setup", "cmd_server"));
		xr_strcpy(cmd_c, file->r_string("setup", "cmd_client"));
		last_port_client = file->r_u32("setup", "start_port");
		xr_strcpy(master_server, file->r_string("setup", "master_sv"));

		int cnt = file->r_u32("setup", "levels");



		for (int i = 1; i <= cnt; i++)
		{
			string128 name;
			sprintf(name, "%d", i);
			levels.push_back(file->r_string("levels", name));
		}

		for (auto l : levels)
		{			
			string256 level; 
			xr_strcpy(level, l);
			StartDedicatedServer(level, wf, process, cmd_c, cmd_s, master_server);
			//START_PARAMS ps(l, wf, process, cmd_c, cmd_s, master_server); 
			//sparams.push_back(ps);
		}

		levels.clear_and_free();
	}

	thread_spawn(ThreadUpdate, "MasterServer update", 0, this);
}

void xrServer::SendReciveMsgsFromCLient(MasterServerID id)
{
	NET_Packet P;
	P.w_begin(MASTER_SERVER_SERVER);
	P.w_u8(MASTER_SERVER_MSGS);
	inherited::Send_master_Packet_TO(id, P, net_flags(true, true));
}
					   
void xrServer::SendMsgToMasterServerCL(LPCSTR msg, MasterServerID id)
{
	NET_Packet packet;
	packet.w_begin(MASTER_SERVER_SERVER);
	packet.w_u8(MASTER_SERVER_CHAT_DATA);
	packet.w_stringZ(msg);
	inherited::Send_master_Packet_TO(id, packet, net_flags(true));
}

void xrServer::MasterServerRecive(NET_Packet packet, MasterServerID id)
{
	vec_entitys* vec = 0;

	for (auto sv : master_server_data)
	{
		if (sv.second.master_client_id == id)
		{
			vec = &sv.second.ents;
		}
	}
 
	u16 type;
	packet.r_begin(type);
	switch (type)
	{
		case (MASTER_SERVER_CLIENT):
		{
			u8 type; 
			packet.r_u8(type);

			if (type == MASTER_SERVER_CHAT_DATA)
			{
				shared_str msg;
				packet.r_stringZ(msg);
				Msg("MS client[%d] recive msg(%s)", id.value(), msg.c_str());
			}

			if (type == MASTER_SERVER_SPAWN_DATA)
			{
				
				CSE_ENTITY_MS ms;
				ms.Read(packet);
				vec->push_back(&ms);

				Msg("Recive Ent [%d][%s][%d] pos[%f][%f][%f]", ms.id, ms.s_name.c_str(), ms.id_parrent, ms.pos.x, ms.pos.y, ms.pos.z);
			} 

			if (type == MASTER_SERVER_CMD)
			{
				int cnt = packet.r_u32();
				for (int i = 0; i < cnt; i++)
				{
					shared_str msg;
					packet.r_stringZ(msg);
					Msg("%s", msg);
				}
			}

			if (type == MASTER_SERVER_MSGS)
			{
				shared_str msg;
				packet.r_stringZ(msg);
				Msg("ReciveMSG [%d] -> [%s]", id, msg.c_str());
			}

			if (type == MASTER_SERVER_RECVEST_MAP)
			{
				shared_str str;
				packet.r_stringZ(str);
				master_server_data[id.value()].map = str.c_str();
			}

			if (type == MASTER_SERVER_RECVEST_MEMORY)
			{
				int cnt = packet.r_u32();
				for (int i = 0; i < cnt; i++)
				{
					shared_str msg;
					packet.r_stringZ(msg);
					Msg("Server[%d], STATS: ", id.value(), msg);
				}
			}

		}break;
	}
}

void xrServer::SendCMD_To_Server(MasterServerID id, LPCSTR cmd)
{
	Msg("Send (%d) -> (%s)", id.value(), cmd);
	NET_Packet packet;
	packet.w_begin(MASTER_SERVER_SERVER);
	packet.w_u8(MASTER_SERVER_CMD);
	packet.w_stringZ(cmd);
	inherited::Send_master_Packet_TO(id, packet, net_flags(true, true));
}

void xrServer::SendKillProcess(MasterServerID id)
{
 	NET_Packet packet;
	packet.w_begin(MASTER_SERVER_SERVER);
	packet.w_u8(MASTER_SERVER_KILL_PROCESS);
 	inherited::Send_master_Packet_TO(id, packet, net_flags(true, true)); 
}

void xrServer::MasterServerConnected(MasterServerID id)
{
	client_DATA data;
	for (auto cl : getMasterCLS())
	{
		if (cl.id == id)
		{
			CopyMemory(data.IP_V4, cl.IP_V4, 48);
			data.master_client_id = id;
		}
	}

	master_server_data[id.value()] = data;
	
	NET_Packet P;
	P.w_begin(MASTER_SERVER_SERVER);
	P.w_u8(MASTER_SERVER_RECVEST_MAP);
	Level().Server->Send_master_Packet_TO(id, P, net_flags(true, true));

	//need_send_data.push_back(id.value());
}

void xrServer::MasterServerDisconnected(MasterServerID id)
{
	master_server_data.erase(id.value());
}

void xrServer::MasterServer_StatsMemory(MasterServerID id)
{
	NET_Packet P;
	P.w_begin(MASTER_SERVER_SERVER);
	P.w_u8(MASTER_SERVER_RECVEST_MEMORY);
	inherited::Send_master_Packet_TO(id, P, net_flags(true, true));
}


