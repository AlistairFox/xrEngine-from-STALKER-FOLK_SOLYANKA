////////////////////////////////////////////////////////////////////////////
//	Module 		: script_ini_file_script.cpp
//	Created 	: 25.06.2004
//  Modified 	: 25.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Script ini file class export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_ini_file.h"
#include "script_json_file.h"

#include "..\jsonxx\jsonxx.h"

using namespace luabind;
using namespace jsonxx;

CScriptIniFile *get_system_ini()
{
	return	((CScriptIniFile*)pSettings);
}

#ifdef XRGAME_EXPORTS
CScriptIniFile *get_game_ini()
{
	return	((CScriptIniFile*)pGameIni);
}
#endif // XRGAME_EXPORTS

bool r_line(CScriptIniFile *self, LPCSTR S, int L,	luabind::internal_string &N, luabind::internal_string &V)
{
	THROW3			(self->section_exist(S),"Cannot find section",S);
	THROW2			((int)self->line_count(S) > L,"Invalid line number");
	
	N				= "";
	V				= "";
	
	LPCSTR			n,v;
	bool			result = !!self->r_line(S,L,&n,&v);
	if (!result)
		return		(false);

	N				= n;
	if (v)
		V			= v;
	return			(true);
}

CScriptIniFile* create_ini(LPCSTR path, LPCSTR ini_file)
{
	return xr_new<CScriptIniFile>(false, ini_file, path, false);
}

CScriptIniFile* read_ini(LPCSTR path, LPCSTR ini_file)
{
	return xr_new<CScriptIniFile>(true, ini_file, path, true);
}

#pragma warning(push)
#pragma warning(disable:4238)
CScriptIniFile *create_ini_file	(LPCSTR ini_string)
{
	return			(
		(CScriptIniFile*)
		xr_new<CInifile>(
			&IReader			(
				(void*)ini_string,
				xr_strlen(ini_string)
			),
			FS.get_path("$game_config$")->m_Path
		)
	);
}
#pragma warning(pop)

#pragma optimize("s",on)
void CScriptIniFile::script_register(lua_State *L)
{
	module(L)
	[
		class_<CScriptIniFile>("ini_file")
			.def(					constructor<LPCSTR>())


			.def("section_exist",	&CScriptIniFile::section_exist	)
			.def("line_exist",		&CScriptIniFile::line_exist		)
			
			.def("r_clsid",			&CScriptIniFile::r_clsid		)
			.def("r_bool",			&CScriptIniFile::r_bool			)
			.def("r_token",			&CScriptIniFile::r_token		)
			.def("r_string_wq",		&CScriptIniFile::r_string_wb	)
			.def("line_count",		&CScriptIniFile::line_count)
			.def("r_string",		&CScriptIniFile::r_string)
			.def("r_u32",			&CScriptIniFile::r_u32)
			.def("r_s32",			&CScriptIniFile::r_s32)
			.def("r_float",			&CScriptIniFile::r_float)
			.def("r_vector",		&CScriptIniFile::r_fvector3)

			.def("w_string", &CScriptIniFile::w_string)
			.def("w_u32", &CScriptIniFile::w_u32)
			.def("w_s32", &CScriptIniFile::w_s32)
			.def("w_float", &CScriptIniFile::w_float)
			.def("w_vector", &CScriptIniFile::w_fvector3)
			.def("w_bool", &CScriptIniFile::w_bool)
			.def("fname", &CScriptIniFile::fname)
			.def("save", &CScriptIniFile::save)
			.def("r_line", &::r_line, out_value(_4) + out_value(_5)),

			
		def("system_ini",			&get_system_ini),
#ifdef XRGAME_EXPORTS
		def("game_ini",				&get_game_ini),
		def("create_ini", &create_ini),
		def("read_ini", &read_ini),
#endif // XRGAME_EXPORTS
		def("create_ini_file",		&create_ini_file,	adopt(result))
	];

	module(L)
	[

		class_<CJsonFile>("json")
			.def(constructor<>())

			.def("has_boolean", &CJsonFile::has_boolean)
			.def("has_number", &CJsonFile::has_number)
			.def("has_string", &CJsonFile::has_string)
			.def("has_object", &CJsonFile::has_object)
			.def("has_array", &CJsonFile::has_array)

			.def("get_boolean", &CJsonFile::get_boolean)
			.def("get_number", &CJsonFile::get_number)
			.def("get_string", &CJsonFile::get_string)
			.def("get_object", &CJsonFile::get_object)
			.def("get_array", &CJsonFile::get_array)

			.def("set_boolean", &CJsonFile::set_boolean)
			.def("set_number", &CJsonFile::set_number)
			.def("set_string", &CJsonFile::set_string)

			.def("set_object", &CJsonFile::set_object)
			.def("set_array", &CJsonFile::set_array)
			
		
			.def("set_object_array", &CJsonFile::set_object_array)
			.def("get_object_from_array", &CJsonFile::get_object_from_array)
			.def("array_size", &CJsonFile::array_size)

			.def("load_json", &CJsonFile::LoadJSON)
			.def("save_json", &CJsonFile::SaveJSON)
	];

	module(L)
	[
		class_<Object>("json_object")
			.def( constructor<>() ),

		class_<CObjectJsonEx>("json_o_ex")
			.def(constructor<>() )
			
			.def("has_bool", &CObjectJsonEx::has_bool)
			.def("has_string", &CObjectJsonEx::has_string)
			.def("has_number", &CObjectJsonEx::has_number)
		
			.def("get_bool", &CObjectJsonEx::get_bool)
			.def("get_string", &CObjectJsonEx::get_string)
			.def("get_number", &CObjectJsonEx::get_number)

			.def("set_bool", &CObjectJsonEx::set_bool)
			.def("set_string", &CObjectJsonEx::set_string)
			.def("set_number", &CObjectJsonEx::set_number)

			.def("has_object", &CObjectJsonEx::has_object)
			.def("get_object", &CObjectJsonEx::get_object)
			.def("set_object", &CObjectJsonEx::set_object)

			.def("has_array", &CObjectJsonEx::has_array)
			.def("get_array", &CObjectJsonEx::get_array)
			.def("set_array", &CObjectJsonEx::set_array)

			.def("load", &CObjectJsonEx::load)
			.def("save", &CObjectJsonEx::save)
	];

	module(L)
	[
		class_<Array>("json_array")
			.def(constructor<>())

		
	];
}