#include "stdafx.h"
#include "missile.h"
//.#include "WeaponHUD.h"
#include "../xrphysics/PhysicsShell.h"
#include "actor.h"
#include "../xrEngine/CameraBase.h"
#include "xrserver_objects_alife.h"
#include "ActorEffector.h"
#include "level.h"
#include "xr_level_controller.h"
#include "../Include/xrRender/Kinematics.h"
#include "ai_object_location.h"
#include "../xrphysics/ExtendedGeom.h"
#include "../xrphysics/MathUtils.h"
#include "characterphysicssupport.h"
#include "inventory.h"
#include "../xrEngine/IGame_Persistent.h"
#ifdef DEBUG
#	include "phdebug.h"
#endif


#define PLAYING_ANIM_TIME 10000

#include "ui/UIProgressShape.h"
#include "ui/UIXmlInit.h"
#include "physicsshellholder.h"
#include <Grenade.h>
#include <Bolt.h>


// void MsgSync(LPCSTR format, ...)
// {
// 	va_list		mark;
// 	string2048	buf;
// 	va_start(mark, format);
// 	int sz = _vsnprintf(buf, sizeof(buf) - 1, format, mark); buf[sizeof(buf) - 1] = 0;
// 	va_end(mark);
// 
// 	if (OnServer())
// 	{
// 		NET_Packet P;
// 		P.w_begin(M_MESSAGE_TEXT);
// 		P.w_stringZ(buf);
// 		Level().Server->SendBroadcast(BroadcastCID, P, net_flags(true, true));
// 	}
// 	else
// 	{
// 		if (sz)
// 			Log(buf);
// 	}
// }


CUIProgressShape* g_MissileForceShape = NULL;

void create_force_progress()
{
	VERIFY(!g_MissileForceShape);
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "grenade.xml");


	CUIXmlInit						xml_init;
	g_MissileForceShape = xr_new<CUIProgressShape>();
	xml_init.InitProgressShape(uiXml, "progress", 0, g_MissileForceShape);
}

CMissile::CMissile(void)
{
	m_dwStateTime = 0;
}

CMissile::~CMissile(void)
{
}

void CMissile::reinit()
{
	inherited::reinit();
	m_throw = false;
	m_constpower = false;
	m_fThrowForce = 0;
	m_dwDestroyTime = 0xffffffff;
	SetPending(FALSE);
	m_fake_missile = NULL;
	SetState(eHidden);
}

void CMissile::Load(LPCSTR section)
{
	inherited::Load(section);

	m_fMinForce = pSettings->r_float(section, "force_min");
	m_fConstForce = pSettings->r_float(section, "force_const");
	m_fMaxForce = pSettings->r_float(section, "force_max");
	m_fForceGrowSpeed = pSettings->r_float(section, "force_grow_speed");

	m_dwDestroyTimeMax = pSettings->r_u32(section, "destroy_time");

	m_vThrowPoint = pSettings->r_fvector3(section, "throw_point");
	m_vThrowDir = pSettings->r_fvector3(section, "throw_dir");

	m_ef_weapon_type = READ_IF_EXISTS(pSettings, r_u32, section, "ef_weapon_type", u32(-1));
}

BOOL CMissile::net_Spawn(CSE_Abstract* DC)
{
	BOOL l_res = inherited::net_Spawn(DC);

	dwXF_Frame = 0xffffffff;

	m_throw_direction.set(0.0f, 1.0f, 0.0f);
	m_throw_matrix.identity();

	return l_res;
}

void CMissile::net_Destroy()
{
	inherited::net_Destroy();
	m_fake_missile = 0;
	m_dwStateTime = 0;
}


void CMissile::OnActiveItem()
{
	SwitchState(eShowing);
	inherited::OnActiveItem();
	SetState(eIdle);
	SetNextState(eIdle);
}

void CMissile::OnHiddenItem()
{
#ifdef 	USE_CLIENT_SIDE_WEAPONS
	SwitchState(eHiding);
#else 
	if (IsGameTypeSingle())
		SwitchState(eHiding);
	else
		SwitchState(eHidden);
#endif 

	inherited::OnHiddenItem();
	SetState(eHidden);
	SetNextState(eHidden);
}
 
void CMissile::spawn_fake_missile()
{
	if (OnClient())
	{
		NET_Packet P;
		Game().u_EventGen(P, GE_MISSILE_SPAWN, ID() );
		Game().u_EventSend(P);
		return;
	}
	else
	{
		if (!getDestroy())
		{
			CSE_Abstract* object = Level().spawn_item(
				*cNameSect(),
				Position(),
				ai_location().level_vertex_id(),
				ID(),
				true
			);

			CSE_ALifeObject* alife_object = smart_cast<CSE_ALifeObject*>(object);
 			alife_object->m_flags.set(CSE_ALifeObject::flCanSave, FALSE);
 			NET_Packet			P;
			object->Spawn_Write(P, TRUE);
			Level().Send(P, net_flags(TRUE));
			F_entity_Destroy(object);
		}
	}	 
}

// Physic States
void CMissile::PH_A_CrPr()
{
	if (m_just_after_spawn)
	{
		CPhysicsShellHolder& obj = CInventoryItem::object();
		VERIFY(obj.Visual());
		IKinematics* K = obj.Visual()->dcast_PKinematics();
		VERIFY(K);
		if (!obj.PPhysicsShell())
		{
			Msg("! ERROR: PhysicsShell is NULL, object [%s][%d]", obj.cName().c_str(), obj.ID());
			return;
		}
		if (!obj.PPhysicsShell()->isFullActive())
		{
			K->CalculateBones_Invalidate();
			K->CalculateBones(TRUE);
		}
		obj.PPhysicsShell()->GetGlobalTransformDynamic(&obj.XFORM());
		K->CalculateBones_Invalidate();
		K->CalculateBones(TRUE);
		obj.spatial_move();
		m_just_after_spawn = false;
	}
}

void CMissile::OnH_A_Chield()
{
	inherited::OnH_A_Chield();
}
 
void CMissile::OnH_B_Independent(bool just_before_destroy)
{
	inherited::OnH_B_Independent(just_before_destroy);

	if (!just_before_destroy)
	{
		VERIFY(PPhysicsShell());
		PPhysicsShell()->SetAirResistance(0.f, 0.f);
		PPhysicsShell()->set_DynamicScales(1.f, 1.f);

		if (GetState() == eThrow)
		{
 			Throw();
		}
	}
  
	if (!m_dwDestroyTime && Local())
	{
		DestroyObject();
		return;
	}
}

extern u32 hud_adj_mode;

// Обновляторы
void CMissile::UpdateCL()
{ 
	m_dwStateTime += Device.dwTimeDelta;

	inherited::UpdateCL();

	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (pActor && !pActor->AnyMove() && this == pActor->inventory().ActiveItem())
	{
		if (hud_adj_mode == 0 && GetState() == eIdle && (Device.dwTimeGlobal - m_dw_curr_substate_time > 20000))
		{
			SwitchState(eBore);
			ResetSubStateTime();
		}
	}


	if (GetState() == eReady)
	{
		if (m_throw)
		{
			SwitchState(eThrow);
		}
		else
		{
			CActor* actor = smart_cast<CActor*>(H_Parent());
			if (actor)
			{
				m_fThrowForce += (m_fForceGrowSpeed * Device.dwTimeDelta) * .001f;
				clamp(m_fThrowForce, m_fMinForce, m_fMaxForce);
			}
		}
	}

}

void CMissile::shedule_Update(u32 dt)
{
	inherited::shedule_Update(dt);
	if (!H_Parent() && getVisible() && m_pPhysicsShell)
	{
		if (m_dwDestroyTime <= Level().timeServer())
		{
			m_dwDestroyTime = 0xffffffff;
			VERIFY(!m_pInventory);
			Destroy();
			return;
		}
	}
}

// Выборка анимации
void CMissile::State(u32 state)
{
	switch (GetState())
	{
		case eShowing:
		{
			SetPending(TRUE);
			PlayHUDMotion("anm_show", FALSE, this, GetState());
		} break;

		case eIdle:
		{
			SetPending(FALSE);
			PlayAnimIdle();
		} break;

		case eHiding:
		{
			if (H_Parent())
			{
				SetPending(TRUE);
				PlayHUDMotion("anm_hide", TRUE, this, GetState());
			}
		} break;

		case eHidden:
		{
			StopCurrentAnimWithoutCallback();

			if (H_Parent())
			{
				setVisible(FALSE);
				setEnabled(FALSE);
			};
			SetPending(FALSE);
		} break;

		case eThrowStart:
		{
			SetPending(TRUE);
			m_fThrowForce = m_fMinForce;
			PlayHUDMotion("anm_throw_begin", TRUE, this, GetState());
		} break;

		case eReady:
		{
			PlayHUDMotion("anm_throw_idle", TRUE, this, GetState());
		} break;

		case eThrow:
		{
			SetPending(TRUE);
			m_throw = false;
			PlayHUDMotion("anm_throw", TRUE, this, GetState());
		} break;

		case eThrowEnd:
		{
			SwitchState(eShowing);
		} break;
	}
}

void CMissile::OnStateSwitch(u32 S)
{
	m_dwStateTime = 0;
	inherited::OnStateSwitch(S);
	State(S);
}

// Действия при окончании анимации
void CMissile::OnAnimationEnd(u32 state)
{
	switch (state)
	{
		case eHiding:
		{
			setVisible(FALSE);
			SwitchState(eHidden);
		} break;
		case eShowing:
		{
			setVisible(TRUE);
			SwitchState(eIdle);
		} break;
		case eThrowStart:
		{
			if (!m_fake_missile && !smart_cast<CMissile*>(H_Parent()))
				spawn_fake_missile();
	
			if (m_throw)
				SwitchState(eThrow);
			else
				SwitchState(eReady);
		} break;
		case eThrow:
		{
			SwitchState(eThrowEnd);
		} break;
		case eThrowEnd:
		{
			SwitchState(eShowing);
		} break;
		default:
			inherited::OnAnimationEnd(state);
	}
}
 
void CMissile::UpdatePosition(const Fmatrix& trans)
{
	XFORM().mul(trans, offset());
}

// Update Мировой модели когда у нас в слоте перед броском
void CMissile::UpdateXForm()
{
	if (Device.dwFrame != dwXF_Frame)
	{
		dwXF_Frame = Device.dwFrame;

		if (0 == H_Parent())	
			return;

		// Get access to entity and its visual
		CEntityAlive* E = smart_cast<CEntityAlive*>(H_Parent());

		if (!E)			
			return;

		const CInventoryOwner* parent = smart_cast<const CInventoryOwner*>(E);
		if (parent && parent->use_simplified_visual())
			return;

		if (parent->attached(this))
			return;

 		IKinematics* V = smart_cast<IKinematics*>	(E->Visual());
 
		// Get matrices
		int					boneL = -1, boneR = -1, boneR2 = -1;
		E->g_WeaponBones(boneL, boneR, boneR2);
		if (boneR == -1)	
			return;
		boneL = boneR2;

		V->CalculateBones();
		Fmatrix& mL = V->LL_GetTransform(u16(boneL));
		Fmatrix& mR = V->LL_GetTransform(u16(boneR));

		// Calculate
		Fmatrix				mRes;
		Fvector				R, D, N;
		D.sub(mL.c, mR.c);	D.normalize_safe();
		R.crossproduct(mR.j, D);		R.normalize_safe();
		N.crossproduct(D, R);			N.normalize_safe();
		mRes.set(R, N, D, mR.c);
		mRes.mulA_43(E->XFORM());
		UpdatePosition(mRes);
	}
}


void CMissile::OnMotionMark(u32 state, const motion_marks& M)
{
	inherited::OnMotionMark(state, M);
	if (state == eThrow && !m_throw)
	{
 
		CInventoryOwner* invOwner = smart_cast<CInventoryOwner*>(H_Parent());
		CActor* pActor = smart_cast<CActor*>(invOwner);
 		if (pActor && pActor == Level().CurrentControlEntity() || !pActor && OnServer())
 		{
			Throw();
		}

	}
}
 
// Устанавлюем позици куда бросаем
void CMissile::setup_throw_params()
{
	CEntity* entity = smart_cast<CEntity*>(H_Parent());
 	CInventoryOwner* inventory_owner = smart_cast<CInventoryOwner*>(H_Parent());

	Fvector	FirePos, FireDir;
	if (this == inventory_owner->inventory().ActiveItem())
	{
		CInventoryOwner* io = smart_cast<CInventoryOwner*>(H_Parent());
		entity->g_fireParams(this, FirePos, FireDir);
	}
	else
	{
		FirePos = XFORM().c;
		FireDir = XFORM().k;
	}

	Fmatrix	TransformMatrix;
	TransformMatrix.identity();
	TransformMatrix.k.set(FireDir);

	Fvector::generate_orthonormal_basis(TransformMatrix.k, TransformMatrix.j, TransformMatrix.i);
	TransformMatrix.c.set(FirePos);
	
	Fmatrix m_value_throw_matrix; 
	m_value_throw_matrix.set(TransformMatrix);
	
	Fvector m_value_throw_direction;
	m_value_throw_direction.set(TransformMatrix.k);

 
	// 
	float ThrowForces = 0.01f;
 	if (inventory_owner->use_default_throw_force())
 		ThrowForces = m_constpower ? m_fConstForce : m_fThrowForce;
	else
 		ThrowForces = inventory_owner->missile_throw_force();

	// m_fThrowForce = m_fMinForce;

	NET_Packet packet;
	Game().u_EventGen(packet, GE_MISSILE_THROW, ID());
	packet.w_matrix(m_value_throw_matrix);
	packet.w_vec3(m_value_throw_direction);
	packet.w_float(ThrowForces);
 	Game().u_EventSend(packet);
}


void CMissile::Throw()
{
	// MsgSync("Throw[%s]", cName().c_str());
 	setup_throw_params();
}

void CMissile::OnEvent(NET_Packet& P, u16 type)
{
	if (Level().CurrentControlEntity() != H_Parent())
		inherited::OnEvent(P, type);
 
	u16						id;
	switch (type)
	{
		case GE_OWNERSHIP_TAKE: 
		{
			P.r_u16(id);
			CMissile* missile = smart_cast<CMissile*>(Level().Objects.net_Find(id));
			m_fake_missile = missile;
			missile->H_SetParent(this);
			missile->Position().set(Position());
			break;
		}

		case GE_OWNERSHIP_REJECT: 
		{

			P.r_u16(id);
			bool IsFakeMissile = false;
			if (m_fake_missile && (id == m_fake_missile->ID()))
			{
				m_fake_missile = NULL;
				IsFakeMissile = true;
			}

			Msg("Reject ID: %u", id);

			CMissile* missile = smart_cast<CMissile*>(Level().Objects.net_Find(id));
			if (!missile)
 				break;
 			missile->H_SetParent(0, !P.r_eof() && P.r_u8());
			if (IsFakeMissile && OnClient())
				missile->set_destroy_time(m_dwDestroyTimeMax);
			break;
		}

		case GE_MISSILE_SPAWN:
		{
			if (OnServer())
			{
				spawn_fake_missile();
			}
		}break;

		case GE_MISSILE_THROW:
		{
			Fmatrix throw_matrix;
			P.r_matrix(throw_matrix);
			Fvector throw_direction;
			P.r_vec3(throw_direction);
			float ThrowForce;
			P.r_float(ThrowForce);

			CGrenade* pGrenade = smart_cast<CGrenade*>(m_fake_missile);
			if (pGrenade)
			{
				// MsgSync("Throw Grenade [%u]", ID());
				pGrenade->set_destroy_time(m_dwDestroyTimeMax);
				pGrenade->SetInitiator(H_Parent()->ID()); //установить ID того кто кинул гранату
				pGrenade->m_thrown = true;
				 
				m_fake_missile->m_throw_direction = throw_direction;
				m_fake_missile->m_throw_matrix = throw_matrix;
				m_fake_missile->m_fThrowForce = ThrowForce;

				m_throw_direction = throw_direction;
				m_throw_matrix = throw_matrix;
				m_fThrowForce = ThrowForce;
				// Setup Direction to missile
				 
				if (OnServer() && H_Parent())
				{
					NET_Packet P;
					u_EventGen(P, GE_OWNERSHIP_REJECT, ID());
					P.w_u16(u16(m_fake_missile->ID()));
					u_EventSend(P);
				}

				m_fake_missile->processing_activate();//@sliph

				if (OnServer())
				{
					CGrenade* pGrenade = smart_cast<CGrenade*>(this);
					if (pGrenade)
					{
						pGrenade->PutNextToSlot();
						pGrenade->DestroyObject();
					}
				}
			}

			CBolt* bolt = smart_cast<CBolt*>(m_fake_missile);
			if (bolt)
			{
				bolt->set_destroy_time(m_dwDestroyTimeMax);
				bolt->SetInitiator(H_Parent()->ID()); //установить ID того кто кинул гранату
 
				m_fake_missile->m_throw_direction = throw_direction;
				m_fake_missile->m_throw_matrix = throw_matrix;
				m_fake_missile->m_fThrowForce = ThrowForce;

				m_throw_direction = throw_direction;
				m_throw_matrix = throw_matrix;
				m_fThrowForce = ThrowForce;

				if (OnServer() && H_Parent())
				{
					NET_Packet P;
					u_EventGen(P, GE_OWNERSHIP_REJECT, ID());
					P.w_u16(u16(m_fake_missile->ID()));
					u_EventSend(P);
				}

				m_fake_missile->processing_activate();//@sliph
			}


		}break;
	}
}

void CMissile::Destroy()
{
	if (Local())
		DestroyObject();
}
 
bool CMissile::Action(u16 cmd, u32 flags)
{
	if (inherited::Action(cmd, flags))
		return true;

	switch (cmd)
	{
		case kWPN_FIRE:
		{
			m_constpower = true;
			if (flags & CMD_START)
			{
				if (GetState() == eIdle)
				{
					m_throw = true;
					SwitchState(eThrowStart);
				}
			}
			return true;
		}break;

		case kWPN_ZOOM:
		{
			m_constpower = false;
			if (flags & CMD_START)
			{
				m_throw = false;
				if (GetState() == eIdle)
					SwitchState(eThrowStart);
				else
				if (GetState() == eReady)
				{
					m_throw = true;
				}

			}
			else
			if (GetState() == eReady || GetState() == eThrowStart || GetState() == eIdle)
			{
				m_throw = true;
				if (GetState() == eReady)
					SwitchState(eThrow);
			}
			return true;
		}break;
	}
	return false;
}

void CMissile::UpdateFireDependencies_internal()
{
	if (0 == H_Parent())		
		return;

	if (Device.dwFrame != dwFP_Frame) 
	{
		dwFP_Frame = Device.dwFrame;

		UpdateXForm();

		if (GetHUDmode() && !IsHidden())
		{
			R_ASSERT(0);  
			//implement this!
		}
		else 
		{
			// 3rd person
			Fmatrix& parent = H_Parent()->XFORM();
			m_throw_direction.set(m_vThrowDir);
			parent.transform_dir(m_throw_direction);
		}
	}
}

void CMissile::activate_physic_shell()
{
	if (!smart_cast<CMissile*>(H_Parent()))
	{
		inherited::activate_physic_shell();
		if (m_pPhysicsShell && m_pPhysicsShell->isActive() && !IsGameTypeSingle())
		{
			m_pPhysicsShell->add_ObjectContactCallback(ExitContactCallback);
			m_pPhysicsShell->set_CallbackData(smart_cast<CPhysicsShellHolder*>(H_Root()));
		}
		return;
	}

	Fvector				l_vel;
	l_vel.set(m_throw_direction);
	l_vel.normalize_safe();
	l_vel.mul(m_fThrowForce);

	Fvector				a_vel;
	CInventoryOwner* inventory_owner = smart_cast<CInventoryOwner*>(H_Root());
	if (inventory_owner && inventory_owner->use_throw_randomness()) {
		float			fi, teta, r;
		fi = ::Random.randF(0.f, 2.f * M_PI);
		teta = ::Random.randF(0.f, M_PI);
		r = ::Random.randF(2.f * M_PI, 3.f * M_PI);
		float			rxy = r * _sin(teta);
		a_vel.set(rxy * _cos(fi), rxy * _sin(fi), r * _cos(teta));
	}
	else
		a_vel.set(0.f, 0.f, 0.f);

	XFORM().set(m_throw_matrix);

	CEntityAlive* entity_alive = smart_cast<CEntityAlive*>(H_Root());
	if (entity_alive && entity_alive->character_physics_support()) {
		Fvector			parent_vel;
		entity_alive->character_physics_support()->movement()->GetCharacterVelocity(parent_vel);
		l_vel.add(parent_vel);
	}

	R_ASSERT(!m_pPhysicsShell);
	create_physic_shell();
	m_pPhysicsShell->Activate(m_throw_matrix, l_vel, a_vel);
	
	//	m_pPhysicsShell->AddTracedGeom		();
	m_pPhysicsShell->SetAllGeomTraced();
	m_pPhysicsShell->add_ObjectContactCallback(ExitContactCallback);
	m_pPhysicsShell->set_CallbackData(smart_cast<CPhysicsShellHolder*>(entity_alive));

	//	m_pPhysicsShell->remove_ObjectContactCallback	(ExitContactCallback);
	m_pPhysicsShell->SetAirResistance(0.f, 0.f);
	m_pPhysicsShell->set_DynamicScales(1.f, 1.f);

	IKinematics* kinematics = smart_cast<IKinematics*>(Visual());
	VERIFY(kinematics);
	kinematics->CalculateBones_Invalidate();
	kinematics->CalculateBones(TRUE);
}

void CMissile::net_Relcase(CObject* O)
{
	inherited::net_Relcase(O);
	if (PPhysicsShell() && PPhysicsShell()->isActive())
	{
		if (O == smart_cast<CObject*>((CPhysicsShellHolder*)PPhysicsShell()->get_CallbackData()))
		{
			PPhysicsShell()->remove_ObjectContactCallback(ExitContactCallback);
			PPhysicsShell()->set_CallbackData(NULL);
		}
	}

}

void CMissile::create_physic_shell()
{
	//create_box2sphere_physic_shell();
	CInventoryItemObject::CreatePhysicsShell();
}

void CMissile::setup_physic_shell()
{
	R_ASSERT(!m_pPhysicsShell);
	create_physic_shell();
	m_pPhysicsShell->Activate(XFORM(), 0, XFORM());//,true 
	IKinematics* kinematics = smart_cast<IKinematics*>(Visual());
	R_ASSERT(kinematics);
	kinematics->CalculateBones_Invalidate();
	kinematics->CalculateBones(TRUE);
}

u32	 CMissile::ef_weapon_type() const
{
	VERIFY(m_ef_weapon_type != u32(-1));
	return	(m_ef_weapon_type);
}
 
bool CMissile::render_item_ui_query()
{
	bool b_is_active_item = m_pInventory->ActiveItem() == this;
	return b_is_active_item && (GetState() == eReady) && !m_throw && smart_cast<CActor*>(H_Parent());
}

void CMissile::render_item_ui()
{
	CActor* actor = smart_cast<CActor*>(H_Parent());
	R_ASSERT(actor);

	if (!g_MissileForceShape)
		create_force_progress();
	float k = (m_fThrowForce - m_fMinForce) / (m_fMaxForce - m_fMinForce);
	g_MissileForceShape->SetPos(k);
	g_MissileForceShape->Draw();
}

void CMissile::ExitContactCallback(bool& do_colide, bool bo1, dContact& c, SGameMtl* /*material_1*/, SGameMtl* /*material_2*/)
{
	dxGeomUserData* gd1 = NULL, * gd2 = NULL;
	if (bo1)
	{
		gd1 = PHRetrieveGeomUserData(c.geom.g1);
		gd2 = PHRetrieveGeomUserData(c.geom.g2);
	}
	else
	{
		gd2 = PHRetrieveGeomUserData(c.geom.g1);
		gd1 = PHRetrieveGeomUserData(c.geom.g2);
	}
	if (gd1 && gd2 && (CPhysicsShellHolder*)gd1->callback_data == gd2->ph_ref_object)
		do_colide = false;
}

bool CMissile::GetBriefInfo(II_BriefInfo& info)
{
	info.clear();
	info.name._set(m_nameShort);
	return true;
}

