#include "stdafx.h"
#include "IGame_Persistent.h"

#include "environment.h"
#include "x_ray.h"
#include "IGame_Level.h"
#include "XR_IOConsole.h"
#include "Render.h"
#include "ps_instance.h"
#include "CustomHUD.h"
#include "perlin.h"

ENGINE_API	IGame_Persistent*		g_pGamePersistent	= NULL;

extern Fvector4 ps_ssfx_grass_interactive;

IGame_Persistent::IGame_Persistent	()
{
	RDEVICE.seqAppStart.Add			(this);
	RDEVICE.seqAppEnd.Add			(this);
	RDEVICE.seqFrame.Add			(this,REG_PRIORITY_HIGH+1, 0, "render");
	RDEVICE.seqAppActivate.Add		(this);
	RDEVICE.seqAppDeactivate.Add	(this);

	m_pMainMenu						= NULL;

	pEnvironment					= xr_new<CEnvironment>();
	m_pGShaderConstants = new ShadersExternalData();
}

IGame_Persistent::~IGame_Persistent	()
{
	RDEVICE.seqFrame.Remove			(this);
	RDEVICE.seqAppStart.Remove		(this);
	RDEVICE.seqAppEnd.Remove			(this);
	RDEVICE.seqAppActivate.Remove	(this);
	RDEVICE.seqAppDeactivate.Remove	(this);

	xr_delete (pEnvironment);
	xr_delete (m_pGShaderConstants);
}

void IGame_Persistent::OnAppActivate		()
{
}

void IGame_Persistent::OnAppDeactivate		()
{
}

void IGame_Persistent::OnAppStart	()
{
	Environment().load				();   
}

void IGame_Persistent::OnAppEnd		()
{
	Environment().unload			 ();    
	OnGameEnd						();
	DEL_INSTANCE					(g_hud);   
}


void IGame_Persistent::PreStart		(LPCSTR op)
{
	string256						prev_type;
	params							new_game_params;
	xr_strcpy							(prev_type,m_game_params.m_game_type);
	new_game_params.parse_cmd_line	(op);

	// change game type
	if (0!=xr_strcmp(prev_type,new_game_params.m_game_type)){
		OnGameEnd					();
	}
}
void IGame_Persistent::Start		(LPCSTR op)
{
	string256						prev_type;
	xr_strcpy							(prev_type,m_game_params.m_game_type);
	m_game_params.parse_cmd_line	(op);
	// change game type
	if ((0!=xr_strcmp(prev_type,m_game_params.m_game_type))) 
	{
		if (*m_game_params.m_game_type)
			OnGameStart					();
		if(g_hud)
			DEL_INSTANCE			(g_hud);           
	}
	else UpdateGameType();

	VERIFY							(ps_destroy.empty());
}

void IGame_Persistent::Disconnect	()
{
	// clear "need to play" particles
	destroy_particles					(true);
	if(g_hud)
		DEL_INSTANCE			(g_hud);
}

void IGame_Persistent::OnGameStart()
{
	LoadTitle("st_prefetching_objects");
 	if(!strstr(Core.Params,"-noprefetch"))
		Prefetch();
}

void IGame_Persistent::Prefetch()
{
	// prefetch game objects & models
	float	p_time		=			1000.f*Device.GetTimerGlobal()->GetElapsed_sec();
	u32	mem_0			=			Memory.mem_usage()	;

	Log				("Loading objects...");
	ObjectPool.prefetch					();
	
	Log				("Loading models...");
	Render->models_Prefetch				();
	 
	Device.m_pRender->ResourcesDeferredUpload();

	p_time				=			1000.f*Device.GetTimerGlobal()->GetElapsed_sec() - p_time;
	u32		p_mem		=			Memory.mem_usage() - mem_0	;

	Msg					("* [prefetch] time:    %d ms",	iFloor(p_time));
	Msg					("* [prefetch] memory:  %dKb",	p_mem/1024);
}

void IGame_Persistent::OnGameEnd	()
{
	ObjectPool.clear					();
	Render->models_Clear				(TRUE);
}

void IGame_Persistent::OnFrame		()
{
	OPTICK_EVENT("IGame_Persistent::OnFrame");

  	if(!Device.Paused() || Device.dwPrecacheFrame)
		Environment().OnFrame	();


	Device.Statistic->Particles_starting= ps_needtoplay.size	();
	Device.Statistic->Particles_active	= ps_active.size		();
	Device.Statistic->Particles_destroy	= ps_destroy.size		();

	// Play req particle systems
	while (ps_needtoplay.size())
	{
		CPS_Instance*	psi		= ps_needtoplay.back	();
		ps_needtoplay.pop_back	();
		psi->Play				(false);
	}

	// Destroy inactive particle systems
	while (ps_destroy.size())
	{
		CPS_Instance*	psi		= ps_destroy.back();
		VERIFY					(psi);
		if (psi->Locked())
  			break;
 		ps_destroy.pop_back		();
		psi->PSI_internal_delete();
	}
}

void IGame_Persistent::destroy_particles		(const bool &all_particles)
{
	ps_needtoplay.clear				();

	while (ps_destroy.size())
	{
		CPS_Instance*	psi		= ps_destroy.back	();		
		VERIFY					(psi);
		VERIFY					(!psi->Locked());
		ps_destroy.pop_back		();
		psi->PSI_internal_delete();
	}

	// delete active particles
	if (all_particles) 
	{
		for (;!ps_active.empty();)
			(*ps_active.begin())->PSI_internal_delete	();
	}
	else
	{
		u32								active_size = ps_active.size();
		CPS_Instance					**I = (CPS_Instance**)_alloca(active_size*sizeof(CPS_Instance*));
		std::copy						(ps_active.begin(),ps_active.end(),I);

		struct destroy_on_game_load {
			static IC bool predicate (CPS_Instance*const& object)
			{
				return					(!object->destroy_on_game_load());
			}
		};

		CPS_Instance					**E = std::remove_if(I,I + active_size,&destroy_on_game_load::predicate);
		for ( ; I != E; ++I)
			(*I)->PSI_internal_delete	();
	}
	VERIFY								(ps_needtoplay.empty() && ps_destroy.empty() && (!all_particles || ps_active.empty()));
}

void IGame_Persistent::OnAssetsChanged()
{
	Device.m_pRender->OnAssetsChanged(); //Resources->m_textures_description.Load();   
}

void IGame_Persistent::GrassBendersUpdate(u16 id, u8& data_idx, u32& data_frame, Fvector& position, float init_radius, float init_str, bool CheckDistance)
{
	// Interactive grass disabled
	if (ps_ssfx_grass_interactive.y < 1)
		return;

	// Just update position if not NULL
	if (data_idx != 0)
	{
		// Explosions can take the mem spot, unassign and try to get a spot later.
		if (grass_shader_data.id[data_idx] != id)
		{
			data_idx = 0;
			data_frame = Device.dwFrame + Random.randI(10, 35);
		}
		else
		{
			grass_shader_data.pos[data_idx] = position;
		}
	}

	if (Device.dwFrame < data_frame)
		return;

	// Wait some random frames to split the checks
	data_frame = Device.dwFrame + Random.randI(10, 35);

	// Check Distance
	if (CheckDistance)
	{
		if (position.distance_to_xz_sqr(Device.vCameraPosition) > ps_ssfx_grass_interactive.z)
		{
			GrassBendersRemoveByIndex(data_idx);
			return;
		}
	}

	CFrustum& view_frust = ::Render->ViewBase;
	u32 mask = 0xff;
	float rad = data_idx == 0 ? 1.0 : std::max(1.0f, grass_shader_data.radius_curr[data_idx] + 0.5f);

	// In view frustum?
	if (!view_frust.testSphere(position, rad, mask))
	{
		GrassBendersRemoveByIndex(data_idx);
		return;
	}

	// Empty slot, let's use this
	if (data_idx == 0)
	{
		u8 idx = grass_shader_data.index + 1;

		// Add to grass blenders array
		if (grass_shader_data.id[idx] == 0)
		{
			data_idx = idx;
			GrassBendersSet(idx, id, position, Fvector3().set(0, -99, 0), 0, 0, 0.0f, init_radius, BENDER_ANIM_DEFAULT, true);

			grass_shader_data.str_target[idx] = init_str;
			grass_shader_data.radius_curr[idx] = init_radius;
		}
		// Back to 0 when the array limit is reached
		grass_shader_data.index = idx < ps_ssfx_grass_interactive.y ? idx : 0;
	}
	else
	{
		// Already in view, let's add more time to re-check
		data_frame += 60;
		grass_shader_data.pos[data_idx] = position;
	}
}

void IGame_Persistent::GrassBendersAddExplosion(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
	if (ps_ssfx_grass_interactive.y < 1)
		return;

	for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
	{
		// Add explosion to any spot not already taken by an explosion.
		if (grass_shader_data.anim[idx] != BENDER_ANIM_EXPLOSION)
		{
			// Add 99 to the ID to avoid conflicts between explosions and basic benders happening at the same time with the same ID.
			GrassBendersSet(idx, id + 99, position, dir, fade, speed, intensity, radius, BENDER_ANIM_EXPLOSION, true);
			grass_shader_data.str_target[idx] = intensity;
			break;
		}
	}
}

void IGame_Persistent::GrassBendersAddShot(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
	// Is disabled?
	if (ps_ssfx_grass_interactive.y < 1 || intensity <= 0.0f)
		return;

	// Check distance
	if (position.distance_to_xz_sqr(Device.vCameraPosition) > ps_ssfx_grass_interactive.z)
		return;

	int AddAt = -1;

	// Look for a spot
	for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
	{
		// Already exist, just update and increase intensity
		if (grass_shader_data.id[idx] == id)
		{
			float currentSTR = grass_shader_data.str[idx];
			GrassBendersSet(idx, id, position, dir, fade, speed, currentSTR, radius, BENDER_ANIM_EXPLOSION, false);
			grass_shader_data.str_target[idx] += intensity;
			AddAt = -1;
			break;
		}
		else
		{
			// Check all indexes and keep usable index to use later if needed...
			if (AddAt == -1 && fsimilar(grass_shader_data.radius[idx], 0.f))
				AddAt = idx;
		}
	}

	// We got an available index... Add bender at AddAt
	if (AddAt != -1)
	{
		GrassBendersSet(AddAt, id, position, dir, fade, speed, 0.001f, radius, BENDER_ANIM_EXPLOSION, true);
		grass_shader_data.str_target[AddAt] = intensity;
	}
}

bool IsMainMenuActive()
{
	return g_pGamePersistent && g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive();
} //ECO_RENDER add

void IGame_Persistent::GrassBendersUpdateAnimations()
{
	for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
	{
		if (grass_shader_data.id[idx] != 0)
		{
			switch (grass_shader_data.anim[idx])
			{
			case BENDER_ANIM_EXPLOSION: // Internal Only ( You can use BENDER_ANIM_PULSE for anomalies )
			{
				// Radius
				grass_shader_data.time[idx] += Device.fTimeDelta * grass_shader_data.speed[idx];
				grass_shader_data.radius_curr[idx] = grass_shader_data.radius[idx] * std::min(1.0f, grass_shader_data.time[idx]);

				grass_shader_data.str_target[idx] = std::min(1.0f, grass_shader_data.str_target[idx]);

				// Easing
				float diff = abs(grass_shader_data.str[idx] - grass_shader_data.str_target[idx]);
				diff = std::max(0.1f, diff);

				// Intensity
				if (grass_shader_data.str_target[idx] <= grass_shader_data.str[idx])
				{
					grass_shader_data.str[idx] -= Device.fTimeDelta * grass_shader_data.fade[idx] * diff;
				}
				else
				{
					grass_shader_data.str[idx] += Device.fTimeDelta * grass_shader_data.speed[idx] * diff;

					if (grass_shader_data.str[idx] >= grass_shader_data.str_target[idx])
						grass_shader_data.str_target[idx] = 0;
				}

				// Remove Bender
				if (grass_shader_data.str[idx] < 0.0f)
					GrassBendersReset(idx);
			}
			break;

			case BENDER_ANIM_WAVY:
			{
				// Anim Speed
				grass_shader_data.time[idx] += Device.fTimeDelta * 1.5f * grass_shader_data.speed[idx];

				// Curve
				float curve = sin(grass_shader_data.time[idx]);

				// Intensity using curve
				grass_shader_data.str[idx] = curve * cos(curve * 1.4f) * 1.8f * grass_shader_data.str_target[idx];
			}

			break;

			case BENDER_ANIM_SUCK:
			{
				// Anim Speed
				grass_shader_data.time[idx] += Device.fTimeDelta * grass_shader_data.speed[idx];

				// Perlin Noise
				float curve = clampr(PerlinNoise1D->GetContinious(grass_shader_data.time[idx]) + 0.5f, 0.f, 1.f) * -1.0;

				// Intensity using Perlin
				grass_shader_data.str[idx] = curve * grass_shader_data.str_target[idx];
			}
			break;

			case BENDER_ANIM_BLOW:
			{
				// Anim Speed
				grass_shader_data.time[idx] += Device.fTimeDelta * 1.2f * grass_shader_data.speed[idx];

				// Perlin Noise
				float curve = clampr(PerlinNoise1D->GetContinious(grass_shader_data.time[idx]) + 1.0f, 0.f, 2.0f) * 0.25f;

				// Intensity using Perlin
				grass_shader_data.str[idx] = curve * grass_shader_data.str_target[idx];
			}
			break;

			case BENDER_ANIM_PULSE:
			{
				// Anim Speed
				grass_shader_data.time[idx] += Device.fTimeDelta * grass_shader_data.speed[idx];

				// Radius
				grass_shader_data.radius_curr[idx] = grass_shader_data.radius[idx] * std::min(1.0f, grass_shader_data.time[idx]);

				// Diminish intensity when radius target is reached
				if (grass_shader_data.radius_curr[idx] >= grass_shader_data.radius[idx])
					grass_shader_data.str[idx] += GrassBenderToValue(grass_shader_data.str[idx], 0.0f, grass_shader_data.speed[idx] * 0.6f, true);

				// Loop when intensity is <= 0
				if (grass_shader_data.str[idx] <= 0.0f)
				{
					grass_shader_data.str[idx] = grass_shader_data.str_target[idx];
					grass_shader_data.radius_curr[idx] = 0.0f;
					grass_shader_data.time[idx] = 0.0f;
				}

			}
			break;

			case BENDER_ANIM_DEFAULT:

				// Just fade to target strength
				grass_shader_data.str[idx] += GrassBenderToValue(grass_shader_data.str[idx], grass_shader_data.str_target[idx], 2.0f, true);

				break;
			}
		}
	}
}

void IGame_Persistent::GrassBendersRemoveByIndex(u8& idx)
{
	if (idx != 0)
	{
		GrassBendersReset(idx);
		idx = 0;
	}
}

void IGame_Persistent::GrassBendersRemoveById(u16 id)
{
	// Search by Object ID ( Used when removing benders CPHMovementControl::DestroyCharacter() )
	for (int i = 1; i < ps_ssfx_grass_interactive.y + 1; i++)
		if (grass_shader_data.id[i] == id)
			GrassBendersReset(i);
}

void IGame_Persistent::GrassBendersReset(u8 idx)
{
	// Reset Everything
	GrassBendersSet(idx, 0, Fvector3().set(0, 0, 0), Fvector3().set(0, -99, 0), 0, 0, 0, 0, BENDER_ANIM_DEFAULT, true);
	grass_shader_data.str_target[idx] = 0;
}

void IGame_Persistent::GrassBendersSet(u8 idx, u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius, GrassBenders_Anim anim, bool resetTime)
{
	// Set values
	grass_shader_data.anim[idx] = anim;
	grass_shader_data.pos[idx] = position;
	grass_shader_data.id[idx] = id;
	grass_shader_data.radius[idx] = radius;
	grass_shader_data.str[idx] = intensity;
	grass_shader_data.fade[idx] = fade;
	grass_shader_data.speed[idx] = speed;
	grass_shader_data.dir[idx] = dir;

	if (resetTime)
	{
		grass_shader_data.radius_curr[idx] = 0.01f;
		grass_shader_data.time[idx] = 0;
	}
}

float IGame_Persistent::GrassBenderToValue(float& current, float go_to, float intensity, bool use_easing)
{
	float diff = abs(current - go_to);

	float r_value = Device.fTimeDelta * intensity * (use_easing ? std::min(0.5f, diff) : 1.0f);

	if (diff - r_value <= 0)
	{
		current = go_to;
		return 0;
	}

	return current < go_to ? r_value : -r_value;
}