#pragma once
struct Weapon_State_Network
{
	u8 m_u8NumItems;    // 1 bit				     // 1 byte

	float m_fCondition;   // состояние u8 8 bit      //1 byte В данный момент
	
	flags8 m_addon_flags; // 4 bit //max Value 16    //1 byte
	u8 wpn_state;		  // 4 bit //max Value 16    //1 byte

	u8 a_elapsed;		  // 8 bit					 //1 byte 

	//для сжатия 
	u8 ammo_type;		// 1 bit					 // 1 byte
	u8 m_bZoom;			// 1 bit					 // 1 byte
	u8 m_cur_scope;		// 1 bit					 // 1 byte
	u8 need_to_update;		// 1 bit					 // 1 byte

	
	//Если сжать		//	4byte + 5 bit			 // Total 10 byte  

	Weapon_State_Network();
	void fill_state();
	void write_state(NET_Packet &P);
	void read_state(NET_Packet& P);
};

