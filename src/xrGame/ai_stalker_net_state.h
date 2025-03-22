#pragma once
#include "../Include/xrRender/animation_motion.h"
#include "net_physics_state.h"
#include "../xrServerEntities/PHSynchronize.h"
#include "sight_manager.h"
 
struct ai_stalker_net_state
{
public:
		template<typename T>
		void write_bites(const u32& bit_count, const u32& value, u32& current, T& output)
		{
			output |= ((value & ((T(1) << bit_count) - 1)) << current);
			current += bit_count;
		}

		template<typename T>
		T read_bites(const u32& bit_count, u32& current, const T& read_value)
		{
			T			result = (read_value >> current) & ((T(1) << bit_count) - 1);
			current += bit_count;
			return		(result);
		}
 
		net_physics_state physics_state;
		Fvector fv_position;
 		Fvector fv_linear_vel;

		u8 phSyncFlag;

		u16 u_active_item;
		u8 u_active_slot;
	
		float u_health;
		 
		float torso_yaw;
		float head_yaw;
 		float torso_pitch;
		float head_pitch;
 
		/*
		bone_or_part:
		0: legs
		1: torso
		2: head
		*/

		MotionID m_torso; // slot = 1
		MotionID m_head;  // slot = 2
		MotionID m_legs;  // slot = 0
		MotionID m_script;  // slot = 0
		MotionID m_global;  // slot = 0

		CSightManager::aiming_type m_aiming_type;
		shared_str m_aiming_anim;
		CSightManager::animation_frame_type m_aiming_frame;

		ai_stalker_net_state();

		void    fill_position(CPHSynchronize * state_new);
		void	state_write(NET_Packet& packet);
		void	state_read(NET_Packet& packet);
		 
};



