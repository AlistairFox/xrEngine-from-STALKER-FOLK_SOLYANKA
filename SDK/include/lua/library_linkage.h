////////////////////////////////////////////////////////////////////////////
//	Module 		: library_linkage.h
//	Created 	: 14.04.2007
//  Modified 	: 23.04.2008
//	Author		: Dmitriy Iassenev
//	Description : library linkage file
////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef LUAJIT_2
	#pragma comment(lib, "lua.JIT.1.1.4.lib" )
#else
	#pragma comment (lib, "lua51.lib")
#endif