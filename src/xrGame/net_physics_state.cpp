#include "StdAfx.h"
#include "net_physics_state.h"
#include "Level.h"

typedef unsigned short ushort;
typedef unsigned int uint;

uint as_uint(const float x) {
	return *(uint*)&x;
}
float as_float(const uint x) {
	return *(float*)&x;
}

float half_to_float(const ushort x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
	const uint e = (x & 0x7C00) >> 10; // exponent
	const uint m = (x & 0x03FF) << 13; // mantissa
	const uint v = as_uint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
	return as_float((x & 0x8000) << 16 | 
		(e != 0) * ((e + 112) << 23 | m) |
		((e == 0) & (m != 0)) * ((v - 37) << 23 |
		((m << (150 - v)) & 0x007FE000))); 
	// sign : normalized : denormalized
}

ushort float_to_half(const float x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
	const uint b = as_uint(x) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
	const uint e = (b & 0x7F800000) >> 23; // exponent
	const uint m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
	return (b & 0x80000000) >> 16 |
		   (e > 112) * ((((e - 112) << 10) & 0x7C00) | 
			m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) |
		   (e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
}

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

void net_physics_state::write(NET_Packet &packet)
{
	// linear velocity
	//clamp(physics_linear_velocity.x, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);
	//clamp(physics_linear_velocity.y, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);
	//clamp(physics_linear_velocity.z, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);
 
	//packet.w_float_q8(physics_linear_velocity.x, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);
	//packet.w_float_q8(physics_linear_velocity.y, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);
	//packet.w_float_q8(physics_linear_velocity.z, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);

	// position
	//packet.w_float(physics_position.x);
	//packet.w_float(physics_position.y);
	//packet.w_float(physics_position.z);

	u16 x = float_to_half(physics_position.x);
	u16 y = float_to_half(physics_position.z);
	u16 z = float_to_half(physics_position.y);

	packet.w_u16(x);
	packet.w_u16(y);
	packet.w_u16(z);

	// physics state enabled
	packet.w_u8(physics_state_enabled ? 1 : 0);
}

void net_physics_state::read(NET_Packet &packet)
{
	dwTimeStamp = Level().Objects.net_Import_Time();

	// linear velocity
	//packet.r_float_q8(physics_linear_velocity.x, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);
	//packet.r_float_q8(physics_linear_velocity.y, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);
	//packet.r_float_q8(physics_linear_velocity.z, MIN_LINEAR_VELOCITY_COMPONENT, MAX_LINEAR_VELOCITY_COMPONENT);

	// position
	//packet.r_float(physics_position.x);
	//packet.r_float(physics_position.y);
	//packet.r_float(physics_position.z);
	u16 x,y,z;
	packet.r_u16(x);
	packet.r_u16(y);
	packet.r_u16(z);


	float decode_x = half_to_float(x);
	float decode_y = half_to_float(z);
	float decode_z = half_to_float(y);

	physics_position.x = decode_x;
	physics_position.y = decode_y;
	physics_position.z = decode_z;

	// physics state enabled
	physics_state_enabled = static_cast<bool>(packet.r_u8());
}
