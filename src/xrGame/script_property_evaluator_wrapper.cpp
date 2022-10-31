////////////////////////////////////////////////////////////////////////////
//	Module 		: script_property_evaluator_wrapper.cpp
//	Created 	: 19.03.2004
//  Modified 	: 26.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Script property evaluator wrapper
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_property_evaluator_wrapper.h"
#include "script_game_object.h"
#include "ai_space.h"
#include "script_engine.h"

void CScriptPropertyEvaluatorWrapper::setup			(CScriptGameObject *object, CPropertyStorage *storage)
{  
	try 
	{
		luabind::call_member<void>(this, "setup", object, storage);
	}
	catch (...)
	{
		Msg("SCRIPT RUNTIME WARNING : SETUP ( object [%s] evaluator [%s] alive [%s]) crashed!",
			m_object->Name(),
			m_evaluator_name,
			m_object->Alive() ? "true" : "false"
		);

		ai().script_engine().print_stack();
	}
}

void CScriptPropertyEvaluatorWrapper::setup_static	(CScriptPropertyEvaluator *evaluator, CScriptGameObject *object, CPropertyStorage *storage)
{
	evaluator->CScriptPropertyEvaluator::setup(object,storage);
}

bool CScriptPropertyEvaluatorWrapper::evaluate		()
{
	if (m_object) 
	if (m_object->Alive())
	{
		try 
		{
 			return	(luabind::call_member<bool>(this,"evaluate"));
		}
	#ifdef DEBUG
		catch(luabind::cast_failed &exception)
		{
	#ifdef LOG_ACTION
			ai().script_engine().script_log (ScriptStorage::eLuaMessageTypeError,"SCRIPT RUNTIME ERROR : evaluator [%s] returns value with not a %s type!",m_evaluator_name,exception.info()->name());
	#else
			ai().script_engine().script_log (ScriptStorage::eLuaMessageTypeError,"SCRIPT RUNTIME ERROR : evaluator returns value with not a %s type!",exception.info()->name());
	#endif
		}
	#endif
		catch(...) 
		{
			/*
			ai().script_engine().script_log (ScriptStorage::eLuaMessageTypeError,
				"SCRIPT RUNTIME ERROR : object [%s] evaluator [%s] alive [%s] crashed!", 
				this->m_object->Name(), 
				m_evaluator_name,
				this->m_object->Alive() ? "true" : "false"
			);
			*/

			Msg("SCRIPT RUNTIME WARNING : EVALUATE ( object [%s] evaluator [%s] alive [%s] ) crashed!",
				this->m_object->Name(),
				m_evaluator_name,
				this->m_object->Alive() ? "true" : "false"
			);
			ai().script_engine().print_stack();

			//this->m_object = 0;
			//this->m_storage = 0;
			
 		}
	}

	return		(false);
}

bool CScriptPropertyEvaluatorWrapper::evaluate_static	(CScriptPropertyEvaluator *evaluator)
{
	return		(evaluator->CScriptPropertyEvaluator::evaluate());
}
