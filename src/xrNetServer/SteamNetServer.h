#pragma once
#include "BaseServer.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"
#include "GameNetworkingSockets/steam/isteamnetworkingutils.h"
#pragma warning(pop)

enum enmDisconnectReason
{
	EUnknownReason = k_ESteamNetConnectionEnd_App_Min + 1,
	EDetailedReason = k_ESteamNetConnectionEnd_App_Min + 2,
	EInvalidPassword = k_ESteamNetConnectionEnd_App_Min + 3,
	ESessionFull = k_ESteamNetConnectionEnd_App_Min + 4,
	EPlayerBanned = k_ESteamNetConnectionEnd_App_Min + 5,
	EServerShutdown = k_ESteamNetConnectionEnd_App_Min + 6
};

enum enmDisconnectReason_MS
{
	EUnknownReasonMS = k_ESteamNetConnectionEnd_App_Min + 1,
	EServerAlreadyConnected = k_ESteamNetConnectionEnd_App_Min + 2
};




class XRNETSERVER_API SteamNetServer : public BaseServer
{
	friend void steam_net_update_server(void* P);
	friend void SvSteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
	friend void MS_SV_ConnectionChange(SteamNetConnectionStatusChangedCallback_t* pInfo);

public:
	struct xrMasterServerClient
	{
		bool							net_ready;
		bool							net_Accepted;
		MasterServerID					id;
		HSteamNetConnection				connection;
		char							IP_V4[48];

		SteamNetServer* server;

		xrMasterServerClient(SteamNetServer* s) : server(s) {};

		virtual			~xrMasterServerClient() {};
		virtual void	Clear() { net_ready = false; net_Accepted = false; id.set(0); };
		void			SendToCL(NET_Packet& P, u32 flags) { server->Send_master_Packet_TO(id, P, flags); };
	};

	typedef xr_vector<xrMasterServerClient> v_MS_servers;

private:
	ISteamNetworkingSockets*	  m_pInterface = nullptr;
	HSteamListenSocket            m_hListenSock = k_HSteamListenSocket_Invalid;
	HSteamNetPollGroup            m_hPollGroup = k_HSteamListenSocket_Invalid;

	GameDescriptionData           m_game_description;

	xrCriticalSection             csConnection;
	xr_string                     m_server_password;
	u32														m_max_players = 0;
	bool													m_bServerClientConnected = false;

	// Using in update thread!
	xr_vector<HSteamNetConnection> m_players;
	xr_vector<SClientConnectData>  m_pending_clients;
public:
	SteamNetServer(CTimer* timer, BOOL	dedicated);
	virtual ~SteamNetServer();

private:
	bool					IsConnectionCreated() { return m_hListenSock != k_HSteamListenSocket_Invalid; }

	void					ProcessConnection(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void					FinishConnection(SClientConnectData& cl_data);

	void					AddPendingClient(SClientConnectData& cl_data);
	void					HandlePendingClients();
	void					OnClientDataReceived(HSteamNetConnection connection, SteamNetworkingIdentity& identity, MSYS_CLIENT_DATA* data);

	// update thread
	void					OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

	void			        Update();
	void			        PollConnectionStateChanges();
	void			        PollIncomingMessages();

	void			        DisconnectAll();
	void			        CloseConnection(HSteamNetConnection connection, enmDisconnectReason nReason = EUnknownReason, LPCSTR sReason = nullptr);
	void			        DestroyCleint(ClientID clientId);

	void			        GetIpAddress(SteamNetConnectionInfo_t& info, ip_address& out);

protected:
	virtual bool			CreateConnection(GameDescriptionData& game_descr, ServerConnectionOptions& opt) override;
	virtual void			DestroyConnection() override;

	virtual bool			GetClientPendingMessagesCount(ClientID ID, DWORD& dwPending) override;

	virtual void			_SendTo_LL(ClientID ID, void* data, u32 size, u32 dwFlags = DPNSEND_GUARANTEED, u32 dwTimeout = 0) override;

public:
	virtual void			UpdateClientStatistic(IClient* C) override;


	virtual bool			GetClientAddress(ClientID ID, ip_address& Address, DWORD* pPort = NULL) override;
	virtual bool			DisconnectClient(IClient* C, LPCSTR Reason) override;

protected:
	//Master Server Create And Send, Recive Message (Между серверами)
	virtual void			CreateConnection_Master(ServerConnectionOptions& connectOpt);
 	void					OnMS_Changed(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void					ProcessMasterServerConnection(SteamNetConnectionStatusChangedCallback_t* pInfo);

	HSteamNetPollGroup		servers_pool = k_HSteamNetPollGroup_Invalid;
	HSteamListenSocket      servers_sock = k_HSteamListenSocket_Invalid;
	int last_id_MS_CL = 0;
	v_MS_servers servers_MS;
	xr_vector<HSteamNetConnection>  servers_connected;

	void					CloseConnectionMS(HSteamNetConnection connection, enmDisconnectReason_MS nReason = EUnknownReasonMS, LPCSTR sReason = nullptr);
	
	void					_Send_Master_To_LL(HSteamNetConnection ID, void* data, u32 size, u32 dwFlags = DPNSEND_GUARANTEED, u32 dwTimeout = 0);
	void					_Send_For_All(NET_Packet& P, u32 dwFlags, u32 dwTimeout);

	void					CloseConnectionMS_Server(MasterServerID ID, enmDisconnectReason_MS nReason, LPCSTR sReason);

	void					PollIncomingMessagesMasterServer();
																  
	HSteamNetConnection		GetConnectionFromID(MasterServerID& ID);
	MasterServerID			GetMasterServerIDFromCon(HSteamNetConnection& con);
	
public:
	void					Send_master_Packet_TO(MasterServerID ID, NET_Packet& P, u32 dwFlags);
	virtual void			MasterServerRecive(NET_Packet, MasterServerID id) {};
	virtual void			MasterServerConnected(MasterServerID id) {};
	virtual void			MasterServerDisconnected(MasterServerID id) {};
	v_MS_servers			getMasterCLS() { return servers_MS; };
	bool					isMaster() { return strstr(Core.Params, "-ms"); };
};
