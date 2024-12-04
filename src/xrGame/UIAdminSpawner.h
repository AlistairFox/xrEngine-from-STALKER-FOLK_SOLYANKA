#pragma once

#include "ui/UIDialogWnd.h"

class CUIStatic;
class CUITabControl;
class CUIXml;
class CUIWindow;
class CUI3tButton;
class CUIListBox;
class CUIScrollView;
class CUITextWnd;

class CUIAdminSpawner : public CUIDialogWnd
{
private:
	typedef CUIWindow	inherited;

	CUIXml* xml_doc;
	CUIStatic* m_pBack;
	CUIStatic* m_pBackForDescr;
	CUITabControl* m_pTabControl;
	CUIListBox* items_list_box;
	CUIStatic* item_icon;
	CUIScrollView* scroll_v;
	CUIStatic* itm_name; //Drive: this is section
	CUITextWnd* itm_desc;
	CUIStatic* itm_weight;
	CUIStatic* itm_cost;

	CUI3tButton* m_pClose;
	CUI3tButton* m_pSpawnItem;

	shared_str m_sActiveSection;

	LPCSTR active_tab_dialog;

	xr_map<shared_str, LPCSTR> translate_and_sect;
public:
	virtual	void		FillSpawnerList();
	virtual void 		SendMessage(CUIWindow* pWnd, s16 msg, void* pData = NULL);
	void		SetActiveSubdialogSpawner(const shared_str& section);

	void		DrawSelectedItem();
	void		SpawnItem();
	void        AddItemsToSpawnerList(shared_str section);
	CUIAdminSpawner();
	virtual				~CUIAdminSpawner();
	void				Init();
	virtual bool		OnKeyboardAction(int dik, EUIMessages keyboard_action);
};