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

#pragma once

#define LUABIND_DONT_COPY_STRINGS

#ifndef LUA_DEBUG

#	ifndef LUABIND_NO_ERROR_CHECKING
// #		define LUABIND_NO_ERROR_CHECKING
#	endif // LUABIND_NO_ERROR_CHECKING

#	pragma warning(disable: 4251 577 297)
#   define LUABIND_DTOR_NOEXCEPT noexcept
#else
#   define LUABIND_DTOR_NOEXCEPT
#endif // LUA_DEBUG
// LUABIND_NO_ERROR_CHECKING
// define this to remove all error checks
// this will improve performance and memory
// footprint.
// if it is defined matchers will only be called on
// overloaded functions, functions that's
// not overloaded will be called directly. The
// parameters on the lua stack are assumed
// to match those of the function.
// exceptions will still be catched when there's
// no error checking.

// LUABIND_NOT_THREADSAFE
// this define will make luabind non-thread safe. That is,
// it will rely on a static variable. You can still have
// multiple lua states and use coroutines, but only
// one of your real threads may run lua code.

// If you don't want to use the rtti supplied by C++
// you can supply your own type-info structure with the
// LUABIND_TYPE_INFO define. Your type-info structure
// must be copyable and it must be able to compare itself
// against other type-info structures. You supply the compare
// function through the LUABIND_TYPE_INFO_EQUAL()
// define. It should compare the two type-info structures
// it is given and return true if they represent the same type
// and false otherwise. You also have to supply a function
// to generate your type-info structure. You do this through
// the LUABIND_TYPEID() define. It takes a type as it's
// parameter. That is, a compile time parameter. To use it
// you probably have to make a traits class with specializations
// for all classes that you have type-info for.

#ifndef LUABIND_TYPE_INFO
#	define LUABIND_TYPE_INFO const type_info*
#	define LUABIND_TYPEID(t) &typeid(t)
#	define LUABIND_TYPE_INFO_EQUAL(i1, i2) *i1 == *i2
#	define LUABIND_INVALID_TYPE_INFO &typeid(detail::null_type)
#	include <typeinfo>
#endif

// LUABIND_NO_EXCEPTIONS
// this define will disable all usage of try, catch and throw in
// luabind. This will in many cases disable runtime-errors, such
// as invalid casts, when calling lua-functions that fails or
// returns values that cannot be converted by the given policy.
// Luabind requires that no function called directly or indirectly
// by luabind throws an exception (throwing exceptions through
// C code has undefined behavior, lua is written in C).
// #define LUABIND_NO_EXCEPTIONS

// If you're building luabind as a dll on windows with devstudio
// you can set LUABIND_EXPORT to __declspec(dllexport)
// and LUABIND_IMPORT to __declspec(dllimport)

// this define is set if we're currently building a luabind file
// select import or export depending on it
#include "../../../src/xrScripts/xrScripts.h"
#include <luabind/luabind_memory.h>

using	string_class =	luabind::internal_string;
#define vector_class	luabind::internal_vector
#define list_class		luabind::internal_list
#define map_class				luabind::internal_map
#define set_class				luabind::internal_set
#define multimap_class			luabind::internal_multimap
#define multiset_class			luabind::internal_multiset