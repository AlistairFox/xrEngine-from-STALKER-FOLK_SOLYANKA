#include "stdafx.h"
#include "SteamNetServer.h"

extern SteamNetServer* s_pCallbackInstance;

void MS_SV_ConnectionChange(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	if (s_pCallbackInstance)
		s_pCallbackInstance->OnMS_Changed(pInfo);
}

void SteamNetServer::CreateConnection_Master(ServerConnectionOptions& connectOpt)
{
	// SETUP SERVER PORT
	SteamNetworkingIPAddr bindServerAddress;
	bindServerAddress.Clear();
	bindServerAddress.m_port = 28017;

	// CREATE LISTENER
	SteamNetworkingConfigValue_t ms_opt;
	ms_opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)MS_SV_ConnectionChange);
	
	servers_sock = m_pInterface->CreateListenSocketIP(bindServerAddress, 1, &ms_opt);
	servers_pool = m_pInterface->CreatePollGroup();
 
}

void SteamNetServer::OnMS_Changed(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	switch (pInfo->m_info.m_eState)
	{
		case k_ESteamNetworkingConnectionState_Connecting:
			ProcessMasterServerConnection(pInfo);
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		{
			Msg("[MasterServer] Connection %s problem detected locally, reason %d: %s\n",
				pInfo->m_info.m_szConnectionDescription,
				pInfo->m_info.m_eEndReason,
				pInfo->m_info.m_szEndDebug
			);
			CloseConnectionMS(pInfo->m_hConn, EUnknownReasonMS);
 		}break;
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			Msg("[MasterServer] Connection %s closed by peer, reason %d: %s\n",
				pInfo->m_info.m_szConnectionDescription,
				pInfo->m_info.m_eEndReason,
				pInfo->m_info.m_szEndDebug
			);
			CloseConnectionMS(pInfo->m_hConn, EUnknownReasonMS);
 		}
			break;

		case k_ESteamNetworkingConnectionState_None:
		{
			char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
			pInfo->m_info.m_addrRemote.ToString(szAddr, sizeof(szAddr), true);
			Msg("[MasterServer] disconnected %s", szAddr);
			
			MasterServerDisconnected(GetMasterServerIDFromCon(pInfo->m_hConn));
		}break;

		case k_ESteamNetworkingConnectionState_Connected:
		{
			char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
			pInfo->m_info.m_addrRemote.ToString(szAddr, sizeof(szAddr), true);
			Msg("[MasterServer] connected %s", szAddr);


		}break;

		default:
			Msg("! [MasterServer] unknown steam connection state %d", pInfo->m_info.m_eState);
	}
}

void SteamNetServer::ProcessMasterServerConnection(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	auto iter = std::find(servers_connected.begin(), servers_connected.end(), pInfo->m_hConn);

	if (iter == servers_connected.end())
	{
		xrMasterServerClient cl(this);
		cl.id = last_id_MS_CL;
		cl.connection = pInfo->m_hConn;
		pInfo->m_info.m_addrRemote.ToString(cl.IP_V4, sizeof(cl.IP_V4), true);
		servers_MS.push_back(cl);
		last_id_MS_CL++;
		
		Msg("[MasterServer] connected sv_client(%d), ip:%s", cl.connection, cl.IP_V4);

		MasterServerConnected(cl.id);

		servers_connected.push_back(pInfo->m_hConn);
		 
		EResult con_res = m_pInterface->AcceptConnection(pInfo->m_hConn);

		if (con_res != k_EResultOK)
		{
			Msg("[MasterServer] Can't accept connection. Reason: %d", con_res);
			CloseConnectionMS(pInfo->m_hConn, EUnknownReasonMS);
			return;
		}	 

		if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, servers_pool))
		{
			Msg("[MasterServer] Close connection. Failed to set poll group");
			CloseConnectionMS(pInfo->m_hConn, EUnknownReasonMS);
			return;
		}
	}
	else
	{
		CloseConnectionMS(pInfo->m_hConn, EServerAlreadyConnected);
		ip_address ip;
		GetIpAddress(pInfo->m_info, ip);

		Msg("! [MasterServer] server %s, close connection (”же в списке servers_connected) ", ip.to_string());
	}		
}
 
void SteamNetServer::CloseConnectionMS(HSteamNetConnection connection, enmDisconnectReason_MS nReason, LPCSTR sReason)
{
	m_pInterface->CloseConnection(connection, nReason, sReason, false);
	int erase = -1;

	for (int i = 0; i < servers_MS.size(); i++)
	{
		if (servers_MS[i].connection = connection)
		{
			erase = i;
		}
	}
	if (erase != -1)
	{
		servers_MS.erase(servers_MS.begin() + erase);
		last_id_MS_CL--;
	}
}

void SteamNetServer::_Send_Master_To_LL(HSteamNetConnection ID, void* data, u32 size, u32 dwFlags, u32 dwTimeout)
{
	EResult res = m_pInterface->SendMessageToConnection(ID, data, size, convert_flags_for_steam(dwFlags), 0);
	if (res != k_EResultOK)
	{
		Msg("! [MasterServer] ERROR: Failed to send net-packet, reason: %d", res);
	}
}
	   
void SteamNetServer::Send_master_Packet_TO(MasterServerID ID, NET_Packet& P, u32 dwFlags)
{
 	_Send_Master_To_LL(GetConnectionFromID(ID), P.B.data, P.B.count, dwFlags, 0);
}

void SteamNetServer::_Send_For_All(NET_Packet& P, u32 dwFlags, u32 dwTimeout)
{
	for (auto ms : servers_MS)	 
		ms.SendToCL(P, dwFlags);
}
 
void SteamNetServer::CloseConnectionMS_Server(MasterServerID ID, enmDisconnectReason_MS nReason, LPCSTR sReason)
{
	CloseConnectionMS(ID.value(), nReason, sReason);
}

void SteamNetServer::PollIncomingMessagesMasterServer()
{
	while (true)
	{
		if (m_pInterface == nullptr)
			break;

		ISteamNetworkingMessage* pIncomingMsg = nullptr;

		int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(servers_pool, &pIncomingMsg, 1);
		if (numMsgs <= 0)
			break;		 	 

		void* data = pIncomingMsg->m_pData;
		int size = pIncomingMsg->m_cbSize;

		NET_Packet p;
		p.construct(data, size);

		MasterServerRecive(p, GetMasterServerIDFromCon(pIncomingMsg->m_conn));
	}
}

HSteamNetConnection SteamNetServer::GetConnectionFromID(MasterServerID& ID)
{
	HSteamNetConnection con = 0;
	for (auto cl : servers_MS)
	{
		if (cl.id.value() == ID.value())
			con = cl.connection;
	}
	return con;
}

MasterServerID SteamNetServer::GetMasterServerIDFromCon(HSteamNetConnection& con)
{
	MasterServerID id = -1;
	for (auto cl : getMasterCLS())
		if (cl.connection == con)
			id = cl.id;
	return id;
}

