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

#ifdef STATICRENDER_R1
#pragma comment(lib, "xrRender_R1.lib")
#pragma comment(lib, "d3dx9.lib")
#endif

#ifdef STATICRENDER_R2
#pragma comment(lib, "xrRender_R2.lib")
#endif

#ifdef STATICRENDER_R4

#pragma comment(lib, "xrRender_R4.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "imgui.lib")

//#pragma comment(lib, "GFSDK_SSAO_D3D11.win64.lib")

#endif

extern BOOL DllMainRenderR1(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
extern BOOL DllMainRenderR2(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
extern BOOL DllMainRenderR3(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
extern BOOL DllMainRenderR4(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

void CEngineAPI::InitializeNotDedicated()
{
	LPCSTR			r1_name = "xrRender_R1.dll";
	LPCSTR			r2_name = "xrRender_R2.dll";
	LPCSTR			r3_name = "xrRender_R3.dll";
	LPCSTR			r4_name = "xrRender_R4.dll";

#ifdef STATICRENDER_R4
	// try to initialize R2
	psDeviceFlags.set(rsR2, FALSE);
	psDeviceFlags.set(rsR3, FALSE);
	Log("Loading DLL:", r4_name);
	DllMainRenderR4(NULL, DLL_PROCESS_ATTACH, NULL);
	g_current_renderer = 4;
	renderer_value = 5;
#endif

#ifdef STATICRENDER_R2
	// try to initialize R2
	psDeviceFlags.set(rsR2, FALSE);
	psDeviceFlags.set(rsR4, FALSE);
	Log("Loading DLL:", r2_name);
	DllMainRenderR2(NULL, DLL_PROCESS_ATTACH, NULL);
	g_current_renderer = 2;
	renderer_value = 3;
#endif

#ifdef STATICRENDER_R1
	// try to initialize R1
	psDeviceFlags.set(rsR2, FALSE);
	psDeviceFlags.set(rsR3, FALSE);
	psDeviceFlags.set(rsR4, FALSE);
	Log("Loading DLL:", r1_name);
	DllMainRenderR1(NULL, DLL_PROCESS_ATTACH, NULL);
	g_current_renderer = 1;
	renderer_value = 0;
#endif 
}

void CEngineAPI::InitializeDedicated()
{
	LPCSTR			r1_name = "xrRender_R1.dll";

	if (0 == hRender)
	{
		// try to load R1
		psDeviceFlags.set(rsR4, FALSE);
		psDeviceFlags.set(rsR3, FALSE);
		psDeviceFlags.set(rsR2, FALSE);
		renderer_value = 0; //con cmd

		Log("Loading DLL:", r1_name);
		hRender = LoadLibrary(r1_name);
		if (0 == hRender)	R_CHK(GetLastError());
		R_ASSERT(hRender);
		g_current_renderer = 1;
	}
}

#pragma comment(lib, "xrGame.lib")

extern BOOL DllMainXrGame(HANDLE hModule, u32 ul_reason_for_call, LPVOID lpReserved);

extern "C"
DLL_Pure* __cdecl xrFactory_Create(CLASS_ID clsid);
extern "C"
void __cdecl xrFactory_Destroy(DLL_Pure* O);

ENGINE_API extern bool CoreXrayClearSky = false;

#include "IGame_Persistent.h"

void CEngineAPI::Initialize(void)
{
	//////////////////////////////////////////////////////////////////////////
	// render
	LPCSTR			r1_name = "xrRender_R1.dll";

	if (!g_dedicated_server)
		InitializeNotDedicated();
	else
		InitializeDedicated();


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
	if (hGame) { FreeLibrary(hGame);	hGame = 0; }
	if (hRender) { FreeLibrary(hRender); hRender = 0; }
	pCreate = 0;
	pDestroy = 0;
	Engine.Event._destroy();
	XRC.r_clear_compact();
}

typedef bool __cdecl SupportsAdvancedRenderingREF(void);
typedef bool /*_declspec(dllexport)*/ SupportsDX10RenderingREF();
typedef bool /*_declspec(dllexport)*/ SupportsDX11RenderingREF();

extern "C"
{
	bool __cdecl SupportsAdvancedRendering(void);
	bool /*_declspec(dllexport)*/ SupportsDX10Rendering();
	bool /*_declspec(dllexport)*/ SupportsDX11Rendering();
};

void CEngineAPI::CreateRendererList()
{
	if (g_dedicated_server)
	{
		vid_quality_token = xr_alloc<xr_token>(2);

		vid_quality_token[0].id = 0;
		vid_quality_token[0].name = xr_strdup("renderer_r1");

		vid_quality_token[1].id = -1;
		vid_quality_token[1].name = NULL;

		return;
	}

	//	TODO: ask renderers if they are supported!
	if (vid_quality_token != NULL)
		return;

	bool bSupports_r2 = false;
	bool bSupports_r2_5 = false;
	bool bSupports_r3 = false;
	bool bSupports_r4 = false;

	LPCSTR			r2_name = "xrRender_R2.dll";
	LPCSTR			r3_name = "xrRender_R3.dll";
	LPCSTR			r4_name = "xrRender_R4.dll";

#ifdef STATICRENDER_R2
	// try to initialize R2
	Log("Loading DLL:", r2_name);
	//hRender = LoadLibrary(r2_name);
	DllMainRenderR2(NULL, DLL_PROCESS_ATTACH, NULL);
	//if (hRender)
	{
		bSupports_r2 = true;
		//SupportsAdvancedRenderingREF* test_rendering = (SupportsAdvancedRenderingREF*)GetProcAddress(hRender, "SupportsAdvancedRendering");
		SupportsAdvancedRenderingREF* test_rendering = SupportsAdvancedRendering;
		R_ASSERT(test_rendering);
		bSupports_r2_5 = test_rendering();
		//FreeLibrary(hRender);
		Msg("Support R4: %d", bSupports_r2_5);
	}
#endif

#ifdef STATICRENDER_R3
	// try to initialize R3
	Log("Loading DLL:", r3_name);
	// Hide "d3d10.dll not found" message box for XP
	SetErrorMode(SEM_FAILCRITICALERRORS);
	//hRender = LoadLibrary(r3_name);
	DllMainRenderR3(NULL, DLL_PROCESS_ATTACH, NULL);
	// Restore error handling
	SetErrorMode(0);
	//if (hRender)
	{
		//SupportsDX10RenderingREF* test_dx10_rendering = (SupportsDX10RenderingREF*)GetProcAddress(hRender, "SupportsDX10Rendering");
		SupportsDX10RenderingREF* test_dx10_rendering = SupportsDX10Rendering;
		R_ASSERT(test_dx10_rendering);
		bSupports_r3 = test_dx10_rendering();
		//FreeLibrary(hRender);
		Msg("Support R4: %d", bSupports_r3);
	}
#endif

#ifdef STATICRENDER_R4
	// try to initialize R4
	Log("Loading DLL:", r4_name);
	// Hide "d3d10.dll not found" message box for XP
	SetErrorMode(SEM_FAILCRITICALERRORS);
	//hRender = LoadLibrary(r4_name);
	DllMainRenderR4(NULL, DLL_PROCESS_ATTACH, NULL);
	// Restore error handling
	SetErrorMode(0);
	//if (hRender)
	{
		//SupportsDX11RenderingREF* test_dx11_rendering = (SupportsDX11RenderingREF*)GetProcAddress(hRender, "SupportsDX11Rendering");
		SupportsDX11RenderingREF* test_dx11_rendering = SupportsDX11Rendering;
		R_ASSERT(test_dx11_rendering);
		bSupports_r4 = test_dx11_rendering();
		//FreeLibrary(hRender);
		Msg("Support R4: %d", bSupports_r4);
	}
#endif

	Msg("Renders: R2: [%d], R2.5: [%d], R3: [%d], R4: [%d]", bSupports_r2, bSupports_r2_5, bSupports_r3, bSupports_r4);

	hRender = 0;

	xr_vector<LPCSTR>			_tmp;
	u32 i = 0;
	bool bBreakLoop = false;
	for (; i < 5; ++i)
	{
		switch (i)
		{
		case 1:
			if (!bSupports_r2)
				bBreakLoop = true;
			break;
		case 3:		//"renderer_r2.5"
			if (!bSupports_r2_5)
				bBreakLoop = true;
			break;
		case 4:
			if (!bSupports_r4)
				bBreakLoop = true;
			break;
		default:;
		}

		if (bBreakLoop)
			break;

		_tmp.push_back(NULL);
		LPCSTR val = NULL;
		switch (i)
		{
		case 0: val = "renderer_r1";			break;
		case 1: val = "renderer_r2a";		break;
		case 2: val = "renderer_r2";			break;
		case 3: val = "renderer_r2.5";		break;
		case 4: val = "renderer_r4";			break; //  -)
		}
		if (bBreakLoop) break;
		_tmp.back() = xr_strdup(val);
	}

	u32 _cnt = 2;

	vid_quality_token = xr_alloc<xr_token>(_cnt);

	if (bSupports_r2 && !bSupports_r2_5)
	{
		vid_quality_token[0].id = 2;
		vid_quality_token[0].name = _strdup("renderer_r2");
	}
	else
		if (bSupports_r2_5)
		{
			vid_quality_token[0].id = 3;
			vid_quality_token[0].name = _strdup("renderer_r2.5");
		}
		else
			if (bSupports_r3)
			{
				vid_quality_token[0].id = 4;
				vid_quality_token[0].name = _strdup("renderer_r3");
			}
			else
				if (bSupports_r4)
				{
					vid_quality_token[0].id = 5;
					vid_quality_token[0].name = _strdup("renderer_r4");
				}
				else
				{
					vid_quality_token[0].id = 0;
					vid_quality_token[0].name = _strdup("renderer_r1");
				}

	vid_quality_token[_cnt - 1].id = -1;
	vid_quality_token[_cnt - 1].name = NULL;

	vid_quality_token[1].id = -1;
	vid_quality_token[1].name = NULL;

	Msg("DEBUG_RENDERS: [%s]", _tmp[0]);
}