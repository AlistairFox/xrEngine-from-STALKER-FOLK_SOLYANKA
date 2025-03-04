#include "stdafx.h"
#include "GameFont.h"
#pragma hdrstop

#include "../xrcdb/ISpatial.h"
#include "IGame_Persistent.h"
#include "render.h"
#include "xr_object.h"

#include "../Include/xrRender/DrawUtils.h"
#include "IGame_Level.h"

int		g_ErrorLineCount	= 15;
Flags32 g_stats_flags		= {0};

// stats
DECLARE_RP(Stats);

class	optimizer	{
	float	average_	;
	BOOL	enabled_	;
public:
	optimizer	()		{
		average_	= 30.f;
//		enabled_	= TRUE;
//		disable		();
		// because Engine is not exist
		enabled_	= FALSE;
	}

	BOOL	enabled	()	{ return enabled_;	}
	void	enable	()	{ if (!enabled_)	{ Engine.External.tune_resume	();	enabled_=TRUE;	}}
	void	disable	()	{ if (enabled_)		{ Engine.External.tune_pause	();	enabled_=FALSE; }}
	void	update	(float value)	{
		if (value < average_*0.7f)	{
			// 25% deviation
			enable	();
		} else {
			disable	();
		};
		average_	= 0.99f*average_ + 0.01f*value;
	};
};
static	optimizer	vtune;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BOOL			g_bDisableRedText	= FALSE;
CStats::CStats	()
{
	fFPS				= 30.f;
	fRFPS				= 30.f;
	fTPS				= 0;
	pFont				= 0;
	fMem_calls			= 0;
	RenderDUMP_DT_Count = 0;
	Device.seqRender.Add		(this,REG_PRIORITY_LOW-1000);
}

CStats::~CStats()
{
	Device.seqRender.Remove		(this);
	xr_delete		(pFont);
}

void _draw_cam_pos(CGameFont* pFont)
{
	float sz		= pFont->GetHeight();
	pFont->SetHeightI(0.02f);
	pFont->SetColor	(0xffffffff);
	pFont->Out		(10, 600, "CAMERA POSITION:  [%3.2f,%3.2f,%3.2f]",VPUSH(Device.vCameraPosition));
	pFont->SetHeight(sz);
	pFont->OnRender	();
}
 
void drawStatParam(CGameFont* F, LPCSTR text)
{
	F->SetColor(color_argb(255, 128, 128, 192));
	F->OutNext(text);
}


void drawStatParam_Calls(CGameFont* F, CStats* stats, LPCSTR text, u32 stat)
{
	F->SetColor(color_argb(255, 128, 255, 0));
	F->OutNext(text, stat);
};

void drawStatParamByMS(CGameFont* F, CStats* stats, LPCSTR text, float stat)
{
	if (stat < 3.0f)
		F->SetColor(color_argb(255, 128, 128, 192));
	else
	if (stat > 3.0f && stat < 5.0f)
		F->SetColor(color_argb(255, 255, 0, 128));
	else
		F->SetColor(color_argb(255, 255, 0, 0));

	F->OutNext(text, stat);
};


void drawStatParamByMS(CGameFont* F, CStats* stats, LPCSTR text, float stat, float stat2)
{
	if (stat < 3.0f)
		F->SetColor(color_argb(255, 128, 128, 192));
	else
	if (stat > 3.0f && stat < 5.0f)
		F->SetColor(color_argb(255, 255, 0, 128));
	else
		F->SetColor(color_argb(255, 255, 0, 0));

	F->OutNext(text, stat, stat2);
};

void drawStatParamByMS(CGameFont* F, CStats* stats, LPCSTR text, float stat, u32 stat2, float stat3)
{
	if (stat < 3.0f)
		F->SetColor(color_argb(255, 128, 128, 192));
	else
	if (stat > 3.0f && stat < 5.0f)
		F->SetColor(color_argb(255, 255, 0, 128));
	else
		F->SetColor(color_argb(255, 255, 0, 0));

	F->OutNext(text, stat, stat2, stat3);
};


void CStats::Show() 
{
	// calc FPS & TPS
	if (Device.fTimeDelta > EPS_S)
	{
		float fps = 1.f / Device.fTimeDelta;
		//if (Engine.External.tune_enabled)	vtune.update	(fps);
		float fOne = 0.3f;
		float fInv = 1.f - fOne;
		fFPS = fInv * fFPS + fOne * fps;
 
		if (RenderTOTAL.result > EPS_S)
		{
			u32	rendered_polies = Device.m_pRender->GetCacheStatPolys();
			fTPS = fInv * fTPS + fOne * float(rendered_polies) / (RenderTOTAL.result * 1000.f);
			//fTPS = fInv*fTPS + fOne*float(RCache.stat.polys)/(RenderTOTAL.result*1000.f);
			fRFPS = fInv * fRFPS + fOne * 1000.f / RenderTOTAL.result;
		}
	}

	{
		float mem_count = float(Memory.stat_calls);
		if (mem_count > fMem_calls)	fMem_calls = mem_count;
		else						fMem_calls = .9f * fMem_calls + .1f * mem_count;
		Memory.stat_calls = 0;
	}

	////////////////////////////////////////////////
	if (g_dedicated_server) return;
	////////////////////////////////////////////////


	// Stop timers
 	{
		for (auto& stat : FrameTicks)
			stat.second.FrameEnd();

		EngineFrame.FrameEnd();
		EngineMTFrame.FrameEnd();
		EngineMTFrameSCore.FrameEnd();

		Sheduler.FrameEnd();
		ShedulerLow.FrameEnd();

		UpdateClient.FrameEnd();
		Physics.FrameEnd();
		ph_collision.FrameEnd();
		ph_core.FrameEnd();
		Animation.FrameEnd();
		AI_Think.FrameEnd();
		AI_Range.FrameEnd();
		AI_Path.FrameEnd();
		AI_Node.FrameEnd();
		AI_Vis.FrameEnd();
		AI_Vis_Query.FrameEnd();
		AI_Vis_RayTests.FrameEnd();

		RenderTOTAL.FrameEnd();
		RenderCALC.FrameEnd();
		RenderCALC_HOM.FrameEnd();
		RenderDUMP.FrameEnd();
		RenderDUMP_RT.FrameEnd();
		RenderDUMP_SKIN.FrameEnd();
		RenderDUMP_Wait.FrameEnd();
		RenderDUMP_Wait_S.FrameEnd();
		RenderDUMP_HUD.FrameEnd();
		RenderDUMP_Glows.FrameEnd();
		RenderDUMP_Lights.FrameEnd();
		RenderDUMP_WM.FrameEnd();
		RenderDUMP_DT_VIS.FrameEnd();
		RenderDUMP_DT_Render.FrameEnd();
		RenderDUMP_DT_Cache.FrameEnd();
		RenderDUMP_Pcalc.FrameEnd();
		RenderDUMP_Scalc.FrameEnd();
		RenderDUMP_Srender.FrameEnd();

		Sound.FrameEnd();
		Input.FrameEnd();
		clRAY.FrameEnd();
		clBOX.FrameEnd();
		clFRUSTUM.FrameEnd();

		netClient1.FrameEnd();
		netClient2.FrameEnd();
		netServer.FrameEnd();

		netClientCompressor.FrameEnd();
		netServerCompressor.FrameEnd();

		TEST0.FrameEnd();
		TEST1.FrameEnd();
		TEST2.FrameEnd();
		TEST3.FrameEnd();

		g_SpatialSpace->stat_insert.FrameEnd();
		g_SpatialSpace->stat_remove.FrameEnd();
		g_SpatialSpacePhysic->stat_insert.FrameEnd();
		g_SpatialSpacePhysic->stat_remove.FrameEnd();


		RenderMain.FrameEnd();
		RenderMain_Calcualte.FrameEnd();

		RenderMainVIS_Static.FrameEnd();
		RenderMainVIS_Dynamic.FrameEnd();
		RenderSun.FrameEnd();
		RenderLights.FrameEnd();
		Render_shadow_mp_process.FrameEnd();
		Render_dsgHUD_UI.FrameEnd();
		Render_dsgrender.FrameEnd();
		Render_postprocess.FrameEnd();
		Render_mssa.FrameEnd();

		NetworkSpawnCreate_xrEngine.FrameEnd();
		NetworkSpawn.FrameEnd();
		NetworkSpawn_ProcessCSE.FrameEnd();
		NetworkRelcase.FrameEnd();

		Dynamic_Rendarable.FrameEnd();
		Dynamic_HOM_Light.FrameEnd();
		Dynamic_HOM.FrameEnd();
		Dynamic_Visible_Check.FrameEnd();

		Particles_update_Time.FrameEnd();
		Particles_render_Time.FrameEnd();


		ThreadEngine.FrameEnd();
		ThreadParticles.FrameEnd();
		ThreadSecond.FrameEnd();


		OnFrame1.FrameEnd();
		OnFrame2.FrameEnd();
		OnFrame3.FrameEnd();
		OnFrame4.FrameEnd();

		// UpdateCL DATA !!!!
		UpdateClientReal.FrameEnd();
		UpdateClientPH.FrameEnd();
		UpdateClientUnsorted.FrameEnd();
		UpdateClientA.FrameEnd();
		UpdateClientAI_mutant.FrameEnd();
		UpdateClientAI.FrameEnd();
		UpdateClientInv.FrameEnd();
		UpdateClientZones.FrameEnd();

		RenderMainVIS_StaticTraverce.FrameEnd();
	}

	///////////////////////////////////////////////////
	/////////////// TIMERS END
	//////////////////////////////////////////////////

	if (!pFontGame)
	{
		Msg("!pFont");
		pFontGame = xr_new<CGameFont>("ui_font_graffiti22_russian", CGameFont::fsDeviceIndependent);
	}



	CGameFont& F = *pFontGame;
	float		f_base_size = 0.015f;
	F.SetHeightI(f_base_size);

	pFontGame->OutSet(100, 10);

	bool any_flag = psDeviceFlags.test(rsStatistic_Advanced) || psDeviceFlags.test(rsStatistic_fps) || psDeviceFlags.test(rsStatistic);

	if (any_flag)
	{
		pFontGame->SetHeightI(0.02f);
		F.SetColor(color_argb(255, 128, 128, 192));
		F.OutSet(1550, 20);
	}

	if (psDeviceFlags.test(rsStatistic_Advanced))
	{
		F.OutNext("FPS:   %3.0f", fFPS);
 
		drawStatParamByMS(pFontGame, this, "ThreadMain:					%2.4fms", ThreadEngine.result);
		drawStatParamByMS(pFontGame, this, "ThreadSecond:				%2.4fms", ThreadSecond.result);
		drawStatParamByMS(pFontGame, this, "ThreadParticles:			%2.4fms", ThreadParticles.result);

		drawStatParamByMS(pFontGame, this, "R_Main_StaticTraver			%2.4fms", RenderMainVIS_StaticTraverce.result);
		drawStatParamByMS(pFontGame, this, "R_Main_Static:				%2.4fms", RenderMainVIS_Static.result);
		drawStatParamByMS(pFontGame, this, "Dynamic_Rendarable:			%2.4fms", Dynamic_Rendarable.result);
 
		drawStatParamByMS(pFontGame, this, "clRay:						%2.4fms / %u", clRAY.result, clRAY.count);
		drawStatParamByMS(pFontGame, this, "clBOX:						%2.4fms / %u", clBOX.result, clBOX.count);


		drawStatParamByMS(pFontGame, this, "Particles_update:			%2.4fms", Particles_update_Time.result);
		drawStatParamByMS(pFontGame, this, "Particles_render:			%2.4fms", Particles_render_Time.result);
	}
	else
		if (psDeviceFlags.test(rsStatistic_fps))
		{
			F.SetColor(color_rgba(255, 0, 0, 255));
 			F.SetColor(color_rgba(128, 128, 192, 255));
			F.OutNext("FPS:   %3.0f", fFPS);

			drawStatParamByMS(pFontGame, this, "ThreadMain:			%2.4fms", ThreadEngine.result);
			drawStatParamByMS(pFontGame, this, "ThreadSecond:		%2.4fms", ThreadSecond.result);
			drawStatParam(pFontGame, "----------------");

			drawStatParamByMS(pFontGame, this, "EngineFrame:		%2.4fms", EngineFrame.result);

			drawStatParam(pFontGame, "----------------");
			drawStatParamByMS(pFontGame, this, "uUpdateCL:			%2.4fms | %2.4fms(relcase)", UpdateClient.result, NetworkRelcase.result);
			drawStatParamByMS(pFontGame, this, "uShedule:			%2.4fms | %2.4fms|(%.3f)", Sheduler.result, ShedulerLow.result, fShedulerLoad);

			drawStatParam(pFontGame, "----------------");
			drawStatParamByMS(pFontGame, this, "Render:				%2.4fms", RenderTOTAL_Real.result);

			drawStatParam(pFontGame, "----------------");
			drawStatParamByMS(pFontGame, this, "Render Wait Gpus:	%2.4fms", RenderDUMP_Wait_S.result);
			drawStatParamByMS(pFontGame, this, "Render Build Graph:	%2.4fms", RenderMain.result);
			drawStatParamByMS(pFontGame, this, "Render GPU Draw:	%2.4fms", RenderDUMP.result);
			// drawStatParamByMS(pFontGame, this, "Render GPU DrawFwd:	%2.4fms", RenderDUMP_Second.result);

			drawStatParam(pFontGame, "----------------");
			drawStatParamByMS(pFontGame, this, "R_Main_Sun:			%2.4fms", RenderSun.result);
			drawStatParamByMS(pFontGame, this, "R_Main_Lights:		%2.4fms", RenderLights.result);
			drawStatParamByMS(pFontGame, this, "R_Main_Shadow:		%2.4fms", Render_shadow_mp_process.result);
			drawStatParamByMS(pFontGame, this, "R_Postprocess:		%2.4fms", Render_postprocess.result);

			drawStatParamByMS(pFontGame, this, "RDT_Ren:   %2.4fms", RenderDUMP_DT_Render.result);
			drawStatParamByMS(pFontGame, this, "RDT_Vis:   %2.4fms", RenderDUMP_DT_VIS.result);
			drawStatParamByMS(pFontGame, this, "RDT_Cache: %2.4fms", RenderDUMP_DT_Cache.result);


			drawStatParam(pFontGame, "----------------");
			u32 dcalls; u32 verts; u32 polys;
			m_pRender->DrawCalls(dcalls);
			m_pRender->DrawVerticy(verts);
			m_pRender->DrawPoly(polys);

			drawStatParam_Calls(pFontGame, this, "Draw Calls(DPI):		%u", dcalls);
			drawStatParam_Calls(pFontGame, this, "Draw Vertex:			%u", verts);
			drawStatParam_Calls(pFontGame, this, "Draw Pollys:			%u", polys);

			drawStatParam(pFontGame, "----------------");
			pFontGame->OutNext("UpdateCL Real: %.2fms", UpdateClientReal.result);
			pFontGame->OutNext("PH:   %.2fms, NPC:   %.2fms, Monster: %.2fms", UpdateClientPH.result, UpdateClientAI.result, UpdateClientAI_mutant.result);
			pFontGame->OutNext("Item: %.2fms, Actor: %.2fms, Zones: %.2fms| OTHER: %.2fms", UpdateClientInv.result, UpdateClientA.result, UpdateClientZones.result, UpdateClientUnsorted.result);


			drawStatParam(pFontGame, "----------------");
			if (g_pGameLevel != nullptr)
				g_pGameLevel->OnStatsNetwork(pFontGame);
		}
		 
		else
			if (psDeviceFlags.test(rsStatistic))
			{
 				F.OutNext("FPS:   %3.0f", fFPS);

				drawStatParamByMS(pFontGame, this, "EngineMTFrame: %2.4fms", EngineMTFrame.result);
				drawStatParamByMS(pFontGame, this, "EngineFrame:		%2.4fms", EngineFrame.result);

				drawStatParamByMS(pFontGame, this, "uUpdateCL:			%2.4fms", UpdateClient.result);
				drawStatParamByMS(pFontGame, this, "uShedule:			%2.4fms", Sheduler.result);
				drawStatParamByMS(pFontGame, this, "uSheduleLov:		%2.4fms", ShedulerLow.result);

				F.OutNext("RenderVis: %d, RenderInv: %d", RenderVISIBLE, RenderINVISIBLE);

				//drawStatParamByMS(pFontGame, this, "Memory:      %2.2fa", fMem_calls);
				//drawStatParamByMS(pFontGame, this, "Network_xrEngine: %2.2f ms", NetworkSpawnCreate_xrEngine.result);
				//drawStatParamByMS(pFontGame, this, "Network_Spawn:    %2.2f ms", NetworkSpawn.result);
				//drawStatParamByMS(pFontGame, this, "Network_CSE:      %2.2f ms", NetworkSpawn_ProcessCSE.result);		
				//drawStatParamByMS(pFontGame, this, "Network_Relcase	  %2.2f ms", NetworkRelcase.result);

				F.OutNext("uParticles:  Qstart[%d] Qactive[%d] Qdestroy[%d]", Particles_starting, Particles_active, Particles_destroy);

				drawStatParamByMS(pFontGame, this, "Physics:     %2.2fms, %2.1f%%", Physics.result);
				drawStatParamByMS(pFontGame, this, "  collider:  %2.2fms", ph_collision.result);
				drawStatParamByMS(pFontGame, this, "  solver:    %2.2fms, %d", ph_core.result, ph_core.count);

				//if (Device.PhysicData.size() > 0)
				//	F.OutNext	(Device.PhysicData.c_str());

				//drawStatParamByMS	(pFontGame, this, "aiThink:     %2.2fms, %d",AI_Think.result, AI_Think.count);	
				//drawStatParamByMS	(pFontGame, this, "  aiRange:   %2.2fms, %d",AI_Range.result, AI_Range.count);
				//drawStatParamByMS	(pFontGame, this, "  aiPath:    %2.2fms, %d",AI_Path.result,  AI_Path.count);
				//drawStatParamByMS	(pFontGame, this, "  aiNode:    %2.2fms, %d",AI_Node.result,  AI_Node.count);
				//drawStatParamByMS	(pFontGame, this, "aiVision:    %2.2fms, %d",AI_Vis.result,  AI_Vis.count);
				//drawStatParamByMS	(pFontGame, this, "  Query:     %2.2fms",	AI_Vis_Query.result);
				//drawStatParamByMS	(pFontGame, this, "  RayCast:   %2.2fms",	AI_Vis_RayTests.result);
				//drawStatParamByMS	(pFontGame, this, "netClientRecv:   %2.2fms, %d",	netClient1.result, netClient1.count);
				//drawStatParamByMS	(pFontGame, this, "netClientSend:   %2.2fms, %d",	netClient2.result, netClient2.count);

				m_pRender->OutDetails(*pFontGame);

				drawStatParamByMS(pFontGame, this, "Render:				%2.4fms", RenderTOTAL_Real.result);


				drawStatParamByMS(pFontGame, this, "Render Dump:		%2.4fms", RenderDUMP.result);
				drawStatParamByMS(pFontGame, this, "Render WaitSync:	%2.4fms", RenderDUMP_Wait_S.result);
				drawStatParamByMS(pFontGame, this, "Render WaitLight:	%2.4fms", RenderDUMP_Wait.result);


				drawStatParamByMS(pFontGame, this, "R_Main:				%2.4fms", RenderMain.result);
				drawStatParamByMS(pFontGame, this, "R_Calculate:		%2.4fms", RenderMain_Calcualte.result);


				drawStatParamByMS(pFontGame, this, "R_Main_StaticTraver %2.4fms", RenderMainVIS_StaticTraverce.result);
				drawStatParamByMS(pFontGame, this, "R_Main_Static:		%2.4fms", RenderMainVIS_Static.result);
				drawStatParamByMS(pFontGame, this, "R_Main_Dynamic:		%2.4fms", RenderMainVIS_Dynamic.result);

				drawStatParamByMS(pFontGame, this, "R_Main_Sun:			%2.4fms", RenderSun.result);
				drawStatParamByMS(pFontGame, this, "R_Main_Lights:		%2.4fms", RenderLights.result);

				drawStatParamByMS(pFontGame, this, "R_Main_Shadow:		%2.4fms", Render_shadow_mp_process.result);
				drawStatParamByMS(pFontGame, this, "R_Main_DSG_HUD:		%2.4fms", Render_dsgHUD_UI.result);
				drawStatParamByMS(pFontGame, this, "R_Main_DSG_REN:		%2.4fms", Render_dsgrender.result);
				drawStatParamByMS(pFontGame, this, "R_Main_POST:		%2.4fms", Render_postprocess.result);

				drawStatParamByMS(pFontGame, this, "RenderDT:			%2.4fms", RenderDUMP_DT_Render.result);
				drawStatParamByMS(pFontGame, this, "RenderDT_VIS:		%2.4fms", RenderDUMP_DT_VIS.result);
				drawStatParamByMS(pFontGame, this, "RenderDT_CACHE:		%2.4fms", RenderDUMP_DT_Cache.result);
			}

 
	if (psDeviceFlags.test(rsCameraPos))
	{
		_draw_cam_pos(pFontGame);
	}

	if (any_flag)
		pFontGame->OnRender();


 	{
		for (auto& stat : FrameTicks)
			stat.second.FrameStart();

		Dynamic_Rendarable.FrameStart();
		Dynamic_HOM_Light.FrameStart();
		Dynamic_HOM.FrameStart();
		Dynamic_Visible_Check.FrameStart();

		EngineFrame.FrameStart();
		EngineMTFrame.FrameStart();
		EngineMTFrameSCore.FrameStart();

		Sheduler.FrameStart();
		ShedulerLow.FrameStart();
		UpdateClient.FrameStart();
		Physics.FrameStart();
		ph_collision.FrameStart();
		ph_core.FrameStart();
		Animation.FrameStart();
		AI_Think.FrameStart();
		AI_Range.FrameStart();
		AI_Path.FrameStart();
		AI_Node.FrameStart();
		AI_Vis.FrameStart();
		AI_Vis_Query.FrameStart();
		AI_Vis_RayTests.FrameStart();

		RenderTOTAL.FrameStart();
		RenderCALC.FrameStart();
		RenderCALC_HOM.FrameStart();
		RenderDUMP.FrameStart();
		RenderDUMP_RT.FrameStart();
		RenderDUMP_SKIN.FrameStart();
		RenderDUMP_Wait.FrameStart();
		RenderDUMP_Wait_S.FrameStart();
		RenderDUMP_HUD.FrameStart();
		RenderDUMP_Glows.FrameStart();
		RenderDUMP_Lights.FrameStart();
		RenderDUMP_WM.FrameStart();
		RenderDUMP_DT_VIS.FrameStart();
		RenderDUMP_DT_Render.FrameStart();
		RenderDUMP_DT_Cache.FrameStart();
		RenderDUMP_Pcalc.FrameStart();
		RenderDUMP_Scalc.FrameStart();
		RenderDUMP_Srender.FrameStart();

		Sound.FrameStart();
		Input.FrameStart();
		clRAY.FrameStart();
		clBOX.FrameStart();
		clFRUSTUM.FrameStart();

		netClient1.FrameStart();
		netClient2.FrameStart();
		netServer.FrameStart();
		netClientCompressor.FrameStart();
		netServerCompressor.FrameStart();

		TEST0.FrameStart();
		TEST1.FrameStart();
		TEST2.FrameStart();
		TEST3.FrameStart();

		g_SpatialSpace->stat_insert.FrameStart();
		g_SpatialSpace->stat_remove.FrameStart();

		g_SpatialSpacePhysic->stat_insert.FrameStart();
		g_SpatialSpacePhysic->stat_remove.FrameStart();

		RenderMain.FrameStart();
		RenderMain_Calcualte.FrameStart();

		RenderMainVIS_Static.FrameStart();
		RenderMainVIS_Dynamic.FrameStart();

		RenderSun.FrameStart();
		RenderLights.FrameStart();

		Render_shadow_mp_process.FrameStart();

		Render_dsgHUD_UI.FrameStart();
		Render_dsgrender.FrameStart();
		Render_postprocess.FrameStart();
		Render_mssa.FrameStart();

		NetworkSpawnCreate_xrEngine.FrameStart();
		NetworkSpawn.FrameStart();
		NetworkSpawn_ProcessCSE.FrameStart();
		NetworkRelcase.FrameStart();

		Particles_update_Time.FrameStart();
		Particles_render_Time.FrameStart();

		ThreadEngine.FrameStart();
		ThreadParticles.FrameStart();
		ThreadSecond.FrameStart();

		OnFrame1.FrameStart();
		OnFrame2.FrameStart();
		OnFrame3.FrameStart();
		OnFrame4.FrameStart();

		// UpdateCL DATA !!!!
		UpdateClientReal.FrameStart();
		UpdateClientPH.FrameStart();
		UpdateClientUnsorted.FrameStart();
		UpdateClientA.FrameStart();
		UpdateClientAI_mutant.FrameStart();
		UpdateClientAI.FrameStart();
		UpdateClientInv.FrameStart();
		UpdateClientZones.FrameStart();

		RenderMainVIS_StaticTraverce.FrameStart();
	}


	dwSND_Played = dwSND_Allocated = 0;
	Particles_starting = Particles_active = Particles_destroy = 0;
}

void	_LogCallback				(LPCSTR string)
{
	if (string && '!'==string[0] && ' '==string[1])
		Device.Statistic->errors.push_back	(shared_str(string));
}

void CStats::OnDeviceCreate			()
{
	g_bDisableRedText				= strstr(Core.Params,"-xclsx")?TRUE:FALSE;

//	if (!strstr(Core.Params, "-dedicated"))
#ifndef DEDICATED_SERVER
	pFont	= xr_new<CGameFont>		("stat_font", CGameFont::fsDeviceIndependent);
#endif
	
	if(!pSettings->section_exist("evaluation")
		||!pSettings->line_exist("evaluation","line1")
		||!pSettings->line_exist("evaluation","line2")
		||!pSettings->line_exist("evaluation","line3") )
		FATAL	("");

	eval_line_1 = pSettings->r_string_wb("evaluation","line1");
	eval_line_2 = pSettings->r_string_wb("evaluation","line2");
	eval_line_3 = pSettings->r_string_wb("evaluation","line3");

	// 
#ifdef DEBUG
	if (!g_bDisableRedText)			SetLogCB	(_LogCallback);
#endif
}

void CStats::OnDeviceDestroy		()
{
	SetLogCB(0);
	xr_delete	(pFont);
}

void CStats::OnRender				()
{
#ifdef DEBUG
	if (g_stats_flags.is(st_sound)){
		CSound_stats_ext				snd_stat_ext;
		::Sound->statistic				(0,&snd_stat_ext);
		CSound_stats_ext::item_vec_it	_I = snd_stat_ext.items.begin();
		CSound_stats_ext::item_vec_it	_E = snd_stat_ext.items.end();
		for (;_I!=_E;_I++){
			const CSound_stats_ext::SItem& item = *_I;
			if (item._3D)
			{
				m_pRender->SetDrawParams(&*Device.m_pRender);
				//RCache.set_xform_world(Fidentity);
				//RCache.set_Shader		(Device.m_SelectionShader);
				//RCache.set_c			("tfactor",1,1,1,1);
				DU->DrawCross			(item.params.position, 0.5f, 0xFF0000FF, true );
				if (g_stats_flags.is(st_sound_min_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.min_distance, 0x400000FF,	0xFF0000FF, true, true);
				if (g_stats_flags.is(st_sound_max_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.max_distance, 0x4000FF00,	0xFF008000, true, true);
				
				xr_string out_txt		= (out_txt.size() && g_stats_flags.is(st_sound_info_name)) ? item.name.c_str():"";

				if (item.game_object)
				{
					if (g_stats_flags.is(st_sound_ai_dist))
						DU->DrawSphere	(Fidentity, item.params.position, item.params.max_ai_distance, 0x80FF0000,0xFF800000,true,true);
					if (g_stats_flags.is(st_sound_info_object)){
						out_txt			+= "  (";
						out_txt			+= item.game_object->cNameSect().c_str();
						out_txt			+= ")";
					}
				}
				if (g_stats_flags.is_any(st_sound_info_name|st_sound_info_object) && item.name.size())
					DU->OutText			(item.params.position, out_txt.c_str(),0xFFFFFFFF,0xFF000000);
			}
		}
	}
#endif
}
