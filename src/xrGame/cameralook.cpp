#include "stdafx.h"
#pragma hdrstop

#include "CameraLook.h"
#include "../xrEngine/Cameramanager.h"
#include "xr_level_controller.h"
#include "actor.h"

Fvector CCameraLook2::m_cam_offset;
Fvector CCameraLook2::m_cam_offset_rs;
Fvector CCameraLook2::m_cam_offset_ANIMMODE;

CCameraLook::CCameraLook(CObject* p, u32 flags ) 
:CCameraBase(p, flags)
{
}

void CCameraLook::Load(LPCSTR section)
{
	inherited::Load		(section);
	style				= csLookAt;
	lim_zoom			= pSettings->r_fvector2	(section,"lim_zoom");
	dist				= (lim_zoom[0]+lim_zoom[1])*0.5f;
	prev_d				= 0;
}

CCameraLook::~CCameraLook()
{
}

void CCameraLook::Update(Fvector& point, Fvector& /**noise_dangle/**/)
{
	vPosition.set		(point);
	Fmatrix mR;
	mR.setHPB			(-yaw,-pitch,-roll);

	vDirection.set		(mR.k);
	vNormal.set			(mR.j);

	if (m_Flags.is(flRelativeLink))
	{
		parent->XFORM().transform_dir(vDirection);
		parent->XFORM().transform_dir(vNormal);
	}

	UpdateDistance		(point);
}

void CCameraLook::UpdateDistance( Fvector& point )
{
	Fvector				vDir;
	collide::rq_result	R;

	float				covariance = 0;//VIEWPORT_NEAR*6.f;
	vDir.invert			(vDirection);

	g_pGameLevel->ObjectSpace.RayPick( point, vDir, dist+covariance, collide::rqtBoth, R, parent);
 
	float d				= psCamSlideInert*prev_d + (1.f-psCamSlideInert) * (R.range-covariance);
	prev_d = d;

	vPosition.mul		(vDirection, -d-VIEWPORT_NEAR);
	vPosition.add		(point);
}

void CCameraLook::Move( int cmd, float val, float factor)
{
	switch (cmd)
	{
		case kCAM_ZOOM_IN:	dist -= val ? val : (rot_speed.z * Device.fTimeDelta);	break;
		case kCAM_ZOOM_OUT:	dist += val ? val : (rot_speed.z * Device.fTimeDelta);	break;
		case kDOWN:			pitch -= val ? val : (rot_speed.x * Device.fTimeDelta / factor);	break;
		case kUP:			pitch += val ? val : (rot_speed.x * Device.fTimeDelta / factor);	break;
		case kLEFT:			yaw -= val ? val : (rot_speed.y * Device.fTimeDelta / factor);	break;
		case kRIGHT:		yaw += val ? val : (rot_speed.y * Device.fTimeDelta / factor);	break;
	}
	if (bClampYaw)		clamp(yaw, lim_yaw[0], lim_yaw[1]);
	if (bClampPitch)	clamp(pitch, lim_pitch[0], lim_pitch[1]);
	clamp(dist, lim_zoom[0], lim_zoom[1]);
}

void CCameraLook::OnActivate(CCameraBase* old_cam)
{
	if (old_cam && (m_Flags.is(flRelativeLink) == old_cam->m_Flags.is(flRelativeLink)))
	{
		yaw = old_cam->yaw;
		vPosition.set(old_cam->vPosition);
	}
	if (yaw > PI_MUL_2) yaw -= PI_MUL_2;
	if (yaw < -PI_MUL_2)yaw += PI_MUL_2;
}

#include "../xrEngine/xr_input.h"
#include "visual_memory_manager.h"
#include "actor_memory.h"

int cam_dik = DIK_LALT;

void CCameraLook2::OnActivate(CCameraBase* old_cam)
{
	yaw = old_cam->yaw;
	pitch = old_cam->pitch;
	roll = old_cam->roll;
	//CCameraLook::OnActivate(old_cam);
}

void CCameraLook2::Update(Fvector& point, Fvector& noise_dangle)
{
	Fmatrix mR;

	vPosition.set(point);
	mR.setHPB(-yaw, -pitch, -roll);
	vDirection.set(mR.k);
	vNormal.set(mR.j);

	Fmatrix	a_xform;
	a_xform.setXYZ(0, -yaw, 0);
	a_xform.translate_over(point);

	CActor* actor = smart_cast<CActor*>(Level().CurrentControlEntity());

	if (actor)
	{
		if (actor->MpAnimationMode())
		{
			dist = 1.f;
			Fvector offset = m_cam_offset_ANIMMODE;
			a_xform.transform_tiny(offset);
			UpdateDistance(point, a_xform);
		}
		else
		{
			dist = 1.f;
			UpdateDistance(point, a_xform);

		}
	}

}

void CCameraLook2::UpdateDistance(Fvector& point, Fmatrix matr)
{
	collide::rq_result	R, R2, R3;
 
	Fvector offset = m_cam_offset;
	Fvector pos = point;
  
	//g_pGameLevel->ObjectSpace.RayPick(point, Right(), dist, collide::rqtBoth, R, parent);
	//g_pGameLevel->ObjectSpace.RayPick(point, Right().invert(), dist, collide::rqtBoth, R2, parent);
	
	/*
	g_pGameLevel->ObjectSpace.RayPick(point, Direction().invert(), dist, collide::rqtBoth, R3, parent);
  
	if (R3.range < dist)
	{
		offset.z = (R.range / dist) * offset.z;
	}
	*/
  
	prev_x = offset.x;
	prev_y = offset.y;
	prev_z = offset.z;

	matr.transform_tiny(offset);
	vPosition.set(offset);
}

   
void CCameraLook2::Load(LPCSTR section)
{
	CCameraLook::Load		(section);
 	
	m_cam_offset			= pSettings->r_fvector3	  (section, "offset");
	m_cam_offset_ANIMMODE   = pSettings->r_fvector3   (section, "offset_anim_mode");
}

 
void CCameraLook2::Move(int cmd, float val, float factor)
{
	CCameraLook::Move(cmd, val, factor);
}

 
void CCameraFixedLook::Load	(LPCSTR section)
{
	CCameraLook::Load(section);
	style = csFixed;
}

void CCameraFixedLook::OnActivate(CCameraBase* old_cam)
{
	inherited::OnActivate(old_cam);
	m_current_dir.rotationYawPitchRoll(-pitch, -yaw, -roll);	
	
	Fmatrix	rm;
	Fmatrix	trm;
	Fmatrix brm;
	brm.setXYZ(-pitch, -yaw, -roll);
	trm.rotateX(PI_DIV_2);
	rm.mul(brm, trm);
	rm.getXYZ(pitch, yaw, roll);
	m_final_dir.set(rm);
	pitch	= -pitch;
	yaw		= -yaw;
	roll	= -roll;
}

void CCameraFixedLook::Move	(int cmd, float val, float factor)
{
}

void CCameraFixedLook::Update(Fvector& point, Fvector& noise_dangle)
{
	Fquaternion	new_dir;
	new_dir.slerp		(m_current_dir, m_final_dir, Device.fTimeDelta);	//1 sec
	m_current_dir.set	(new_dir);
	
	Fmatrix	rm;
	rm.rotation			(m_current_dir);
	vPosition.set		(point);
	vDirection.set		(rm.k);
	vNormal.set			(rm.j);

	UpdateDistance		(point);
}

void CCameraFixedLook::Set(float Y, float P, float R)
 {
	inherited::Set(Y, P, R);
	Fmatrix	rm;
	rm.setXYZ			(-P, -Y, -R);	
	m_current_dir.set	(rm);
	m_final_dir.set		(m_current_dir);
}