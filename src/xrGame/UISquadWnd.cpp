#include "stdafx.h"
#include "UISquadWnd.h"
#include "ui/UIXmlInit.h"
#include "ui/UIWndCallback.h"
#include "ui/UIListBoxItem.h"
#include "UICursor.h"
#include "Actor.h"
#include "ui/UIStatic.h"
#include "UIGameCustom.h"
#include "ui/UIActorMenu.h"

#include "game_cl_freemp.h"

#define PDA_SQUAD_XML	"xrmpe\\pda_squad.xml"

CUISquadWnd::CUISquadWnd() {}

CUISquadWnd::~CUISquadWnd()
{
	xr_delete(m_UIPropertiesBox);

	for (int i = 0; i < squad_size; i++)
	{
		xr_delete(m_frame[i]);
		xr_delete(m_cap[i]);
	}
}

void CUISquadWnd::Init()
{
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, PDA_SQUAD_XML);

	CUIXmlInit	xml_init;

	xml_init.InitWindow(uiXml, "squad_wnd", 0, this);

	//---------------------------FRAMES and CAPS
	string64 buff;
	for (int i = 0; i < squad_size; i++)
	{
		xr_sprintf(buff, "squad_wnd:frame_%d", i);

		m_frame[i] = xr_new<CUIFrameWindow>();
		AttachChild(m_frame[i]);

		xml_init.InitFrameWindow(uiXml, buff, 0, m_frame[i]);

		//---------------------------------------
		xr_sprintf(buff, "squad_wnd:cap_%d", i);

		m_cap[i] = xr_new<CUIStatic>();
		AttachChild(m_cap[i]);

		xml_init.InitStatic(uiXml, buff, 0, m_cap[i]);
	}

	//---------------------------SQUAD LIST
	m_SquadList = xr_new<CUIPdaSquadList>(); m_SquadList->SetAutoDelete(true);
	AttachChild(m_SquadList);
	xml_init.InitWindow(uiXml, "squad_wnd:squad_list", 0, m_SquadList);

	//---------------------------SQUAD LEADER
	m_leader_icon = xr_new<CUIStatic>(); m_leader_icon->SetAutoDelete(true);
	AttachChild(m_leader_icon);

	xml_init.InitWindow(uiXml, "squad_wnd:squad_leader", 0, m_leader_icon);
	m_leader_icon->Show(false);

	//---------------------------PropertiesBox
	m_UIPropertiesBox = xr_new<CUIPropertiesBox>();

	m_UIPropertiesBox->InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(300, 300), "xrmpe\\pda_contacts.xml");
	AttachChild(m_UIPropertiesBox);
	m_UIPropertiesBox->Hide();
	m_UIPropertiesBox->SetWindowName("property_box");

	//Callback
	Register(m_UIPropertiesBox);
	AddCallback(m_UIPropertiesBox, PROPERTY_CLICKED, CUIWndCallback::void_function(this, &CUISquadWnd::ProcessPropertiesBoxClicked));

	//Init Game()
	game = smart_cast<game_cl_freemp*>(&Game());

	pda_squad_item_distance = (int)m_frame[0]->GetWidth() + 4;
}

void CUISquadWnd::ProcessPropertiesBoxClicked(CUIWindow* w, void* d)
{
	CUIPdaSquadItem* pItem = smart_cast<CUIPdaSquadItem*>(m_SquadList->GetSelected());
	if (!pItem)
	{
		Msg("! FAILED to create member from selected item");
		return;
	}

	switch (m_UIPropertiesBox->GetClickedItem()->GetTAG())
	{
	case UI_PDA_SQUAD_MAKE_LEADER:
	{
		CurrentGameUI()->ActorMenu().PlaySnd(CUIActorMenu::eSquadAction);

		if (Level().GetClientID() != game->local_squad->squad_leader_cid)
		{
			Msg("! SQUAD: You are not squad leader!");
			return;
		}

		//Дали лидера
		game_PlayerState* ps = (game_PlayerState*)pItem->m_data;
		if (!ps) return;

		NET_Packet P;
		CGameObject::u_EventGen(P, GE_PDA_SQUAD_MAKE_LEADER, Level().CurrentControlEntity()->ID());
		P.w_u16(ps->GameID);
		CGameObject::u_EventSend(P);

	}break;
	case UI_PDA_SQUAD_KICK_PLAYER:
	{
		CurrentGameUI()->ActorMenu().PlaySnd(CUIActorMenu::eSquadAction);

		//Кикнули
		game_PlayerState* ps = (game_PlayerState*)pItem->m_data;
		if (!ps) return;

		NET_Packet P;
		CGameObject::u_EventGen(P, GE_PDA_SQUAD_KICK_PLAYER, Level().CurrentControlEntity()->ID());
		P.w_u16(ps->GameID);
		CGameObject::u_EventSend(P);
	}break;
	}
}

void CUISquadWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUISquadWnd::ShowPropertiesBox()
{
	m_UIPropertiesBox->RemoveAll();

	CurrentGameUI()->ActorMenu().PlaySnd(CUIActorMenu::eProperties);

	//leader
	if (Level().GetClientID() == game->local_squad->squad_leader_cid)
	{
		CUIPdaSquadItem* pItem = smart_cast<CUIPdaSquadItem*>(m_SquadList->GetSelected());
		if (pItem)
		{
			game_PlayerState* ps = (game_PlayerState*)pItem->m_data;
			if (ps->GameID == Game().local_player->GameID)
				PropertiesBoxForLeader(true);
			else
				PropertiesBoxForLeader(false);
		}
	}
	//member
	else
	{
		CUIPdaSquadItem* pItem = smart_cast<CUIPdaSquadItem*>(m_SquadList->GetSelected());
		if (pItem)
		{
			game_PlayerState* ps = (game_PlayerState*)pItem->m_data;
			if (ps->GameID == Game().local_player->GameID)
				PropertiesBoxForMember();
			else
				return;
		}
	}

	m_UIPropertiesBox->AutoUpdateSize();
	Fvector2 cursor_pos;
	Frect vis_rect;
	GetAbsoluteRect(vis_rect);
	cursor_pos = GetUICursor().GetCursorPosition();
	cursor_pos.sub(vis_rect.lt);

	m_UIPropertiesBox->Show(vis_rect, cursor_pos);
}

void CUISquadWnd::HidePropertiesBox()
{
	if (m_UIPropertiesBox->IsShown())
		m_UIPropertiesBox->Hide();
}

void CUISquadWnd::PropertiesBoxForLeader(bool b_self)
{
	if (b_self)
	{
		m_UIPropertiesBox->AddItem("st_squad_leave_squad", NULL, UI_PDA_SQUAD_KICK_PLAYER);
	}
	else
	{
		m_UIPropertiesBox->AddItem("st_squad_make_leader", NULL, UI_PDA_SQUAD_MAKE_LEADER);
		m_UIPropertiesBox->AddItem("st_squad_kick_player", NULL, UI_PDA_SQUAD_KICK_PLAYER);
	}
}

void CUISquadWnd::PropertiesBoxForMember()
{
	m_UIPropertiesBox->AddItem("st_squad_leave_squad", NULL, UI_PDA_SQUAD_KICK_PLAYER);
}

void CUISquadWnd::AddSquadMember(game_PlayerState* pPlayer)
{
	VERIFY(pPlayer);

	int member_number = -1;

	for (int i = 0; i < squad_size; i++)
	{
		if (!m_SquadList->FindChild("squad_member_" + i))
		{
			member_number = i;
			break;
		}
	}

	if (member_number == -1)
	{
		Msg("! The squad is full");
		return;
	}

	CUIPdaSquadItem* pItem = xr_new<CUIPdaSquadItem>(this);

	pItem->SetAutoDelete(true); m_SquadList->AttachChild(pItem);
	pItem->SetWindowName("squad_member_" + member_number);

	m_cap[member_number]->Show(false);

	Msg("ADD Squad Member In SquadUI[%s]", pPlayer->getName());

	pItem->InitSquadItem(Fvector2().set((pda_squad_item_distance * member_number), 0), Fvector2().set(84, 115));

	pItem->InitCharacter(pPlayer);

	pItem->m_data = (void*)pPlayer;

	game_PlayerState* squad_leader = Game().players[game->local_squad->squad_leader_cid];
	if (!squad_leader)
		return;

	if (squad_leader == pPlayer)
	{
		m_leader_icon->SetWndPos(pItem->GetWndPos());
		m_leader_icon->Show(true);
	}
}

void CUISquadWnd::RemoveSquadMember(CUIPdaSquadItem* squad_item)
{
	m_SquadList->DetachChild(squad_item);
	m_cap[squad_item->GetChildNum()]->Show(true);
	m_leader_icon->Show(false);
}

void CUISquadWnd::RemoveSquadMember(int member_number)
{
	if (m_SquadList->FindChild("squad_member_" + member_number)) m_SquadList->DetachChild(m_SquadList->FindChild("squad_member_" + member_number));
	m_cap[member_number]->Show(true);
	m_leader_icon->Show(false);
}

void CUISquadWnd::RemoveAll()
{
	for (int i = 0; i < squad_size; i++)
	{
		if (m_SquadList->FindChild("squad_member_" + i)) m_SquadList->DetachChild(m_SquadList->FindChild("squad_member_" + i));
		m_cap[i]->Show(true);
		m_leader_icon->Show(false);
	}
}

//----------------------------------------------------------------------- class CUIPdaSquadItem
#define PDA_SQUAD_CHARECTER		"xrmpe\\pda_squad_character.xml"

#include "Actor.h"
#include "character_info.h"
#include "InventoryOwner.h"
#include "ui/UICharacterInfo.h"

#include "game_cl_mp.h"
#include "actor_mp_client.h"

CUIPdaSquadItem::CUIPdaSquadItem(CUISquadWnd* sw)
{
	m_sw = sw;
	UIInfo = NULL;
}

CUIPdaSquadItem::~CUIPdaSquadItem()
{

}

extern CSE_ALifeTraderAbstract* ch_info_get_from_id(u16 id);

void CUIPdaSquadItem::SetSelected(bool b)
{
	CUISelectable::SetSelected(b);
}

bool CUIPdaSquadItem::OnMouseDown(int mouse_btn)
{
	if (mouse_btn == MOUSE_1)
	{
		m_sw->HidePropertiesBox();
		return true;
	}

	if (mouse_btn == MOUSE_2 || mouse_btn == MOUSE_3)
	{
		m_sw->m_SquadList->SetSelected(this);
		m_sw->ShowPropertiesBox();
		return true;
	}
	return false;
}

void CUIPdaSquadItem::InitSquadItem(Fvector2 pos, Fvector2 size)
{
	inherited::SetWndPos(pos);
	inherited::SetWndSize(size);

	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, PDA_SQUAD_CHARECTER);

	CUIXmlInit xml_init;
	UIInfo = xr_new<CUICharacterInfo>();
	UIInfo->SetAutoDelete(true);
	AttachChild(UIInfo);
	UIInfo->InitCharacterInfo(Fvector2().set(0, 0), size, PDA_SQUAD_CHARECTER);

	xml_init.InitAutoStaticGroup(uiXml, "pda_char_auto_statics", 0, this);
}

void CUIPdaSquadItem::InitCharacter(game_PlayerState* pPlayer)
{
	if (!pPlayer) return;

	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(Level().Objects.net_Find(pPlayer->GameID));

	// Se7kills Нужно поченить не забыть потом нормальное определение иконки
	if (pPlayer && pInvOwner)
	{
		UIInfo->InitCharacterOnClient(pPlayer->getName(), pInvOwner->CharacterInfo().Community().id(), /* pPlayer->getIcon() */ pInvOwner->IconName());
	}
	else
	{
		Msg("Player State not found");
	}

	m_data = pPlayer;
}

CUIPdaSquadList::CUIPdaSquadList() {}
CUIPdaSquadList::~CUIPdaSquadList() {}

void CUIPdaSquadList::SetSelected(CUIWindow* w)
{
	for (WINDOW_LIST_it it = GetChildWndList().begin(); GetChildWndList().end() != it; ++it)
	{
		smart_cast<CUISelectable*>(*it)->SetSelected(*it == w);
	}
}

CUIWindow* CUIPdaSquadList::GetSelected()
{
	for (WINDOW_LIST_it it = GetChildWndList().begin(); GetChildWndList().end() != it; ++it)
	{
		if (smart_cast<CUISelectable*>(*it)->GetSelected())
			return *it;
	}

	return NULL;
}