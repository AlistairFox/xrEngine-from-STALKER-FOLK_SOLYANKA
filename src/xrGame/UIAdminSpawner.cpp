#include "stdafx.h"
#include "UIAdminSpawner.h"
#include "ui/UIXmlInit.h"
#include "object_broker.h"
#include "ui/UIStatic.h"
#include "ui/UI3tButton.h"
#include "ui/UIListBox.h"
#include "ui/UIListBoxItem.h"
#include "ui/UITabControl.h"
#include "ui/UIScrollView.h"
#include "ui/UIHelper.h"
#include "Level.h"
#include "Actor.h"
#include "../xrEngine/xr_ioconsole.h"
#include <dinput.h>
#include "string_table.h"
#include "ui/UIInventoryUtilities.h"

CUIAdminSpawner::CUIAdminSpawner()
{
	xml_doc = nullptr;

	m_sActiveSection = "";
	active_tab_dialog = "";

	m_pBack = xr_new<CUIStatic>();
	m_pBack->SetAutoDelete(true);
	AttachChild(m_pBack);

	m_pBackForDescr = xr_new<CUIStatic>();
	m_pBackForDescr->SetAutoDelete(true);
	AttachChild(m_pBackForDescr);

	m_pTabControl = xr_new<CUITabControl>();
	m_pTabControl->SetAutoDelete(true);
	AttachChild(m_pTabControl);

	item_icon = nullptr;
	itm_name = nullptr;
	itm_desc = nullptr;
	itm_weight = nullptr;
	itm_cost = nullptr;

	items_list_box = xr_new<CUIListBox>();
	items_list_box->SetAutoDelete(true);
	AttachChild(items_list_box);

	scroll_v = xr_new<CUIScrollView>();
	scroll_v->SetAutoDelete(true);
	AttachChild(scroll_v);

	m_pClose = xr_new<CUI3tButton>();
	m_pClose->SetAutoDelete(true);
	AttachChild(m_pClose);

	m_pSpawnItem = xr_new<CUI3tButton>();
	m_pSpawnItem->SetAutoDelete(true);
	AttachChild(m_pSpawnItem);

	Init();
}

CUIAdminSpawner::~CUIAdminSpawner()
{
	xr_delete(xml_doc);
}

void CUIAdminSpawner::FillSpawnerList()
{
	if (!pSettings->section_exist(active_tab_dialog))	return;

	//Drive: Очищаем
	items_list_box->Clear();

	if (active_tab_dialog == "")
		return;

	CInifile::Sect& sect = pSettings->r_section(active_tab_dialog);
	CInifile::SectCIt it_ = sect.Data.begin();
	CInifile::SectCIt it_e_ = sect.Data.end();

	for (; it_ != it_e_; ++it_)
	{
		string512 tmp_string;
		xr_sprintf(tmp_string, "%s", it_->first.c_str());
		AddItemsToSpawnerList(it_->first);
	}
}

void CUIAdminSpawner::AddItemsToSpawnerList(shared_str section)
{
	xr_string name_translate = CStringTable().translate(pSettings->r_string(section, "inv_name")).c_str();

	//Drive: если длина строки больше 90 символов, урезаем до 87 и добавляем в конце троеточие
	if (name_translate.size() > 90)
		name_translate.substr(0, 87) + "...";

	CUIListBoxItem* itm = items_list_box->AddTextItem(name_translate.c_str());

	translate_and_sect[section] = name_translate.c_str();
	for (auto& pair : translate_and_sect)
	{
		pair.second = CStringTable().translate(pSettings->r_string(pair.first, "inv_name")).c_str();
	}
}

void CUIAdminSpawner::SetActiveSubdialogSpawner(const shared_str& section)
{
	if (m_sActiveSection == section)
		return;

	if (section == "weapons")
		active_tab_dialog = "weapon_sect_for_spawner";
	else if (section == "ammos")
		active_tab_dialog = "ammo_sect_for_spawner";
	else if (section == "addons")
		active_tab_dialog = "addon_sect_for_spawner";
	else if (section == "outfits")
		active_tab_dialog = "outfit_sect_for_spawner";
	else if (section == "artefacts")
		active_tab_dialog = "artefact_sect_for_spawner";
	else if (section == "items")
		active_tab_dialog = "items_sect_for_spawner";
	else if (section == "quests")
		active_tab_dialog = "quest_sect_for_spawner";
	else if (section == "uniques")
		active_tab_dialog = "unique_sect_for_spawner";

	m_sActiveSection = section;

	FillSpawnerList();
}

void CUIAdminSpawner::Init()
{
	if (!xml_doc)
		xml_doc = xr_new<CUIXml>();

	xml_doc->Load(CONFIG_PATH, UI_PATH, "ui_mp_adm_spawner.xml");

	CUIXmlInit::InitWindow(*xml_doc, "ui_mp_adm_spawner", 0, this);
	CUIXmlInit::InitStatic(*xml_doc, "ui_mp_adm_spawner:background", 0, m_pBack);
	CUIXmlInit::InitStatic(*xml_doc, "ui_mp_adm_spawner:background_description", 0, m_pBackForDescr);
	CUIXmlInit::InitTabControl(*xml_doc, "ui_mp_adm_spawner:tab_control", 0, m_pTabControl);
	CUIXmlInit::InitScrollView(*xml_doc, "ui_mp_adm_spawner:scroll_v", 0, scroll_v);

	m_pTabControl->SetActiveTab("weapons");
	active_tab_dialog = "weapon_sect_for_spawner";

	CUIXmlInit::InitListBox(*xml_doc, "ui_mp_adm_spawner:skin_list", 0, items_list_box);
	FillSpawnerList();

	CUIXmlInit::Init3tButton(*xml_doc, "ui_mp_adm_spawner:spawn_item_button", 0, m_pSpawnItem);
	CUIXmlInit::Init3tButton(*xml_doc, "ui_mp_adm_spawner:close_button", 0, m_pClose);
}

void CUIAdminSpawner::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	switch (msg)
	{
	case TAB_CHANGED:
	{
		if (pWnd == m_pTabControl)
		{
			SetActiveSubdialogSpawner(m_pTabControl->GetActiveId());
		}
		break;
	}
	case LIST_ITEM_SELECT:
	{
		DrawSelectedItem();
		break;
	}
	case BUTTON_CLICKED:
	{
		if (pWnd == m_pClose)
			HideDialog();
		else if (pWnd == m_pSpawnItem)
			SpawnItem();
		break;
	}
	};
}

void CUIAdminSpawner::SpawnItem()
{
	if (items_list_box->GetSize() == 0) return;

	CUIListBoxItem* itm = items_list_box->GetSelectedItem();

	if (!itm)
		return;

	string128 section = "";
	xr_strcpy(section, itm->GetText());

	shared_str true_sect = nullptr;

	for (auto& pair : translate_and_sect)
	{
		if (0 == xr_strcmp(pair.second, section))
		{
			true_sect = pair.first;
		}
	}

	if (!pSettings->section_exist(true_sect))
		Debug.fatal(DEBUG_INFO, "SPAWNER: section doesnt exists");

	//Drive: Решение проблемы сохранки, т.к через Console->Execute нужен не только флаг админа, но и переменная в xrServer.
	//Поэтому будем спавнить предмет через сервер.
	NET_Packet P;
	Game().u_EventGen(P, GE_GAME_EVENT, Game().local_player->GameID);
	P.w_u16(GAME_EVENT_SPAWNER_SPAWN_ITEM);
	P.w_stringZ(true_sect);
	Game().u_EventSend(P);
}

void CUIAdminSpawner::DrawSelectedItem()
{
	CUIListBoxItem* itm = items_list_box->GetSelectedItem();

	if (!itm) return;

	if (itm_name == nullptr)
	{
		itm_name = xr_new<CUIStatic>();
		itm_name->SetAutoDelete(true);
		scroll_v->AddWindow(itm_name, true);

		CUIXmlInit::InitStatic(*xml_doc, "ui_mp_adm_spawner:itm_name", 0, itm_name);
	}

	if (itm_weight == nullptr)
	{
		itm_weight = xr_new<CUIStatic>();
		itm_weight->SetAutoDelete(true);
		scroll_v->AddWindow(itm_weight, true);

		CUIXmlInit::InitStatic(*xml_doc, "ui_mp_adm_spawner:itm_weight", 0, itm_weight);
	}

	if (itm_cost == nullptr)
	{
		itm_cost = xr_new<CUIStatic>();
		itm_cost->SetAutoDelete(true);
		scroll_v->AddWindow(itm_cost, true);

		CUIXmlInit::InitStatic(*xml_doc, "ui_mp_adm_spawner:itm_cost", 0, itm_cost);
	}

	if (itm_desc == nullptr)
	{
		itm_desc = xr_new<CUITextWnd>();
		itm_desc->SetAutoDelete(true);
		scroll_v->AddWindow(itm_desc, true);

		CUIXmlInit::InitTextWnd(*xml_doc, "ui_mp_adm_spawner:itm_desc", 0, itm_desc);
	}

	string256 section, name = "";
	LPCSTR desc = "";
	float weight;
	u32 cost;
	shared_str true_sect = nullptr;

	xr_strcpy(section, itm->GetText());

	for (auto& pair : translate_and_sect)
	{
		if (0 == xr_strcmp(pair.second, section))
		{
			true_sect = pair.first;
		}
	}

	if (!true_sect.size())
		Debug.fatal(DEBUG_INFO, "section in spawner is empty");

	xr_strcpy(name, READ_IF_EXISTS(pSettings, r_string, true_sect, "inv_name", "<No Name>"));
	weight = READ_IF_EXISTS(pSettings, r_float, true_sect, "inv_weight", 0);
	cost = READ_IF_EXISTS(pSettings, r_u32, true_sect, "cost", 0);
	desc = READ_IF_EXISTS(pSettings, r_string, true_sect, "description", "<No description>");

	string256 text_name = "";
	string4096 text_descr = "";
	string128 text_weight, text_cost = "";

	xr_sprintf(text_descr, "Описание: %s", *CStringTable().translate(desc));
	xr_sprintf(text_weight, "Вес: %.2f кг.", weight);
	xr_sprintf(text_cost, "Цена: %d руб.", cost);

	xr_sprintf(text_name, "Секция: %s", true_sect.c_str());

	itm_name->TextItemControl()->SetText(text_name);
	itm_name->TextItemControl()->SetTextColor(color_rgba(255, 255, 255, 255)); //White color

	itm_weight->TextItemControl()->SetText(text_weight);
	itm_weight->TextItemControl()->SetTextColor(color_rgba(255, 255, 255, 255)); //White color

	itm_cost->TextItemControl()->SetText(text_cost);
	itm_cost->TextItemControl()->SetTextColor(color_rgba(255, 255, 255, 255)); //White color

	//Item description interface 
	itm_desc->SetWidth(scroll_v->GetDesiredChildWidth());
	itm_desc->SetTextComplexMode(true);
	itm_desc->SetText(text_descr);
	itm_desc->AdjustHeightToText();
	//Item description interface

	scroll_v->ScrollToBegin();

	Frect texture_rect;
	texture_rect.x1 = pSettings->r_float(true_sect, "inv_grid_x") * INV_GRID_WIDTH;
	texture_rect.y1 = pSettings->r_float(true_sect, "inv_grid_y") * INV_GRID_HEIGHT;
	texture_rect.x2 = pSettings->r_float(true_sect, "inv_grid_width") * INV_GRID_WIDTH;
	texture_rect.y2 = pSettings->r_float(true_sect, "inv_grid_height") * INV_GRID_HEIGHT;

	float pw = (texture_rect.x2 / 5) * 4;
	float ph = (texture_rect.y2 / 5) * 4;;
	float px = xml_doc->ReadAttribFlt("ui_mp_adm_spawner:itm_icon", 0, "x") - (pw / 2);
	float py = xml_doc->ReadAttribFlt("ui_mp_adm_spawner:itm_icon", 0, "y") - (ph / 2);

	if (item_icon == nullptr)
	{
		item_icon = xr_new<CUIStatic>();
		item_icon->SetWindowName("pict");
		item_icon->SetAutoDelete(true);
		AttachChild(item_icon);
	}

	item_icon->InitTexture("ui\\ui_icon_equipment");
	item_icon->SetTextureRect(Frect().set(texture_rect.x1, texture_rect.y1, texture_rect.x2 + texture_rect.x1, texture_rect.y2 + texture_rect.y1));
	item_icon->SetWndRect(Frect().set(px, py, pw + px, ph + py));
	item_icon->SetStretchTexture(true);
}

bool CUIAdminSpawner::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if (keyboard_action == WINDOW_KEY_PRESSED)
	{
		if (dik == DIK_ESCAPE || is_binded(kQUIT, dik))
			HideDialog();
		else if (dik == DIK_RETURN)
			SpawnItem();

		return true;
	}

	return CUIDialogWnd::OnKeyboardAction(dik, keyboard_action);
}