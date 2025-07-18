// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
#include "stdafx.h"
#include <luabind/lua_include.hpp>
#include <luabind/luabind.hpp>

namespace luabind { namespace detail
{
	namespace
	{
		// expects two tables on the lua stack:
		// 1: destination
		// 2: source
		void copy_member_table(lua_State* L)
		{
			lua_pushnil(L);

			while (lua_next(L, -2))
			{
				lua_pushstring(L, "__init");
				if (lua_equal(L, -1, -3))
				{
					lua_pop(L, 2);
					continue;
				}
				else lua_pop(L, 1); // __init string

				lua_pushstring(L, "__finalize");
				if (lua_equal(L, -1, -3))
				{
					lua_pop(L, 2);
					continue;
				}
				else lua_pop(L, 1); // __finalize string

				lua_pushvalue(L, -2); // copy key
				lua_insert(L, -2);
				lua_settable(L, -5);
			}
		}
	}


	int create_class::stage2(lua_State* L)
	{
		class_rep* cl_rep = static_cast<class_rep*>(lua_touserdata(L, lua_upvalueindex(1)));
		assert((cl_rep != 0) && "internal error, please report");
		assert((is_class_rep(L, lua_upvalueindex(1))) && "internal error, please report");

	#ifndef LUABIND_NO_ERROR_CHECKING

		if (!is_class_rep(L, 1))
		{
			// Added class name to error info
			string_class err_line("expected class '");
			err_line += cl_rep->name();
			err_line += "' to derive from or a newline";
			lua_pushstring(L, err_line.c_str());
			lua_error(L);
		}

	#endif

		class_rep* base = static_cast<class_rep*>(lua_touserdata(L, 1));
		class_rep::base_info binfo;

		binfo.pointer_offset = 0;
		binfo.base = base;
		cl_rep->add_base_class(binfo);

		// set holder size and alignment so that we can class_rep::allocate
		// can return the correctly sized buffers
		cl_rep->derived_from(base);
		
		// copy base class members

		cl_rep->get_table(L);
		base->get_table(L);
		copy_member_table(L);

		cl_rep->get_default_table(L);
		base->get_default_table(L);
		copy_member_table(L);

		cl_rep->set_type(base->type());

		return 0;
	}

	int create_class::stage1(lua_State* L)
	{

	#ifndef LUABIND_NO_ERROR_CHECKING

		if (lua_gettop(L) != 1 || lua_type(L, 1) != LUA_TSTRING || lua_isnumber(L, 1))
		{
			lua_pushstring(L, "invalid construct, expected class name");
			lua_error(L);
		}

		if (std::strlen(lua_tostring(L, 1)) != lua_strlen(L, 1))
		{
			lua_pushstring(L, "luabind does not support class names with extra nulls");
			lua_error(L);
		}

	#endif

		const char* name = lua_tostring(L, 1);

//		int stack_level = lua_gettop(L);
		//////////////////////////////////////////////////////////////////////////
		// Here we are trying to add the class to the namespace in the local variable "this" if exist
		//////////////////////////////////////////////////////////////////////////

		int index = LUA_GLOBALSINDEX;
		lua_Debug ar;
		if ( lua_getstack (L, 1, &ar) )
		{
			unsigned int i = 1;
			const char *cname;
			while ((cname = lua_getlocal(L, &ar, i++)) != nullptr) {
				if (!strcmp("this", cname)) {
					if (lua_istable(L,-1))
						index = lua_gettop(L);
					else
						lua_pop(L, 1);
					break;
				}
				lua_pop(L, 1);  /* remove variable value */
			}
		}
		
		//////////////////////////////////////////////////////////////////////////
		// End of change
		//////////////////////////////////////////////////////////////////////////

		void* c = lua_newuserdata(L, sizeof(class_rep));
		new(c) class_rep(L, name);

		// make the class globally available
		lua_pushstring(L, name);
		lua_pushvalue(L, -2);
		lua_settable(L, index);
		if (index != LUA_GLOBALSINDEX)
			lua_remove(L,index);

		// also add it to the closure as return value
		lua_pushcclosure(L, &stage2, 1);

//		int stack_level2 = lua_gettop(L);
		return 1;
	}

}}

