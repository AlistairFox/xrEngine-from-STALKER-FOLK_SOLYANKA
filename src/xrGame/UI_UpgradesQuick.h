#pragma once
#include "ui\UIDialogWnd.h"
#include "Weapon.h"

class CUIDialogWnd;


class CWeapon;
class CUIScrollView;
class CUITextWnd;
class CUIStatic;
class CUIXml;

typedef xr_map<int, CInventoryItem*> map_avail;

class UI_UpgradesQuickDialog : public CUIDialogWnd
{
	CUIXml* uiXML;
	CWeapon* wpn;
	map_avail available;
	CUIScrollView* list;
	CUITextWnd* text[8];
	CUIStatic* background;

	virtual bool	OnKeyboardAction(int dik, EUIMessages keyboard_action);

	virtual bool	OnMouseAction(float x, float y, EUIMessages mouse_action);


	int old_attached = -1;
public:
	void			Init(CWeapon* wpn, LPCSTR section_list, LPCSTR section_text, map_avail* items);
	void			UpdateData();

	int attached_id = -1;
	virtual void 	Show(bool status);

};


class CUI_UpgradesQuick
{
	map_avail items_silencers;
	map_avail items_scopes;
	map_avail items_attach;
 
	UI_UpgradesQuickDialog d_silencers;
	UI_UpgradesQuickDialog d_attach;
	UI_UpgradesQuickDialog d_scopes;

	int old_active_item = 0;

public:
	

	void Init();
	void Update();
	void Show(bool value);

};