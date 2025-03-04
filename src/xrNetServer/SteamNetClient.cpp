#include "stdafx.h"
#include "SteamNetClient.h"
#include "SteamNetServer.h"
#include "ip_address.h"
#include "..\..\SDK\include\GameNetworkingSockets\steam\steamnetworkingtypes.h"

extern SteamNetClient* s_pCallbackInstance = nullptr;
XRNETSERVER_API int	simulate_netwark_ping_cl = 0;		// FPS

#pragma region CALLBACKS

void ClSteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
	if (s_pCallbackInstance)
	{
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}
}

void CLSteamSessionFailed(SteamNetworkingMessagesSessionFailed_t *pInfo)
{
	if (s_pCallbackInstance)
	{
		s_pCallbackInstance->OnSteamNetConnectionFailed(pInfo);
	}
}

// -----------------------------------------------------------------------------

void steam_net_update_client(void* P)
{
	SetThreadDescription(GetCurrentThread(), L"X-RAY client thread");

	Msg("- [SteamNetClient] Thread for steam network client is started");
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	SteamNetClient*	C = (SteamNetClient*)P;
	C->Update();
}

#pragma endregion  



// -----------------------------------------------------------------------------

SteamNetClient::SteamNetClient(CTimer* tm) : BaseClient(tm)
#ifdef PROFILE_CRITICAL_SECTIONS
	, csConnection(MUTEX_PROFILE_ID(SteamNetClient::csConnection))
#endif // PROFILE_CRITICAL_SECTIONS
{
}

// -----------------------------------------------------------------------------

SteamNetClient::~SteamNetClient()
{

}

// -----------------------------------------------------------------------------

#pragma region connect / disconnect

bool master_server = false;

bool SteamNetClient::CreateConnection(ClientConnectionOptions & connectOpt)
{
	m_bWasConnected = false;
	m_pInterface = SteamNetworkingSockets();

	bool isLocal = strstr(Core.LocalIP, "localhost") != 0;

	if (m_pInterface != nullptr && isLocal)
	{
		// server client
		SteamNetworkingIdentity identity;
		m_pInterface->GetIdentity(&identity);
		R_ASSERT2(identity.IsLocalHost(), "! [SteamNetClient] Interface is initialized and identity is not localhost!");
		m_bServerClient = true;
	}
	else
	{
		SteamNetworkingIdentity identity;
		identity.Clear();
		if (!identity.SetGenericString(connectOpt.server_pass))
		{
			Msg("! [SteamNetClient] server password is too long!");
			return false;
		}

		SteamDatagramErrMsg errMsg;
		if (!GameNetworkingSockets_Init(&identity, errMsg))
		{
			Msg("! [SteamNetClient] GameNetworkingSockets_Init failed.  %s", errMsg);
			return false;
		}

		m_pInterface = SteamNetworkingSockets();

		if (m_pInterface == nullptr)
		{
			Msg("! [SteamNetClient] Server interface is NULL");
			return false;
		}
	}

	SteamNetworkingIPAddr serverAddr;
	serverAddr.Clear();

	int sv_port = connectOpt.sv_port;
#pragma warning(disable:4996)
	if (stricmp(connectOpt.server_name, "localhost") == 0) // 127.0.0.1 ?!
	{
		serverAddr.SetIPv6LocalHost((uint16)sv_port);
	}
	else
	{
		serverAddr.ParseString(connectOpt.server_name);
		serverAddr.m_port = (uint16)sv_port;
	}

	char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
	serverAddr.ToString(szAddr, sizeof(szAddr), true);
	Msg("[SteamNetClient] Connecting to server at %s", szAddr);

	if (simulate_netwark_ping_cl > 0)
	{
		int ping = 0;

		ping = simulate_netwark_ping_cl;

		SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Send, ping);
		SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Recv, ping); 
	}

	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_SendBufferSize, 1024 * 1024 * 10);
	//SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_TimeoutInitial, 30000);
	//SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_TimeoutConnected, 30000);

	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)ClSteamNetConnectionStatusChangedCallback);
	//opt.SetPtr(k_ESteamNetworkingConfig_Callback_MessagesSessionFailed, (void*)CLSteamSessionFailed);

	m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);

	if (m_hConnection == k_HSteamNetConnection_Invalid)
	{
		Msg("! [SteamNetClient] Failed to create connection");
		return false;
	}

	Msg("m_hConnection: %d", m_hConnection);

	m_user_name = connectOpt.user_name;
	m_user_pass = connectOpt.user_pass;					 

	thread_spawn(steam_net_update_client, "snetwork-update-client", 0, this);

	PollConnectionStateChanges();

	//Connect To Second Server To recive Data (stalkers pos, player pos, for quest, maps, ets )	 (2 server cross sync)
	//Меж серверное взаимодействие для передачи данных от серверов (позиций, квестов, карты, синхра)
	// (Пока в разработке)
//	if (xr_strlen(connectOpt.master_server) > 1)
//	{
//		CreateConnectionMS(connectOpt);
//		master_server = true;
//	}
//	else
//		master_server = false;

	return true;
}

extern bool ServerHost;

void SteamNetClient::DestroyConnection()
{
	Msg("- [SteamNetClient] destroy connection");

	Msg("SyncServ Recived: %s", GameDescriptionReceived() ? "true" : "false");
 
	xrCriticalSection::raii lock(&csConnection);

	net_Disconnected = TRUE;

	if (m_pInterface == nullptr)
	{
		return;
	}

	if (m_hConnection != k_HSteamNetConnection_Invalid)
	{
		m_pInterface->CloseConnection(m_hConnection, 0, nullptr, false);
		m_hConnection = k_HSteamNetConnection_Invalid;
	}

	if (!ServerHost)
		m_pInterface = nullptr;

	m_user_name.clear();
	m_user_pass.clear();

	// if no server client
	if (!m_bServerClient)
	{
		GameNetworkingSockets_Kill();
	}
}

#pragma endregion

// -----------------------------------------------------------------------------

#pragma region receive
 
void SteamNetClient::Update()
{
	CTimer t;
	t.Start();
	while (true)
	{
		csConnection.Enter();

		if (!IsConnectionCreated())
		{
			csConnection.Leave();
			break;
		}
 		/*
		if (!GameDescriptionReceived() && !m_bWasConnected && t.GetElapsed_ms() % 5000 == 0)
		{
			SteamNetConnectionRealTimeStatus_t status;
			m_pInterface->GetConnectionRealTimeStatus(m_hConnection, &status, 0, nullptr);
			if (status.m_eState == k_ESteamNetworkingConnectionState_Connected)
			{
				m_bWasConnected = true;
				SendClientData();
			}
		}
		*/
 
		PollIncomingMessages();
		//MasterServer
		//if (master_server)
		//	PollIncomingMS_Messages();
		//END
		PollConnectionStateChanges();

		csConnection.Leave();
		Sleep(1);
	}
}

void SteamNetClient::PollIncomingMessages()
{
	while (true)
	{
		if (m_pInterface == nullptr || m_hConnection == k_HSteamNetConnection_Invalid)
			break;

		ISteamNetworkingMessage *pIncomingMsg = nullptr;
		int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);

		if (numMsgs <= 0)
		{
			break;
		}

		void* data = pIncomingMsg->m_pData;
		int size = pIncomingMsg->m_cbSize;

		if (!GameDescriptionReceived())
		{
			MSYS_GAME_DESCRIPTION*	sys_gd = (MSYS_GAME_DESCRIPTION*)data;
			if (
				size == sizeof(MSYS_GAME_DESCRIPTION) &&
				sys_gd->sign1 == 0x02281488 &&
				sys_gd->sign2 == 0x01488228
				)
			{
				CopyMemory(&m_game_description, &sys_gd->data, sizeof(m_game_description));
				m_bGameDescriptionRecieved = true;

				pIncomingMsg->Release();
				continue;
			}
		}

		MultipacketReciever::RecievePacket(pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
		pIncomingMsg->Release();
	}
}

void SteamNetClient::PollConnectionStateChanges()
{
	s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
}

void SteamNetClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo)
{
	switch (pInfo->m_info.m_eState)
	{
		Msg("StatusChanged: %d", pInfo->m_info.m_eState);

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
				Msg("[SteamNetClient] ClosedByPeer");
			if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				Msg("[SteamNetClient] ProblemDetectedLocally");
		
		
			Msg("SyncServ Recived: %s", GameDescriptionReceived() ? "true" : "false");

			net_Connected = EnmConnectionFails;
			net_Disconnected = TRUE;

			Msg("[SteamNetClient] Reason: %d", pInfo->m_info.m_eEndReason);
		
			switch (pInfo->m_info.m_eEndReason)
			{
				case k_ESteamNetConnectionEnd_Misc_Timeout:
					if (m_bWasConnected)
						OnSessionTerminate("st_lost_connection");
					else
						OnInvalidHost();
					break;
				case EUnknownReason:
					OnSessionTerminate("Unknown");
					break;
				case EPlayerBanned:
					OnSessionTerminate("Banned");
					break;
				case EServerShutdown:
					OnSessionTerminate("st_server_shutdown");
					break;
				case EDetailedReason:
					OnSessionTerminate(pInfo->m_info.m_szEndDebug);
					break;
				case EInvalidPassword:
					OnInvalidPassword();
					break;
				case ESessionFull:
					OnSessionFull();
					break;
				default:
					OnSessionTerminate(pInfo->m_info.m_szEndDebug);
					break;
			}
 		 
		}
		break;
		case k_ESteamNetworkingConnectionState_Connected:
			Msg("[SteamNetClient] Connected to server");
			m_bWasConnected = true;
			SendClientData();
			break;
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;
		case k_ESteamNetworkingConnectionState_Connecting:
			// NOTE: We will get this callback when we start connecting. We can ignore this.
 
			break;
		default:
			Msg("! [SteamNetClient] unknown steam connection state %d", pInfo->m_info.m_eState);
			break;
	}
}
void SteamNetClient::OnSteamNetConnectionFailed(SteamNetworkingMessagesSessionFailed_t* pInfo)
{
	Msg("OnSteamNetConnectionFailed");
}
#pragma endregion

// -----------------------------------------------------------------------------

void SteamNetClient::SendClientData()
{
	MSYS_CLIENT_DATA client_data;
	client_data.sign1 = 0x02281488;
	client_data.sign2 = 0x01488228;
	client_data.process_id = GetCurrentProcessId();
	xr_strcpy(client_data.name, m_user_name.c_str());
	xr_strcpy(client_data.pass, m_user_pass.c_str());

	SendTo_LL(&client_data, sizeof(MSYS_CLIENT_DATA), net_flags(TRUE, TRUE, FALSE, FALSE));
}

// -----------------------------------------------------------------------------

#pragma region send

void SteamNetClient::SendTo_LL(void * data, u32 size, u32 dwFlags, u32 dwTimeout)
{
	net_Statistic.dwBytesSended += size;

	EResult result = m_pInterface->SendMessageToConnection(m_hConnection, data, size, convert_flags_for_steam(dwFlags), nullptr);
	if (result != k_EResultOK)
		Msg("! [SteamNetClient] ERROR: Failed to send net-packet, reason: %d", result);
}

bool SteamNetClient::SendPingMessage(MSYS_PING & clPing)
{
	R_ASSERT(m_pInterface);

	EResult result = m_pInterface->SendMessageToConnection(
		m_hConnection,
		LPBYTE(&clPing),
		sizeof(clPing),
		k_nSteamNetworkingSend_UnreliableNoNagle,
		nullptr
	);

	return result == k_EResultOK;
}

#pragma endregion

// -----------------------------------------------------------------------------

#pragma region statistic

void SteamNetClient::UpdateStatistic()
{
	SteamNetConnectionRealTimeStatus_t status;
	if (!m_pInterface->GetConnectionRealTimeStatus(m_hConnection, &status, 0, nullptr))
	{
		return;
	}
	net_Statistic.Update(status);
}

#pragma endregion

// -----------------------------------------------------------------------------

#pragma region pending messages

bool SteamNetClient::GetPendingMessagesCount(DWORD& dwPending)
{
	R_ASSERT(m_pInterface);
	R_ASSERT(m_hConnection != k_HSteamNetConnection_Invalid);

	SteamNetConnectionRealTimeStatus_t pStatus;
	if (m_pInterface->GetConnectionRealTimeStatus(m_hConnection, &pStatus, 0, nullptr))
	{
		dwPending = pStatus.m_cbPendingReliable + pStatus.m_cbPendingUnreliable;
		return true;
	}

	return false;
}

#pragma endregion

// -----------------------------------------------------------------------------

#pragma region ip address


void SteamNetClient::GetIpAddress(SteamNetConnectionInfo_t & info, ip_address & out)
{
	string64 ip_str;
	info.m_addrRemote.ToString(ip_str, sizeof(string64), false);
	out.set(ip_str);
}

bool SteamNetClient::GetServerAddress(ip_address& pAddress, DWORD* pPort)
{
	*pPort = 0;

	SteamNetConnectionInfo_t pInfo;
	if (m_pInterface->GetConnectionInfo(m_hConnection, &pInfo))
	{
		GetIpAddress(pInfo, pAddress);
		*pPort = pInfo.m_addrRemote.m_port;
		return true;
	}

	return false;
};

#pragma endregion

// -----------------------------------------------------------------------------
