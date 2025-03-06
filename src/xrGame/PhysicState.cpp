#include "StdAfx.h"
#include "PhysicState.h"

#include "script_game_object.h"
#include "doors.h"

#include "GameObject.h"
#include "..\xrPhysics\PhysicsShell.h"
#include "PhysicObject.h"
#include "xrServer_Objects_ALife.h"

#include "Level.h"
#include "../xrServerEntities/PHNetState.h"
#include "../xrServerEntities/PHSynchronize.h"

#define XFORM_VPUSH_1(FORM) \
FORM._11, FORM._12, FORM._13, FORM._14 \

#define XFORM_VPUSH_2(FORM) \
FORM._21, FORM._22, FORM._23, FORM._24 \

#define XFORM_VPUSH_3(FORM) \
FORM._31, FORM._32, FORM._33, FORM._34 \

#define XFORM_VPUSH_4(FORM) \
FORM._41, FORM._42, FORM._43, FORM._44 \

#include "HUDManager.h"

void GetPlayerHUD_Selected(CGameFont* font)
{
	//font->OutSet(400, 150);
	//font->SetHeightI(0.015f);
	font->SetColor(color_rgba(128, 128, 255, 255));

	CObject* O = HUD().GetCurrentRayQuery().O;
	if (!O)
		return;

	font->OutNext("Object: %s", O->cName());
	font->OnRender();

	CPhysicObject* ph_object = smart_cast<CPhysicObject*>(O);
	if (!ph_object)
		return;

	CPhysicsShell* shell = ph_object->m_pPhysicsShell;
	if (!shell)
		return;

	font->OutNext("Active: %d", shell->isActive());
	// DOOOR STATE
	font->OutNext("Door: %d, DoorPSize: %d", ph_object->state_sync.DoorState, ph_object->state_sync.door_packet.B.count);

	font->OnRender();

	int count = shell->get_ElementsNumber();
	if (count == 0)
		return;

	for (auto SyncID = 0; SyncID < count; SyncID++)
	{
		SPHNetState state;
		shell->get_ElementSync(SyncID)->get_State(state);

		font->OutNext("--SYNC[%d]--", SyncID);
		font->OutNext("PH_Sync Pos[%.2f, %.2f, %.2f], Q[%.2f, %.2f, %.2f, %.2f] \n", VPUSH(state.position), VPUSH(state.quaternion), state.quaternion.w);
		font->OutNext("PH_Sync Force[%.2f, %.2f, %.2f], Torque[%.2f, %.2f, %.2f]  \n", VPUSH(state.force), VPUSH(state.torque));
		font->OutNext("PH_Sync Speed Linear[%.2f, %.2f, %.2f], Angular[%.2f, %.2f, %.2f]  \n", VPUSH(state.linear_vel), VPUSH(state.angular_vel));

		//font->OutNext("--XFORM[%d] --", SyncID);

		// Fmatrix M = shell->get_ElementByStoreOrder(SyncID)->mXFORM;
		// 
		// font->OutNext("1[%.3f,%.3f,%.3f,%.3f] \n", XFORM_VPUSH_1(M));
		// font->OutNext("2[%.3f,%.3f,%.3f,%.3f] \n", XFORM_VPUSH_2(M));
		// font->OutNext("3[%.3f,%.3f,%.3f,%.3f] \n", XFORM_VPUSH_3(M));
		// font->OutNext("4[%.3f,%.3f,%.3f,%.3f] \n", XFORM_VPUSH_4(M));
	}
}

// State

CPhysicObject* cast_phobject(void* ptr)
{
	return ((CPhysicObject*)ptr);
}

void CPhysicStorage::SyncPhysic::WritePacket(NET_Packet& Packet)
{
	enabled_forces.zero();
	enabled_forces.set(eEnable, enabled);
	enabled_forces.set(eAngular, !fis_zero(angular_vel.square_magnitude()));
	enabled_forces.set(eLinear, !fis_zero(linear_vel.square_magnitude()));

	Packet.w_u8(enabled_forces.get());

	if (enabled_forces.test(eLinear))
	{
		Packet.w_float_q8(linear_vel.x, -32, 32);
		Packet.w_float_q8(linear_vel.y, -32, 32);
		Packet.w_float_q8(linear_vel.z, -32, 32);
		//Packet.w_vec3(linear_vel);
	}

	if (enabled_forces.test(eAngular))
	{
		Packet.w_float_q8(angular_vel.x, -32, 32);
		Packet.w_float_q8(angular_vel.y, -32, 32);
		Packet.w_float_q8(angular_vel.z, -32, 32);
		//Packet.w_vec3(angular_vel);
	}

	Packet.w_float_q8(quaternion.x, -1, 1);
	Packet.w_float_q8(quaternion.y, -1, 1);
	Packet.w_float_q8(quaternion.z, -1, 1);
	Packet.w_float_q8(quaternion.w, -1, 1);

	Packet.w_vec3(position);

	// TODO
	//Packet.w_vec3(force);
	//Packet.w_vec3(torque);
}

void CPhysicStorage::SyncPhysic::ReadPacket(NET_Packet& Packet)
{
	enabled_forces.flags = Packet.r_u8();

	bool isEnabled = enabled_forces.test(eEnable);
	bool angular = enabled_forces.test(eAngular);
	bool lieanear = enabled_forces.test(eLinear);

	enabled = isEnabled;

	if (lieanear)
	{
		Packet.r_float_q8(linear_vel.x, -32, 32);
		Packet.r_float_q8(linear_vel.y, -32, 32);
		Packet.r_float_q8(linear_vel.z, -32, 32);
		//Packet.r_vec3(linear_vel);
	}
	else
	{
		linear_vel.set(0, 0, 0);
	}

	if (angular)
	{
		Packet.r_float_q8(angular_vel.x, -32, 32);
		Packet.r_float_q8(angular_vel.y, -32, 32);
		Packet.r_float_q8(angular_vel.z, -32, 32);
		//Packet.r_vec3(angular_vel);
	}
	else
	{
		angular_vel.set(0, 0, 0);
	}

	Packet.r_float_q8(quaternion.x, -1, 1);
	Packet.r_float_q8(quaternion.y, -1, 1);
	Packet.r_float_q8(quaternion.z, -1, 1);
	Packet.r_float_q8(quaternion.w, -1, 1);
	Packet.r_vec3(position);

	// TODO
	//Packet.r_vec3(force);
	//Packet.r_vec3(torque);

	force.set(0.f, 0.f, 0.f);
	torque.set(0.f, 0.f, 0.f);
}


void CPhysicStorage::WriteData(NET_Packet& P)
{
	// Выключено на сингле 
	P.w_u8(SyncActive);
	P.w_u8(DoorState);

	if (!SyncActive)
		return;

	// BONES
	P.w_u8(current_state.count);
	if (current_state.count > 0)
		current_state.State.WritePacket(P);

	P.w_u8(freezed);
}


void CPhysicStorage::ReadData(NET_Packet& P)
{
	P.r_u8(SyncActive);
	P.r_u8((u8&)DoorState);

	if (!SyncActive)
		return;

	P.r_u8(current_state.count);
	if (current_state.count > 0)
	{
		current_state.State.ReadPacket(P);
		current_state.dwTimeStamp = Device.dwTimeGlobal;

		current_state.State.previous_position = current_state.State.position;
		current_state.State.previous_quaternion = current_state.State.quaternion;
	}

	// SERVER FREEZE
	u8 active;
	P.r_u8(active);

	if (!active)
	{
		if (!freezed)
			m_freeze_time = Device.dwTimeGlobal;
		freezed = true;
	}
	else
	{
		freezed = false;
	}
}


// TODO NOT USED ))) (IN ORIGINAL GAME)
void CPhysicStorage::CalculateInterpolationParams(void* ptr)
{
	if (cast_phobject(ptr)->m_pPhysicsShell)
		cast_phobject(ptr)->m_pPhysicsShell->NetInterpolationModeON();
}

void CPhysicStorage::Interpolate(void* ph)
{
	//simple linear interpolation...

	if (!(cast_phobject(ph) && cast_phobject(ph)->PPhysicsShell()))
		return;

	bool has_door = cast_phobject(ph)->PPhysicsShell()->get_Joint("door");

	if (!has_door)
		if (!cast_phobject(ph)->H_Parent() && cast_phobject(ph)->getVisible() && cast_phobject(ph)->m_pPhysicsShell && OnClient() && NET_IItem.size())
		{
			SyncPhysic newState = NET_IItem.front().State;
			if (NET_IItem.size() >= 2)
			{
				float ret_interpolate = interpolate_states(NET_IItem.front(), NET_IItem.back(), newState);

				if (ret_interpolate >= 1.f)
				{
					NET_IItem.pop_front();
					if (cast_phobject(ph)->m_activated)
					{
						cast_phobject(ph)->processing_deactivate();
						cast_phobject(ph)->m_activated = false;
					}
				}
			}

			// APPLY POS
			cast_phobject(ph)->PHGetSyncItem(0)->set_State(newState.GetPhysicState());
		}
}

float CPhysicStorage::interpolate_states(net_update_PItem const& first, net_update_PItem const& last, SyncPhysic& current)
{
	float ret_val = 0.f;
	u32 CurTime = Device.dwTimeGlobal;

	if (CurTime == last.dwTimeStamp)
		return 0.f;

	float factor = float(CurTime - last.dwTimeStamp) / float(last.dwTimeStamp - first.dwTimeStamp);

	ret_val = factor;
	if (factor > 1.f)
	{
		factor = 1.f;
	}
	else if (factor < 0.f)
	{
		factor = 0.f;
	}

	{
		current.position.x = first.State.position.x + (factor * (last.State.position.x - first.State.position.x));
		current.position.y = first.State.position.y + (factor * (last.State.position.y - first.State.position.y));
		current.position.z = first.State.position.z + (factor * (last.State.position.z - first.State.position.z));
		current.previous_position = current.position;

		current.quaternion.slerp(first.State.quaternion, last.State.quaternion, factor);
		current.previous_quaternion = current.quaternion;
	}

	return ret_val;
}


// SERVER DATA

void CPhysicStorage::GetStateServer(void* ptr)
{
	CSE_ALifeObjectPhysic* physic = (CSE_ALifeObjectPhysic*)ptr;
	if (physic)
	{
		// NOTHING
		// physic->isDoor = DoorState;

	}
}

void CPhysicStorage::SetStateServer(void* ptr)
{
	CSE_ALifeObjectPhysic* physic = (CSE_ALifeObjectPhysic*)ptr;
	if (physic)
	{
		physic->freezed = freezed;
		physic->prev_freezed = prev_freezed;
		physic->m_freeze_time = m_freeze_time;
		physic->isDoor = DoorState;
	}
}

// CLIENT DATA
extern int OFF_PHYSIC_SYNC = 0;

void CPhysicStorage::GetStateClient(void* ph)
{
	// Выключено на сингле 
	if (cast_phobject(ph)->H_Parent() || !cast_phobject(ph)->m_pPhysicsShell || OFF_PHYSIC_SYNC)
	{
		// NO SYNC
		SyncActive = false;
		return;
	}

	// SYNC	
	SyncActive = true;

	if (auto DoorJoint = cast_phobject(ph)->PPhysicsShell()->get_Joint("door"))
	{
		DoorState = true;
		//door_packet.write_start();
		//cast_phobject(ph)->PPhysicsShell()->net_ExportDoor(door_packet);

		current_state.count = 0;

		if (cast_phobject(ph)->PPhysicsShell()->isEnabled())
			freezed = false;
		else
			freezed = true;

		return;
	}

	current_state.count = cast_phobject(ph)->PPhysicsShell()->get_ElementsNumber();
	if (current_state.count > 1)
		current_state.count = 1;

	if (auto sync = cast_phobject(ph)->PHGetSyncItem(0))
	{
		SPHNetState State;
		sync->get_State(State);
		current_state.State.GetFromPhysicState(State);
	}
	else
	{
		current_state.State.position.set(cast_phobject(ph)->Position());
	}

	if (cast_phobject(ph)->PPhysicsShell()->isEnabled())
		freezed = false;
	else
		freezed = true;
}

void CPhysicStorage::SetStateClient(void* ph)
{
	if (cast_phobject(ph)->Local())	// Считать все о то будет плохо
		return;

	if (DoorState)
	{
		if (!cast_phobject(ph)->m_activated)
		{
			cast_phobject(ph)->processing_activate();
			cast_phobject(ph)->m_activated = true;
		}

		return;
	}

	if (current_state.count > 0)
	{
		Level().AddObject_To_Objects4CrPr(cast_phobject(ph));
		NET_IItem.push_back(current_state);

		while (NET_IItem.size() > 2)
		{
			NET_IItem.pop_front();
		}
	}


	if (!cast_phobject(ph)->m_activated)
	{
		cast_phobject(ph)->processing_activate();
		cast_phobject(ph)->m_activated = true;
	}
}
