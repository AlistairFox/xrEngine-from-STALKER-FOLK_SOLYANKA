#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"

#include "alife_simulator.h"
#include "alife_graph_registry.h"
#include "alife_time_manager.h"


#include "alife_object_registry.h"
#include "alife_spawn_registry.h"

#include "ai_space.h"
#include "level_graph.h"	   

#include "restriction_space.h"


bool game_sv_freemp::change_level(NET_Packet& net_packet, ClientID sender)
{
	if (ai().get_alife())
		return					(alife().change_level(net_packet));
	else
		return					(false);
}

#include "../xrEngine/x_ray.h"

void game_sv_freemp::restart_simulator(LPCSTR saved_game_name)
{
	shared_str& options = *alife().server_command_line();

	delete_data(m_alife_simulator);
	server().clear_ids();

	xr_strcpy(g_pGamePersistent->m_game_params.m_game_or_spawn, saved_game_name);
	xr_strcpy(g_pGamePersistent->m_game_params.m_new_or_load, "load");

	pApp->ls_header[0] = '\0';
	pApp->ls_tip_number[0] = '\0';
	pApp->ls_tip[0] = '\0';
	pApp->LoadBegin();

	m_alife_simulator = xr_new<CALifeSimulator>(&server(), &options);
	loaded_inventory = false;

	g_pGamePersistent->LoadTitle("st_client_synchronising");
	//g_pGamePersistent->LoadTitle();
	Device.PreCache(60, true, true);
	pApp->LoadEnd();
}

void game_sv_freemp::save_game(NET_Packet& net_packet, ClientID sender)
{
	if (!ai().get_alife())
		return;

	alife().save(net_packet);
}

bool game_sv_freemp::load_game(NET_Packet& net_packet, ClientID sender)
{
	if (!ai().get_alife())
		return					(inherited::load_game(net_packet, sender));

	shared_str						game_name;
	net_packet.r_stringZ(game_name);
	return	(alife().load_game(*game_name, true));
}



bool game_sv_freemp::SpawnItemToPos(LPCSTR section, Fvector3 position)
{
	if (!pSettings->section_exist(section))
	{
		Msg("! WARNING section \"%s\" doesnt exist", section);
		return false;
	}

	CSE_Abstract* E = spawn_begin(section);

	if (E->cast_human_abstract() || E->cast_monster_abstract())
	{

		if (!m_alife_simulator)
		{
			Msg("! You can't spawn \"%s\" because alife simulator is not initialized!", section);
			return false;
		}
		else
		{
			Msg("Alife_Simulator Finded");
		}

		u32 graph = ai().alife().graph().actor()->m_tGraphID;

		u32 LV = ai().get_level_graph()->vertex_id(position);

		if (ai().get_level_graph()->valid_vertex_id(LV))
			alife().spawn_item(section, position, ai().get_level_graph()->vertex_id(position), graph, 0xffff);
		else
			Msg("! Level vertex incorrect");

		F_entity_Destroy(E);
	}
	else if (E->cast_anomalous_zone())
	{
		CShapeData::shape_def		_shape;
		_shape.data.sphere.P.set(0.0f, 0.0f, 0.0f);
		_shape.data.sphere.R = 3;
		_shape.type = CShapeData::cfSphere;

		CSE_ALifeAnomalousZone* anomaly = E->cast_anomalous_zone();
		anomaly->assign_shapes(&_shape, 1);
		anomaly->m_owner_id = u32(-1);
		anomaly->m_space_restrictor_type = RestrictionSpace::eRestrictorTypeNone;

		anomaly->o_Position = position;
		spawn_end(anomaly, m_server->GetServerClient()->ID);
	}
	else
		if (smart_cast<CSE_ALifeCar*>(E))
		{
			position.y += 0.2;
			E->o_Position = position;
			spawn_end(E, m_server->GetServerClient()->ID);
		}
		else
		{
			E->o_Position = position;
			spawn_end(E, m_server->GetServerClient()->ID);
		}

	return true;
};


void game_sv_freemp::on_death(CSE_Abstract* e_dest, CSE_Abstract* e_src)
{
	inherited::on_death(e_dest, e_src);

	if (!ai().get_alife())
		return;

	alife().on_death(e_dest, e_src);
}



ALife::_TIME_ID game_sv_freemp::GetStartGameTime()
{
	if (ai().get_alife() && ai().alife().initialized())
		return(ai().alife().time_manager().start_game_time());
	else
		return(inherited::GetStartGameTime());
}

ALife::_TIME_ID game_sv_freemp::GetGameTime()
{
 
	if (ai().get_alife() && ai().alife().initialized())
		return(ai().alife().time_manager().game_time());
	else
		return(inherited::GetGameTime());
}

float game_sv_freemp::GetGameTimeFactor()
{
	if (ai().get_alife() && ai().alife().initialized())
		return(ai().alife().time_manager().time_factor());
	else
		return(inherited::GetGameTimeFactor());
}

void game_sv_freemp::SetGameTimeFactor(const float fTimeFactor)
{
	if (ai().get_alife() && ai().alife().initialized())
		return(alife().time_manager().set_time_factor(fTimeFactor));
	else
		return(inherited::SetGameTimeFactor(fTimeFactor));
}

void game_sv_freemp::ChangeGameTime(u32 day, u32 hour, u32 minute)
{
	if (ai().get_alife())
	{
		u32 value = day * 86400 + hour * 3600 + minute * 60;
		float fValue = static_cast<float> (value);
		value *= 1000;//msec		
		g_pGamePersistent->Environment().ChangeGameTime(fValue);
		alife().time_manager().change_game_time(value);
	}
}

void game_sv_freemp::ChangeGameTime(u32 value)
{
	if (ai().get_alife())
	{
		float fValue = static_cast<float> (value);
		g_pGamePersistent->Environment().ChangeGameTime(fValue);
		alife().time_manager().change_game_time(value);
	}
}
 

ALife::_TIME_ID game_sv_freemp::GetEnvironmentGameTime()
{
	if (ai().get_alife() && ai().alife().initialized())
		return(alife().time_manager().game_time());
	else
		return(inherited::GetGameTime());
}

float game_sv_freemp::GetEnvironmentGameTimeFactor()
{
	return(inherited::GetGameTimeFactor());
}

void game_sv_freemp::SetEnvironmentGameTimeFactor(const float fTimeFactor)
{
	return(inherited::SetGameTimeFactor(fTimeFactor));
}

 

void game_sv_freemp::WriteAlifeObjectsToClient(ClientID client_id)
{
	auto objects = &ai().alife().objects().objects();
 
	u32 size_spawn = 0, size_updates = 0;

	u16 id = 0;
	for (auto object : *objects)
	{
		id += 1;
  
		NET_Packet packet;
 		
		packet.w_begin(M_GAMEMESSAGE);
		packet.w_u32(M_ALIFE_OBJECTS_SPAWN);

		if (id == objects->size())
			packet.w_u8(1);
		else 
			packet.w_u8(0);

		packet.w_stringZ(object.second->s_name.c_str());
 	
		u32 packet_size = packet.w_tell();
		object.second->Spawn_WriteNoBeginPacket(packet, false);
		u32 position_spawn_end = packet.w_tell() - packet_size;
		object.second->UPDATE_Write(packet);
		u32 position_update_end = packet.w_tell() - position_spawn_end;

		u8 level_id = ai().game_graph().vertex(object.second->m_tGraphID)->level_id();
		shared_str name = ai().game_graph().header().level(level_id).name();

		//Msg("Object (%s) id (%d), level(%s), p_s(%u), p_u(%u)", object.second->name(), object.first, name.c_str(), position_spawn_end, position_update_end);
 
		server().SendTo(client_id, packet, net_flags(true));
	}
 
}

u32 ids_alife_objects = 0;

void game_sv_freemp::UpdateAlifeObjects()
{
	auto objects = &ai().alife().objects().objects();
	u32 size_spawn = 0, size_updates = 0;

	u32 id_last = ids_alife_objects;

	for (auto object : *objects)
	{ 
		if (!smart_cast<CSE_ALifeHumanStalker*>(object.second) &&
			!smart_cast<CSE_ALifeMonsterAbstract*>(object.second) && 
			!smart_cast<CSE_ALifeCreatureActor*>(object.second)  &&
			!smart_cast<CSE_ALifeOnlineOfflineGroup*> (object.second) &&
			!smart_cast<CSE_ALifeSmartZone*> (object.second)
		)
			continue;

		NET_Packet packet; 
		packet.w_begin(M_GAMEMESSAGE);
		packet.w_u32(M_ALIFE_OBJECTS_UPDATE);
		packet.w_u8(3);
		packet.w_u16(object.first);
		packet.w_stringZ(object.second->s_name);
		object.second->UPDATE_Write(packet);
		server().SendBroadcast(server().GetServerClient()->ID, packet, net_flags(false));
	}


}

void game_sv_freemp::RegisterUpdateAlife(CSE_ALifeDynamicObject* object, bool reg)
{
	if (Phase() != GAME_PHASE_INPROGRESS)
		return;
	

	if (reg)
	{
		NET_Packet packet;
		packet.w_begin(M_GAMEMESSAGE);
		packet.w_u32(M_ALIFE_OBJECTS_UPDATE);
		packet.w_u8(1);
		packet.w_u16(object->ID);
		packet.w_stringZ(object->s_name.c_str());
		object->Spawn_WriteNoBeginPacket(packet, false);
		object->UPDATE_Write(packet);
		server().SendBroadcast(server().GetServerClient()->ID, packet, net_flags(true));
	}
	else
	{
		NET_Packet packet;
		packet.w_begin(M_GAMEMESSAGE);
		packet.w_u32(M_ALIFE_OBJECTS_UPDATE);
		packet.w_u8(2);
		packet.w_u16(object->ID);
		server().SendBroadcast(server().GetServerClient()->ID, packet, net_flags(true));
	}
}
