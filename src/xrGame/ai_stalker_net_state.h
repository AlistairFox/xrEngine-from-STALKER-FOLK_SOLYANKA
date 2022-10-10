#pragma once
#include "../Include/xrRender/animation_motion.h"
#include "net_physics_state.h"
#include "../xrServerEntities/PHSynchronize.h"
 
BIT_TO_BYTE BIT;
HF HFLT;
 
struct MotionID_numered
{
	MotionID id;	 //10 BYTES 
	u8 num;
	bool loop;
	float pos;
};
 
class EXPORT_MOTIONS
{
	void E(MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head, u64 val)
	{
		u32 cur = 0;
		BIT.write_bites(10, legs.id.idx, cur, val);
		BIT.write_bites(10, torso.id.idx, cur, val);
		BIT.write_bites(10, head.id.idx, cur, val);

		BIT.write_bites(8, legs.num, cur, val);
		BIT.write_bites(8, torso.num, cur, val);
		BIT.write_bites(8, head.num, cur, val);

		BIT.write_bites(1, legs.loop, cur, val);
		BIT.write_bites(1, torso.loop, cur, val);
		BIT.write_bites(1, head.loop, cur, val);
	}

	void I(MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head, u64 val)
	{
		u32 cur = 0;
		legs.id.idx = BIT.read_bites(10, cur, val);
		torso.id.idx = BIT.read_bites(10, cur, val);
		head.id.idx = BIT.read_bites(10, cur, val);
	
		legs.num = BIT.read_bites(8, cur, val);
		torso.num = BIT.read_bites(8, cur, val);
		head.num = BIT.read_bites(8, cur, val);

		legs.loop = BIT.read_bites(1, cur, val);
		torso.loop = BIT.read_bites(1, cur, val);
		head.loop = BIT.read_bites(1, cur, val);
	}

public: 
	void Export(NET_Packet& P, MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head)
	{
		//30 BYTES NO COMPRESS 

		// 6 + 8 = 14 BYTES AFTER COMPRESS (HALF FLOAT MAYBY COMPRESS TO 1 BYTE (1 * 3) )


		P.w_u16(HFLT.float_to_half(legs.pos));
		P.w_u16(HFLT.float_to_half(torso.pos));
		P.w_u16(HFLT.float_to_half(head.pos));	

		u64 value = 0; 
		E(legs,torso, head, value); P.w_u64(value);
		
	};

	void Import(NET_Packet& P, MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head)
	{
		legs.pos = HFLT.half_to_float(P.r_u16());
		torso.pos = HFLT.half_to_float(P.r_u16());
		head.pos = HFLT.half_to_float(P.r_u16());
		I(legs, torso, head, P.r_u64());
	};
};

EXPORT_MOTIONS exp_motions;


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



