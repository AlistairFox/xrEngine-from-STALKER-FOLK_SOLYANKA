#include "StdAfx.h"
#include "net_physics_state.h"
#include "Level.h"

//#define HALF_FLOAT;

net_physics_state::net_physics_state()
{
	physics_linear_velocity.set(0,0,0);
	physics_position.set(0, 0, 0);
	physics_state_enabled = false;
	dwTimeStamp = 0;
}

void net_physics_state::fill(SPHNetState &state)
{
	physics_linear_velocity = state.linear_vel;
	physics_position = state.position;
	physics_state_enabled = state.enabled;
}

void net_physics_state::write(NET_Packet& packet)
{
	// position
 
#ifdef	HALF_FLOAT 
	u16 x = HFLOAT.float_to_half(physics_position.x);
	u16 y = HFLOAT.float_to_half(physics_position.z);
	u16 z = HFLOAT.float_to_half(physics_position.y);

	packet.w_u16(x);
	packet.w_u16(y);
	packet.w_u16(z);   
#else	
	packet.w_float(physics_position.x);
	packet.w_float(physics_position.y);
	packet.w_float(physics_position.z);	
#endif
	// physics state enabled
	packet.w_u8(physics_state_enabled ? 1 : 0);
}

void net_physics_state::read(NET_Packet &packet)
{
	dwTimeStamp = Level().Objects.net_Import_Time();
 
#ifdef	HALF_FLOAT 
	{
		u16 x,y,z;
		packet.r_u16(x);
		packet.r_u16(y);
		packet.r_u16(z);
 
		float decode_x = HFLOAT.half_to_float(x);
		float decode_y = HFLOAT.half_to_float(z);
		float decode_z = HFLOAT.half_to_float(y);

		physics_position.x = decode_x;
		physics_position.y = decode_y;
		physics_position.z = decode_z;
	}
#else
	{
		packet.r_float(physics_position.x);
		packet.r_float(physics_position.y);
		packet.r_float(physics_position.z);
	}
#endif
	// physics state enabled
	physics_state_enabled = static_cast<bool>(packet.r_u8());
}
