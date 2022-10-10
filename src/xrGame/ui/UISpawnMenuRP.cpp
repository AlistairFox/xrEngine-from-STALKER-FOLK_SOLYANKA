#include "stdafx.h"
#include "UISpawnMenuRP.h"
#include <dinput.h>
#include "UIXmlInit.h"
#include "../level.h"
#include "../game_cl_roleplay.h"
#include "UIStatix.h"
#include "UIScrollView.h"
#include "UI3tButton.h"
#include "UIEditBox.h"

#include "../xr_level_controller.h"
#include "uicursor.h"
#include "uigamecustom.h"
#include "game_cl_roleplay.h"

CUISpawnMenuRP::CUISpawnMenuRP()
{
	game_cl_mp* game = smart_cast<game_cl_mp*>(&Game());
	team_buttons_count = game->GetTeamCount();

	m_pBackground = xr_new<CUIStatic>();
	m_pBackground->SetAutoDelete(true);
	AttachChild(m_pBackground);

	m_pCaption = xr_new<CUIStatic>();
	m_pCaption->SetAutoDelete(true);
	AttachChild(m_pCaption);

	m_pTextDesc = xr_new<CUIScrollView>();
	m_pTextDesc->SetAutoDelete(true);
//	AttachChild(m_pTextDesc);

	m_pCaptionNickName = xr_new<CUIStatic>();
	m_pCaptionNickName->SetAutoDelete(true);
	AttachChild(m_pCaptionNickName);

	m_pNickname = xr_new<CUIEditBox>();
	m_pNickname->SetAutoDelete(true);
	AttachChild(m_pNickname);


	for (u8 i = 0; i < team_buttons_count; i++)
	{
		m_pImages.push_back(xr_new<CUIStatix>());
		AttachChild(m_pImages.back());
	}
}

CUISpawnMenuRP::~CUISpawnMenuRP()
{
	for (u32 i = 0; i < m_pImages.size(); i++)
		xr_delete(m_pImages[i]);
}

void CUISpawnMenuRP::Init()
{
	CUIXml xml_doc;
	xml_doc.Load(CONFIG_PATH, UI_PATH, "spawn_roleplay.xml");

	CUIXmlInit::InitWindow(xml_doc, "team_selector", 0, this);
	CUIXmlInit::InitStatic(xml_doc, "team_selector:caption", 0, m_pCaption);
	CUIXmlInit::InitStatic(xml_doc, "team_selector:background", 0, m_pBackground);
	CUIXmlInit::InitScrollView(xml_doc, "team_selector:text_desc", 0, m_pTextDesc);

	CUIXmlInit::InitStatic(xml_doc, "team_selector:caption_nickname", 0, m_pCaptionNickName);
	CUIXmlInit::InitEditBox(xml_doc, "team_selector:nickname", 0, m_pNickname);
	
	string32 node;
	for (u8 i = 1; i <= team_buttons_count; i++)
	{
		xr_sprintf(node, "team_selector:image_%d", i);
		CUIXmlInit::InitStatic(xml_doc, node, 0, m_pImages[i - 1]);
	}
}

void CUISpawnMenuRP::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if (BUTTON_CLICKED == msg)
	{
		LPCSTR nick = m_pNickname->GetText();
		
		if (xr_strlen(nick) == 0)
 			return;

		game_cl_roleplay * game = smart_cast<game_cl_roleplay*>(&Game());
		R_ASSERT(game);
		u32 team;
		
		for (u8 i = 0; i < team_buttons_count; i++)
		{
			if (pWnd == m_pImages[i])
			{
				game->OnTeamSelect(i + 1);
				team = i + 1;
			}
		}

		NET_Packet				P;
		Game().u_EventGen(P, GE_GAME_EVENT, Game().local_player->GameID);
		P.w_u16(GAME_EVENT_PLAYER_NAME_ACCAUNT);
		P.w_stringZ(nick);
		P.w_u32(team);
		Game().u_EventSend(P);

		HideDialog(); 
	}
}