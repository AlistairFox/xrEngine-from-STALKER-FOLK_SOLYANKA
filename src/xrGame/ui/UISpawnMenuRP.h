#pragma once
#include "UIDialogWnd.h"

class CUIStatic;
class CUIStatix;
class CUI3tButton;
class CUIScrollView;
class CUIEditBox;

class CUISpawnMenuRP : public CUIDialogWnd
{
private:
	typedef CUIDialogWnd inherited;

public:
	CUISpawnMenuRP();
	virtual ~CUISpawnMenuRP();

protected:
	u8											team_buttons_count = 0;

	CUIStatic*							m_pCaption;
	CUIStatic*							m_pBackground;
	CUIScrollView*					    m_pTextDesc;

	CUIStatic*							m_pCaptionNickName;
	CUIEditBox*							m_pNickname;

	xr_vector<CUIStatix*>		m_pImages;

public:
	void										Init();
	virtual void						SendMessage(CUIWindow *pWnd, s16 msg, void *pData);
};

