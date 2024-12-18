#include "stdafx.h"
#include "game_cl_freemp.h"
#include "game_sv_freemp.h" 


void game_cl_freemp::ReadSpawnAlife(NET_Packet* packet)
{
	if (OnServer())
		return;

	data_networking_alife.SpawnNetSpawn += packet->B.count;

	u8 end = packet->r_u8();

	if (end == true)
		alife_objects_synchronized = true;

	shared_str name;
	packet->r_stringZ(name);

	if (!pSettings->section_exist(name.c_str()))
	{
		Msg("Cant Find Sec(%s)", name.c_str());
		return;
	}

	CSE_Abstract* entity = F_entity_Create(name.c_str());
	entity->Spawn_ReadNoBeginPacket(*packet);
	entity->UPDATE_Read(*packet);

	u16 id = entity->ID;

	CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(entity);

	if (dynamic)
	{
 		u32 level = 0;
		shared_str name_level;
		
 		if (&ai().game_graph())
		{
			level = ai().game_graph().vertex(dynamic->m_tGraphID)->level_id();
			name_level = ai().game_graph().header().level(level).name();
		}

		// Msg("Recived id[%d], level[%d][%s], alife_object[%s], name_replace (%s), POS[%.0f][%.0f][%.0f]", id, level, name_level.c_str(), name.c_str(),  entity->name_replace(), entity->o_Position.x, entity->o_Position.y, entity->o_Position.z);
	 
		alife_objects[id] = dynamic;
	  
		//CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(dynamic);
		//CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(dynamic);
		//CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(dynamic);
		CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(dynamic);
		CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(dynamic);

		if (group || zone) // stalker || changer || monster || 
		{
 			dynamic->on_register_client();
		}
	}
}

void game_cl_freemp::ReadUpdateAlife(NET_Packet* packet)
{
	if (!alife_objects_synchronized)
		return;

	data_networking_alife.UpdateNet += packet->B.count;
	if (m_last_net_per_second < Device.dwTimeGlobal)
	{
		m_last_net_per_second = Device.dwTimeGlobal + 1000;
		data_networking_alife.UpdateNet_ps = data_networking_alife.UpdateNet - m_last_net_per_second_kb;
		m_last_net_per_second_kb = data_networking_alife.UpdateNet;
	}
 
	u8 type = packet->r_u8();
	if (type == 1) // REGISTER
	{
		u16 id = packet->r_u16();
		shared_str name;
		packet->r_stringZ(name);


		if (!alife_objects[id])
		{
 			if (!pSettings->section_exist(name.c_str()))
  				return;
			Msg("Register Alife[%d] : Sec[%s]", id, name.c_str());
			CSE_Abstract* ent = F_entity_Create(name.c_str());
			CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(ent);

			if (dynamic)
			{
				dynamic->Spawn_ReadNoBeginPacket(*packet);
				dynamic->UPDATE_Read(*packet);

				alife_objects[id] = dynamic;
				// CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(dynamic);
				// CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(dynamic);
				// CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(dynamic);
				CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(dynamic);
				CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(dynamic);

				if ( group || zone) //stalker || changer || monster ||
				{
 					dynamic->on_register_client();
				}

			}
		}
	}
	else if (type == 2) // UNREGISTER
	{
		u16 id = packet->r_u16();
		CSE_ALifeDynamicObject* dynamic = alife_objects[id];
 
		Msg("UnRegister Alife[%d] : Section[%s] Replace[%s]", id, dynamic->name_replace(), dynamic->name());


		if (dynamic)
		{
			CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(dynamic);
			CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(dynamic);
			CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(dynamic);
			CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(dynamic);
			CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(dynamic);

			if (stalker || changer || monster || group || zone)
				dynamic->on_unregister_client();
			alife_objects.erase(id);
		}

	}
	else if (type == 3) // Update Full
	{
		u16 id = packet->r_u16();
		shared_str name_spawn;
		packet->r_stringZ(name_spawn);
		CSE_ALifeDynamicObject* object = GetAlifeObject(id);

		if (object)
		{
			object->UPDATE_Read(*packet);
			object->UPDATE_ReadScript(*packet);		  // Для вызова нужна функция в скрипте иле вылет 
		}
	}
	else if (type == 4)  // Update Position
	{
		u16 id = packet->r_u16();
		if (CSE_ALifeDynamicObject* obj = GetAlifeObject(id))
		{
			obj->o_Position = packet->r_vec3();
			obj->m_tGraphID = packet->r_u16();
			obj->m_tNodeID = packet->r_u16();
		}
	}
}
 