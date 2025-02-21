#include "stdafx.h"
#include "SteamNetClient.h"
#include "SteamNetServer.h"
#include "ip_address.h"

//TODO: лучше мастер сделать на пустой карте иле вообще без уровня просто соеденение

extern SteamNetClient* s_pCallbackInstance;

void Steam_con_change_Callback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	if (s_pCallbackInstance)
	{
		s_pCallbackInstance->OnSteamConChanged_MS(pInfo);
	}
}

void SteamNetClient::_Send_MasterCL_To_LL(void* data, u32 size, u32 dwFlags, u32 dwTimeout)
{
	net_Statistic.dwBytesSended += size;

	EResult result = m_pInterface->SendMessageToConnection(m_hConnection_MS, data, size, convert_flags_for_steam(dwFlags), nullptr);
	 
	if (result != k_EResultOK)
		Msg("! [MasterServerCL] [Worker]  ERROR: Failed to send net-packet, reason: %d", result);
}

void SteamNetClient::Send_MasterCL_Packet(NET_Packet packet, u32 dwFlags, u32 dwTimeout)
{
	_Send_MasterCL_To_LL(packet.B.data, packet.B.count, dwFlags, dwTimeout);
}

void SteamNetClient::PollIncomingMS_Messages()
{
	while (true)
	{
		if (m_pInterface == nullptr)
			return;
 
		ISteamNetworkingMessage* pInc = 0;
		int num = m_pInterface->ReceiveMessagesOnConnection(m_hConnection_MS, &pInc, 1);

		if (num <= 0)
			break;

		int size = pInc->m_cbSize;
		void* data = pInc->m_pData;
		MasterServerID id;
		id = pInc->m_conn;

		NET_Packet p;
		p.construct(data, size);

		MasterClientRecive(p, id);
	}
}

bool SteamNetClient::CreateConnectionMS(ClientConnectionOptions& opt)
{
	if (m_pInterface)
	{
		SteamNetworkingIPAddr serverAddr;
		serverAddr.Clear();
		serverAddr.ParseString(opt.master_server); 
		serverAddr.m_port = 28017;//opt.master_port ## SetDefault 28017
		Msg("Master: %s", opt.master_server);
		
		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)Steam_con_change_Callback);

		m_hConnection_MS = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);

		if (m_hConnection_MS)
		{
			Msg("[MasterServer] [Worker] Created");
			MasterServerClient.set(m_hConnection_MS);
 		}
	}

	return false;
}

void SteamNetClient::OnSteamConChanged_MS(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		Msg("[MasterServer] [Worker]  ERROR: %d (Если 5003 то лег Мастер Сервер (Авто закрытие всех серверов(workers) и нужен перезапуск))", pInfo->m_info.m_eEndReason);
		TerminateProcess(GetCurrentProcess(), 0);
	}break;

	case k_ESteamNetworkingConnectionState_Connected:
		Msg("[MasterServer] [Worker] Connected to server");
		//m_bWasConnected = true;
		//SendClientData();
		break;
	case k_ESteamNetworkingConnectionState_None:
		// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
		Msg("[MasterServer] [Worker]  Disconected Closing Worker");
		TerminateProcess(GetCurrentProcess(), 0);
		break;
	case k_ESteamNetworkingConnectionState_Connecting:
		// NOTE: We will get this callback when we start connecting. We can ignore this.
		break;
	default:
		Msg("! [MasterServer] [Worker]  unknown steam connection state %d", pInfo->m_info.m_eState);
		break;
	}

}

