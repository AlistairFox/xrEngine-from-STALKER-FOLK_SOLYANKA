#pragma once

#include "game_base.h"
#include "alife_space.h"
#include "script_export_space.h"
#include "../xrCore/client_id.h"
#include "game_sv_base_console_vars.h"
#include "game_sv_event_queue.h"
#include "game_sv_item_respawner.h"

#define MAX_PLAYERS_COUNT 128

enum ERoundEnd_Result
{
	eRoundEnd_Finish		= u32(0),
	eRoundEnd_GameRestarted,
	eRoundEnd_GameRestartedFast,
	eRoundEnd_TimeLimit,
	eRoundEnd_FragLimit,
	eRoundEnd_ArtrefactLimit,
	eRoundEnd_Force			= u32(-1)
};

class IClient;
class CSE_Abstract;
class xrServer;
// [OLES] Policy:
// it_*		- means ordinal number of player
// id_*		- means player-id
// eid_*	- means entity-id
class GameEventQueue;

class	game_sv_GameState	: public game_GameState
{
	typedef game_GameState inherited;
protected:
	xrServer*						m_server;
	
	GameEventQueue*					m_event_queue;
	item_respawn_manager			m_item_respawner;
		
	//Events
	virtual		void				OnEvent					(NET_Packet &tNetPacket, u16 type, u32 time, ClientID sender );

	virtual		void				ReadOptions				(shared_str &options);
	virtual		void				ConsoleCommands_Create	();
	virtual		void				ConsoleCommands_Clear	();

	struct SMapRot{
		shared_str	map_name;
		shared_str	map_ver;
	};
	DEF_DEQUE(MAP_ROTATION_LIST,	SMapRot);
	bool							m_bMapRotation;
	bool							m_bMapNeedRotation;
	bool							m_bMapSwitched;
	bool							m_bFastRestart;
	MAP_ROTATION_LIST				m_pMapRotation_List;

public:
#define		TEAM_COUNT 32

	BOOL							sv_force_sync;
	float							rpoints_MinDist [TEAM_COUNT];
	xr_vector<RPoint>				rpoints	[TEAM_COUNT];
	DEF_VECTOR(RPRef, RPoint*);
	RPRef							rpointsBlocked;
  	ERoundEnd_Result				round_end_reason;
	
	virtual		void				SaveMapList				();
	virtual		bool				HasMapRotation			() {return m_bMapRotation; };

				bool				FindPlayerName			(char const * name, IClient const * to_exclude);
				void				GenerateNewName			(char const * old_name, char * dest, u32 const dest_size);
				void				CheckPlayerName			(xrClientData* CL);
public:
	virtual		void				OnPlayerConnect			(ClientID id_who);
	virtual		void				OnPlayerDisconnect		(ClientID id_who, LPSTR Name, u16 GameID);
	virtual		void				OnPlayerReady			(ClientID id_who)							   {};
	virtual		void				OnPlayerEnteredGame		(ClientID id_who)	{};
	virtual		void				OnPlayerConnectFinished	(ClientID id_who)	{};
	virtual		void				OnPlayerFire			(ClientID id_who, NET_Packet &P) {};
	virtual		void				OnPlayer_Sell_Item		(ClientID id_who, NET_Packet &P) {};
				void				GenerateGameMessage		(NET_Packet &P);
	

	virtual		void				OnRoundStart			();									// ����� ������
	virtual		void				OnRoundEnd				();	//	round_end_reason			// ����� ������

				void				MapRotation_AddMap		(LPCSTR MapName, LPCSTR MapVer);
				void				MapRotation_ListMaps	();
	virtual		bool				OnNextMap				()									{return false;}
	virtual		void				OnPrevMap				()									{}
	virtual		bool				SwitchToNextMap			()	{ return m_bMapNeedRotation; };
	
	virtual		BOOL				IsVotingEnabled			();
	virtual		BOOL				IsVotingEnabled			(u16 flag);
	virtual		bool				IsVotingActive			()	{ return false; };
	virtual		void				SetVotingActive			( bool Active )	{ };
	virtual		void				OnVoteStart				(LPCSTR VoteCommand, ClientID sender)			{};
	virtual		void				OnVoteStop				()				{};

public:
									game_sv_GameState		();
	virtual							~game_sv_GameState		();

	// Main accessors
	// PLAYER STATE GETTERS
	virtual		game_PlayerState*	get_eid					(u16 id);
	virtual		game_PlayerState*	get_id					(ClientID id);

	// PLAYER CLIENT GETTER 
	virtual		void* get_client(u16 id); //if exist

	// PLAYER NAME
	virtual		LPCSTR				get_name_id				(ClientID id);								
				LPCSTR				get_player_name_id		(ClientID id);

	// ID TO GAMEID
	virtual		u16					get_id_2_eid			(ClientID id);
	virtual		u32					get_players_count		();

	// CSE ENTITY FROM ID
				CSE_Abstract*		get_entity_from_eid		(u16 id);


				RPoint				getRP					(u16 team_idx, u32 rp_idx);
				u32					getRPcount				(u16 team_idx);
	// Signals
	virtual		void				signal_Syncronize		();
	virtual		void				assign_RP				(CSE_Abstract* E, game_PlayerState* ps_who);
	virtual		bool				IsPointFreezed			(RPoint* rp);
	virtual		void				SetPointFreezed			(RPoint* rp);

#ifdef DEBUG
	virtual		void				OnRender				();
#endif
	
	virtual		void				OnSwitchPhase			(u32 old_phase, u32 new_phase);	
				CSE_Abstract*		spawn_begin				(LPCSTR N);
				CSE_Abstract*		spawn_end				(CSE_Abstract* E, ClientID id);

	// Utilities
	float							get_option_f			(LPCSTR lst, LPCSTR name, float def = 0.0f);
	s32								get_option_i			(LPCSTR lst, LPCSTR name, s32 def = 0);
	string64&						get_option_s			(LPCSTR lst, LPCSTR name, LPCSTR def = 0);
	virtual		u32					get_alive_count			(u32 team);
	virtual		xr_vector<u16>*		get_children			(ClientID id_who);
	void							u_EventGen				(NET_Packet& P, u16 type, u16 dest	);
	void							u_EventSend				(NET_Packet& P, u32 dwFlags = DPNSEND_GUARANTEED);

	// Events
	virtual		BOOL				OnPreCreate				(CSE_Abstract* E)				{return TRUE;};
	virtual		void				OnCreate				(u16 id_who)					{};
	virtual		void				OnPostCreate			(u16 id_who)					{};
	virtual		BOOL				OnTouch					(u16 eid_who, u16 eid_target, BOOL bForced = FALSE)	= 0;			// TRUE=allow ownership, FALSE=denied
	virtual		void				OnDetach				(u16 eid_who, u16 eid_target)	= 0;
	virtual		BOOL				OnActivate				(u16 eid_who, u16 eid_target)	{return TRUE;};
	virtual		void				OnAlifeCreate(CSE_Abstract* E) {};


	virtual		void				OnDestroyObject			(u16 eid_who);			

	virtual		void				OnHit					(u16 id_hitter, u16 id_hitted, NET_Packet& P);	//���-�� ������� Hit
	virtual		void				OnPlayerHitPlayer		(u16 id_hitter, u16 id_hitted, NET_Packet& P){}; //����� ������� Hit

	// Main
	virtual		void				Create					(shared_str& options);
	virtual		void				Update					();
	virtual		void				net_Export_State		(NET_Packet& P, ClientID id_to);				// full state
	virtual		void				net_Export_Update		(NET_Packet& P, ClientID id_to, ClientID id);		// just incremental update for specific client
	virtual		void				net_Export_GameTime		(NET_Packet& P);						// update GameTime only for remote clients

	virtual		bool				change_level			(NET_Packet &net_packet, ClientID sender);
	virtual		void				save_game				(NET_Packet &net_packet, ClientID sender);
	virtual		bool				load_game				(NET_Packet &net_packet, ClientID sender);
	virtual		void				reload_game				(NET_Packet &net_packet, ClientID sender);
	virtual		void				switch_distance			(NET_Packet &net_packet, ClientID sender);

				void				AddDelayedEvent			(NET_Packet &tNetPacket, u16 type, u32 time, ClientID sender );
				void				ProcessDelayedEvent		();
				//this method will delete all events for entity that already not exist (in case when player was kicked)
				void				CleanDelayedEventFor	(u16 id_entity_victim);
				void				CleanDelayedEventFor	(ClientID const & clientId);
				void				CleanDelayedEvents		();

	virtual		BOOL				isFriendlyFireEnabled	()	{return FALSE;};
	virtual		BOOL				CanHaveFriendlyFire		()	= 0;
	virtual		void				teleport_object			(NET_Packet &packet, u16 id);
	virtual		void				add_restriction			(NET_Packet &packet, u16 id);
	virtual		void				remove_restriction		(NET_Packet &packet, u16 id);
	virtual		void				remove_all_restrictions	(NET_Packet &packet, u16 id);
	virtual		bool				custom_sls_default		() {return false;};
	virtual		void				sls_default				() {};
	virtual		shared_str			level_name				(const shared_str &server_options) const;

	virtual		shared_str			name_map_alife() { return "no_map"; };
	
	static		shared_str			parse_level_name		(const shared_str &server_options);	
	static		shared_str			parse_level_version		(const shared_str &server_options);

	virtual		void				on_death				(CSE_Abstract *e_dest, CSE_Abstract *e_src);

	virtual		void				DumpOnlineStatistic		(){};
				
				bool				CheckNewPlayer			(xrClientData* CL);



	// XRMPE 

				virtual		void				join_player_in_squad(NET_Packet& packet, u16 id)					{};
				virtual		void				delete_player_from_squad(NET_Packet& packet, u16 id)				{};
				virtual		void				make_player_squad_leader(NET_Packet& packet, u16 id)				{};
				
		 
};
