#include "stdafx.h"
#include "ai_stalker_net_state.h"

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

void ai_stalker_net_state::fill_state(
	u16 torso_idx, u16 legs_idx, u16 head_idx, u16 script_idx,
	 u8 torso_slot, u8 legs_slot, u8 head_slot, u8 script_slot,
	float health, u16 active_slot, Fvector3 position
)
{
	old_torso_idx = torso_idx;
	old_legs_idx = legs_idx;
	old_head_idx = head_idx;
	old_script_idx = script_idx;

	old_torso_slot = torso_slot;
	old_legs_slot = legs_slot;
	old_head_slot = head_slot;
	old_script_slot = script_slot;

	u_health = health; 
	u_active_slot = active_slot;
	fv_position = position;
}

ai_stalker_net_state::ai_stalker_net_state()
{
	old_torso_idx = 0;
	old_legs_idx = 0;
	old_head_idx = 0;
	old_script_idx = 0;

	old_torso_slot = 0;
	old_legs_slot = 0;
	old_head_slot = 0;
	old_script_slot = 0;

	u_health = 1.0f;
	u_active_slot = 0;
	u_body_yaw = 0;
	u_head_yaw = 0;

	fv_position.set(0,0,0);
	fv_linear_vel.set(0,0,0);

	u_time_torso = 0;
	u_time_head = 0;
	u_time_legs = 0;

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
		packet.w_angle8(u_body_yaw);
		packet.w_angle8(u_head_yaw);
	}

	packet.w_float(u_time_script);
	packet.w_float(u_time_torso);
	packet.w_float(u_time_legs);
	packet.w_float(u_time_head);

	//Animation
	{
		u32 current = 0;
		u64 output = 0;

		u32 bit_idx = 10;
		u32 bit_slot = 2;

		::write_bites(bit_idx, old_torso_idx, current, output);
		::write_bites(bit_idx, old_legs_idx, current, output);
		::write_bites(bit_idx, old_head_idx, current, output);
		::write_bites(bit_idx, old_script_idx, current, output);

		::write_bites(bit_slot, old_torso_slot, current, output);
		::write_bites(bit_slot, old_legs_slot, current, output);
		::write_bites(bit_slot, old_head_slot, current, output);
		::write_bites(bit_slot, old_script_slot, current, output);

		packet.w_u8(u8(output >> 0));
		packet.w_u8(u8(output >> 8));
		packet.w_u8(u8(output >> 16));
		packet.w_u8(u8(output >> 24));
		packet.w_u8(u8(output >> 32));
		packet.w_u8(u8(output >> 40));


	}

	if (u_health < 1.0f && false)
	{
		Msg("write H[%.4f] S[%d] body[%.2f] head[%.2f]", u_health, u_active_slot, u_body_yaw, u_head_yaw);

		Msg("write TORSO	[%d][%d]", old_torso_idx, old_torso_slot);
		Msg("write LEGS		[%d][%d]", old_legs_idx, old_legs_slot);
		Msg("write HEAD		[%d][%d]", old_head_idx, old_head_slot);
		Msg("write SCRIPT	[%d][%d]", old_script_idx, old_script_slot);
	}
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
	 
	//Msg("Import [%.0f][%.0f][%.0f]", fv_position.x, fv_position.y, fv_position.z);

	//States
	{	
		u_health = packet.r_u8();
		packet.r_u8(u_active_slot);  //8 bit
		packet.r_angle8(u_body_yaw); //8 bit
		packet.r_angle8(u_head_yaw); //8 bit

	}

	u_time_script = packet.r_float();
	u_time_torso  = packet.r_float();
	u_time_legs   = packet.r_float();
	u_time_head   = packet.r_float();

	//Animation
	{
		u32 current = 0;
		u64 output = 0;

		output |= u64(packet.r_u8()) << 0;
		output |= u64(packet.r_u8()) << 8;
		output |= u64(packet.r_u8()) << 16;
		output |= u64(packet.r_u8()) << 24;
		output |= u64(packet.r_u8()) << 32;
		output |= u64(packet.r_u8()) << 40;

		u32 bit_idx = 10;
		u32 bit_slot = 2;

		old_torso_idx = ::read_bites(bit_idx, current, output);
		old_legs_idx = ::read_bites(bit_idx, current, output);
		old_head_idx = ::read_bites(bit_idx, current, output);
		old_script_idx = ::read_bites(bit_idx, current, output);

		old_torso_slot = ::read_bites(bit_slot, current, output);
		old_legs_slot = ::read_bites(bit_slot, current, output);
		old_head_slot = ::read_bites(bit_slot, current, output);
		old_script_slot = ::read_bites(bit_slot, current, output);

		if (old_torso_idx == 1023)
			old_torso_idx = 65535;

		if (old_legs_idx == 1023)
			old_legs_idx = 65535;

		if (old_head_idx == 1023)
			old_head_idx = 65535;

		if (old_script_idx == 1023)
			old_script_idx = 65535;


	}
	
	if (u_health < 1.0f && false)
	{
		Msg("read H[%.4f] S[%d] body[%.2f] head[%.2f]", u_health, u_active_slot, u_body_yaw, u_head_yaw);

		Msg("read TORSO		[%d][%d]", old_torso_idx, old_torso_slot);
		Msg("read LEGS		[%d][%d]", old_legs_idx, old_legs_slot);
		Msg("read HEAD		[%d][%d]", old_head_idx, old_head_slot);
		Msg("read SCRIPT	[%d][%d]", old_script_idx, old_script_slot);
	}
}
