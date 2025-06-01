//-----------------------------------------------------------------------------
// File: x_ray.cpp
//
// Programmers:
//	Oles		- Oles Shishkovtsov
//	AlexMX		- Alexander Maksimchuk
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "igame_level.h"
#include "igame_persistent.h"

#include "dedicated_server_only.h"
#include "no_single.h"
#include "../xrNetServer/NET_AuthCheck.h"

#include "xr_input.h"
#include "xr_ioconsole.h"
#include "x_ray.h"
#include "std_classes.h"
#include "GameFont.h"
#include "resource.h"
#include "LightAnimLibrary.h"
#include "../xrcdb/ispatial.h" 
#include "Text_Console.h"
#include <process.h>
#include <locale.h>
 
#include "securom_api.h"

//---------------------------------------------------------------------
ENGINE_API CInifile* pGameIni		= NULL;
BOOL	g_bIntroFinished			= FALSE;
 
// computing build id
extern	LPCSTR	build_date;
extern	u32		build_id;

static LPSTR month_id[12] = {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static int days_in_month[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int start_day	= 31;	// 31
static int start_month	= 1;	// January
static int start_year	= 1999;	// 1999
 
void compute_build_id	()
{
	build_date			= __DATE__;

	int					days;
	int					months = 0;
	int					years;
	string16			month;
	string256			buffer;
	xr_strcpy				(buffer,__DATE__);
	sscanf				(buffer,"%s %d %d",month,&days,&years);

	for (int i=0; i<12; i++) {
		if (_stricmp(month_id[i],month))
			continue;

		months			= i;
		break;
	}

	build_id			= (years - start_year)*365 + days - start_day;

	for (int i=0; i<months; ++i)
		build_id		+= days_in_month[i];

	for (int i=0; i<start_month-1; ++i)
		build_id		-= days_in_month[i];
}

//////////////////////////////////////////////////////////////////////////
// global variables
ENGINE_API	CApplication*	pApp			= NULL;
static		HWND			logoWindow		= NULL;
ENGINE_API	bool			g_bBenchmark	= false;
string512	g_sBenchmarkName;


ENGINE_API	string512		g_sLaunchOnExit_params;
ENGINE_API	string512		g_sLaunchOnExit_app;
ENGINE_API	string_path		g_sLaunchWorkingFolder;
// -------------------------------------------
// startup point
void InitEngine		()
{
	Engine.Initialize			( );
	while (!g_bIntroFinished)	
		Sleep	(100);
	Device.Initialize			( );
}

struct path_excluder_predicate
{
	explicit path_excluder_predicate(xr_auth_strings_t const * ignore) :
		m_ignore(ignore)
	{
	}
	bool xr_stdcall is_allow_include(LPCSTR path)
	{
		if (!m_ignore)
			return true;
		
		return allow_to_include_path(*m_ignore, path);
	}
	xr_auth_strings_t const *	m_ignore;
};

void InitSettings	()
{
	string_path					fname; 
	FS.update_path				(fname,"$game_config$","system.ltx");
#ifdef DEBUG
	Msg							("Updated path to system.ltx is %s", fname);
#endif // #ifdef DEBUG
	pSettings					= xr_new<CInifile>	(fname,TRUE);
	CHECK_OR_EXIT				(0!=pSettings->section_count(), make_string("Cannot find file %s.\nReinstalling application may fix this problem.",fname));

	xr_auth_strings_t			tmp_ignore_pathes;
	xr_auth_strings_t			tmp_check_pathes;
	fill_auth_check_params		(tmp_ignore_pathes, tmp_check_pathes);
	
	path_excluder_predicate			tmp_excluder(&tmp_ignore_pathes);
	CInifile::allow_include_func_t	tmp_functor;
	tmp_functor.bind(&tmp_excluder, &path_excluder_predicate::is_allow_include);
	pSettingsAuth					= xr_new<CInifile>(
		fname,
		TRUE,
		TRUE,
		FALSE,
		0,
		tmp_functor
	);

	FS.update_path				(fname,"$game_config$","game.ltx");
	pGameIni					= xr_new<CInifile>	(fname,TRUE);
	CHECK_OR_EXIT				(0!=pGameIni->section_count(), make_string("Cannot find file %s.\nReinstalling application may fix this problem.",fname));
}

void InitSettings_xray()
{
	string_path					fname;
	FS.update_path(fname, "$game_config$", "xray_settings.ltx");

	pSettingsSe7kills = xr_new<CInifile>(fname, TRUE);
	CHECK_OR_EXIT(0 != pSettingsSe7kills->section_count(), make_string("Cannot find file %s.\nReinstalling application may fix this problem.", fname));
}


void InitConsole	()
{
#ifdef DEDICATED_SERVER
		Console						= xr_new<CTextConsole>	();		
#else
		Console						= xr_new<CConsole>	();
#endif
	Console->Initialize			( );

	xr_strcpy						(Console->ConfigFile,"user.ltx");
	if (strstr(Core.Params,"-ltx ")) {
		string64				c_name;
		sscanf					(strstr(Core.Params,"-ltx ")+5,"%[^ ] ",c_name);
		xr_strcpy					(Console->ConfigFile,c_name);
	}
}

void InitInput		()
{
	BOOL bCaptureInput			= !strstr(Core.Params,"-i");

	pInput						= xr_new<CInput>		(bCaptureInput);
}
void destroyInput	()
{
	xr_delete					( pInput		);
}

void InitSound1		()
{
	CSound_manager_interface::_create				(0);
}

void InitSound2		()
{
	CSound_manager_interface::_create				(1);
}

void destroySound	()
{
	CSound_manager_interface::_destroy				( );
}

void destroySettings()
{
	CInifile** s				= (CInifile**)(&pSettings);
	xr_delete					( *s		);
	xr_delete					( pGameIni		);
}

void destroyConsole	()
{
	Console->Execute			("cfg_save");
	Console->Destroy			();
	xr_delete					(Console);
}

void destroyEngine	()
{
	Device.Destroy				( );
	Engine.Destroy				( );
}

void execUserScript				( )
{
	Console->Execute			("default_controls");
	Console->ExecuteScript		(Console->ConfigFile);
}
 
void Startup()
{
	InitSound1		();
	execUserScript	();
	InitSound2		();

	// ...command line for auto start
	{
		LPCSTR	pStartup			= strstr				(Core.Params,"-start ");
		if (pStartup)				Console->Execute		(pStartup+1);
	}
	{
		LPCSTR	pStartup			= strstr				(Core.Params,"-load ");
		if (pStartup)				Console->Execute		(pStartup+1);
	}

	// Initialize APP
 
	ShowWindow( Device.m_hWnd , SW_SHOWNORMAL );
	Device.Create				( );
 
	LALib.OnCreate				( );
	pApp						= xr_new<CApplication>	();
	g_pGamePersistent			= (IGame_Persistent*)	NEW_INSTANCE (CLSID_GAME_PERSISTANT);
	g_SpatialSpace				= xr_new<ISpatial_DB>	();
	g_SpatialSpacePhysic		= xr_new<ISpatial_DB>	();
	
	// Destroy LOGO
	DestroyWindow				(logoWindow);
	logoWindow					= NULL;

	// Main cycle
 	Memory.mem_usage();
	Device.Run					( );

	// Destroy APP
	xr_delete					( g_SpatialSpacePhysic	);
	xr_delete					( g_SpatialSpace		);
	DEL_INSTANCE				( g_pGamePersistent		);
	xr_delete					( pApp					);
	Engine.Event.Dump			( );

	// Destroying
 	destroyInput();
	destroySettings();
	LALib.OnDestroy				( );
 	destroyConsole();
 	destroySound();
	destroyEngine();
}

static INT_PTR CALLBACK logDlgProc( HWND hw, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg ){
		case WM_DESTROY:
			break;
		case WM_CLOSE:
			DestroyWindow( hw );
			break;
		case WM_COMMAND:
			if( LOWORD(wp)==IDCANCEL )
				DestroyWindow( hw );
			break;
		default:
			return FALSE;
	}
	return 1;
}

#include "xr_ioc_cmd.h"
 
ENGINE_API	bool g_dedicated_server	= false;
 
int APIENTRY WinMain_impl(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     char *    lpCmdLine,
                     int       nCmdShow)
{
#ifdef DEDICATED_SERVER
	Debug._initialize			(true);
	g_dedicated_server = true;
#else // DEDICATED_SERVER
	Debug._initialize			(false);
#endif // DEDICATED_SERVER
	  
	{
		logoWindow = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STARTUP), nullptr, logDlgProc);

		HWND logoPicture = GetDlgItem(logoWindow, IDC_STATIC_LOGO);
		RECT logoRect;
		GetWindowRect(logoPicture, &logoRect);
#ifdef DEDICATED_SERVER
		bool Show = !strstr(Core.Params, "-hide") && g_dedicated_server;
		UINT flags;
		if (Show)
			flags = SWP_NOMOVE | SWP_SHOWWINDOW;
		else
			flags = SWP_NOMOVE | SWP_HIDEWINDOW;
#else
		UINT flags;
		flags = SWP_NOMOVE | SWP_SHOWWINDOW;
#endif 

 		SetWindowPos(
			logoWindow,
			HWND_TOPMOST,
			0,
			0,
			logoRect.right - logoRect.left,
			logoRect.bottom - logoRect.top,
			flags // | SWP_NOSIZE
		);
		
		UpdateWindow(logoWindow);
	}
    

	// AVI
	g_bIntroFinished			= TRUE;
 	g_sLaunchOnExit_app[0]		= NULL;
	g_sLaunchOnExit_params[0]	= NULL;

	LPCSTR						fsgame_ltx_name = "-fsltx ";
	string_path					fsgame = "";

	if (strstr(lpCmdLine, fsgame_ltx_name))
	{
		int						sz = xr_strlen(fsgame_ltx_name);
		sscanf					(strstr(lpCmdLine,fsgame_ltx_name)+sz,"%[^ ] ",fsgame);
 	}
  	
	compute_build_id			();
	Core._initialize			("xray",NULL, TRUE, fsgame[0] ? fsgame : NULL);


	// Dump FileList

	FS_FileSet files;
	FS.file_list(files, "$game_data$", FS_ListFiles);
	for (auto file : files)
	{
		Msg("File : %s", file.name.c_str());
	}


	InitSettings				();
	InitSettings_xray();

	// Adjust player & computer name for Asian
	if ( pSettings->line_exist( "string_table" , "no_native_input" ) ) {
			xr_strcpy( Core.UserName , sizeof( Core.UserName ) , "Player" );
			xr_strcpy( Core.CompName , sizeof( Core.CompName ) , "Computer" );
	}

 	InitEngine				();
	InitInput				();
	InitConsole				();
	Engine.External.CreateRendererList();

	Msg("command line %s", lpCmdLine);		 
#ifndef DEDICATED_SERVER
	if(strstr(Core.Params,"-r2a"))	
		Console->Execute			("renderer renderer_r2a");
	else
	if(strstr(Core.Params,"-r2"))	
		Console->Execute			("renderer renderer_r2");
	else
	{
		CCC_LoadCFG_custom*	pTmp = xr_new<CCC_LoadCFG_custom>("renderer ");
		pTmp->Execute				(Console->ConfigFile);
		xr_delete					(pTmp);
	}
#else
		Console->Execute			("renderer renderer_r1");
#endif
//.		InitInput					( );
	Engine.External.Initialize	( );
	Console->Execute			("stat_memory");

	Startup	 					( );
	Core._destroy				( );

	// check for need to execute something external
	if (/*xr_strlen(g_sLaunchOnExit_params) && */xr_strlen(g_sLaunchOnExit_app) ) 
	{
		//CreateProcess need to return results to next two structures
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		//We use CreateProcess to setup working folder
		char const * temp_wf = (xr_strlen(g_sLaunchWorkingFolder) > 0) ? g_sLaunchWorkingFolder : NULL;
		CreateProcess(g_sLaunchOnExit_app, g_sLaunchOnExit_params, NULL, NULL, FALSE, 0, NULL, 
			temp_wf, &si, &pi);

	}   
 	// here damn_keys_filter class instanse will be destroyed
	return						0;
}

int stack_overflow_exception_filter	(int exception_code)
{
   if (exception_code == EXCEPTION_STACK_OVERFLOW)
   {
       // Do not call _resetstkoflw here, because
       // at this point, the stack is not yet unwound.
       // Instead, signal that the handler (the __except block)
       // is to be executed.
       return EXCEPTION_EXECUTE_HANDLER;
   }
   else
       return EXCEPTION_CONTINUE_SEARCH;
}

#include <boost/crc.hpp>
 

LPCSTR _GetFontTexName (LPCSTR section)
{
	static char* tex_names[]={"texture800","texture","texture1600"};
	int def_idx		= 1;//default 1024x768
	int idx			= def_idx;

	u32 h = Device.dwHeight;
 	if(h<=600)	
		idx = 0;
	else if(h<1024)	
		idx = 1;
	else
		idx = 2;

	while(idx>=0){
		if( pSettings->line_exist(section,tex_names[idx]) )
			return pSettings->r_string(section,tex_names[idx]);
		--idx;
	}
	return pSettings->r_string(section,tex_names[def_idx]);
}

void _InitializeFont(CGameFont*& F, LPCSTR section, u32 flags)
{
	LPCSTR font_tex_name = _GetFontTexName(section);
	R_ASSERT(font_tex_name);

	LPCSTR sh_name = pSettings->r_string(section,"shader");
	if(!F){
		F = xr_new<CGameFont> (sh_name, font_tex_name, flags);
	}else
		F->Initialize(sh_name, font_tex_name);

	if (pSettings->line_exist(section,"size")){
		float sz = pSettings->r_float(section,"size");
		if (flags&CGameFont::fsDeviceIndependent)
			F->SetHeightI(sz);
		else									
			F->SetHeight(sz);
	}
	if (pSettings->line_exist(section,"interval"))
		F->SetInterval(pSettings->r_fvector2(section,"interval"));

}
