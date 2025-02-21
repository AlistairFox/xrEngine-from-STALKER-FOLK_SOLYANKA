#pragma once
#include "game_cl_mp.h"
#include "level_events.h"
#include "script_json_file.h"


class CUIGameFMP;
class CVoiceChat;
class CGameTask;

class game_cl_freemp : public game_cl_mp
{
private:

	typedef game_cl_mp inherited;

public:
	u32 m_last_net_per_second;
	u64 m_last_net_per_second_kb;
	
	struct UserDataNetworking {
		u64 SpawnNetSpawn = 0;
		u64 UpdateNet = 0;
		u64 UpdateNet_ps = 0;
	} data_networking_alife;

	
	bool alife_objects_synchronized;
 
	xr_hash_map<u16, CSE_ALifeDynamicObject*> alife_objects;

	CUIGameFMP* m_game_ui;
	level_events* l_events;
	
	float		Indicator_render1;
	float		Indicator_render2;
	Fvector		IndicatorPosition;
	Fvector     IndicatorPositionText;
	ui_shader	IndicatorShaderFreemp;
	ui_shader	IndicatorShaderFreempLeader;

	bool load_game_tasks;

			game_cl_freemp();
	virtual	~game_cl_freemp();

	virtual LPCSTR		GetTeamColor(u32 /*team*/) const { return "%c[255,255,240,190]"; }
	virtual u32				GetTeamColor_u32(u32 /*team*/) const { return color_rgba(255, 240, 190, 255); }

	virtual CUIGameCustom* createGameUI();
	virtual void SetGameUI(CUIGameCustom*);


	virtual	void net_import_state(NET_Packet& P);
	virtual	void net_import_update(NET_Packet& P);
	
	virtual void shedule_Update(u32 dt);


	virtual void shedule_voice();
	virtual void shedule_InventoryOwner();
	virtual void shedule_Quests();

	virtual float shedule_Scale();


	virtual	bool OnKeyboardPress(int key);
	virtual	bool OnKeyboardRelease(int key);

	virtual LPCSTR GetGameScore(string32&	score_dest);
	virtual bool Is_Rewarding_Allowed()  const { return false; };

	virtual void OnConnected();
 
	virtual	void TranslateGameMessage(u32 msg, NET_Packet& P);

	virtual void OnRender(); 

	virtual void ReadSpawnAlife(NET_Packet *packet);
	virtual void ReadUpdateAlife(NET_Packet *packet);
 
	virtual void OnScreenResolutionChanged();

	virtual CSE_ALifeDynamicObject* GetAlifeObject(u16 id)
	{
		auto object = alife_objects.find(id);
	
		if (object == alife_objects.end())
			return 0;

		return	(*object).second;
	};

	CParticlesObject* pobjec;

	void CreateParticle(LPCSTR name, Fvector3 pos);
	
	//TASKS
	void load_task(CGameTask* t);
	void save_task(CGameTask* t);

	void callback_load(CGameTask* t);
	void callback_save(CGameTask* t);

	CObjectJsonEx json_ex;
	

private:
	void OnVoiceMessage(NET_Packet* P);
	CVoiceChat* m_pVoiceChat = nullptr;


public:
	// XRMPE SQUAD SYSYTEM
	struct MP_SquadInvite
	{
		shared_str InviteMessage;
		u16	   InviterID;
		u32	   ReceivedTime;
	};

	struct MP_SquadCL
	{
		xr_vector<game_PlayerState*> players;
		ClientID squad_leader_cid;
		u16 id;
		shared_str current_quest;
		u16 current_map_point;
		bool need_update;
	};

	ui_shader			m_SquadLeaderShader;
	ui_shader			m_SquadMemberShader;
	xr_vector<MP_SquadInvite*> mp_squad_invites;

	ref_sound 	m_sndPDAmsg;

	MP_SquadCL* local_squad = nullptr;
	MP_SquadInvite*		FindInviteByInviterID(u16 gameid);
	void				RemoveInviteByInviterID(u16 gameid);

	void				OnSquadInvniteReceived(NET_Packet& P);
 	void				OnSquadCancelReceived(NET_Packet& P);

	bool				m_bSwitchToNextInvite;
	void GiveNews(LPCSTR caption, LPCSTR text, LPCSTR texture_name, int delay, int show_time, int type, bool noSound);
	void GiveNews(LPCSTR caption, LPCSTR text, LPCSTR texture_name, int delay, int show_time, int type);
	// END XRMPE SYSTEM
};
 

