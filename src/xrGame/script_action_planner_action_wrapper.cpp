////////////////////////////////////////////////////////////////////////////
//	Module 		: script_action_planner_action_wrapper.cpp
//	Created 	: 29.03.2004
//  Modified 	: 29.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Script action planner action wrapper
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_action_planner_action_wrapper.h"
#include "script_game_object.h"

void CScriptActionPlannerActionWrapper::setup		(CScriptGameObject *object, CPropertyStorage *storage)
{
	try 
	{
		luabind::call_member<void>(this, "setup", object, storage);
	}
	catch (...)
	{
		Msg("CScriptActionPlannerActionWrapper Setup Crushed (%s), ID (%d)", object->Name(), object->ID() );
		ai().script_engine().print_stack();
	}
}

void CScriptActionPlannerActionWrapper::setup_static(CScriptActionPlannerAction *planner, CScriptGameObject *object, CPropertyStorage *storage)
{
	planner->CScriptActionPlannerAction::setup	(object,storage);
}

void CScriptActionPlannerActionWrapper::initialize			()
{
	try
	{
		luabind::call_member<void>(this, "initialize");
	}
	catch (...)
	{
		Msg("CScriptActionPlannerActionWrapper INIT Crushed (%s), ID (%d)", object().Name(), object().ID());
		ai().script_engine().print_stack();
	}

}

void CScriptActionPlannerActionWrapper::initialize_static	(CScriptActionPlannerAction *action)
{
	action->CScriptActionPlannerAction::initialize	();
}

void CScriptActionPlannerActionWrapper::execute				()
{
	try
	{
		luabind::call_member<void>			(this,"execute");
	}
	catch (...)
	{
		Msg("CScriptActionPlannerActionWrapper [ERROR] execute Action Name [%s], Action [%d]", object().Name(), m_current_action_id);
		ai().script_engine().print_stack();
	}
}

void CScriptActionPlannerActionWrapper::execute_static		(CScriptActionPlannerAction *action)
{
	action->CScriptActionPlannerAction::execute		();
}

void CScriptActionPlannerActionWrapper::finalize				()
{
	try
	{
		luabind::call_member<void>(this, "finalize");
	}
	catch (...)
	{
		Msg("CScriptActionPlannerActionWrapper Finalize Crushed [%s], ID [%d]", object().Name(), object().ID());
		ai().script_engine().print_stack();
	}
}

void CScriptActionPlannerActionWrapper::finalize_static		(CScriptActionPlannerAction *action)
{
	action->CScriptActionPlannerAction::finalize		();
}

CScriptActionPlannerActionWrapper::_edge_value_type CScriptActionPlannerActionWrapper::weight	(const CSConditionState &condition0, const CSConditionState &condition1) const
{
	return								(luabind::call_member<_edge_value_type>(const_cast<CScriptActionPlannerActionWrapper*>(this),"weight",condition0,condition1));
}

CScriptActionPlannerActionWrapper::_edge_value_type CScriptActionPlannerActionWrapper::weight_static	(CScriptActionPlannerAction *action, const CSConditionState &condition0, const CSConditionState &condition1)
{
	return								(((const CScriptActionPlannerActionWrapper*)action)->CScriptActionPlannerAction::weight(condition0,condition1));
}
