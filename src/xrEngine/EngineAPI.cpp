// EngineAPI.cpp: implementation of the CEngineAPI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EngineAPI.h"
#include "../xrcdb/xrXRC.h"

#pragma comment(lib, "OptickCore.lib")

#pragma comment(lib, "xrScripts.lib")

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "nvapi.lib")

#pragma	comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib") 
#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "libtheora_static.lib")
#pragma comment(lib, "oalib.lib")
#pragma comment(lib, "xrCPU_Pipe.lib")
#pragma comment(lib, "openal32.lib")

// Base LIBS
// Compression Librarys
#pragma comment(lib, "zlibstatic-ng.lib")

#pragma comment(lib, "ode.lib")
#pragma comment(lib, "crypto.lib")

#pragma comment(lib, "xrCore.lib")
#pragma comment(lib, "xrCDB.lib")


#pragma comment(lib, "xrNetServer.lib")
#pragma comment(lib, "xrParticles.lib")
#pragma comment(lib, "xrSound.lib")
#pragma comment(lib, "xrXMLParser.lib")
#pragma comment(lib, "xrPhysics.lib")
#pragma comment(lib, "GameNetworkingSockets.lib")

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "nvapi.lib")

// SOUNDS 
#pragma comment(lib, "opus.lib")

#pragma comment(lib, "libspeexdsp.lib")

#ifdef DEDICATED_SERVER

#pragma comment(lib, "xrServerRender.lib")

#else
#pragma comment(lib, "xrRender_R4.lib")
#pragma comment(lib, "GFSDK_SSAO_D3D11.win64.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d10.lib")

#endif // DEDICATED_SERVER


extern xr_token* vid_quality_token;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void __cdecl dummy(void) {
};
CEngineAPI::CEngineAPI()
{
	hGame = 0;
	hRender = 0;
	hTuner = 0;
	pCreate = 0;
	pDestroy = 0;
	tune_pause = dummy;
	tune_resume = dummy;
}

CEngineAPI::~CEngineAPI()
{
}

extern u32 renderer_value; //con cmd
extern int g_current_renderer = 0;

ENGINE_API bool is_enough_address_space_available()
{
	SYSTEM_INFO		system_info;
	GetSystemInfo(&system_info);
	return			(*(u32*)&system_info.lpMaximumApplicationAddress) > 0x90000000;
}

#ifndef DEDICATED_SERVER
extern BOOL DllMainXrRenderR4(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
#define DLL_MAIN_RENDER DllMainXrRenderR4
#else
extern BOOL DllMainXrServerRender(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
#define DLL_MAIN_RENDER DllMainXrServerRender
#endif // !DEDICATED_SERVER


extern BOOL DllMainXrServerRender(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
extern BOOL DllMainXrRenderR4(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

LPCSTR r1_name = "xrServerRender";
LPCSTR r4_name = "xrRender_R4";

#ifndef DEDICATED_SERVER
void CEngineAPI::InitializeNotDedicated()
{
	{
		// try to initialize R4
		psDeviceFlags.set(rsR2, FALSE);
		psDeviceFlags.set(rsR4, TRUE);
		Log("Loading DLL:", r4_name);
		DllMainXrRenderR4(NULL, DLL_PROCESS_ATTACH, NULL);
	}
}
#endif

#pragma comment(lib, "xrGame.lib")

extern BOOL DllMainXrGame(HANDLE hModule, u32 ul_reason_for_call, LPVOID lpReserved);

extern "C"
DLL_Pure* __cdecl xrFactory_Create(CLASS_ID clsid);
extern "C"
void __cdecl xrFactory_Destroy(DLL_Pure* O);

ENGINE_API extern bool CoreXrayClearSky = false;

#include "IGame_Persistent.h"
#include <openal/alc.h>

void CEngineAPI::Initialize(void)
{
	//////////////////////////////////////////////////////////////////////////
	// render

#ifdef DEDICATED_SERVER
	psDeviceFlags.set(rsR4, FALSE);
	psDeviceFlags.set(rsR2, FALSE);

	Log("Loading DLL:", r1_name);
	DllMainXrServerRender(NULL, DLL_PROCESS_ATTACH, NULL);
#else
	InitializeNotDedicated();
#endif // DEDICATED_SERVER


	Device.ConnectToRender();
	CoreXrayClearSky = strstr(Core.Params, "-gametype=clearsky") != 0;

	Msg("~~~ LOAD CLEAR SKY GAMETYPE: %s", CoreXrayClearSky ? "true" : "false");
	// game	
	{
		LPCSTR g_name = "xrGame.dll";
		Log("Loading DLL:", g_name);
		DllMainXrGame(NULL, DLL_PROCESS_ATTACH, NULL);
		pCreate = xrFactory_Create;
		R_ASSERT(pCreate);
		pDestroy = xrFactory_Destroy;
		R_ASSERT(pDestroy);
	}
}

void CEngineAPI::Destroy(void)
{
	DllMainXrGame(NULL, DLL_PROCESS_DETACH, NULL);
	DLL_MAIN_RENDER(NULL, DLL_PROCESS_DETACH, NULL);
	pCreate = 0;
	pDestroy = 0;
	Engine.Event._destroy();
	XRC.r_clear_compact();
}

extern "C" {
	typedef bool __cdecl SupportsAdvancedRenderingREF(void);
	typedef bool SupportsDX10RenderingREF();
	typedef bool SupportsDX11RenderingREF();
};

extern "C" {

	bool SupportsDX11Rendering();

};


void CEngineAPI::CreateRendererList()
{
#ifdef DEDICATED_SERVER
	vid_quality_token = xr_alloc<xr_token>(2);

	vid_quality_token[0].id = 0;
	vid_quality_token[0].name = xr_strdup("renderer_r1");

	vid_quality_token[1].id = -1;
	vid_quality_token[1].name = NULL;
#else

	// TODO: ask renderers if they are supported!
	if (vid_quality_token != NULL) return;
	bool bSupports_r2 = false;
	bool bSupports_r2_5 = false;
	bool bSupports_r3 = false;
	bool bSupports_r4 = false;

	LPCSTR r2_name = "xrRender_R2.dll";
	LPCSTR r3_name = "xrRender_R3.dll";
	LPCSTR r4_name = "xrRender_R4.dll";

	if (strstr(Core.Params, "-perfhud_hack"))
	{
		bSupports_r2 = true;
		bSupports_r2_5 = true;
		bSupports_r3 = true;
		bSupports_r4 = true;
	}
	else
	{



		// try to initialize R4
		Log("Loading DLL:", r4_name);
		// Hide "d3d10.dll not found" message box for XP
		SetErrorMode(SEM_FAILCRITICALERRORS);
		//hRender = LoadLibrary(r4_name);
		DllMainXrRenderR4(NULL, DLL_PROCESS_ATTACH, NULL);
		// Restore error handling
		SetErrorMode(0);
		//if (hRender)
		{
			//SupportsDX11RenderingREF* test_dx11_rendering = (SupportsDX11RenderingREF*)GetProcAddress(hRender, "SupportsDX11Rendering");
			SupportsDX11RenderingREF* test_dx11_rendering = SupportsDX11Rendering;
			R_ASSERT(test_dx11_rendering);
			bSupports_r4 = test_dx11_rendering();
			//FreeLibrary(hRender);
		}
	}

	//hRender = 0;
	bool proceed = true;
	xr_vector<LPCSTR> _tmp;
	if (proceed &= bSupports_r4, proceed)
		_tmp.push_back("renderer_r4");

	R_ASSERT2(_tmp.size() != 0, "No valid renderer found, please use a render system that's supported by your PC");

	u32 _cnt = _tmp.size() + 1;
	vid_quality_token = xr_alloc<xr_token>(_cnt);

	vid_quality_token[_cnt - 1].id = -1;
	vid_quality_token[_cnt - 1].name = NULL;

	Msg("Available render modes[%d]:", _tmp.size());
	for (u32 i = 0; i < _tmp.size(); ++i)
	{
		vid_quality_token[i].id = i;
		vid_quality_token[i].name = _tmp[i];
		Msg("[%s]", _tmp[i]);
	}
#endif //#ifndef DEDICATED_SERVER
}

// Dedicated Server Sizes
extern int WidthDedicatedX = 1024;
extern int WidthDedicatedY = 900;


extern BOOL DllMainCore(HANDLE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved);
extern BOOL DllMainXrPhysics(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
// extern BOOL DllMainOpenAL32(HANDLE module, DWORD reason, LPVOID reserved);

void al_log(char* msg)
{
	Msg(msg);
}

int APIENTRY WinMain_impl(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char* lpCmdLine,
	int       nCmdShow);

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char* lpCmdLine,
	int       nCmdShow)
{
	pLog = &al_log;
	DllMainCore(NULL, DLL_PROCESS_ATTACH, NULL);
	// DllMainOpenAL32(NULL, DLL_PROCESS_ATTACH, NULL);
	DllMainXrPhysics(NULL, DLL_PROCESS_ATTACH, NULL);

	WinMain_impl(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	DllMainCore(NULL, DLL_PROCESS_DETACH, NULL);
	/// DllMainOpenAL32(NULL, DLL_PROCESS_DETACH, NULL);
	DllMainXrPhysics(NULL, DLL_PROCESS_DETACH, NULL);
	return					(0);
}
