#include "stdafx.h"
#include "UIPda_Contacts.h"
#include "UIPda_Squad.h"
#include "UIPda_Chat.h"

//Include UI
#include "ui/UIXmlInit.h"
#include "ui/UIFrameWindow.h"
#include "ui/UIHelper.h"
#include "ui/UIScrollView.h"
#include "ui/UIFixedScrollBar.h"
#include "ui/UIListBoxItem.h"
#include "ui/UI3tButton.h"
#include "ui/UIStatic.h"

//Character
#include "ui/UICharacterInfo.h"
#include "ui/UIPropertiesBox.h"

//Others

#include "Level.h"
#include "Actor.h"
//UI BASE
#include "UICursor.h"
#include "ui_base.h"


CUIPda_Contacts::CUIPda_Contacts()
{
}

CUIPda_Contacts::~CUIPda_Contacts()
{
	contacts_list->Clear();
	old_size_players = 0;
}

#define  PDA_CONTACTS_XML		"pda_mp_contacts.xml"

void CUIPda_Contacts::Init()
{
	CUIXml xml;
	xml.Load(CONFIG_PATH, UI_PATH, PDA_CONTACTS_XML);

	CUIXmlInit::InitWindow(xml, "main_wnd", 0, this);
	m_delay = (u32)xml.ReadAttribInt("main_wnd", 0, "delay", 3000);
	m_background = UIHelper::CreateFrameWindow(xml, "background", this);

	contacts_caption = UIHelper::CreateTextWnd(xml, "contacts_caption", this);
	contacts_window = UIHelper::CreateFrameWindow(xml, "contacts_wnd", this);

	//Scroll
	CUIFixedScrollBar* tmp_scroll = xr_new<CUIFixedScrollBar>();
	contacts_list = xr_new<CUIScrollView>(tmp_scroll);
	contacts_list->SetAutoDelete(true);
	contacts_window->AttachChild(contacts_list);

	CUIXmlInit::InitScrollView(xml, "contacts_list", 0, contacts_list);
	contacts_list->SetWindowName("ListPlayers");
	//End Scroll

	squad_cap = UIHelper::CreateTextWnd(xml, "squad_caption", this);
	squad_window = UIHelper::CreateFrameWindow(xml, "squad_window", this);

	squad_UI = xr_new<CUIPda_Squad>();
	squad_UI->Init();
	squad_UI->SetAutoDelete(true);

	squad_window->AttachChild(squad_UI);

	property_box = xr_new<CUIPropertiesBox>();
	property_box->InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(100, 75));
	AttachChild(property_box);	
	property_box->Hide();
	property_box->SetWindowName("property_box");

 //	property_box->AddItem("Выйти", NULL, PDA_PROPERTY_EXIT);

	squad_UI_invite = UIHelper::CreateFrameWindow(xml, "squad_invite", squad_window);

	squad_UI_invite_text = UIHelper::CreateTextWnd(xml, "squad_invite_text", squad_UI_invite);

	squad_UI_invite_yes = UIHelper::Create3tButton(xml, "squad_invite_yes", squad_UI_invite);
	squad_UI_invite_no = UIHelper::Create3tButton(xml, "squad_invite_no", squad_UI_invite);


	squad_UI_invite->Show(false);

	InitCallBacks();
}

void CUIPda_Contacts::InitCallBacks()
{
	Register(property_box);
	Register(squad_UI_invite_yes);
	Register(squad_UI_invite_no);

	AddCallback(property_box, PROPERTY_CLICKED, CUIWndCallback::void_function(this, &CUIPda_Contacts::property_box_clicked));
	AddCallback(squad_UI_invite_yes, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUIPda_Contacts::button_yes));
	AddCallback(squad_UI_invite_no, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUIPda_Contacts::button_no));
}

void CUIPda_Contacts::Show(bool status)
{
	inherited::Show(status);

	squad_UI->Show(status);

	if (!status)
		old_size_players = 0;
}

void CUIPda_Contacts::Update()
{
	inherited::Update();
 
	if (invite_mode)
	{
		if (squad_UI->IsShown())
		{
			squad_UI->Show(false);
		}

		if (!squad_UI_invite->IsShown())
		{
			squad_UI_invite->Show(true);
		}

		int TextSize = xr_strlen(squad_UI_invite_text->GetText());
 
		if (TextSize == 0 && last_inviter.value())
		{
			CActor* actor;
			for (auto pl : Game().players)
			{
				if (pl.first == last_inviter)
				{
					actor = smart_cast<CActor*>(Level().Objects.net_Find(pl.second->GameID));
				}
			}

			if (actor)
			if (xr_strlen(actor->Name()) > 0)
			{
 				SetInvite(actor->Name());
			}
		}
	}
	else
	{
		if (!squad_UI->IsShown())
		{
			squad_UI->Show(true);
		}

		if (squad_UI_invite->IsShown())
		{
			squad_UI_invite->Show(false);
			squad_UI_invite_yes->Reset();
			squad_UI_invite_no->Reset();
			squad_UI_invite_text->SetText("");
		}
	} 

	if (Device.dwTimeGlobal - m_previous_time > m_delay && old_size_players != Game().players.size())
	{
		contacts_list->Clear();

		for (auto pl : Level().game->players)
		{
			if (pl.second == Level().game->local_player)
			{
				continue;
			}

			CActor* actor = smart_cast<CActor*>(Level().Objects.net_Find(pl.second->GameID));

			if (!actor)
			{
				m_previous_time = Device.dwTimeGlobal;
				return;
			}

			if (actor)
			{
				CUICharacterInfo* player_info = xr_new<CUICharacterInfo>();
				CUIXml xml;
				xml.Load(CONFIG_PATH, UI_PATH, PDA_CONTACTS_XML);
				player_info->InitCharacterInfo(&xml, "char_info");
				player_info->InitCharacterMP(actor, pl.first.value());

				CUIWindow* static_line = xr_new<CUIWindow>();
				CUIXmlInit::InitAutoStaticGroup(xml, "pda_char_auto_statics", 0, static_line);

				static_line->AttachChild(player_info);
				static_line->SetHeight(player_info->GetWndSize().y);
				contacts_list->AddWindow(static_line, true);
			}
		}

		old_size_players = Game().players.size();

		m_previous_time = Device.dwTimeGlobal;
	}
}

void CUIPda_Contacts::ResetAll()
{
	inherited::ResetAll();
}

bool CUIPda_Contacts::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	inherited::OnMouseAction(x, y, mouse_action);

	if (mouse_action != WINDOW_RBUTTON_DOWN)
		return true;

	CUIScrollView* win = contacts_list;
	auto child = win->Items();

	for (auto iter = child.rbegin(); iter != child.rend(); iter++)
	{
		CUIWindow* w = (*iter);

		auto begin = w->GetChildWndList().rbegin();
		auto end   = w->GetChildWndList().rend();

		for (; begin != end; begin++)
		{
			CUIWindow* wind = (*begin);
			Frect wndRect; wind->GetAbsoluteRect(wndRect);

			CUICharacterInfo* info = smart_cast<CUICharacterInfo*>(wind);

			Fvector2 pos = GetUICursor().GetCursorPosition();

			if  (info && wndRect.in(pos) )
			{
				pos.x = x;
				pos.y = y;

				if (!squad_UI->in_squad || ( squad_UI->leader_id == Level().game->local_svdpnid.value() && squad_UI->players.size() <= 3) )
				{
					property_box->RemoveAll();
					property_box->AddItem("Пригласить в отряд", NULL, PDA_PROPERTY_ADD_TO_SQUAD);

					property_box->Show(wndRect, pos);

					id_actor = info->OwnerID();
					id_client = info->getClientID();
				}

			}
		}
	}
   

	return true;
}

void xr_stdcall CUIPda_Contacts::property_box_clicked(CUIWindow* w, void* d)
{
 	if (!property_box->GetClickedItem())
		return;

	switch (property_box->GetClickedItem()->GetTAG())
	{
		case (PDA_PROPERTY_ADD_TO_SQUAD):
		{
			NET_Packet packet;
			Level().game->u_EventGen(packet, GAME_EVENT_UI_PDA_SERVER, -1);
			packet.w_u8(1);	//SEND RECVEST
			packet.w_u32(id_client);
 			packet.w_u32(Game().local_svdpnid.value());
			Level().game->u_EventSend(packet);
		};
		break;
	}
}

void xr_stdcall CUIPda_Contacts::button_yes(CUIWindow* w, void* d)
{
	invite_mode = false;

	if (!last_inviter.value())
		return;
 
	NET_Packet packet;
	Game().u_EventGen(packet, GAME_EVENT_UI_PDA_SERVER, -1);
	packet.w_u8(2);	 //	ADD NEW CLIENT
	packet.w_u32(Game().local_svdpnid.value());
	packet.w_u32(last_inviter.value());
	Game().u_EventSend(packet);

 	return;
}

void xr_stdcall CUIPda_Contacts::button_no(CUIWindow* w, void* d)
{
	invite_mode = false;
	return;
}

void CUIPda_Contacts::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUIPda_Contacts::EventRecive(NET_Packet& P)
{
	u8 type = P.r_u8();
	if (type == 1)
	{
		if (squad_UI->in_squad)
			return;

		last_inviter = ClientID( P.r_u32() );
		invite_mode = true;
	}
	else if (type == 2)
	{
		u32 lead; P.r_u32(lead);
		u32 id; P.r_u32(id);
		
		if (!squad_UI->leader_in_squad)
		{
			squad_UI->leader_id = last_inviter.value();
 			squad_UI->players.push_back(squad_UI->leader_id);
			squad_UI->leader_in_squad = true;
		}

		if (squad_UI->players.size() > 3)
			return;

		squad_UI->players.push_back(id);
		squad_UI->in_squad = true;
	}
	else if (type == 3)
	{
		u32 id; P.r_u32(id);
		if (Level().game->local_svdpnid.value() == id)
		{
			squad_UI->in_squad = false;
			squad_UI->leader_in_squad = false;
			squad_UI->leader_id = 0;
			squad_UI->players.clear();
		}
		else
		{
			for (auto b = squad_UI->players.begin(); b != squad_UI->players.end(); b++)
			if (*b == id)
				squad_UI->players.erase(b);
		}
	}
	else if (type == 4)
	{
		squad_UI->players.clear();
		squad_UI->leader_id = 0;
		squad_UI->in_squad = false;
		squad_UI->leader_in_squad = false;
	}
}

void CUIPda_Contacts::SetInvite(LPCSTR name)
{
	string128 text = { 0 };
	xr_strcat(text, name);
	xr_strcat(text, " приглашает Вас в отряд");

	squad_UI_invite_text->SetText(text);
	squad_UI_invite_text->SetTextColor(color_rgba(255, 255, 0, 255));
}
