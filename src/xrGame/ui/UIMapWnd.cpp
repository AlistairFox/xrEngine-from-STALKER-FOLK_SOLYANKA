#include "stdafx.h"
#include "UIMapWnd.h"
#include "UIMap.h"
#include "UIXmlInit.h"

#include "../Actor.h"
#include "../map_manager.h"
#include "UIInventoryUtilities.h"
#include "../map_spot.h"
#include "../map_location.h"

#include "UIFixedScrollBar.h"
#include "UIFrameWindow.h"
#include "UIFrameLineWnd.h"
#include "UITabControl.h"
#include "UI3tButton.h"
#include "UIMapWndActions.h"
#include "UIMapWndActionsSpace.h"
#include "UIHint.h"
#include "map_hint.h"
#include "uicursor.h"

#include "../../xrEngine/xr_input.h"		//remove me !!!

#include "UIPropertiesBox.h"
#include "UIListBoxItem.h"

CUIMapWnd* g_map_wnd = NULL; // quick temporary solution -(
CUIMapWnd* GetMapWnd()
{
	return g_map_wnd;
}

CUIMapWnd::CUIMapWnd()
{
	m_tgtMap				= NULL;
	m_GlobalMap				= NULL;
	m_view_actor			= false;
	m_prev_actor_pos.set	(0,0);
	m_currentZoom			= 1.0f;
	m_map_location_hint		= NULL;
	m_map_move_step			= 10.0f;
/*
#ifdef DEBUG
//	m_dbg_text_hint			= NULL;
//	m_dbg_info				= NULL;
#endif // DEBUG /**/

//	UIMainMapHeader			= NULL;
	m_scroll_mode			= false;
	m_nav_timing			= Device.dwTimeGlobal;
	hint_wnd				= NULL;
	g_map_wnd				= this;
}

CUIMapWnd::~CUIMapWnd()
{
	delete_data( m_ActionPlanner );
	delete_data( m_GameMaps );
	delete_data( m_map_location_hint );
/*
#ifdef DEBUG
	delete_data( m_dbg_text_hint );
	delete_data( m_dbg_info );
#endif // DEBUG/**/
	g_map_wnd				= NULL;
}


void CUIMapWnd::ActivatePropertiesBox(CUIWindow* wnd)
{
	property_box->RemoveAll();

	if (!Level().game || !Level().game->local_player)
		return;
 	if (!Level().game->local_player->testFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS))
		return;

	luabind::functor<void> funct;	
	if (ai().script_engine().functor("pda.property_box_add_properties", funct))
	{
		CMapSpot* sp = smart_cast<CMapSpot*>(wnd);
		if (sp)
			funct(property_box, sp->MapLocation()->ObjectID(), (LPCSTR)sp->MapLocation()->GetLevelName().c_str(), (LPCSTR)sp->MapLocation()->GetHint());
	}

	if (property_box->GetItemsCount() > 0)
	{
		property_box->AutoUpdateSize();

		Fvector2 cursor_pos;
		Frect vis_rect;

		GetAbsoluteRect(vis_rect);
		cursor_pos = GetUICursor().GetCursorPosition();
		cursor_pos.sub(vis_rect.lt);
		property_box->Show(vis_rect, cursor_pos);
	}
}

void CUIMapWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	//	inherited::SendMessage( pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);

	if (!Level().game || !Level().game->local_player)
		return;

	if (!Level().game->local_player->testFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS))
		return;

	if (pWnd == property_box && msg == PROPERTY_CLICKED && property_box->GetClickedItem())
	{
		Msg("Property box Clicked");
		luabind::functor<void> funct;
		if (ai().script_engine().functor("pda.property_box_clicked", funct))
			funct(property_box);
	}
}


void CUIMapWnd::Init(LPCSTR xml_name, LPCSTR start_from)
{
	//Msg("MAP WND INIT");
	CUIXml uiXml;
	uiXml.Load						(CONFIG_PATH, UI_PATH, xml_name);

	string512						pth;
	CUIXmlInit						xml_init;
	strconcat						(sizeof(pth),pth,start_from,":main_wnd");
	xml_init.InitWindow				(uiXml, pth, 0, this);

	m_map_move_step					= uiXml.ReadAttribFlt( start_from, 0, "map_move_step", 10.0f );

	m_UILevelFrame					= xr_new<CUIWindow>(); m_UILevelFrame->SetAutoDelete(true);
	strconcat(sizeof(pth),pth,start_from,":level_frame");
	xml_init.InitWindow				(uiXml, pth, 0, m_UILevelFrame);
//	m_UIMainFrame->AttachChild		(m_UILevelFrame);
	AttachChild						(m_UILevelFrame);

	m_UIMainFrame					= xr_new<CUIFrameWindow>(); m_UIMainFrame->SetAutoDelete(true);
	AttachChild						(m_UIMainFrame);
	strconcat(sizeof(pth),pth,start_from,":main_map_frame");
	xml_init.InitFrameWindow		(uiXml, pth, 0, m_UIMainFrame);

	m_scroll_mode = (uiXml.ReadAttribInt(start_from, 0, "scroll_enable", 0) == 1)? true : false;
	if ( m_scroll_mode )
	{
		float dx, dy, sx, sy;
		strconcat(sizeof(pth),pth,start_from,":main_map_frame");
		dx = uiXml.ReadAttribFlt( pth, 0, "dx", 0.0f );
		dy = uiXml.ReadAttribFlt( pth, 0, "dy", 0.0f );
		sx = uiXml.ReadAttribFlt( pth, 0, "sx", 5.0f );
		sy = uiXml.ReadAttribFlt( pth, 0, "sy", 5.0f );

		CUIWindow* rect_parent			= m_UIMainFrame;//m_UILevelFrame;
		Frect r							= rect_parent->GetWndRect();

		m_UIMainScrollH					= xr_new<CUIFixedScrollBar>(); m_UIMainScrollH->SetAutoDelete(true);
		m_UIMainScrollH->InitScrollBar	(Fvector2().set(r.left+dx, r.bottom-sy), true);
		m_UIMainScrollH->SetStepSize	( _max( 1, (int)(m_UILevelFrame->GetWidth()*0.1f) ) );
		m_UIMainScrollH->SetPageSize	( (int)m_UILevelFrame->GetWidth() ); // iFloor
		AttachChild						(m_UIMainScrollH);
		Register						(m_UIMainScrollH);
		AddCallback						(m_UIMainScrollH, SCROLLBAR_HSCROLL,CUIWndCallback::void_function(this,&CUIMapWnd::OnScrollH));

		m_UIMainScrollV					= xr_new<CUIFixedScrollBar>(); m_UIMainScrollV->SetAutoDelete(true);
		m_UIMainScrollV->InitScrollBar	(Fvector2().set(r.right-sx, r.top+dy), false);
		m_UIMainScrollV->SetStepSize	( _max( 1, (int)(m_UILevelFrame->GetHeight()*0.1f) ) );
		m_UIMainScrollV->SetPageSize	( (int)m_UILevelFrame->GetHeight() );
		AttachChild						(m_UIMainScrollV);
		Register						(m_UIMainScrollV);
		AddCallback						(m_UIMainScrollV,SCROLLBAR_VSCROLL,CUIWndCallback::void_function(this,&CUIMapWnd::OnScrollV));
	}

	m_map_location_hint					= xr_new<CUIMapLocationHint>();
	strconcat							(sizeof(pth),pth,start_from,":map_hint_item");
	m_map_location_hint->Init			(uiXml, pth);
	m_map_location_hint->SetAutoDelete	(false);

// Load maps

	m_GlobalMap								= xr_new<CUIGlobalMap>(this);
	m_GlobalMap->SetAutoDelete				(true);
	m_GlobalMap->Initialize					();

	m_UILevelFrame->AttachChild				(m_GlobalMap);
	m_GlobalMap->OptimalFit					(m_UILevelFrame->GetWndRect());
	m_GlobalMap->SetMinZoom					(m_GlobalMap->GetCurrentZoom().x);
	m_currentZoom							= m_GlobalMap->GetCurrentZoom().x;
	
	init_xml_nav( uiXml );

	// initialize local maps
	xr_string sect_name;
 
	if( IsGameTypeSingle() )
		sect_name = "level_maps_single";
	else
		sect_name = "level_maps_mp";

	if (pGameIni->section_exist(sect_name.c_str()))
	{
		CInifile::Sect& S		= pGameIni->r_section(sect_name.c_str());
		CInifile::SectCIt	it	= S.Data.begin(), end = S.Data.end();
		for (;it!=end; it++)
		{
			shared_str map_name = it->first;

			Msg("Read Map sections : %s", map_name.c_str());
			if (!pGameIni->line_exist(map_name, "global_rect"))
				continue;
			xr_strlwr				(map_name);
			R_ASSERT2				(m_GameMaps.end() == m_GameMaps.find(map_name), "Duplicate level name not allowed");
			
			CUICustomMap*& l		= m_GameMaps[map_name];

			l						= xr_new<CUILevelMap>(this);
			R_ASSERT2				(pGameIni->section_exist(map_name),map_name.c_str());
			l->Initialize			(map_name, "hud\\default");

			l->OptimalFit			( m_UILevelFrame->GetWndRect() );
		}
	}

#ifdef DEBUG
	GameMaps::iterator it = m_GameMaps.begin();
	GameMaps::iterator it2;
	for(;it!=m_GameMaps.end();++it){
		CUILevelMap* l = smart_cast<CUILevelMap*>(it->second);VERIFY(l);
		for(it2=it; it2!=m_GameMaps.end();++it2){
			if(it==it2) continue;
			CUILevelMap* l2 = smart_cast<CUILevelMap*>(it2->second);VERIFY(l2);
			if(l->GlobalRect().intersected(l2->GlobalRect())){
				Msg(" --error-incorrect map definition global rect of map [%s] intersects with [%s]", *l->MapName(), *l2->MapName());
			}
		}
		if(FALSE == l->GlobalRect().intersected(GlobalMap()->BoundRect())){
			Msg(" --error-incorrect map definition map [%s] places outside global map", *l->MapName());
		}

	}
#endif

	Register				(m_GlobalMap);
	m_ActionPlanner			= xr_new<CMapActionPlanner>();
	m_ActionPlanner->setup	(this);
	m_view_actor			= true;

	property_box = xr_new<CUIPropertiesBox>();
	property_box->SetAutoDelete(true);
	property_box->InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(300, 300));
	AttachChild(property_box);
	property_box->Hide();
	property_box->SetWindowName("property_box");


}

void CUIMapWnd::Show(bool status)
{


	inherited::Show(status);

	if (Level().CurrentEntity())
		Activated();

	if ( GlobalMap() )
	{
		m_GlobalMap->DetachAll();
		m_GlobalMap->Show( false );
	}
	GameMaps::iterator	it = m_GameMaps.begin();
	for ( ; it != m_GameMaps.end(); ++it )
	{
		it->second->DetachAll();
	}

	if ( status )
	{
		m_GlobalMap->Show				(true);
		m_GlobalMap->WorkingArea().set	(ActiveMapRect());
		GameMaps::iterator	it			= m_GameMaps.begin();
		GameMaps::iterator	it_e		= m_GameMaps.end();
		for(;it!=it_e;++it)
		{
			m_GlobalMap->AttachChild		(it->second);
			it->second->Show				(true);
			it->second->WorkingArea().set	(ActiveMapRect());
		}

		if(	m_view_actor )
		{
			inherited::Update		();// only maps, not action planner
			ViewActor				();
			m_view_actor			= false;
		}
		InventoryUtilities::SendInfoToActor("ui_pda_map_local");
	}
	HideCurHint();
}

void CUIMapWnd::Activated()
{
	Fvector v					= Level().CurrentEntity()->Position();
	Fvector2 v2;
	v2.set						(v.x,v.z);
	if ( v2.distance_to( m_prev_actor_pos ) > 3.0f )
	{
		ViewActor				();
	}
}

void CUIMapWnd::AddMapToRender			(CUICustomMap* m)
{
	Register							( m );
	m_UILevelFrame->AttachChild			( m );
	m->Show								( true );
	m->WorkingArea().set				( ActiveMapRect() );
}

void CUIMapWnd::RemoveMapToRender		(CUICustomMap* m)
{
	if( m!=GlobalMap() )
		m_UILevelFrame->DetachChild			(smart_cast<CUIWindow*>(m));
}

void CUIMapWnd::SetTargetMap			(const shared_str& name, const Fvector2& pos, bool bZoomIn)
{
	u16	idx								= GetIdxByName			(name);
	if (idx!=u16(-1)){
		CUICustomMap* lm				= GetMapByIdx			(idx);
		SetTargetMap					(lm, pos, bZoomIn);
	}
}

void CUIMapWnd::SetTargetMap			(const shared_str& name, bool bZoomIn)
{
	u16	idx								= GetIdxByName			(name);
	if (idx!=u16(-1)){
		CUICustomMap* lm				= GetMapByIdx			(idx);
		SetTargetMap					(lm, bZoomIn);
	}
}

void CUIMapWnd::SetTargetMap			(CUICustomMap* m, bool bZoomIn)
{
	m_tgtMap							= m;
	Fvector2							pos;
	Frect r								= m->BoundRect();
	r.getcenter							(pos);
	SetTargetMap						(m, pos, bZoomIn);
}

void CUIMapWnd::SetTargetMap			(CUICustomMap* m, const Fvector2& pos, bool bZoomIn)
{
	m_tgtMap							= m;

	if ( m==GlobalMap() )
	{
		CUIGlobalMap* gm				= GlobalMap();
		SetZoom							(gm->GetMinZoom());
		Frect vis_rect					= ActiveMapRect		();
		vis_rect.getcenter				(m_tgtCenter);
		Fvector2						_p;
		gm->GetAbsolutePos				(_p);
		m_tgtCenter.sub					(_p);
		m_tgtCenter.div					(gm->GetCurrentZoom());
 	}
	else
	{

		if(bZoomIn/* && fsimilar(GlobalMap()->GetCurrentZoom(), GlobalMap()->GetMinZoom(),EPS_L )*/)
			SetZoom(GlobalMap()->GetMaxZoom());

//		m_tgtCenter						= m->ConvertRealToLocalNoTransform(pos, m->BoundRect());
		m_tgtCenter						= m->ConvertRealToLocal(pos, true);
		m_tgtCenter.add					(m->GetWndPos()).div(GlobalMap()->GetCurrentZoom());
	}
	ResetActionPlanner				();
}

void CUIMapWnd::MoveMap( Fvector2 const& pos_delta )
{
	GlobalMap()->MoveWndDelta		(pos_delta);
	UpdateScroll					();
	HideCurHint();
}

void CUIMapWnd::Draw()
{
	inherited::Draw();
/*
#ifdef DEBUG
	m_dbg_text_hint->Draw	();
	m_dbg_info->Draw		();
#endif // DEBUG/**/

	m_btn_nav_parent->Draw();
}

void CUIMapWnd::MapLocationRelcase(CMapLocation* ml)
{
	CUIWindow*	owner = m_map_location_hint->GetOwner();
	if (owner)
	{
		CMapSpot* ms = smart_cast<CMapSpot*>(owner);
		if(ms && ms->MapLocation()==ml) //CUITaskItem also can be a HintOwner
			m_map_location_hint->SetOwner(NULL);
	}
}

void CUIMapWnd::DrawHint()
{
	CUIWindow*	owner = m_map_location_hint->GetOwner();
	if ( owner )
	{
		CMapSpot* ms = smart_cast<CMapSpot*>(owner);
		if ( ms )
		{
			if ( ms->MapLocation() && ms->MapLocation()->HintEnabled() ) 
			{
				m_map_location_hint->Draw_();
			}
		}
		else
		{
			m_map_location_hint->Draw_();
		}
	}
}

bool CUIMapWnd::OnKeyboardHold(int dik)
{
	switch(dik)
	{
		case DIK_UP:
		case DIK_DOWN:
		case DIK_LEFT:
		case DIK_RIGHT:
			{
				Fvector2 pos_delta;
				pos_delta.set(0.0f, 0.0f);

				if(dik==DIK_UP)					pos_delta.y	+= m_map_move_step;
				if(dik==DIK_DOWN)				pos_delta.y	-= m_map_move_step;
				if(dik==DIK_LEFT)				pos_delta.x	+= m_map_move_step;
				if(dik==DIK_RIGHT)				pos_delta.x	-= m_map_move_step;
				MoveMap							(pos_delta);
				return true;
			}break;
	}
	return inherited::OnKeyboardHold(dik);
}

bool CUIMapWnd::OnKeyboardAction				(int dik, EUIMessages keyboard_action)
{
	switch(dik){
		case DIK_NUMPADMINUS:
			{
				//SetZoom(GetZoom()/1.5f);
				UpdateZoom( false );
				//ResetActionPlanner();
				return true;
			}break;
		case DIK_NUMPADPLUS:
			{
				//SetZoom(GetZoom()*1.5f);
				UpdateZoom( true );
				//ResetActionPlanner();
				return true;
			}break;
			
		/*
		case DIK_CAPSLOCK:
		{
			
			string_path					fname;
			pGameIni = 0;
			FS.update_path(fname, "$game_config$", "game.ltx");
			pGameIni = xr_new<CInifile>(fname, TRUE);


			delete_data(m_GameMaps);

			xr_string sect_name;
			if (IsGameTypeSingle())
				sect_name = "level_maps_single";
			else
				sect_name = "level_maps_mp";

			if (pGameIni->section_exist(sect_name.c_str()))
			{
				CInifile::Sect& S = pGameIni->r_section(sect_name.c_str());
				CInifile::SectCIt	it = S.Data.begin(), end = S.Data.end();
				for (; it != end; it++)
				{
 					shared_str map_name = it->first;

					if (!pGameIni->line_exist(map_name, "global_rect"))
						continue;

					xr_strlwr(map_name);

					CUICustomMap*& l = m_GameMaps[map_name];
					l = xr_new<CUILevelMap>(this);
 					l->Initialize(map_name, "hud\\default");
					l->OptimalFit(m_UILevelFrame->GetWndRect());
				}
			}
			
		}break;
		*/
	}
	
	return inherited::OnKeyboardAction	(dik, keyboard_action);
}

#include "game_cl_freemp.h"
#include "..\..\xrServerEntities\xrServer_Objects_ALife_Monsters.h"

bool CUIMapWnd::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	if (mouse_action == WINDOW_RBUTTON_UP)
	{
		 
		for (auto item : this->GetChildWndList())
		{
			CMapSpot* sp = smart_cast<CMapSpot*>(item);
				
			if (sp && sp->MapLocation())
			{
				Msg("Rect: [%d,%d] [%d, %d]", item->GetWndRect().x1, item->GetWndRect().x2, item->GetWndRect().y1, item->GetWndRect().y2);
				Msg("POS: [%d] [%d]", item->GetWndPos().x, item->GetWndPos().y);
				Frect abs;
				item->GetAbsoluteRect(abs);
				Msg("AbsRect[%d,%d][%d,%d]", abs.x1, abs.x2, abs.y1, abs.y2);
 				
				game_cl_freemp* freemp = smart_cast<game_cl_freemp*>(Level().game);

				if (freemp)
				{
					Msg("ObjID: %d", sp->MapLocation()->ObjectID());

					CSE_ALifeDynamicObject* offline = freemp->GetAlifeObject(sp->MapLocation()->ObjectID());
					CSE_ALifeOnlineOfflineGroup* online_offline_group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(offline);

					if (online_offline_group)
					{
						Msg("ObjectSpot: %d, %s", online_offline_group->ID, online_offline_group->name());
					}
				}
 
			}
		}
	}


	if ( inherited::OnMouseAction(x,y,mouse_action) /*|| m_btn_nav_parent->OnMouseAction(x,y,mouse_action)*/ )
	{
		return true;
	}

	Fvector2 cursor_pos1			= GetUICursor().GetCursorPosition();

	if(GlobalMap() && !GlobalMap()->Locked() && ActiveMapRect().in( cursor_pos1 ) )
	{
		switch ( mouse_action )
		{
		case WINDOW_RBUTTON_UP:
  			ActivatePropertiesBox(NULL);
 		break;

		case WINDOW_MOUSE_MOVE:
			if( pInput->iGetAsyncBtnState(0) )
			{
				GlobalMap()->MoveWndDelta		(GetUICursor().GetCursorPositionDelta());
				UpdateScroll					();
				HideCurHint						();
				return							true;
			}
		break;

		case WINDOW_MOUSE_WHEEL_DOWN:
			UpdateZoom( true );
			return true;
		break;
		case WINDOW_MOUSE_WHEEL_UP:
			UpdateZoom( false );
			return true;
		break;

		}//switch	
	}
 
	return false;
}

bool CUIMapWnd::UpdateZoom( bool b_zoom_in )
{
	float prev_zoom = GetZoom();
	float z = 0.0f;
	if ( b_zoom_in )
	{	
		z = GetZoom() * 1.2f;
		SetZoom( z );
	}
	else					
	{
		z = GetZoom() / 1.2f;
		SetZoom( z );
	}

	
	if ( !fsimilar( prev_zoom, GetZoom() ) )
	{
//		m_tgtCenter.set( 0, 0 );// = cursor_pos;
		Frect vis_rect					= ActiveMapRect();
		vis_rect.getcenter				(m_tgtCenter);

		Fvector2						pos;
		CUIGlobalMap* gm				= GlobalMap();
		gm->GetAbsolutePos				(pos);
		m_tgtCenter.sub					(pos);
		m_tgtCenter.div					(gm->GetCurrentZoom());
		
		ResetActionPlanner();
		HideCurHint();
		return false;
	}
	return true;
}

CUICustomMap* CUIMapWnd::GetMapByIdx(u16 idx)
{
	VERIFY							(idx!=u16(-1));
	GameMapsPairIt it				= m_GameMaps.begin();
	std::advance					(it, idx);
	return							it->second;
}

u16 CUIMapWnd::GetIdxByName(const shared_str& map_name)
{
	GameMapsPairIt it				= m_GameMaps.find(map_name);
	if(it==m_GameMaps.end()){	
		Msg							("~ Level Map '%s' not registered",map_name.c_str());
		return						u16(-1);
	}
	return (u16)std::distance		(m_GameMaps.begin(),it);
}

void CUIMapWnd::UpdateScroll()
{
	if ( m_scroll_mode )
	{
		Fvector2 w_pos					= GlobalMap()->GetWndPos();
		m_UIMainScrollV->SetRange(m_UIMainScrollV->GetMinRange(),iFloor(GlobalMap()->GetHeight()));
		m_UIMainScrollH->SetRange(m_UIMainScrollV->GetMinRange(),iFloor(GlobalMap()->GetWidth()));

		m_UIMainScrollV->SetScrollPos	(iFloor(-w_pos.y));
		m_UIMainScrollH->SetScrollPos	(iFloor(-w_pos.x));
	}

}

void CUIMapWnd::OnScrollV(CUIWindow*, void*)
{
	if ( m_scroll_mode && GlobalMap())
	{
		MoveScrollV( -1.0f * float(m_UIMainScrollV->GetScrollPos()));
	}
}

void CUIMapWnd::OnScrollH(CUIWindow*, void*)
{
	if ( m_scroll_mode && GlobalMap())
	{
		MoveScrollH( -1.0f * float(m_UIMainScrollH->GetScrollPos()) );
	}
}

void CUIMapWnd::MoveScrollV( float dy )
{
	Fvector2 w_pos				= GlobalMap()->GetWndPos();
	GlobalMap()->SetWndPos		( Fvector2().set( w_pos.x, dy ) );
}

void CUIMapWnd::MoveScrollH( float dx )
{
	Fvector2 w_pos				= GlobalMap()->GetWndPos();
	GlobalMap()->SetWndPos		( Fvector2().set( dx , w_pos.y ) );
}

void CUIMapWnd::Update()
{
	if(m_GlobalMap)
		m_GlobalMap->WorkingArea().set(ActiveMapRect());
	inherited::Update			();

	if (ai().get_game_graph())
		m_ActionPlanner->update		();


	UpdateNav					();
}

void CUIMapWnd::SetZoom(float value)
{
	m_currentZoom	= value;
	clamp			(m_currentZoom, GlobalMap()->GetMinZoom(), GlobalMap()->GetMaxZoom());
}

void CUIMapWnd::ViewGlobalMap()
{
	if (GlobalMap()->Locked())			return;
	SetTargetMap(GlobalMap());
}

void CUIMapWnd::ResetActionPlanner()
{
	m_ActionPlanner->m_storage.set_property(1,false);
	m_ActionPlanner->m_storage.set_property(2,false);
	m_ActionPlanner->m_storage.set_property(3,false);
}

void CUIMapWnd::ViewZoomIn()
{
	if (GlobalMap()->Locked())		return;
	UpdateZoom( true );
}

void CUIMapWnd::ViewZoomOut()
{
	if (GlobalMap()->Locked())		return;
	UpdateZoom( false );
}

void CUIMapWnd::ViewActor()
{
	if (GlobalMap()->Locked())			return;

	Fvector v					= Level().CurrentEntity()->Position();
	m_prev_actor_pos.set		(v.x,v.z);

	CUICustomMap* lm			= NULL;
	u16	idx						= GetIdxByName( Level().name() );
	if ( idx != u16(-1) )
	{
		lm						= GetMapByIdx( idx );
	}
	else
	{
		lm						= GlobalMap();
	}

	SetTargetMap				(lm, m_prev_actor_pos, true);
}

void CUIMapWnd::ShowHintStr(CUIWindow* parent, LPCSTR text) //map name
{
	if(m_map_location_hint->GetOwner())
		return;

	m_map_location_hint->SetInfoStr		(text);
	m_map_location_hint->SetOwner		(parent);
	ShowHint							();
}

void CUIMapWnd::ShowHintSpot( CMapSpot* spot )
{
	CUIWindow* owner = m_map_location_hint->GetOwner();
	if ( !owner )
	{
		m_map_location_hint->SetInfoMSpot( spot );
		m_map_location_hint->SetOwner( spot );
		ShowHint();
		return;
	}

	CMapSpot* prev_spot = smart_cast<CMapSpot*>( owner );
	if ( prev_spot && ( prev_spot->get_location_level() < spot->get_location_level() ) )
	{
		m_map_location_hint->SetInfoMSpot( spot );
		m_map_location_hint->SetOwner( spot );
		ShowHint();
		return;
	}
}

void CUIMapWnd::ShowHintTask( CGameTask* task, CUIWindow* owner )
{
	if ( task )
	{
		m_map_location_hint->SetInfoTask( task );
		m_map_location_hint->SetOwner( owner );
		ShowHint( true );
		return;
	}
	HideCurHint();
}

void CUIMapWnd::ShowHint( bool extra )
{
	Frect vis_rect;
	if ( extra )
	{
		vis_rect.set( Frect().set( 0.0f, 0.0f, UI_BASE_WIDTH, UI_BASE_HEIGHT ) );
	} 
	else
	{
		vis_rect = ActiveMapRect();
	}

	bool is_visible = fit_in_rect(m_map_location_hint, vis_rect );
	if ( !is_visible )
	{
		HideCurHint();
	}
}

void CUIMapWnd::HideHint(CUIWindow* parent)
{
	if(m_map_location_hint->GetOwner() == parent)
	{
		HideCurHint();
	}
}

void CUIMapWnd::HideCurHint()
{
	m_map_location_hint->SetOwner( NULL );
}

void CUIMapWnd::Hint(const shared_str& text)
{
	/*
#ifdef DEBUG
	m_dbg_text_hint->SetTextST( *text );
#endif // DEBUG/**/
}

void CUIMapWnd::Reset()
{
	inherited::Reset			();
	ResetActionPlanner			();
}

#include "../gametaskmanager.h"
#include "../actor.h"
#include "../map_spot.h"
#include "../gametask.h"

void CUIMapWnd::SpotSelected( CUIWindow* w )
{
	CMapSpot* sp	= smart_cast<CMapSpot*>( w );
	if ( !sp )
	{
		return;
	}
	
	CGameTask* t	= Level().GameTaskManager().HasGameTask( sp->MapLocation(), true );
	if ( t )
	{
		Level().GameTaskManager().SetActiveTask( t );
	}
}
