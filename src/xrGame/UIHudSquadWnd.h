#pragma once

#include "ui/UIWindow.h"
#include "UISquadWnd.h"

class CUIXml;
class CUIStatic;
class CUIProgressBar;
class game_cl_freemp;
class CActor;
class CUIHudSquadMember;

class CUIHudSquadWnd : public CUIWindow
{
	game_cl_freemp* m_game;
	typedef CUIWindow inherited;

public:
	CUIHudSquadWnd();
	~CUIHudSquadWnd();

	void	Init();
	virtual void Update();

	void	UpdateMembers();

private:
	CUIHudSquadMember* m_pSquadMembers[squad_size - 1];
};

class CUIHudSquadMember : public CUIWindow
{
	CActor* m_actor;
	game_PlayerState* m_ps;

	typedef CUIWindow inherited;

public:
	CUIHudSquadMember() {};
	~CUIHudSquadMember() {};

	void	Init(CUIXml& xml_doc, LPCSTR path);
	virtual void	Update();
	void	Add(game_PlayerState* ps);
	void	Clear();
	void	ShowVoiceIcon(bool status);

private:
	float	m_last_health;
	float	m_health_blink;

	u32		m_name_color; // white
	u32		m_distance_color; // white
	u32		m_wounded_color; // red
	u32		m_dead_color; // black

	u8	m_status_icon_state;
	enum { e_none_state, e_dead_state, e_wounded_state };

	CUIStatic* m_pName;
	CUIStatic* m_pDistance;
	CUIStatic* m_pVoiceIcon;
	CUIStatic* m_pStatusIcon;
	CUIProgressBar* m_pHealthBar;
};