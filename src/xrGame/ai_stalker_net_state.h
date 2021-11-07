#pragma once
#include "../Include/xrRender/animation_motion.h"
#include "net_physics_state.h"
#include "../xrServerEntities/PHSynchronize.h"


struct ai_stalker_net_state
{
	public:
		net_physics_state physics_state;
		Fvector fv_position;
		Fvector fv_linear_vel;

		u8 phSyncFlag;
		
		u16 old_torso_idx;
		u16 old_legs_idx;
		u16 old_head_idx;
		u16 old_script_idx;

		u8 old_torso_slot;
		u8 old_legs_slot;
		u8 old_head_slot;
		u8 old_script_slot;

		u8 u_active_slot;
	
		float u_health;

		float u_body_yaw;
		float u_head_yaw;

		float u_time_torso;
		float u_time_head;
		float u_time_legs;

		float u_time_script;
		
		ai_stalker_net_state();

		void fill_position(CPHSynchronize * state_new);
		

		void	fill_state(
			u16 torso_idx, u16 legs_idx, u16 head_idx, u16 script_idx,
			u8 torso_slot, u8 legs_slot, u8 head_slot, u8 script_slot,
			float healtch, u16 active_slot, Fvector3 position
		);

		void	state_write(NET_Packet& packet);
		void	state_read(NET_Packet& packet);
		 
};



