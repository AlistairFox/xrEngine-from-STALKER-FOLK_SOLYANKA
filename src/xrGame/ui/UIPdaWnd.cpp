#include "stdafx.h"
#include "UIPdaWnd.h"
#include "../Pda.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIInventoryUtilities.h"

#include "../level.h"
#include "UIGameCustom.h"

#include "UIStatic.h"
#include "UIFrameWindow.h"
#include "UITabControl.h"
#include "UIMapWnd.h"
#include "UIFrameLineWnd.h"
#include "object_broker.h"
#include "UIMessagesWindow.h"
#include "UIMainIngameWnd.h"
#include "UITabButton.h"
#include "UIAnimatedStatic.h"

#include "UIHelper.h"
#include "UIHint.h"
#include "UIBtnHint.h"
#include "UITaskWnd.h"
#include "UIRankingWnd.h"
#include "UILogsWnd.h"
// #include "UIPda_Contacts.h"
#include "UIPda_Chat.h"

#include "UIPdaContactsWnd.h"


#define PDA_XML		"pda.xml"

u32 g_pda_info_state = 0;

void RearrangeTabButtons(CUITabControl* pTab);

CUIPdaWnd::CUIPdaWnd()
{
	pUITaskWnd       = NULL;
//-	pUIFactionWarWnd = NULL;
	pUIRankingWnd    = NULL;
	pUILogsWnd       = NULL;
//	pUIContacts		 = NULL;
	pUIChatWnd		 = NULL;

	m_hint_wnd       = NULL;
	Init();
}

CUIPdaWnd::~CUIPdaWnd()
{
	delete_data( pUITaskWnd );
//-	delete_data( pUIFactionWarWnd );
	delete_data( pUIRankingWnd );
	delete_data( pUILogsWnd );
//	delete_data( pUIContacts );
	delete_data(pUIChatWnd);

	delete_data( m_hint_wnd );
	delete_data( UINoice );
}

void CUIPdaWnd::Init()
{
	CUIXml					uiXml;
	uiXml.Load				(CONFIG_PATH, UI_PATH, PDA_XML);

	m_pActiveDialog			= NULL;
	m_sActiveSection		= "";

	CUIXmlInit::InitWindow	(uiXml, "main", 0, this);

	UIMainPdaFrame			= UIHelper::CreateStatic	( uiXml, "background_static", this );
	m_caption				= UIHelper::CreateTextWnd	( uiXml, "caption_static", this );
	m_caption_const			= ( m_caption->GetText() );
	m_clock					= UIHelper::CreateTextWnd	( uiXml, "clock_wnd", this );

/*
	m_anim_static			= xr_new<CUIAnimatedStatic>();
	AttachChild				(m_anim_static);
	m_anim_static->SetAutoDelete(true);
	CUIXmlInit::InitAnimatedStatic(uiXml, "anim_static", 0, m_anim_static);
*/

	m_btn_close				= UIHelper::Create3tButton( uiXml, "close_button", this );
	m_hint_wnd				= UIHelper::CreateHint( uiXml, "hint_wnd" );


	pUITaskWnd					= xr_new<CUITaskWnd>();
	pUITaskWnd->hint_wnd		= m_hint_wnd;
	pUITaskWnd->Init			();

	// pUIFactionWarWnd				= xr_new<CUIFactionWarWnd>();
	// pUIFactionWarWnd->hint_wnd		= m_hint_wnd;
	// pUIFactionWarWnd->Init			();

	pUIRankingWnd					= xr_new<CUIRankingWnd>();
	pUIRankingWnd->Init				();

	pUILogsWnd						= xr_new<CUILogsWnd>();
	pUILogsWnd->Init				();

	//MP 
	//pUIContacts = xr_new<CUIPda_Contacts>();
	//pUIContacts->Init();

	pUIContactsWnd = xr_new<CUIPdaContactsWnd>();
	pUIContactsWnd->Init();

	pUIChatWnd = xr_new<UIPdaChat>();
	pUIChatWnd->Init();

	UITabControl					= xr_new<CUITabControl>();
	UITabControl->SetAutoDelete		(true);
	AttachChild						(UITabControl);
	CUIXmlInit::InitTabControl		(uiXml, "tab", 0, UITabControl);
	UITabControl->SetMessageTarget	(this);

	UINoice					= xr_new<CUIStatic>();
	UINoice->SetAutoDelete	( true );
	CUIXmlInit::InitStatic	( uiXml, "noice_static", 0, UINoice );

//	RearrangeTabButtons		(UITabControl);
}

void CUIPdaWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	switch ( msg )
	{
	case TAB_CHANGED:
		{
			if ( pWnd == UITabControl )
			{
				SetActiveSubdialog			(UITabControl->GetActiveId());
			}
			break;
		}
	case BUTTON_CLICKED:
		{
			if ( pWnd == m_btn_close )
			{
				HideDialog();
			}
			break;
		}
	default:
		{
			R_ASSERT						(m_pActiveDialog);
			m_pActiveDialog->SendMessage	(pWnd, msg, pData);
		}
	};
}

void CUIPdaWnd::Show(bool status)
{
	inherited::Show						(status);
	if(status)
	{
		InventoryUtilities::SendInfoToActor	("ui_pda");
		
		if ( !m_pActiveDialog )
		{
			SetActiveSubdialog				("eptTasks");
		}
		m_pActiveDialog->Show				(true);
	}else
	{
		InventoryUtilities::SendInfoToActor	("ui_pda_hide");
		CurrentGameUI()->UIMainIngameWnd->SetFlashIconState_(CUIMainIngameWnd::efiPdaTask, false);
		m_pActiveDialog->Show				(false);
		g_btnHint->Discard					();
		g_statHint->Discard					();
	}
}

void CUIPdaWnd::Update()
{
	inherited::Update();
	m_pActiveDialog->Update();
	m_clock->TextItemControl().SetText(InventoryUtilities::GetGameTimeAsString(InventoryUtilities::etpTimeToMinutes).c_str());

	Device.seqParallel.push_back	(fastdelegate::FastDelegate0<>(pUILogsWnd,&CUILogsWnd::PerformWork));
}

void CUIPdaWnd::SetActiveSubdialog(const shared_str& section)
{
	if ( m_sActiveSection == section ) return;

	if ( m_pActiveDialog )
	{
		UIMainPdaFrame->DetachChild( m_pActiveDialog );
		m_pActiveDialog->Show( false );
	}

	if ( section == "eptTasks" )
	{
		m_pActiveDialog = pUITaskWnd;
	}
//-	else if ( section == "eptFractionWar" )
//-	{
//-		m_pActiveDialog = pUIFactionWarWnd;
//-	}
	else if ( section == "eptRanking" )
	{
		m_pActiveDialog = pUIRankingWnd;
	}
	else if ( section == "eptLogs" )
	{
		m_pActiveDialog = pUILogsWnd;
	} 
	else if (section == "eptContacts")
	{
		m_pActiveDialog = pUIContactsWnd;
	}
	else if (section == "eptChat")
	{
		m_pActiveDialog = pUIChatWnd;
	}

	R_ASSERT2						(m_pActiveDialog, "active dialog is not initialized");
	UIMainPdaFrame->AttachChild		(m_pActiveDialog);
	m_pActiveDialog->Show			(true);

	if ( UITabControl->GetActiveId() != section )
	{
		UITabControl->SetActiveTab( section );
	}
	m_sActiveSection = section;
	SetActiveCaption();
}

void CUIPdaWnd::SetActiveCaption()
{
	TABS_VECTOR*	btn_vec		= UITabControl->GetButtonsVector();
	TABS_VECTOR::iterator it_b	= btn_vec->begin();
	TABS_VECTOR::iterator it_e	= btn_vec->end();
	for ( ; it_b != it_e; ++it_b )
	{
		if ( (*it_b)->m_btn_id == m_sActiveSection )
		{
			LPCSTR cur = (*it_b)->TextItemControl()->GetText();
			string256 buf;
			strconcat( sizeof(buf), buf, m_caption_const.c_str(), cur );
			SetCaption( buf );
			return;
		}
	}
}

void CUIPdaWnd::Show_SecondTaskWnd( bool status )
{
	if ( status )
	{
		SetActiveSubdialog( "eptTasks" );
	}
	pUITaskWnd->Show_TaskListWnd( status );
}

void CUIPdaWnd::Show_MapLegendWnd( bool status )
{
	if ( status )
	{
		SetActiveSubdialog( "eptTasks" );
	}
	pUITaskWnd->ShowMapLegend( status );
}

void CUIPdaWnd::Draw()
{
	inherited::Draw();
//.	DrawUpdatedSections();
	DrawHint();
	UINoice->Draw(); // over all
}

void CUIPdaWnd::DrawHint()
{
	if ( m_pActiveDialog == pUITaskWnd )
	{
		pUITaskWnd->DrawHint();
	}
	else if ( m_pActiveDialog == pUIRankingWnd )
	{
		pUIRankingWnd->DrawHint();
	}
	else if ( m_pActiveDialog == pUILogsWnd )
	{

	}
	m_hint_wnd->Draw();
}

void CUIPdaWnd::UpdatePda()
{
	pUILogsWnd->UpdateNews();

	if ( m_pActiveDialog == pUITaskWnd )
	{
		pUITaskWnd->ReloadTaskInfo();
	}
}

void CUIPdaWnd::UpdateRankingWnd()
{
	pUIRankingWnd->Update();
}

void CUIPdaWnd::Reset()
{
	inherited::ResetAll		();

	if ( pUITaskWnd )		pUITaskWnd->ResetAll();
//-	if ( pUIFactionWarWnd )	pUITaskWnd->ResetAll();
	if ( pUIRankingWnd )	pUIRankingWnd->ResetAll();
	if ( pUILogsWnd )		pUILogsWnd->ResetAll();
	if (pUIContactsWnd)		pUIContactsWnd->ResetAll();
	if (pUIChatWnd)			pUIChatWnd->ResetAll();
}

void CUIPdaWnd::SetCaption( LPCSTR text )
{
	m_caption->SetText( text );
}

void RearrangeTabButtons(CUITabControl* pTab)
{
	TABS_VECTOR *	btn_vec		= pTab->GetButtonsVector();
	TABS_VECTOR::iterator it	= btn_vec->begin();
	TABS_VECTOR::iterator it_e	= btn_vec->end();

	Fvector2					pos;
	pos.set						((*it)->GetWndPos());
	float						size_x;

	for ( ; it != it_e; ++it )
	{
		(*it)->SetWndPos		(pos);
		(*it)->AdjustWidthToText();
		size_x					= (*it)->GetWndSize().x + 30.0f;
		(*it)->SetWidth			(size_x);
		pos.x					+= size_x - 6.0f;
	}
	
	pTab->SetWidth( pos.x + 5.0f );
	pos.x = pTab->GetWndPos().x - pos.x;
	pos.y = pTab->GetWndPos().y;
	pTab->SetWndPos( pos );
}

bool CUIPdaWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
 	if ( is_binded(kACTIVE_JOBS, dik) )
	{
		if ( WINDOW_KEY_PRESSED == keyboard_action )
			HideDialog();

		return true;
	}	

	return inherited::OnKeyboardAction(dik,keyboard_action);
}
