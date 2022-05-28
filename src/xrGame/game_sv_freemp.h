#pragma once

#include "game_sv_mp.h"
#include "../xrEngine/pure_relcase.h"

#include "../jsonxx/jsonxx.h"
using namespace jsonxx;

struct Team
{
	ClientID players[4];
	ClientID ClientLeader = -1;
 	u16 cur_players = 0;
};

struct SpawnSect
{
	s32 StartMoney;
	xr_vector<xr_string> StartItems;
};

 
class xrServer;
class CALifeSimulator;

class game_sv_freemp : public game_sv_mp, private pure_relcase
{
	typedef game_sv_mp inherited;
	SpawnSect spawned_items;
	bool loaded_inventory = false;
	bool loaded_gametime = false;


	xr_map<ClientID, u32> map_alife_sended;

protected:
	CALifeSimulator* m_alife_simulator;


public:
	xr_map<ClientID, Team> teamPlayers;
	bool surge_started;
	
	
	game_sv_freemp();
	virtual							~game_sv_freemp();
	
	virtual		void				Create(shared_str &options);


	virtual		bool				UseSKin() const { return false; }

	virtual		LPCSTR				type_name() const { return "freemp"; };
	void __stdcall					net_Relcase(CObject* O) {};

	// helper functions
	void									AddMoneyToPlayer(game_PlayerState* ps, s32 amount);
	void									SetMoneyToPlayer(game_PlayerState* ps, s32 amount);
	void									SpawnItemToActor(u16 actorId, LPCSTR name);

	CSE_Abstract*							SpawnItemToActorReturn(u16 actorId, LPCSTR name);

	virtual		void				OnPlayerReady(ClientID id_who);
	virtual		void				OnPlayerConnect(ClientID id_who);
	virtual		void				OnPlayerConnectFinished(ClientID id_who);
	virtual		void				OnPlayerDisconnect(ClientID id_who, LPSTR Name, u16 GameID);

	virtual		void				OnPlayerKillPlayer(game_PlayerState* ps_killer, game_PlayerState* ps_killed, KILL_TYPE KillType, SPECIAL_KILL_TYPE SpecialKillType, CSE_Abstract* pWeaponA);

	virtual		void				OnEvent(NET_Packet &tNetPacket, u16 type, u32 time, ClientID sender);

	virtual		void				Update();

	virtual		BOOL				OnTouch(u16 eid_who, u16 eid_what, BOOL bForced = FALSE);

	bool		SpawnItemToPos(LPCSTR section, Fvector3 position);

	virtual		void				on_death(CSE_Abstract* e_dest, CSE_Abstract* e_src);

	virtual		void				OnPlayerTrade(NET_Packet &P, ClientID const & clientID);
	virtual		void				OnTransferMoney(NET_Packet &P, ClientID const & clientID);

	virtual		void				SavePlayer(game_PlayerState* cl);
	virtual		bool                LoadPlayer(game_PlayerState* id_who);
	virtual		bool				LoadPlayerPosition(game_PlayerState* ps, Fvector& position, Fvector& angle);

			//	bool				GetPosAngleFromActor(ClientID id, Fvector& Pos, Fvector& Angle);

				void				assign_RP(CSE_Abstract* E, game_PlayerState* ps_who);

				void				set_account_nickname(LPCSTR login, LPCSTR password, LPCSTR new_nickname, u32 team);

				int                 get_account_team(LPCSTR login, LPCSTR password);

				void				save_inventoryBox(CSE_Abstract* ent);
				void                load_inventoryBox(CSE_Abstract* ent);

	virtual void LoadParamsDeffaultFMP();


	void OnPlayerUIContacts(NET_Packet& P, ClientID const& clientID);
	void OnPlayerUIContactsRecvest(NET_Packet& P, ClientID const& clientID);
	void OnPlayerUIContactsRecvestUpdate(ClientID Client, ClientID leader);
	void OnPlayerUIContactsRemoveUser(ClientID Client, ClientID Leader);

 

	virtual	bool change_level(NET_Packet& net_packet, ClientID sender);
	void restart_simulator(LPCSTR saved_game_name);
	void save_game(NET_Packet& net_packet, ClientID sender);
	bool load_game(NET_Packet& net_packet, ClientID sender);

	virtual		ALife::_TIME_ID		GetStartGameTime();
	virtual		ALife::_TIME_ID		GetGameTime();
	virtual		float				GetGameTimeFactor();
	virtual		void				SetGameTimeFactor(const float fTimeFactor);

	virtual		void				ChangeGameTime(u32 day, u32 hour, u32 minute);
	virtual		void				ChangeGameTime(u32 value);

	virtual		ALife::_TIME_ID		GetEnvironmentGameTime();
	virtual		float				GetEnvironmentGameTimeFactor();
	virtual		void				SetEnvironmentGameTimeFactor(const float fTimeFactor);

	virtual		void				WriteAlifeObjectsToClient(ClientID id);
	virtual		void				UpdateAlifeObjects();

	virtual		void				RegisterUpdateAlife(CSE_ALifeDynamicObject* object, bool reg);
 
	IC			xrServer& server() const
	{
		VERIFY(m_server);
		return						(*m_server);
	}

	IC			CALifeSimulator& alife() const
	{
		VERIFY(m_alife_simulator);
		return						(*m_alife_simulator);
	}
};
