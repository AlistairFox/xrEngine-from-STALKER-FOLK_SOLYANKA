#pragma once

#include "game_sv_mp.h"
#include "../xrEngine/pure_relcase.h"

#include "../jsonxx/jsonxx.h"
using namespace jsonxx;
 
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
	u32 last_alife_update_time = 0;
	u32 last_alife_update_time_pos = 0;

	xr_map<ClientID, u32> map_alife_sended;
	u32 LastSaveFile = 0;

protected:
	CALifeSimulator* m_alife_simulator;




public:
 	bool surge_started;
	
	
	game_sv_freemp();
	virtual							~game_sv_freemp();
	
	virtual		void				Create(shared_str &options);


	virtual		bool				UseSKin() const { return false; }

	virtual		LPCSTR				type_name() const { return "freemp"; };
	void __stdcall					net_Relcase(CObject* O) {};

	// helper functions
	void							AddMoneyToPlayer(game_PlayerState* ps, s32 amount);
	void							SetMoneyToPlayer(game_PlayerState* ps, s32 amount);
	void							SpawnItemToActor(u16 actorId, LPCSTR name);

	CSE_Abstract*					SpawnItemToActorReturn(u16 actorId, LPCSTR name);

	virtual		void				OnPlayerReady(ClientID id_who);
	virtual		void				OnPlayerConnect(ClientID id_who);
	virtual		void				OnPlayerConnectFinished(ClientID id_who);
	virtual		void				OnPlayerDisconnect(ClientID id_who, LPSTR Name, u16 GameID);

	virtual		void				OnPlayerKillPlayer(game_PlayerState* ps_killer, game_PlayerState* ps_killed, KILL_TYPE KillType, SPECIAL_KILL_TYPE SpecialKillType, CSE_Abstract* pWeaponA);

	virtual		void				OnEvent(NET_Packet &tNetPacket, u16 type, u32 time, ClientID sender);

	virtual		void				Update();
 	virtual		void				UpdateAlifeData();

	virtual		BOOL				OnTouch(u16 eid_who, u16 eid_what, BOOL bForced = FALSE);

	bool							SpawnItemToPos(LPCSTR section, Fvector3 position);

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

	void RecivePdaChatMSG(NET_Packet& P, ClientID& sender);
	void RecivePdaContactMSG(NET_Packet& P, ClientID& sender);
 

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

	virtual		shared_str			level_name(const shared_str& server_options) const;
	virtual		shared_str			name_map_alife();

	// SYNC ALIFE DATA
	struct update_data 
	{
		Fvector3 pos; 
		u32 time;
	};

	xr_map<u16, update_data> old_export_pos;

	virtual		void				WriteAlifeObjectsToClient(ClientID id);
	
	virtual		void				UpdateAlifeObjects();
	virtual		void				UpdateAlifeObjectsPOS();

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

	void GetServerInfo(CServerInfo* info);

	// SQUAD 

	virtual		void	join_player_in_squad(NET_Packet& packet, u16 id);
	virtual		void	delete_player_from_squad(NET_Packet& packet, u16 id);
	virtual		void	delete_player_from_squad(u16 id);
	virtual		void	make_player_squad_leader(NET_Packet& packet, u16 id);
	 
	struct MP_Squad
	{
		xr_vector<game_PlayerState*> players;
		ClientID squad_leader_cid;
		u16 id;
		shared_str current_quest = "";
		u16 current_map_point;
	};
	xr_vector<MP_Squad*> mp_squads;

	MP_Squad* m_lobby_squad;


	MP_Squad* create_squad(game_PlayerState* ps);
	void delete_squad(u16 squad_id);
	MP_Squad* find_squad_by_squadid(u16 id);
	bool check_player_in_squad(game_PlayerState* ps, MP_Squad* squad);
	void SendMpSuqadToMembers(MP_Squad* squad);
	void delete_player_from_player_list(MP_Squad* squad, game_PlayerState* pPlayer);
	void find_new_squad_leader(u16 squad_id);
 
};
