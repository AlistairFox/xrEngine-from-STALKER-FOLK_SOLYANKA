////////////////////////////////////////////////////////////////////////////
//	Module 		: moving_objects.cpp
//	Created 	: 27.03.2007
//  Modified 	: 27.03.2007
//	Author		: Dmitriy Iassenev
//	Description : moving objects
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "moving_objects.h"
#include "ai_space.h"
#include "level_graph.h"
#include "moving_object.h"

moving_objects::moving_objects				() :
	m_tree					(0)
{
}

moving_objects::~moving_objects				()
{
	xr_delete				(m_tree);
}

void moving_objects::on_level_load			()
{
	xr_delete				(m_tree);
	m_tree					= xr_new<TREE>(ai().level_graph().header().box(),ai().level_graph().header().cell_size()*.5f,16*1024,16*1024);
}

void moving_objects::register_object		(moving_object *moving_object)
{
	if (moving_object != nullptr)
		m_tree->insert			(moving_object);
}

void moving_objects::unregister_object		(moving_object *moving_object)
{
	if (moving_object != nullptr)
		m_tree->remove			(moving_object);
}

void moving_objects::on_object_move			(moving_object *moving_object)
{
	if (moving_object != nullptr)
	{
		m_tree->remove(moving_object);
		moving_object->update_position();
		m_tree->insert(moving_object);
	}
}

void moving_objects::clear					()
{
	m_previous_collisions.clear_not_free	();
}