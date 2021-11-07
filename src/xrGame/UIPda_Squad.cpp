#include "stdafx.h"
#include "UIPda_Squad.h"

#include "stdafx.h"
#include "UIPda_Squad.h"

//Include UI
#include "ui/UIXmlInit.h"
#include "ui/UIFrameWindow.h"
#include "ui/UIHelper.h"
#include "ui/UIScrollView.h"

//Character
#include "ui/UICharacterInfo.h"


#include "Level.h"
#include "Actor.h"

//PropertyBox
#include "ui/UIPropertiesBox.h"
#include "UICursor.h"
#include "ui/UIListBoxItem.h"

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

	property_box_squad = xr_new<CUIPropertiesBox>();
	property_box_squad->InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(100, 100));
	AttachChild(property_box_squad);
	property_box_squad->Hide();
	property_box_squad->SetWindowName("property_box");

	property_box_squad->AddItem("Выкинуть игрока", NULL, PDA_PROPERTY_REMOVE_USER);
	property_box_squad->AddItem("Чат с игроком", NULL, PDA_PROPERTY_ADD_TO_CHAT);

	InitCallBacks();

}

void CUIPda_Squad::Show(bool status)
{
	inherited::Show(status);
	if (!status)
		initPanel = false;
}

void CUIPda_Squad::Update()
{
	inherited::Update();

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
					Msg("InitCharacterMP [%d] [%d] [%u]", idx, player.GameID, player.Client);
					character_team[idx]->InitCharacterMP(actor_player, player.Client);
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
	inherited::ResetAll();

	character_team[0]->ClearInfo();
	character_team[1]->ClearInfo();
	character_team[2]->ClearInfo();
	character_team[3]->ClearInfo();
	initPanel = false;
}

void CUIPda_Squad::EventRecive(Team player_team)
{
 	team_players = player_team;

	Msg("Leader [%d]", player_team.LeaderGameID);
	Msg("LeaderCL[%u]", player_team.ClientLeader);
	Msg("NumPlayers [%d]", player_team.cur_players);
	Msg("Player1 [%d]", player_team.players[0].GameID);
	Msg("Player2 [%d]", player_team.players[1].GameID);
	Msg("Player3 [%d]", player_team.players[2].GameID);
	Msg("Player4 [%d]", player_team.players[3].GameID);

	ResetAll();
}

bool CUIPda_Squad::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	inherited::OnMouseAction(x, y, mouse_action);

	if (mouse_action != WINDOW_RBUTTON_DOWN)
		return true;
 
	int id = 0;

	for (auto item : character_team)
	{
 		Frect wndAbsolute;
		
		CUICharacterInfo* info = smart_cast<CUICharacterInfo*>(item);
		info->GetParent()->GetAbsoluteRect(wndAbsolute);
		Fvector2 pos = GetUICursor().GetCursorPosition();
	   
		if (info && wndAbsolute.in(pos))
		{
			pos.x = x;
			pos.y = y;

			property_box_squad->Show(wndAbsolute, pos);

			selected_GameID = team_players.players[id].GameID;
			selected_user = team_players.players[id].Client;

		}
		id += 1;
	}

	return true;

}
 
void xr_stdcall CUIPda_Squad::property_box_squad_clicked(CUIWindow* w, void* d)
{
	if (!property_box_squad->GetClickedItem())
		return;

	switch (property_box_squad->GetClickedItem()->GetTAG())
	{

		case (PDA_PROPERTY_REMOVE_USER):
		{
			Msg("Remove user");
		
			NET_Packet packet;

			Game().u_EventGen(packet, GE_UI_PDA, -1);
			packet.w_u8(2);
			packet.w_clientID(selected_user);
			packet.w_u16(selected_GameID);
			packet.w_u16(team_players.LeaderGameID);

			Msg("GameID %d", selected_GameID);
			Msg("UserID %u", selected_user.value());
			Msg("LeaderID %d", team_players.LeaderGameID);
			Game().u_EventSend(packet);

		}	break;

		case (PDA_PROPERTY_ADD_TO_CHAT):
		{
			Msg("Clicked CHAT");

			

		};
		break;
	}
}

void CUIPda_Squad::InitCallBacks()
{
	Register(property_box_squad);	
	AddCallback(property_box_squad, PROPERTY_CLICKED, CUIWndCallback::void_function(this, &CUIPda_Squad::property_box_squad_clicked));
}

void CUIPda_Squad::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}
