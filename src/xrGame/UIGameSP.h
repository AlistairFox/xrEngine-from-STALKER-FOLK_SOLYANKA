#pragma once
#include "uigamecustom.h"
#include "ui/UIDialogWnd.h"
#include "game_graph_space.h"

class CUITradeWnd;	
class CInventory;

class game_cl_Single;
class CChangeLevelWnd;
class CUIMessageBox;
class CInventoryOwner;

class CUIGameSP : public CUIGameCustom
{
private:
	game_cl_Single*		m_game;
	typedef CUIGameCustom inherited;
public:
	CUIGameSP									();
	virtual				~CUIGameSP				();

	virtual void		SetClGame				(game_cl_GameState* g);
	virtual bool		IR_UIOnKeyboardPress	(int dik);
	virtual void _BCL	OnFrame					();

	void				ChangeLevel				(GameGraph::_GRAPH_ID game_vert_id, u32 level_vert_id, Fvector pos, Fvector ang, Fvector pos2, Fvector ang2, bool b, const shared_str& message, bool b_allow_change_level);

#ifdef DEBUG
	virtual void		Render					();
#endif
	CChangeLevelWnd*	UIChangeLevelWnd;

	SDrawStaticStruct*	m_game_objective;
};


class CChangeLevelWnd :public CUIDialogWnd
{
	CUIMessageBox*			m_messageBox;
	typedef CUIDialogWnd	inherited;
	void					OnCancel			();
	void					OnOk				();
public:
	GameGraph::_GRAPH_ID	m_game_vertex_id;
	u32						m_level_vertex_id;
	Fvector					m_position;
	Fvector					m_angles;
	Fvector					m_position_cancel;
	Fvector					m_angles_cancel;
	bool					m_b_position_cancel;
	bool					m_b_allow_change_level;
	shared_str				m_message_str;

						CChangeLevelWnd				();
	virtual				~CChangeLevelWnd			()									{};
	virtual void		SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);
	virtual bool		WorkInPause					()const {return true;}
	virtual void		Show						();
	virtual void		Hide						();
	virtual bool		OnKeyboardAction					(int dik, EUIMessages keyboard_action);
};