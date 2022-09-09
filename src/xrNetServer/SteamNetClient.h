#pragma once
#include "BaseClient.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"
#pragma warning(pop)

class XRNETSERVER_API SteamNetClient : public BaseClient
{
	friend void ClSteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo);
	friend void CLSteamSessionFailed(SteamNetworkingMessagesSessionFailed_t* pInfo);

	friend void Steam_con_change_Callback(SteamNetConnectionStatusChangedCallback_t* pInfo);
	friend void steam_net_update_client(void* P);
 

private:
	xrCriticalSection		      csConnection;
	ISteamNetworkingSockets*	m_pInterface = nullptr;
	HSteamNetConnection		    m_hConnection = k_HSteamNetConnection_Invalid;


	xr_string									m_user_name = "";
	xr_string									m_user_pass = "";

	bool											m_bWasConnected = false;
	bool											m_bServerClient = false;
	bool											m_bMasterServer = false;
	bool											m_bGameDescriptionRecieved = false;

public:
	SteamNetClient(CTimer* tm);
	virtual ~SteamNetClient();

private:
	IC bool                 IsConnectionCreated() const { return m_pInterface != nullptr; }
	IC bool									GameDescriptionReceived() const { return m_bGameDescriptionRecieved; }

	void										SendClientData();

	void                    Update();
	void                    OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo);
	void					OnSteamNetConnectionFailed(SteamNetworkingMessagesSessionFailed_t *pInfo);


	void					          GetIpAddress(SteamNetConnectionInfo_t& info, ip_address& out);

	void					          PollConnectionStateChanges();
	void					          PollIncomingMessages();


protected:
	virtual bool            IsConnectionInit() override { return m_pInterface != nullptr; }

	virtual bool			CreateConnection(ClientConnectionOptions& opt) override;

	virtual void			DestroyConnection() override;
	virtual	void			SendTo_LL(void* data, u32 size, u32 dwFlags = DPNSEND_GUARANTEED, u32 dwTimeout = 0) override;

	virtual bool            GetPendingMessagesCount(DWORD& dwPending) override;
	virtual bool            SendPingMessage(MSYS_PING& clPing) override;

public:
	virtual	bool			GetServerAddress(ip_address& pAddress, DWORD* pPort)  override;

	virtual bool			      HasSessionName() { return GameDescriptionReceived(); }
	virtual LPCSTR			net_SessionName() const override { return m_game_description.map_name; }

	// statistic
	virtual	void			      UpdateStatistic()  override;

//MasterServer

private:
	HSteamNetConnection				m_hConnection_MS = k_HSteamNetConnection_Invalid;
 
	void							PollIncomingMS_Messages();
	virtual bool					CreateConnectionMS(ClientConnectionOptions& opt);
	void							OnSteamConChanged_MS(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void							MS_update();

	//Send MESSAGES
	void							_Send_MasterCL_To_LL(void* data, u32 size, u32 dwFlags, u32 dwTimeout);

public:
	MasterServerID					MasterServerClient;
	virtual void					MasterClientRecive(NET_Packet P, MasterServerID id) {};
	void							Send_MasterCL_Packet(NET_Packet P, u32 dwFlags = DPNSEND_GUARANTEED, u32 dwTimeout = 0);

};

