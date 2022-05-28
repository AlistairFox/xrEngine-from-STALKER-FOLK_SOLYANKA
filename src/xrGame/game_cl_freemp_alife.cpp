#include "stdafx.h"
#include "game_cl_freemp.h"

#include "game_sv_freemp.h" 


void game_cl_freemp::ReadSpawnAlife(NET_Packet* packet)
{
	if (OnServer())
		return;

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
		/*
		u32 level = 0;
		shared_str name_level;
		
	
		if (&ai().game_graph())
		{
			level = ai().game_graph().vertex(dynamic->m_tGraphID)->level_id();
			name_level = ai().game_graph().header().level(level).name();
		}

		Msg("Recived id[%d], level[%d][%s], alife_object[%s], name_replace (%s), POS[%.0f][%.0f][%.0f]", id, level, name_level.c_str(), name.c_str(),  entity->name_replace(), entity->o_Position.x, entity->o_Position.y, entity->o_Position.z);
		*/
		alife_objects[id] = dynamic;
	}
}

void game_cl_freemp::ReadUpdateAlife(NET_Packet* packet)
{
	if (!alife_objects_synchronized || !alife_objects_registered)
		return;

	u8 type = packet->r_u8();
	if (type == 1)
	{
		u16 id = packet->r_u16();
		shared_str name;
		packet->r_stringZ(name);


		if (!alife_objects[id])
		{
			//Msg("---Register New Alife Object Client (%d) (%s)", id, name.c_str());

			if (!pSettings->section_exist(name.c_str()))
			{
				Msg("Cant Find Sec(%s)", name.c_str());
				return;
			}

			CSE_Abstract* ent = F_entity_Create(name.c_str());
			CSE_ALifeDynamicObject* dynamic = smart_cast<CSE_ALifeDynamicObject*>(ent);

			if (dynamic)
			{
				dynamic->Spawn_ReadNoBeginPacket(*packet);
				dynamic->UPDATE_Read(*packet);

				alife_objects[id] = dynamic;
				CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(dynamic);
				CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(dynamic);
				CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(dynamic);
				CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(dynamic);
				CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(dynamic);

				if (stalker || changer || monster || group || zone)
				{
					//Msg("ID: %d, Name %s, spawn_name %s", dynamic->ID, dynamic->name_replace(), dynamic->s_name.c_str());
					dynamic->on_register_client();
				}
			}
		}
	}
	else if (type == 2)
	{
		u16 id = packet->r_u16();

		CSE_ALifeDynamicObject* dynamic = alife_objects[id];

		if (dynamic)
		{
			CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(dynamic);
			CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(dynamic);
			CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(dynamic);
			CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(dynamic);
			CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(dynamic);

			if (stalker || changer || monster || group || zone)
				dynamic->on_unregister_client();

			//Msg("---UNREGISTER Alife Object Client ID: %d, Name %s", id, dynamic->name_replace());
			alife_objects[id] = 0;
		}

	}
	else if (type == 3)
	{
		u16 id = packet->r_u16();
		shared_str name_spawn;
		packet->r_stringZ(name_spawn);

		/*
		if (alife_objects[id])
			Msg("UpdateObject [%d]->[%s]==[%s]", id, alife_objects[id]->s_name.c_str(), name_spawn.c_str());
		else
			Msg("UpdateObject not Find [%d] [%s]", id, name_spawn.c_str());
		*/

		if (alife_objects[id] && xr_strcmp(alife_objects[id]->s_name, name_spawn) == 0)
			alife_objects[id]->UPDATE_Read(*packet);

	}
}

void game_cl_freemp::RegisterObjectsAfterSpawn()
{
	for (auto alife_object : alife_objects)
	{
		CSE_ALifeDynamicObject* dynamic = alife_object.second;

		CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(dynamic);
		CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(dynamic);
		CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(dynamic);
		CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(dynamic);
		CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(dynamic);

		if (stalker || changer || monster || group || zone)
		{
			//Msg("Name %s, spawn_name %s", dynamic->name_replace(), dynamic->s_name.c_str());
			dynamic->on_register_client();
		}
	}

	alife_objects_registered = true;
}