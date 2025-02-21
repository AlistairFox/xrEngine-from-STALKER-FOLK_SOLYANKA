#pragma once

#include "ui/UIWindow.h"
#include "ui/UIPropertiesBox.h"
#include "ui/UIActorMenu.h"
#include "game_cl_freemp.h"

class CUIFrameWindow;
class CUIFrameLineWnd;
class CUIStatic;
class CUIAnimatedStatic;
class CUIScrollView;
class CPda;
class CUIActorMenu;
class CUITextWnd;
class CUIEditBox;
class CUI3tButton;
class CUIInviteToSquadMsgBox;
class CUISquadWnd;

class CUIPdaContactsWnd : public CUIWindow, public CUIWndCallback
{
	CUIPropertiesBox* m_UIPropertiesBox;
protected:
	void		xr_stdcall		ProcessPropertiesBoxClicked(CUIWindow* w, void* d);
private:
	typedef CUIWindow inherited;
	xr_vector<CPda*>	m_pda_list;

	CUIFrameWindow* m_header_background;
	CUIFrameWindow* m_contacts_frame;
	CUIFrameWindow* m_squad_frame;
	CUIFrameWindow* m_details_frame;
	CUIFrameWindow* m_private_chat_frame;
	CUIFrameWindow* m_private_chat_over;

	CUITextWnd* m_contacts_caption;
	CUITextWnd* m_squad_caption;
	CUITextWnd* m_details_caption;

	CUIStatic* m_private_chat_background;

	CUIEditBox* m_private_chat_editBox;
	CUI3tButton* m_private_chat_send_message;

public:
	CUIPdaContactsWnd();
	virtual void				SendMessage(CUIWindow* pWnd, s16 msg, void* pData);
	void						ShowPropertiesBox();
	void						HidePropertiesBox();

	enum { flNeedUpdate = (1 << 0), };

	virtual						~CUIPdaContactsWnd();

	void						Init();


	virtual void				Update();
	virtual void				Reset();

	virtual void				Show(bool status);

	void 						AddContact(CPda* pda);
	void 						RemoveContact(CPda* pda);
	void 						RemoveAll();
	void 						Reload();

	CUIScrollView* UIContactsList;
	CUIScrollView* UIDetailsList;
	CUIScrollView* UIPrivateChatList;

	CUIInviteToSquadMsgBox* m_invite_msg_box;

	CUISquadWnd* m_squad_wnd;

	Flags8						m_flags;
	u32							m_delay;
	u32							m_previous_time;

private:
	void						OnPropertyPdaSendInviteClicked(CPda* pda);
	void						OnPropertyPdaCancelInviteClicked(CPda* pda);

	game_cl_freemp* game;
};

#include "UIPdaListItem.h"
class CUIPdaContactItem :public CUIPdaListItem, public CUISelectable, public CUIWndCallback
{
	CUIPdaContactsWnd* m_cw;
public:
	CUIPdaContactItem(CUIPdaContactsWnd* cw);
	virtual						~CUIPdaContactItem();
	virtual void				SetSelected(bool b);
	virtual bool				OnMouseDown(int mouse_btn);

};