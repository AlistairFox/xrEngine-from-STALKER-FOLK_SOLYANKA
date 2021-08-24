#include "stdafx.h"
#include "ai_stalker_net_state.h"

void write_bites(const u32& bit_count, const u32& value, u32& current, u64& output)
{
	output |= ((value & ((u64(1) << bit_count) - 1)) << current);
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


void ai_stalker_net_state::fill_state(
	u16 torso_idx, u16 legs_idx, u16 head_idx, u16 script_idx,
	 u8 torso_slot, u8 legs_slot, u8 head_slot, u8 script_slot,
	float health, u16 active_slot
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

}

void ai_stalker_net_state::animation_write(NET_Packet& packet)
{ 
	//States
	packet.w_float(u_health);
	packet.w_u16(u_active_slot);
	packet.w_angle8(u_body_yaw);
	packet.w_angle8(u_head_yaw);

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

void ai_stalker_net_state::animation_read(NET_Packet& packet)
{
	//States
	packet.r_float(u_health);
	packet.r_u16(u_active_slot);
	packet.r_angle8(u_body_yaw);
	packet.r_angle8(u_head_yaw);

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
