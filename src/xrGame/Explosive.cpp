// Explosive.cpp: ��������� ��� ������������� ��������
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "explosive.h"

#include "../xrphysics/PhysicsShell.h"
#include "entity.h"
#include "ParticlesObject.h"

//��� ������ ����������� ������� ��������� ���������
#include "Weapon.h"

#include "actor.h"
#include "actoreffector.h"
#include "level.h"
#include "level_bullet_manager.h"
#include "xrmessages.h"
#include "../xrEngine/gamemtllib.h"

#ifdef DEBUG
#	include "../xrEngine/StatGraph.h"
#	include "PHDebug.h"
#endif

#include "../xrphysics/MathUtils.h"
#include "../xrphysics/iActivationShape.h"
#include "../xrphysics/iphworld.h"
#include "game_base_space.h"
#include "profiler.h"

#include "../Include/xrRender/Kinematics.h"

#include "HudSound.h"

#define EFFECTOR_RADIUS 30.f
const u16	TEST_RAYS_PER_OBJECT = 5;
const u16	BLASTED_OBJ_PROCESSED_PER_FRAME = 3;
const float	exp_dist_extinction_factor = 3.f;//(>1.f, 1.f -means no dist change of exp effect)	on the dist of m_fBlastRadius exp. wave effect in exp_dist_extinction_factor times less than maximum

CExplosive::CExplosive(void)
{
	m_fBlastHit = 50.0f;
	m_fBlastRadius = 10.0f;
	m_iFragsNum = 20;
	m_fFragsRadius = 30.0f;
	m_fFragHit = 50.0f;
	m_fUpThrowFactor = 0.f;

	m_eSoundExplode = ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
	m_eHitTypeBlast = ALife::eHitTypeExplosion;
	m_eHitTypeFrag = ALife::eHitTypeFireWound;

	m_iCurrentParentID = 0xffff;
	m_explosion_flags.assign(0);
	m_vExplodeSize.set(0.001f, 0.001f, 0.001f);
	m_bHideInExplosion = TRUE;
	m_fExplodeHideDurationMax = 0;
	m_bDynamicParticles = FALSE;
	m_pExpParticle = NULL;
}

void CExplosive::LightCreate()
{
	m_pLight = ::Render->light_create();
	m_pLight->set_shadow(true);
}

void CExplosive::LightDestroy()
{
	m_pLight.destroy();
}

CExplosive::~CExplosive(void)
{
	sndExplode.destroy();
}

void CExplosive::Load(LPCSTR section)
{
	Load(pSettings, section);
}

void CExplosive::Load(CInifile const* ini, LPCSTR section)
{
	m_fBlastHit = ini->r_float(section, "blast");
	m_fBlastRadius = ini->r_float(section, "blast_r");
	m_fBlastHitImpulse = ini->r_float(section, "blast_impulse");

	m_iFragsNum = ini->r_s32(section, "frags");
	m_fFragsRadius = ini->r_float(section, "frags_r");
	m_fFragHit = ini->r_float(section, "frag_hit");
	m_fFragHitImpulse = ini->r_float(section, "frag_hit_impulse");

	m_eHitTypeBlast = ALife::g_tfString2HitType(ini->r_string(section, "hit_type_blast"));
	m_eHitTypeFrag = ALife::g_tfString2HitType(ini->r_string(section, "hit_type_frag"));

	m_fUpThrowFactor = ini->r_float(section, "up_throw_factor");


	fWallmarkSize = ini->r_float(section, "wm_size");
	R_ASSERT(fWallmarkSize > 0);

	m_sExplodeParticles = ini->r_string(section, "explode_particles");

	sscanf(ini->r_string(section, "light_color"), "%f,%f,%f", &m_LightColor.r, &m_LightColor.g, &m_LightColor.b);
	m_fLightRange = ini->r_float(section, "light_range");
	m_fLightTime = ini->r_float(section, "light_time");

	//������ ��� ������� ��������
	m_fFragmentSpeed = ini->r_float(section, "fragment_speed");


	// se7kills SOUND LAYERS FROM ADVANCED XRAY
	//LPCSTR	snd_name = ini->r_string(section, "snd_explode");
	//sndExplode.create(snd_name, st_Effect, m_eSoundExplode);

	m_layered_sounds.LoadSound(ini, section, "snd_explode", "sndExplode", false, m_eSoundExplode);

	m_fExplodeDurationMax = ini->r_float(section, "explode_duration");

	effector.effect_sect_name = ini->r_string("explode_effector", "effect_sect_name");
	//	if( ini->line_exist(section,"wallmark_section") )
	//	{
	m_wallmark_manager.m_owner = cast_game_object();
	//		m_wallmark_manager.Load(pSettings,ini->r_string(section,"wallmark_section"));
	//	}

	m_bHideInExplosion = TRUE;
	if (ini->line_exist(section, "hide_in_explosion"))
	{
		m_bHideInExplosion = ini->r_bool(section, "hide_in_explosion");
		m_fExplodeHideDurationMax = 0;
		if (ini->line_exist(section, "explode_hide_duration"))
		{
			m_fExplodeHideDurationMax = ini->r_float(section, "explode_hide_duration");
		}
	}

	m_bDynamicParticles = FALSE;
	if (ini->line_exist(section, "dynamic_explosion_particles"))
		m_bDynamicParticles = ini->r_bool(section, "dynamic_explosion_particles");
}

void CExplosive::net_Destroy()
{
	m_blasted_objects.clear();
	StopLight();
	m_explosion_flags.assign(0);
}


struct SExpQParams
{
	SExpQParams()
	{
		shoot_factor = 1.f;
	}
	float		shoot_factor;
};

//�������� �� ��������� "��������" �� �������
ICF static BOOL grenade_hit_callback(collide::rq_result& result, LPVOID params)
{
	SExpQParams& ep = *(SExpQParams*)params;
	u16 mtl_idx = GAMEMTL_NONE_IDX;
	if (result.O) {
		IKinematics* V = 0;
		if (0 != (V = smart_cast<IKinematics*>(result.O->Visual()))) {
			CBoneData& B = V->LL_GetData((u16)result.element);
			mtl_idx = B.game_mtl_idx;
		}
	}
	else {
		//�������� ����������� � ������ ��� ��������
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
		mtl_idx = T->material;
	}
	SGameMtl* mtl = GMLib.GetMaterialByIdx(mtl_idx);
	float shoot_factor = 1.f - mtl->fShootFactor;
	ep.shoot_factor *= shoot_factor;

	return				(ep.shoot_factor > 0.01f);
}



float CExplosive::ExplosionEffect(collide::rq_results& storage, CExplosive* exp_obj, CPhysicsShellHolder* blasted_obj, const Fvector& expl_centre, const float expl_radius)
{

	const Fmatrix& obj_xform = blasted_obj->XFORM();
	Fmatrix	inv_obj_form; inv_obj_form.invert(obj_xform);
	Fvector	local_exp_center; inv_obj_form.transform_tiny(local_exp_center, expl_centre);

	const Fbox& l_b1 = blasted_obj->BoundingBox();
	if (l_b1.contains(local_exp_center))
		return 1.f;
	Fvector l_c, l_d; l_b1.get_CD(l_c, l_d);
	float effective_volume = l_d.x * l_d.y * l_d.z;
	float max_s = effective_volume / (_min(_min(l_d.x, l_d.y), l_d.z));
	if (blasted_obj->PPhysicsShell() && blasted_obj->PPhysicsShell()->isActive())
	{
		float ph_volume = blasted_obj->PPhysicsShell()->getVolume();
		if (ph_volume < effective_volume)effective_volume = ph_volume;
	}

	float effect = 0.f;
	for (u16 i = 0; i < TEST_RAYS_PER_OBJECT; ++i) {
		Fvector l_source_p, l_end_p;
		l_end_p.random_point(l_d);
		l_end_p.add(l_c);
		obj_xform.transform_tiny(l_end_p);
		GetRaySourcePos(exp_obj, expl_centre, l_source_p);
		Fvector l_local_source_p; inv_obj_form.transform_tiny(l_local_source_p, l_source_p);
		if (l_b1.contains(l_local_source_p))
		{
			effect += 1.f;
			continue;
		}

		Fvector l_dir; l_dir.sub(l_end_p, l_source_p);
		float mag = l_dir.magnitude();

		if (fis_zero(mag))
			return 1.f;

		l_dir.mul(1.f / mag);

		float l_S = effective_volume * (_abs(l_dir.dotproduct(obj_xform.i)) / l_d.x + _abs(l_dir.dotproduct(obj_xform.j)) / l_d.y + _abs(l_dir.dotproduct(obj_xform.k)) / l_d.z);
		effect += _sqrt(l_S / max_s) * TestPassEffect(l_source_p, l_dir, mag, expl_radius, storage, blasted_obj);
	}

	return effect / TEST_RAYS_PER_OBJECT;

}
float CExplosive::TestPassEffect(const	Fvector& source_p, const	Fvector& dir, float range, float ef_radius, collide::rq_results& storage, CObject* blasted_obj)
{
	float sq_ef_radius = ef_radius * ef_radius;
	float dist_factor = sq_ef_radius / (range * range * (exp_dist_extinction_factor - 1.f) + sq_ef_radius);
	float shoot_factor = 1.f;
	if (range > EPS_L)
	{
		VERIFY(!fis_zero(dir.square_magnitude()));
		collide::ray_defs	RD(source_p, dir, range, CDB::OPT_CULL, collide::rqtBoth);
		VERIFY(!fis_zero(RD.dir.square_magnitude()));

		SExpQParams			ep;
		g_pGameLevel->ObjectSpace.RayQuery(storage, RD, grenade_hit_callback, &ep, NULL, blasted_obj);
		shoot_factor = ep.shoot_factor;
	}
	else return dist_factor;
	return shoot_factor * dist_factor;
}

void CExplosive::Explode()
{
	m_explosion_flags.set(flExploding, TRUE);
	cast_game_object()->processing_activate();

	Fvector& pos = m_vExplodePos;
	Fvector& dir = m_vExplodeDir;

	OnBeforeExplosion();

	m_layered_sounds.PlaySound("sndExplode", pos, smart_cast<CObject*>(this), false, false, (u8)-1);

	//���������� �������

	m_wallmark_manager.PlaceWallmarks(pos);

	Fvector									vel;
	smart_cast<CPhysicsShellHolder*>(cast_game_object())->PHGetLinearVell(vel);

	Fmatrix explode_matrix;
	explode_matrix.identity();
	explode_matrix.j.set(dir);
	Fvector::generate_orthonormal_basis(explode_matrix.j, explode_matrix.i, explode_matrix.k);
	explode_matrix.c.set(pos);

	CParticlesObject* pStaticPG;
	pStaticPG = CParticlesObject::Create(*m_sExplodeParticles, !m_bDynamicParticles);
	if (m_bDynamicParticles) 
		m_pExpParticle = pStaticPG;
	pStaticPG->UpdateParent(explode_matrix, vel);
	pStaticPG->Play(false);

	//�������� ��������� �� ������
	StartLight();

	//trace frags
	Fvector frag_dir;

	//////////////////////////////
	//�������
	//////////////////////////////
	//-------------------------------------
	bool SendHits = false;
	if (OnServer())
		SendHits = true;
	else
		SendHits = false;


	for (int i = 0; i < m_iFragsNum; ++i)
	{
		frag_dir.random_dir();
		frag_dir.normalize();

		CCartridge cartridge;
		cartridge.param_s.kDist = 1.f;
		cartridge.param_s.kHit = 1.f;
 		cartridge.param_s.kImpulse = 1.f;
		cartridge.param_s.kAP = 1.f;
		cartridge.param_s.fWallmarkSize = fWallmarkSize;
		cartridge.bullet_material_idx = GMLib.GetMaterialIdx(WEAPON_MATERIAL_NAME);
		cartridge.m_flags.set(CCartridge::cfTracer, FALSE);

		Level().BulletManager().AddBullet(pos, frag_dir, m_fFragmentSpeed,
			m_fFragHit, m_fFragHitImpulse, Initiator(),
			cast_game_object()->ID(), m_eHitTypeFrag, m_fFragsRadius,
			cartridge, 1.f, SendHits);
	}

	if (cast_game_object()->Remote())
		return;

 	/////////////////////////////////
	//�������� �����
	////////////////////////////////
	//---------------------------------------------------------------------
	xr_vector<ISpatial*>	ISpatialResult;
	g_SpatialSpace->q_sphere(ISpatialResult, 0, STYPE_COLLIDEABLE, pos, m_fBlastRadius);

	m_blasted_objects.clear();
	for (u32 o_it = 0; o_it < ISpatialResult.size(); o_it++)
	{
		ISpatial* spatial = ISpatialResult[o_it];
		//		feel_touch_new(spatial->dcast_CObject());

		CPhysicsShellHolder* pGameObject = smart_cast<CPhysicsShellHolder*>(spatial->dcast_CObject());
		if (pGameObject && cast_game_object()->ID() != pGameObject->ID())
			m_blasted_objects.push_back(pGameObject);
	}

	GetExplosionBox(m_vExplodeSize);
	
	START_PROFILE("explosive/activate explosion box")
	ActivateExplosionBox(m_vExplodeSize, m_vExplodePos);
	STOP_PROFILE
		//---------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// Explode Effector	//////////////
	CGameObject* GO = smart_cast<CGameObject*>(Level().CurrentEntity());
	CActor* pActor = smart_cast<CActor*>(GO);
	if (pActor)
	{
		float dist_to_actor = pActor->Position().distance_to(pos);
		float max_dist = EFFECTOR_RADIUS;
		if (dist_to_actor < max_dist)
			AddEffector(pActor, effExplodeHit, effector.effect_sect_name, (max_dist - dist_to_actor) / max_dist);
	}
}

void CExplosive::PositionUpdate()
{
	Fvector vel;
	Fvector& pos = m_vExplodePos;
	Fvector& dir = m_vExplodeDir;
	GetExplVelocity(vel);
	GetExplPosition(pos);
	GetExplDirection(dir);
	Fmatrix explode_matrix;
	explode_matrix.identity();
	explode_matrix.j.set(dir);
	Fvector::generate_orthonormal_basis(explode_matrix.j, explode_matrix.i, explode_matrix.k);
	explode_matrix.c.set(pos);

}
void CExplosive::GetExplPosition(Fvector& p)
{
	p.set(m_vExplodePos);
}

void CExplosive::GetExplDirection(Fvector& d)
{
	d.set(m_vExplodeDir);
}
void CExplosive::GetExplVelocity(Fvector& v)
{
	smart_cast<CPhysicsShellHolder*>(cast_game_object())->PHGetLinearVell(v);
}

void CExplosive::UpdateCL()
{
	VERIFY(!physics_world()->Processing());
	if (!m_explosion_flags.test(flExploding))
		return;

	if (m_explosion_flags.test(flExploded))
	{
		CGameObject* go = cast_game_object();
		go->processing_deactivate();
		m_explosion_flags.set(flExploding, FALSE);
		OnAfterExplosion();
		return;
	}

	//����� �����, ������� ������ ����������
	if (m_fExplodeDuration < 0.f && m_blasted_objects.empty())
	{
		m_explosion_flags.set(flExploded, TRUE);
		StopLight();
	}
	else
	{
		m_fExplodeDuration -= Device.fTimeDelta;
		if (!m_bHideInExplosion && !m_bAlreadyHidden)
		{
			if (m_fExplodeHideDurationMax <= (m_fExplodeDurationMax - m_fExplodeDuration))
			{
				HideExplosive();
			}
		}
		UpdateExplosionPos();
		UpdateExplosionParticles();
		ExplodeWaveProcess();
		//�������� ��������� ������
		if (m_pLight && m_pLight->get_active() && m_fLightTime > 0)
		{
			if (m_fExplodeDuration > (m_fExplodeDurationMax - m_fLightTime))
			{
				float scale = (m_fExplodeDuration - (m_fExplodeDurationMax - m_fLightTime)) / m_fLightTime;
				m_pLight->set_color(m_LightColor.r * scale, m_LightColor.g * scale, m_LightColor.b * scale);
				m_pLight->set_range(m_fLightRange * scale);
			}
			else
				StopLight();
		}
	}
}

void CExplosive::OnAfterExplosion()
{
	if (m_pExpParticle)
	{
		m_pExpParticle->Stop();
		CParticlesObject::Destroy(m_pExpParticle);
		m_pExpParticle = NULL;
	}

	//������������� ��� ������ 
	if (cast_game_object()->Local())
		cast_game_object()->DestroyObject();
}
void CExplosive::OnBeforeExplosion()
{
	m_bAlreadyHidden = false;
	if (m_bHideInExplosion)
	{
		HideExplosive();
	}
}
void CExplosive::HideExplosive()
{
	CGameObject* GO = cast_game_object();
	GO->setVisible(FALSE);
	GO->setEnabled(FALSE);
	CPhysicsShell* phshell = (smart_cast<CPhysicsShellHolder*>(GO))->PPhysicsShell();
	if (phshell)
	{
		phshell->Disable();
		phshell->DisableCollision();
	}
	m_bAlreadyHidden = true;
};

void CExplosive::OnEvent(NET_Packet& P, u16 type)
{
	switch (type)
	{
	case GE_GRENADE_EXPLODE:
	{
		if (!m_explosion_flags.test(flExploding))
		{
			Fvector pos, normal;
			u16 parent_id;
			P.r_u16(parent_id);
			P.r_vec3(pos);
			P.r_vec3(normal);

			SetInitiator(parent_id);
			ExplodeParams(pos, normal);
			Explode();
			m_fExplodeDuration = m_fExplodeDurationMax;
		}
		break;
	}
	}
}

void CExplosive::ExplodeParams(const Fvector& pos, const Fvector& dir)
{
	m_explosion_flags.set(flReadyToExplode, TRUE);
	m_vExplodePos = pos;
	m_vExplodePos.y += 0.1f;// fake
	m_vExplodeDir = dir;
}

void CExplosive::GenExplodeEvent(const Fvector& pos, const Fvector& normal)
{
	if (OnClient() || cast_game_object()->Remote())
		return;

	VERIFY(!m_explosion_flags.test(flExplodEventSent));
	VERIFY(0xffff != Initiator());

	NET_Packet		P;
	cast_game_object()->u_EventGen(P, GE_GRENADE_EXPLODE, cast_game_object()->ID());
	P.w_u16(Initiator());
	P.w_vec3(pos);
	P.w_vec3(normal);
	cast_game_object()->u_EventSend(P);

	//m_bExplodeEventSent = true;
	m_explosion_flags.set(flExplodEventSent, TRUE);
}

void CExplosive::GenExplodeEventFromClient(const Fvector& pos, const Fvector& normal)
{
 	NET_Packet		P;
	cast_game_object()->u_EventGen(P, GE_GRENADE_EXPLODE, cast_game_object()->ID());
	P.w_u16(Initiator());
	P.w_vec3(pos);
	P.w_vec3(normal);
	cast_game_object()->u_EventSend(P);

	m_explosion_flags.set(flExplodEventSent, TRUE);
}

void CExplosive::FindNormal(Fvector& normal)
{
	collide::rq_result RQ;

	Fvector pos, dir;
	dir.set(0, -1.f, 0);
	cast_game_object()->Center(pos);

	BOOL result = Level().ObjectSpace.RayPick(pos, dir, cast_game_object()->Radius(),
		collide::rqtBoth, RQ, NULL);
	if (!result || RQ.O) {
		normal.set(0, 1, 0);
		//���� ����� �� �������
		//����� ����������� � ��������� ������� �� ����
	}
	else
	{
		Fvector* pVerts = Level().ObjectSpace.GetStaticVerts();
		CDB::TRI* pTri = Level().ObjectSpace.GetStaticTris() + RQ.element;
		normal.mknormal(pVerts[pTri->verts[0]], pVerts[pTri->verts[1]], pVerts[pTri->verts[2]]);
	}
}

void CExplosive::StartLight()
{

	VERIFY(!physics_world()->Processing());
	if (m_fLightTime > 0)
	{
		LightCreate();

		m_pLight->set_color(m_LightColor.r, m_LightColor.g, m_LightColor.b);
		m_pLight->set_range(m_fLightRange);
		m_pLight->set_position(m_vExplodePos);
		m_pLight->set_active(true);
	}
}

void CExplosive::StopLight()
{
	if (m_pLight) {
		VERIFY(!physics_world()->Processing());
		m_pLight->set_active(false);
		LightDestroy();
	}
}

void CExplosive::GetRaySourcePos(CExplosive* exp_obj, const	Fvector& expl_center, Fvector& p)
{
	if (exp_obj)
	{
		exp_obj->GetRayExplosionSourcePos(p);
	}
}

void CExplosive::GetRayExplosionSourcePos(Fvector& pos)
{
	pos.set(m_vExplodeSize); pos.mul(0.5f);
	pos.random_point(pos);
	pos.add(m_vExplodePos);
}

void CExplosive::ExplodeWaveProcessObject(collide::rq_results& storage, CPhysicsShellHolder* l_pGO)
{
	Fvector	l_goPos;
	if (l_pGO->Visual())
		l_pGO->Center(l_goPos);
	else
		return; //��� ��������� ����� �������� ��� �� ������ �� �������� �� ������� ������ - ������� ����������

	float l_effect = ExplosionEffect(storage, this, l_pGO, m_vExplodePos, m_fBlastRadius);
	float l_impuls = m_fBlastHitImpulse * l_effect;
	float l_hit = m_fBlastHit * l_effect;

	if (l_impuls > .001f || l_hit > 0.001)
	{

		Fvector l_dir; l_dir.sub(l_goPos, m_vExplodePos);

		float rmag = _sqrt(m_fUpThrowFactor * m_fUpThrowFactor + 1.f + 2.f * m_fUpThrowFactor * l_dir.y);
		l_dir.y += m_fUpThrowFactor;
		//rmag -������ l_dir ����� l_dir.y += m_fUpThrowFactor, ������=_sqrt(l_dir^2+y^2+2.*(l_dir,y)),y=(0,m_fUpThrowFactor,0) (�� ����� ������ l_dir =1)
		l_dir.mul(1.f / rmag);//��������������
		NET_Packet		P;
		SHit	HS;
		HS.GenHeader(GE_HIT, l_pGO->ID());			//		cast_game_object()->u_EventGen		(P,GE_HIT,l_pGO->ID());
		HS.whoID = Initiator();						//		P.w_u16			(Initiator());
		HS.weaponID = cast_game_object()->ID();		//		P.w_u16			(cast_game_object()->ID());
		HS.dir = l_dir;								//		P.w_dir			(l_dir);
		HS.power = l_hit;							//		P.w_float		(l_hit);
		HS.p_in_bone_space = l_goPos;				//		P.w_vec3		(l_goPos);
		HS.impulse = l_impuls;						//		P.w_float		(l_impuls);
		HS.hit_type = (m_eHitTypeBlast);			//		P.w_u16			(u16(m_eHitTypeBlast));
		HS.boneID = 0;								//		P.w_s16			(0);
		HS.Write_Packet(P);
		cast_game_object()->u_EventSend(P);
	}
}

struct SRemovePred
{
	bool operator () (CGameObject* O)
	{
		return !!O->getDestroy();
	}
};
void CExplosive::ExplodeWaveProcess()
{
	BLASTED_OBJECTS_I		I = std::remove_if(m_blasted_objects.begin(), m_blasted_objects.end(), SRemovePred());
	m_blasted_objects.erase(I, m_blasted_objects.end());
	rq_storage.r_clear();
	u16						i = BLASTED_OBJ_PROCESSED_PER_FRAME;
	while (m_blasted_objects.size() && 0 != i) {
		ExplodeWaveProcessObject(rq_storage, m_blasted_objects.back());
		m_blasted_objects.pop_back();
		--i;
	}
}

void CExplosive::GetExplosionBox(Fvector& size)
{
	size.set(m_vExplodeSize);
}

void CExplosive::SetExplosionSize(const Fvector& new_size)
{
	m_vExplodeSize.set(new_size);
}

void CExplosive::ActivateExplosionBox(const Fvector& size, Fvector& in_out_pos)
{
	CPhysicsShellHolder* self_obj = smart_cast<CPhysicsShellHolder*>(cast_game_object());
	CPhysicsShell* self_shell = self_obj->PPhysicsShell();
	if (self_shell && self_shell->isActive())
		self_shell->DisableCollision();

	ActivateShapeExplosive(self_obj, size, m_vExplodeSize, in_out_pos);
	
	if (self_shell && self_shell->isActive())
		self_shell->EnableCollision();
}

void CExplosive::net_Relcase(CObject* O)
{
	if (GameID() == eGameIDSingle)
	{
		if (O->ID() == m_iCurrentParentID)
			m_iCurrentParentID = u16(-1);
	}

	BLASTED_OBJECTS_I I = std::find(m_blasted_objects.begin(), m_blasted_objects.end(), smart_cast<CPhysicsShellHolder*>(O));
	if (m_blasted_objects.end() != I)
	{
		m_blasted_objects.erase(I);
	}
}

u16	CExplosive::Initiator()
{
	u16 initiator = CurrentParentID();
	if (initiator == u16(-1))initiator = cast_game_object()->ID();
	return initiator;
}

void CExplosive::UpdateExplosionParticles()
{
	if (!m_bDynamicParticles || m_pExpParticle == NULL || !m_pExpParticle->IsPlaying())
		return;

	CGameObject* GO = cast_game_object();
	if (!GO)
		return;

	Fmatrix ParticleMatrix = m_pExpParticle->XFORM();
	Fvector Vel;
	Vel.sub(GO->Position(), ParticleMatrix.c);
	ParticleMatrix.c.set(GO->Position());
	m_pExpParticle->UpdateParent(ParticleMatrix, Vel);
}

bool CExplosive::Useful() const
{
	return m_explosion_flags.flags == 0;
}