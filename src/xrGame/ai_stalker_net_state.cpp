#include "stdafx.h"
#include "ai_stalker_net_state.h"
 
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
} 
