#include "stdafx.h"
#include "UIPda_Squad.h"

#include "stdafx.h"
#include "UIPda_Squad.h"

//Include UI
#include "ui/UIXmlInit.h"
#include "ui/UIFrameWindow.h"
#include "ui/UIHelper.h"
#include "ui/UIScrollView.h"

#include "ui/UIFixedScrollBar.h"

//Character
#include "ui/UICharacterInfo.h"
#include "Level.h"
#include "Actor.h"

#define	 PDA_SQUAD_XML			"pda_mp_squad.xml"

CUIPda_Squad::CUIPda_Squad()
{
}

CUIPda_Squad::~CUIPda_Squad()
{
}

void CUIPda_Squad::Init()
{
	//Load XML SQUAD
	CUIXml xml_squad;
	xml_squad.Load(CONFIG_PATH, UI_PATH, PDA_SQUAD_XML);
	CUIXmlInit::InitWindow(xml_squad, "squad_wnd", 0, this);

	for (int i = 0; i <= 3; i++)
	{
		string32 name = { 0 };
		string16 index = { 0 };
		itoa(i, index, 10);

		xr_strcat(name, "squad_wnd:frame_");
		xr_strcat(name, index);

		squad_wnd_team[i] = UIHelper::CreateFrameWindow(xml_squad, name, this);
		character_team[i] = xr_new<CUICharacterInfo>();
		character_team[i]->SetAutoDelete(true);
		character_team[i]->InitCharacterInfo(&xml_squad, "character_info");
		character_team[i]->ClearInfo();
		squad_wnd_team[i]->AttachChild(character_team[i]);

		Msg("InitUI[%d] [%s]", i, name);

	}

}

void CUIPda_Squad::Show(bool status)
{
	inherited::Show(status);
	if (!status)
		initPanel = false;
}

void CUIPda_Squad::Update()
{
	if (!initPanel && this->IsShown())
	{
		int idx = 0;

		for (auto player : team_players.players)
		{
			if (player.GameID != u16(-1) && character_team[idx])
			{
				CActor* actor_player = smart_cast<CActor*>(Level().Objects.net_Find(player.GameID));
				if (actor_player)
				{
					//Msg("InitCharacterMP [%d] [%d] [%u]", idx, player.GameID, player.Client);
					character_team[idx]->InitCharacterMP(actor_player, 0);
				}
			}
			else
			{
				character_team[idx]->ClearInfo();
			}

			//Msg("InitCharacterMP [%d]", idx);
			idx += 1;
		}

		initPanel = true;
	}
}

void CUIPda_Squad::ResetAll()
{
	character_team[0]->ClearInfo();
	character_team[1]->ClearInfo();
	character_team[2]->ClearInfo();
	character_team[3]->ClearInfo();
	initPanel = false;
}

void CUIPda_Squad::EventRecive(Team player_team)
{
	//Msg("EventRecive CUIPda_Squad");
	team_players = player_team;
	ResetAll();
}
