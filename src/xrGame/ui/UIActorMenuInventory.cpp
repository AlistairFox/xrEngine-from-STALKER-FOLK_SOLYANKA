#include "stdafx.h"
#include "UIActorMenu.h"
#include "../inventory.h"
#include "../inventoryOwner.h"
#include "UIInventoryUtilities.h"
#include "UIItemInfo.h"
#include "../Level.h"
#include "UICellItemFactory.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UICellCustomItems.h"
#include "UIItemInfo.h"
#include "UIFrameLineWnd.h"
#include "UIPropertiesBox.h"
#include "UIListBoxItem.h"
#include "UIMainIngameWnd.h"
#include "UIGameCustom.h"
#include "eatable_item_object.h"

#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../Artefact.h"
#include "../eatable_item.h"
#include "../BottleItem.h"
#include "../WeaponMagazined.h"
#include "../Medkit.h"
#include "../Antirad.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../UICursor.h"
#include "../MPPlayersBag.h"
#include "../player_hud.h"
#include "../CustomDetector.h"
#include "../PDA.h"

#include "../actor_defs.h"
#include "../CampFireSwitcher.h"
#include "Actor.h"
#include "UICharacterInfo.h"

#pragma optimize(off, "")

void move_item_from_to(u16 from_id, u16 to_id, u16 what_id);

void CUIActorMenu::InitInventoryMode()
{
	m_pInventoryBagList->Show			(true);
	m_pInventoryBeltList->Show			(true);
	m_pInventoryOutfitList->Show		(true);
	m_pInventoryHelmetList->Show		(true);
	m_pInventoryDetectorList->Show		(true);
	m_pInventoryPistolList->Show		(true);
	m_pInventoryAutomaticList->Show		(true);
	m_pQuickSlot->Show					(true);
	m_pTrashList->Show					(true);
	m_RightDelimiter->Show				(false);

	m_pInventoryPdaList->Show			(true);

	InitInventoryContents				(m_pInventoryBagList);

	VERIFY( CurrentGameUI() );
	CurrentGameUI()->UIMainIngameWnd->ShowZoneMap(true);

//	m_pInventoryKnifeList->Show(true);
}

void CUIActorMenu::DeInitInventoryMode()
{
	m_pTrashList->Show				(false);
}

void CUIActorMenu::SendEvent_ActivateSlot(u16 slot, u16 recipient)
{
	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ACTIVATE_SLOT, recipient);
	P.w_u16							(slot);
	CGameObject::u_EventSend		(P);
}

void CUIActorMenu::SendEvent_Item2Slot(PIItem pItem, u16 recipient, u16 slot_id)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2SLOT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	P.w_u16							(slot_id);
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToSlot);
};

void CUIActorMenu::SendEvent_Item2Belt(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2BELT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToBelt);
};

void CUIActorMenu::SendEvent_Item2Ruck(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2RUCK, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToRuck);
};

void CUIActorMenu::SendEvent_Item_Eat(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM_EAT, recipient);
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);
};

void CUIActorMenu::SendEvent_Item_Drop(PIItem pItem, u16 recipient)
{
	R_ASSERT(pItem->parent_id()==recipient);
	if (!IsGameTypeSingle())
		pItem->DenyTrade();

	NET_Packet					P;
	pItem->object().u_EventGen	(P,GE_OWNERSHIP_REJECT,pItem->parent_id());
	P.w_u16						(pItem->object().ID());
	pItem->object().u_EventSend	(P);
	PlaySnd						(eDropItem);
}

void CUIActorMenu::DropAllCurrentItem()
{
	if ( CurrentIItem() && !CurrentIItem()->IsQuestItem() )
	{
		u32 const cnt = CurrentItem()->ChildsCount();
		for( u32 i = 0; i < cnt; ++i )
		{
			CUICellItem*	itm  = CurrentItem()->PopChild(NULL);
			PIItem			iitm = (PIItem)itm->m_pData;
			SendEvent_Item_Drop( iitm, m_pActorInvOwner->object_id() );
		}

		SendEvent_Item_Drop( CurrentIItem(), m_pActorInvOwner->object_id() );
	}
	SetCurrentItem								(NULL);
}

bool CUIActorMenu::DropAllItemsFromRuck( bool quest_force )
{
	if ( !IsShown() || !m_pInventoryBagList || m_currMenuMode != mmInventory )
	{
		return false;
	}

	u32 const ci_count = m_pInventoryBagList->ItemsCount();
	for ( u32 i = 0; i < ci_count; ++i )
	{
		CUICellItem* ci = m_pInventoryBagList->GetItemIdx( i );
		VERIFY( ci );
		PIItem item = (PIItem)ci->m_pData;
		VERIFY( item );

		if ( !quest_force && item->IsQuestItem() )
		{
			continue;
		}

		u32 const cnt = ci->ChildsCount();
		for( u32 j = 0; j < cnt; ++j )
		{
			CUICellItem*	child_ci   = ci->PopChild(NULL);
			PIItem			child_item = (PIItem)child_ci->m_pData;
			SendEvent_Item_Drop( child_item, m_pActorInvOwner->object_id() );
		}
		SendEvent_Item_Drop( item, m_pActorInvOwner->object_id() );
	}

	SetCurrentItem( NULL );
	return true;
}

bool FindItemInList(CUIDragDropListEx* lst, PIItem pItem, CUICellItem*& ci_res)
{
	u32 count = lst->ItemsCount();
	for (u32 i=0; i<count; ++i)
	{
		CUICellItem* ci				= lst->GetItemIdx(i);
		for(u32 j=0; j<ci->ChildsCount(); ++j)
		{
			CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(ci->Child(j));
			if(ici->object()==pItem)
			{
				ci_res = ici;
				//lst->RemoveItem(ci,false);
				return true;
			}
		}

		CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(ci);
		if(ici->object()==pItem)
		{
			ci_res = ci;
			//lst->RemoveItem(ci,false);
			return true;
		}
	}
	return false;
}

bool RemoveItemFromList(CUIDragDropListEx* lst, PIItem pItem)
{// fixme
	CUICellItem*	ci	= NULL;
	if(FindItemInList(lst, pItem, ci))
	{
		R_ASSERT		(ci);

		CUICellItem* dying_cell = lst->RemoveItem	(ci, false);
		xr_delete(dying_cell);

		return			true;
	}else
		return			false;
}

void CUIActorMenu::OnInventoryAction(PIItem pItem, u16 action_type)
{
	CUIDragDropListEx* all_lists[] =
	{
		m_pInventoryBeltList,
		m_pInventoryPistolList,
		m_pInventoryAutomaticList,
		m_pInventoryOutfitList,
		m_pInventoryHelmetList,
		m_pInventoryDetectorList,
		m_pInventoryBagList,
		m_pTradeActorBagList,
		m_pTradeActorList,
		m_pInventoryPdaList,
		//m_pInventoryKnifeList,
		NULL
	};

	switch (action_type)
	{
		case GE_TRADE_BUY :
		case GE_OWNERSHIP_TAKE :
			{
				u32 i			= 0;
				bool b_already	= false;

				CUIDragDropListEx* lst_to_add		= NULL;

				SInvItemPlace pl						= pItem->m_ItemCurrPlace;
				if ( pItem->BaseSlot() == GRENADE_SLOT )
				{
					pl.type		= eItemPlaceRuck;
					pl.slot_id	= GRENADE_SLOT;
				}

#ifndef MASTER_GOLD
				Msg("item place [%d]", pl);
#endif // #ifndef MASTER_GOLD

				if(pl.type==eItemPlaceSlot)
					lst_to_add						= GetSlotList(pl.slot_id);
				else if(pl.type==eItemPlaceBelt)
					lst_to_add						= GetListByType(iActorBelt);
				else/* if(pl.type==eItemPlaceRuck)*/
				{
					if(pItem->parent_id()==m_pActorInvOwner->object_id())
						lst_to_add						= GetListByType(iActorBag);
					else
						lst_to_add						= GetListByType(iDeadBodyBag);
				}

				while ( all_lists[i] )
				{
					CUIDragDropListEx*	curr = all_lists[i];
					CUICellItem*		ci   = NULL;

					if ( FindItemInList(curr, pItem, ci) )
					{
						if ( lst_to_add != curr )
						{
							RemoveItemFromList(curr, pItem);
						}
						else
						{
							b_already = true;
						}
						//break;
					}
					++i;
				}
				CUICellItem*		ci   = NULL;
				if(GetMenuMode()==mmDeadBodySearch && FindItemInList(m_pDeadBodyBagList, pItem, ci))
					break;

				if ( !b_already )
				{
					if ( lst_to_add )
					{
						CUICellItem* itm	= create_cell_item(pItem);
						lst_to_add->SetItem	(itm);
					}
				}
				if(m_pActorInvOwner)
					m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
			}break;
		case GE_TRADE_SELL :
		case GE_OWNERSHIP_REJECT :
			{
				if(CUIDragDropListEx::m_drag_item)
				{
					CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(CUIDragDropListEx::m_drag_item->ParentItem());
					R_ASSERT(ici);
					if(ici->object()==pItem)
					{
						CUIDragDropListEx*	_drag_owner		= ici->OwnerList();
						_drag_owner->DestroyDragItem		();
					}
				}

				if (!IsGameTypeSingle() && pItem->parent_id() != m_pActorInvOwner->object_id())
				{
					CUIDragDropListEx* lst_to_remove = NULL;
					lst_to_remove = GetListByType(iDeadBodyBag);
					RemoveItemFromList(lst_to_remove, pItem);
				}
				else
				{
					u32 i = 0;
					while (all_lists[i])
					{
						CUIDragDropListEx* curr = all_lists[i];
						if (RemoveItemFromList(curr, pItem))
						{
							break;
						}
						++i;
					}
				}

				if(m_pActorInvOwner)
					m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
			}break;
	}
	UpdateItemsPlace();
	UpdateConditionProgressBars();
}
void CUIActorMenu::AttachAddon(PIItem item_to_upgrade)
{
	PlaySnd										(eAttachAddon);
	R_ASSERT									(item_to_upgrade);
	if (OnClient())
	{
		NET_Packet								P;
		CGameObject::u_EventGen					(P, GE_ADDON_ATTACH, item_to_upgrade->object().ID());
		P.w_u16									(CurrentIItem()->object().ID());
		CGameObject::u_EventSend				(P);
	};

	item_to_upgrade->Attach						(CurrentIItem(), true);

	SetCurrentItem								(NULL);
}

void CUIActorMenu::DetachAddon(LPCSTR addon_name, PIItem itm)
{
	PlaySnd										(eDetachAddon);
	if (OnClient())
	{
		NET_Packet								P;
		if(itm==NULL)
			CGameObject::u_EventGen				(P, GE_ADDON_DETACH, CurrentIItem()->object().ID());
		else
			CGameObject::u_EventGen				(P, GE_ADDON_DETACH, itm->object().ID());

		P.w_stringZ								(addon_name);
		CGameObject::u_EventSend				(P);
		return;
	}
	if(itm==NULL)
		CurrentIItem()->Detach					(addon_name, true);
	else
		itm->Detach								(addon_name, true);
}

void CUIActorMenu::InitCellForSlot( u16 slot_idx )
{
	VERIFY( KNIFE_SLOT <= slot_idx && slot_idx <= LAST_SLOT );
	PIItem item	= m_pActorInvOwner->inventory().ItemFromSlot(slot_idx);
	if ( !item )
	{
		return;
	}

	CUIDragDropListEx* curr_list	= GetSlotList( slot_idx );
	CUICellItem* cell_item			= create_cell_item( item );
	curr_list->SetItem( cell_item );
	if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		ColorizeItem( cell_item, !CanMoveToPartner( item ) );
}

void CUIActorMenu::InitInventoryContents(CUIDragDropListEx* pBagList)
{
	ClearAllLists				();
	m_pMouseCapturer			= NULL;
	m_UIPropertiesBox->Hide		();
	SetCurrentItem				(NULL);

	CUIDragDropListEx*			curr_list = NULL;
	//Slots
	InitCellForSlot				(INV_SLOT_2);
	InitCellForSlot				(INV_SLOT_3);
	InitCellForSlot				(OUTFIT_SLOT);
	InitCellForSlot				(DETECTOR_SLOT);
	InitCellForSlot				(GRENADE_SLOT);
	InitCellForSlot				(HELMET_SLOT);
	//InitCellForSlot				(PDA_SLOT);
	//InitCellForSlot				(KNIFE_SLOT);

	curr_list					= m_pInventoryBeltList;
	TIItemContainer::iterator itb = m_pActorInvOwner->inventory().m_belt.begin();
	TIItemContainer::iterator ite = m_pActorInvOwner->inventory().m_belt.end();
	for ( ; itb != ite; ++itb )
	{
		CUICellItem* itm		= create_cell_item(*itb);
		curr_list->SetItem		(itm);
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );
	}

	TIItemContainer				ruck_list;
	ruck_list					= m_pActorInvOwner->inventory().m_ruck;
	std::sort					( ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck );

	curr_list					= pBagList;

	itb = ruck_list.begin();
	ite = ruck_list.end();
	for ( ; itb != ite; ++itb )
	{
		CMPPlayersBag* bag = smart_cast<CMPPlayersBag*>( &(*itb)->object() );
		if ( bag )
			continue;

		CUICellItem* itm = create_cell_item( *itb );
		curr_list->SetItem(itm);
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );

		//CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(*itb);
		//if(outfit)
		//	outfit->ReloadBonesProtection();

		//CHelmet* helmet = smart_cast<CHelmet*>(*itb);
		//if(helmet)
		//	helmet->ReloadBonesProtection();
	}
	m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
}

bool CUIActorMenu::TryActiveSlot(CUICellItem* itm)
{
	PIItem	iitem	= (PIItem)itm->m_pData;
	u16 slot		= iitem->BaseSlot();

	if ( slot == GRENADE_SLOT )
	{
		PIItem	prev_iitem = m_pActorInvOwner->inventory().ItemFromSlot(slot);
		if ( prev_iitem && (prev_iitem->object().cNameSect() != iitem->object().cNameSect()) )
		{
			SendEvent_Item2Ruck( prev_iitem, m_pActorInvOwner->object_id() );
			SendEvent_Item2Slot( iitem, m_pActorInvOwner->object_id(), slot );
		}
		SendEvent_ActivateSlot( slot, m_pActorInvOwner->object_id() );
		return true;
	}
	if ( slot == DETECTOR_SLOT )
	{

	}
	return false;
}

bool CUIActorMenu::ToSlot(CUICellItem* itm, bool force_place, u16 slot_id)
{
	CUIDragDropListEx*	old_owner			= itm->OwnerList();
	PIItem	iitem							= (PIItem)itm->m_pData;

	bool b_own_item							= (iitem->parent_id()==m_pActorInvOwner->object_id());
	if (slot_id==HELMET_SLOT)
	{
		CCustomOutfit* pOutfit = m_pActorInvOwner->GetOutfit();
		if(pOutfit && !pOutfit->bIsHelmetAvaliable)
			return false;
	}

	if(m_pActorInvOwner->inventory().CanPutInSlot(iitem, slot_id))
	{
		CUIDragDropListEx* new_owner		= GetSlotList(slot_id);

		if ( slot_id == GRENADE_SLOT || !new_owner )
		{
			return true; //fake, sorry (((
		}

		if(slot_id==OUTFIT_SLOT)
		{
			CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(iitem);
			if(pOutfit && !pOutfit->bIsHelmetAvaliable)
			{
				CUIDragDropListEx* helmet_list		= GetSlotList(HELMET_SLOT);
				if(helmet_list->ItemsCount()==1)
				{
					CUICellItem* helmet_cell		= helmet_list->GetItemIdx(0);
					ToBag(helmet_cell, false);
				}
			}
		}


		bool result							= (!b_own_item) || m_pActorInvOwner->inventory().Slot(slot_id, iitem);
		VERIFY								(result);

		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );

		new_owner->SetItem					(i);

		SendEvent_Item2Slot					(iitem, m_pActorInvOwner->object_id(), slot_id);

		SendEvent_ActivateSlot				(slot_id, m_pActorInvOwner->object_id());

		//ColorizeItem						( itm, false );
		if ( slot_id == OUTFIT_SLOT )
		{
			MoveArtefactsToBag();
		}
		return								true;
	}
	else
	{ // in case slot is busy
		if ( !force_place || slot_id == NO_ACTIVE_SLOT ) 
			return false;

		if ( m_pActorInvOwner->inventory().SlotIsPersistent(slot_id) && slot_id != DETECTOR_SLOT  )
			return false;

		if ( slot_id == INV_SLOT_2 && m_pActorInvOwner->inventory().CanPutInSlot(iitem, INV_SLOT_3))
			return ToSlot(itm, force_place, INV_SLOT_3);

		if ( slot_id == INV_SLOT_3 && m_pActorInvOwner->inventory().CanPutInSlot(iitem, INV_SLOT_2))
			return ToSlot(itm, force_place, INV_SLOT_2);

		bool result = false;

		if (slot_id <= LAST_SLOT)
		{
			PIItem	_iitem = m_pActorInvOwner->inventory().ItemFromSlot(slot_id);
			CUIDragDropListEx* slot_list = GetSlotList(slot_id);
			
			if (slot_list != nullptr)
			{
				// Msg("Try move to slot: %u item: %s", slot_id, _iitem->NameItem());
				VERIFY(slot_list->ItemsCount() == 1);

				CUICellItem* slot_cell = slot_list->GetItemIdx(0);
				VERIFY(slot_cell && ((PIItem)slot_cell->m_pData) == _iitem);

				result = ToBag(slot_cell, false);
				VERIFY(result);

				result = ToSlot(itm, false, slot_id);
				if (b_own_item && result && slot_id == DETECTOR_SLOT)
				{
					CCustomDetector* det = smart_cast<CCustomDetector*>(iitem);
					det->ToggleDetector(g_player_hud->attached_item(0) != NULL);
				}
			}
			else
			{
				Debug.LogStackTrace("slot_list == nullptr");
				Msg("[ERROR] UI Dragdrop list is nullptr");
			}
		}
		else
		{
			Debug.LogStackTrace("slot_id > LAST_SLOT");

			Msg("[ERROR] Slot ID: %u > max: %u", slot_id, LAST_SLOT);
		}

		return result;
	}
}


bool CUIActorMenu::ToBag(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	bool b_own_item						= (iitem->parent_id()==m_pActorInvOwner->object_id());

	bool b_already						= m_pActorInvOwner->inventory().InRuck(iitem);

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= NULL;
	if(b_use_cursor_pos)
	{
			new_owner					= CUIDragDropListEx::m_drag_item->BackList();
			VERIFY						(GetListType(new_owner)==iActorBag);
	}else
			new_owner					= GetListByType(iActorBag);

	if(m_pActorInvOwner->inventory().CanPutInRuck(iitem) || (b_already && (new_owner!=old_owner)) )
	{
		// Pavel: ���� ������� � iActorTrade, �� �� ��� ������ ��������� � �������
		// �������� ����� ��� ����, ����� �� ������������ ������� � ��,
		// ��� �������������� �� iActorTrade
		if (GetListType(old_owner) != iActorTrade)
		{
			bool result = b_already || (!b_own_item || m_pActorInvOwner->inventory().Ruck(iitem));
			R_ASSERT(result);
		}

		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		if(!i)
			return false;

		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);


		// Pavel: ���� ������� ��������� � iActorTrade, �� �� ��� ������ ��������� � �������
		// ������� ����� ����� �� �����
		if ((!b_already || !b_own_item) && GetListType(old_owner) != iActorTrade)
		{
			SendEvent_Item2Ruck(iitem, m_pActorInvOwner->object_id());
		}

		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( iitem ) );
		}
		return true;
	}
	return false;
}

bool CUIActorMenu::ToBelt(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;
	bool b_own_item						= (iitem->parent_id()==m_pActorInvOwner->object_id());

	if(m_pActorInvOwner->inventory().CanPutInBelt(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pInventoryBeltList);
		}else
				new_owner					= m_pInventoryBeltList;

		bool result							= (!b_own_item) || m_pActorInvOwner->inventory().Belt(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );

		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		if (IsGameTypeSingle())
		{
			if (!b_own_item)
				SendEvent_Item2Belt(iitem, m_pActorInvOwner->object_id());
		}
		else
		{
			SendEvent_Item2Belt(iitem, m_pActorInvOwner->object_id());
		}

		//ColorizeItem						(itm, false);
		return								true;
	}
	else
	{ // in case belt slot is busy
		if(!iitem->Belt() || m_pActorInvOwner->inventory().BeltWidth()==0)
			return false;

		CUIDragDropListEx* belt_list		= NULL;
		if(b_use_cursor_pos)
			belt_list						= CUIDragDropListEx::m_drag_item->BackList();
		else
			return false;

		Ivector2 belt_cell_pos				= belt_list->PickCell(GetUICursor().GetCursorPosition());
		if(belt_cell_pos.x==-1 && belt_cell_pos.y==-1)
			return false;

//		PIItem	_iitem						= m_pActorInvOwner->inventory().ItemFromSlot(slot_id);

		CUICellItem* slot_cell				= belt_list->GetCellAt(belt_cell_pos).m_item;
//		VERIFY								(slot_cell && ((PIItem)slot_cell->m_pData)==_iitem);

		bool result							= ToBag(slot_cell, false);
		VERIFY								(result);

		result								= ToBelt(itm, b_use_cursor_pos);
		return result;
	}
}
CUIDragDropListEx* CUIActorMenu::GetSlotList(u16 slot_idx)
{
	if ( slot_idx == NO_ACTIVE_SLOT )
	{
		return NULL;
	}
	switch ( slot_idx )
	{
		case INV_SLOT_2:
			return m_pInventoryPistolList;
			break;

		case INV_SLOT_3:
			return m_pInventoryAutomaticList;
			break;

		case OUTFIT_SLOT:
			return m_pInventoryOutfitList;
			break;

		case HELMET_SLOT:
			return m_pInventoryHelmetList;
			break;

		case DETECTOR_SLOT:
			return m_pInventoryDetectorList;
			break;

		case GRENADE_SLOT://fake
			if ( m_currMenuMode == mmTrade )
			{
				return m_pTradeActorBagList;
			}
			return m_pInventoryBagList;
		break;

		case PDA_SLOT:
			return m_pInventoryPdaList;
		break;

		//case KNIFE_SLOT:
		//	return m_pInventoryKnifeList;
		break;
	};
	return NULL;
}

bool CUIActorMenu::TryUseItem( CUICellItem* cell_itm )
{
	if ( !cell_itm )
	{
		return false;
	}
	PIItem item	= (PIItem)cell_itm->m_pData;

	CBottleItem*	pBottleItem		= smart_cast<CBottleItem*>	(item);
	CMedkit*		pMedkit			= smart_cast<CMedkit*>		(item);
	CAntirad*		pAntirad		= smart_cast<CAntirad*>		(item);
	CEatableItem*	pEatableItem	= smart_cast<CEatableItem*>	(item);
	CCampFireSwitcher* pCampSwitcher = smart_cast<CCampFireSwitcher*>(item);
	CArtefact* pArtefact			= smart_cast<CArtefact*>(item);
	
	if (pArtefact)
	{
		//pArtefact->ActivateArtefactFast();
		Actor()->inventory().Slot(ARTEFACT_SLOT, pArtefact, false, true);
		Actor()->inventory().Activate(ARTEFACT_SLOT);
		return true;
	}

	if (pCampSwitcher && !pCampSwitcher->GetNearestCampFire())
		return false;


	if ( !(pMedkit || pAntirad || pEatableItem || pBottleItem || pCampSwitcher) )
	{
		return false;
	}
	if ( !item->Useful() )
	{
		return false;
	}
	u16 recipient = m_pActorInvOwner->object_id();
	if ( item->parent_id() != recipient )
	{
		//move_item_from_to	(itm->parent_id(), recipient, itm->object_id());
		cell_itm->OwnerList()->RemoveItem( cell_itm, false );
	}

	SendEvent_Item_Eat		( item, recipient );
	PlaySnd					( eItemUse );
	SetCurrentItem			( NULL );
	return true;
}

bool CUIActorMenu::TryGiftItem(CUICellItem* cell_itm)
{
	if (!cell_itm)
		return false;

	PIItem item = (PIItem)cell_itm->m_pData;

	if (!item)
		return false;

	CActor* who_give = smart_cast<CActor*>(Level().Objects.net_Find(m_pActorInvOwner->object_id()));
	CActor* to_give = smart_cast<CActor*>(who_give->PersonWeLookingAt());

	if (!who_give) return false;
	if (!to_give) return false;

	move_item_from_to(who_give->ID(), to_give->ID(), item->object_id());

		NET_Packet P;
		who_give->u_EventGen(P, GE_ACTOR_GIFT_RECIEVE, who_give->ID());
		P.w_u8(1);
		P.w_u16(item->object_id());
		who_give->u_EventSend(P);

		who_give->u_EventGen(P, GE_ACTOR_GIFT_RECIEVE, to_give->ID());
		P.w_u8(0);
		P.w_u16(item->object_id());
		who_give->u_EventSend(P);

	SetCurrentItem(NULL);
	return true;
}

bool CUIActorMenu::TryInjectItem(CUICellItem* cell_itm)
{
	if (!cell_itm)
		return false;

	PIItem item = (PIItem)cell_itm->m_pData;

	if (!item)
		return false;

	CActor* who_give = smart_cast<CActor*>(Level().Objects.net_Find(m_pActorInvOwner->object_id()));
	CActor* to_give = smart_cast<CActor*>(who_give->PersonWeLookingAt());

	if (!who_give) return false;
	if (!to_give) return false;

	move_item_from_to(who_give->ID(), to_give->ID(), item->object_id());


		NET_Packet Pack;
		who_give->u_EventGen(Pack, GE_ACTOR_INJECT_RECIEVE, who_give->ID());
		Pack.w_u8(1);
		Pack.w_u16(item->object_id());
		who_give->u_EventSend(Pack);
		//Level().Server->SendTo(data->ID, Pack, net_flags(TRUE, TRUE));

		who_give->u_EventGen(Pack, GE_ACTOR_INJECT_RECIEVE, to_give->ID());
		Pack.w_u8(0);
		Pack.w_u16(item->object_id());
		who_give->u_EventSend(Pack);
		//Level().Server->SendTo(data2->ID, Pack, net_flags(TRUE, TRUE));


	//NET_Packet P;
	//who_give->u_EventGen(P, GE_ACTOR_INJECT_ITEM, -1);
	//P.w_u16(who_give->ID());
	//P.w_u16(to_give->ID());
	//P.w_u16(item->object().ID());
	//who_give->u_EventSend(P);

	SetCurrentItem(NULL);
	return true;
}


bool CUIActorMenu::ToQuickSlot(CUICellItem* itm)
{
	PIItem iitem = (PIItem)itm->m_pData;
	CEatableItemObject* eat_item = smart_cast<CEatableItemObject*>(iitem);
	if(!eat_item)
		return false;

	u8 slot_idx = u8(m_pQuickSlot->PickCell(GetUICursor().GetCursorPosition()).x);
	if(slot_idx==255)
		return false;

	m_pQuickSlot->SetItem(create_cell_item(iitem), GetUICursor().GetCursorPosition());
	xr_strcpy(ACTOR_DEFS::g_quick_use_slots[slot_idx], iitem->m_section_id.c_str());
	return true;
}


bool CUIActorMenu::OnItemDropped(PIItem itm, CUIDragDropListEx* new_owner, CUIDragDropListEx* old_owner)
{
	CUICellItem*	_citem	= (new_owner->ItemsCount()==1) ? new_owner->GetItemIdx(0) : NULL;
	PIItem _iitem	= _citem ? (PIItem)_citem->m_pData : NULL;

	if(!_iitem)						return	false;
	if(!_iitem->CanAttach(itm))		return	false;

	if(old_owner != m_pInventoryBagList)	return	false;

	AttachAddon						(_iitem);

	return							true;
}

void CUIActorMenu::TryHidePropertiesBox()
{
	if ( m_UIPropertiesBox->IsShown() )
	{
		m_UIPropertiesBox->Hide();
	}
}

void CUIActorMenu::ActivatePropertiesBox()
{
	TryHidePropertiesBox();
	if ( !(m_currMenuMode == mmInventory || m_currMenuMode == mmDeadBodySearch || m_currMenuMode == mmUpgrade) )
	{
		return;
	}

	PIItem item = CurrentIItem();
	if ( !item )
	{
		return;
	}

	CUICellItem* cell_item = CurrentItem();
	m_UIPropertiesBox->RemoveAll();
	bool b_show = false;

	if ( m_currMenuMode == mmInventory || m_currMenuMode == mmDeadBodySearch)
	{
		PropertiesBoxForSlots( item, b_show );
		PropertiesBoxForWeapon( cell_item, item, b_show );
		PropertiesBoxForAddon( item, b_show );
		PropertiesBoxForUsing( item, b_show );
		PropertiesBoxForPlaying(item, b_show);
		if (m_currMenuMode == mmInventory)
 		{
 			PropertiesBoxForDrop(cell_item, item, b_show);
 			PropertiesBoxForGift(item, b_show);
  			PropertiesBoxForInject(item, b_show);
 		}
	}
	//else if ( m_currMenuMode == mmDeadBodySearch )
	//{
	//	PropertiesBoxForUsing( item, b_show );
	//}
	else if ( m_currMenuMode == mmUpgrade )
	{
		PropertiesBoxForRepair( item, b_show );
	}

	if ( b_show )
	{
		m_UIPropertiesBox->AutoUpdateSize();

		Fvector2 cursor_pos;
		Frect						vis_rect;
		GetAbsoluteRect				(vis_rect);
		cursor_pos					= GetUICursor().GetCursorPosition();
		cursor_pos.sub				(vis_rect.lt);
		m_UIPropertiesBox->Show		(vis_rect, cursor_pos);
		PlaySnd						(eProperties);
	}
}

void CUIActorMenu::PropertiesBoxForSlots( PIItem item, bool& b_show )
{
	CCustomOutfit* pOutfit	= smart_cast<CCustomOutfit*>( item );
	CHelmet* pHelmet		= smart_cast<CHelmet*>		( item );
	CInventory&  inv		= m_pActorInvOwner->inventory();

	// ����-������� ��� ���������� ������ ������������ ����: Dreess Outfit, ���� ������ ��� �����
	bool bAlreadyDressed	= false;
	u16 cur_slot			= item->BaseSlot();

	if (	!pOutfit && !pHelmet &&
			cur_slot != NO_ACTIVE_SLOT &&
			!inv.SlotIsPersistent(cur_slot) &&
			inv.CanPutInSlot(item, cur_slot) )
	{
		m_UIPropertiesBox->AddItem( "st_move_to_slot",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show = true;
	}
	if (	item->Belt() &&
			inv.CanPutInBelt( item ) )
	{
		m_UIPropertiesBox->AddItem( "st_move_on_belt",  NULL, INVENTORY_TO_BELT_ACTION );
		b_show = true;
	}

	if (	item->Ruck() &&
			inv.CanPutInRuck(item) &&
			( cur_slot == NO_ACTIVE_SLOT || !inv.SlotIsPersistent(cur_slot) ) )
	{
		if( !pOutfit )
		{
			if( !pHelmet )
				m_UIPropertiesBox->AddItem( "st_move_to_bag",  NULL, INVENTORY_TO_BAG_ACTION );
			else
				m_UIPropertiesBox->AddItem( "st_undress_helmet",  NULL, INVENTORY_TO_BAG_ACTION );
		}
		else
			m_UIPropertiesBox->AddItem( "st_undress_outfit",  NULL, INVENTORY_TO_BAG_ACTION );

		bAlreadyDressed = true;
		b_show			= true;
	}
	if ( pOutfit && !bAlreadyDressed )
	{
		m_UIPropertiesBox->AddItem( "st_dress_outfit",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show			= true;
	}

	CCustomOutfit* outfit_in_slot = m_pActorInvOwner->GetOutfit();
	if ( pHelmet && !bAlreadyDressed && (!outfit_in_slot || outfit_in_slot->bIsHelmetAvaliable))
	{
		m_UIPropertiesBox->AddItem( "st_dress_helmet",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show			= true;
	}
}

void CUIActorMenu::PropertiesBoxForWeapon( CUICellItem* cell_item, PIItem item, bool& b_show )
{
	//������������ ������� �� ����
	CWeapon*	pWeapon = smart_cast<CWeapon*>( item );
	if ( !pWeapon )
	{
		return;
	}

	if ( pWeapon->GrenadeLauncherAttachable() )
	{
		if ( pWeapon->IsGrenadeLauncherAttached() )
		{
			m_UIPropertiesBox->AddItem( "st_detach_gl",  NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON );
			b_show			= true;
		}
		else
		{
		}
	}
	if ( pWeapon->ScopeAttachable() )
	{
		if ( pWeapon->IsScopeAttached() )
		{
			m_UIPropertiesBox->AddItem( "st_detach_scope",  NULL, INVENTORY_DETACH_SCOPE_ADDON );
			b_show			= true;
		}
		else
		{
		}
	}
	if ( pWeapon->SilencerAttachable() )
	{
		if ( pWeapon->IsSilencerAttached() )
		{
			m_UIPropertiesBox->AddItem( "st_detach_silencer",  NULL, INVENTORY_DETACH_SILENCER_ADDON );
			b_show			= true;
		}
		else
		{
		}
	}
	if ( smart_cast<CWeaponMagazined*>(pWeapon) )
	{
		bool b = ( pWeapon->GetAmmoElapsed() !=0 );
		if ( !b )
		{
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>( (CWeapon*)cell_item->Child(i)->m_pData );
				if ( weap_mag && weap_mag->GetAmmoElapsed() )
				{
					b = true;
					break; // for
				}
			}
		}
		if ( b )
		{
			m_UIPropertiesBox->AddItem( "st_unload_magazine",  NULL, INVENTORY_UNLOAD_MAGAZINE );
			b_show = true;
		}
	}
}
#include "../string_table.h"
void CUIActorMenu::PropertiesBoxForAddon( PIItem item, bool& b_show )
{
	//������������� ������� � ��������� ����� (2 ��� 3)

	CScope*				pScope				= smart_cast<CScope*>			(item);
	CSilencer*			pSilencer			= smart_cast<CSilencer*>		(item);
	CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(item);
	CInventory*			inv					= &m_pActorInvOwner->inventory();

	PIItem	item_in_slot_2 = inv->ItemFromSlot(INV_SLOT_2);
	PIItem	item_in_slot_3 = inv->ItemFromSlot(INV_SLOT_3);

	if(!item_in_slot_2 && !item_in_slot_3)	return;

	if ( pScope )
	{
		if ( item_in_slot_2 && item_in_slot_2->CanAttach(pScope) )
		{
			shared_str str = CStringTable().translate("st_attach_scope_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_2->m_name.c_str());
			m_UIPropertiesBox->AddItem( str.c_str(),  (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON );
//			m_UIPropertiesBox->AddItem( "st_attach_scope_to_pistol",  (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		if ( item_in_slot_3 && item_in_slot_3->CanAttach(pScope) )
		{
			shared_str str = CStringTable().translate("st_attach_scope_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_3->m_name.c_str());
			m_UIPropertiesBox->AddItem( str.c_str(),  (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON );
//			m_UIPropertiesBox->AddItem( "st_attach_scope_to_rifle",  (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		return;
	}

	if ( pSilencer )
	{
		if ( item_in_slot_2 && item_in_slot_2->CanAttach(pSilencer) )
		{
			shared_str str = CStringTable().translate("st_attach_silencer_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_2->m_name.c_str());
			m_UIPropertiesBox->AddItem( str.c_str(),  (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON );
//			m_UIPropertiesBox->AddItem( "st_attach_silencer_to_pistol",  (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		if ( item_in_slot_3 && item_in_slot_3->CanAttach(pSilencer) )
		{
			shared_str str = CStringTable().translate("st_attach_silencer_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_3->m_name.c_str());
			m_UIPropertiesBox->AddItem( str.c_str(),  (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON );
//			m_UIPropertiesBox->AddItem( "st_attach_silencer_to_rifle",  (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		return;
	}


	if ( pGrenadeLauncher )
	{
		if ( item_in_slot_2 && item_in_slot_2->CanAttach(pGrenadeLauncher) )
		{
			shared_str str = CStringTable().translate("st_attach_gl_to_rifle");
			str.printf("%s %s", str.c_str(), item_in_slot_2->m_name.c_str());
			m_UIPropertiesBox->AddItem( str.c_str(),  (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON );
//			m_UIPropertiesBox->AddItem( "st_attach_gl_to_pistol",  (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		if ( item_in_slot_3 && item_in_slot_3->CanAttach(pGrenadeLauncher) )
		{
			shared_str str = CStringTable().translate("st_attach_gl_to_rifle");
			str.printf("%s %s", str.c_str(), item_in_slot_3->m_name.c_str());
			m_UIPropertiesBox->AddItem( str.c_str(),  (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON );
//			m_UIPropertiesBox->AddItem( "st_attach_gl_to_rifle",  (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
	}
}

void CUIActorMenu::PropertiesBoxForUsing( PIItem item, bool& b_show )
{
	CMedkit*		pMedkit			= smart_cast<CMedkit*>		(item);
	CAntirad*		pAntirad		= smart_cast<CAntirad*>		(item);
	CEatableItem*	pEatableItem	= smart_cast<CEatableItem*>	(item);
	CBottleItem*	pBottleItem		= smart_cast<CBottleItem*>	(item);
	CCampFireSwitcher* pCampSwitcher = smart_cast<CCampFireSwitcher*>(item);
	CArtefact* pArtefact			= smart_cast<CArtefact*>(item);

	LPCSTR act_str = NULL;

	if (pArtefact)
	{
		act_str = "st_activate";
	}
	else 
	if ( pMedkit || pAntirad)
	{
		act_str = "st_use";
	}
	else if ( pBottleItem )
	{
		act_str = "st_drink";
	}
	else if (pCampSwitcher)
	{
		if (pCampSwitcher->GetNearestCampFire())
		{
			shared_str str = CStringTable().translate("st_use");
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)pCampSwitcher, INVENTORY_CAMPSWITCH);
			b_show = true;
		}
	}
	else if ( pEatableItem )
	{
		CObject*	pObj			= smart_cast<CObject*>		(item);
		shared_str	section_name	= pObj->cNameSect();
		if ( !xr_strcmp(section_name,"vodka") || !(xr_strcmp(section_name,"energy_drink")) )
		{
			act_str = "st_drink";
		}
		else if( !xr_strcmp(section_name,"bread") || !xr_strcmp(section_name,"kolbasa") || !xr_strcmp(section_name,"conserva"))
		{
			act_str = "st_eat";
		}
		else
		{
			act_str = "st_use";
		}
	}


	if ( act_str )
	{
		m_UIPropertiesBox->AddItem( act_str,  NULL, INVENTORY_EAT_ACTION );
		b_show			= true;
	}
}

void CUIActorMenu::PropertiesBoxForPlaying(PIItem item, bool& b_show)
{
	CPda* pPda = smart_cast<CPda*>(item);
	if(!pPda || !pPda->CanPlayScriptFunction())
		return;

	LPCSTR act_str = "st_play";
	m_UIPropertiesBox->AddItem(act_str,  NULL, INVENTORY_PLAY_ACTION);
	b_show = true;
}

void CUIActorMenu::PropertiesBoxForDrop( CUICellItem* cell_item, PIItem item, bool& b_show )
{
	if ( !item->IsQuestItem() )
	{
		m_UIPropertiesBox->AddItem( "st_drop", NULL, INVENTORY_DROP_ACTION );
		b_show = true;

		if ( cell_item->ChildsCount() )
		{
			m_UIPropertiesBox->AddItem( "st_drop_all", (void*)33, INVENTORY_DROP_ACTION );
		}
	}
}

void CUIActorMenu::PropertiesBoxForRepair( PIItem item, bool& b_show )
{
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>( item );
	CWeapon*       pWeapon = smart_cast<CWeapon*>( item );
	CHelmet*       pHelmet = smart_cast<CHelmet*>( item );

	if ( (pOutfit || pWeapon || pHelmet) && item->GetCondition() < 0.99f )
	{
		m_UIPropertiesBox->AddItem( "ui_inv_repair", NULL, INVENTORY_REPAIR );
		b_show = true;
	}
}

void CUIActorMenu::PropertiesBoxForGift( PIItem item, bool& b_show )
 {
 	CActor* to_give = smart_cast<CActor*>(Actor()->PersonWeLookingAt());
 	CActor* who_give = smart_cast<CActor*>(Level().Objects.net_Find(m_pActorInvOwner->object_id()));
 
 	if ( who_give && who_give->is_player && to_give && to_give->is_player && who_give->Position().distance_to_sqr(to_give->Position()) < 49.f)
 	{
 		shared_str str = CStringTable().translate("st_gift");
 		str.printf("%s %s", str.c_str(), to_give->Name());
 		m_UIPropertiesBox->AddItem(str.c_str(), NULL, INVENTORY_GIFT_ITEM);
 		b_show = true;
 	}
 }

void CUIActorMenu::PropertiesBoxForInject(PIItem item, bool& b_show)
{
	if (!item->cast_eatable_item())
		return;

	CActor* to_give = smart_cast<CActor*>(Actor()->PersonWeLookingAt());
	CActor* who_give = smart_cast<CActor*>(Level().Objects.net_Find(m_pActorInvOwner->object_id()));

	if (who_give && who_give->is_player && to_give && to_give->is_player && who_give->Position().distance_to_sqr(to_give->Position()) < 49.f)
	{
		shared_str str = CStringTable().translate("st_inject");
		str.printf("%s %s", str.c_str(), to_give->Name());
		m_UIPropertiesBox->AddItem(str.c_str(), NULL, INVENTORY_INJECT_ITEM);
		b_show = true;
	}
}

void CUIActorMenu::ProcessPropertiesBoxClicked( CUIWindow* w, void* d )
{
	PIItem			item		= CurrentIItem();
	CUICellItem*	cell_item	= CurrentItem();
	if ( !m_UIPropertiesBox->GetClickedItem() || !item || !cell_item || !cell_item->OwnerList() )
	{
		return;
	}
	CWeapon* weapon = smart_cast<CWeapon*>( item );

	switch ( m_UIPropertiesBox->GetClickedItem()->GetTAG() )
	{
	case INVENTORY_TO_SLOT_ACTION:	ToSlot( cell_item, true, item->BaseSlot() );		break;
	case INVENTORY_TO_BELT_ACTION:	ToBelt( cell_item, false );		break;
	case INVENTORY_TO_BAG_ACTION:	ToBag ( cell_item, false );		break;
	case INVENTORY_EAT_ACTION:		TryUseItem( cell_item ); 		break;
	case INVENTORY_GIFT_ITEM:		TryGiftItem( cell_item ); 		break;
	case INVENTORY_INJECT_ITEM:		TryInjectItem( cell_item ); 	break;
	case INVENTORY_DROP_ACTION:
		{
			void* d = m_UIPropertiesBox->GetClickedItem()->GetData();
			if ( d == (void*)33 )
			{
				DropAllCurrentItem();
			}
			else
			{
				SendEvent_Item_Drop( item, m_pActorInvOwner->object_id() );
			}
			break;
		}
	case INVENTORY_ATTACH_ADDON:
		{
			PIItem item = CurrentIItem(); // temporary storing because of AttachAddon is setting curiitem to NULL
			AttachAddon((PIItem)(m_UIPropertiesBox->GetClickedItem()->GetData()));
			if(m_currMenuMode==mmDeadBodySearch)
				RemoveItemFromList(m_pDeadBodyBagList, item);
			
			break;
		}
	case INVENTORY_DETACH_SCOPE_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetScopeName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetScopeName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_DETACH_SILENCER_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetSilencerName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetSilencerName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetGrenadeLauncherName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetGrenadeLauncherName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_RELOAD_MAGAZINE:
		if ( weapon )
		{
			weapon->Action( kWPN_RELOAD, CMD_START );
		}
		break;
	case INVENTORY_UNLOAD_MAGAZINE:
		{
			if (IsGameTypeSingle())
			{
				CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>( (CWeapon*)cell_item->m_pData );
				if ( !weap_mag )
				{
					break;
				}

				weap_mag->UnloadMagazine();

				for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
				{
					CUICellItem*		child_itm		= cell_item->Child(i);
					CWeaponMagazined*	child_weap_mag	= smart_cast<CWeaponMagazined*>( (CWeapon*)child_itm->m_pData );
					if ( child_weap_mag )
					{
						child_weap_mag->UnloadMagazine();
					}
				}
			}
			else
			{
				CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>((CWeapon*)cell_item->m_pData);
				if (!weap_mag)
					break;
				NET_Packet packet;
				Game().u_EventGen(packet, GE_UNLOAD_AMMO, weap_mag->ID());
				Game().u_EventSend(packet);


				for (u32 i = 0; i < cell_item->ChildsCount(); ++i)
				{
					CUICellItem* child_itm = cell_item->Child(i);
					CWeaponMagazined* child_weap_mag = smart_cast<CWeaponMagazined*>((CWeapon*)child_itm->m_pData);
					
					if (!child_weap_mag)
						continue;

					NET_Packet packet;
					Game().u_EventGen(packet, GE_UNLOAD_AMMO, child_weap_mag->ID());
					Game().u_EventSend(packet);
				}

			}


			break;
		}
	case INVENTORY_REPAIR:
		{
			TryRepairItem(this,0);
			return;
			break;
		}
	case INVENTORY_PLAY_ACTION:
		{
			CPda* pPda = smart_cast<CPda*>(item);
			if(!pPda)
				break;
			pPda->PlayScriptFunction();
			break;
		}
	case INVENTORY_CAMPSWITCH:
	{
		CCampFireSwitcher* campfireswitcher = smart_cast<CCampFireSwitcher*>(item);
		if (!campfireswitcher)
			break;

		TryUseItem(cell_item);
		break;
	}
	}//switch

	SetCurrentItem( NULL );
	UpdateItemsPlace();
	UpdateConditionProgressBars();
}//ProcessPropertiesBoxClicked

void CUIActorMenu::UpdateOutfit()
{
	for ( u8 i = 0; i < e_af_count ; ++i )
	{
		m_belt_list_over[i]->SetVisible( true );
	}

	u32 af_count = m_pActorInvOwner->inventory().BeltWidth();
	VERIFY( 0 <= af_count && af_count <= 5 );

	VERIFY( m_pInventoryBeltList );
	CCustomOutfit* outfit    = m_pActorInvOwner->GetOutfit();
	if(outfit && !outfit->bIsHelmetAvaliable)
		m_HelmetOver->Show(true);
	else
		m_HelmetOver->Show(false);

	if ( !outfit )
	{
		MoveArtefactsToBag();
		return;
	}

	Ivector2 afc;
	afc.x = af_count;//1;
	afc.y = 1;//af_count;

	m_pInventoryBeltList->SetCellsCapacity( afc );

	for ( u8 i = 0; i < af_count ; ++i )
	{
		m_belt_list_over[i]->SetVisible( false );
	}

}

void CUIActorMenu::MoveArtefactsToBag()
{
	while ( m_pInventoryBeltList->ItemsCount() )
	{
		CUICellItem* ci = m_pInventoryBeltList->GetItemIdx(0);
		VERIFY( ci && ci->m_pData );
		ToBag( ci, false );
	}//for i
	m_pInventoryBeltList->ClearAll( true );
}
