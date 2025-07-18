////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_storage_manager.cpp
//	Created 	: 25.12.2002
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator storage manager
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_storage_manager.h"
#include "alife_simulator_header.h"
#include "alife_time_manager.h"
#include "alife_spawn_registry.h"
#include "alife_object_registry.h"
#include "alife_graph_registry.h"
#include "alife_group_registry.h"
#include "alife_registry_container.h"
#include "xrserver.h"
#include "level.h"
#include "../xrEngine/x_ray.h"
#include "saved_game_wrapper.h"
#include "string_table.h"
#include "../xrEngine/igame_persistent.h"
#include "autosave_manager.h"

#include "actor_mp_server.h"	

#include "InventoryBox.h"
 

using namespace ALife;

extern string_path g_last_saved_game;

CALifeStorageManager::~CALifeStorageManager	()
{
	*g_last_saved_game			= 0;
}

bool CALifeStorageManager::save	(LPCSTR save_name_no_check, bool update_name)
{
	LPCSTR game_saves_path		= FS.get_path("$game_saves$")->m_Path;

	string_path					save_name;
	strncpy_s					(save_name, sizeof(save_name), save_name_no_check, sizeof(save_name)-5-xr_strlen(SAVE_EXTENSION)-xr_strlen(game_saves_path));

	xr_strcpy					(g_last_saved_game, save_name);

	string_path					save;
	xr_strcpy					(save,m_save_name);
	if (save_name)
	{
		strconcat				(sizeof(m_save_name), m_save_name, save_name, SAVE_EXTENSION);
	}
	else {
		if (!xr_strlen(m_save_name)) {
			Log					("There is no file name specified!");
			return false;
		}
	}

	u32							source_count;
	u32							dest_count;
	void						*dest_data;
	{
		CMemoryWriter			stream;
		header().save			(stream);
		time_manager().save		(stream);
		spawns().save			(stream);
		objects().save			(stream);
		registry().save			(stream);

		source_count			= stream.tell();
		void					*source_data = stream.pointer();
		dest_count				= rtc_csize(source_count);
		dest_data				= xr_malloc(dest_count);
		dest_count				= rtc_compress(dest_data,dest_count,source_data,source_count);
	}

	string_path					temp;
	FS.update_path				(temp,"$game_saves$",m_save_name);
	IWriter						*writer = FS.w_open(temp);
	writer->w_u32				(u32(-1));
	writer->w_u32				(ALIFE_VERSION);
	
	writer->w_u32				(source_count);
	writer->w					(dest_data,dest_count);
	xr_free						(dest_data);
	FS.w_close					(writer);
#ifdef DEBUG
	Msg							("* Game %s is successfully saved to file '%s' (%d bytes compressed to %d)",m_save_name,temp,source_count,dest_count + 4);
#else // DEBUG
	Msg							("* Game %s is successfully saved to file '%s'",m_save_name,temp);
#endif // DEBUG

	if (!update_name)
		xr_strcpy					(m_save_name,save);

	return true;
}

void CALifeStorageManager::load	(void *buffer, const u32 &buffer_size, LPCSTR file_name)
{
	IReader						source(buffer,buffer_size);
	header().load				(source);
	time_manager().load			(source);
	spawns().load				(source,file_name);
	graph().on_load				();
	objects().load				(source);

	VERIFY						(can_register_objects());

	can_register_objects		(false);
	
	CALifeObjectRegistry::OBJECT_REGISTRY::iterator	B = objects().objects().begin();
	CALifeObjectRegistry::OBJECT_REGISTRY::iterator	E = objects().objects().end();
	CALifeObjectRegistry::OBJECT_REGISTRY::iterator	I;

	for (I = B; I != E; ++I)
	{
		ALife::_OBJECT_ID		id = (*I).second->ID;
		if (smart_cast<CSE_ActorMP*>(I->second))
			continue;

		//if (smart_cast<CSE_ALifeInventoryBox*>((I->second)))
		//	continue;

		u32 parrentID = (*I).second->ID_Parent;
		CObject* obj = Level().Objects.net_Find(parrentID);

		if (smart_cast<CInventoryBox*>(obj))
			continue;

		Msg("Load ID_OBJ [%d] Name [%s] Spawn[%s], Parrent[%d], PAR_NAME[%s]", 
			I->second->ID, I->second->name_replace(), I->second->s_name.c_str(), I->second->ID_Parent, 
			server().ID_to_entity(I->second->ID_Parent) ? server().ID_to_entity(I->second->ID_Parent)->name() : "NOT FIND PARRENT"
		);

		(*I).second->ID			= server().PerformIDgen(id);
		VERIFY					(id == (*I).second->ID);
		register_object			((*I).second,false);
	}

	registry().load				(source);

	can_register_objects		(true);

	for (I = B; I != E; ++I)
		(*I).second->on_register();

	if (!g_pGameLevel)
		return;

	Level().autosave_manager().on_game_loaded	();
}

bool CALifeStorageManager::load	(LPCSTR save_name_no_check)
{
	LPCSTR game_saves_path		= FS.get_path("$game_saves$")->m_Path;

	string_path					save_name;
	strncpy_s					(save_name, sizeof(save_name), save_name_no_check, sizeof(save_name)-5-xr_strlen(SAVE_EXTENSION)-xr_strlen(game_saves_path));

	CTimer						timer;
	timer.Start					();

	string_path					save;
	xr_strcpy					(save,m_save_name);
	if (!save_name) {
		if (!xr_strlen(m_save_name))
			R_ASSERT2			(false,"There is no file name specified!");
	}
	else
	{
		strconcat				(sizeof(m_save_name), m_save_name, save_name, SAVE_EXTENSION);
	}
	string_path					file_name;
	FS.update_path				(file_name,"$game_saves$",m_save_name);

	xr_strcpy					(g_last_saved_game, save_name);
 
	IReader						*stream;
	stream						= FS.r_open(file_name);
	if (!stream) {
		Msg						("* Cannot find saved game %s",file_name);
		xr_strcpy				(m_save_name,save);
		return					(false);
	}

	CHECK_OR_EXIT				(CSavedGameWrapper::valid_saved_game(*stream),make_string("%s\nSaved game version mismatch or saved game is corrupted",file_name));
 
	string512					temp;
	strconcat					(sizeof(temp),temp,CStringTable().translate("st_loading_saved_game").c_str()," \"",save_name,SAVE_EXTENSION,"\"");
	g_pGamePersistent->LoadTitle(temp);
 
//g_pGamePersistent->LoadTitle();

	unload						();
	reload						(m_section);

	u32							source_count = stream->r_u32();
	void						*source_data = xr_malloc(source_count);
	rtc_decompress				(source_data,source_count,stream->pointer(),stream->length() - 3*sizeof(u32));
	FS.r_close					(stream);
	load						(source_data, source_count, file_name);
	xr_free						(source_data);

	groups().on_after_game_load	();

	VERIFY						(graph().actor());
	
	Msg							("* Game %s is successfully loaded from file '%s' (%.3fs)",save_name, file_name,timer.GetElapsed_sec());

	return						(true);
}

void CALifeStorageManager::save	(NET_Packet &net_packet)
{
	prepare_objects_for_save	();

	shared_str					game_name;
	net_packet.r_stringZ		(game_name);
	save						(*game_name,!!net_packet.r_u8());
}

void CALifeStorageManager::prepare_objects_for_save	()
{
	Level().ClientSend			();
	Level().ClientSave			();
}
