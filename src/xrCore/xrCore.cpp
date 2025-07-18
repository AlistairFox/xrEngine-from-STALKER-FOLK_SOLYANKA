// xrCore.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#pragma hdrstop

#include <mmsystem.h>
#include <objbase.h>
#include "xrCore.h"
 
#pragma comment(lib,"winmm.lib")

#ifdef DEBUG
#	include	<malloc.h>
#endif // DEBUG

XRCORE_API		xrCore	Core;
XRCORE_API		u32		build_id;
XRCORE_API		LPCSTR	build_date;

namespace CPU
{
	extern	void			Detect	();
};

static u32	init_counter	= 0;

char g_application_path[256];
 
void xrCore::_initialize	(LPCSTR _ApplicationName, LogCallback cb, BOOL init_fs, LPCSTR fs_fname)
{
	xr_strcpy					(ApplicationName,_ApplicationName);
	if (0==init_counter) 
	{
		// Init COM so we can use CoCreateInstance
 		CoInitializeEx	(NULL, COINIT_MULTITHREADED);

		xr_strcpy			(Params,sizeof(Params),GetCommandLine());
		_strlwr_s			(Params,sizeof(Params));

		string_path		fn,dr,di;

		// application path
        GetModuleFileName(GetModuleHandle(MODULE_NAME),fn,sizeof(fn));
        _splitpath		(fn,dr,di,0,0);
        strconcat		(sizeof(ApplicationPath),ApplicationPath,dr,di);
 		xr_strcpy		(g_application_path,sizeof(g_application_path),ApplicationPath);
  
		GetCurrentDirectory(sizeof(WorkingPath),WorkingPath);

		// User/Comp Name
		DWORD	sz_user		= sizeof(UserName);
		GetUserName			(UserName,&sz_user);

		DWORD	sz_comp		= sizeof(CompName);
		GetComputerName		(CompName,&sz_comp);

		// Mathematics & PSI detection
		CPU::Detect			();
		
		Memory._initialize	(strstr(Params,"-mem_debug") ? TRUE : FALSE);

		InitLog				();
		_initialize_cpu		();
		rtc_initialize		();

		xr_FS				= xr_new<CLocatorAPI>	();
		xr_EFS				= xr_new<EFS_Utils>		();

	}
	if (init_fs){
		u32 flags			= 0;
		if (0!=strstr(Params,"-build"))	 flags |= CLocatorAPI::flBuildCopy;
		if (0!=strstr(Params,"-ebuild")) flags |= CLocatorAPI::flBuildCopy|CLocatorAPI::flEBuildCopy;

		flags |= CLocatorAPI::flScanAppRoot;

		if (0!=strstr(Params,"-file_activity"))	
			flags |= CLocatorAPI::flDumpFileActivity;
		FS._initialize		(flags,0,fs_fname);
		Msg					("'%s' build %d, %s\n","xrCore",build_id, build_date);
		EFS._initialize		();
	}
	SetLogCB				(cb);
	init_counter++;
}

#ifndef	_EDITOR
#include "compression_ppmd_stream.h"
extern compression::ppmd::stream	*trained_model;
#endif
void xrCore::_destroy		()
{
	--init_counter;
	if (0==init_counter){
		FS._destroy			();
		EFS._destroy		();
		xr_delete			(xr_FS);
		xr_delete			(xr_EFS);

#ifndef	_EDITOR
		if (trained_model) {
			void			*buffer = trained_model->buffer();
			xr_free			(buffer);
			xr_delete		(trained_model);
		}
#endif

		Memory._destroy		();
	}
}

BOOL DllMainCore(HANDLE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			_clear87		();
			_control87		( _PC_53,   MCW_PC );
			_control87		( _RC_CHOP, MCW_RC );
			_control87		( _RC_NEAR, MCW_RC );
			_control87		( _MCW_EM,  MCW_EM );
		}
 		break;
	case DLL_THREAD_ATTACH:
		if (!strstr(GetCommandLine(),"-editor"))
			CoInitializeEx	(NULL, COINIT_MULTITHREADED);
		timeBeginPeriod	(1);
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
 