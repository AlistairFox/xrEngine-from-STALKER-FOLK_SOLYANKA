#include "stdafx.h"
#include "ai_stalker_net_state.h"
 

#define HALF_FLOAT

void write_bites(const u32& bit_count, const u32& value, u32& current, u64& output)
{
	output |= ( (value & ( (u64(1) << bit_count) - 1) ) << current);
	current += bit_count;
	//VERIFY(current <= 32);
}

u32 read_bites(const u32& bit_count, u32& current, const u64& output)
{
	u32			result = (output >> current) & ((u64(1) << bit_count) - 1);
	current += bit_count;
	//VERIFY(current <= 32);
	return		(result);
}

u8 float_to_char4(float value) //0.0666 //4bit (ÒÎÊ ÄËß FLOAT 0.f->1.f)
{
	if (value > 1.0f)
		value = 1.0f;
	return u8(15.f * value);
}
 
float char4_to_float(u8 value) //0.0666 //4 bit (ÒÎÊ ÄËß FLOAT 0.f->1.f)
{
	return value / 15.0f;
}

u8 float_to_char8(float value) //0.0039 //8bit (ÒÎÊ ÄËß FLOAT 0.f->1.f)
{
	if (value > 1.0f)
		value = 1.0f;
	return u8(255.0f * value);
}

float char8_to_float(u8 value) //0.0039 //8 bit (ÒÎÊ ÄËß FLOAT 0.f->1.f)
{
	return  value / 255.0f ;
}

u8 char_health(float value)
{
	bool isAlive = true;
	
	if (value == 0.0f)
		isAlive = false;

	if (value > 1.0f)
		value = 1.0f;

	u8 heltch_write = value * 127;

	u8 heltch = 0;

	heltch |= (isAlive & (u8(1) << 1) - 1) << 0;
	
	heltch |= (heltch_write & (u8(1) << 7) - 1) << 1;

	return heltch;
}

float char_health_to_float(u8 value)
{

	u8 alive = value >> 0 & (u8(1) << 1) - 1;
	
	float health = value >> 1 & (u8(1) << 7) - 1;
	
	//Msg("alive[%f]", health);

	health /= 127;

	return health + (alive ? 0.01 : 0);
}
 
void ai_stalker_net_state::fill_position(CPHSynchronize * sync)
{
	CPHSynchronize* new_sync = sync;

	if (new_sync)
	{
		phSyncFlag = true;
		SPHNetState state;
		new_sync->get_State(state);

		physics_state.fill(state);
	}
	else
	{
		phSyncFlag = false;
	}

}

ai_stalker_net_state::ai_stalker_net_state()
{
	u_health = 1.0f;
	u_active_slot = 0;
	fv_position.set(0,0,0);
	fv_linear_vel.set(0,0,0);

	phSyncFlag = false;
}

void ai_stalker_net_state::state_write(NET_Packet& packet)
{ 
	//Physic states
	if (phSyncFlag)
	{
		packet.w_u8(1);
		physics_state.write(packet);
	}
	else 			
	{
		packet.w_u8(0);
		packet.w_vec3(fv_position);
	}

  
	//Body State
	{
		packet.w_u8(u_health > 0.00001f ? 1 : 0);
		packet.w_u8(u_active_slot);
		packet.w_u16(u_active_item);
		packet.w_u8(script_animation);

		packet.w_angle8(torso_yaw);
		packet.w_angle8(head_yaw);
 
		packet.w_angle8(torso_pitch);
		packet.w_angle8(head_pitch);
 
 	}


//	Msg("PacketPos: %d", packet.B.count);

	packet.w(&torso_anim, sizeof(MotionID_numered));
	packet.w(&legs_anim,  sizeof(MotionID_numered));
	packet.w(&head_anim,  sizeof(MotionID_numered));
}

void ai_stalker_net_state::state_read(NET_Packet& packet)
{
	//physic
	packet.r_u8(phSyncFlag);

	if (phSyncFlag)
	{
		physics_state.read(packet);
		fv_position.set(physics_state.physics_position);
	}
	else
	{
		packet.r_vec3(fv_position);
	}

 	 
	//States
	{	
		u_health = packet.r_u8();
		packet.r_u8(u_active_slot);  //8 bit
		packet.r_u16(u_active_item);
		script_animation = packet.r_u8();
		 
		packet.r_angle8(torso_yaw);
		packet.r_angle8(head_yaw);

		packet.r_angle8(torso_pitch);
		packet.r_angle8(head_pitch);
 	}

//	Msg("PacketPosRead: %d", packet.r_pos);

 	packet.r(&torso_anim, sizeof(MotionID_numered));
	packet.r(&legs_anim, sizeof(MotionID_numered));
	packet.r(&head_anim, sizeof(MotionID_numered));
 
} 
