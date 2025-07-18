#include "stdafx.h"
#include "radioactivezone.h"
#include "level.h"
#include "xrmessages.h"
#include "../xrEngine/bone.h"
#include "actor.h"
#include "game_base_space.h"
#include "Hit.h"
#include "../xrengine/xr_collide_form.h"

CRadioactiveZone::CRadioactiveZone(void) 
{}

CRadioactiveZone::~CRadioactiveZone(void) 
{}

void CRadioactiveZone::Load(LPCSTR section) 
{
	inherited::Load(section);
}

bool  CRadioactiveZone::BlowoutState	()
{
	bool result = inherited::BlowoutState();
	if(!result) UpdateBlowout();
	return result;
}

extern int debuging_hit_zones;
void CRadioactiveZone::Affect(SZoneObjectInfo* O) 
{
	float one				= 0.1f;
	float tg				= Device.fTimeGlobal;

	if(!O->object || O->f_time_affected+one > Device.fTimeGlobal) 
		return;

	clamp					(O->f_time_affected, tg-(one*3), tg);

	Fvector					pos; 
	XFORM().transform_tiny	(pos,CFORM()->getSphere().P);

	Fvector dir				={0,0,0}; 
	float power				= Power(O->object->Position().distance_to(pos),nearest_shape_radius(O));

	float impulse			= 0.0f;
	if(power < EPS)			
	{
		O->f_time_affected	= tg;
		return;
	}
	
	float send_power		= power*one;

	u32 Hits = 0;
	while(O->f_time_affected+one < tg)
	{
		CreateHit	(	O->object->ID(),
						ID(),
						dir,
						send_power,
						BI_NONE,
						Fvector().set(0.0f,0.0f,0.0f),
						impulse,
						m_eHitTypeBlowout);
		O->f_time_affected += one;
	}

	if (debuging_hit_zones)
		Msg("[CRadioactiveZone] dwFrame[%u] Anomaly [%s] Hits[%u] is Hit Sended Event[9] [40] Byte", Device.dwFrame, this->cName().c_str(), Hits);
}

void CRadioactiveZone::feel_touch_new					(CObject* O	)
{
	inherited::feel_touch_new(O);
	if (GameID() != eGameIDSingle)
	{
		if (smart_cast<CActor*>(O))
		{
			CreateHit(O->ID(),ID(),Fvector().set(0, 0, 0),0.0f,BI_NONE,Fvector().set(0, 0, 0),0.0f,m_eHitTypeBlowout);// ALife::eHitTypeRadiation
		}
	};
};

#include "actor.h"
BOOL CRadioactiveZone::feel_touch_contact(CObject* O)
{

	CActor* A = smart_cast<CActor*>(O);
	if ( A )
	{ 
		if (!((CCF_Shape*)CFORM())->Contact(O))		return	FALSE;
		return										A->feel_touch_on_contact(this);
	}else
		return										FALSE;
}

void CRadioactiveZone::UpdateWorkload					(u32	dt)
{
	if (IsEnabled() && GameID() != eGameIDSingle)
	{	
		OBJECT_INFO_VEC_IT it;
		Fvector pos; 
		XFORM().transform_tiny(pos,CFORM()->getSphere().P);
		for(it = m_ObjectInfoMap.begin(); m_ObjectInfoMap.end() != it; ++it) 
		{
			if( !(*it).object->getDestroy() && smart_cast<CActor*>((*it).object))
			{
				//=====================================
				NET_Packet	l_P;
				l_P.write_start();
				l_P.read_start();

				float dist			= (*it).object->Position().distance_to(pos);
				float power			= Power(dist,nearest_shape_radius(&*it))*dt/1000;

				SHit				HS;
				HS.GenHeader		(GE_HIT, (*it).object->ID());
				HS.whoID			= ID();
				HS.weaponID			= ID();
				HS.dir				= Fvector().set(0,0,0);
				HS.power			= power;
				HS.boneID			= BI_NONE;
				HS.p_in_bone_space	= Fvector().set(0, 0, 0);
				HS.impulse			= 0.0f;
				HS.hit_type			= m_eHitTypeBlowout;
				
				HS.Write_Packet_Cont(l_P);

				(*it).object->OnEvent(l_P, HS.PACKET_TYPE);
				//=====================================
			};
		}
	}
	inherited::UpdateWorkload(dt);
}

float CRadioactiveZone::nearest_shape_radius(SZoneObjectInfo* O)
{
	CCF_Shape* Sh		= (CCF_Shape*)CFORM();

	if(Sh->Shapes().size()==1)
	{
		return			Radius();
	}else
	{
		xr_vector<CCF_Shape::shape_def>& Shapes = Sh->Shapes();
		CCF_Shape::shape_def& s = Shapes[0];
		return s.data.sphere.R;
	}
}

