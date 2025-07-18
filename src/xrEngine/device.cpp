#include "stdafx.h"
#include "../xrCDB/frustum.h"

#pragma warning(disable:4995)
// mmsystem.h
#define MMNOSOUND
#define MMNOMIDI
#define MMNOAUX
#define MMNOMIXER
#define MMNOJOY
#include <mmsystem.h>
#include <dxsdk/d3dx9.h>
#pragma warning(default:4995)

#include "x_ray.h"
#include "render.h"

#define INCLUDE_FROM_ENGINE
#include "../xrCore/FS_impl.h"
#include "igame_persistent.h"
 
#include "..\3rd party\imgui\imgui.h"
#include <chrono>

extern void ImGui_NewFrame();
extern void ImGui_EndFrame();
extern void ImGUI_OnRender();

ENGINE_API CRenderDevice Device;
ENGINE_API CLoadScreenRenderer load_screen_renderer;

std::chrono::high_resolution_clock::time_point tlastf = std::chrono::high_resolution_clock::now(), tcurrentf = std::
chrono::high_resolution_clock::now();
std::chrono::duration<float> time_span;
ENGINE_API float refresh_rate = 0;


ENGINE_API BOOL g_bRendering = FALSE;

BOOL		g_bLoaded = FALSE;
ref_light	precache_light = 0;

// need for imgui
static INT64 g_Time = 0;
static INT64 g_TicksPerSecond = 0;

BOOL CRenderDevice::Begin()
{
#ifndef DEDICATED_SERVER
	switch (m_pRender->GetDeviceState())
	{
	case IRenderDeviceRender::dsOK:
		break;

	case IRenderDeviceRender::dsLost:
		// If the device was lost, do not render until we get it back
		Sleep(33);
		return FALSE;
		break;

	case IRenderDeviceRender::dsNeedReset:
		// Check if the device is ready to be reset
		Reset();
		break;

	default:
		R_ASSERT(0);
	}

	m_pRender->Begin();
	g_bRendering = TRUE;
#endif

	return		TRUE;
}

void CRenderDevice::Clear()
{
	m_pRender->Clear();
}


void CRenderDevice::End(void)
{
#ifndef DEDICATED_SERVER
 
	if (dwPrecacheFrame)
	{
		::Sound->set_master_volume(0.f);
		dwPrecacheFrame--;

		if (0 == dwPrecacheFrame)
		{
			m_pRender->updateGamma();

			if (precache_light) precache_light->set_active(false);
			if (precache_light) precache_light.destroy();
			::Sound->set_master_volume(1.f);

			m_pRender->ResourcesDestroyNecessaryTextures();
			Memory.mem_compact();
			Msg("* MEMORY USAGE: %d K", Memory.mem_usage() / 1024);
			Msg("* End of synchronization A[%d] R[%d]", b_is_Active, b_is_Ready);

			if (g_pGamePersistent->GameType() == 1)//haCk
			{
				WINDOWINFO	wi;
				GetWindowInfo(m_hWnd, &wi);
				if (wi.dwWindowStatus != WS_ACTIVECAPTION)
					Pause(TRUE, TRUE, TRUE, "application start");
			}
		}
	}

	g_bRendering = FALSE;

	ImGUI_OnRender();
 	m_pRender->End();
 	ImGui_EndFrame();
#endif
}


volatile u32	mt_Thread_marker = 0x12345678;
void 			mt_Thread(void* ptr) {
	while (true) {
		// waiting for Device permission to execute
		Device.mt_csEnter.Enter();

		if (Device.mt_bMustExit) {
			Device.mt_bMustExit = FALSE;				// Important!!!
			Device.mt_csEnter.Leave();					// Important!!!
			return;
		}
		// we has granted permission to execute
		mt_Thread_marker = Device.dwFrame;

		for (u32 pit = 0; pit < Device.seqParallel.size(); pit++)
			Device.seqParallel[pit]();
		Device.seqParallel.clear_not_free();
		Device.seqFrameMT.Process(rp_Frame);

		// now we give control to device - signals that we are ended our work
		Device.mt_csEnter.Leave();
		// waits for device signal to continue - to start again
		Device.mt_csLeave.Enter();
		// returns sync signal to device
		Device.mt_csLeave.Leave();
	}
}

#include "igame_level.h"
void CRenderDevice::PreCache(u32 amount, bool b_draw_loadscreen, bool b_wait_user_input)
{
	if (m_pRender->GetForceGPU_REF()) amount = 0;
#ifdef DEDICATED_SERVER
	amount = 0;
#endif
	// Msg			("* PCACHE: start for %d...",amount);
	dwPrecacheFrame = dwPrecacheTotal = amount;
	if (amount && !precache_light && g_pGameLevel && g_loading_events.empty()) {
		precache_light = ::Render->light_create();
		precache_light->set_shadow(false);
		precache_light->set_position(vCameraPosition);
		precache_light->set_color(255, 255, 255);
		precache_light->set_range(5.0f);
		precache_light->set_active(true);
	}

	if (amount && b_draw_loadscreen && load_screen_renderer.b_registered == false)
	{
		load_screen_renderer.start(b_wait_user_input);
	}
}


int g_svDedicateServerUpdateReate = 100;

extern bool IsMainMenuActive(); //ECO_RENDER add

ENGINE_API xr_list<LOADING_EVENT>			g_loading_events;

float CRenderDevice::GetMonitorRefresh()
{
	DEVMODE lpDevMode;
	memset(&lpDevMode, 0, sizeof(DEVMODE));
	lpDevMode.dmSize = sizeof(DEVMODE);
	lpDevMode.dmDriverExtra = 0;

	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode) == 0)
	{
		return 1.f / 60.f;
	}
	else
		return 1.f / lpDevMode.dmDisplayFrequency;
}

extern int ps_framelimiter;

//#define MOVE_CURRENT_FRAME_COUNTR // This is to determine, if the second vp bugs are happening because there were no frame step

void CRenderDevice::UpdateRefresh()
{
	if (Device.Paused() || IsMainMenuActive() || ps_framelimiter)
	{
		if (refresh_rate == 0)
			refresh_rate = GetMonitorRefresh();

		float rr;

		if (ps_framelimiter)
			rr = 1.f / ps_framelimiter;
		else
			rr = refresh_rate;

		time_span = std::chrono::duration_cast<std::chrono::duration<float>>(tcurrentf - tlastf);
		while (time_span.count() < rr)
		{
			tcurrentf = std::chrono::high_resolution_clock::now();
			time_span = std::chrono::duration_cast<std::chrono::duration<float>>(tcurrentf - tlastf);
		}
		tlastf = std::chrono::high_resolution_clock::now();
	}
}

void CRenderDevice::MatrixUpdate()
{
 	// Precache
	if (dwPrecacheFrame)
	{
		float factor = float(dwPrecacheFrame) / float(dwPrecacheTotal);
		float angle = PI_MUL_2 * factor;
		vCameraDirection.set(_sin(angle), 0, _cos(angle));	vCameraDirection.normalize();
		vCameraTop.set(0, 1, 0);
		vCameraRight.crossproduct(vCameraTop, vCameraDirection);

		mView.build_camera_dir(vCameraPosition, vCameraDirection, vCameraTop);
	}

	// Matrices
	mFullTransform.mul(mProject, mView);
	m_pRender->SetCacheXform(mView, mProject);
	D3DXMatrixInverse((D3DXMATRIX*)&mInvFullTransform, 0, (D3DXMATRIX*)&mFullTransform);

	// Set "_saved" for this frame each vport
	vCameraPosition_saved = vCameraPosition;
	mFullTransform_saved = mFullTransform;
	mView_saved = mView;
	mProject_saved = mProject;

}

void CRenderDevice::on_idle()
{
	if (!b_is_Ready) 
	{
		Sleep(100);
		return;
	}


	if (psDeviceFlags.test(rsStatistic))
		g_bEnableStatGather = TRUE;
	else								
		g_bEnableStatGather = FALSE;

	if (g_loading_events.size())
	{
		if (g_loading_events.front()())
			g_loading_events.pop_front();
		pApp->LoadDraw();
		return;
	}
 
#ifndef DEDICATED_SERVER
	ImGui_NewFrame();
#endif 

	FrameMove();
	MatrixUpdate();

	if (!g_dedicated_server)
	{
		mt_csLeave.Enter();
		mt_csEnter.Leave();
 	}
	
	// Render Total

	Statistic->RenderTOTAL_Real.FrameStart();
	Statistic->RenderTOTAL_Real.Begin();
 	if (b_is_Active && Begin())
	{
		seqRender.Process(rp_Render);
		Statistic->Show();
		End();
 	}
 	Statistic->RenderTOTAL_Real.End();
	Statistic->RenderTOTAL_Real.FrameEnd();
 
	// *** Suspend threads
	// Capture startup point
	// Release end point - allow thread to wait for startup point
	if (!g_dedicated_server)
	{
		mt_csEnter.Enter();
		mt_csLeave.Leave();
	}

	// Ensure, that second thread gets chance to execute anyway
	if (dwFrame != mt_Thread_marker) {
		for (u32 pit = 0; pit < Device.seqParallel.size(); pit++)
			Device.seqParallel[pit]();
		Device.seqParallel.clear_not_free();
		seqFrameMT.Process(rp_Frame);
	}

	if (!b_is_Active)
		Sleep(1);
}

void CRenderDevice::message_loop()
{
	MSG						msg;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		on_idle();
	}
}

void CRenderDevice::Run()
{
	//	DUMP_PHASE;
	g_bLoaded = FALSE;
	Log("Starting engine...");
	thread_name("X-RAY Primary thread");

	// Startup timers and calculate timer delta
	dwTimeGlobal = 0;
	Timer_MM_Delta = 0;
	{
		u32 time_mm = timeGetTime();
		while (timeGetTime() == time_mm);			// wait for next tick
		u32 time_system = timeGetTime();
		u32 time_local = TimerAsync();
		Timer_MM_Delta = time_system - time_local;
	}

	// Start all threads
//	InitializeCriticalSection	(&mt_csEnter);
//	InitializeCriticalSection	(&mt_csLeave);
	mt_csEnter.Enter();
	mt_bMustExit = FALSE;
	thread_spawn(mt_Thread, "X-RAY Secondary thread", 0, 0);

	// Message cycle
	seqAppStart.Process(rp_AppStart);

	//CHK_DX(HW.pDevice->Clear(0,0,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1,0));
	m_pRender->ClearTarget();

	message_loop();

	seqAppEnd.Process(rp_AppEnd);

	// Stop Balance-Thread
	mt_bMustExit = TRUE;
	mt_csEnter.Leave();
	while (mt_bMustExit)	Sleep(0);
	//	DeleteCriticalSection	(&mt_csEnter);
	//	DeleteCriticalSection	(&mt_csLeave);
}

u32 app_inactive_time = 0;
u32 app_inactive_time_start = 0;

void ProcessLoading(RP_FUNC* f);
void CRenderDevice::FrameMove()
{
	dwFrame++;

	Core.dwFrame = dwFrame;

	dwTimeContinual = TimerMM.GetElapsed_ms() - app_inactive_time;

	if (psDeviceFlags.test(rsConstantFPS)) {
		// 20ms = 50fps
		//fTimeDelta		=	0.020f;			
		//fTimeGlobal		+=	0.020f;
		//dwTimeDelta		=	20;
		//dwTimeGlobal	+=	20;
		// 33ms = 30fps
		fTimeDelta = 0.033f;
		fTimeGlobal += 0.033f;
		dwTimeDelta = 33;
		dwTimeGlobal += 33;
	}
	else {
		// Timer
		float fPreviousFrameTime = Timer.GetElapsed_sec(); Timer.Start();	// previous frame
		fTimeDelta = 0.1f * fTimeDelta + 0.9f * fPreviousFrameTime;			// smooth random system activity - worst case ~7% error
		//fTimeDelta = 0.7f * fTimeDelta + 0.3f*fPreviousFrameTime;			// smooth random system activity
		if (fTimeDelta > .1f)
			fTimeDelta = .1f;							// limit to 15fps minimum

		if (fTimeDelta <= 0.f)
			fTimeDelta = EPS_S + EPS_S;					// limit to 15fps minimum

		if (Paused())
			fTimeDelta = 0.0f;

		//		u64	qTime		= TimerGlobal.GetElapsed_clk();
		fTimeGlobal = TimerGlobal.GetElapsed_sec(); //float(qTime)*CPU::cycles2seconds;
		u32	_old_global = dwTimeGlobal;
		dwTimeGlobal = TimerGlobal.GetElapsed_ms();
		dwTimeDelta = dwTimeGlobal - _old_global;
	}

	// Frame move
	ProcessLoading(rp_Frame);

}

void ProcessLoading(RP_FUNC* f)
{
	Device.seqFrame.Process(rp_Frame);
	g_bLoaded = TRUE;
}

ENGINE_API BOOL bShowPauseString = TRUE;
#include "IGame_Persistent.h"

void CRenderDevice::Pause(BOOL bOn, BOOL bTimer, BOOL bSound, LPCSTR reason)
{
	static int snd_emitters_ = -1;

#ifndef DEDICATED_SERVER	

	if (bOn)
	{
		if (!Paused())
			bShowPauseString =	TRUE;

		if (bTimer && (!g_pGamePersistent || g_pGamePersistent->CanBePaused()))
		{
			g_pauseMngr.Pause(TRUE);
		}

		if (bSound && ::Sound) {
			snd_emitters_ = ::Sound->pause_emitters(true);
		}
	}
	else
	{
		if (bTimer && g_pauseMngr.Paused())
		{
			fTimeDelta = EPS_S + EPS_S;
			g_pauseMngr.Pause(FALSE);
		}

		if (bSound)
		{
			if (snd_emitters_ > 0) //avoid crash
			{
				snd_emitters_ = ::Sound->pause_emitters(false);
			}
		}
	}

#endif

}

BOOL CRenderDevice::Paused()
{
	return g_pauseMngr.Paused();
};

void CRenderDevice::OnWM_Activate(WPARAM wParam, LPARAM lParam)
{
	u16 fActive = LOWORD(wParam);
	BOOL fMinimized = (BOOL)HIWORD(wParam);
	BOOL bActive = ((fActive != WA_INACTIVE) && (!fMinimized)) ? TRUE : FALSE;

	if (bActive != Device.b_is_Active)
	{
		Device.b_is_Active = bActive;

		if (Device.b_is_Active)
		{
			Device.seqAppActivate.Process(rp_AppActivate);
			app_inactive_time += TimerMM.GetElapsed_ms() - app_inactive_time_start;

#ifndef DEDICATED_SERVER
			ShowCursor(FALSE);
#endif // #ifndef DEDICATED_SERVER
		}
		else
		{
			app_inactive_time_start = TimerMM.GetElapsed_ms();
			Device.seqAppDeactivate.Process(rp_AppDeactivate);
			ShowCursor(TRUE);
		}
	}
}

void	CRenderDevice::AddSeqFrame(pureFrame* f, bool mt)
{
	if (mt)
		seqFrameMT.Add(f, REG_PRIORITY_HIGH);
	else
		seqFrame.Add(f, REG_PRIORITY_LOW);

}

void	CRenderDevice::RemoveSeqFrame(pureFrame* f)
{
	seqFrameMT.Remove(f);
	seqFrame.Remove(f);
}

CLoadScreenRenderer::CLoadScreenRenderer()
	:b_registered(false)
{
}

void CLoadScreenRenderer::start(bool b_user_input)
{
	Device.seqRender.Add(this, 0);
	b_registered = true;
	b_need_user_input = b_user_input;
}

void CLoadScreenRenderer::stop()
{
	if (!b_registered)				return;
	Device.seqRender.Remove(this);
	pApp->destroy_loading_shaders();
	b_registered = false;
	b_need_user_input = false;
}

void CLoadScreenRenderer::OnRender()
{
	pApp->load_draw_internal();
}