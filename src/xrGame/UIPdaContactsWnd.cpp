#include "stdafx.h"
#include "UIPdaContactsWnd.h"
#include "PDA.h"
#include "ui/UIXmlInit.h"
#include "Actor.h"
#include "ui/UIFrameWindow.h"
#include "ui/UIFrameLineWnd.h"
#include "ui/UIAnimatedStatic.h"
#include "ui/UIScrollView.h"
#include "string_table.h"

#include "UICursor.h"
#include "ui/UIActorMenu.h"
#include "ui/UIWndCallback.h"
#include "ui/UIListBoxItem.h"
#include "ui/UI3tButton.h"
#include "ui/UIFixedScrollBar.h"
#include "UIInviteToSquadMsgBox.h"
#include "UISquadWnd.h"
#include "UIGameCustom.h"

#include "ui/UIHelper.h"

#define PDA_CONTACTS_XML		"xrmpe\\pda_contacts.xml"
#define PDA_CONTACT_HEIGHT		70

CUIPdaContactsWnd::CUIPdaContactsWnd()
{
	m_flags.zero();
}

CUIPdaContactsWnd::~CUIPdaContactsWnd()
{
	xr_delete(m_UIPropertiesBox);
}

void CUIPdaContactsWnd::Show(bool status)
{
	inherited::Show(status);
	if (status) UIDetailsList->Clear();
}

void CUIPdaContactsWnd::Init()
{
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, PDA_CONTACTS_XML);

	CUIXmlInit xml_init;

	xml_init.InitWindow(uiXml, "main_wnd", 0, this);

	m_header_background = UIHelper::CreateFrameWindow(uiXml, "header_background", this);
	m_delay = 3000;

	//---------------------------CONTACTS
	m_contacts_caption = UIHelper::CreateTextWnd(uiXml, "contacts_caption", this);
	m_contacts_frame = UIHelper::CreateFrameWindow(uiXml, "contacts_frame", this);

	CUIFixedScrollBar* tmp_scroll = xr_new<CUIFixedScrollBar>();
	UIContactsList = new CUIScrollView(tmp_scroll); UIContactsList->SetAutoDelete(true);
	m_contacts_frame->AttachChild(UIContactsList);
	xml_init.InitScrollView(uiXml, "contacts_list", 0, UIContactsList);

	//---------------------------SQUAD
	m_squad_caption = UIHelper::CreateTextWnd(uiXml, "squad_caption", this);
	m_squad_frame = UIHelper::CreateFrameWindow(uiXml, "squad_frame", this);

	m_squad_wnd = xr_new<CUISquadWnd>(); m_squad_wnd->SetAutoDelete(true);
	m_squad_frame->AttachChild(m_squad_wnd);
	m_squad_wnd->Init();

	m_invite_msg_box = xr_new<CUIInviteToSquadMsgBox>(); m_invite_msg_box->SetAutoDelete(true);
	m_squad_frame->AttachChild(m_invite_msg_box);
	m_invite_msg_box->Init();

	//---------------------------DETAILS
	m_details_caption = UIHelper::CreateTextWnd(uiXml, "details_caption", this);
	m_details_frame = UIHelper::CreateFrameWindow(uiXml, "details_frame", this);

	UIDetailsList = new CUIScrollView(); UIDetailsList->SetAutoDelete(true);
	m_details_frame->AttachChild(UIDetailsList);
	xml_init.InitScrollView(uiXml, "details_list", 0, UIDetailsList);

	//---------------------------PRIVATE CHAT
	m_private_chat_background = UIHelper::CreateStatic(uiXml, "details_frame:private_chat_background", m_details_frame);
	m_private_chat_frame = UIHelper::CreateFrameWindow(uiXml, "details_frame:private_chat_frame", m_details_frame);
	m_private_chat_over = UIHelper::CreateFrameWindow(uiXml, "details_frame:private_chat_over", m_details_frame);
	m_private_chat_editBox = UIHelper::CreateEditBox(uiXml, "details_frame:private_chat_edit_box", m_details_frame);
	m_private_chat_send_message = UIHelper::Create3tButton(uiXml, "details_frame:private_chat_send_message", m_details_frame);

	UIPrivateChatList = xr_new<CUIScrollView>(); UIPrivateChatList->SetAutoDelete(true);
	m_details_frame->AttachChild(UIPrivateChatList);
	xml_init.InitScrollView(uiXml, "details_frame:private_chat_logs_list", 0, UIPrivateChatList);

	//Init PropertiesBox
	m_UIPropertiesBox = xr_new<CUIPropertiesBox>();

	m_UIPropertiesBox->InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(300, 300), PDA_CONTACTS_XML);
	AttachChild(m_UIPropertiesBox);
	m_UIPropertiesBox->Hide();
	m_UIPropertiesBox->SetWindowName("property_box");

	m_UIPropertiesBox->AddItem("st_pda_send_invite", NULL, UI_PDA_SQUAD_SEND_INVITE);
	m_UIPropertiesBox->AddItem("st_pda_cancel_invite", NULL, UI_PDA_SQUAD_CANCEL_INVITE);

	//Callback
	Register(m_UIPropertiesBox);
	AddCallback(m_UIPropertiesBox, PROPERTY_CLICKED, CUIWndCallback::void_function(this, &CUIPdaContactsWnd::ProcessPropertiesBoxClicked));

	game = smart_cast<game_cl_freemp*>(&Game());
}

void CUIPdaContactsWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUIPdaContactsWnd::ShowPropertiesBox()
{
	CurrentGameUI()->ActorMenu().PlaySnd(CUIActorMenu::eProperties);

	m_UIPropertiesBox->AutoUpdateSize();
	Fvector2 cursor_pos;
	Frect vis_rect;
	GetAbsoluteRect(vis_rect);
	cursor_pos = GetUICursor().GetCursorPosition();
	cursor_pos.sub(vis_rect.lt);

	m_UIPropertiesBox->Show(vis_rect, cursor_pos);
}

void CUIPdaContactsWnd::HidePropertiesBox()
{
	if (m_UIPropertiesBox->IsShown())
		m_UIPropertiesBox->Hide();
}

void CUIPdaContactsWnd::ProcessPropertiesBoxClicked(CUIWindow* w, void* d)
{

	CUIPdaContactItem* item = smart_cast<CUIPdaContactItem*>(UIContactsList->GetSelected());
	if (!item)
	{
		Msg("! FAILED to create contact from selected item: %p", item);
		return;
	}

	CPda* temp_pda = (CPda*)item->m_data;
	if (!temp_pda)
	{
		Msg("! FAILED to create PDA from contact! Player may be dead or disconnected!");
		return;
	}

	switch (m_UIPropertiesBox->GetClickedItem()->GetTAG())
	{
	case UI_PDA_SQUAD_SEND_INVITE:
	{
		CurrentGameUI()->ActorMenu().PlaySnd(CUIActorMenu::eSquadInvite);
		OnPropertyPdaSendInviteClicked(temp_pda);
	}break;
	case UI_PDA_SQUAD_CANCEL_INVITE:
	{
		CurrentGameUI()->ActorMenu().PlaySnd(CUIActorMenu::eSquadAction);
		OnPropertyPdaCancelInviteClicked(temp_pda);
	}break;
	}
}
#include "Actor.h"
#include "actor_mp_client.h"
void CUIPdaContactsWnd::Update()
{
	if (Device.dwTimeGlobal - m_previous_time > m_delay)
	{
		m_previous_time = Device.dwTimeGlobal;

		RemoveAll();

		CActor* tmp_actor = smart_cast<CActor*>(Level().CurrentControlEntity());
		if (!tmp_actor) return;

		CPda* pPda = tmp_actor->GetPDA();
		if (!pPda) return;
 
		pPda->ActivePDAContacts(m_pda_list);
 
		for (auto pda : m_pda_list)
		{
			auto A = smart_cast<CActor*> (pda->GetOwnerObject());
			auto MP = smart_cast<CActorMP*>(pda->GetOwnerObject());
			if (A && MP)
 				AddContact(pda);
		}
 
		m_flags.set(flNeedUpdate, FALSE);
	}

	if (IsShown() && game->m_bSwitchToNextInvite)
	{
		if (game->mp_squad_invites.empty())
		{
			m_invite_msg_box->Show(false);
			game->m_bSwitchToNextInvite = false;
			return;
		}

		game_cl_freemp::MP_SquadInvite* tmp_invite = game->mp_squad_invites.back();

		game_PlayerState* ps = game->GetPlayerByGameID(tmp_invite->InviterID);
		if (!ps)
		{
			game->mp_squad_invites.pop_back();
			return;
		}

		m_invite_msg_box->Show(true);
		m_invite_msg_box->OnInviteReceived(tmp_invite->InviteMessage.c_str());

		game->m_bSwitchToNextInvite = false; //Waiting Player's respond;
	}

	if (game->local_squad->need_update && IsShown())
	{
		m_squad_wnd->RemoveAll();

		for (u32 o_it = 0; o_it < game->local_squad->players.size(); o_it++) 
		{
			auto PS = smart_cast<game_PlayerState*> ( game->local_squad->players[o_it] );
			if (PS != nullptr)
				m_squad_wnd->AddSquadMember(PS);
		};

		game->local_squad->need_update = false;
	}

	inherited::Update();
}

void CUIPdaContactsWnd::AddContact(CPda* pda)
{
	VERIFY(pda);

	CUIPdaContactItem* pItem = NULL;
	pItem = xr_new<CUIPdaContactItem>(this);
 
	UIContactsList->AddWindow(pItem, true);
	pItem->InitPdaListItem(Fvector2().set(0, 0), Fvector2().set(UIContactsList->GetWidth(), 120.0f));

	if (pda->H_Parent())
		pItem->InitCharacter(smart_cast<CInventoryOwner*>(pda->H_Parent()));

	//pItem->InitCharacter(pda->GetOriginalOwner());

	pItem->m_data = (void*)pda;

}

void CUIPdaContactsWnd::RemoveContact(CPda* pda)
{
	u32 cnt = UIContactsList->GetSize();

	for (u32 i = 0; i < cnt; ++i) {
		CUIWindow* w = UIContactsList->GetItem(i);
		CUIPdaContactItem* itm = (CUIPdaContactItem*)(w);

		if (itm->m_data == (void*)pda) {
			if (itm->GetSelected())
				UIDetailsList->Clear();
			HidePropertiesBox();
			UIContactsList->RemoveWindow(w);
			return;
		}
	}

}

void CUIPdaContactsWnd::RemoveAll()
{
	UIContactsList->Clear();
	UIDetailsList->Clear();
}

void CUIPdaContactsWnd::Reload()
{
	m_flags.set(flNeedUpdate, TRUE);
}

void CUIPdaContactsWnd::Reset()
{
	inherited::Reset();
	Reload();
}

void CUIPdaContactsWnd::OnPropertyPdaSendInviteClicked(CPda* temp_pda)
{
	Msg("Sending invite to %s", temp_pda->H_Parent()->cName().c_str());
	game_PlayerState* ps = Level().game->GetPlayerByGameID(temp_pda->H_Parent()->ID());
	if (!ps) return; //Send only to players
	if (ps->MPSquadID != NULL)
	{
		Msg("! SQUAD: Player already in squad!");
		return;
	}

	Msg("--- PlayerState No Has Squad Send Event !!!");

	NET_Packet P;
	CGameObject::u_EventGen(P, GE_PDA_SQUAD_SEND_INVITE, Level().CurrentControlEntity()->ID());
	P.w_u16(ps->GameID);
	CGameObject::u_EventSend(P);
}

void CUIPdaContactsWnd::OnPropertyPdaCancelInviteClicked(CPda* temp_pda)
{
	Msg("Sending invite cancellation to %s", temp_pda->H_Parent()->cName().c_str());
	game_PlayerState* ps = Level().game->GetPlayerByGameID(temp_pda->H_Parent()->ID());
	if (!ps) return; //Send only to players

	Msg("--- PlayerState FINDED Send Event !!!");

	NET_Packet P;
	CGameObject::u_EventGen(P, GE_PDA_SQUAD_CANCEL_INVITE, Level().CurrentControlEntity()->ID());
	P.w_u16(ps->GameID);
	CGameObject::u_EventSend(P);
}


CUIPdaContactItem::CUIPdaContactItem(CUIPdaContactsWnd* cw)
{
	m_cw = cw;
}

CUIPdaContactItem::~CUIPdaContactItem()
{

}

extern CSE_ALifeTraderAbstract* ch_info_get_from_id(u16 id);

#include "ui/UICharacterInfo.h"

void CUIPdaContactItem::SetSelected(bool b)
{
	CUISelectable::SetSelected(b);
}

bool CUIPdaContactItem::OnMouseDown(int mouse_btn)
{
	if (mouse_btn == MOUSE_1)
	{
		m_cw->UIContactsList->SetSelected(this);
		m_cw->HidePropertiesBox();
		return true;
	}

	if (mouse_btn == MOUSE_2 || mouse_btn == MOUSE_3)
	{
		Msg("Set Selected THIS: %p", this);
		m_cw->UIContactsList->SetSelected(this);
		m_cw->ShowPropertiesBox();
		return true;
	}
	return false;
}