#include "stdafx.h"
#include "phantom.h"
#include "../../level.h"
#include "../../../xrServerEntities/xrserver_objects_alife_monsters.h"
#include "../../../xrEngine/motion.h"
#include "../Include/xrRender/RenderVisual.h"
#include "../Include/xrRender/KinematicsAnimated.h"

// PLAYER FINDER
CObject* FindBestPlayer(float dist, CObject* o)
{
	CObject* entity = 0;
	for (auto pl : Game().players)
	{
		CObject* ent = Level().Objects.net_Find(pl.second->GameID);
		if (ent)
		{
			float d = ent->Position().distance_to(o->Position());
			if (d < dist)
			{
				entity = ent;
				dist = d;
			}
		}
	}

	return entity;
}

CPhantom::CPhantom()
{
	fSpeed = 4.f;
	fASpeed = 1.7f;
	vHP.set(0, 0);
	fContactHit = 0.f;
	m_CurState = stInvalid;
	m_TgtState = stInvalid;
}

CPhantom::~CPhantom()
{
}

//---------------------------------------------------------------------
void CPhantom::Load(LPCSTR section)
{
	inherited::Load(section);
	//////////////////////////////////////////////////////////////////////////
	ISpatial* self = smart_cast<ISpatial*> (this);
	if (self)
	{
		self->spatial.type &= ~STYPE_VISIBLEFORAI;
		self->spatial.type &= ~STYPE_REACTTOSOUND;
	}
	//////////////////////////////////////////////////////////////////////////
	fSpeed = pSettings->r_float(section, "speed");
	fASpeed = pSettings->r_float(section, "angular_speed");
	fContactHit = pSettings->r_float(section, "contact_hit");

	LPCSTR snd_name = 0;
	m_state_data[stBirth].particles = pSettings->r_string(section, "particles_birth");
	snd_name = pSettings->r_string(section, "sound_birth");
	if (snd_name && snd_name[0])		m_state_data[stBirth].sound.create(snd_name, st_Effect, sg_SourceType);

	m_state_data[stFly].particles = pSettings->r_string(section, "particles_fly");
	snd_name = pSettings->r_string(section, "sound_fly");
	if (snd_name && snd_name[0])		m_state_data[stFly].sound.create(snd_name, st_Effect, sg_SourceType);

	m_state_data[stContact].particles = pSettings->r_string(section, "particles_contact");
	snd_name = pSettings->r_string(section, "sound_contact");
	if (snd_name && snd_name[0])		m_state_data[stContact].sound.create(snd_name, st_Effect, sg_SourceType);

	m_state_data[stShoot].particles = pSettings->r_string(section, "particles_shoot");
	snd_name = pSettings->r_string(section, "sound_shoot");
	if (snd_name && snd_name[0])		m_state_data[stShoot].sound.create(snd_name, st_Effect, sg_SourceType);
}

BOOL CPhantom::net_Spawn(CSE_Abstract* DC)
{
	CSE_ALifeCreaturePhantom* OBJ = smart_cast<CSE_ALifeCreaturePhantom*>(DC); VERIFY(OBJ);

	// select visual at first
	LPCSTR vis_name = OBJ->get_visual();
	if (!(vis_name && vis_name[0]))
	{
		LPCSTR visuals = pSettings->r_string(cNameSect(), "visuals");
		u32 cnt = _GetItemCount(visuals);
		string256 tmp;
		OBJ->set_visual(_GetItem(visuals, Random.randI(cnt), tmp));

		if (OnServer())
		{
			// inform server
			NET_Packet		P;
			u_EventGen(P, GE_CHANGE_VISUAL, OBJ->ID);
			P.w_stringZ(tmp);
			u_EventSend(P);
		}
	}

	SwitchToState(stBirth);			// initial state (changed on load method in inherited::)

	// inherited
	if (!inherited::net_Spawn(DC))
		return FALSE;

	if (!IsGameTypeSingle())
	{
		m_enemy = FindBestPlayer(99999, this);
	}

	if (!m_enemy)
		m_enemy = Level().CurrentEntity();


	VERIFY(m_enemy);

	// default init 
	m_fly_particles = 0;
	SetfHealth(0.001f);

	// orientate to enemy
	if (m_enemy)
	{
		XFORM().k.sub(m_enemy->Position(), Position()).normalize();
		XFORM().j.set(0, 1, 0);
		XFORM().i.crossproduct(XFORM().j, XFORM().k);
		XFORM().k.getHP(vHP.x, vHP.y);
	}


	// set animation
	IKinematicsAnimated* K = smart_cast<IKinematicsAnimated*>(Visual());
	m_state_data[stBirth].motion = K->ID_Cycle("birth_0");
	m_state_data[stFly].motion = K->ID_Cycle("fly_0");
	m_state_data[stContact].motion = K->ID_Cycle("contact_0");
	m_state_data[stShoot].motion = K->ID_Cycle("shoot_0");

	VERIFY(K->LL_GetMotionDef(m_state_data[stBirth].motion)->flags & esmStopAtEnd);
	VERIFY(K->LL_GetMotionDef(m_state_data[stContact].motion)->flags & esmStopAtEnd);
	VERIFY(K->LL_GetMotionDef(m_state_data[stShoot].motion)->flags & esmStopAtEnd);

	// set state
	SwitchToState_internal(m_TgtState);

	setVisible(m_CurState > stIdle ? TRUE : FALSE);
	setEnabled(TRUE);

	return			TRUE;
}

void CPhantom::net_Destroy()
{
	inherited::net_Destroy();

	// stop looped
	SStateData& sdata = m_state_data[stFly];
	sdata.sound.stop();
	CParticlesObject::Destroy(m_fly_particles);
}

//---------------------------------------------------------------------
// Animation Callbacks
void CPhantom::animation_end_callback(CBlend* B)
{
	CPhantom* phantom = (CPhantom*)B->CallbackParam;
	switch (phantom->m_CurState)
	{
	case stBirth:
		phantom->SwitchToState(stFly);
		break;
	case stContact:
		phantom->SwitchToState(stIdle);
		break;
	case stShoot:
		phantom->SwitchToState(stIdle);
		break;
	}
}
//---------------------------------------------------------------------
void CPhantom::SwitchToState_internal(EState new_state)
{
	if (new_state != m_CurState)
	{
		IKinematicsAnimated* K = smart_cast<IKinematicsAnimated*>(Visual());
		Fmatrix	xform = XFORM_center();
		UpdateEvent = 0;

		// after event
		switch (m_CurState)
		{
		case stBirth:		break;
		case stFly:			break;
		case stContact:
		{
			SStateData& sdata = m_state_data[m_CurState];
			PlayParticles(sdata.particles.c_str(), FALSE, xform);

			if (OnServer())
			{
				Fvector vE, vP;
				m_enemy->Center(vE);
				Center(vP);
				if (vP.distance_to_sqr(vE) < _sqr(Radius()))
				{
					// hit enemy
					PsyHit(m_enemy, fContactHit);
				}
			}
		}break;
		case stShoot:
		{
			SStateData& sdata = m_state_data[m_CurState];
			PlayParticles(sdata.particles.c_str(), FALSE, xform);
		}break;
		case stIdle:		break;
		}

		// before event
		switch (new_state)
		{
		case stBirth:
		{
			SStateData& sdata = m_state_data[new_state];
			PlayParticles(sdata.particles.c_str(), TRUE, xform);
			sdata.sound.play_at_pos(0, xform.c);
			K->PlayCycle(sdata.motion, TRUE, animation_end_callback, this);
		}break;
		case stFly: {
			UpdateEvent.bind(this, &CPhantom::OnFlyState);
			SStateData& sdata = m_state_data[new_state];
			m_fly_particles = PlayParticles(sdata.particles.c_str(), FALSE, xform);
			sdata.sound.play_at_pos(0, xform.c, sm_Looped);
			K->PlayCycle(sdata.motion);
		}break;
		case stContact: {
			UpdateEvent.bind(this, &CPhantom::OnDeadState);
			SStateData& sdata = m_state_data[new_state];
			sdata.sound.play_at_pos(0, xform.c);
			K->PlayCycle(sdata.motion, TRUE, animation_end_callback, this);
		}break;
		case stShoot: {
			UpdateEvent.bind(this, &CPhantom::OnDeadState);
			SStateData& sdata = m_state_data[new_state];
			PlayParticles(sdata.particles.c_str(), TRUE, xform);
			sdata.sound.play_at_pos(0, xform.c);
			K->PlayCycle(sdata.motion, TRUE, animation_end_callback, this);
		}break;
		case stIdle: {
			UpdateEvent.bind(this, &CPhantom::OnIdleState);
			SStateData& sdata = m_state_data[m_CurState];
			sdata.sound.stop();
			CParticlesObject::Destroy(m_fly_particles);
		}break;
		}

		if (OnServer())
		{
			NET_Packet packet;
			u_EventGen(packet, GE_PHANTOM_MODE, this->ID());
			packet.w_s32(new_state);
			u_EventSend(packet);
		}

		m_CurState = new_state;
	}
}

void CPhantom::OnIdleState()
{
	DestroyObject();
}

void CPhantom::OnFlyState()
{
	UpdateFlyMedia();

	if (g_Alive() && OnServer())
	{
		try
		{
			Fvector vE, vP;
			m_enemy->Center(vE);
			Center(vP);
			if (vP.distance_to_sqr(vE) < _sqr(Radius() + m_enemy->Radius()))
			{
				SwitchToState(stContact);
				float power = 1000.0f;
				float impulse = 100.0f;
				SHit HDS(power, Fvector().set(0, 0, 1), this, BI_NONE, Fvector().set(0, 0, 0), impulse, ALife::eHitTypeFireWound, 0.0f, false);
				Hit(&HDS);
			}
		}
		catch (...)
		{
		}
	}
}

void CPhantom::OnDeadState()
{
	UpdateFlyMedia();
}

void CPhantom::UpdateFlyMedia()
{
	if (OnServer())
		UpdatePosition(m_enemy->Position());

	Fmatrix	xform = XFORM_center();
	// update particles

	if (m_fly_particles)
	{
		Fvector		vel;
		vel.sub(m_enemy->Position(), Position()).normalize_safe().mul(fSpeed);
		m_fly_particles->UpdateParent(xform, vel);
	}

	// update sound
	if (m_state_data[stFly].sound._feedback())
		m_state_data[stFly].sound.set_position(xform.c);
}
//---------------------------------------------------------------------


void CPhantom::shedule_Update(u32 DT)
{
	spatial.type &= ~STYPE_VISIBLEFORAI;

	inherited::shedule_Update(DT);

	IKinematicsAnimated* K = smart_cast<IKinematicsAnimated*>(Visual());
	K->UpdateTracks();
}


void CPhantom::UpdateCL()
{
	inherited::UpdateCL();

	if (LastUpdateDistance < Device.dwTimeGlobal || !m_enemy)
	{
		m_enemy = FindBestPlayer(99999, this);
		LastUpdateDistance = Device.dwTimeGlobal + 500;
	}


	if (!UpdateEvent.empty())
		UpdateEvent();

	if (OnServer())
	{
		if (m_TgtState != m_CurState)
			SwitchToState_internal(m_TgtState);
	}
}
//---------------------------------------------------------------------
void	CPhantom::Hit(SHit* pHDS)
{
	if (m_TgtState == stFly)
		SwitchToState(stShoot);

	if (g_Alive())
	{
		SetfHealth(-1.f);
		inherited::Hit(pHDS);
	}
}

void CPhantom::OnEvent(NET_Packet& Packet, u16 type)
{
	switch (type)
	{
	case GE_PHANTOM_MODE:
	{
		if (OnClient())
			SwitchToState_internal((EState)Packet.r_s32());
	}break;

	default:
		inherited::OnEvent(Packet, type);
		break;
	}
}


//---------------------------------------------------------------------
Fmatrix	CPhantom::XFORM_center()
{
	Fvector			center;
	Center(center);
	Fmatrix	xform = XFORM();
	return			xform.translate_over(center);
}

CParticlesObject* CPhantom::PlayParticles(const shared_str& name, BOOL bAutoRemove, const Fmatrix& xform)
{
	CParticlesObject* ps = CParticlesObject::Create(name.c_str(), bAutoRemove);
	ps->UpdateParent(xform, zero_vel);
	ps->Play(false);
	return bAutoRemove ? 0 : ps;
}

//---------------------------------------------------------------------
void CPhantom::UpdatePosition(const Fvector& tgt_pos)
{
	float			tgt_h, tgt_p;
	Fvector			tgt_dir, cur_dir;
	tgt_dir.sub(tgt_pos, Position());
	tgt_dir.getHP(tgt_h, tgt_p);

	angle_lerp(vHP.x, tgt_h, fASpeed, Device.fTimeDelta);
	angle_lerp(vHP.y, tgt_p, fASpeed, Device.fTimeDelta);

	cur_dir.setHP(vHP.x, vHP.y);

	Fvector prev_pos = Position();
	XFORM().rotateY(-vHP.x);
	Position().mad(prev_pos, cur_dir, fSpeed * Device.fTimeDelta);
}

void CPhantom::PsyHit(const CObject* object, float value)
{
	NET_Packet			P;
	SHit				HS;
	HS.GenHeader(GE_HIT, object->ID());				//				//	u_EventGen		(P,GE_HIT, object->ID());				
	HS.whoID = (ID());					// own			//	P.w_u16			(object->ID());							
	HS.weaponID = (ID());					// own			//	P.w_u16			(object->ID());							
	HS.dir = (Fvector().set(0.f, 1.f, 0.f));		// direction	//	P.w_dir			(Fvector().set(0.f,1.f,0.f));			
	HS.power = (value);							// hit value	//	P.w_float		(value);								
	HS.boneID = (BI_NONE);						// bone			//	P.w_s16			(BI_NONE);								
	HS.p_in_bone_space = (Fvector().set(0.f, 0.f, 0.f));						//	P.w_vec3		(Fvector().set(0.f,0.f,0.f));			
	HS.impulse = (0.f);											//	P.w_float		(0.f);									
	HS.hit_type = (ALife::eHitTypeTelepatic);						//	P.w_u16			(u16(ALife::eHitTypeTelepatic));
	HS.Write_Packet(P);

	u_EventSend(P);
}

//---------------------------------------------------------------------
// Core events
void CPhantom::save(NET_Packet& output_packet)
{
	output_packet.w_s32(s32(m_CurState));
}
void CPhantom::load(IReader& input_packet)
{
	SwitchToState(EState(input_packet.r_s32()));
}
void CPhantom::net_Export(NET_Packet& P)					// export to server
{
	// export 
	R_ASSERT(Local());

	if (IsGameTypeSingle())
	{
		u8					flags = 0;
		P.w_float(GetfHealth());

		P.w_float(0);
		P.w_u32(0);
		P.w_u32(0);

		P.w_u32(Device.dwTimeGlobal);
		P.w_u8(flags);

		float				yaw, pitch, bank;
		XFORM().getHPB(yaw, pitch, bank);

		P.w_float(yaw);
		P.w_float(yaw);
		P.w_float(pitch);
		P.w_float(0);
		P.w_u8(u8(g_Team()));
		P.w_u8(u8(g_Squad()));
		P.w_u8(u8(g_Group()));


		P.w_vec3(Position());
	}
	else
	{
		float yaw, pitch, tmp;
		XFORM().getHPB(yaw, pitch, tmp);
		P.w_angle8(yaw);
		P.w_angle8(pitch);
		P.w_vec3(Position());
		P.w_float_q8(GetfHealth(), 0, 1);
	}
}

void CPhantom::net_Import(NET_Packet& P)
{
	// import
	R_ASSERT(Remote());

	if (IsGameTypeSingle())
	{
		u8					flags;

		float health;
		P.r_float(health);
		SetfHealth(health);

		float fDummy;
		u32 dwDummy;
		P.r_float(fDummy);
		P.r_u32(dwDummy);
		P.r_u32(dwDummy);

		P.r_u32(dwDummy);
		P.r_u8(flags);

		float				yaw, pitch, bank = 0, roll = 0;

		P.r_float(yaw);
		P.r_float(yaw);
		P.r_float(pitch);
		P.r_float(roll);

		id_Team = P.r_u8();
		id_Squad = P.r_u8();
		id_Group = P.r_u8();

		XFORM().setHPB(yaw, pitch, bank);

		Position().set(P.r_vec3());
	}
	else
	{
		float yaw, pitch;
		P.r_angle8(yaw);
		P.r_angle8(pitch);
		XFORM().setHPB(yaw, pitch, 0);
		Position().set(P.r_vec3());
		SetfHealth(P.r_float_q8(0, 1));
	}

}

