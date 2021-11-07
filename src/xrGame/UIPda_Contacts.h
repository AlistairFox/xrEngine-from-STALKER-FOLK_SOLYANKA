#pragma once

#include "ui/UIWindow.h"
#include "ui/UIWndCallback.h"

class CUIXmlInit;
class CUIFrameWindow;
class CUIScrollView;
class CUITextWnd;
class CUIPda_Squad;
class CUIPropertiesBox;
class CUI3tButton;

class CUIPda_Contacts : public CUIWindow, public CUIWndCallback
{
private:
	typedef CUIWindow	inherited;

	//datatypes
	u32					m_delay;
	u32					m_previous_time;
	u32					old_size_players = 0;

	u16					id_actor;
	u32					id_client;

	u32					last_inviter;
	bool				invite_mode = false;
	u8                  UI_ID = 0;

public:

	CUIFrameWindow* m_background;
	CUIScrollView* contacts_list;
	CUITextWnd* contacts_caption;
	CUIFrameWindow* contacts_window;

	CUIPda_Squad* squad_UI;

	CUIFrameWindow* squad_UI_invite;
	CUITextWnd* squad_UI_invite_text;
	CUI3tButton* squad_UI_invite_yes;
	CUI3tButton* squad_UI_invite_no;

	CUITextWnd* squad_cap;
	CUIFrameWindow* squad_window;



	CUIPropertiesBox* property_box;


	//functions


	CUIPda_Contacts();
	virtual ~CUIPda_Contacts();


	void				Init();
	void				InitCallBacks();
	virtual void 		Show(bool status);
	virtual void		Update();
	virtual void		ResetAll();
	virtual bool		OnMouseAction(float x, float y, EUIMessages mouse_action);

	//void методы
	void xr_stdcall		property_box_clicked(CUIWindow* w, void* d);

	void xr_stdcall     button_yes(CUIWindow* w, void* d);
	void xr_stdcall     button_no(CUIWindow* w, void* d);

	void				SendMessage(CUIWindow* pWnd, s16 msg, void* pData);

	void EventRecive(NET_Packet& P);

	void SetInvite(LPCSTR name);
};


