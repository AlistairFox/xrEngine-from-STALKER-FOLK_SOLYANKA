#include "stdafx.h"
#include "UIPdaListItem.h"
#include "Actor.h"
#include "ui/UIInventoryUtilities.h"
#include "string_table.h"
#include "ui/xrUIXmlParser.h"
#include "ui/UIXmlInit.h"
#include "character_info.h"
#include "ui/UIFrameWindow.h"
#include "InventoryOwner.h"
#include "ui/UICharacterInfo.h"
#include "ui/UIStatic.h"

#include "game_cl_mp.h"
#include "actor_mp_client.h"

#define			PDA_CONTACT_CHAR		"xrmpe\\pda_character.xml"

CUIPdaListItem::CUIPdaListItem()
{
	UIInfo = NULL;
}

CUIPdaListItem::~CUIPdaListItem()
{
}

void CUIPdaListItem::InitPdaListItem(Fvector2 pos, Fvector2 size)
{
	inherited::SetWndPos(pos);
	inherited::SetWndSize(size);

	CUIXml										uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, PDA_CONTACT_CHAR);

	CUIXmlInit xml_init;
	UIInfo = new CUICharacterInfo();
	UIInfo->SetAutoDelete(true);
	AttachChild(UIInfo);
	UIInfo->InitCharacterInfo(Fvector2().set(0, 0), size, PDA_CONTACT_CHAR);

	xml_init.InitAutoStaticGroup(uiXml, "pda_char_auto_statics", 0, this);
}

void CUIPdaListItem::InitCharacter(CInventoryOwner* pInvOwner)
{
	VERIFY(pInvOwner);
	// Msg("New Contact found, his name is %s", pInvOwner->Name());
	//UIInfo->InitCharacter(pInvOwner->object_id());

	// Se7kills Нужно поченить не забыть потом нормальное определение иконки
	// game_PlayerState* pPlayer = Level().game->GetPlayerByGameID(pInvOwner->cast_game_object()->ID());

	//if (pPlayer)
	//{
	//	UIInfo->InitCharacterOnClient(pPlayer->getName(), pInvOwner->CharacterInfo().Community().id(), pPlayer->getIcon());
	//}
	//else
	//{
		UIInfo->InitCharacterOnClient(pInvOwner->Name(), pInvOwner->CharacterInfo().Community().id(), pInvOwner->IconName());
	//}

}