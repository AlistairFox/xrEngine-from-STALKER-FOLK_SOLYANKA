#include "stdafx.h"
#include "Weapon_State_Network.h"

IC	void write_binary(const u32& bit_count, const u32& value, u32& current, u32& output)
{
	output |= ((value & ((u32(1) << bit_count) - 1)) << current);
	current += bit_count;
	VERIFY(current <= 32);
}

IC	u32 read_binary(const u32& bit_count, u32& current, const u32& output)
{
	u32			result = (output >> current) & ((u32(1) << bit_count) - 1);
	current += bit_count;
	VERIFY(current <= 32);
	return		(result);
}

Weapon_State_Network::Weapon_State_Network()
{
	m_u8NumItems	= 0;

	m_fCondition	= 1.0f;
	m_addon_flags.zero();
	wpn_state		= 0;		   
	a_elapsed		= 0; 

	ammo_type		= 0;
	m_bZoom			= 0;
	m_cur_scope		= 0;
	need_to_update	= 0;

	m_cur_slot = 0;
}

void Weapon_State_Network::fill_state()
{

}

void Weapon_State_Network::write_state(NET_Packet& P)
{
//Cant Change
	P.w_float_q8(m_fCondition, 0.0f, 1.0f);
	P.w_u8(a_elapsed);
	P.w_u8(m_addon_flags.get());

//Convert
	u32 current = 0;
	u32 output = 0;

	u32 byte1 = 1;
	
	::write_binary(4, wpn_state, current, output);
	::write_binary(byte1, need_to_update, current, output);
	::write_binary(byte1, ammo_type, current, output);
	::write_binary(byte1, m_bZoom, current, output);
	::write_binary(byte1, m_cur_scope, current, output);
	
	
	P.w_u8(u8(output));
	

	
	
	/*
	// 4 bit 
	P.w_u8(wpn_state);


	//CAN MOVE TO 1 U8 -> 4 bit data
	P.w_u8(need_to_update);
	P.w_u8(ammo_type);
	P.w_u8(m_bZoom);
	P.w_u8(m_cur_scope);
	*/

}

void Weapon_State_Network::read_state(NET_Packet& P)
{

	//Cant Change
	P.r_float_q8(m_fCondition, 0.0f, 1.0f);
	P.r_u8(a_elapsed);
	P.r_u8(m_addon_flags.flags);
 
	u32 current = 0;
	u32 output = 0;
	u32 byte1 = 1;

	output = u32(P.r_u8());

	 

	wpn_state =		 ::read_binary(4, current, output);
	need_to_update = ::read_binary(byte1, current, output);
	ammo_type =		 ::read_binary(byte1, current, output);
	m_bZoom =		 ::read_binary(byte1, current, output);
	m_cur_scope =	 ::read_binary(byte1, current, output);

	/*
// 4 bit 
	P.r_u8(wpn_state);
	

//CAN MOVE TO 1 U8 -> 4 bit data
	P.r_u8(need_to_update);
	P.r_u8(ammo_type);
	P.r_u8(m_bZoom);
	P.r_u8(m_cur_scope);

	*/
}
