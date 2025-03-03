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
// d3dx9.h
#include <d3dx9.h>
#pragma warning(default:4995)

#include "x_ray.h"
#include "render.h"

// must be defined before include of FS_impl.h
#define INCLUDE_FROM_ENGINE
#include "../xrCore/FS_impl.h"

#ifdef INGAME_EDITOR
#	include "../include/editor/ide.hpp"
#	include "engine_impl.hpp"
#endif // #ifdef INGAME_EDITOR

#include "xrSash.h"
#include "igame_persistent.h"
#pragma comment( lib, "OptickCore.lib")

#pragma comment( lib, "d3dx9.lib"		)

ENGINE_API CRenderDevice Device;
ENGINE_API CLoadScreenRenderer load_screen_renderer;


ENGINE_API BOOL g_bRendering = FALSE;

BOOL		g_bLoaded = FALSE;
ref_light	precache_light = 0;

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

	FPU::m24r();
	g_bRendering = TRUE;
#endif
	return		TRUE;
}

void CRenderDevice::Clear()
{
	m_pRender->Clear();
}

extern void CheckPrivilegySlowdown();

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

			CheckPrivilegySlowdown();

			if (g_pGamePersistent->GameType() == 1)
			{
				WINDOWINFO	wi;
				GetWindowInfo(m_hWnd, &wi);
				if (wi.dwWindowStatus != WS_ACTIVECAPTION)
					Pause(TRUE, TRUE, TRUE, "application start");
			}
		}
	}

	g_bRendering = FALSE;

	//	Present goes here, so call OA Frame end.
	if (g_SASH.IsBenchmarkRunning())
		g_SASH.DisplayFrame(Device.fTimeGlobal);

	extern void ImGUI_OnRender();

	ImGUI_OnRender();

	m_pRender->End();
#endif
}
 
volatile u32	mt_Thread_marker = 0x12345678;
void 			mt_Thread(void* ptr)
{
	OPTICK_THREAD("X-RAY SECONDARY THREAD");

	while (true)
	{
		OPTICK_FRAME("X-RAY SECONDARY FRAME");
		// waiting for Device permission to execute
		Device.mt_csEnter.Enter();

		if (Device.mt_bMustExit)
		{
			Device.mt_bMustExit = FALSE;				// Important!!!
			Device.mt_csEnter.Leave();					// Important!!!
			return;
		}

		// we has granted permission to execute
		mt_Thread_marker = Device.dwFrame;

		{
			OPTICK_EVENT("seqParralel (MT)");
			for (u32 pit = 0; pit < Device.seqParallel.size(); pit++)
				Device.seqParallel[pit]();
		}


		Device.seqParallel.clear_not_free();

		{
			OPTICK_EVENT("OnFrame (MT)");
			Device.seqFrameMT.Process(rp_Frame);
		}
 
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


extern int g_svDedicateServerUpdateReate = 100;

ENGINE_API xr_list<LOADING_EVENT>			g_loading_events;
extern int fps_limit;

extern void ImGui_NewFrame();
extern void ImGui_EndFrame();

void CRenderDevice::UpdateCamera()
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

	vCameraPosition_saved = vCameraPosition;
	mFullTransform_saved = mFullTransform;
	mView_saved = mView;
	mProject_saved = mProject;
}

void CRenderDevice::on_idle()
{
	OPTICK_FRAME("X-RAY MAIN FRAME");
	if (!b_is_Ready)
	{
		Sleep(100);
		return;
	}

	u32 FrameStartTime = TimerGlobal.GetElapsed_ms();
	g_bEnableStatGather = true;

	if (g_loading_events.size())
	{
		if (g_loading_events.front()())
			g_loading_events.pop_front();
		pApp->LoadDraw();
		return;
	}
 	
	
	if ((!Device.dwPrecacheFrame) && (!g_SASH.IsBenchmarkRunning()) && g_bLoaded)
		g_SASH.StartBenchmark();

#ifndef DEDICATED_SERVER
	ImGui_NewFrame();
#endif 

	FrameMove();
	//se7kills Refactory Camera
	UpdateCamera();

	// *** Resume threads
	// Capture end point - thread must run only ONE cycle
	// Release start point - allow thread to run
	mt_csLeave.Enter();
	mt_csEnter.Leave();
	Sleep(0);

#ifndef DEDICATED_SERVER
	Statistic->RenderTOTAL_Real.FrameStart();
	Statistic->RenderTOTAL_Real.Begin();
	if (b_is_Active)
	{
		OPTICK_EVENT("Render");
 		if (Begin())
		{
 			seqRender.Process(rp_Render);
			Statistic->Show();
			
			End();
			ImGui_EndFrame();
		}
 	}
	Statistic->RenderTOTAL_Real.End();
	Statistic->RenderTOTAL_Real.FrameEnd();
	Statistic->RenderTOTAL.accum = Statistic->RenderTOTAL_Real.accum;
#endif // #ifndef DEDICATED_SERVER

	if (g_dedicated_server)
		g_pGamePersistent->Statistics(nullptr);


	if (Device.fTimeDelta > EPS_S)
	{
		float fps = 1.f / Device.fTimeDelta;
 		float fOne = 0.3f;
		float fInv = 1.f - fOne;
		Statistic->fFPS = fInv * Statistic->fFPS + fOne * fps;

		if (Statistic->RenderTOTAL.result > EPS_S)
		{
 			Statistic->fTPS = fInv * Statistic->fTPS + fOne * float(Device.m_pRender->GetCacheStatPolys()) / (Statistic->RenderTOTAL.result * 1000.f);
 			Statistic->fRFPS = fInv * Statistic->fRFPS + fOne * 1000.f / Statistic->RenderTOTAL.result;
		}
	}

	// *** Suspend threads
	// Capture startup point
	// Release end point - allow thread to wait for startup point
	mt_csEnter.Enter();
	mt_csLeave.Leave();

	// Ensure, that second thread gets chance to execute anyway
 	if (dwFrame != mt_Thread_marker)
	{
		OPTICK_EVENT("seqParralel (MAIN)")
		for (u32 pit = 0; pit < Device.seqParallel.size(); pit++)
			Device.seqParallel[pit]();
		Device.seqParallel.clear_not_free();
		seqFrameMT.Process(rp_Frame);
	}
 
	//FPS LOCK FOR CLIENT
	if (!b_is_Active)
		Sleep(1);
}

void CRenderDevice::message_loop()
{
	MSG						msg;

	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
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
	OPTICK_THREAD("Xray Primary Thread");
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
	mt_csEnter.Enter();
	mt_bMustExit = FALSE;
	thread_spawn(mt_Thread, "X-RAY Secondary thread", 0, 0);

	// Message cycle
	seqAppStart.Process(rp_AppStart);
	m_pRender->ClearTarget();

	message_loop();

	seqAppEnd.Process(rp_AppEnd);

	// Stop Balance-Thread
	mt_bMustExit = TRUE;
	mt_csEnter.Leave();

	while (mt_bMustExit)	
		Sleep(0);
}

u32 app_inactive_time = 0;
u32 app_inactive_time_start = 0;

void CRenderDevice::FrameMove()
{
	dwFrame++;

	Core.dwFrame = dwFrame;

	dwTimeContinual = TimerMM.GetElapsed_ms() - app_inactive_time;

	if (psDeviceFlags.test(rsConstantFPS))
	{
		// 40 FPS 
		fTimeDelta = 0.016f;
		fTimeGlobal += 0.016f;
		dwTimeDelta = 16;
		dwTimeGlobal += 16;
	}
	else
	{
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
	Statistic->ThreadEngine.Begin();

	//	TODO: HACK to test loading screen.

	OPTICK_EVENT("OnFrame (MAIN)")
	Device.seqFrame.Process(rp_Frame);
	g_bLoaded = TRUE;
 

	Statistic->ThreadEngine.End();
}
 
ENGINE_API BOOL bShowPauseString = TRUE;
#include "IGame_Persistent.h"

void CRenderDevice::Pause(BOOL bOn, BOOL bTimer, BOOL bSound, LPCSTR reason)
{
	static int snd_emitters_ = -1;

	if (g_bBenchmark)	
		return;

#ifndef DEDICATED_SERVER	

	if (bOn)
	{
		if (!Paused())
			bShowPauseString = TRUE;

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
		if (bTimer && /*g_pGamePersistent->CanBePaused() &&*/ g_pauseMngr.Paused())
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
			else {
#ifdef DEBUG
				Log("Sound->pause_emitters underflow");
#endif // DEBUG
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
			ShowCursor(FALSE);
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
		seqFrame.Add(f, REG_PRIORITY_LOW, 0, "render");

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