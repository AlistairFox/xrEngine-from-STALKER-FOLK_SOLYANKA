#include "stdafx.h"
#include "game_sv_freemp.h"
#include "Level.h"

#include "alife_simulator.h"
#include "alife_graph_registry.h"
#include "alife_time_manager.h"

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

		u32 LV = ai().get_level_graph()->vertex_id(position);


		if (ai().get_level_graph()->valid_vertex_id(LV))
		{
			u32 GV = ai().cross_table().vertex(LV).game_vertex_id();
			Msg("LV[%d], GV[%d]", LV, GV);
			alife().spawn_item(section, position, LV, GV, 0xffff);
		}
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

shared_str game_sv_freemp::level_name(const shared_str& server_options) const
{
	if (!ai().get_alife())
		return				(inherited::level_name(server_options));
	return					(alife().level_name());
}

shared_str game_sv_freemp::name_map_alife()
{
	if (ai().get_alife())
		return					(alife().level_name());

	return "no_alife";
}
