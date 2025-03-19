#include "stdafx.h"
#include "mosquitobald.h"
#include "ParticlesObject.h"
#include "level.h"
#include "physicsshellholder.h"
#include "../xrengine/xr_collide_form.h"

CMosquitoBald::CMosquitoBald(void) 
{
	m_fHitImpulseScale		= 1.f;
	m_bLastBlowoutUpdate	= false;
}

CMosquitoBald::~CMosquitoBald(void) 
{
}

void CMosquitoBald::Load(LPCSTR section) 
{
	inherited::Load(section);
}


bool CMosquitoBald::BlowoutState()
{
	bool result = inherited::BlowoutState();
	if(!result)
	{
		m_bLastBlowoutUpdate = false;
		UpdateBlowout();
	}
	else if(!m_bLastBlowoutUpdate)
	{
		m_bLastBlowoutUpdate = true;
		UpdateBlowout();
	}

	return result;
}
//bool CMosquitoBald::SecondaryHitState()
//{
//	bool result = inherited::SecondaryHitState();
//	if(!result)
//		UpdateBlowout();
//
//	return result;
//}

extern int debuging_hit_zones;
void CMosquitoBald::Affect(SZoneObjectInfo* O) 
{
	CPhysicsShellHolder *pGameObject = smart_cast<CPhysicsShellHolder*>(O->object);
	if(!pGameObject) return;

	if(O->zone_ignore) return;

	Fvector P; 
	XFORM().transform_tiny(P,CFORM()->getSphere().P);

	Fvector hit_dir; 
	hit_dir.set(	::Random.randF(-.5f,.5f), 
					::Random.randF(.0f,1.f), 
					::Random.randF(-.5f,.5f)); 
	hit_dir.normalize();

	Fvector position_in_bone_space;

	VERIFY(!pGameObject->getDestroy());

	float dist = pGameObject->Position().distance_to(P) - pGameObject->Radius();
	float power = Power(dist>0.f?dist:0.f, Radius());
	float impulse = m_fHitImpulseScale*power*pGameObject->GetMass();

	if(power > 0.01f) 
	{
		position_in_bone_space.set(0.f,0.f,0.f);

		if (debuging_hit_zones)
		Msg("[CMosquitoBald] dwFrame[%u] Anomaly [%s] is Hit Sended Event[9] [40] Byte", Device.dwFrame, this->cName().c_str());

		CreateHit(pGameObject->ID(),ID(),hit_dir,power,0,position_in_bone_space,impulse,m_eHitTypeBlowout);
		PlayHitParticles(pGameObject);
	}
}

#include <Actor.h>
void CMosquitoBald::UpdateSecondaryHit()
{
	if (!OnServer())
		return;

	if(m_dwAffectFrameNum == Device.dwFrame)	
		return;

	m_dwAffectFrameNum	= Device.dwFrame;
	if(Device.dwPrecacheFrame)					
		return;
 
	OBJECT_INFO_VEC_IT it;
	bool isSendedHit = false;
 
	for(it = m_ObjectInfoMap.begin(); m_ObjectInfoMap.end() != it; ++it) 
	{
		if(!(*it).object->getDestroy())
		{
			CPhysicsShellHolder *pGameObject = smart_cast<CPhysicsShellHolder*>((&(*it))->object);
			if(!pGameObject)
				return;

			if((&(*it))->zone_ignore)
				return;
			 
			Fvector P; 
			XFORM().transform_tiny(P,CFORM()->getSphere().P);

			Fvector hit_dir; 
			hit_dir.set(	::Random.randF(-.5f,.5f), 
							::Random.randF(.0f,1.f), 
							::Random.randF(-.5f,.5f)); 
			hit_dir.normalize();

			Fvector position_in_bone_space;

			VERIFY(!pGameObject->getDestroy());

			float dist = pGameObject->Position().distance_to(P) - pGameObject->Radius();
			float power = m_fSecondaryHitPower * RelativePower(dist>0.f?dist:0.f, Radius());
			if(power<0.0f)
				return;

			float impulse = m_fHitImpulseScale*power*pGameObject->GetMass();
			position_in_bone_space.set(0.f,0.f,0.f);
			 
			if (!smart_cast<CActor*>(pGameObject))
			{
				auto CL = Level().Server->GetServerClient()->ID;
 				u16		id_to	 = pGameObject->ID();
				u16     id_from  = ID();
				
			
				SHit Hit = SHit(power, hit_dir, this, 0, position_in_bone_space, impulse, m_eHitTypeBlowout, 0.0f, false);
				Hit.GenHeader(GE_HIT, id_to);
				Hit.whoID = id_from;
				Hit.weaponID = this->ID();


				NET_Packet			Packet;
				Hit.Write_Packet(Packet);				
				Level().Server->SendTo(CL, Packet);
  			}
			else
			{
				CreateHit(pGameObject->ID(), ID(), hit_dir, power, 0, position_in_bone_space, impulse, m_eHitTypeBlowout);
			}
 
			isSendedHit = true;
  		}
	}

	if (isSendedHit && debuging_hit_zones)
	{
		Msg("[MosquitoBald] [SecondHit] dwFrame[%u] Anomaly [%s] is Hit Sended Event[9] [40] Byte", Device.dwFrame, this->cName().c_str());
	}
}
