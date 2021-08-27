#pragma once

#include "game_sv_mp.h"
#include "../xrEngine/pure_relcase.h"

struct SpawnSect
{
	s32 StartMoney;
	xr_vector<xr_string> StartItems;
//	xr_vector<xr_string> DefaultItems;
};

class game_sv_freemp : public game_sv_mp, private pure_relcase
{
	typedef game_sv_mp inherited;
	SpawnSect spawned_items;

public:
									game_sv_freemp();
	virtual							~game_sv_freemp();
	
	virtual		void				Create(shared_str &options);


	virtual		bool				UseSKin() const { return false; }

	virtual		LPCSTR				type_name() const { return "freemp"; };
	void __stdcall					net_Relcase(CObject* O) {};

	// helper functions
	void									AddMoneyToPlayer(game_PlayerState* ps, s32 amount);
	void									SpawnItemToActor(u16 actorId, LPCSTR name);

	virtual		void				OnPlayerReady(ClientID id_who);
	virtual		void				OnPlayerConnect(ClientID id_who);
	virtual		void				OnPlayerConnectFinished(ClientID id_who);
	virtual		void				OnPlayerDisconnect(ClientID id_who, LPSTR Name, u16 GameID);

	virtual		void				OnPlayerKillPlayer(game_PlayerState* ps_killer, game_PlayerState* ps_killed, KILL_TYPE KillType, SPECIAL_KILL_TYPE SpecialKillType, CSE_Abstract* pWeaponA);

	virtual		void				OnEvent(NET_Packet &tNetPacket, u16 type, u32 time, ClientID sender);

	virtual		void				Update();

	virtual		BOOL				OnTouch(u16 eid_who, u16 eid_what, BOOL bForced = FALSE);

	virtual		void				on_death(CSE_Abstract* e_dest, CSE_Abstract* e_src);

	virtual		void				OnPlayerTrade(NET_Packet &P, ClientID const & clientID);
	virtual		void				OnTransferMoney(NET_Packet &P, ClientID const & clientID);

	virtual		bool                LoadPlayer(ClientID id_who);
	virtual		bool				LoadPlayerPosition(game_PlayerState* ps, Fvector& position, Fvector& angle);

				bool				GetPosAngleFromActor(ClientID id, Fvector& Pos, Fvector& Angle);

				void				assign_RP(CSE_Abstract* E, game_PlayerState* ps_who);

	virtual void LoadParamsDeffaultFMP();

};
