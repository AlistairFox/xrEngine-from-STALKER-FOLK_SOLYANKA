#include "stdafx.h"
#include "pch_script.h"

#include "InventoryBox.h"
#include "level.h"
#include "actor.h"
#include "game_object_space.h"

#include "script_callback_ex.h"
#include "script_game_object.h"
#include "ui/UIActorMenu.h"
#include "uigamecustom.h"
#include "inventory_item.h"

CInventoryBox::CInventoryBox()
{
	m_in_use = false;
	m_can_take = true;
	m_closed = false;
}

CInventoryBox::~CInventoryBox()
{
}

void CInventoryBox::OnEvent(NET_Packet& P, u16 type)
{
	inherited::OnEvent(P, type);

	switch (type)
	{
		case GE_INV_BOX_PRIVATE_SAFE:
		{
			u16 size = P.r_u16();
			for (int i = 0; i < size; i++)
			{
				shared_str PlayerName;
				u32 itemID;
				P.r_stringZ(PlayerName);
				P.r_u32(itemID);
				m_safe_items[itemID] = PlayerName;
			}
		}	
		break;

		case GE_PRIVATE_INVENTORY_BUY:
		{
			shared_str player;
			P.r_stringZ(player);
 			u32 ID = P.r_u32();
			Msg("InvBox PERSONAL Player[%s] == Item[%u]", *player, ID);

			auto itm = Level().Objects.net_Find(ID);
			m_safe_items[ID] = player;
 
			itm->H_SetParent(this);
 			itm->setVisible(FALSE);
			itm->setEnabled(FALSE);

			CInventoryItem* pIItem = smart_cast<CInventoryItem*>(itm);
			VERIFY(pIItem);
			if (CurrentGameUI())
			{
				if (CurrentGameUI()->ActorMenu().GetMenuMode() == mmDeadBodySearch)
				{
					if (this == CurrentGameUI()->ActorMenu().GetInvBox())
						CurrentGameUI()->OnInventoryAction(pIItem, GE_OWNERSHIP_TAKE);
				}
			};

		}
		break;

		case GE_TRADE_BUY:
		case GE_OWNERSHIP_TAKE:
		{
			u16 id;
			P.r_u16(id);

			CObject* itm = Level().Objects.net_Find(id); 
			VERIFY(itm);
			m_items.push_back(id);

			itm->H_SetParent(this);

			itm->setVisible(FALSE);
			itm->setEnabled(FALSE);

			CInventoryItem* pIItem = smart_cast<CInventoryItem*>(itm);
			VERIFY(pIItem);
			if (CurrentGameUI())
			{
				if (CurrentGameUI()->ActorMenu().GetMenuMode() == mmDeadBodySearch)
				{
					if (this == CurrentGameUI()->ActorMenu().GetInvBox())
						CurrentGameUI()->OnInventoryAction(pIItem, GE_OWNERSHIP_TAKE);
				}
			};
		}break;

		case GE_TRADE_SELL:
		case GE_OWNERSHIP_REJECT:
	{
		u16 id;
		P.r_u16(id);
		CObject* itm = Level().Objects.net_Find(id);
		VERIFY(itm);

		if (personal_safe)
		{
			m_safe_items.erase(id);
		}
		else
		{
			auto it = std::find(m_items.begin(), m_items.end(), id);
			VERIFY(it != m_items.end());

			if (m_items.end() != it)
				m_items.erase(it);
		}


		bool just_before_destroy = !P.r_eof() && P.r_u8();
		bool dont_create_shell = (type == GE_TRADE_SELL) || just_before_destroy;

		itm->H_SetParent(NULL, dont_create_shell);

		if (!IsGameTypeSingle() && CurrentGameUI())
		{
			if (CurrentGameUI()->ActorMenu().GetMenuMode() == mmDeadBodySearch)
			{
				if (this == CurrentGameUI()->ActorMenu().GetInvBox())
					CurrentGameUI()->OnInventoryAction(smart_cast<CInventoryItem*>(itm), GE_OWNERSHIP_REJECT);
			}
		}

		if (m_in_use)
		{
			CGameObject* GO = smart_cast<CGameObject*>(itm);
			Actor()->callback(GameObject::eInvBoxItemTake)(this->lua_game_object(), GO->lua_game_object());
		}

	}break;


	};
}

void CInventoryBox::UpdateCL()
{
	inherited::UpdateCL();
}

void CInventoryBox::net_Destroy()
{
	inherited::net_Destroy();
}

#include "../xrServerEntities/xrServer_Objects_Alife.h"
BOOL CInventoryBox::net_Spawn(CSE_Abstract* DC)
{
	inherited::net_Spawn(DC);
	setVisible(TRUE);
	setEnabled(TRUE);
	set_tip_text("inventory_box_use");

	CSE_ALifeInventoryBox* pSE_box = smart_cast<CSE_ALifeInventoryBox*>(DC);
	if ( pSE_box)
	{
		m_can_take = pSE_box->m_can_take;
		m_closed = pSE_box->m_closed;
		set_tip_text(pSE_box->m_tip_text.c_str());
 
		if (pSE_box->m_ini_string.size() > 0)
		{
 			void* data	= (void*)(pSE_box->m_ini_string.c_str());
			int size	= xr_strlen(pSE_box->m_ini_string);			 
			CInifile* ini = new CInifile(&IReader(data, size), FS.get_path("$game_config$")->m_Path);
 			
			if (ini->section_exist("spawn"))
			{
				isTrausure = true;
				Msg("Set Box Is Tresure[%s] : [%u]", cName().c_str(), ID());
			}
			
			if (ini->section_exist("personal"))
			{
				personal_safe = true;
				Msg("Set Box Is Personal[%s] : [%u]", cName().c_str(), ID());
			}
		}
		else
		{
			//Msg("CInventoryBox is no contains ini file");
		}
	}

 	personal_safe = pSettings->line_exist(cNameSect(), "private_box") ? pSettings->r_bool(cNameSect(), "private_box") : false;
	isTrausure    = pSettings->line_exist(cNameSect(), "tresure_box") ? pSettings->r_bool(cNameSect(), "tresure_box") : false;
	
	if (personal_safe)
		set_tip_text("st_personal_box");
	if (isTrausure)
		set_tip_text("st_trause_box");

	if (OnClient())
		Recvest_items_safe(Level().game->local_svdpnid);

	return					TRUE;
}

void CInventoryBox::net_Relcase(CObject* O)
{
	inherited::net_Relcase(O);
}

#include "game_cl_roleplay.h"
void CInventoryBox::AddAvailableItems(TIItemContainer& items_container) const
{
//	Msg("m_items: %u, private: %u", m_items.size(), m_safe_items.size() );

	if (personal_safe)
	{
 		for (auto safed_items : m_safe_items)
		{
			if ( xr_strcmp ( Level().game->local_player->getLogin(), safed_items.second) == 0)
			{
				PIItem itm = smart_cast<PIItem>(Level().Objects.net_Find(safed_items.first));
				items_container.push_back(itm);
			}
		}
	//	return;
	}

	xr_vector<u16>::const_iterator it = m_items.begin();
	xr_vector<u16>::const_iterator it_e = m_items.end();

	for (; it != it_e; ++it)
	{
		PIItem itm = smart_cast<PIItem>(Level().Objects.net_Find(*it)); VERIFY(itm);
		items_container.push_back(itm);
	}
}

void CInventoryBox::set_can_take(bool status)
{
	m_can_take = status;
	SE_update_status();
}

void CInventoryBox::set_closed(bool status, LPCSTR reason)
{
	m_closed = status;

	if (reason && xr_strlen(reason))
	{
		set_tip_text(reason);
	}
	else
	{
		set_tip_text("inventory_box_use");
	}
	SE_update_status();
}

void CInventoryBox::SE_update_status()
{
	NET_Packet P;
	CGameObject::u_EventGen(P, GE_INV_BOX_STATUS, ID());
	P.w_u8((m_can_take) ? 1 : 0);
	P.w_u8((m_closed) ? 1 : 0);
	P.w_stringZ(tip_text());
	CGameObject::u_EventSend(P);
}

void CInventoryBox::SE_update_ITEMS_SAFE(ClientID id)
{
	NET_Packet P;
	CGameObject::u_EventGen(P, GE_INV_BOX_PRIVATE_SAFE, ID());

	P.w_u16(m_safe_items.size());

	for (auto item : m_safe_items)
	{
		P.w_stringZ(item.second);
		P.w_u32(item.first);
	}

	if (OnServer())
	{
		Level().Server->SendTo(id, P, net_flags(true, true));
	}
}

void CInventoryBox::Recvest_items_safe(ClientID id)
{
	NET_Packet P;
	CGameObject::u_EventGen(P, GE_INV_BOX_PRIVATE_SAFE, ID());
	P.w_clientID(id);
	CGameObject::u_EventSend(P);
}
