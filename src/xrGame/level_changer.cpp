////////////////////////////////////////////////////////////////////////////
//	Module 		: level_changer.cpp
//	Created 	: 10.07.2003
//  Modified 	: 10.07.2003
//	Author		: Dmitriy Iassenev
//	Description : Level change object
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "level_changer.h"
#include "hit.h"
#include "actor.h"
#include "xrserver_objects_alife.h"
#include "level.h"
#include "ai_object_location.h"
#include "ai_space.h"
#include "level_graph.h"
#include "game_level_cross_table.h"

#include "UIGameSP.h"
#include "../xrengine/xr_collide_form.h"

xr_vector<CLevelChanger*>	g_lchangers;

CLevelChanger::~CLevelChanger	()
{
}

void CLevelChanger::Center		(Fvector& C) const
{
	XFORM().transform_tiny		(C,CFORM()->getSphere().P);
}

float CLevelChanger::Radius		() const
{
	return CFORM()->getRadius	();
}

void CLevelChanger::net_Destroy	() 
{
	inherited ::net_Destroy	();
	xr_vector<CLevelChanger*>::iterator it = std::find(g_lchangers.begin(), g_lchangers.end(), this);
	if(it != g_lchangers.end())
		g_lchangers.erase(it);
}
#define DEF_INVITATION "level_changer_invitation"

BOOL CLevelChanger::net_Spawn	(CSE_Abstract* DC) 
{
	m_entrance_time				= 0;
	m_b_enabled					= true;
	m_invite_str				= DEF_INVITATION;
	CCF_Shape *l_pShape			= xr_new<CCF_Shape>(this);
	collidable.model			= l_pShape;
	
	CSE_Abstract				*l_tpAbstract = (CSE_Abstract*)(DC);
	CSE_ALifeLevelChanger		*l_tpALifeLevelChanger = smart_cast<CSE_ALifeLevelChanger*>(l_tpAbstract);
	R_ASSERT					(l_tpALifeLevelChanger);

	m_game_vertex_id			= l_tpALifeLevelChanger->m_tNextGraphID;
	m_level_vertex_id			= l_tpALifeLevelChanger->m_dwNextNodeID;
	m_position					= l_tpALifeLevelChanger->m_tNextPosition;
	m_angles					= l_tpALifeLevelChanger->m_tAngles;

	//Msg("Name[%s]", l_tpALifeLevelChanger->name_replace());
	//Msg("game_id[%d], vertex_id[%d]", m_game_vertex_id, m_level_vertex_id);
	//Msg("Pos[%.0f][%.0f][%.0f]", m_position.x, m_position.y, m_position.z);

	Fvector pos =  this->Position();
	//Msg("RealPos[%.0f][%.0f][%.0f]", pos.x, pos.y, pos.z);

	m_bSilentMode				= !!l_tpALifeLevelChanger->m_bSilentMode;
	if (ai().get_level_graph())
	{
		//. this information should be computed in xrAI
		ai_location().level_vertex	(ai().level_graph().vertex(u32(-1),Position()));
		ai_location().game_vertex	(ai().cross_table().vertex(ai_location().level_vertex_id()).game_vertex_id());
	}

	feel_touch.clear			();
	
	for (u32 i=0; i < l_tpALifeLevelChanger->shapes.size(); ++i) {
		CSE_Shape::shape_def	&S = l_tpALifeLevelChanger->shapes[i];
		switch (S.type) {
			case 0 : {
				l_pShape->add_sphere(S.data.sphere);
				break;
			}
			case 1 : {
				l_pShape->add_box(S.data.box);
				break;
			}
		}
	}

	BOOL						bOk = inherited::net_Spawn(DC);
	if (bOk) {
		l_pShape->ComputeBounds	();
		Fvector					P;
		XFORM().transform_tiny	(P,CFORM()->getSphere().P);
		setEnabled				(TRUE);
	}
	g_lchangers.push_back		(this);
	return						(bOk);
}

void CLevelChanger::shedule_Update(u32 dt)
{
	inherited::shedule_Update	(dt);

	const Fsphere				&s = CFORM()->getSphere();
	Fvector						P;
	XFORM().transform_tiny		(P,s.P);
	feel_touch_update			(P,s.R);

	update_actor_invitation		();
}
#include "patrol_path.h"
#include "patrol_path_storage.h"
void CLevelChanger::feel_touch_new	(CObject *tpObject)
{
	CActor*			actor = smart_cast<CActor*>(tpObject);
	VERIFY			(actor);

	/*

	if (!actor->g_Alive())
		return;
	 
	if (false)
	if (m_bSilentMode)
	{
		NET_Packet	p;
		p.w_begin	(M_CHANGE_LEVEL);
		p.w			(&m_game_vertex_id,sizeof(m_game_vertex_id));
		p.w			(&m_level_vertex_id,sizeof(m_level_vertex_id));
		p.w_vec3	(m_position);
		p.w_vec3	(m_angles);

		Msg("game_vertex_id %d", m_game_vertex_id);
		Msg("m_level_vertex_id %d", m_level_vertex_id);
		Msg("pos x[%.0f] y[%.0f] z[%.0f]", m_position.x, m_position.y, m_position.z);
	
		if (m_game_vertex_id != 65535)
			Level().Send(p,net_flags(TRUE));

		return;
	}

	if (false)
	{
		Fvector			p, r;
		bool			b = get_reject_pos(p, r);
		CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(CurrentGameUI());
		if (pGameSP)
			pGameSP->ChangeLevel(m_game_vertex_id, m_level_vertex_id, m_position, m_angles, p, r, b, m_invite_str, m_b_enabled);
	}

	*/

	m_entrance_time	= Device.fTimeGlobal;
}

bool CLevelChanger::get_reject_pos(Fvector& p, Fvector& r)
{
		p.set(0,0,0);
		r.set(0,0,0);
//--		db.actor:set_actor_position(patrol("t_way"):point(0))
//--		local dir = patrol("t_look"):point(0):sub(patrol("t_way"):point(0))
//--		db.actor:set_actor_direction(-dir:getH())

		if(m_ini_file && m_ini_file->section_exist("pt_move_if_reject"))
		{
			LPCSTR p_name = m_ini_file->r_string("pt_move_if_reject", "path");
			const CPatrolPath*		patrol_path = ai().patrol_paths().path(p_name);
			VERIFY					(patrol_path);
			
			const CPatrolPoint*		pt;
			pt						= &patrol_path->vertex(0)->data();
			p						= pt->position();

			Fvector tmp;
			pt						= &patrol_path->vertex(1)->data();
			tmp.sub					(pt->position(),p);
			tmp.getHP				(r.y,r.x);
			return true;
		}
		return false;
}

BOOL CLevelChanger::feel_touch_contact	(CObject *object)
{
	BOOL bRes	= (((CCF_Shape*)CFORM())->Contact(object));
	bRes		= bRes && smart_cast<CActor*>(object) && smart_cast<CActor*>(object)->g_Alive();
	return		bRes;
}

void CLevelChanger::update_actor_invitation()
{
/*
	if(m_bSilentMode)					
		return;

	xr_vector<CObject*>::iterator it		= feel_touch.begin();
	xr_vector<CObject*>::iterator it_e		= feel_touch.end();

	for(;it!=it_e;++it)
	{
		CActor*			l_tpActor = smart_cast<CActor*>(*it);
		VERIFY			(l_tpActor);
		
		if(!l_tpActor->g_Alive())
			continue;

		if(m_entrance_time+5.0f < Device.fTimeGlobal){
			CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(CurrentGameUI());
			Fvector p,r;
			bool b = get_reject_pos(p,r);
			
			if(pGameSP)
				pGameSP->ChangeLevel(m_game_vertex_id,m_level_vertex_id,m_position,m_angles,p,r,b, m_invite_str, m_b_enabled);

			m_entrance_time		= Device.fTimeGlobal;
		}
	}

*/
	
	int pl_size = Game().players.size() - 1;
	
	if (OnServer())
	if (feel_touch.size() == pl_size && pl_size != 0)
	{
		Msg("feel_size[%d]/ pl_size[%d]", feel_touch.size(), pl_size);
		NET_Packet	p;
		p.w_begin(M_CHANGE_LEVEL);
		p.w(&m_game_vertex_id, sizeof(m_game_vertex_id));
		p.w(&m_level_vertex_id, sizeof(m_level_vertex_id));
		p.w_vec3(m_position);
		p.w_vec3(m_angles);
  
		if (m_game_vertex_id != 65535)
			Level().Send(p, net_flags(TRUE));
	}
	  
}

void CLevelChanger::save(NET_Packet &output_packet)
{
	inherited::save			(output_packet);
	output_packet.w_stringZ	(m_invite_str);
	output_packet.w_u8		(m_b_enabled?1:0);
}

void CLevelChanger::load(IReader &input_packet)
{
	inherited::load			(input_packet);
	input_packet.r_stringZ	(m_invite_str);
	m_b_enabled				= !!input_packet.r_u8();
}

BOOL CLevelChanger::net_SaveRelevant()
{
	if(!m_b_enabled || m_invite_str!=DEF_INVITATION )
		return TRUE;
	else
		return inherited::net_SaveRelevant();
}
