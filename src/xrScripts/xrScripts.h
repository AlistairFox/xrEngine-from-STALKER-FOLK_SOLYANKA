#pragma once

#ifdef XR_SCRIPTS
#	define SCRIPT_API 		__declspec(dllexport)
#else 
#	define SCRIPT_API		__declspec(dllimport)
#endif // #ifdef XR_SCRIPTS


#define LUABIND_API		SCRIPT_API
#define LUASTUDIO_API	SCRIPT_API
#define LUACORE			SCRIPT_API
#define LUA_API			SCRIPT_API




// se7kills LUYBIND ERRORS CHECKING !!!
// #	define LUABIND_NO_ERROR_CHECKING
#	define LUABIND_NO_EXCEPTION
#	define BOOST_NO_EXCEPTIONS

#	define LUABIND_DONT_COPY_STRINGS