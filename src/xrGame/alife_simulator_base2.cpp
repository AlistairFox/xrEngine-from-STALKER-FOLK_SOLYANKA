////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_simulator_base2.cpp
//	Created 	: 25.12.2002
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator base class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_simulator_base.h"
#include "relation_registry.h"
#include "alife_registry_wrappers.h"
#include "xrServer_Objects_ALife_Items.h"
#include "alife_graph_registry.h"
#include "alife_object_registry.h"
#include "alife_story_registry.h"
#include "alife_schedule_registry.h"
#include "alife_smart_terrain_registry.h"
#include "alife_group_registry.h"

#include "game_sv_freemp.h"
#include "xrServer.h"

using namespace ALife;

void CALifeSimulatorBase::register_object	(CSE_ALifeDynamicObject *object, bool add_object)
{
	object->on_before_register			();

	if (add_object)
		objects().add					(object);
	
	graph().update						(object);
	scheduled().add						(object);
	story_objects().add					(object->m_story_id,object);
	smart_terrains().add				(object);
	groups().add						(object);

	setup_simulator						(object);
	
	if (psDeviceFlags.test(rsDebug))
		Msg("register_object : [%s], id [%d], time[%d]", object->name(), object->ID, Device.dwTimeGlobal);

	CSE_ALifeInventoryItem				*item = smart_cast<CSE_ALifeInventoryItem*>(object);
	if (item && item->attached()) 
	{
		CSE_ALifeDynamicObject			*II = objects().object(item->base()->ID_Parent);

#ifdef DEBUG
		if (std::find(II->children.begin(),II->children.end(),item->base()->ID) != II->children.end()) {
			Msg							("[LSS] Specified item [%s][%d] is already attached to the specified object [%s][%d]",item->base()->name_replace(),item->base()->ID,II->name_replace(),II->ID);
			FATAL						("[LSS] Cannot recover from the previous error!");
		}
#endif

		II->children.push_back			(item->base()->ID);
		II->attach						(item,true,false);
	}

	if (can_register_objects())
		object->on_register				();

	game_sv_freemp* freemp = smart_cast<game_sv_freemp*>(server().game);
	if (freemp)
		freemp->RegisterUpdateAlife(object, true);
		
}

void CALifeSimulatorBase::unregister_object	(CSE_ALifeDynamicObject *object, bool alife_query)
{
	if (!object) 
		return;
	
	object->on_unregister				();

	CSE_ALifeInventoryItem				*item = smart_cast<CSE_ALifeInventoryItem*>(object);
	if (item && item->attached())
		graph().detach					(*objects().object(item->base()->ID_Parent),item,objects().object(item->base()->ID_Parent)->m_tGraphID,alife_query);
 
	objects().remove					(object->ID);
	story_objects().remove				(object->m_story_id);
	smart_terrains().remove				(object);
	groups().remove						(object);

	if (!object->m_bOnline) {
		graph().remove					(object,object->m_tGraphID);
		scheduled().remove				(object);
 	}
	else
	if (object->ID_Parent == 0xffff)
	{
//			if (object->used_ai_locations())
 			graph().level().remove	(object,!object->used_ai_locations());
	}

	game_sv_freemp* freemp = smart_cast<game_sv_freemp*>(server().game);
	if (freemp)
		freemp->RegisterUpdateAlife(object, false);
}

void CALifeSimulatorBase::on_death			(CSE_Abstract *killed, CSE_Abstract *killer)
{
	typedef CSE_ALifeOnlineOfflineGroup::MEMBER	GROUP_MEMBER;

	CSE_ALifeCreatureAbstract			*creature = smart_cast<CSE_ALifeCreatureAbstract*>(killed);
	if (creature)
		creature->on_death				(killer);

	GROUP_MEMBER						*member = smart_cast<GROUP_MEMBER*>(killed);
	if (!member)
		return;

	if (member->m_group_id == 0xffff)
		return;

	groups().object(member->m_group_id).notify_on_member_death	(member);
}

void CALifeSimulatorBase::register_in_objects(CSE_ALifeDynamicObject* object)
{
	objects().add(object);
}

void CALifeSimulatorBase::unregister_in_objects(CSE_ALifeDynamicObject* object)
{
	objects().remove(object->ID);
}


