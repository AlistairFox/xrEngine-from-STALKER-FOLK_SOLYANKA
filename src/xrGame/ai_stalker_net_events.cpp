#include "stdafx.h"
#include "pch_script.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_animation_manager.h"
#include "inventory.h"
#include "../xrCore/_flags.h"

#include "ai_stalker_net_state.h"

enum sync_flags
{
	animation_flag = (1 << 0),
	animation_update = (1 << 1),

};
 
void CAI_Stalker::OnEventAnimations(bool update)
{
	MotionID torso, legs, head, script;
	u16 torso_idx, legs_idx, head_idx, script_idx;
	u8 torso_slot, legs_slot, head_slot, script_slot;

	IKinematicsAnimated* m_skeleton_animated = smart_cast<IKinematicsAnimated*>(Visual());

	torso = animation().torso().animation();
	legs = animation().legs().animation();
	head = animation().head().animation();
	script = animation().script().animation();

	torso_idx = torso.idx;
	torso_slot = torso.slot;

	legs_idx = legs.idx;
	legs_slot = legs.slot;

	head_idx = head.idx;
	head_slot = head.slot;

	script_idx = script.idx;
	script_slot = script.slot;


	NET_Packet packet;
	Game().u_EventGen(packet, GE_ANIMATION_SCRIPT, this->ID());

	flags8 flag;
	flag.zero();
	flag.set(sync_flags::animation_flag, psDeviceFlags.test(rsDrawStatic));
	flag.set(sync_flags::animation_update, update);

	packet.w_u8(flag.flags);

	if (flag.test(animation_flag))
	{
		packet.w_u16(torso_idx);
		packet.w_u16(legs_idx);
		packet.w_u16(head_idx);
		packet.w_u16(script_idx);

		packet.w_u8(torso_slot);
		packet.w_u8(legs_slot);
		packet.w_u8(head_slot);
		packet.w_u8(script_slot);
	}
	else
	{/*
		ai_stalker_net_state state;
		state.fill_state(torso_idx, legs_idx, head_idx, script_idx,
						 torso_slot, legs_slot, head_slot, script_slot	
						 
		);
		state.animation_write(packet);
	*/	 
	}

	//Msg("PacketSize[%d]", packet.B.count);
	Game().u_EventSend(packet);
}

void CAI_Stalker::OnEventAnimations(NET_Packet packet)
{
	MotionID torso, legs, head, script;
	u16 torso_idx, legs_idx, head_idx, script_idx;
	u8 torso_slot, legs_slot, head_slot, script_slot;
	
	//u32 pos = packet.r_pos;

	flags8 flag_test;
	packet.r_u8(flag_test.flags);

	bool test = flag_test.test(sync_flags::animation_flag);
	bool update = flag_test.test(sync_flags::animation_update);

	
	if (test) 
	{
		packet.r_u16(torso_idx);
		packet.r_u16(legs_idx);
		packet.r_u16(head_idx);
		packet.r_u16(script_idx);
		packet.r_u8(torso_slot);
		packet.r_u8(legs_slot);
		packet.r_u8(head_slot);
		packet.r_u8(script_slot);

		torso.set(torso_slot, torso_idx);
		legs.set(legs_slot, legs_idx);
		head.set(head_slot, head_idx);
		script.set(script_slot, script_idx);
	}
	else
	{
		/*
		ai_stalker_net_state state;
		state.animation_read(packet);
		
		torso = state.s_torso;
		legs = state.s_legs;
		head = state.s_head;
		script = state.s_script;
		*/

	}

	//Msg("SizePucket [%d] [%s] [%s]", packet.r_pos - pos, test ? "true" : "false", update ? "true" : "false");

	
	
	bool check = false;

	if (check)
	{
		Msg("OnEventAnimation TORSO	 [%d][%d]", torso.idx, torso.slot);
		Msg("OnEventAnimation LEGS	 [%d][%d]", legs.idx, legs.slot);
		Msg("OnEventAnimation HEAD	 [%d][%d]", head.idx, head.slot);
		Msg("OnEventAnimation SCRIPT [%d][%d]", script.idx, script.slot);
	}

	IKinematicsAnimated* m_skeleton_animated = smart_cast<IKinematicsAnimated*>(Visual());
	if (!this->g_Alive() || !m_skeleton_animated || true)
		return;
 
	if (torso.valid())
	{
		if (u_last_torso_motion_idx != torso.idx)
		{
			m_skeleton_animated->LL_PlayCycle(
				m_skeleton_animated->LL_PartID("torso"),
				torso,
				TRUE,
				m_skeleton_animated->LL_GetMotionDef(torso)->Accrue(),
				m_skeleton_animated->LL_GetMotionDef(torso)->Falloff(),
				m_skeleton_animated->LL_GetMotionDef(torso)->Speed(),
				FALSE, 0, 0, 0
			);

			u_last_torso_motion_idx = torso.idx;
		}
	}
	 
	if (legs.valid())
	{
		if (u_last_legs_motion_idx != legs.idx) 
		{
			CStepManager::on_animation_start(legs,
				m_skeleton_animated->LL_PlayCycle(
					m_skeleton_animated->LL_PartID("legs"),
					legs,
					TRUE,
					m_skeleton_animated->LL_GetMotionDef(legs)->Accrue(),
					m_skeleton_animated->LL_GetMotionDef(legs)->Falloff(),
					m_skeleton_animated->LL_GetMotionDef(legs)->Speed(),
					FALSE, 0, 0, 0
				)
			);

			u_last_legs_motion_idx = legs.idx;
		}
	}
 
 
	if (head.valid())
	{
		if (u_last_head_motion_idx != head.idx) 
		{
			m_skeleton_animated->LL_PlayCycle(
				m_skeleton_animated->LL_PartID("head"),
				head,
				TRUE,
				m_skeleton_animated->LL_GetMotionDef(head)->Accrue(),
				m_skeleton_animated->LL_GetMotionDef(head)->Falloff(),
				m_skeleton_animated->LL_GetMotionDef(head)->Speed(),
				FALSE, 0, 0, 0
			);

			u_last_head_motion_idx = head.idx;
		}
	}

	if (script.valid())
	{
		if (u_last_script_motion_idx != script.idx) {
			m_skeleton_animated->LL_PlayCycle(
				m_skeleton_animated->LL_GetMotionDef(script)->bone_or_part,
				script,
				TRUE,
				m_skeleton_animated->LL_GetMotionDef(script)->Accrue(),
				m_skeleton_animated->LL_GetMotionDef(script)->Falloff(),
				m_skeleton_animated->LL_GetMotionDef(script)->Speed(),
				m_skeleton_animated->LL_GetMotionDef(script)->StopAtEnd(),
				0, 0, 0
			);

			u_last_script_motion_idx = script.idx;
		}
	}

}