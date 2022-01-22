#pragma once
#include "ui/UIWindow.h"
#include "ui/UIWndCallback.h"

#include "game_sv_freemp.h"

class CUIFrameWindow;
class CUIStatic;
class CUICharacterInfo;
class CUIPropertiesBox;

struct TeamPlayer
{
	ClientID Client;
	u16 GameID = -1;
};

class CUIPda_Squad : public CUIWindow, public CUIWndCallback
{
private:
	typedef CUIWindow	inherited;


	u8 idxPlayer = 0;
	bool initPanel = false;
	ClientID selected_user;

	CUIFrameWindow* squad_wnd_team[4];

	CUIStatic* squad_cap_1;
	CUIStatic* squad_cap_2;
	CUIStatic* squad_cap_3;
	CUIStatic* squad_cap_4;

	CUICharacterInfo* character_team[4];

	CUIPropertiesBox* property_box_squad;

public:
	Team team_players;

	CUIPda_Squad();
	virtual ~CUIPda_Squad();


	void				Init();
	virtual void 		Show(bool status);
	virtual void		Update();
	virtual void		ResetAll();

	void				EventRecive(Team player_team);
	
	//PropertyBox
	virtual bool		OnMouseAction(float x, float y, EUIMessages mouse_action);
	void xr_stdcall		property_box_squad_clicked(CUIWindow* w, void* d);
	void				InitCallBacks();
	void				SendMessage(CUIWindow* pWnd, s16 msg, void* pData);

};

