#pragma once
#include "../Include/xrRender/animation_motion.h"
#include "net_physics_state.h"
#include "../xrServerEntities/PHSynchronize.h"
#include "sight_manager.h"
 
class BIT_TO_BYTE
{
public:
 	void write_bites(const u32& bit_count, const u32& value, u32& current, u64& output)
	{
		output |= ((value & ((u64(1) << bit_count) - 1)) << current);
		current += bit_count;
	}

	u64 read_bites(const u32& bit_count, u32& current, const u64& read_value)
	{
		u64			result = (read_value >> current) & ((u64(1) << bit_count) - 1);
		current += bit_count;
		return		(result);
	}
};
  

struct ai_stalker_net_state
{
 
	public:
 
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

		CSightManager::aiming_type m_aiming_type;
		shared_str m_aiming_anim;
		CSightManager::animation_frame_type m_aiming_frame;

		ai_stalker_net_state();

		void    fill_position(CPHSynchronize * state_new);
		void	state_write(NET_Packet& packet);
		void	state_read(NET_Packet& packet);
		 
};



