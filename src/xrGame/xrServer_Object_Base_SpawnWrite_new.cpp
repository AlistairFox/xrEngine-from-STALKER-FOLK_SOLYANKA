#include "stdafx.h"
#include "xrServer_Objects.h"
#include "clsid_game.h"


u16	script_server_object_version();
 


void CSE_Abstract::Spawn_WriteNoBeginPacket(NET_Packet& tNetPacket, BOOL bLocal)
{
	// generic
	tNetPacket.w_stringZ(s_name);
	tNetPacket.w_stringZ(s_name_replace ? s_name_replace : "");
	tNetPacket.w_u8(0);
	tNetPacket.w_u8(s_RP);
	tNetPacket.w_vec3(o_Position);
	tNetPacket.w_vec3(o_Angle);
	tNetPacket.w_u16(RespawnTime);
	tNetPacket.w_u16(ID);
	tNetPacket.w_u16(ID_Parent);
	tNetPacket.w_u16(ID_Phantom);

	s_flags.set(M_SPAWN_VERSION, TRUE);

	if (bLocal)
		tNetPacket.w_u16(u16(s_flags.flags | M_SPAWN_OBJECT_LOCAL));
	else
		tNetPacket.w_u16(u16(s_flags.flags & ~(M_SPAWN_OBJECT_LOCAL | M_SPAWN_OBJECT_ASPLAYER)));

	tNetPacket.w_u16(SPAWN_VERSION);

	tNetPacket.w_u16(m_gameType.m_GameType.get());

	tNetPacket.w_u16(script_server_object_version());


	//client object custom data serialization SAVE
	u16 client_data_size = (u16)client_data.size(); //не может быть больше 256 байт
	tNetPacket.w_u16(client_data_size);
	//	Msg							("SERVER:saving:save:%d bytes:%d:%s",client_data_size,ID,s_name_replace ? s_name_replace : "");
	if (client_data_size > 0)
	{
		tNetPacket.w(&*client_data.begin(), client_data_size);
	}

	tNetPacket.w_u16(m_tSpawnID);
	//	tNetPacket.w_float			(m_spawn_probability);
	//	tNetPacket.w_u32			(m_spawn_flags.get());
	//	tNetPacket.w_stringZ		(m_spawn_control);
	//	tNetPacket.w_u32			(m_max_spawn_count);
	//	tNetPacket.w_u64			(m_min_spawn_interval);
	//	tNetPacket.w_u64			(m_max_spawn_interval);

#ifdef XRSE_FACTORY_EXPORTS
	CScriptValueContainer::assign();
#endif

	// write specific data
	u32	position = tNetPacket.w_tell();
	tNetPacket.w_u16(0);
	STATE_Write(tNetPacket);
	u16 size = u16(tNetPacket.w_tell() - position);
	//#ifdef XRSE_FACTORY_EXPORTS
	R_ASSERT3((m_tClassID == CLSID_SPECTATOR) || (size > sizeof(size)), "object isn't successfully saved, get your backup :(", name_replace());
	//#endif
	tNetPacket.w_seek(position, &size, sizeof(u16));
}


BOOL CSE_Abstract::Spawn_ReadNoBeginPacket(NET_Packet& tNetPacket)
{
	tNetPacket.r_stringZ(s_name);

	string256					temp;
	tNetPacket.r_stringZ(temp);
	set_name_replace(temp);
	u8							temp_gt;
	tNetPacket.r_u8(temp_gt/*s_gameid*/);
	tNetPacket.r_u8(s_RP);
	tNetPacket.r_vec3(o_Position);
	tNetPacket.r_vec3(o_Angle);
	tNetPacket.r_u16(RespawnTime);
	tNetPacket.r_u16(ID);
	tNetPacket.r_u16(ID_Parent);
	tNetPacket.r_u16(ID_Phantom);

	tNetPacket.r_u16(s_flags.flags);

	// dangerous!!!!!!!!!
	if (s_flags.is(M_SPAWN_VERSION))
		tNetPacket.r_u16(m_wVersion);

	if (m_wVersion > 120)
	{
		u16 gt;
		tNetPacket.r_u16(gt);
		m_gameType.m_GameType.assign(gt);
	}
	else
		m_gameType.SetDefaults();

	if (0 == m_wVersion) {
		tNetPacket.r_pos -= sizeof(u16);
		m_wVersion = 0;
		return					FALSE;
	}

	if (m_wVersion > 69)
		m_script_version = tNetPacket.r_u16();

	// read specific data

	//client object custom data serialization LOAD
	if (m_wVersion > 70) {
		u16 client_data_size = (m_wVersion > 93) ? tNetPacket.r_u16() : tNetPacket.r_u8(); //не может быть больше 256 байт
		if (client_data_size > 0) {
			//			Msg					("SERVER:loading:load:%d bytes:%d:%s",client_data_size,ID,s_name_replace ? s_name_replace : "");
			client_data.resize(client_data_size);
			tNetPacket.r(&*client_data.begin(), client_data_size);
		}
		else
			client_data.clear();
	}
	else
		client_data.clear();

	if (m_wVersion > 79)
		tNetPacket.r_u16(m_tSpawnID);

	if (m_wVersion < 112) {
		if (m_wVersion > 82)
			tNetPacket.r_float();//m_spawn_probability);

		if (m_wVersion > 83) {
			tNetPacket.r_u32();//m_spawn_flags.assign(tNetPacket.r_u32());
			xr_string				temp;
			tNetPacket.r_stringZ(temp);//tNetPacket.r_stringZ(m_spawn_control);
			tNetPacket.r_u32();//m_max_spawn_count);
			// this stuff we do not need even in case of uncomment
			tNetPacket.r_u32();//m_spawn_count);
			tNetPacket.r_u64();//m_last_spawn_time);
		}

		if (m_wVersion > 84) {
			tNetPacket.r_u64();//m_min_spawn_interval);
			tNetPacket.r_u64();//m_max_spawn_interval);
		}
	}

	u16							size;
	tNetPacket.r_u16(size);	// size
	bool b1 = (m_tClassID == CLSID_SPECTATOR);
	bool b2 = (size > sizeof(size)) || (tNetPacket.inistream != NULL);
	R_ASSERT3((b1 || b2), "cannot read object, which is not successfully saved :(", name_replace());
	STATE_Read(tNetPacket, size);
	return						TRUE;
}