#pragma once

enum
{
	MASTER_SERVER_SERVER = 0,
	MASTER_SERVER_CLIENT
};

enum
{
	MASTER_SERVER_CHAT_DATA = 0,
	MASTER_SERVER_SAVE_DATA,
	MASTER_SERVER_SPAWN_DATA,
	MASTER_SERVER_QVESTS_DATA,
	MASTER_SERVER_CMD,
	MASTER_SERVER_MSGS,
	MASTER_SERVER_KILL_PROCESS,

	MASTER_SERVER_RECVEST_MAP,
	MASTER_SERVER_RECVEST_MEMORY
};

struct CSE_ENTITY_MS
{
	u16 id;
	u16 id_parrent;
	u16 ID_Phantom;
	CLASS_ID ClassID;
	u16 m_tSpawnID;

	//xr_vector<u16> ids_child;

	bool m_bALifeControl;
	u32 m_script_clsid;


	Fvector pos;
	Fvector dir;
 

	shared_str s_name_replace;
	shared_str s_name;
	shared_str m_ini_string;


	void SetDataFromCSE(CSE_Abstract* ent)
	{
		id = ent->ID;
		id_parrent = ent->ID_Parent;
		ID_Phantom = ent->ID_Phantom;
		ClassID = ent->m_tClassID;
		m_tSpawnID = ent->m_tSpawnID;
		
		s_name = ent->s_name.c_str();
 		s_name_replace = ent->name_replace();
		m_ini_string = ent->m_ini_string.c_str();

		m_bALifeControl = ent->m_bALifeControl;
		m_script_clsid = ent->m_script_clsid;

		pos = ent->o_Position;
		dir = ent->o_Angle;
	}
    
	void Read(NET_Packet& P) 
	{
		P.r_stringZ(s_name);
 		P.r_stringZ(s_name_replace);
		P.r_stringZ(m_ini_string);
 
		m_bALifeControl = P.r_u8();
		P.r_u32(m_script_clsid);
		P.r_u16(id);
		P.r_u16(id_parrent);
		P.r_u16(ID_Phantom);
		P.r_u16(m_tSpawnID);

		P.r(&ClassID, sizeof(ClassID));

		P.r_vec3(pos);
		P.r_vec3(dir);
	}
	void Write(NET_Packet& P)
	{
		P.w_stringZ(s_name);
		P.w_stringZ(s_name_replace);
		P.w_stringZ(m_ini_string);

		P.w_u8(m_bALifeControl);
		P.w_u32(m_script_clsid);
		P.w_u16(id);
		P.w_u16(id_parrent);
		P.w_u16(ID_Phantom);
		P.w_u16(m_tSpawnID);

		P.w(&ClassID, sizeof(ClassID));

		P.w_vec3(pos);
		P.w_vec3(dir);
	}


};

typedef xr_vector<CSE_ENTITY_MS*> vec_entitys;

struct client_DATA
{
	MasterServerID master_client_id;
	char IP_V4[48];
	LPCSTR map;

	int entitys;
	vec_entitys ents;
};