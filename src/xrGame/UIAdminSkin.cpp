#include "stdafx.h"
#include "UIAdminSkin.h"
#include "ui/UIXmlInit.h"
#include "object_broker.h"
#include "ui/UIStatic.h"
#include "ui/UI3tButton.h"
#include "ui/UIListBox.h"
#include "ui/UIListBoxItem.h"
#include "Level.h"
#include "Actor.h"
#include "../xrEngine/xr_ioconsole.h"
#include <dinput.h>
#include "player_hud.h"

CUISkinSelector::CUISkinSelector()
{
	xml_doc = nullptr;

	m_pBack = xr_new<CUIStatic>();
	m_pBack->SetAutoDelete(true);
	AttachChild(m_pBack);

	skin_tex_pic = xr_new<CUIStatic>();
	skin_tex_pic->SetAutoDelete(true);
	AttachChild(skin_tex_pic);

	skin_tex_frame = xr_new<CUIStatic>();
	skin_tex_frame->SetAutoDelete(true);
	AttachChild(skin_tex_frame);

	skin_list_box = xr_new<CUIListBox>();
	skin_list_box->SetAutoDelete(true);
	AttachChild(skin_list_box);

	m_pClose = xr_new<CUI3tButton>();
	m_pClose->SetAutoDelete(true);
	AttachChild(m_pClose);

	m_pSetVisual = xr_new<CUI3tButton>();
	m_pSetVisual->SetAutoDelete(true);
	AttachChild(m_pSetVisual);

	Init();
}

CUISkinSelector::~CUISkinSelector()
{
	xr_delete(xml_doc);
}

void CUISkinSelector::FillSkinList()
{
	//Drive: очищаем
	skin_list_box->Clear();

	if (!pSettings->section_exist("visuals_for_skin_selector"))
		return;

	CInifile::Sect& sect = pSettings->r_section("visuals_for_skin_selector");
	CInifile::SectCIt it_ = sect.Data.begin();
	CInifile::SectCIt it_e_ = sect.Data.end();

	CInifile::SectCIt it = it_;
	CInifile::SectCIt it_e = it_e_;

	for (; it != it_e; ++it)
	{
		string512 tmp_string;
		xr_sprintf(tmp_string, "%s", it->first.c_str());
		CUIListBoxItem* itm = skin_list_box->AddTextItem(tmp_string);
	}
}

void CUISkinSelector::Init()
{
	if (!xml_doc)
		xml_doc = xr_new<CUIXml>();

	xml_doc->Load(CONFIG_PATH, UI_PATH, "ui_mp_skin_selector.xml");

	CUIXmlInit::InitWindow(*xml_doc, "ui_skin_selector", 0, this);
	CUIXmlInit::InitStatic(*xml_doc, "ui_skin_selector:background", 0, m_pBack);
	CUIXmlInit::InitStatic(*xml_doc, "ui_skin_selector:skin_tex_frame", 0, skin_tex_frame);
	CUIXmlInit::InitStatic(*xml_doc, "ui_skin_selector:skin_tex_pic", 0, skin_tex_pic);
	CUIXmlInit::Init3tButton(*xml_doc, "ui_skin_selector:set_visual_button", 0, m_pSetVisual);
	CUIXmlInit::Init3tButton(*xml_doc, "ui_skin_selector:close_button", 0, m_pClose);

	FillSkinList();

	CUIXmlInit::InitListBox(*xml_doc, "ui_skin_selector:skin_list", 0, skin_list_box);
}

void CUISkinSelector::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	switch (msg)
	{
	case LIST_ITEM_SELECT:
	{
		OnSelectedSkinListItem();
		break;
	}
	case BUTTON_CLICKED:
	{
		if (pWnd == m_pClose)
			HideDialog();
		else if (pWnd == m_pSetVisual)
			SetVisual();

		break;
	}
	};
}

void CUISkinSelector::OnSelectedSkinListItem()
{
	if (!pSettings->section_exist("textures_for_skin_selector"))
		Debug.fatal(DEBUG_INFO, "Can't find section [textures_for_skin_selector]");

	CUIListBoxItem* itm = skin_list_box->GetSelectedItem();

	if (!itm) return;

	string256 skin_name = "";
	string256 pic_cache = "";

	xr_strcpy(skin_name, itm->GetText());

	if (skin_name == "")
		Debug.fatal(DEBUG_INFO, "Skin Name is empty");

	xr_strcpy(pic_cache, skin_name);
	xr_strcat(pic_cache, "_texture_pic");

	xr_string skin_texture_name = "skin_selector_textures\\pic_";
	skin_texture_name += pSettings->r_string("textures_for_skin_selector", pic_cache);
	xr_string full_name_skin_tex = skin_texture_name + ".dds";

	Frect orig_rect = skin_tex_pic->GetTextureRect();

	if (FS.exist("$game_textures$", full_name_skin_tex.c_str()))
		skin_tex_pic->InitTexture(skin_texture_name.c_str());
	else
		skin_tex_pic->InitTexture("ui\\ui_noise");

	skin_tex_pic->SetTextureRect(orig_rect);
}

void CUISkinSelector::SetVisual()
{
	if (skin_list_box->GetSize() == 0) return;

	CUIListBoxItem* itm = skin_list_box->GetSelectedItem();
	CActor* pActor = smart_cast<CActor*>(Level().CurrentViewEntity());
	CGameObject* obj = smart_cast<CGameObject*>(pActor);

	if (!itm) return;
	if (!obj) return;

	if (!pActor)
	{
		Msg("!!! ACTOR IS NULL CUISkinSelector::SetVisual()");
		return;
	}

	if (!pActor->g_Alive()) return;

	shared_str visual = nullptr;
	string256 SkinName;
	string256 SkinNameCacheForHud;

	xr_strcpy(SkinName, itm->GetText());

	xr_strcpy(SkinNameCacheForHud, SkinName);
	xr_strcat(SkinNameCacheForHud, "_hud_hands");

	xr_strcat(SkinName, ".ogf");
	visual._set(SkinName);

	R_ASSERT(visual);

	NET_Packet P;
	Game().u_EventGen(P, GE_GAME_EVENT, Game().local_player->GameID);
	P.w_u16(GAME_EVENT_CHANGE_VISUAL_FROM_SKIN_SELECTOR);
	P.w_stringZ(SkinName);
	Game().u_EventSend(P);

	if (!pSettings->section_exist("huds_for_skin_selector"))
		Debug.fatal(DEBUG_INFO, "Can't find section [huds_for_skin_selector]");

	g_player_hud->load(pSettings->r_string("huds_for_skin_selector", SkinNameCacheForHud));
}

bool CUISkinSelector::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if (keyboard_action == WINDOW_KEY_PRESSED)
	{
		if (dik == DIK_ESCAPE || is_binded(kQUIT, dik))
			HideDialog();
		else if (dik == DIK_RETURN)
			SetVisual();

		return true;
	}

	return CUIDialogWnd::OnKeyboardAction(dik, keyboard_action);
}