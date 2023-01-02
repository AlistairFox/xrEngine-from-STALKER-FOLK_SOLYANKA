#include "stdafx.h"
#include "UIPda_Chat.h"


#include "stdafx.h"
#include "UIPda_Chat.h"
#include "game_news.h"
#include "Level.h"

#include "ui/UI3tButton.h"
#include "ui/UIXmlInit.h"
#include "ui/UIScrollView.h"
#include "ui/UIFrameWindow.h"
#include "ui/UIEditBox.h"
#include "ui/UIHelper.h"
#include "ui/UICharacterInfo.h"
#include "ui/UINewsItemWnd.h"
#include "ui/UIFixedScrollBar.h"

#include "UICursor.h"


#define  PDA_CHAT_XML		"pda_mp_chat.xml"

UIPdaChat::UIPdaChat()
{
	LocalActor = 0;
	SecondActor = 0;

}

UIPdaChat::~UIPdaChat()
{
	chat_text->Clear();
	chat_users->Clear();
}

void UIPdaChat::Init()
{
	xml.Load(CONFIG_PATH, UI_PATH, PDA_CHAT_XML);

	CUIXmlInit::InitWindow(xml, "main_wnd", 0, this);
	m_background = UIHelper::CreateFrameWindow(xml, "background", this);
	m_background_players = UIHelper::CreateFrameWindow(xml, "players_background", this);
	m_background_chat = UIHelper::CreateFrameWindow(xml, "chat_background", this);

	chat_users = xr_new<CUIScrollView>();
	chat_users->SetAutoDelete(true);
	CUIXmlInit::InitScrollView(xml, "contacts_list", 0, chat_users);
	m_background_players->AttachChild(chat_users);

	CaptionOnline = UIHelper::CreateTextWnd(xml, "online_players", m_background_players);



	infoActor = xr_new<CUICharacterInfo>();
	infoPartner = xr_new<CUICharacterInfo>();
	infoActor->InitCharacterInfo(&xml, "character_info_actor");
	infoPartner->InitCharacterInfo(&xml, "character_info_partner");
	m_background_chat->AttachChild(infoActor);
	m_background_chat->AttachChild(infoPartner);

	send_money_button = UIHelper::Create3tButton(xml, "send_chat_money", m_background_chat);
	send_msg_to_user = UIHelper::Create3tButton(xml, "send_chat_msg", m_background_chat);
	chat_editbox = UIHelper::CreateEditBox(xml, "msg_editbox", m_background_chat);
	money_editbox = UIHelper::CreateEditBox(xml, "money_editbox", m_background_chat);

	chat_editbox->Init(256);
	money_editbox->Init(16, true);
	// Scrool Bar Text 

	m_background_chat_text = UIHelper::CreateFrameWindow(xml, "chat_text_background", m_background_chat);

	chat_text = xr_new <CUIScrollView>();
	chat_text->SetAutoDelete(true);
	CUIXmlInit::InitScrollView(xml, "list_text", 0, chat_text);
	m_background_chat_text->AttachChild(chat_text);

	player_1_money = UIHelper::CreateTextWnd(xml, "player_1_money", m_background_chat);
	player_2_money = UIHelper::CreateTextWnd(xml, "player_2_money", m_background_chat);
	player_2_money->Show(false);


	//Global Chat
	CaptionMode = UIHelper::CreateTextWnd(xml, "switch_mode_cap", m_background_players);
	switch_anonimous = UIHelper::Create3tButton(xml, "switch_anonimous", m_background_chat);
	switch_mode_button = UIHelper::Create3tButton(xml, "switch_mode", m_background_players);

	InitCallBacks();

}

void xr_stdcall UIPdaChat::button_click_mode_switch(CUIWindow* w, void* d)
{
	if (ModeGlobalChat)
		ModeGlobalChat = false;
	else
		ModeGlobalChat = true;
}

void xr_stdcall UIPdaChat::button_click_anonimous_mode_switch(CUIWindow* w, void* d)
{
	if (Anonymous)
		Anonymous = false;
	else
		Anonymous = true;
}

void UIPdaChat::InitCallBacks()
{
	Register(send_msg_to_user);
	Register(send_money_button);
	Register(switch_mode_button);
	Register(switch_anonimous);

	AddCallback(send_msg_to_user, BUTTON_CLICKED, CUIWndCallback::void_function(this, &UIPdaChat::button_click_send_msg));
	AddCallback(send_money_button, BUTTON_CLICKED, CUIWndCallback::void_function(this, &UIPdaChat::button_click_send_money));
	AddCallback(switch_mode_button, BUTTON_CLICKED, CUIWndCallback::void_function(this, &UIPdaChat::button_click_mode_switch));
	AddCallback(switch_anonimous, BUTTON_CLICKED, CUIWndCallback::void_function(this, &UIPdaChat::button_click_anonimous_mode_switch));

}

void UIPdaChat::Show(bool status)
{
	//	Msg("Show Chat %s", status ? "True" : "False");
	inherited::Show(status);

	money_editbox->ClearText();
	chat_editbox->ClearText();

	if (status)
	{
		CObject* obj = Level().Objects.net_Find(Game().local_player->GameID);
		CActor* act = smart_cast<CActor*>(obj);

		if (act)
		{
			LocalActor = act;
			if (infoActor)
				infoActor->InitCharacterMP(act);
		}
	}
}

void UIPdaChat::Update()
{
	inherited::Update();
	if (Device.dwFrame % 20 == 0)
	{
		string32 tmp;

		string32 online = { 0 };
		xr_strcat(online, "������: ");
		xr_strcat(online, itoa(Game().players.size(), tmp, 10));
		CaptionOnline->SetText(online);
		CaptionMode->SetText(ModeGlobalChat ? "����� ���" : "��������� ���");

		chat_users->Clear();

		bool f_Second = false;
		bool f_Local = false;

		for (auto player : Level().game->players)
		{
			if (SecondActor)
			{
				if (SecondActor->ID() == player.second->GameID)
				{
					SecondActorCL = player.first;
					f_Second = true;
				}
			}

			if (LocalActor)
			{
				if (LocalActor->ID() == player.second->GameID)
				{
					LocalActorCL = player.first;
					f_Local = true;
				}
			}

			if (player.first == Game().local_svdpnid)
				continue;

			CActor* actor = smart_cast<CActor*>(Level().Objects.net_Find(player.second->GameID));
			if (actor)
			{
				CUICharacterInfo* info = xr_new<CUICharacterInfo>();
				info->InitCharacterInfo(&xml, "character_info");
				info->InitCharacterMP(actor->cast_inventory_owner());

				chat_users->AddWindow(info, true);
			}
		}

		if (!f_Second)
		{
			SecondActor = 0;
			SecondActorCL = 0;
		}

		if (!f_Local)
		{
			LocalActor = 0;
			LocalActorCL = 0;
		}

		CObject* obj = Level().Objects.net_Find(Game().local_player->GameID);
		CActor* act = smart_cast<CActor*>(obj);

		if (act)
		{
			LocalActor = act;
			if (infoActor)
				infoActor->InitCharacterMP(act);


			string32 money1 = { 0 };
			xr_strcat(money1, itoa(LocalActor->get_money(), tmp, 10));
			xr_strcat(money1, " RU");
			if (Anonymous)
				xr_strcat(money1, " (������)");

			player_1_money->SetText(money1);
		}

		if (ModeGlobalChat == 0)
		{
			Anonymous = false;
			if (switch_anonimous->IsShown())
				switch_anonimous->Show(false);

			if (SecondActor)
			{
				if (old_chat != ModeGlobalChat || old_second_id != SecondActorCL.value())
				{
					old_chat = ModeGlobalChat;
					old_second_id = SecondActorCL.value();
					chat_text->Clear();

					for (auto id : news_data)
					{
						if (id.first == SecondActorCL)
							for (auto news : id.second)
							{
								CUINewsItemWnd* itm_res = xr_new<CUINewsItemWnd>();
								itm_res->Init(xml, "news_item");
								itm_res->Setup(news);

								chat_text->AddWindow(itm_res, true);
							}
					}
				}
			}
			else
			{
				chat_text->Clear();
			}


			if (SecondActor)
			{
				string32 money2 = { 0 };
				xr_strcat(money2, itoa(SecondActor->get_money(), tmp, 10));
				xr_strcat(money2, " RU");
				infoPartner->Show(true);
				infoPartner->InitCharacterMP(SecondActor);
			}

			if (!SecondActor)
			{
				if (money_editbox->IsShown())
					money_editbox->Show(false);
				if (send_money_button->IsShown())
					send_money_button->Show(false);
			}
			else
			{
				if (!money_editbox->IsShown())
					money_editbox->Show(true);
				if (!send_money_button->IsShown())
					send_money_button->Show(true);
			}
		}
		else
		{
			if (old_chat != ModeGlobalChat || chat_text->GetSize() == 0)
			{
				//Msg("Update GlobalChat");

				old_chat = ModeGlobalChat;
				old_second_id = 0;
				chat_text->Clear();

				for (auto news : global_chat)
				{
					CUINewsItemWnd* itm_res = xr_new<CUINewsItemWnd>();
					itm_res->Init(xml, "news_item");
					itm_res->Setup(news);
					chat_text->AddWindow(itm_res, true);
				}
			}

			if (money_editbox->IsShown())
				money_editbox->Show(false);
			if (send_money_button->IsShown())
				send_money_button->Show(false);
			if (infoPartner->IsShown())
				infoPartner->Show(false);
			if (!switch_anonimous->IsShown())
				switch_anonimous->Show(true);
			if (SecondActor)
				SecondActor = 0;
		}
	}
}

void UIPdaChat::ResetAll()
{
	inherited::ResetAll();
}

bool UIPdaChat::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	inherited::OnMouseAction(x, y, mouse_action);

	if (mouse_action == WINDOW_LBUTTON_DOWN)
	{
		auto child = chat_users->Items();

		for (auto iter = child.rbegin(); iter != child.rend(); iter++)
		{

			CUIWindow* wind = (*iter);
			CUICharacterInfo* info = smart_cast<CUICharacterInfo*>(wind);

			Frect wndRect;
			wind->GetAbsoluteRect(wndRect);
			Fvector2 pos = GetUICursor().GetCursorPosition();

			if (info && wndRect.in(pos))
			{
				u16 owner = info->OwnerID();

				//Msg("Set OwnerID %d", owner);

				CActor* actor = smart_cast<CActor*>(Level().Objects.net_Find(owner));

				if (actor)
				{
					SecondActor = actor;

					infoPartner->InitCharacterMP(actor);
				}
			}
		}

		return true;
	}

}

void xr_stdcall UIPdaChat::button_click_send_msg(CUIWindow* w, void* d)
{
	if (!LocalActor)
		return;

	if (!SecondActor && !ModeGlobalChat)
		return;

	shared_str text = chat_editbox->GetText();

	if (text.size() < 2)
		return;

	CInventoryOwner* owner = LocalActor->cast_inventory_owner();

	if (owner)
	{
		GAME_NEWS_DATA data;
		data.m_type = data.eNews;
		data.news_caption = LocalActor->Name();
		data.news_text = text.c_str();
		data.texture_name = owner->IconName();
		data.receive_time = Level().GetGameTime();

		//Msg("Send %d, text %s", ModeGlobalChat, data.news_text.c_str());

		if (!ModeGlobalChat)
			AddNewsData(data, SecondActor->ID(), ModeGlobalChat);
		else
			AddNewsData(data, 0, ModeGlobalChat);


		SendPacket(data);
	}

}

void xr_stdcall UIPdaChat::button_click_send_money(CUIWindow* w, void* d)
{

	if (!LocalActor)
		return;

	if (!SecondActor)
		return;

	shared_str text = money_editbox->GetText();

	if (text.size() < 2)
		return;

	u32 money = atoi(text.c_str());

	CInventoryOwner* owner = LocalActor->cast_inventory_owner();

	if (money < 0 || money > 1000000)
	{
		Msg("���������� ������������� �������� 1���");
		return;
	}

	if (owner)
	{
		NET_Packet packet;
		Game().u_EventGen(packet, GAME_EVENT_UI_PDA_CHAT_SERVER, -1);
		packet.w_u8(1);
		packet.w_u16(LocalActor->ID());
		packet.w_u16(SecondActor->ID());
		packet.w_u32(money);
		Game().u_EventSend(packet);
	}


}

void UIPdaChat::AddNewsData(GAME_NEWS_DATA data, ClientID PlayerID, bool GlobalChat)
{
	if (!GlobalChat)
		news_data[PlayerID].push_back(data);
	else
		global_chat.push_back(data);

	if (OnServer())
		Msg("����� [%s] �������� ���������[%s] ", data.news_caption.c_str(), data.news_text.c_str());
}

#include "UIGameCustom.h"
#include "ui/UIMessagesWindow.h"

void UIPdaChat::RecivePacket(NET_Packet& P)
{
	u8 Global;
	P.r_u8(Global);

	if (Global == 1)
	{
		GAME_NEWS_DATA data;
		data.m_type = data.eNews;
		shared_str news_caption;
		P.r_stringZ(news_caption);
		shared_str news_text;
		P.r_stringZ(news_text);
		shared_str texture_name;
		P.r_stringZ(texture_name);

		data.news_caption = news_caption;
		data.news_text = news_text;
		data.receive_time = Level().GetGameTime();
		data.texture_name = texture_name;

		AddNewsData(data, 0, true);

		if (old_chat == 1)
		{
			CUINewsItemWnd* itm_res = xr_new<CUINewsItemWnd>();
			itm_res->Init(xml, "news_item");
			itm_res->Setup(data);
			chat_text->AddWindow(itm_res, true);
		}

		if (CurrentGameUI())
			if (CurrentGameUI()->UIMainIngameWnd)
				CurrentGameUI()->m_pMessagesWnd->AddIconedPdaMessage(&data);

	}
	else
	{
		GAME_NEWS_DATA data;
		data.m_type = data.eNews;

		ClientID SenderID;
		P.r_clientID(SenderID);
		ClientID GameID;
		P.r_clientID(GameID);
		shared_str news_caption;
		P.r_stringZ(news_caption);
		shared_str news_text;
		P.r_stringZ(news_text);
		shared_str texture_name;
		P.r_stringZ(texture_name);


		data.news_caption = news_caption;
		data.news_text = news_text;
		data.receive_time = Level().GetGameTime();
		data.texture_name = texture_name;

		if (GameID == LocalActorCL)
			AddNewsData(data, SenderID, false);
		else
			AddNewsData(data, GameID, false);

		if (old_chat == 0)
		{
			if (SecondActorCL == GameID)
			{
				CUINewsItemWnd* itm_res = xr_new<CUINewsItemWnd>();
				itm_res->Init(xml, "news_item");
				itm_res->Setup(data);
				chat_text->AddWindow(itm_res, true);
			}

			if (LocalActorCL == GameID)
			{
				CUINewsItemWnd* itm_res = xr_new<CUINewsItemWnd>();
				itm_res->Init(xml, "news_item");
				itm_res->Setup(data);
				chat_text->AddWindow(itm_res, true);
			}


		}

		{
			GAME_NEWS_DATA data_news;

			data_news.news_caption = "�������� ����� ���������";
			data_news.news_text = news_text;
			data_news.texture_name = texture_name;

			if (LocalActorCL != GameID)
				if (CurrentGameUI())
					if (CurrentGameUI()->UIMainIngameWnd)
						CurrentGameUI()->m_pMessagesWnd->AddIconedPdaMessage(&data_news);
		}
	}

}

void UIPdaChat::SendPacket(GAME_NEWS_DATA data)
{
	NET_Packet P;

	Game().u_EventGen(P, GAME_EVENT_UI_PDA_CHAT_SERVER, -1);

	P.w_u8(0);
	P.w_u8(ModeGlobalChat);

	if (ModeGlobalChat == 0)
		P.w_clientID(SecondActorCL);

	P.w_clientID(LocalActorCL);
	shared_str news_caption = data.news_caption;
	P.w_stringZ(news_caption);
	shared_str news_text = data.news_text;
	P.w_stringZ(news_text);
	shared_str texture_name = data.texture_name;
	P.w_stringZ(texture_name);

	Game().u_EventSend(P);
}

void UIPdaChat::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

