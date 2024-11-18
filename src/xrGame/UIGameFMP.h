#pragma once

#include "UIGameMP.h"
#include "UIHudSquadWnd.h"

class game_cl_freemp;
class CUITextWnd;
class CUIAMode;
class CUIStatic;
class CUI_UpgradesQuick;

class CUIGameFMP : public UIGameMP
{
private:
	game_cl_freemp *	m_game;
	typedef UIGameMP inherited;

protected:
	CUITextWnd*			m_stats;
	CUIAMode* m_animation;
	CUI_UpgradesQuick* m_attach_quck;
	
	CUIStatic*  surge_background;
	CUITextWnd* surge_cap;


	CUIHudSquadWnd* m_hud_squad;

	

	bool upgrades_activated = false;

public:
	void UpdateHudSquad();

				CUIGameFMP();
	virtual		~CUIGameFMP();

	virtual	void Init(int stage);

	virtual void SetClGame(game_cl_GameState* g);

	virtual void HideShownDialogs();

	virtual void	_BCL OnFrame();

	virtual bool IR_UIOnKeyboardPress(int dik);



	u32 surge_time = 0;
	u32 surge_time_end = 0;
	bool surge_started = false;

	void setSurgeTimer(u32 time, u32 timeGlobal, bool val) {
		surge_time = time;
		surge_started = val;
		surge_time_end = timeGlobal;
	};

 };
