#include "stdafx.h"
#include "UIInviteToSquadMsgBox.h"
#include "ui/UIWndCallback.h"
#include "ui/UIXmlInit.h"
#include "ui/UI3tButton.h"
#include "ui/UIFrameWindow.h"
#include "string_table.h"

#include "UIPdaContactsWnd.h"
#include "game_cl_freemp.h"

#define SQUAD_INVITE_XML		"xrmpe\\squad_invite.xml"

CUIInviteToSquadMsgBox::CUIInviteToSquadMsgBox()
{
	m_UIFrame = NULL;

	m_UIButtonAccept = NULL;
	m_UIButtonDecline = NULL;
	m_UIStaticText = NULL;
}

CUIInviteToSquadMsgBox::~CUIInviteToSquadMsgBox()
{
	Clear();
}

void CUIInviteToSquadMsgBox::OnInviteReceived(LPCSTR invite_message)
{
	if (!this->IsShown())
	{
		this->Show(true);
	}

	m_UIStaticText->SetText(invite_message);
}

void CUIInviteToSquadMsgBox::Init()
{
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, SQUAD_INVITE_XML);

	CUIXmlInit	xml_init;

	xml_init.InitWindow(uiXml, "main", 0, this);

	m_UIFrame = xr_new<CUIFrameWindow>();
	AttachChild(m_UIFrame);
	xml_init.InitFrameWindow(uiXml, "frame", 0, m_UIFrame);

	m_UIStaticText = xr_new<CUITextWnd>();
	AttachChild(m_UIStaticText);
	xml_init.InitTextWnd(uiXml, "message_text", 0, m_UIStaticText);
	//m_UIStaticText->SetText(invite_message);

	m_UIButtonAccept = xr_new<CUI3tButton>();
	AttachChild(m_UIButtonAccept);
	xml_init.Init3tButton(uiXml, "button_accept", 0, m_UIButtonAccept);
	Register(m_UIButtonAccept);
	AddCallback(m_UIButtonAccept, BUTTON_CLICKED/*UI_PDA_SQUAD_ACCEPT_INVITE*/, CUIWndCallback::void_function(this, &CUIInviteToSquadMsgBox::OnAccept));

	m_UIButtonDecline = xr_new<CUI3tButton>();
	AttachChild(m_UIButtonDecline);
	xml_init.Init3tButton(uiXml, "button_decline", 0, m_UIButtonDecline);
	Register(m_UIButtonDecline);
	AddCallback(m_UIButtonDecline, BUTTON_CLICKED/*UI_PDA_SQUAD_DECLINE_INVITE*/, CUIWndCallback::void_function(this, &CUIInviteToSquadMsgBox::OnDecline));

	this->Show(false);
}

void CUIInviteToSquadMsgBox::Clear()
{
	xr_delete(m_UIFrame);

	xr_delete(m_UIButtonAccept);
	xr_delete(m_UIButtonDecline);
	xr_delete(m_UIStaticText);
}

void CUIInviteToSquadMsgBox::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUIInviteToSquadMsgBox::OnAccept(CUIWindow* w, void* d)
{
	CUIPdaContactsWnd* temp_cnct_wnd = smart_cast<CUIPdaContactsWnd*>(GetParent()->GetParent());
	game_cl_freemp* game = smart_cast<game_cl_freemp*>(&Game());
	if (game) {
		u16 id = game->mp_squad_invites.back()->InviterID;
		SendJoinInSquad(id);
		game->mp_squad_invites.clear(); //delete all invites because we made a choice
	}

	game->m_bSwitchToNextInvite = true;
	Msg("Accept Invite");
	this->Show(false);
}

void CUIInviteToSquadMsgBox::OnDecline(CUIWindow* w, void* d)
{
	CUIPdaContactsWnd* temp_cnct_wnd = smart_cast<CUIPdaContactsWnd*>(GetParent()->GetParent());

	game_cl_freemp* game = smart_cast<game_cl_freemp*>(&Game());
	if (game)
		game->mp_squad_invites.pop_back(); //delete last invite and switch to next

	game->m_bSwitchToNextInvite = true;

	Msg("Decline Invite");
	this->Show(false);
}

void CUIInviteToSquadMsgBox::SendJoinInSquad(u16 id)
{
	NET_Packet P;
	CGameObject::u_EventGen(P, GE_PDA_SQUAD_RESPOND_INVITE, Level().CurrentControlEntity()->ID());
	P.w_u16(id);
	CGameObject::u_EventSend(P);
}