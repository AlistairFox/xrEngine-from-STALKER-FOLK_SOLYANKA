#pragma once
#include "../Include/xrRender/animation_motion.h"
#include "net_physics_state.h"
#include "../xrServerEntities/PHSynchronize.h"
 
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
  
struct MotionID_numered	 
{
	MotionID id;		//4	 
	u8 num;				//1
	bool loop;			//1
	bool anim_ctrl;
	float pos;			//4 
						//TOTAL: 10
	MotionID_numered() 
	{
		id.invalidate();
		num = 0;
		loop = false;
		anim_ctrl = false;
		pos = 0;
	};
};
 
class EXPORT_MOTIONS
{
	BIT_TO_BYTE BIT;

	void Export_Base(MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head, u64& val);
	void Import_Base(MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head, u64& val);

	enum mask_flags
	{
		legs_flag  = 1 << 0,
		torso_flag = 1 << 1,
		head_flag  = 1 << 2,
	};

public: 
	void Export(NET_Packet& P, MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head);
	void Import(NET_Packet& P, MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head);
};



struct ai_stalker_net_state
{
 
	public:
		EXPORT_MOTIONS exp_motions;

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
 
		ai_stalker_net_state();

		void    fill_position(CPHSynchronize * state_new);
		void	state_write(NET_Packet& packet);
		void	state_read(NET_Packet& packet);
		 
};



