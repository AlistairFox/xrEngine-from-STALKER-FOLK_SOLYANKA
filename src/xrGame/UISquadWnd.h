#pragma once

#include "ui/UIWindow.h"
#include "ui/UIWndCallback.h"
#include "ui/UIPropertiesBox.h"

class CUIStatic;
struct game_PlayerState;
class CUIPdaSquadList;
class CUIPdaSquadItem;
class game_cl_freemp;

#define squad_size	4

class CUISquadWnd : public CUIWindow, public CUIWndCallback
{
	typedef CUIWindow inherited;

	CUIPropertiesBox* m_UIPropertiesBox;
public:
	CUISquadWnd();
	~CUISquadWnd();

	void	Init();

	virtual void				SendMessage(CUIWindow* pWnd, s16 msg, void* pData);
	void						ShowPropertiesBox();
	void						HidePropertiesBox();
	void 						AddSquadMember(game_PlayerState* pPlayer);
	void 						RemoveSquadMember(CUIPdaSquadItem* squad_item);
	void 						RemoveSquadMember(int member_number);
	void 						RemoveAll();

	CUIPdaSquadList* m_SquadList;

protected:
	void		xr_stdcall		ProcessPropertiesBoxClicked(CUIWindow* w, void* d);
private:
	int					pda_squad_item_distance;
	CUIFrameWindow* m_frame[squad_size];
	CUIStatic* m_cap[squad_size];
	CUIWindow* m_leader_icon;
	void				PropertiesBoxForLeader(bool b_self);
	void				PropertiesBoxForMember();
	game_cl_freemp* game;

}; // class CUISquadWnd

class CUICharacterInfo;
class CInventoryOwner;

class CUIPdaSquadItem : public CUIWindow, public CUISelectable, public CUIWndCallback
{
	CUISquadWnd* m_sw;
private:
	typedef CUIWindow inherited;

public:
	CUIPdaSquadItem(CUISquadWnd* sw);
	virtual				~CUIPdaSquadItem();
	virtual void		SetSelected(bool b);
	virtual bool		OnMouseDown(int mouse_btn);

	void		InitSquadItem(Fvector2 pos, Fvector2 size);
	void		InitCharacter(game_PlayerState* pPlayer);

	void* m_data;

protected:
	CUICharacterInfo* UIInfo;
};

class CUIPdaSquadList : public CUIWindow
{
private:
	typedef CUIWindow inherited;

public:
	CUIPdaSquadList();
	~CUIPdaSquadList();
	virtual	void	SetSelected(CUIWindow*);
	CUIWindow* GetSelected();
};