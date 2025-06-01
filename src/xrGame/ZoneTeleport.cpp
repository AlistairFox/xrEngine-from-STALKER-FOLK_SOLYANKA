////////////////////////////////////////////////////////////////////////////
//	Module 		: level_changer.cpp
//	Created 	: 10.07.2003
//  Modified 	: 10.07.2003
//	Author		: Dmitriy Iassenev
//	Description : Level change object
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZoneTeleport.h"

#include "hit.h"
#include "actor.h"
#include "xrserver_objects_alife.h"
#include "Level.h"
#include "../xrengine/xr_collide_form.h"
 
CZoneTeleport::CZoneTeleport()
{
}

CZoneTeleport::~CZoneTeleport()
{
}

void CZoneTeleport::Center(Fvector& C) const
{
	XFORM().transform_tiny(C, CFORM()->getSphere().P);
}

float CZoneTeleport::Radius() const
{
	return CFORM()->getRadius();
}

void CZoneTeleport::net_Destroy()
{
	inherited::net_Destroy();
}

BOOL CZoneTeleport::net_Spawn(CSE_Abstract* DC)
{
 	CCF_Shape* l_pShape = xr_new<CCF_Shape>(this);
	collidable.model = l_pShape;

	CSE_Abstract* l_tpAbstract = (CSE_Abstract*)(DC);
	CSE_ALifeLevelChanger* l_tpALifeLevelChanger = smart_cast<CSE_ALifeLevelChanger*>(l_tpAbstract);
	feel_touch.clear();

	for (u32 i = 0; i < l_tpALifeLevelChanger->shapes.size(); ++i) 
	{
		CSE_Shape::shape_def& S = l_tpALifeLevelChanger->shapes[i];
		switch (S.type) 
		{
			case 0: 
			{
				l_pShape->add_sphere(S.data.sphere);
				break;
			}
			case 1: 
			{
				l_pShape->add_box(S.data.box);
				break;
			}
		}
	}

	BOOL bOk = inherited::net_Spawn(DC);
	if (bOk) 
	{
		l_pShape->ComputeBounds();
		Fvector					P;
		XFORM().transform_tiny(P, CFORM()->getSphere().P);
		setEnabled(TRUE);
	}
	 
	return						(bOk);
}

void CZoneTeleport::shedule_Update(u32 dt)
{
	inherited::shedule_Update(dt);

	const Fsphere& s = CFORM()->getSphere();
	Fvector						P;
	XFORM().transform_tiny(P, s.P);
	feel_touch_update(P, s.R);
	// update_actor_invitation();
}

void CZoneTeleport::feel_touch_new(CObject* tpObject)
{
	CActor* actor = smart_cast<CActor*>(tpObject);
	VERIFY(actor);

	if (actor && OnServer())
	{

	}
}
  
BOOL CZoneTeleport::feel_touch_contact(CObject* object)
{
	BOOL bRes = (((CCF_Shape*)CFORM())->Contact(object));
	bRes = bRes && smart_cast<CActor*>(object) && smart_cast<CActor*>(object)->g_Alive();
	return		bRes;
}
 