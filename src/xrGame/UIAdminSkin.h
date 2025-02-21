#pragma once

#include "ui/UIDialogWnd.h"

class CUIStatic;
class CUIXml;
class CUIWindow;
class CUI3tButton;
class CUIListBox;

class CUISkinSelector : public CUIDialogWnd
{
private:
	typedef CUIWindow	inherited;

	CUIStatic* m_pBack;
	CUIStatic* skin_tex_pic;
	CUIStatic* skin_tex_frame;

	CUIXml* xml_doc;

	CUI3tButton* m_pClose;
	CUI3tButton* m_pSetVisual;

	CUIListBox* skin_list_box;

public:
	virtual void		SetVisual();
	virtual	void		FillSkinList();
	virtual void 		SendMessage(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	void 		OnSelectedSkinListItem();

	CUISkinSelector();
	virtual				~CUISkinSelector();
	void				Init();
	virtual bool		OnKeyboardAction(int dik, EUIMessages keyboard_action);
};