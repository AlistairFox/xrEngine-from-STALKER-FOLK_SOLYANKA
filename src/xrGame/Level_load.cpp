#include "stdafx.h"
#include "LevelGameDef.h"
#include "ai_space.h"
#include "ParticlesObject.h"
#include "script_process.h"
#include "script_engine.h"
#include "script_engine_space.h"
#include "level.h"
#include "game_cl_base.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/gamemtllib.h"
#include "../xrphysics/PhysicsCommon.h"
#include "level_sounds.h"
#include "GamePersistent.h"

BOOL CLevel::Load_GameSpecific_Before()
{
	// AI space
	g_pGamePersistent->LoadTitle("st_loading_ai_objects");
  	if (xr_strcmp(m_game_description.spawn_name, "alife_off") == 0)
		return true;

	string_path current_level;
	FS.update_path(current_level, "$level$", "");
	Msg("Current Level: %s", current_level);

	string_path							fn_game;
 	if (OnClient()  && FS.exist(fn_game, "$level$", "alife.spawn"))
	{
		spawn = FS.r_open(fn_game);
	
		IReader* chunk;
	
		chunk = spawn->open_chunk(3);
		R_ASSERT2(chunk, "Spawn version mismatch - REBUILD SPAWN!");
		ai().patrol_path_storage(*chunk);
		chunk->close();
	
		m_chunk = spawn->open_chunk(4);
		R_ASSERT2(m_chunk, "Spawn version mismatch - REBUILD SPAWN!");
		ai().game_graph(xr_new<CGameGraph>(*m_chunk));
	}
	 

	// if (!ai().get_alife() && FS.exist(fn_game, "$level$", "level.ai") && net_SessionName())
	// {
	// 	ai().load(net_SessionName());
	// }
  	// 
	// if (! ai().get_alife() && FS.exist(fn_game, "$level$", "level.game"))
	// {
	// 	IReader* stream = FS.r_open(fn_game);
	// 	ai().patrol_path_storage_raw(*stream);
	// 	FS.r_close(stream);
	// }
 
	return								(TRUE);
}

BOOL CLevel::Load_GameSpecific_After()
{
	R_ASSERT(m_StaticParticles.empty());
	// loading static particles
	string_path		fn_game;
	if (FS.exist(fn_game, "$level$", "level.ps_static")) 
	{
		IReader *F = FS.r_open	(fn_game);
		CParticlesObject* pStaticParticles;
		u32				chunk = 0;
		string256		ref_name;
		Fmatrix			transform;
		Fvector			zero_vel={0.f,0.f,0.f};
		u32 ver			= 0;
		for (IReader *OBJ = F->open_chunk_iterator(chunk); OBJ; OBJ = F->open_chunk_iterator(chunk,OBJ)) 
		{
			if(chunk==0)
			{
				if(OBJ->length()==sizeof(u32))
				{
					ver		= OBJ->r_u32();
#ifndef MASTER_GOLD
					Msg		("PS new version, %d", ver);
#endif // #ifndef MASTER_GOLD
					continue;
				}
			}
			u16 gametype_usage				= 0;
			if(ver>0)
			{
				gametype_usage				= OBJ->r_u16();
			}
			OBJ->r_stringZ					(ref_name,sizeof(ref_name));
			OBJ->r							(&transform,sizeof(Fmatrix));transform.c.y+=0.01f;
			
						
			if ((g_pGamePersistent->m_game_params.m_e_game_type & EGameIDs(gametype_usage)) || (ver == 0))
			{
				pStaticParticles				= CParticlesObject::Create(ref_name,FALSE,false);
				pStaticParticles->UpdateParent	(transform,zero_vel);
				pStaticParticles->Play			(false);
				m_StaticParticles.push_back		(pStaticParticles);
			}
		}
		FS.r_close		(F);
	}
	
	if	(!g_dedicated_server)
	{
		// loading static sounds
		VERIFY								(m_level_sound_manager);
		m_level_sound_manager->Load			();

		// loading sound environment
		if ( FS.exist(fn_game, "$level$", "level.snd_env")) {
			IReader *F				= FS.r_open	(fn_game);
			::Sound->set_geometry_env(F);
			FS.r_close				(F);
		}
		// loading SOM
		if (FS.exist(fn_game, "$level$", "level.som")) {
			IReader *F				= FS.r_open	(fn_game);
			::Sound->set_geometry_som(F);
			FS.r_close				(F);
		}

		// loading random (around player) sounds
		if (pSettings->section_exist("sounds_random"))
		{ 
			CInifile::Sect& S		= pSettings->r_section("sounds_random");
			Sounds_Random.reserve	(S.Data.size());
			for (CInifile::SectCIt I=S.Data.begin(); S.Data.end()!=I; ++I) 
			{
				Sounds_Random.push_back	(ref_sound());
				Sound->create			(Sounds_Random.back(),*I->first,st_Effect,sg_SourceType);
			}
			Sounds_Random_dwNextTime= Device.TimerAsync	()	+ 50000;
			Sounds_Random_Enabled	= FALSE;
		}

		if ( FS.exist(fn_game, "$level$", "level.fog_vol")) 
		{
			IReader *F				= FS.r_open	(fn_game);
			u16 version				= F->r_u16();
			if(version == 2)
			{
				u32 cnt					= F->r_u32();

				Fmatrix					volume_matrix;
				for(u32 i=0; i<cnt; ++i)
				{
					F->r				(&volume_matrix, sizeof(volume_matrix));
					u32 sub_cnt			= F->r_u32();
					for(u32 is=0; is<sub_cnt; ++is)
					{
						F->r			(&volume_matrix, sizeof(volume_matrix));
					}

				}
			}
			FS.r_close				(F);
		}
	}	

	// loading scripts
	ai().script_engine().remove_script_process(ScriptEngine::eScriptProcessorLevel);

	if (pLevel->section_exist("level_scripts") && pLevel->line_exist("level_scripts","script"))
		ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorLevel,xr_new<CScriptProcess>("level",pLevel->r_string("level_scripts","script")));
	else
		ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorLevel,xr_new<CScriptProcess>("level",""));
		
	BlockCheatLoad();

	g_pGamePersistent->Environment().SetGameTime	(GetEnvironmentGameDayTimeSec(),game->GetEnvironmentGameTimeFactor());

	return TRUE;
}

struct translation_pair {
	u32			m_id;
	u16			m_index;

	IC			translation_pair	(u32 id, u16 index)
	{
		m_id	= id;
		m_index	= index;
	}

	IC	bool	operator==	(const u16 &id) const
	{
		return	(m_id == id);
	}

	IC	bool	operator<	(const translation_pair &pair) const
	{
		return	(m_id < pair.m_id);
	}

	IC	bool	operator<	(const u16 &id) const
	{
		return	(m_id < id);
	}
};

void CLevel::Load_GameSpecific_CFORM	( CDB::TRI* tris, u32 count )
{
	typedef xr_vector<translation_pair>	ID_INDEX_PAIRS;
	ID_INDEX_PAIRS						translator;
	translator.reserve					(GMLib.CountMaterial());
	u16									default_id = (u16)GMLib.GetMaterialIdx("default");
	translator.push_back				(translation_pair(u32(-1),default_id));

	u16									index = 0, static_mtl_count = 1;
	int max_ID							= 0;
	int max_static_ID					= 0;
	for (GameMtlIt I=GMLib.FirstMaterial(); GMLib.LastMaterial()!=I; ++I, ++index) {
		if (!(*I)->Flags.test(SGameMtl::flDynamic)) {
			++static_mtl_count;
			translator.push_back		(translation_pair((*I)->GetID(),index));
			if ((*I)->GetID()>max_static_ID)	max_static_ID	= (*I)->GetID(); 
		}
		if ((*I)->GetID()>max_ID)				max_ID			= (*I)->GetID(); 
	}
	// Msg("* Material remapping ID: [Max:%d, StaticMax:%d]",max_ID,max_static_ID);
	VERIFY(max_static_ID<0xFFFF);
	
	if (static_mtl_count < 128) {
		CDB::TRI						*I = tris;
		CDB::TRI						*E = tris + count;
		for ( ; I != E; ++I) {
			ID_INDEX_PAIRS::iterator	i = std::find(translator.begin(),translator.end(),(u16)(*I).material);
			if (i != translator.end()) {
				(*I).material			= (*i).m_index;
				SGameMtl* mtl			= GMLib.GetMaterialByIdx	((*i).m_index);
				(*I).suppress_shadows	= mtl->Flags.is(SGameMtl::flSuppressShadows);
				(*I).suppress_wm		= mtl->Flags.is(SGameMtl::flSuppressWallmarks);
				continue;
			}

			Debug.fatal					(DEBUG_INFO,"Game material '%d' not found",(*I).material);
		}
		return;
	}

	std::sort							(translator.begin(),translator.end());
	{
		CDB::TRI						*I = tris;
		CDB::TRI						*E = tris + count;
		for ( ; I != E; ++I) {
			ID_INDEX_PAIRS::iterator	i = std::lower_bound(translator.begin(),translator.end(),(u16)(*I).material);
			if ((i != translator.end()) && ((*i).m_id == (*I).material)) {
				(*I).material			= (*i).m_index;
				SGameMtl* mtl			= GMLib.GetMaterialByIdx	((*i).m_index);
				(*I).suppress_shadows	= mtl->Flags.is(SGameMtl::flSuppressShadows);
				(*I).suppress_wm		= mtl->Flags.is(SGameMtl::flSuppressWallmarks);
				continue;
			}

			Debug.fatal					(DEBUG_INFO,"Game material '%d' not found",(*I).material);
		}
	}
}

void CLevel::BlockCheatLoad()
{
#ifndef	DEBUG
	if( game && (GameID() != eGameIDSingle) ) phTimefactor=1.f;
#endif
}
