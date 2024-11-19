#pragma once
/*
#include "ui/UIWindow.h"
#include "ui/UIWndCallback.h"

class CUIFrameWindow;
class CUIStatic;
class CUICharacterInfo;
class CUIPropertiesBox;

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
 
	CUIPda_Squad();
	virtual ~CUIPda_Squad();

	xr_vector<u32> players;
	u32 leader_id = 0;
	bool in_squad = false;
	bool leader_in_squad = false;

	void				Init();
	virtual void 		Show(bool status);
	virtual void		Update();
	virtual void		ResetAll();

	void				EventRecive(NET_Packet packet);
	
	//PropertyBox
	virtual bool		OnMouseAction(float x, float y, EUIMessages mouse_action);
	void xr_stdcall		property_box_squad_clicked(CUIWindow* w, void* d);
	void				InitCallBacks();
	void				SendMessage(CUIWindow* pWnd, s16 msg, void* pData);

};


*/