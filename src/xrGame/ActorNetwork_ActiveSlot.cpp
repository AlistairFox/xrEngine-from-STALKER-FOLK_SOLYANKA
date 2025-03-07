#include "stdafx.h"
#include "Actor.h"
#include "Weapon.h"
#include "Inventory.h"
#include "HudItem.h"
#include "Level.h"

void CActor::SyncActiveSlot(NET_Packet& P)
{
	if (!g_Alive() || !Level().CurrentControlEntity())
		return;

	if (Level().CurrentControlEntity()->ID() == ID())
		return;

	u16 ActiveSlot;
	u16 ActiveItem;
	P.r_u16(ActiveSlot);
	P.r_u16(ActiveItem);

	CWeapon* Weapon = smart_cast<CWeapon*> (Level().Objects.net_Find(ActiveItem));
	CInventoryItem* ActiveSlotItem = smart_cast<CInventoryItem*> (inventory().ActiveItem());

	if (inventory().GetActiveSlot() != ActiveSlot)
		inventory().SetActiveSlot(ActiveSlot);

	if (ActiveSlot == NO_ACTIVE_SLOT)
		return;

	if (inventory().ItemFromSlot(ActiveSlot) == nullptr || inventory().ItemFromSlot(ActiveSlot) != ActiveSlotItem)
	{
		bool NotCorrectItem = ActiveSlotItem && ActiveSlotItem->object().ID() != ActiveItem;
		if (NotCorrectItem || !ActiveSlotItem)
		{
			CObject* obj = Level().Objects.net_Find(ActiveItem);
			CInventoryItem* itemINV = smart_cast<CInventoryItem*>(obj);
			CGameObject* game_object = smart_cast<CGameObject*>(obj);

			if (itemINV && itemINV->parent_id() == ID())
			{
				inventory().Slot(ActiveSlot, itemINV, true, true);
				Msg("Actor Put Item To Slot(%d) ID(%d) ", ActiveSlot, ActiveItem);
			}
		}
	}
	if (Weapon != nullptr)
		Weapon->OnUpdateActiveSlot(P);

}

void CActor::SyncPacketSlot()
{
	if (inventory().ActiveItem())
	{
		CWeapon* weapon = smart_cast<CWeapon*> (inventory().ActiveItem());
		if (weapon != nullptr)
		{
			NET_Packet packet;
			packet.w_begin(M_CL_UPDATE_ACTIVE_SLOT);
			packet.w_u16(ID());
			packet.w_u16(inventory().GetActiveSlot());
			packet.w_u16(u16(weapon->object_id()));
			weapon->WriteUpdateActiveSlot(packet);
			Level().Send(packet, net_flags(FALSE));
		}
	}
}
