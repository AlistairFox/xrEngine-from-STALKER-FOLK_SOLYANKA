#pragma once
#include "ui/UIWindow.h"

class CUIFrameWindow;
class CUIStatic;
class CUICharacterInfo;

struct TeamPlayer
{
	ClientID Client;
	u16 GameID = -1;
};

struct Team
{
	TeamPlayer players[4];
	u16 leader = -1;
	u16 cur_players = 0;
};



class CUIPda_Squad : public CUIWindow
{
private:
	typedef CUIWindow	inherited;


	u8 idxPlayer = 0;
	bool initPanel = false;

	CUIFrameWindow* squad_wnd_team[4];

	CUIStatic* squad_cap_1;
	CUIStatic* squad_cap_2;
	CUIStatic* squad_cap_3;
	CUIStatic* squad_cap_4;

	CUICharacterInfo* character_team[4];


public:
	Team team_players;

	CUIPda_Squad();
	virtual ~CUIPda_Squad();


	void				Init();
	virtual void 		Show(bool status);
	virtual void		Update();
	virtual void		ResetAll();

	void				EventRecive(Team player_team);

};

