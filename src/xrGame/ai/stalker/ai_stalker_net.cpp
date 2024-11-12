#include "stdafx.h"
#include "pch_script.h"
#include "ai_stalker.h"

#include "../../ai_object_location.h"
#include "../../game_graph.h"
#include "../../ai_space.h"
#include "../../CharacterPhysicsSupport.h"
#include "../../stalker_movement_manager_smart_cover.h"
#include "../../inventory.h"
#include "../../stalker_animation_manager.h"
#include "../../Weapon.h"

#include "../../CharacterPhysicsSupport.h"
#include "../../../xrphysics/iPHWorld.h"
#include "../../PHMovementControl.h"

#include "net_physics_state.h"
#include "../xrphysics/phvalide.h"

#include "ai_stalker_net_state.h"
#include "Actor.h"
#include "HUDManager.h"

#include "movement_manager.h"

#include "../../movement_manager_space.h"
#include "../../patrol_path_manager.h"
#include "../../level_path_manager.h"
#include "../../game_path_manager.h"
#include "../../detail_path_manager.h"


void MsgMatrix(LPCSTR prefix, Fmatrix matr)
{
	Msg("XFORM: %s", prefix);
	Msg("[%f, %f, %f, %f]", matr._11, matr._12, matr._13, matr._14);
	Msg("[%f, %f, %f, %f]", matr._21, matr._22, matr._23, matr._24);
	Msg("[%f, %f, %f, %f]", matr._31, matr._32, matr._33, matr._34);
	Msg("[%f, %f, %f, %f]", matr._41, matr._42, matr._33, matr._44);
}

#include "..\xrEngine\motion.h"
 
void CAI_Stalker::anim_sync(IKinematicsAnimated* skeleton_animated, CBlend* blend_1, CBlend* blend_2)
{
	if(!blend_1 || !blend_1->motionID.valid())
		return;

	CMotionDef* motion0 = skeleton_animated->LL_GetMotionDef(blend_1->motionID);
	 
	if (!(motion0->flags & esmSyncPart))
		return;

	if (!blend_2 || blend_2->motionID.valid())
		return;

	CMotionDef* motion1 = skeleton_animated->LL_GetMotionDef(blend_2->motionID);
	
	if (!(motion1->flags & esmSyncPart))
		return;

	if (blend_1->timeTotal != blend_2->timeTotal)
		return;

	blend_1->timeCurrent = blend_2->timeCurrent;
}

extern int g_cl_InterpolationType; 
 
void CAI_Stalker::net_Save(NET_Packet& P)
{
	inherited::net_Save(P);

	m_pPhysics_support->in_NetSave(P);
}

BOOL CAI_Stalker::net_SaveRelevant()
{
	return (inherited::net_SaveRelevant() /* || BOOL(PPhysicsShell() != NULL) */);
}

void CAI_Stalker::net_Export(NET_Packet& P)
{
	R_ASSERT(Local());
	if (IsGameTypeSingle()) 
	{
		// export last known packet
		R_ASSERT(!NET.empty());
		net_update& N = NET.back();
		//	P.w_float						(inventory().TotalWeight());
		//	P.w_u32							(m_dwMoney);

		P.w_float(GetfHealth());

		P.w_u32(N.dwTimeStamp);
		P.w_u8(0);
		P.w_vec3(N.p_pos);
		P.w_float /*w_angle8*/(N.o_model);
		P.w_float /*w_angle8*/(N.o_torso.yaw);
		P.w_float /*w_angle8*/(N.o_torso.pitch);
		P.w_float /*w_angle8*/(N.o_torso.roll);
		P.w_u8(u8(g_Team()));
		P.w_u8(u8(g_Squad()));
		P.w_u8(u8(g_Group()));


		float					f1 = 0;
		GameGraph::_GRAPH_ID		l_game_vertex_id = ai_location().game_vertex_id();
		P.w(&l_game_vertex_id, sizeof(l_game_vertex_id));
		P.w(&l_game_vertex_id, sizeof(l_game_vertex_id));
		//	P.w						(&f1,						sizeof(f1));
		//	P.w						(&f1,						sizeof(f1));
		if (ai().game_graph().valid_vertex_id(l_game_vertex_id)) {
			f1 = Position().distance_to(ai().game_graph().vertex(l_game_vertex_id)->level_point());
			P.w(&f1, sizeof(f1));
			f1 = Position().distance_to(ai().game_graph().vertex(l_game_vertex_id)->level_point());
			P.w(&f1, sizeof(f1));
		}
		else {
			P.w(&f1, sizeof(f1));
			P.w(&f1, sizeof(f1));
		}

		P.w_stringZ(m_sStartDialog);
	}
	else
	{
		u16 u_active_slot, u_active_item;
		float health = GetfHealth();

 		// inventory
		CWeapon* weapon = smart_cast<CWeapon*>(inventory().ActiveItem());

		if (weapon && !weapon->strapped_mode())
		{
			u_active_slot = inventory().ActiveItem()->CurrSlot();
			PIItem item = inventory().ItemFromSlot(u_active_slot);
			u_active_item = item->object_id();
		}
		else
		{
			u_active_slot = NO_ACTIVE_SLOT;
			u_active_item = 0;
		}

		ai_stalker_net_state state;

		//Position Interpolation
 		state.fill_position(PHGetSyncItem(0));
 
		//Body Rotation
		state.torso_yaw = movement().m_body.current.yaw;
		state.head_yaw = movement().m_head.current.yaw;

		state.torso_pitch = movement().m_body.current.pitch;
		state.head_pitch = movement().m_head.current.pitch;
 
		//GLOBAL PARAMS
		state.u_active_item = u_active_item;
		state.u_health = health;
		state.u_active_slot = u_active_slot;
		state.fv_position = Position();
   
		//Animation
		IKinematicsAnimated* ka = Visual()->dcast_PKinematicsAnimated();

		if (ka)
		{
			state.legs_anim.id = legs_motion;
			state.torso_anim.id = torso_motion;
			state.head_anim.id = head_motion;

			state.legs_anim.num = legs_num;
			state.torso_anim.num = torso_num;
			state.head_anim.num = head_num;
 
			state.legs_anim.loop = legs_loop;
			state.torso_anim.loop = torso_loop;
			state.head_anim.loop = head_loop;

			//state.legs_anim.anim_ctrl = legs_anim_ctrl;
			//state.torso_anim.anim_ctrl = torso_anim_ctrl;
			//state.head_anim.anim_ctrl = head_anim_ctrl;

			//state.legs_anim.pos = blend_legs->timeCurrent;
			//state.torso_anim.pos = blend_torso->timeCurrent;
			//state.head_anim.pos = blend_head->timeCurrent;
		}

		state.state_write(P);
	}	 	
	
	
	/*
	char* enum_to_str[4] = {"ePathTypeGamePath", "ePathTypeLevelPath", "ePathTypePatrolPath", "ePathTypeNoPath"};
	if (Device.dwFrame % 30 == 0 && this->raycastNOW() && false)
	{
		Msg("PathType: %s", enum_to_str[movement().path_type()]);
		if (movement().path_type() == 1 && !movement().level_path().path().empty())
		{
			Msg("PathSize: %d", movement().level_path().path().size());
			
			Msg("PathDEST %d", movement().level_path().dest_vertex_id());

			u32 vert = movement().level_path().dest_vertex_id();
			Fvector vec = ai().level_graph().vertex_position(vert);
			Msg("DEST POS [%f][%f][%f]", vec.x, vec.y, vec.z);
			//Msg("PathBack: %d", movement().level_path().path().back());
			//Msg("PathFront: %d", movement().level_path().path().front());
			
			int i = 0;
			for (auto vertex : movement().level_path().path())
			{
				i++;
				Msg("ID[%d], vertex[%d]", i, vertex);
		 
				Fvector pos = ai().level_graph().vertex_position(vertex);
				Msg("POS [%f][%f][%f]", pos.x, pos.y, pos.z);
			}
		
		}

	}
	*/
 
}

void CAI_Stalker::net_Import(NET_Packet& P)
{
	R_ASSERT(Remote());
	if (IsGameTypeSingle())
	{
		net_update						N;

		u8 flags;

		P.r_float();
		set_money(P.r_u32(), false);

		float health;
		P.r_float(health);
		SetfHealth(health);
		//	fEntityHealth = health;

		P.r_u32(N.dwTimeStamp);
		P.r_u8(flags);
		P.r_vec3(N.p_pos);
		P.r_float /*r_angle8*/(N.o_model);
		P.r_float /*r_angle8*/(N.o_torso.yaw);
		P.r_float /*r_angle8*/(N.o_torso.pitch);
		P.r_float /*r_angle8*/(N.o_torso.roll);
		id_Team = P.r_u8();
		id_Squad = P.r_u8();
		id_Group = P.r_u8();


		GameGraph::_GRAPH_ID				graph_vertex_id = movement().game_dest_vertex_id();
		P.r(&graph_vertex_id, sizeof(GameGraph::_GRAPH_ID));
		graph_vertex_id = ai_location().game_vertex_id();
		P.r(&graph_vertex_id, sizeof(GameGraph::_GRAPH_ID));

		if (NET.empty() || (NET.back().dwTimeStamp < N.dwTimeStamp)) {
			NET.push_back(N);
			NET_WasInterpolating = TRUE;
		}

		P.r_float();
		P.r_float();

		P.r_stringZ(m_sStartDialog);

		setVisible(TRUE);
		setEnabled(TRUE);
	}
	else
	{
		ai_stalker_net_state state;

 		state.state_read(P);	
 
		SetfHealth(state.u_health);
		
		PIItem item = inventory().ItemFromSlot(state.u_active_slot);
		 
		if ( item && item->object_id() != state.u_active_item || !item)
		{
			CObject* obj = Level().Objects.net_Find(state.u_active_item);
			CInventoryItem* itemINV = smart_cast<CInventoryItem*>(obj);

			if (itemINV && itemINV->parent_id() == this->ID())
			{
				inventory().Slot(state.u_active_slot, itemINV, true, true);
			}
		}
		
		inventory().SetActiveSlot(state.u_active_slot);
	    
		if (state.phSyncFlag)
		{
			stalker_interpolation::net_update_A N_A;
			N_A.State.enabled = state.physics_state.physics_state_enabled;
			N_A.State.linear_vel = state.physics_state.physics_linear_velocity;
			N_A.State.position = state.physics_state.physics_position;

			N_A.o_torso.yaw = state.torso_yaw;
			N_A.o_torso.pitch = state.torso_pitch;
			N_A.head.yaw = state.head_yaw;
			N_A.head.pitch = state.head_pitch;
	 
			N_A.dwTimeStamp = state.physics_state.dwTimeStamp;

			// interpolation
			postprocess_packet(N_A);
		}
		else
		{
			Position().set(state.fv_position);

			NET_A.clear();
			//TODO: disable interpolation?
		}
 		   
		// Pavel: create structure for animation?
		if (g_Alive())
			ApplyAnimation(state);

		setVisible(TRUE);
		setEnabled(TRUE);
		
	}
}

#include "animation_movement_controller.h"
#include "../xrEngine/SkeletonMotions.h"

u16 MAX_BITS_COUNT = (u32(1) << 10);

void CAI_Stalker::ApplyAnimation(ai_stalker_net_state& state)
{
	IKinematicsAnimated* ka = smart_cast<IKinematicsAnimated*>(Visual());
	if (!ka)
		return;

	if (state.torso_anim.id.idx < MAX_BITS_COUNT)
	if (state.torso_anim.num != client_torso_num)
	{
		MotionID motion = state.torso_anim.id;
		blend_torso = ka->LL_PlayCycle(
			ka->LL_GetMotionDef(motion)->bone_or_part,
			state.torso_anim.id, 
			state.torso_anim.loop, 
			ka->LL_GetMotionDef(motion)->Accrue(),
			ka->LL_GetMotionDef(motion)->Falloff(),
			ka->LL_GetMotionDef(motion)->Speed(),
			ka->LL_GetMotionDef(motion)->StopAtEnd(),
			0, 0, 0
		);		
		client_torso_num = state.torso_anim.num;
	}

	if (state.head_anim.id.idx < MAX_BITS_COUNT)
	if (state.head_anim.num != client_head_num)
	{
		MotionID motion = state.head_anim.id;

		blend_head = ka->LL_PlayCycle(
			ka->LL_GetMotionDef(motion)->bone_or_part,
			motion,
			state.head_anim.loop, 
			ka->LL_GetMotionDef(motion)->Accrue(),
			ka->LL_GetMotionDef(motion)->Falloff(),
			ka->LL_GetMotionDef(motion)->Speed(),
			ka->LL_GetMotionDef(motion)->StopAtEnd(),
			0, 0, 0
		);
		client_head_num = state.head_anim.num;
	}
 
	if (state.legs_anim.id.idx < MAX_BITS_COUNT)
	if (state.legs_anim.num != client_legs_num)
	{
		MotionID motion = state.legs_anim.id;
		blend_legs = ka->LL_PlayCycle(
			ka->LL_GetMotionDef(motion)->bone_or_part,
			state.legs_anim.id,
			state.legs_anim.loop,
			ka->LL_GetMotionDef(motion)->Accrue(),
			ka->LL_GetMotionDef(motion)->Falloff(),
			ka->LL_GetMotionDef(motion)->Speed(),
			ka->LL_GetMotionDef(motion)->StopAtEnd(),
			0, 0, 0
		);

		if (blend_legs)
			CStepManager::on_animation_start(state.legs_anim.id, blend_legs);
		client_legs_num = state.legs_anim.num;
	}

 	/*	Синхра ног кривая с рывками анимки
	if (blend_legs && blend_torso && state.legs_anim.pos != 0)
	{
		blend_legs->timeCurrent *= blend_legs->timeTotal * state.legs_anim.pos;
		anim_sync(ka, blend_torso, blend_legs);
	}
	*/
}

void CAI_Stalker::postprocess_packet(stalker_interpolation::net_update_A &N_A)
{
	if (!NET_A.empty())
		N_A.dwTimeStamp = NET_A.back().dwTimeStamp;
	else
		N_A.dwTimeStamp = Level().timeServer();

	N_A.State.previous_position = N_A.State.position;
	N_A.State.previous_quaternion = N_A.State.quaternion;

	if (Local() && OnClient() || !g_Alive())
		return;

	if (!NET_A.empty() && N_A.dwTimeStamp < NET_A.back().dwTimeStamp) return;

	if (!NET_A.empty() && N_A.dwTimeStamp == NET_A.back().dwTimeStamp)
	{
		NET_A.back() = N_A;
	}
	else
	{
		VERIFY(valid_pos(N_A.State.position));
		NET_A.push_back(N_A);
		if (NET_A.size() > 5)
		{
			NET_A.pop_front();
		}
	};

	if (!NET_A.empty()) 
		m_bInterpolate = true;

	Level().AddObject_To_Objects4CrPr(this);
	CrPr_SetActivated(false);
	CrPr_SetActivationStep(0);
}

void CAI_Stalker::PH_B_CrPr()
{
	if (IsGameTypeSingle())
	{
		inherited::PH_B_CrPr();
		return;
	}

	if (CrPr_IsActivated()) return;
	if (CrPr_GetActivationStep() > physics_world()->StepsNum()) return;
	

	if (g_Alive())
	{
		CrPr_SetActivated(true);
			   
		stalker_interpolation::InterpData* pIStart = &IStart;
		pIStart->Pos = Position();
		pIStart->Vel = m_pPhysics_support->movement()->GetVelocity();

		pIStart->o_torso.yaw = angle_normalize(movement().m_body.current.yaw);
		pIStart->o_torso.pitch = angle_normalize(movement().m_body.current.pitch);
		//pIStart->o_torso.roll = angle_normalize(movement().m_body.current.roll);

		pIStart->head.pitch = angle_normalize(movement().m_head.current.pitch);
		pIStart->head.yaw = angle_normalize(movement().m_head.current.yaw);

		CPHSynchronize* pSyncObj = NULL;
		pSyncObj = PHGetSyncItem(0);
		if (!pSyncObj) return;
		pSyncObj->get_State(LastState);

		if (Local() && OnClient())
		{
			PHUnFreeze();
			pSyncObj->set_State(NET_A.back().State);
		}
		else
		{	  
			if (!NET_A.empty()) 
			{
				auto N_A = NET_A.back();
				NET_A_Last = N_A;

				if (!N_A.State.enabled)
				{
					pSyncObj->set_State(N_A.State);
				}
				else
				{
					PHUnFreeze();
					pSyncObj->set_State(N_A.State);
					Position().set(IStart.Pos);
				};
			}
		};
	}
	else
	{
		CrPr_SetActivated(true);
		PHUnFreeze();
	}
}

void CAI_Stalker::PH_I_CrPr()
{
	if (IsGameTypeSingle())
	{
		inherited::PH_I_CrPr();
		return;
	}

	if (!CrPr_IsActivated()) return;
	
	if (g_Alive())
	{
		CPHSynchronize* pSyncObj = NULL;
		pSyncObj = PHGetSyncItem(0);
		if (!pSyncObj) return;
		pSyncObj->get_State(RecalculatedState);	
	};
}

void CAI_Stalker::PH_A_CrPr()
{
	if (IsGameTypeSingle())
	{
		inherited::PH_A_CrPr();
		return;
	}

	if (!CrPr_IsActivated()) return;
	if (!g_Alive()) return;
	
	CPHSynchronize* pSyncObj = NULL;
	pSyncObj = PHGetSyncItem(0);
	if (!pSyncObj) return;

	pSyncObj->get_State(PredictedState);
	pSyncObj->set_State(RecalculatedState);

	if (!m_bInterpolate)
	{
		return;
	}

	CalculateInterpolationParams();
}
	  
void CAI_Stalker::CalculateInterpolationParams()
{
	CPHSynchronize* pSyncObj = NULL;
	pSyncObj = PHGetSyncItem(0);

	stalker_interpolation::InterpData* pIStart = &IStart;
	//stalker_interpolation::InterpData* pIRec = &IRec;
	stalker_interpolation::InterpData* pIEnd = &IEnd;

	//pIRec->Pos = RecalculatedState.position;
	//pIRec->Vel = RecalculatedState.linear_vel;
	//pIRec->o_torso = NET_A_Last.o_torso;
	//pIRec->head = NET_A_Last.head;

	pIEnd->Pos = PredictedState.position;
	pIEnd->Vel = PredictedState.linear_vel;
	pIEnd->o_torso = NET_A_Last.o_torso;
	pIEnd->head = NET_A_Last.head;

	Fvector SP0, SP1, SP2, SP3;
	Fvector HP0, HP1, HP2, HP3;

	SP0 = pIStart->Pos;
	HP0 = pIStart->Pos;

	if (m_bInInterpolation)
	{
		u32 CurTime = Level().timeServer();
		float factor = float(CurTime - m_dwIStartTime) / (m_dwIEndTime - m_dwIStartTime);
		if (factor > 1.0f) factor = 1.0f;

		float c = factor;
		for (u32 k = 0; k < 3; k++)
		{
			SP0[k] = c * (c*(c*SCoeff[k][0] + SCoeff[k][1]) + SCoeff[k][2]) + SCoeff[k][3];
			SP1[k] = (c*c*SCoeff[k][0] * 3 + c * SCoeff[k][1] * 2 + SCoeff[k][2]) / 3; //     3       !!!!

			HP0[k] = c * (c*(c*HCoeff[k][0] + HCoeff[k][1]) + HCoeff[k][2]) + HCoeff[k][3];
			HP1[k] = (c*c*HCoeff[k][0] * 3 + c * HCoeff[k][1] * 2 + HCoeff[k][2]) / 3; //     3       !!!!
		};

		SP1.add(SP0);
	}
	else
	{
		if (LastState.linear_vel.x == 0 && LastState.linear_vel.y == 0 && LastState.linear_vel.z == 0)
		{
			HP1.sub(RecalculatedState.position, RecalculatedState.previous_position);
		}
		else
		{
			HP1.sub(LastState.position, LastState.previous_position);
		};
		HP1.mul(1.0f / fixed_step);
		SP1.add(HP1, SP0);
	}

	HP2.sub(PredictedState.position, PredictedState.previous_position);
	HP2.mul(1.0f / fixed_step);
	SP2.sub(PredictedState.position, HP2);

	SP3.set(PredictedState.position);
	HP3.set(PredictedState.position);

	Fvector TotalPath;
	TotalPath.sub(SP3, SP0);
	float TotalLen = TotalPath.magnitude();

	SPHNetState	State0 = (NET_A.back()).State;
	SPHNetState	State1 = PredictedState;

	float lV0 = State0.linear_vel.magnitude();
	float lV1 = State1.linear_vel.magnitude();

	u32 ConstTime = u32((fixed_step - physics_world()->FrameTime()) * 1000) + Level().GetInterpolationSteps()*u32(fixed_step * 1000);

	m_dwIStartTime = m_dwILastUpdateTime;
	m_dwIEndTime = m_dwIStartTime + ConstTime;

	Fvector V0, V1;
	V0.set(HP1);
	V1.set(HP2);
	lV0 = V0.magnitude();
	lV1 = V1.magnitude();

	if (TotalLen != 0)
	{
		if (V0.x != 0 || V0.y != 0 || V0.z != 0)
		{
			if (lV0 > TotalLen / 3)
			{
				HP1.normalize();
				//				V0.normalize();
				//				V0.mul(TotalLen/3);
				HP1.normalize();
				HP1.mul(TotalLen / 3);
				SP1.add(HP1, SP0);
			}
		}

		if (V1.x != 0 || V1.y != 0 || V1.z != 0)
		{
			if (lV1 > TotalLen / 3)
			{
				//				V1.normalize();
				//				V1.mul(TotalLen/3);
				HP2.normalize();
				HP2.mul(TotalLen / 3);
				SP2.sub(SP3, HP2);
			};
		}
	};
	/////////////////////////////////////////////////////////////////////////////
	for (u32 i = 0; i < 3; i++)
	{
		SCoeff[i][0] = SP3[i] - 3 * SP2[i] + 3 * SP1[i] - SP0[i];
		SCoeff[i][1] = 3 * SP2[i] - 6 * SP1[i] + 3 * SP0[i];
		SCoeff[i][2] = 3 * SP1[i] - 3 * SP0[i];
		SCoeff[i][3] = SP0[i];

		HCoeff[i][0] = 2 * HP0[i] - 2 * HP3[i] + HP1[i] + HP2[i];
		HCoeff[i][1] = -3 * HP0[i] + 3 * HP3[i] - 2 * HP1[i] - HP2[i];
		HCoeff[i][2] = HP1[i];
		HCoeff[i][3] = HP0[i];
	};
	/////////////////////////////////////////////////////////////////////////////
	m_bInInterpolation = true;

	if (m_pPhysicsShell) m_pPhysicsShell->NetInterpolationModeON();
}

void CAI_Stalker::make_Interpolation()
{
	m_dwILastUpdateTime = Level().timeServer();

	if (g_Alive() && m_bInInterpolation)
	{
		u32 CurTime = m_dwILastUpdateTime;

		if (CurTime >= m_dwIEndTime)
		{
			m_bInInterpolation = false;

			CPHSynchronize* pSyncObj = NULL;
			pSyncObj = PHGetSyncItem(0);
			if (!pSyncObj) return;
			pSyncObj->set_State(PredictedState);
			VERIFY2(_valid(renderable.xform), *cName());
		}
		else
		{
			float factor = 0.0f;

			if (m_dwIEndTime != m_dwIStartTime)
				factor = float(CurTime - m_dwIStartTime) / (m_dwIEndTime - m_dwIStartTime);

			clamp(factor, 0.f, 1.0f);

			Fvector NewPos;
			NewPos.lerp(IStart.Pos, IEnd.Pos, factor);
 	 
			movement().m_body.current.pitch = angle_lerp(IStart.o_torso.pitch, IEnd.o_torso.pitch, factor);
 			movement().m_body.current.yaw	= angle_lerp(IStart.o_torso.yaw, IEnd.o_torso.yaw, factor);

			movement().m_head.current.pitch = angle_lerp(IStart.head.pitch, IEnd.head.pitch, factor);
 			movement().m_head.current.yaw	= angle_lerp(IStart.head.yaw, IEnd.head.yaw, factor);
			  
			for (u32 k = 0; k < 3; k++)
			{
				IPosL[k] = NewPos[k];
				IPosS[k] = factor * (factor*(factor*SCoeff[k][0] + SCoeff[k][1]) + SCoeff[k][2]) + SCoeff[k][3];
				IPosH[k] = factor * (factor*(factor*HCoeff[k][0] + HCoeff[k][1]) + HCoeff[k][2]) + HCoeff[k][3];
			};

			Fvector SpeedVector, ResPosition;
			switch (g_cl_InterpolationType)
			{
			case 0:
				{
					ResPosition.set(IPosL);
					SpeedVector.sub(IEnd.Pos, IStart.Pos);
					SpeedVector.div(float(m_dwIEndTime - m_dwIStartTime) / 1000.0f);
				}break;
			case 1:
				{
					for (int k = 0; k < 3; k++)
						SpeedVector[k] = (factor*factor*SCoeff[k][0] * 3 + factor * SCoeff[k][1] * 2 + SCoeff[k][2]) / 3; //     3       !!!!

					ResPosition.set(IPosS);
				}break;
			case 2:
				{
					for (int k = 0; k < 3; k++)
						SpeedVector[k] = (factor*factor*HCoeff[k][0] * 3 + factor * HCoeff[k][1] * 2 + HCoeff[k][2]);

					ResPosition.set(IPosH);
				}break;
			default:
				R_ASSERT2(0, "Unknown interpolation curve type!");
				break;
			}
			Position().set(ResPosition);

 			character_physics_support()->movement()->SetPosition(ResPosition); // we need it ?
			character_physics_support()->movement()->SetVelocity(SpeedVector);
 		};
	}
	else
	{
		m_bInInterpolation = false;
	};
};
