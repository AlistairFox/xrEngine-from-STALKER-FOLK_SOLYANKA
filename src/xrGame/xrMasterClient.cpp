#include "StdAfx.h"
#include "Level.h"
#include "MasterServerEvents.h"
#include "../xrEngine/XR_IOConsole.h"

xr_vector<shared_str> log_CB;
bool enabled_send_to_master;

void console_CALLBACK(LPCSTR text)
{
	log_CB.push_back(text);
}
 
void console_CALLBACK_msg(LPCSTR text)
{
	NET_Packet P;
	P.w_begin(MASTER_SERVER_CLIENT);
	P.w_u8(MASTER_SERVER_MSGS);
	P.w_stringZ(text);
	Level().Send_MasterCL_Packet(P, net_flags(true));
}

void CLevel::MasterClientRecive(NET_Packet P, MasterServerID id)
{
	u16 type;
	P.r_begin(type);
	switch (type)
	{
		case (MASTER_SERVER_SERVER):
		{
			u8 type; P.r_u8(type);

			if (type == MASTER_SERVER_CHAT_DATA)
			{
				shared_str msg;
				P.r_stringZ(msg);
				Msg("MS client [%d] recive msg(%s)", id.value(), msg.c_str());
			}

			if (type == MASTER_SERVER_CMD)
			{
				shared_str cmd;
				P.r_stringZ(cmd);
			
				SetLogCB(console_CALLBACK);
				Console->ExecuteCommand(cmd.c_str());
				SetLogCB(0);

				NET_Packet packet;
				packet.w_begin(MASTER_SERVER_CLIENT);
				packet.w_u8(MASTER_SERVER_CMD);
				packet.w_u32(log_CB.size());

				for (auto msg : log_CB)
				{
					packet.w_stringZ(msg);
				}

				net_class::Send_MasterCL_Packet(packet, net_flags(true, true));
			}

			if (type == MASTER_SERVER_MSGS)
			{
				if (!enabled_send_to_master)
				{
					enabled_send_to_master = true;
					SetLogCB(console_CALLBACK_msg);
				}
				else
				{
					enabled_send_to_master = false;
					SetLogCB(0);
				}
			}

			if (type == MASTER_SERVER_KILL_PROCESS)
			{
				TerminateProcess(GetCurrentProcess(), 0);
			}

			if (type == MASTER_SERVER_RECVEST_MAP)
			{
				string128 name;
				_GetItem(Level().m_caServerOptions.c_str(),  1, name, '\\');

				NET_Packet P;
				P.w_begin(MASTER_SERVER_CLIENT);
				P.w_u8(MASTER_SERVER_RECVEST_MAP);
				P.w_stringZ(name);
				net_class::Send_MasterCL_Packet(P, net_flags(true, true));
			}

			if (type == MASTER_SERVER_RECVEST_MEMORY)
			{
				SetLogCB(console_CALLBACK_msg);
				Console->Execute("stat_memory");
			}

		}break;
	}
}

void CLevel::MasterClientSend(int type)
{
	if (type == 1)
	{
		for (auto ent : *Level().Server->GetEntitys())
		{
			NET_Packet p;
			p.w_begin(MASTER_SERVER_CLIENT);
			p.w_u8(MASTER_SERVER_SPAWN_DATA);
			CSE_ENTITY_MS ms;
			ms.SetDataFromCSE(ent.second);
			ms.Write(p);
			net_class::Send_MasterCL_Packet(p, net_flags(true));
			
		}
	}
}

void CLevel::SendMSG(LPCSTR msg, MasterServerID id)
{
	NET_Packet P;
	P.w_begin(MASTER_SERVER_CLIENT);
	P.w_u8(MASTER_SERVER_CHAT_DATA);
	P.w_stringZ(msg);
	net_class::Send_MasterCL_Packet(P, net_flags(true));
}