﻿////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Object_Base.cpp
//	Created 	: 19.09.2002
//  Modified 	: 16.07.2004
//	Author		: Oles Shyshkovtsov, Alexander Maksimchuk, Victor Reutskiy and Dmitriy Iassenev
//	Description : Server base object
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects.h"
#include "xrMessages.h"
#include "game_base_space.h"
#include "script_value_container_impl.h"
#include "clsid_game.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

#ifndef AI_COMPILER
#	include "object_factory.h"
#endif

#ifndef XRSE_FACTORY_EXPORTS
#	include "xrEProps.h"
	
	IPropHelper &PHelper()
	{
		NODEFAULT;
#	ifdef DEBUG
		return(*(IPropHelper*)0);
#	endif
	}

#	ifdef XRGAME_EXPORTS
#		include "ai_space.h"
#		include "alife_simulator.h"
#	endif // #ifdef XRGAME_EXPORTS
#endif

LPCSTR script_section = "script";
LPCSTR current_version = "current_server_entity_version";

u16	script_server_object_version	()
{
	static bool initialized		= false;
	static u16  script_version	= 0;
	if (!initialized) {
		initialized				= true;
		if (!pSettings->section_exist(script_section) || !pSettings->line_exist(script_section,current_version))
			script_version		= 0;
		script_version			= pSettings->r_u16(script_section,current_version);
	}
	return						(script_version);
}

////////////////////////////////////////////////////////////////////////////
// CPureServerObject
////////////////////////////////////////////////////////////////////////////
void CPureServerObject::save				(IWriter	&tMemoryStream)
{
}

void CPureServerObject::load				(IReader	&tFileStream)
{
}

void CPureServerObject::load				(NET_Packet	&tNetPacket)
{
}

void CPureServerObject::save				(NET_Packet	&tNetPacket)
{
}

////////////////////////////////////////////////////////////////////////////
// CSE_Abstract
////////////////////////////////////////////////////////////////////////////
CSE_Abstract::CSE_Abstract					(LPCSTR caSection)
{
	m_editor_flags.zero			();
	RespawnTime					= 0;
	net_Ready					= FALSE;
	ID							= 0xffff;
	ID_Parent					= 0xffff;
	ID_Phantom					= 0xffff;
	owner						= 0;
	m_gameType.SetDefaults		();
//.	s_gameid					= 0;
	s_RP						= 0xFE;			// Use supplied coords
	s_flags.assign				(0);
	s_name						= caSection;
	s_name_replace				= 0;			//xr_strdup("");
	o_Angle.set					(0.f,0.f,0.f);
	o_Position.set				(0.f,0.f,0.f);
	m_bALifeControl				= false;
	m_wVersion					= 0;
	m_script_version			= 0;
	m_tClassID					= TEXT2CLSID(pSettings->r_string(caSection,"class"));

//	m_spawn_probability			= 1.f;
	m_spawn_flags.zero			();
	m_spawn_flags.set			(flSpawnEnabled			,TRUE);
	m_spawn_flags.set			(flSpawnOnSurgeOnly		,TRUE);
	m_spawn_flags.set			(flSpawnSingleItemOnly	,TRUE);
	m_spawn_flags.set			(flSpawnIfDestroyedOnly	,TRUE);
	m_spawn_flags.set			(flSpawnInfiniteCount	,TRUE);
//	m_max_spawn_count			= 1;
//	m_spawn_control				= "";
//	m_spawn_count				= 0;
//	m_last_spawn_time			= 0;
//	m_next_spawn_time			= 0;
//	m_min_spawn_interval		= 0;
//	m_max_spawn_interval		= 0;
	m_ini_file					= 0;

	if (pSettings->line_exist(caSection,"custom_data"))
	{
		pcstr const raw_file_name	= pSettings->r_string(caSection,"custom_data");
		IReader const* config	= 0;
#ifdef XRGAME_EXPORTS
		if ( ai().get_alife() )
			config				= ai().alife().get_config( raw_file_name );
		else
#endif // #ifdef XRGAME_EXPORTS
		{
			string_path			file_name;
			FS.update_path		(file_name,"$game_config$", raw_file_name);
			if ( FS.exist(file_name) )
				config			= FS.r_open(file_name);
		}

		//Msg("Read Cfg = [%s]", raw_file_name);

		if ( config ) {
			int					size = config->length()*sizeof(char);
			LPSTR				temp = (LPSTR)_alloca(size + 1);
			CopyMemory			(temp,config->pointer(),size);
			temp[size]			= 0;
			m_ini_string		= temp;

#ifdef XRGAME_EXPORTS
		if ( NULL==ai().get_alife() )
#endif // #ifdef XRGAME_EXPORTS
		{
			IReader* _r	= (IReader*)config;
			FS.r_close(_r);
		}

		}
		else
			Msg					( "! cannot open config file %s", raw_file_name );
	}

#ifndef AI_COMPILER
	m_script_clsid				= object_factory().script_clsid(m_tClassID);
#endif
}

CSE_Abstract::~CSE_Abstract					()
{
	xr_free						(s_name_replace);
	xr_delete					(m_ini_file);
}

CSE_Visual* CSE_Abstract::visual			()
{
	return						(0);
}

ISE_Shape*  CSE_Abstract::shape				()
{
	return						(0);
}

CSE_Motion* CSE_Abstract::motion			()
{
	return						(0);
}

#include "xrServer_Objects_ALife.h"
#pragma warning(disable:4238)


extern ENGINE_API	bool g_dedicated_server;
CInifile &CSE_Abstract::spawn_ini			()
{
	// В целом можно сделать подмену на чтение файла
	if (!m_ini_file)
	{
		void* data = (void*)(m_ini_string.c_str());
		int size = m_ini_string.size();
		m_ini_file = xr_new<CInifile>( &IReader(data, size ), FS.get_path("$game_config$")->m_Path );
		
		if ( strstr ( Core.Params, "-dev" ) != 0 )
		if (ID != u16(-1) && this->m_ini_string.size() > 4 && g_dedicated_server)
		{
			CSE_ALifeObjectPhysic* physic = smart_cast<CSE_ALifeObjectPhysic*>(this);
			CSE_ALifeInventoryBox* inventory_box = smart_cast<CSE_ALifeInventoryBox*>(this);
			CSE_ALifeSpaceRestrictor* space_restrictor = smart_cast<CSE_ALifeSpaceRestrictor*>(this);
			CSE_ALifeObjectHangingLamp* lamps = smart_cast<CSE_ALifeObjectHangingLamp*>(this);

			string128 tmp_section;
			if (this->cast_smart_zone())
				sprintf(tmp_section, "world_spawns\\smart_terrains\\%s_game_id_%u.ltx", this->name_replace(), ID);
			else  if (this->cast_inventory_item())
				sprintf(tmp_section, "world_spawns\\inventory_items\\%s_game_id_%u.ltx", this->name_replace(), ID);
			else if (physic)
				sprintf(tmp_section, "world_spawns\\physic_objects\\%s_game_id_%u.ltx", this->name_replace(), ID);
			else if (inventory_box)
				sprintf(tmp_section, "world_spawns\\inventory_box\\%s_game_id_%u.ltx", this->name_replace(), ID);
			else if (space_restrictor)
				sprintf(tmp_section, "world_spawns\\space_restrictor\\%s_game_id_%u.ltx", this->name_replace(), ID);
			else if (lamps)
				sprintf(tmp_section, "world_spawns\\dynamics_lamps\\%s_game_id_%u.ltx", this->name_replace(), ID);
			else
				sprintf(tmp_section, "world_spawns\\unsorted\\%s_game_id_%u.ltx", this->name_replace(), ID);

			string_path path_save;
			FS.update_path(path_save, "$fs_root$", tmp_section);
			IWriter* w = FS.w_open(path_save);
			w->w_string(this->m_ini_string.c_str());
			FS.w_close(w);
		}
		 
	}
	return						(*m_ini_file);
}
	
void CSE_Abstract::Spawn_Write				(NET_Packet	&tNetPacket, BOOL bLocal)
{
	// generic
 
	tNetPacket.w_begin(M_SPAWN);
 
	tNetPacket.w_stringZ		(s_name			);
	tNetPacket.w_stringZ		(s_name_replace ?	s_name_replace : "");
	tNetPacket.w_u8				(0);
	tNetPacket.w_u8				(s_RP			);
	tNetPacket.w_vec3			(o_Position		);
	tNetPacket.w_vec3			(o_Angle		);
	tNetPacket.w_u16			(RespawnTime	);
	tNetPacket.w_u16			(ID				);
	tNetPacket.w_u16			(ID_Parent		);
	tNetPacket.w_u16			(ID_Phantom		);

	s_flags.set					(M_SPAWN_VERSION,TRUE);

	if (bLocal)
		tNetPacket.w_u16		(u16(s_flags.flags|M_SPAWN_OBJECT_LOCAL) );
	else
		tNetPacket.w_u16		(u16(s_flags.flags&~(M_SPAWN_OBJECT_LOCAL|M_SPAWN_OBJECT_ASPLAYER)));
	
	tNetPacket.w_u16			(SPAWN_VERSION);

	tNetPacket.w_u16			(m_gameType.m_GameType.get());
	
	tNetPacket.w_u16			(script_server_object_version());


	//client object custom data serialization SAVE
	u16 client_data_size		= (u16)client_data.size(); //не может быть больше 256 байт
	tNetPacket.w_u16			(client_data_size);
//	Msg							("SERVER:saving:save:%d bytes:%d:%s",client_data_size,ID,s_name_replace ? s_name_replace : "");
	if (client_data_size > 0)
	{
		tNetPacket.w			(&*client_data.begin(),client_data_size);
	}

	tNetPacket.w_u16			(m_tSpawnID);
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
	u32	position				= tNetPacket.w_tell();
	tNetPacket.w_u16			(0);
	STATE_Write					(tNetPacket);
	u16 size					= u16(tNetPacket.w_tell() - position);
//#ifdef XRSE_FACTORY_EXPORTS
	R_ASSERT3					((m_tClassID == CLSID_SPECTATOR) || (size > sizeof(size)), "object isn't successfully saved, get your backup :(",name_replace());
//#endif
	tNetPacket.w_seek			(position,&size,sizeof(u16));
}

enum EGameTypes {
	GAME_ANY							= 0,
	GAME_SINGLE							= 1,
	GAME_DEATHMATCH						= 2,
//	GAME_CTF							= 3,
//	GAME_ASSAULT						= 4,	// Team1 - assaulting, Team0 - Defending
	GAME_CS								= 5,
	GAME_TEAMDEATHMATCH					= 6,
	GAME_ARTEFACTHUNT					= 7,
	GAME_CAPTURETHEARTEFACT				= 8,

	//identifiers in range [100...254] are registered for script game type
	GAME_DUMMY							= 255	// temporary game type
};

BOOL CSE_Abstract::Spawn_Read				(NET_Packet	&tNetPacket)
{
 
	u16							dummy16;
	// generic
	tNetPacket.r_begin(dummy16);
	R_ASSERT(M_SPAWN == dummy16);
 
 
	tNetPacket.r_stringZ		(s_name			);
	
	string256					temp;
	tNetPacket.r_stringZ		(temp);
	set_name_replace			(temp);
	u8							temp_gt;
	tNetPacket.r_u8				(temp_gt/*s_gameid*/		);
	tNetPacket.r_u8				(s_RP			);
	tNetPacket.r_vec3			(o_Position		);
	tNetPacket.r_vec3			(o_Angle		);
	tNetPacket.r_u16			(RespawnTime	);
	tNetPacket.r_u16			(ID				);
	tNetPacket.r_u16			(ID_Parent		);
	tNetPacket.r_u16			(ID_Phantom		);

	tNetPacket.r_u16			(s_flags.flags	); 
	
	// dangerous!!!!!!!!!
	if (s_flags.is(M_SPAWN_VERSION))
		tNetPacket.r_u16		(m_wVersion);
	
	if(m_wVersion > 120)
	{
		u16 gt;
		tNetPacket.r_u16			(gt);
		m_gameType.m_GameType.assign(gt);
	}else
		m_gameType.SetDefaults		();

	if (0==m_wVersion) {
		tNetPacket.r_pos		-= sizeof(u16);
		m_wVersion				= 0;
        return					FALSE;
	}

	if (m_wVersion > 69)
		m_script_version		= tNetPacket.r_u16();

	// read specific data

	//client object custom data serialization LOAD
	if (m_wVersion > 70) {
		u16 client_data_size	= (m_wVersion > 93) ? tNetPacket.r_u16() : tNetPacket.r_u8(); //не может быть больше 256 байт
		if (client_data_size > 0) {
//			Msg					("SERVER:loading:load:%d bytes:%d:%s",client_data_size,ID,s_name_replace ? s_name_replace : "");
			client_data.resize	(client_data_size);
			tNetPacket.r		(&*client_data.begin(),client_data_size);
		}
		else
			client_data.clear	();
	}
	else
		client_data.clear		();

	if (m_wVersion > 79)
		tNetPacket.r_u16			(m_tSpawnID);

	if (m_wVersion < 112) {
		if (m_wVersion > 82)
			tNetPacket.r_float		();//m_spawn_probability);

		if (m_wVersion > 83) {
			tNetPacket.r_u32		();//m_spawn_flags.assign(tNetPacket.r_u32());
			xr_string				temp;
			tNetPacket.r_stringZ	(temp);//tNetPacket.r_stringZ(m_spawn_control);
			tNetPacket.r_u32		();//m_max_spawn_count);
			// this stuff we do not need even in case of uncomment
			tNetPacket.r_u32		();//m_spawn_count);
			tNetPacket.r_u64		();//m_last_spawn_time);
		}

		if (m_wVersion > 84) {
			tNetPacket.r_u64		();//m_min_spawn_interval);
			tNetPacket.r_u64		();//m_max_spawn_interval);
		}
	}

	u16							size;
	tNetPacket.r_u16			(size);	// size
	bool b1						= (m_tClassID == CLSID_SPECTATOR);
	bool b2						= (size > sizeof(size)) || (tNetPacket.inistream!=NULL);
	R_ASSERT3					( (b1 || b2),"cannot read object, which is not successfully saved :(",name_replace());
	STATE_Read					(tNetPacket,size);
	return						TRUE;
}

void	CSE_Abstract::load			(NET_Packet	&tNetPacket)
{
	CPureServerObject::load		(tNetPacket);
	u16 client_data_size		= (m_wVersion > 93) ? tNetPacket.r_u16() : tNetPacket.r_u8(); //не может быть больше 256 байт
	if (client_data_size > 0) {
#ifdef DEBUG
//		Msg						("SERVER:loading:load:%d bytes:%d:%s",client_data_size,ID,s_name_replace ? s_name_replace : "");
#endif // DEBUG
		client_data.resize		(client_data_size);
		tNetPacket.r			(&*client_data.begin(),client_data_size);
	}
	else {
#ifdef DEBUG
		if (!client_data.empty())
			Msg					("CSE_Abstract::load: client_data is cleared for [%d][%s]",ID,name_replace());
#endif // DEBUG
        client_data.clear		();
	}
}

CSE_Abstract *CSE_Abstract::base	()
{
	return						(this);
}

const CSE_Abstract *CSE_Abstract::base	() const
{
	return						(this);
}

CSE_Abstract *CSE_Abstract::init	()
{
	return						(this);
}

LPCSTR		CSE_Abstract::name			() const
{
	return	(*s_name);
}

LPCSTR		CSE_Abstract::name_replace	() const
{
	return	(s_name_replace);
}

Fvector&	CSE_Abstract::position		()
{
	return	(o_Position);
}

Fvector&	CSE_Abstract::angle			()
{
	return	(o_Angle);
}

Flags16&	CSE_Abstract::flags			()
{
	return	(s_flags);
}

xr_token game_types[]={
	{ "any_game",				eGameIDNoGame				},
	{ "single",					eGameIDSingle				},
	{ "deathmatch",				eGameIDDeathmatch			},
	{ "team_deathmatch",		eGameIDTeamDeathmatch		},
	{ "artefacthunt",			eGameIDArtefactHunt			},
	{ "capture_the_artefact",	eGameIDCaptureTheArtefact	},
	{ "freemp",					eGameIDFreeMp				},
	{ "roleplay",					eGameIDRolePlay				},
	//eGameIDDominationZone
	//eGameIDTeamDominationZone
	{ 0,				0				}
};

#ifndef XRGAME_EXPORTS
void CSE_Abstract::FillProps(LPCSTR pref, PropItemVec& items)
{
#ifdef XRSE_FACTORY_EXPORTS
    m_gameType.FillProp(pref, items);
#endif // #ifdef XRSE_FACTORY_EXPORTS
/*
#ifdef XRGAME_EXPORTS
#	ifdef DEBUG
	PHelper().CreateToken8		(items,	PrepareKey(pref,"Game Type"),			&s_gameid,		game_types);
    PHelper().CreateU16			(items,	PrepareKey(pref, "Respawn Time (s)"),	&RespawnTime,	0,43200);

*/
}

void CSE_Abstract::FillProp					(LPCSTR pref, PropItemVec &items)
{
	CScriptValueContainer::assign();
	CScriptValueContainer::clear();
	FillProps					(pref,items);
}
#endif // #ifndef XRGAME_EXPORTS

bool CSE_Abstract::validate					()
{
	return						(true);
}

/**
void CSE_Abstract::save_update				(NET_Packet &tNetPacket)
{
	tNetPacket.w				(&m_spawn_count,sizeof(m_spawn_count));
	tNetPacket.w				(&m_last_spawn_time,sizeof(m_last_spawn_time));
	tNetPacket.w				(&m_next_spawn_time,sizeof(m_next_spawn_time));
}

void CSE_Abstract::load_update				(NET_Packet &tNetPacket)
{
	tNetPacket.r				(&m_spawn_count,sizeof(m_spawn_count));
	tNetPacket.r				(&m_last_spawn_time,sizeof(m_last_spawn_time));
	tNetPacket.r				(&m_next_spawn_time,sizeof(m_next_spawn_time));
}
/**/
