#include "stdafx.h"
#include "UI_UpgradesQuick.h"
 
#include "ui/UIScrollView.h"
#include "ui/UIHelper.h"
#include "ui/UIStatic.h"
#include "ui/UIXmlInit.h"

#include "Actor.h"
#include "Inventory.h"


#define ui_file "ui_upgradesquick.xml"

void CUI_UpgradesQuick::Init()
{
    CWeapon* wpn = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(Actor()->inventory().GetActiveSlot()));

    if (!wpn)
        return;

    TIItemContainer items ;
    Actor()->inventory().AddAvailableItems(items, false);
   
    int id = 0;
    for (auto scope : wpn->m_scopes)
    {
        if (strstr(wpn->m_scopes[wpn->m_cur_scope].c_str(), scope.c_str()))
            d_scopes.attached_id = id;

        if (!strstr(wpn->m_scopes[wpn->m_cur_scope].c_str(), scope.c_str()))
        for (auto item : items)
        {  
            if ( strstr(item->m_name.c_str(), scope.c_str()) )
            {
                items_scopes[id] = item;
               
                break;
            }
        }

        id++;
    }

    d_scopes.Init(wpn, "scopes", "scope_list", &items_scopes);
   // d_scopes.Show(false);
}

void CUI_UpgradesQuick::Update()
{
    CInventoryItem* item = Actor()->inventory().ItemFromSlot(Actor()->inventory().GetActiveSlot());

    if (item && old_active_item != item->object_id())
        Init();
}

void CUI_UpgradesQuick::Show(bool value)
{
    d_scopes.Show(value);
}

void UI_UpgradesQuickDialog::Init(CWeapon* w, LPCSTR section_list, LPCSTR section_text, map_avail* vec)
{
    wpn = w;
    uiXML->Load(CONFIG_PATH, UI_PATH, ui_file);

    background = UIHelper::CreateStatic(*uiXML, "background_static", this);
    UIHelper::CreateTextWnd(*uiXML, "caption", background);

    list = xr_new<CUIScrollView>();
    list->SetAutoDelete(true);
    CUIXmlInit::InitScrollView(*uiXML, section_list, 0, list);
    background->AttachChild(list);
 
    for (auto item : *vec)
    { 
        text[item.first] = UIHelper::CreateTextWnd(*uiXML, "item_name", 0);
        text[item.first]->SetTextST(item.second->NameItem());
        list->AddWindow(text[item.first], true);
    }
}

void UI_UpgradesQuickDialog::UpdateData()
{
    if (attached_id != old_attached && uiXML)
    {
        text[old_attached] = UIHelper::CreateTextWnd(*uiXML, "item_name", 0);
        text[old_attached]->SetTextST(wpn->m_scopes[attached_id].c_str());

        old_attached = attached_id;

        string256 name;
        xr_strcpy(name, wpn->m_scopes[attached_id].c_str());
        xr_strcat(name, " (attached)");

        text[attached_id] = UIHelper::CreateTextWnd(*uiXML, "item_name", 0);
        text[attached_id]->SetTextST(name);
    }


}

void UI_UpgradesQuickDialog::Show(bool status)
{
}
  
bool UI_UpgradesQuickDialog::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
    return false;
}

bool UI_UpgradesQuickDialog::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
    return false;
}
