// TextureManager.cpp: implementation of the CResourceManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#pragma warning(disable:4995)
#include <dxsdk/d3dx9.h>
#pragma warning(default:4995)

#include "ResourceManager.h"
#include "tss.h"
#include "blenders\blender.h"
#include "blenders\blender_recorder.h"

//	Already defined in Texture.cpp
void fix_texture_name(LPSTR fn);
/*
void fix_texture_name(LPSTR fn)
{
	LPSTR _ext = strext(fn);
	if(  _ext					&&
	  (0==stricmp(_ext,".tga")	||
		0==stricmp(_ext,".dds")	||
		0==stricmp(_ext,".bmp")	||
		0==stricmp(_ext,".ogm")	) )
		*_ext = 0;
}
*/
//--------------------------------------------------------------------------------------------------------------
template <class T>
BOOL	reclaim		(xr_vector<T*>& vec, const T* ptr)
{
	xr_vector<T*>::iterator it	= vec.begin	();
	xr_vector<T*>::iterator end	= vec.end	();
	for (; it!=end; it++)
		if (*it == ptr)	{ vec.erase	(it); return TRUE; }
		return FALSE;
}

//--------------------------------------------------------------------------------------------------------------
IBlender* CResourceManager::_GetBlender		(LPCSTR Name)
{
	R_ASSERT(Name && Name[0]);

	LPSTR N = LPSTR(Name);
	map_Blender::iterator I = m_blenders.find	(N);
#ifdef _EDITOR
	if (I==m_blenders.end())	return 0;
#else
//	TODO: DX10: When all shaders are ready switch to common path
#if defined(USE_DX10) || defined(USE_DX11)
	if (I==m_blenders.end())	
	{
		Msg("DX10: Shader '%s' not found in library.",Name); 
		return 0;
	}
#endif
	if (I==m_blenders.end())	{ Msg ("Shader '%s' not found in library.",Name); return 0; }
#endif
	else					return I->second;
}

IBlender* CResourceManager::_FindBlender		(LPCSTR Name)
{
	if (!(Name && Name[0])) return 0;

	LPSTR N = LPSTR(Name);
	map_Blender::iterator I = m_blenders.find	(N);
	if (I==m_blenders.end())	return 0;
	else						return I->second;
}

void	CResourceManager::ED_UpdateBlender	(LPCSTR Name, IBlender* data)
{
	LPSTR N = LPSTR(Name);
	map_Blender::iterator I = m_blenders.find	(N);
	if (I!=m_blenders.end())	{
		R_ASSERT	(data->getDescription().CLS == I->second->getDescription().CLS);
		xr_delete	(I->second);
		I->second	= data;
	} else {
		m_blenders.insert	(mk_pair(xr_strdup(Name),data));
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void	CResourceManager::_ParseList(sh_list& dest, LPCSTR names)
{
	if (0==names || 0==names[0])
 		names 	= "$null";

	ZeroMemory			(&dest, sizeof(dest));
	char*	P			= (char*) names;
	svector<char,128>	N;

	while (*P)
	{
		if (*P == ',') {
			// flush
			N.push_back	(0);
			strlwr		(N.begin());

			fix_texture_name( N.begin() );
//. andy			if (strext(N.begin())) *strext(N.begin())=0;
			dest.push_back(N.begin());
			N.clear		();
		} else {
			N.push_back	(*P);
		}
		P++;
	}
	if (N.size())
	{
		// flush
		N.push_back	(0);
		strlwr		(N.begin());

		fix_texture_name( N.begin() );
//. andy		if (strext(N.begin())) *strext(N.begin())=0;
		dest.push_back(N.begin());
	}
}

ShaderElement* CResourceManager::_CreateElement			(ShaderElement& S)
{
	if (S.passes.empty())		return	0;

	// Search equal in shaders array
	for (u32 it=0; it<v_elements.size(); it++)
		if (S.equal(*(v_elements[it])))	return v_elements[it];

	// Create _new_ entry
	ShaderElement*	N		=	xr_new<ShaderElement>(S);
	N->dwFlags				|=	xr_resource_flagged::RF_REGISTERED;
	v_elements.push_back	(N);
	return N;
}

void CResourceManager::_DeleteElement(const ShaderElement* S)
{
	if (0==(S->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(v_elements,S))						return;
	Msg	("! ERROR: Failed to find compiled 'shader-element'");
}

Shader*	CResourceManager::_cpp_Create	(IBlender* B, LPCSTR s_shader, LPCSTR s_textures, LPCSTR s_constants, LPCSTR s_matrices)
{
	CBlender_Compile	C;
	Shader				S;

	//.
	// if (strstr(s_shader,"transparent"))	__asm int 3;

	// Access to template
	C.BT				= B;
	C.bEditor			= FALSE;
	C.bDetail			= FALSE;

#if defined(USE_DX11)
	C.HudElement = false;
#endif

#ifdef _EDITOR
	if (!C.BT)			{ ELog.Msg(mtError,"Can't find shader '%s'",s_shader); return 0; }
	C.bEditor			= TRUE;
#endif

	// Parse names
	_ParseList			(C.L_textures,	s_textures	);
	_ParseList			(C.L_constants,	s_constants	);
	_ParseList			(C.L_matrices,	s_matrices	);

#if defined(USE_DX11)
	if (::Render->hud_loading && RImplementation.o.ssfx_hud_raindrops)
	{
		Msg(":::::::::::::::: HUD ELEMENT [%s] [%s]", s_shader, s_textures);
		C.HudElement = true;
	}
#endif

	// Compile element	(LOD0 - HQ)
	{
		C.iElement			= 0;
		C.bDetail			= m_textures_description.GetDetailTexture(C.L_textures[0],C.detail_texture,C.detail_scaler);
//.		C.bDetail			= _GetDetailTexture(*C.L_textures[0],C.detail_texture,C.detail_scaler);
		ShaderElement		E;
		C._cpp_Compile		(&E);
		S.E[0]				= _CreateElement	(E);
	}

	// Compile element	(LOD1)
	{
		C.iElement			= 1;
//.		C.bDetail			= _GetDetailTexture(*C.L_textures[0],C.detail_texture,C.detail_scaler);
		C.bDetail			= m_textures_description.GetDetailTexture(C.L_textures[0],C.detail_texture,C.detail_scaler);
		ShaderElement		E;
		C._cpp_Compile		(&E);
		S.E[1]				= _CreateElement	(E);
	}

	// Compile element
	{
		C.iElement			= 2;
		C.bDetail			= FALSE;
		ShaderElement		E;
		C._cpp_Compile		(&E);
		S.E[2]				= _CreateElement	(E);
	}

	// Compile element
	{
		C.iElement			= 3;
		C.bDetail			= FALSE;
		ShaderElement		E;
		C._cpp_Compile		(&E);
		S.E[3]				= _CreateElement	(E);
	}

	// Compile element
	{
		C.iElement			= 4;
		C.bDetail			= TRUE;	//.$$$ HACK :)
		ShaderElement		E;
		C._cpp_Compile		(&E);
		S.E[4]				= _CreateElement	(E);
	}

	// Compile element
	{
		C.iElement			= 5;
		C.bDetail			= FALSE;
		ShaderElement		E;
		C._cpp_Compile		(&E);
		S.E[5]				= _CreateElement	(E);
	}

	// Search equal in shaders array
	for (Shader* it : v_shaders)
	{
		if (S.equal(it))
			return it;
	}

	// Create _new_ entry
	Shader*		N			=	xr_new<Shader>(S);
	N->dwFlags				|=	xr_resource_flagged::RF_REGISTERED;
	v_shaders.push_back		(N);
	return N;
}

Shader*	CResourceManager::_cpp_Create	(LPCSTR s_shader, LPCSTR s_textures, LPCSTR s_constants, LPCSTR s_matrices)
{
//#ifndef DEDICATED_SERVER
#ifndef _EDITOR
	if (!g_dedicated_server)
#endif    
	{
		//	TODO: DX10: When all shaders are ready switch to common path
#if defined(USE_DX10) || defined(USE_DX11)
		IBlender	*pBlender = _GetBlender(s_shader?s_shader:"null");
		if (!pBlender) return NULL;
		return	_cpp_Create(pBlender ,s_shader,s_textures,s_constants,s_matrices);
#else	//	USE_DX10
		return	_cpp_Create(_GetBlender(s_shader?s_shader:"null"),s_shader,s_textures,s_constants,s_matrices);
#endif	//	USE_DX10
//#else
	}
#ifndef _EDITOR
	else
#endif    
	{
		return NULL;
	}
//#endif
}

Shader*		CResourceManager::Create	(IBlender*	B,		LPCSTR s_shader,	LPCSTR s_textures,	LPCSTR s_constants, LPCSTR s_matrices)
{
//#ifndef DEDICATED_SERVER
#ifndef _EDITOR
	if (!g_dedicated_server)
#endif
	{
		return	_cpp_Create	(B,s_shader,s_textures,s_constants,s_matrices);
//#else
	}
#ifndef _EDITOR
	else
#endif
	{
		return NULL;
//#endif
	}
}

Shader*		CResourceManager::Create	(LPCSTR s_shader,	LPCSTR s_textures,	LPCSTR s_constants,	LPCSTR s_matrices)
{
//#ifndef DEDICATED_SERVER
#ifndef _EDITOR
	if (!g_dedicated_server)
#endif
	{
		//	TODO: DX10: When all shaders are ready switch to common path
#if defined(USE_DX10) || defined(USE_DX11)
		if	(_lua_HasShader(s_shader))		
			return	_lua_Create	(s_shader,s_textures);
		else								
		{
			Shader* pShader = _cpp_Create	(s_shader,s_textures,s_constants,s_matrices);
			if (pShader)
				return pShader;
			else
			{
				if (_lua_HasShader("stub_default"))
					return	_lua_Create	("stub_default",s_textures);
				else
				{
					FATAL("Can't find stub_default.s");
					return 0;
				}
			}
		}
#else	//	USE_DX10
#ifndef _EDITOR
		if	(_lua_HasShader(s_shader))		
			return	_lua_Create	(s_shader,s_textures);
		else								
#endif
			return	_cpp_Create	(s_shader,s_textures,s_constants,s_matrices);
#endif	//	USE_DX10
	}
//#else
#ifndef _EDITOR
	else
#endif
	{
		return NULL;
	}
//#endif
}

void CResourceManager::Delete(const Shader* S)
{
	if (0==(S->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(v_shaders,S))						return;
	Msg	("! ERROR: Failed to find complete shader");
}

//xr_vector<CTexture*> tex_to_load;
/*
void TextureLoading(u16 thread_num) {
	Msg("TextureLoading -> thread %d started!", thread_num);

	u16 upperbound = thread_num * 100;
	u32 lowerbound = upperbound - 100;

	for (size_t i = lowerbound; i < upperbound; i++) {
		if (i < tex_to_load.size())
			tex_to_load[i]->Load();
		else
			break;
	}

	Msg("TextureLoading -> thread %d finished!", thread_num);
}
*/

#include <thread>

xr_vector<CTexture*> map_thread;

#define MAX_THREADS 16
xrCriticalSection csTEXTURES;

void MT_LOAD(int th)
{
	CTimer timer;
	u64 total = 0;
	for (;;)
	{
		csTEXTURES.Enter();
		if (map_thread.empty())
		{
			csTEXTURES.Leave();
			break;
		}
		CTexture* t = map_thread.back();
		map_thread.pop_back();
		csTEXTURES.Leave();

		timer.Start();
		t->Load();
		total += timer.GetElapsed_ticks();
	}

	Msg("Texture Thread[%d]: %d ms", th, total);
}

void CResourceManager::DeferredUpload()
{
	if (!RDEVICE.b_is_Ready) return;


	CTimer t;
	t.Start();

	map_thread.clear();

	for (auto tex : m_textures)
		map_thread.push_back(tex.second);

	std::thread* th = new std::thread[MAX_THREADS];

	for (int i = 0; i < MAX_THREADS; i++)
		th[i] = std::thread(MT_LOAD, i);

	for (int i = 0; i < MAX_THREADS; i++)
		th[i].join();


	Msg("Textures Loading: %u", t.GetElapsed_ms());
}

/*
void CResourceManager::DeferredUpload()
{
	/*	if (!RDEVICE.b_is_Ready) return;
		for (map_TextureIt t=m_textures.begin(); t!=m_textures.end(); t++)
		{
			t->second->Load();
		} 
	tex_to_load.clear();

	Msg("CResourceManager::DeferredUpload -> START, size = %d", m_textures.size());

	CTimer timer;
	timer.Start();

	if (m_textures.size() <= 100)
	{
		Msg("CResourceManager::DeferredUpload -> one thread");
		for (map_TextureIt t = m_textures.begin(); t != m_textures.end(); t++)
			t->second->Load();
	}
	else {
		u32 th_count = (m_textures.size() / 100) + 1;

		std::thread* th_arr = new std::thread[th_count];
		for (map_TextureIt t = m_textures.begin(); t != m_textures.end(); t++)
			tex_to_load.push_back((*t).second);

		for (u16 i = 0; i < th_count; i++)
			th_arr[i] = std::thread(TextureLoading, i + 1);

		for (size_t i = 0; i < th_count; i++)
			th_arr[i].join();

		delete[] th_arr;

		tex_to_load.clear();
	}

	Msg("texture loading time: %d", timer.GetElapsed_ms());
}
*/

void CResourceManager::DeferredUnload()
{
	if (!RDEVICE.b_is_Ready)
		return;
}

#ifdef _EDITOR
void	CResourceManager::ED_UpdateTextures(AStringVec* names)
{
	// 1. Unload
	if (names){
		for (u32 nid=0; nid<names->size(); nid++)
		{
			map_TextureIt I = m_textures.find	((*names)[nid].c_str());
			if (I!=m_textures.end())	I->second->Unload();
		}
	}else{
		for (map_TextureIt t=m_textures.begin(); t!=m_textures.end(); t++)
			t->second->Unload();
	}

	// 2. Load
	// DeferredUpload	();
}
#endif

void	CResourceManager::Evict()
{
	//	TODO: DX10: check if we really need this method
#if !defined(USE_DX10) && !defined(USE_DX11)
	CHK_DX	(HW.pDevice->EvictManagedResources());
#endif	//	USE_DX10
}
/*
BOOL	CResourceManager::_GetDetailTexture(LPCSTR Name,LPCSTR& T, R_constant_setup* &CS)
{
	LPSTR N = LPSTR(Name);
	map_TD::iterator I = m_td.find	(N);
	if (I!=m_td.end())
	{
		T	= I->second.T;
		CS	= I->second.cs;
		return TRUE;
	} else {
		return FALSE;
	}
}*/
