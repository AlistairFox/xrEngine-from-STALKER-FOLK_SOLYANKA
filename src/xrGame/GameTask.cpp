#include "pch_script.h"
#include "GameTask.h"
#include "ui/xrUIXmlParser.h"
#include "encyclopedia_article.h"
#include "map_location.h"
#include "map_spot.h"
#include "map_manager.h"

#include "level.h"
#include "actor.h"
#include "script_engine.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "ai_space.h"
#include "alife_object_registry.h"
#include "alife_simulator.h"
#include "alife_story_registry.h"
#include "game_object_space.h"
#include "object_broker.h"
#include "ui/uitexturemaster.h"


CGameTask::CGameTask()
{
	m_ReceiveTime			= 0;
	m_FinishTime			= 0;
	m_timer_finish			= 0;
	m_Title					= NULL;
	m_Description			= NULL;
	m_ID					= NULL;
	m_task_type				= eTaskTypeDummy;
	m_task_state			= eTaskStateDummy;
	m_linked_map_location	= NULL;
	m_read					= false;
}

void CGameTask::SetTaskState(ETaskState state)
{
	m_task_state = state;
	if( (m_task_state == eTaskStateFail) || (m_task_state == eTaskStateCompleted) )
	{
		RemoveMapLocations	(false);
		m_FinishTime = Level().GetGameTime();

		if ( m_task_state == eTaskStateFail )
		{
			SendInfo		(m_infos_on_fail);
			CallAllFuncs	(m_lua_functions_on_fail);
		}
		else
		{
			SendInfo		(m_infos_on_complete);
			CallAllFuncs	(m_lua_functions_on_complete);
		}
	}
	ChangeStateCallback();
}

void CGameTask::OnArrived()
{
	m_task_state   = eTaskStateInProgress;
	m_read         = false;

	CreateMapLocation( false );
}

#include "game_cl_freemp.h"

void CGameTask::CreateMapLocation( bool on_load )
{
	if ( m_map_object_id == u16(-1) || m_map_location.size() == 0)
	{
		return;
	}

	game_cl_freemp* fmp = smart_cast<game_cl_freemp*>(&Game());

	if (!Level().Objects.net_Find(m_map_object_id) && !fmp)
		return; 

	if (!fmp->GetAlifeObject(m_map_object_id))
		return;

	if ( on_load )
	{
		xr_vector<CMapLocation*> res;
		Level().MapManager().GetMapLocations(m_map_location, m_map_object_id, res);
		xr_vector<CMapLocation*>::iterator it = res.begin();
		xr_vector<CMapLocation*>::iterator it_e = res.end();
		for(; it!=it_e; ++it)
		{
			CMapLocation* ml = *it;
			if(ml->m_owner_task_id == m_ID)
			{
				m_linked_map_location = ml;
				break;
			}
		}
//.		m_linked_map_location =	Level().MapManager().GetMapLocation(m_map_location, m_map_object_id);
	}
	else
	{
		m_linked_map_location =	Level().MapManager().AddMapLocation(m_map_location, m_map_object_id);
		m_linked_map_location->m_owner_task_id = m_ID;
	}

	VERIFY( m_linked_map_location );

	if ( !on_load )
	{
		if ( m_map_hint.size() )
		{
			m_linked_map_location->SetHint( m_map_hint );
		}
		m_linked_map_location->DisablePointer();
		m_linked_map_location->SetSerializable( true );
	}

	if ( m_linked_map_location->complex_spot() )
	{
		m_linked_map_location->complex_spot()->SetTimerFinish( m_timer_finish );
	}
}

void CGameTask::RemoveMapLocations(bool notify)
{
	if ( m_linked_map_location && !notify)
		Level().MapManager().RemoveMapLocation( m_linked_map_location );

	m_map_location			= 0;
	m_linked_map_location	= NULL;
	m_map_object_id			= u16(-1);
}

void CGameTask::ChangeMapLocation( LPCSTR new_map_location, u16 new_map_object_id )
{
	RemoveMapLocations	( false );

	m_map_location		= new_map_location;
	m_map_object_id		= new_map_object_id;

	m_task_state		= eTaskStateInProgress;
	CreateMapLocation	( false );
}

void CGameTask::ChangeStateCallback()
{
	if (Actor())
		Actor()->callback(GameObject::eTaskStateChange)(this, GetTaskState() );
}

void CGameTask::LoadStateCallback()
{
	if (Actor())
		Actor()->callback(GameObject::eTaskLoadState) (this, m_ID.c_str());
}

ETaskState CGameTask::UpdateState()
{
	if( (m_ReceiveTime != m_TimeToComplete) )
	{
		if(Level().GetGameTime() > m_TimeToComplete)
		{
			return		eTaskStateFail;
		}
	}
//check fail infos
	if( CheckInfo(m_failInfos) )
		return		eTaskStateFail;

//check fail functor
	if( CheckFunctions(m_fail_lua_functions) )
		return		eTaskStateFail;
	
//check complete infos
	if( CheckInfo(m_completeInfos) )
		return		eTaskStateCompleted;


//check complete functor
	if( CheckFunctions(m_complete_lua_functions) )
		return		eTaskStateCompleted;

	
	return GetTaskState();
}

bool CGameTask::CheckInfo(const xr_vector<shared_str>& v) const
{
	bool res = false;
	xr_vector<shared_str>::const_iterator it	= v.begin();
	for(;it!=v.end();++it)
	{
		res = Actor()->HasInfo					(*it);
		if(!res) break;
	}
	return res;
}

bool CGameTask::CheckFunctions(const task_state_functors& v) const
{
	bool res = false;
	task_state_functors::const_iterator it	= v.begin();
	for(;it!=v.end();++it)
	{
		if( (*it).is_valid() ) res = (*it)(m_ID.c_str());
		if(!res) break;
	}
	return res;

}
void CGameTask::CallAllFuncs(const task_state_functors& v)
{
	task_state_functors::const_iterator it	= v.begin();
	for(;it!=v.end();++it){
		if( (*it).is_valid() ) (*it)(m_ID.c_str());
	}
}
void CGameTask::SendInfo(const xr_vector<shared_str>& v)
{
	xr_vector<shared_str>::const_iterator it	= v.begin();
	for(;it!=v.end();++it)
		Actor()->TransferInfo					((*it),true);

}

void CGameTask::save_task(IWriter &stream)
{
	save_data				(m_task_state,		stream);
	save_data				(m_task_type,		stream);
	save_data				(m_ReceiveTime,		stream);
	save_data				(m_FinishTime,		stream);
	save_data				(m_TimeToComplete,	stream);
	save_data				(m_timer_finish,	stream);

	save_data				(m_Title,			stream);
	save_data				(m_Description,		stream);
	save_data				(m_pScriptHelper,	stream);
	save_data				(m_icon_texture_name,stream);
	save_data				(m_map_hint,		stream);
	save_data				(m_map_location,	stream);
	save_data				(m_map_object_id,	stream);
	save_data				(m_priority,		stream);
}

void CGameTask::load_task(IReader &stream)
{
	load_data				(m_task_state,		stream);
load_data(m_task_type, stream);
load_data(m_ReceiveTime, stream);
load_data(m_FinishTime, stream);
load_data(m_TimeToComplete, stream);
load_data(m_timer_finish, stream);

load_data(m_Title, stream);
load_data(m_Description, stream);
load_data(m_pScriptHelper, stream);
load_data(m_icon_texture_name, stream);
load_data(m_map_hint, stream);
load_data(m_map_location, stream);
load_data(m_map_object_id, stream);
load_data(m_priority, stream);
CommitScriptHelperContents();
CreateMapLocation(true);
}

void CGameTask::save_task_ltx(CInifile& file, shared_str section)
{
	file.w_u8(section.c_str(), "m_task_state", m_task_state);
	file.w_u8(section.c_str(), "m_task_type", m_task_type);
	file.w_u64(section.c_str(), "m_ReceiveTime", m_ReceiveTime);
	file.w_u64(section.c_str(), "m_FinishTime", m_FinishTime);
	file.w_u64(section.c_str(), "m_TimeToComplete", m_TimeToComplete);
	file.w_u64(section.c_str(), "m_timer_finish", m_timer_finish);

	string256 temp;
	xr_strcpy(temp, m_Title.c_str());
	file.w_string(section.c_str(), "m_Title", temp);
	xr_strcpy(temp, m_Description.c_str());
	file.w_string(section.c_str(), "m_Description", temp);
	xr_strcpy(temp, m_icon_texture_name.c_str());
	file.w_string(section.c_str(), "m_icon_texture_name", temp);
	xr_strcpy(temp, m_map_hint.c_str());
	file.w_string(section.c_str(), "m_map_hint", temp);
	xr_strcpy(temp, m_map_location.c_str());
	file.w_string(section.c_str(), "m_map_location", temp);

	file.w_u16(section.c_str(), "m_map_object_id", m_map_object_id);
	file.w_u32(section.c_str(), "m_priority", m_priority);

	string512 res;

	m_pScriptHelper.convert_to_string(m_pScriptHelper.m_s_complete_lua_functions, res);
	file.w_string(section.c_str(), "complete_lua_functions", res);

	m_pScriptHelper.convert_to_string(m_pScriptHelper.m_s_fail_lua_functions, res);
	file.w_string(section.c_str(), "fail_lua_functions", res);

	m_pScriptHelper.convert_to_string(m_pScriptHelper.m_s_lua_functions_on_complete, res);
	file.w_string(section.c_str(), "lua_functions_on_complete", res);

	m_pScriptHelper.convert_to_string(m_pScriptHelper.m_s_lua_functions_on_fail, res);
	file.w_string(section.c_str(), "lua_functions_on_fail", res);

}

void CGameTask::load_task_ltx(CInifile& file, shared_str section)
{
	u8 task_state = file.r_u8(section.c_str(), "m_task_state");
	m_task_state = ETaskState(task_state);

	u8 task_type = file.r_u8(section.c_str(), "m_task_type");
	m_task_type = ETaskType(task_type);

	m_ReceiveTime = file.r_u64(section.c_str(), "m_ReceiveTime");
	m_FinishTime = file.r_u64(section.c_str(), "m_FinishTime");
	m_TimeToComplete = file.r_u64(section.c_str(), "m_TimeToComplete");
	m_timer_finish = file.r_u64(section.c_str(), "m_timer_finish");

	m_Title._set(file.r_string(section.c_str(), "m_Title"));
	m_Description._set(file.r_string(section.c_str(), "m_Description"));
	m_icon_texture_name._set(file.r_string(section.c_str(), "m_icon_texture_name"));


	m_map_hint._set(file.r_string(section.c_str(), "m_map_hint"));
	m_map_location._set(file.r_string(section.c_str(), "m_map_location"));
	m_map_object_id = file.r_u16(section.c_str(), "m_map_object_id");


	m_priority = file.r_u32(section.c_str(), "m_priority");

	m_pScriptHelper.m_s_complete_lua_functions = m_pScriptHelper.convert_to_vector(file.r_string(section.c_str(), "complete_lua_functions"));
	m_pScriptHelper.m_s_fail_lua_functions = m_pScriptHelper.convert_to_vector(file.r_string(section.c_str(), "fail_lua_functions"));
	m_pScriptHelper.m_s_lua_functions_on_complete = m_pScriptHelper.convert_to_vector(file.r_string(section.c_str(), "lua_functions_on_complete"));
	m_pScriptHelper.m_s_lua_functions_on_fail = m_pScriptHelper.convert_to_vector(file.r_string(section.c_str(), "lua_functions_on_fail"));

	CommitScriptHelperContents();
	CreateMapLocation(true);

}

#include "..\jsonxx\jsonxx.h"
using namespace jsonxx;

template <typename T, typename Type>
void save_json_data(CObjectJsonEx& json, Type data, LPCSTR key)
{
	json.import(key, (T) data);
}

void save_json_data(CObjectJsonEx& json, shared_str data, LPCSTR key)
{
	json.set_shared_str(key, data);
}

template <typename T>
void load_json_data(CObjectJsonEx& json, T data, LPCSTR key)
{
	if (json.has<T>(key))
		data = json.get<T>(key);
}

void load_json_data(CObjectJsonEx& json, shared_str data, LPCSTR key)
{
	if (json.has<String>(key))
		json.get_shared_str(key, data);
}

void CGameTask::save_json(CObjectJsonEx& json)
{
	save_json_data<Number>(json, m_task_state, "task_state");
	save_json_data<Number>(json, m_task_type, "task_type");
	save_json_data<Number>(json, m_ReceiveTime,"recive_time");
	save_json_data<Number>(json, m_FinishTime, "finish_time");
	save_json_data<Number>(json, m_TimeToComplete, "time_to_complete");
	save_json_data<Number>(json, m_timer_finish, "time_to_finish");

	save_json_data(json, m_Title, "title");
	save_json_data(json, m_Description, "descr");
	save_json_data(json, m_icon_texture_name, "icon_texture_name");
	save_json_data(json, m_map_hint, "map_hint");
	save_json_data(json, m_map_location, "map_location");
	save_json_data<Number>(json, m_map_object_id, "map_object_id");
	save_json_data<Number>(json, m_priority, "priority");	   

	m_pScriptHelper.save_json(json);
}

void CGameTask::load_json(CObjectJsonEx& json)
{
	int state = json.get_number("task_state");
	m_task_state = ETaskState(state);

	int type = json.get_number("task_type");
	m_task_type = ETaskType(type);

	//load_json_data<Number>(json, m_task_state, "task_state");
	//load_json_data<Number>(json, m_task_type, "task_type");

	load_json_data<Number>(json, m_ReceiveTime, "recive_time");
	load_json_data<Number>(json, m_FinishTime, "finish_time");
	load_json_data<Number>(json, m_TimeToComplete, "time_to_complete");
	load_json_data<Number>(json, m_timer_finish, "time_to_finish");

	load_json_data(json, m_Title, "title");
	load_json_data(json, m_Description, "descr");
	load_json_data(json, m_icon_texture_name, "icon_texture_name");
	load_json_data(json, m_map_hint, "map_hint");
	load_json_data(json, m_map_location, "map_location");
	
	load_json_data<Number>(json, m_map_object_id, "map_object_id");
	load_json_data<Number>(json, m_priority, "priority");

	m_pScriptHelper.load_json(json);
}

void CGameTask::CommitScriptHelperContents()
{
	m_pScriptHelper.init_functors	(m_pScriptHelper.m_s_complete_lua_functions,	m_complete_lua_functions);
	m_pScriptHelper.init_functors	(m_pScriptHelper.m_s_fail_lua_functions,		m_fail_lua_functions);
	m_pScriptHelper.init_functors	(m_pScriptHelper.m_s_lua_functions_on_complete,	m_lua_functions_on_complete);
	m_pScriptHelper.init_functors	(m_pScriptHelper.m_s_lua_functions_on_fail,		m_lua_functions_on_fail);
}


void CGameTask::AddCompleteInfo_script(LPCSTR _str)
{
	m_completeInfos.push_back(_str);
}

void CGameTask::AddFailInfo_script(LPCSTR _str)
{
	m_failInfos.push_back(_str);
}

void CGameTask::AddOnCompleteInfo_script(LPCSTR _str)
{
	m_infos_on_complete.push_back(_str);
}

void CGameTask::AddOnFailInfo_script(LPCSTR _str)
{
	m_infos_on_fail.push_back(_str);
}

void CGameTask::AddCompleteFunc_script(LPCSTR _str)
{
	m_pScriptHelper.m_s_complete_lua_functions.push_back(_str);
}
void CGameTask::AddFailFunc_script(LPCSTR _str)
{
	m_pScriptHelper.m_s_fail_lua_functions.push_back(_str);
}
void CGameTask::AddOnCompleteFunc_script(LPCSTR _str)
{
	m_pScriptHelper.m_s_lua_functions_on_complete.push_back(_str);
}
void CGameTask::AddOnFailFunc_script(LPCSTR _str)
{
	m_pScriptHelper.m_s_lua_functions_on_fail.push_back(_str);
}

void SScriptTaskHelper::init_functors(xr_vector<shared_str>& v_src, task_state_functors& v_dest)
{
	xr_vector<shared_str>::iterator it		= v_src.begin();
	xr_vector<shared_str>::iterator it_e	= v_src.end();
	v_dest.resize(v_src.size());

	for(u32 idx=0 ;it!=it_e;++it,++idx)
	{
			bool functor_exists		= ai().script_engine().functor(*(*it) ,v_dest[idx]);
			if(!functor_exists)		Log("Cannot find script function described in task objective  ", *(*it));
	}
}

void SScriptTaskHelper::load(IReader &stream)
{
		load_data(m_s_complete_lua_functions,		stream);
		load_data(m_s_fail_lua_functions,			stream);
		load_data(m_s_lua_functions_on_complete,	stream);
		load_data(m_s_lua_functions_on_fail,		stream);
}

void SScriptTaskHelper::save_json(CObjectJsonEx& json)
{
	string512 res[4];
	convert_to_string(m_s_complete_lua_functions, res[0]);
	convert_to_string(m_s_fail_lua_functions, res[1]);
	convert_to_string(m_s_lua_functions_on_complete, res[2]);
	convert_to_string(m_s_lua_functions_on_fail, res[3]);

	json.import("functions_lua_1", (String)res[0]);
	json.import("functions_lua_2", (String)res[1]);
	json.import("functions_lua_3", (String)res[2]);
	json.import("functions_lua_4", (String)res[3]);
}

void SScriptTaskHelper::load_json(CObjectJsonEx& json)
{
	LPCSTR source[4];

	source[0] = json.get_string("functions_lua_1");
	source[1] = json.get_string("functions_lua_2");
	source[2] = json.get_string("functions_lua_3");
	source[3] = json.get_string("functions_lua_4");

	m_s_complete_lua_functions = convert_to_vector(source[0]);
	m_s_fail_lua_functions = convert_to_vector(source[1]);
	m_s_lua_functions_on_complete = convert_to_vector(source[2]);
	m_s_lua_functions_on_fail = convert_to_vector(source[3]);
}

void SScriptTaskHelper::convert_to_string(xr_vector<shared_str> functor, string512& res)
{
	int prop_count = 0;
	res[0] = 0;
	for (auto item : functor)
	{
		LPCSTR upgr_section = item.c_str();
		if (prop_count > 0)
		{
			xr_strcat(res, sizeof(res), ", ");
		}
		xr_strcat(res, sizeof(res), upgr_section);
		++prop_count;
	}
}

xr_vector<shared_str> SScriptTaskHelper::convert_to_vector(shared_str save_str)
{
	xr_vector<shared_str> vec;

	int count = _GetItemCount(save_str.c_str(), ',');

	for (int i=0; i != count; i++)
	{
		string256 name_functor;
		_GetItem(save_str.c_str(), i, name_functor, ',');
		vec.push_back(name_functor);
	}

	return vec;
}

 
void SScriptTaskHelper::save(IWriter &stream)
{
	save_data(m_s_complete_lua_functions,		stream);
	save_data(m_s_fail_lua_functions,			stream);
	save_data(m_s_lua_functions_on_complete,	stream);
	save_data(m_s_lua_functions_on_fail,		stream);
}

void SGameTaskKey::save(IWriter &stream)
{
	save_data				(task_id, stream);
	game_task->save_task	(stream);
}

void SGameTaskKey::load(IReader &stream)
{
	game_task					= xr_new<CGameTask>();
	load_data					(task_id, stream);
	game_task->m_ID				= task_id;
	game_task->load_task		(stream);

}
 
void SGameTaskKey::save_ltx(CInifile& file, shared_str section)
{
	file.w_string(section.c_str(), "task_id", task_id.c_str());
 	game_task->save_task_ltx(file, section);
}

void SGameTaskKey::load_ltx(CInifile& file, shared_str section)
{
	game_task = xr_new<CGameTask>();
	task_id = file.r_string(section, "task_id");
	game_task->m_ID = task_id;
	game_task->load_task_ltx(file, section);
}

void SGameTaskKey::destroy()
{
	delete_data(game_task);
}
