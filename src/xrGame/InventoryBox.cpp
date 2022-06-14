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
	m_in_use   = false;
	m_can_take = true;
	m_closed   = false;
	
}

CInventoryBox::~CInventoryBox()
{
}

#include "alife_simulator.h"
#include "alife_object_registry.h"
#include "game_sv_freemp.h"

void CInventoryBox::OnEvent(NET_Packet& P, u16 type)
{
	inherited::OnEvent	(P, type);

	switch (type)
	{
	case GE_TRADE_BUY:
	case GE_OWNERSHIP_TAKE:
		{
			u16 id;
            P.r_u16					(id);

			CObject* itm			= Level().Objects.net_Find(id); 
			VERIFY(itm);

			u16 GameID;

			if (personal_safe && P.r_u8() == 1)
			{
				shared_str CLID;
				P.r_stringZ(CLID);
 				P.r_u16(GameID);
				m_safe_items[id] = CLID;
				Msg("InvBox PERSONAL [%s]", CLID.c_str());
			}
			

			m_items.push_back		(id);
			
			itm->H_SetParent		(this);
			itm->setVisible			(FALSE);
			itm->setEnabled			(FALSE);

			CInventoryItem *pIItem	= smart_cast<CInventoryItem*>(itm);
			VERIFY					(pIItem);

			if( CurrentGameUI())
			{
				if(CurrentGameUI()->ActorMenu().GetMenuMode()==mmDeadBodySearch)
				{
					if(this==CurrentGameUI()->ActorMenu().GetInvBox())
						if (personal_safe && GameID == Level().game->local_player->GameID)
							CurrentGameUI()->OnInventoryAction(pIItem, GE_OWNERSHIP_TAKE);
				}
			};

			if (OnServer())
			{
				game_sv_freemp* freemp_sv = smart_cast<game_sv_freemp*>(Level().Server->game);

				if (freemp_sv)
				{
					CSE_Abstract* ent = freemp_sv->server().ID_to_entity(itm->ID());
					CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(ent);
					if (dynamic)
					{
						freemp_sv->alife().register_in_objects(dynamic);
						Msg("Reg Item in Objects [%d][%s]", itm->ID(), itm->cNameSect().c_str());
					}
				}
			}


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
		 
			auto it = std::find(m_items.begin(), m_items.end(), id);
			VERIFY(it != m_items.end());
			if (it != m_items.end())
				m_items.erase(it);
			 
			 
			bool just_before_destroy		= !P.r_eof() && P.r_u8();
			bool dont_create_shell			= (type==GE_TRADE_SELL) || just_before_destroy;

			itm->H_SetParent	(NULL, dont_create_shell);

			if (!IsGameTypeSingle() && CurrentGameUI())
			{
				if (CurrentGameUI()->ActorMenu().GetMenuMode() == mmDeadBodySearch)
				{
					if (this == CurrentGameUI()->ActorMenu().GetInvBox())
						CurrentGameUI()->OnInventoryAction(smart_cast<CInventoryItem*>(itm), GE_OWNERSHIP_REJECT);
				}
			}

			if( m_in_use )
			{
				CGameObject* GO		= smart_cast<CGameObject*>(itm);
				Actor()->callback(GameObject::eInvBoxItemTake)( this->lua_game_object(), GO->lua_game_object() );
			}

			if (OnServer())
			{
				game_sv_freemp* freemp_sv = smart_cast<game_sv_freemp*>(Level().Server->game);

				if (freemp_sv)
				{
					CSE_Abstract* ent = freemp_sv->server().ID_to_entity(itm->ID());
					CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(ent);
					if (dynamic)
					{
						freemp_sv->alife().unregister_in_objects(dynamic);
						Msg("UNReg Item in Objects [%d][%s]", itm->ID(), itm->cNameSect().c_str());
					}

				}

			}



		}break;

		case GE_INV_BOX_PRIVATE_SAFE:
		{
			SE_Read_items_safe(P);
		}break;
	};
}

void CInventoryBox::UpdateCL()
{
	inherited::UpdateCL	();
}

void CInventoryBox::net_Destroy()
{
	inherited::net_Destroy	();
}
#include "../xrServerEntities/xrServer_Objects_Alife.h"
BOOL CInventoryBox::net_Spawn(CSE_Abstract* DC)
{
	inherited::net_Spawn	(DC);
	setVisible				(TRUE);
	setEnabled				(TRUE);
	set_tip_text			("inventory_box_use");
	
	CSE_ALifeInventoryBox*	pSE_box = smart_cast<CSE_ALifeInventoryBox*>(DC);

	if ( /*IsGameTypeSingle() &&*/ pSE_box )
	{
		m_can_take = pSE_box->m_can_take;
		m_closed   = pSE_box->m_closed;
		set_tip_text( pSE_box->m_tip_text.c_str() );
		
		if (strstr(pSE_box->s_name.c_str(), "inventory_safe"))
		{
			personal_safe = true;
		}
		else
		{
			personal_safe = false;
		}
	
	}

	

	if (OnClient() && personal_safe)
	{
		NET_Packet P;
		CGameObject::u_EventGen(P, GE_INV_BOX_PRIVATE_SAFE, ID());
		P.w_clientID(Level().game->local_svdpnid);
		CGameObject::u_EventSend(P);
	}

	return					TRUE;
}

void CInventoryBox::net_Relcase(CObject* O)
{
	inherited::net_Relcase(O);
}

#include "inventory_item.h"
void CInventoryBox::AddAvailableItems(TIItemContainer& items_container) const
{
	if (personal_safe)
	{
		for (auto safed_items : m_safe_items)
		{
			//Msg("Items In Safe[%d] CL[%d]", safed_items.first, safed_items.second.value());
			if (safed_items.second == Level().game->local_player->getName())
			{
				PIItem itm = smart_cast<PIItem>(Level().Objects.net_Find(safed_items.first));
				VERIFY(itm);
				items_container.push_back(itm);
			}
		}
		return;
	}


	xr_vector<u16>::const_iterator it = m_items.begin();
	xr_vector<u16>::const_iterator it_e = m_items.end();

	for(;it!=it_e;++it)
	{
		PIItem itm = smart_cast<PIItem>(Level().Objects.net_Find(*it));VERIFY(itm);
		items_container.push_back	(itm);
	}
}

void CInventoryBox::set_can_take( bool status )
{
	m_can_take = status;
	SE_update_status();
}

void CInventoryBox::set_closed( bool status, LPCSTR reason )
{
	m_closed = status;

	if ( reason && xr_strlen( reason ) )
	{
		set_tip_text( reason );
	}
	else
	{
		set_tip_text( "inventory_box_use" );
	}
	SE_update_status();
}

void CInventoryBox::SE_update_status()
{
	NET_Packet P;
	CGameObject::u_EventGen( P, GE_INV_BOX_STATUS, ID() );
	P.w_u8( (m_can_take)? 1 : 0 );
	P.w_u8( (m_closed)? 1 : 0 );
	P.w_stringZ( tip_text() );
	CGameObject::u_EventSend( P );
}

void CInventoryBox::SE_Read_items_safe(NET_Packet P)
{
	Msg("GE_INV_BOX_PRIVETE_SAFE");

	u16 size = P.r_u16();
	for (int i = 0; i < size; i++)
	{
		u16 id;
		shared_str name;
		P.r_u16(id);
		P.r_stringZ(name);
		//Msg("Sync Item [%d] Player [%s]", id, name.c_str());
		m_safe_items[id] = name;
	}
	 
}

void CInventoryBox::SE_Write_items_safe(ClientID id)
{  
	NET_Packet packet;
	CGameObject::u_EventGen(packet, GE_INV_BOX_PRIVATE_SAFE, ID());

	packet.w_u16(m_safe_items.size());

	for (auto item : m_safe_items)
	{
		packet.w_u16(item.first);
		packet.w_stringZ(item.second);
	}

	if (m_safe_items.size() > 0)
	if (OnServer())
	{
		Level().Server->SendTo(id, packet, net_flags(true, true));
	}
}
