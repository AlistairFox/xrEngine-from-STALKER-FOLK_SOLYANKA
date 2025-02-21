#pragma once

#include "ui/UIWindow.h"
#include "ui/UIWndCallback.h"

class CUI3tButton;
class CUITextWnd;
class CUIFrameWindow;

class CUIInviteToSquadMsgBox : public CUIWindow, public CUIWndCallback
{
private:
	typedef CUIWindow inherited;

	CUIFrameWindow* m_UIFrame;
protected:
	CUI3tButton* m_UIButtonAccept;
	CUI3tButton* m_UIButtonDecline;

	CUITextWnd* m_UIStaticText;

	void		xr_stdcall		OnAccept(CUIWindow* w, void* d);
	void		xr_stdcall		OnDecline(CUIWindow* w, void* d);
public:
	CUIInviteToSquadMsgBox();
	virtual		~CUIInviteToSquadMsgBox();

	void	Init();
	void	OnInviteReceived(LPCSTR invite_message);
	void	Clear();
	void	SendJoinInSquad(u16 id);
	virtual void	SendMessage(CUIWindow* pWnd, s16 msg, void* pData);
};
