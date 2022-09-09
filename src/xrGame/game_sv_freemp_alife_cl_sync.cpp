#include "StdAfx.h"
#include "game_sv_freemp.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"

void game_sv_freemp::WriteAlifeObjectsToClient(ClientID client_id)
{
	if (!ai().get_alife())
		return;

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

		//u8 level_id = ai().game_graph().vertex(object.second->m_tGraphID)->level_id();
		//shared_str name = ai().game_graph().header().level(level_id).name();

		//Msg("Object (%s) id (%d), level(%s), p_s(%u), p_u(%u)", object.second->name(), object.first, name.c_str(), position_spawn_end, position_update_end);

		server().SendTo(client_id, packet, net_flags(true));
	}

}

void game_sv_freemp::UpdateAlifeObjects()
{
	if (!ai().get_alife())
		return;

	u32 packet_size = 0;
	if (Device.dwTimeGlobal - last_alife_update_time > 1 * 1000)
	{
		last_alife_update_time = Device.dwTimeGlobal;
		auto objects = &ai().alife().objects().objects();
		u32 size_spawn = 0, size_updates = 0;

		for (auto object : *objects)
		{
			if (!smart_cast<CSE_ALifeHumanStalker*>(object.second) &&
				!smart_cast<CSE_ALifeMonsterAbstract*>(object.second) &&
			//	!smart_cast<CSE_ALifeCreatureActor*>(object.second) &&
				!smart_cast<CSE_ALifeOnlineOfflineGroup*> (object.second) &&
				!smart_cast<CSE_ALifeSmartZone*> (object.second)
				)
				continue;

			if (old_export_pos[object.first].pos.distance_to(object.second->position()) > 10 || old_export_pos[object.first].time < Device.dwTimeGlobal)
			{
				NET_Packet packet;
				packet.w_begin(M_GAMEMESSAGE);
				packet.w_u32(M_ALIFE_OBJECTS_UPDATE);
				packet.w_u8(3);
				packet.w_u16(object.first);
				packet.w_stringZ(object.second->s_name);
				object.second->UPDATE_Write(packet);
				object.second->UPDATE_WriteScript(packet);	  // Для вызова нужна функция в скрипте иле вылет
			
				update_data data;
				data.pos = object.second->position();
				data.time = Device.dwTimeGlobal + Random.randI(3000, 6400);  

				old_export_pos[object.first] = data;
	 
				packet_size += packet.w_tell();
				server().SendBroadcast(server().GetServerClient()->ID, packet, net_flags(false));
			}
		}
		//DEBUG INFO 
	 	/*
	//	Msg("Update Size [%d]", packet_size);

		NET_Packet packet;
		GenerateGameMessage(packet);

		string32 buf;
		sprintf(buf, "%d", packet_size);

		packet.w_u32(GAME_EVENT_NEWS_MESSAGE);
 		packet.w_stringZ("КОЛ-ВО КБ:");
		packet.w_stringZ(buf);
		packet.w_stringZ("ui_inGame2_Predmet_otdan");
		server().SendBroadcast(server().GetServerClient()->ID, packet, net_flags(true));
		 */
	}


}

void game_sv_freemp::UpdateAlifeObjectsPOS()
{
	if (!ai().get_alife())
		return;

	u32 packet_size = 0;

	/*
	if (Device.dwTimeGlobal - last_alife_update_time_pos > 1000)
	{
		last_alife_update_time_pos = Device.dwTimeGlobal;
		auto objects = &ai().alife().objects().objects();

		for (auto object : *objects)
		{
			if (!smart_cast<CSE_ALifeHumanStalker*>(object.second) &&
				!smart_cast<CSE_ALifeMonsterAbstract*>(object.second) &&
				!smart_cast<CSE_ALifeCreatureActor*>(object.second) &&
				!smart_cast<CSE_ALifeOnlineOfflineGroup*> (object.second) &&
				!smart_cast<CSE_ALifeSmartZone*> (object.second)
				)
				continue;

			NET_Packet packet;
			packet.w_begin(M_GAMEMESSAGE);
			packet.w_u32(M_ALIFE_OBJECTS_UPDATE);
			packet.w_u8(4);
			packet.w_u16(object.second->ID);
			packet.w_vec3(object.second->position());
			packet.w_u16(object.second->m_tGraphID);
			packet.w_u16(object.second->m_tNodeID);
			server().SendBroadcast(server().GetServerClient()->ID, packet, net_flags(false));
			packet_size += packet.w_tell();
		}

		Msg("Update Size POS [%d]", packet_size);
	}
	*/

}

void game_sv_freemp::RegisterUpdateAlife(CSE_ALifeDynamicObject* object, bool reg)
{
	if (Phase() != GAME_PHASE_INPROGRESS)
		return;
	if (!ai().get_alife())
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
