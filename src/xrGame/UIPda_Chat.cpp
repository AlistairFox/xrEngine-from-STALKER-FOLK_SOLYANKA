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
	InitCallBacks();

}

void UIPdaChat::InitCallBacks()
{
	Register(send_msg_to_user);
	Register(send_money_button);
	AddCallback(send_msg_to_user, BUTTON_CLICKED, CUIWndCallback::void_function(this, &UIPdaChat::button_click_send_msg));
	AddCallback(send_money_button, BUTTON_CLICKED, CUIWndCallback::void_function(this, &UIPdaChat::button_click_send_money));
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

	if (Device.dwFrame % 60 && CaptionOnline)
	{
		string32 tmp;

		string32 online = { 0 };
		xr_strcat(online, "Онлайн: ");
		xr_strcat(online, itoa(Game().players.size(), tmp, 10));
		CaptionOnline->SetText(online);

		if (LocalActor)
		{
			string32 money1 = { 0 };
			xr_strcat(money1, itoa(LocalActor->get_money(), tmp, 10));
			xr_strcat(money1, " RU");
			player_1_money->SetText(money1);

			infoActor->InitCharacterMP(LocalActor);
		}

		if (SecondActor)
		{
			string32 money2 = { 0 };
			xr_strcat(money2, itoa(SecondActor->get_money(), tmp, 10));
			xr_strcat(money2, " RU");
			//player_2_money->SetText(money2);
			//player_2_money->Show(true);

			infoPartner->InitCharacterMP(SecondActor);
		}

		if (Device.dwTimeGlobal - old_time > 3000)
		{
			chat_users->Clear();

			for (auto player : Level().game->players)
			{
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

			old_time = Device.dwTimeGlobal;
		}

		if (!SecondActor)
		{
			money_editbox->Show(false);
			send_money_button->Show(false);
		}
		else
		{
			money_editbox->Show(true);
			send_money_button->Show(true);
		}

		if (chat_text && SecondActor)
		{
			for (auto id : news_data)
			{
				if (id.first == SecondActor->ID() && old_size != id.second.size() ||
					id.first == SecondActor->ID() && old_second_id != SecondActor->ID())
				{
					old_size = id.second.size();
					old_second_id = SecondActor->ID();
					chat_text->Clear();
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

	if (!SecondActor)
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

		AddNewsData(data, SecondActor->ID());
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
		Msg("Превышение максимального перевода 1млн");
		return;
	}

	if (owner)
	{
		NET_Packet packet;
		Game().u_EventGen(packet, GAME_EVENT_PDA_CHAT, -1);
		packet.w_u8(1);
		packet.w_u16(LocalActor->ID());
		packet.w_u16(SecondActor->ID());
		packet.w_u32(money);
		Game().u_EventSend(packet);
	}


}

void UIPdaChat::AddNewsData(GAME_NEWS_DATA data, u16 PlayerID)
{
	news_data[PlayerID].push_back(data);

	Msg("Игрок [%s] Отправил сообщение[%s] ", data.news_caption.c_str(), data.news_text.c_str());
}

#include "UIGameCustom.h"
#include "ui/UIMessagesWindow.h"

void UIPdaChat::RecivePacket(NET_Packet& P)
{
	//Msg("Recive Packet");

	GAME_NEWS_DATA data;

	data.m_type = data.eNews;

	u16 GameID;
	P.r_u16(GameID);

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

	AddNewsData(data, GameID);


	{
		GAME_NEWS_DATA data_news;

		data_news.news_caption = "Получено новое сообщение";
		data_news.news_text = news_text;
		data_news.texture_name = texture_name;

		if (CurrentGameUI())
		{
			if (CurrentGameUI()->UIMainIngameWnd)
			{
				CurrentGameUI()->m_pMessagesWnd->AddIconedPdaMessage(&data_news);

				//luabind::functor<void>		functor;
				//R_ASSERT2(ai().script_engine().functor("news_manager.PlaySnd", functor), "Cant Find (news_manager.PlaySnd)");
				//functor();
			}
		}

	}
}

void UIPdaChat::SendPacket(GAME_NEWS_DATA data)
{
	NET_Packet P;

	Game().u_EventGen(P, GAME_EVENT_PDA_CHAT, -1);

	ClientID id;

	for (auto pl : Game().players)
	{
		if (pl.second->GameID == SecondActor->ID())
		{
			id = pl.first;
		}
	}

	P.w_u8(0);

	P.w_clientID(id);
	P.w_u16(LocalActor->ID());

	shared_str news_caption = data.news_caption;
	P.w_stringZ(news_caption);
	shared_str news_text = data.news_text;
	P.w_stringZ(news_text);
	shared_str texture_name = data.texture_name;
	P.w_stringZ(texture_name);

	//Msg("---export caption[%s] / text [%s] / text [%s]", news_caption.c_str(), news_text.c_str(), texture_name.c_str());

	Game().u_EventSend(P);
}

void UIPdaChat::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

