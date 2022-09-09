#pragma once
#include "../Include/xrRender/animation_motion.h"
#include "net_physics_state.h"
#include "../xrServerEntities/PHSynchronize.h"
 

struct MotionID_numered
{
	MotionID id;
	u8 num;
	bool loop;
	float pos;
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
		 
		MotionID_numered torso_anim;
		MotionID_numered legs_anim;
		MotionID_numered head_anim;

		bool script_animation;

		ai_stalker_net_state();

		void    fill_position(CPHSynchronize * state_new);
		void	state_write(NET_Packet& packet);
		void	state_read(NET_Packet& packet);
		 
};



