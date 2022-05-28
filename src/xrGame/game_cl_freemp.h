#pragma once
#include "game_cl_mp.h"

class CUIGameFMP;
class CVoiceChat;

class game_cl_freemp :public game_cl_mp
{
private:
	typedef game_cl_mp inherited;

public:
	bool alife_objects_synchronized;
	bool alife_objects_registered;

	xr_map<u16, CSE_ALifeDynamicObject*> alife_objects;

	CUIGameFMP* m_game_ui;

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

	virtual	bool OnKeyboardPress(int key);
	virtual	bool OnKeyboardRelease(int key);

	virtual LPCSTR GetGameScore(string32&	score_dest);
	virtual bool Is_Rewarding_Allowed()  const { return false; };

	virtual void OnConnected();
	virtual bool OnConnectedSpawnPlayer();

	virtual	void TranslateGameMessage(u32 msg, NET_Packet& P);

	virtual void OnRender(); 

	virtual void ReadSpawnAlife(NET_Packet *packet);
	virtual void ReadUpdateAlife(NET_Packet *packet);
	virtual void RegisterObjectsAfterSpawn();

	virtual void OnScreenResolutionChanged();

	virtual CSE_ALifeDynamicObject* GetAlifeObject(u16 id) { return alife_objects[id]; };


private:
	void OnVoiceMessage(NET_Packet* P);
	CVoiceChat* m_pVoiceChat = nullptr;

};

