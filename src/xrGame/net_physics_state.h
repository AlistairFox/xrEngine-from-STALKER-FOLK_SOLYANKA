#pragma once
#include "PHNetState.h"

#define MIN_LINEAR_VELOCITY_COMPONENT -32.f
#define MAX_LINEAR_VELOCITY_COMPONENT 32.f

HF HFLOAT;

struct net_physics_state
{
	Fvector physics_linear_velocity;
	Fvector physics_position;
	bool	physics_state_enabled;
	u32		dwTimeStamp;

	net_physics_state();
	void fill(SPHNetState &state);
	void write(NET_Packet &packet);
	void read(NET_Packet &packet);
};

class HF
{

	typedef unsigned short ushort;
	typedef unsigned int uint;

	uint as_uint(const float x) {
		return *(uint*)&x;
	}
	float as_float(const uint x) {
		return *(float*)&x;
	}

public:
	float half_to_float(const ushort x)
	{ // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
		const uint e = (x & 0x7C00) >> 10; // exponent
		const uint m = (x & 0x03FF) << 13; // mantissa
		const uint v = as_uint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
		return as_float((x & 0x8000) << 16 |
			(e != 0) * ((e + 112) << 23 | m) |
			((e == 0) & (m != 0)) * ((v - 37) << 23 |
				((m << (150 - v)) & 0x007FE000)));
		// sign : normalized : denormalized
	}

	ushort float_to_half(const float x)
	{ // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
		const uint b = as_uint(x) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
		const uint e = (b & 0x7F800000) >> 23; // exponent
		const uint m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
		return (b & 0x80000000) >> 16 |
			(e > 112) * ((((e - 112) << 10) & 0x7C00) |
				m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) |
			(e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
	}
};

class BIT_TO_BYTE
{
public:

	void write_bites(const u32& bit_count, const u32& value, u32& current, u64& output)
	{
		output |= ((value & ((u64(1) << bit_count) - 1)) << current);
		current += bit_count;
 	}

	u64 read_bites(const u32& bit_count, u32& current, const u64& output)
	{
		u64			result = (output >> current) & ((u64(1) << bit_count) - 1);
		current += bit_count;
 		return		(result);
	}
};