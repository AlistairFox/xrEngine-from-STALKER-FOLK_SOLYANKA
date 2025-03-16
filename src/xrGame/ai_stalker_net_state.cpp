#include "stdafx.h"
#include "ai_stalker_net_state.h"
 
void EXPORT_MOTIONS::Import(NET_Packet& P, MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head)
{
	//Основа
	u64 value = P.r_u64();
	Import_Base(legs, torso, head, value);
 
 };

void EXPORT_MOTIONS::Export(NET_Packet& P, MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head)
{
	// Основа
	u64 value = 0;
	Export_Base(legs, torso, head, value); P.w_u64(value);	 //8
 
};

void EXPORT_MOTIONS::Import_Base(MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head, u64& val)
{
	u32 cur = 0;
	
	legs.id.idx = BIT.read_bites(10, cur, val);			//u16
	torso.id.idx = BIT.read_bites(10, cur, val);		//u16
	head.id.idx = BIT.read_bites(10, cur, val);			//u16

	legs.id.slot = BIT.read_bites(2, cur, val);			//u8
	torso.id.slot = BIT.read_bites(2, cur, val);		//u8
	head.id.slot = BIT.read_bites(2, cur, val);			//u8

	legs.num = BIT.read_bites(8, cur, val);				//u8
	torso.num = BIT.read_bites(8, cur, val);			//u8
	head.num = BIT.read_bites(8, cur, val);				//u8

	legs.loop = BIT.read_bites(1, cur, val);			//u8
	torso.loop = BIT.read_bites(1, cur, val);			//u8
	head.loop = BIT.read_bites(1, cur, val);			//u8

	//63 total = 8 BYTE || 15 BYTE
}
 
void EXPORT_MOTIONS::Export_Base(MotionID_numered& legs, MotionID_numered& torso, MotionID_numered& head, u64& val)
{
	u32 cur = 0;
	BIT.write_bites(10, legs.id.idx, cur, val);
	BIT.write_bites(10, torso.id.idx, cur, val);
	BIT.write_bites(10, head.id.idx, cur, val);

	BIT.write_bites(2, legs.id.slot, cur, val);
	BIT.write_bites(2, torso.id.slot, cur, val);
	BIT.write_bites(2, head.id.slot, cur, val);

	BIT.write_bites(8, legs.num, cur, val);
	BIT.write_bites(8, torso.num, cur, val);
	BIT.write_bites(8, head.num, cur, val);

	BIT.write_bites(1, legs.loop, cur, val);
	BIT.write_bites(1, torso.loop, cur, val);
	BIT.write_bites(1, head.loop, cur, val);

	//63 bit TOTAL = 8 bytes (1 bit empty)
}

void ai_stalker_net_state::fill_position(CPHSynchronize * sync)
{
	if (sync)
	{
		phSyncFlag = true;
		SPHNetState state;
		sync->get_State(state);

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
	torso_yaw = 0.0f;
	head_yaw = 0.0f;
	torso_pitch = 0.0f;
	head_pitch = 0.0f;

	phSyncFlag = false;
}

void ai_stalker_net_state::state_write(NET_Packet& packet)
{ 
	//Physic states
	if (phSyncFlag)
	{
		packet.w_u8(1); physics_state.write(packet);		 //14
	}
	else 			
	{
		packet.w_u8(0);
		packet.w_vec3(fv_position);
	}
 
	//Body State
	{	
		packet.w_u8(u_active_slot);
		packet.w_u16(u_active_item);
		packet.w_float(u_health);							   //5

		// 4 BYTE or float all (4  * 4 ) = 16 BYTE
		packet.w_angle8(torso_yaw);
		packet.w_angle8(head_yaw);
		packet.w_angle8(torso_pitch);
		packet.w_angle8(head_pitch);						  //4
 	}

	//Animation Export
	// exp_motions.Export(packet, legs_anim, torso_anim, head_anim);
}

void ai_stalker_net_state::state_read(NET_Packet& packet)
{
	//physic
	packet.r_u8(phSyncFlag);															 //1

	if (phSyncFlag)
	{
		physics_state.read(packet);	fv_position.set(physics_state.physics_position);	 // 13
	}
	else
	{
		packet.r_vec3(fv_position);
	}
 
	//States
	{	
 		packet.r_u8(u_active_slot);   													 // 5 BYTE
		packet.r_u16(u_active_item);
		packet.r_float(u_health);
  		packet.r_angle8(torso_yaw);
		packet.r_angle8(head_yaw);
		packet.r_angle8(torso_pitch);
		packet.r_angle8(head_pitch);	
	}																					// 23 BYTE BODY STATE
																						
	//Animation Import	MAY BY timer ( > 500ms ) send ANIMATION
	// exp_motions.Import(packet, legs_anim, torso_anim, head_anim);						// 15 max, 9 min BYTE
} 
