#include "stdafx.h"
#include "../xrRender/DetailManager.h"

#include "../../xrEngine/igame_persistent.h"
#include "../../xrEngine/environment.h"

#include "../xrRenderDX10/dx10BufferUtils.h"

const int quant = 16384;

static D3DVERTEXELEMENT9 dwDecl[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, // pos
	{0, 12, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // uv
	D3DDECL_END()
};

#pragma pack(push,1)
struct vertHW
{
	float x, y, z;
	short u, v, t, mid;
};
#pragma pack(pop)

short QC(float v)
{
	int t = iFloor(v * float(quant));
	clamp(t, -32768, 32767);
	return short(t & 0xffff);
}


void CDetailManager::hw_Load()
{
	hw_Load_Geom();
	hw_Load_Shaders();
}


void CDetailManager::hw_Load_Geom()
{
	// Analyze batch-size
	hw_BatchSize = 1000;
	Msg("* [DETAILS] VertexConsts(%d), Batch(%d)", u32(HW.Caps.geometry.dwRegisters), hw_BatchSize);

	// Pre-process objects
	u32 dwVerts = 0;
	u32 dwIndices = 0;
	for (u32 o = 0; o < objects.size(); o++)
	{
		const CDetail& D = *objects[o];
		dwVerts += D.number_vertices * hw_BatchSize;
		dwIndices += D.number_indices * hw_BatchSize;
	}
	u32 vSize = sizeof(vertHW);
	Msg("* [DETAILS] %d v(%d), %d p", dwVerts, vSize, dwIndices / 3);

	Msg("* [DETAILS] Batch(%d), VB(%dK), IB(%dK)", hw_BatchSize, (dwVerts * vSize) / 1024, (dwIndices * 2) / 1024);

	// Fill VB
	{
		vertHW* pV;
		vertHW* pVOriginal;
		pVOriginal = xr_alloc<vertHW>(dwVerts);
		pV = pVOriginal;
		for (o = 0; o < objects.size(); o++)
		{
			const CDetail& D = *objects[o];
			for (u32 batch = 0; batch < hw_BatchSize; batch++)
			{
				u32 mid = batch;
				for (u32 v = 0; v < D.number_vertices; v++)
				{
					const Fvector& vP = D.vertices[v].P;
					pV->x = vP.x;
					pV->y = vP.y;
					pV->z = vP.z;
					pV->u = QC(D.vertices[v].u);
					pV->v = QC(D.vertices[v].v);
					pV->t = QC(vP.y / (D.bv_bb.max.y - D.bv_bb.min.y));
					pV->mid = short(mid);
					pV++;
				}
			}
		}
		R_CHK(dx10BufferUtils::CreateVertexBuffer(&hw_VB, pVOriginal, dwVerts * vSize));
		HW.stats_manager.increment_stats_vb(hw_VB);
		xr_free(pVOriginal);
	}

	// Fill IB
	{
		u32* pI;
		u32* pIOriginal;
		pIOriginal = xr_alloc<u32>(dwIndices);
		pI = pIOriginal;
		for (o = 0; o < objects.size(); o++)
		{
			const CDetail& D = *objects[o];
			u32 offset = 0;
			for (u32 batch = 0; batch < hw_BatchSize; batch++)
			{
				for (u32 i = 0; i < u32(D.number_indices); i++)
					*pI++ = u32(u32(D.indices[i]) + u32(offset));
				offset = u32(offset + u32(D.number_vertices));
			}
		}
		R_CHK(dx10BufferUtils::CreateIndexBuffer(&hw_IB, pIOriginal, dwIndices * sizeof(u32)));
		HW.stats_manager.increment_stats_ib(hw_IB);
		xr_free(pIOriginal);
	}

	// Declare geometry
	hw_Geom.create(dwDecl, hw_VB, hw_IB);
}

void CDetailManager::hw_Unload()
{
	// Destroy VS/VB/IB
	hw_Geom.destroy();
	HW.stats_manager.decrement_stats_vb(hw_VB);
	HW.stats_manager.decrement_stats_ib(hw_IB);
	_RELEASE(hw_IB);
	_RELEASE(hw_VB);
}



void CDetailManager::hw_Load_Shaders()
{
	// Create shader to access constant storage
	ref_shader S;
	S.create("details\\set");
	R_constant_table& T0 = *(S->E[0]->passes[0]->constants);
	R_constant_table& T1 = *(S->E[1]->passes[0]->constants);
	hwc_consts = T0.get("consts");
	hwc_wave = T0.get("wave");
	hwc_wind = T0.get("dir2D");
	hwc_array = T0.get("array");
	hwc_s_consts = T1.get("consts");
	hwc_s_xform = T1.get("xform");
	hwc_s_array = T1.get("array");
}

void CDetailManager::hw_Render()
{
	// Render-prepare
	//	Update timer
	//	Can't use Device.fTimeDelta since it is smoothed! Don't know why, but smoothed value looks more choppy!
	float fDelta = Device.fTimeGlobal - m_global_time_old;
	if ((fDelta < 0) || (fDelta > 1)) fDelta = 0.03;
	m_global_time_old = Device.fTimeGlobal;

	m_time_rot_1 += (PI_MUL_2 * fDelta / swing_current.rot1);
	m_time_rot_2 += (PI_MUL_2 * fDelta / swing_current.rot2);
	m_time_pos += fDelta * swing_current.speed;

	//float		tm_rot1		= (PI_MUL_2*Device.fTimeGlobal/swing_current.rot1);
	//float		tm_rot2		= (PI_MUL_2*Device.fTimeGlobal/swing_current.rot2);
	float tm_rot1 = m_time_rot_1;
	float tm_rot2 = m_time_rot_2;

	Fvector4 dir1, dir2;
	dir1.set(_sin(tm_rot1), 0, _cos(tm_rot1), 0).normalize().mul(swing_current.amp1);
	dir2.set(_sin(tm_rot2), 0, _cos(tm_rot2), 0).normalize().mul(swing_current.amp2);

	// Setup geometry and DMA
	RCache.set_Geometry32(hw_Geom);

	// Wave0
	float scale = 1.f / float(quant);
	Fvector4 wave;
	Fvector4 consts;
	consts.set(scale, scale, ps_r__Detail_l_aniso, ps_r__Detail_l_ambient);
	//wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	Device.fTimeGlobal*swing_current.speed);
	wave.set(1.f / 5.f, 1.f / 7.f, 1.f / 3.f, m_time_pos);
	//RCache.set_c			(&*hwc_consts,	scale,		scale,		ps_r__Detail_l_aniso,	ps_r__Detail_l_ambient);				// consts
	//RCache.set_c			(&*hwc_wave,	wave.div(PI_MUL_2));	// wave
	//RCache.set_c			(&*hwc_wind,	dir1);																					// wind-dir
	//hw_Render_dump			(&*hwc_array,	1, 0, c_hdr );
	hw_Render_dump(consts, wave.div(PI_MUL_2), dir1, 1, 0);

	// Wave1
	//wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	Device.fTimeGlobal*swing_current.speed);
	wave.set(1.f / 3.f, 1.f / 7.f, 1.f / 5.f, m_time_pos);
	//RCache.set_c			(&*hwc_wave,	wave.div(PI_MUL_2));	// wave
	//RCache.set_c			(&*hwc_wind,	dir2);																					// wind-dir
	//hw_Render_dump			(&*hwc_array,	2, 0, c_hdr );
	hw_Render_dump(consts, wave.div(PI_MUL_2), dir2, 2, 0);

	// Still
	consts.set(scale, scale, scale, 1.f);
	//RCache.set_c			(&*hwc_s_consts,scale,		scale,		scale,				1.f);
	//RCache.set_c			(&*hwc_s_xform,	Device.mFullTransform);
	//hw_Render_dump			(&*hwc_s_array,	0, 1, c_hdr );
	hw_Render_dump(consts, wave.div(PI_MUL_2), dir2, 0, 1);
}

void CDetailManager::hw_Render_dump(const Fvector4& consts, const Fvector4& wave, const Fvector4& wind, u32 var_id,
	u32 lod_id)
{
	static shared_str strConsts("consts");
	static shared_str strWave("wave");
	static shared_str strDir2D("dir2D");
	static shared_str strArray("array");
	static shared_str strXForm("xform");

	static shared_str strPos("benders_pos");
	static shared_str strGrassSetup("benders_setup");
	
			// Grass benders data
		IGame_Persistent::grass_data & GData = g_pGamePersistent->grass_shader_data;
	Fvector4 player_pos = { 0, 0, 0, 0 };
	int BendersQty = _min(16, (int)(ps_ssfx_grass_interactive.y + 1));
	
			// Add Player?
		if (ps_ssfx_grass_interactive.x > 0)
		 player_pos.set(Device.vCameraPosition.x, Device.vCameraPosition.y, Device.vCameraPosition.z, -1);


	Device.Statistic->RenderDUMP_DT_Count = 0;

	// Matrices and offsets
	u32 vOffset = 0;
	u32 iOffset = 0;

	vis_list& list = m_visibles[var_id];

	CEnvDescriptor& desc = *g_pGamePersistent->Environment().CurrentEnv;
	Fvector c_sun, c_ambient, c_hemi;
	c_sun.set(desc.sun_color.x, desc.sun_color.y, desc.sun_color.z);
	c_sun.mul(.5f);
	c_ambient.set(desc.ambient.x, desc.ambient.y, desc.ambient.z);
	c_hemi.set(desc.hemi_color.x, desc.hemi_color.y, desc.hemi_color.z);

	// Iterate
	for (u32 O = 0; O < objects.size(); O++)
	{
		CDetail& Object = *objects[O];
		xr_vector<SlotItemVec*>& vis = list[O];
		if (!vis.empty())
		{
			for (u32 iPass = 0; iPass < Object.shader->E[lod_id]->passes.size(); ++iPass)
			{
				// Setup matrices + colors (and flush it as necessary)
				//RCache.set_Element				(Object.shader->E[lod_id]);
				RCache.set_Element(Object.shader->E[lod_id], iPass);
				RImplementation.apply_lmaterial();

				//	This could be cached in the corresponding consatant buffer
				//	as it is done for DX9
				RCache.set_c(strConsts, consts);
				RCache.set_c(strWave, wave);
				RCache.set_c(strDir2D, wind);
				RCache.set_c(strXForm, Device.mFullTransform);

				if (ps_ssfx_grass_interactive.y > 0)
					 {
					RCache.set_c(strGrassSetup, ps_ssfx_int_grass_params_1);
					
						Fvector4 * c_grass;
					{
						void* GrassData;
						RCache.get_ConstantDirect(strPos, BendersQty * sizeof(Fvector4), &GrassData, 0, 0);
						c_grass = (Fvector4*)GrassData;
						}
					
						if (c_grass)
						{
						c_grass[0].set(player_pos);
						c_grass[16].set(0.0f, -99.0f, 0.0f, 1.0f);
						
							for (int Bend = 1; Bend < BendersQty; Bend++)
							 {
							c_grass[Bend].set(GData.pos[Bend].x, GData.pos[Bend].y, GData.pos[Bend].z, GData.radius_curr[Bend]);
							c_grass[Bend + 16].set(GData.dir[Bend].x, GData.dir[Bend].y, GData.dir[Bend].z, GData.str[Bend]);
							}
						 }
					 }

				u32 dwBatch = 0;
				static Fmatrix* c_storage = NULL;
				RCache.get_ConstantDirect(strArray, hw_BatchSize * sizeof(Fmatrix), (void**)&c_storage, 0, 0);
				

				xr_vector<SlotItemVec*>::iterator _vI = vis.begin();
				xr_vector<SlotItemVec*>::iterator _vE = vis.end();
				for (; _vI != _vE; _vI++)
				{
					SlotItemVec* items = *_vI;
					SlotItemVecIt _iI = items->begin();
					SlotItemVecIt _iE = items->end();
					for (; _iI != _iE; _iI++)
					{

						if (!c_storage)
							continue;

						SlotItem& Instance = **_iI;

						// Build matrix ( 3x4 matrix, last row - color )
						float scale = Instance.scale_calculated;

						// Sort of fade using the scale
												// fade_distance == -1 use light_position to define "fade", anything else uses fade_distance
							if (fade_distance <= -1)
							 scale *= 1.0f - Instance.position.distance_to_xz_sqr(light_position) * 0.005f;
						else if (Instance.distance > fade_distance)
							 scale *= 1.0f - abs(Instance.distance - fade_distance) * 0.005f;
						
							if (scale <= 0)
							 break;
							float s = Instance.c_sun;
							float h = Instance.c_hemi;

		 				Fmatrix& M = Instance.mRotY;
						c_storage[dwBatch] =  { M._11 * scale, M._21 * scale, M._31 * scale, M._41,
						 					    M._12* scale, M._22* scale, M._32* scale, M._42,
						 					    M._13* scale, M._23* scale, M._33* scale, M._43,
						 					    s, s, s, h};

						dwBatch++;
						if (dwBatch == hw_BatchSize)
						{
							// flush
							Device.Statistic->RenderDUMP_DT_Count += dwBatch;
							u32 dwCNT_verts = dwBatch * Object.number_vertices;
							u32 dwCNT_prims = (dwBatch * Object.number_indices) / 3;

							RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, dwCNT_verts, iOffset, dwCNT_prims);
							RCache.stat.r.s_details.add(dwCNT_verts);

							// restart
							dwBatch = 0;

							//	Remap constants to memory directly (just in case anything goes wrong)
							{
								void* pVData;
								RCache.get_ConstantDirect(strArray,
									hw_BatchSize * sizeof(Fmatrix),
									&pVData, 0, 0);
								c_storage = (Fmatrix*)pVData;
							}
							VERIFY(c_storage);
						}
					}
				}
				// flush if nessecary
				if (dwBatch)
				{
					// TODO: add phase to RImplementation
					if (ps_ssfx_grass_shadows.x <= 0)
					{
						//auto& dsgraph = RImplementation.get_context(CHW::IMM_CTX_ID);
						if (!ps_r2_ls_flags.test(R2FLAG_SUN_DETAILS) ||
							((ps_r2_ls_flags.test(R2FLAG_SUN_DETAILS) && (RImplementation.PHASE_SMAP == RImplementation.phase)) ||
								(ps_r2_ls_flags.test(R2FLAG_SUN_DETAILS) && (RImplementation.PHASE_NORMAL == RImplementation.phase) && (!RImplementation.o_sun)) ||
								(!ps_r2_ls_flags.test(R2FLAG_SUN_DETAILS) && (RImplementation.PHASE_NORMAL == RImplementation.phase))))
						{
							vis.erase(vis.begin(), vis.end());
						}
					}

					Device.Statistic->RenderDUMP_DT_Count += dwBatch;
					u32 dwCNT_verts = dwBatch * Object.number_vertices;
					u32 dwCNT_prims = (dwBatch * Object.number_indices) / 3;
					//RCache.get_ConstantCache_Vertex().b_dirty				=	TRUE;
					//RCache.get_ConstantCache_Vertex().get_array_f().dirty	(c_base,c_base+dwBatch*4);
					RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, dwCNT_verts, iOffset, dwCNT_prims);
					RCache.stat.r.s_details.add(dwCNT_verts);
				}
			}
			// Clean up
			// KD: we must not clear vis on r2 since we want details shadows
			if (ps_ssfx_grass_shadows.x <= 0)
				 {
				if (!psDeviceFlags2.test(rsGrassShadow) || ((ps_r2_ls_flags.test(R2FLAG_SUN_DETAILS) && (RImplementation.PHASE_SMAP ==
				RImplementation.phase)) // phase smap with shadows
					|| (ps_r2_ls_flags.test(R2FLAG_SUN_DETAILS) && (RImplementation.PHASE_NORMAL == RImplementation.phase)
						 && (!RImplementation.is_sun())) // phase normal with shadows without sun
					 || (!ps_r2_ls_flags.test(R2FLAG_SUN_DETAILS) && (RImplementation.PHASE_NORMAL == RImplementation.phase))
					 )) // phase normal without shadows
					 vis.clear_not_free();
				
			}
		}
		vOffset += hw_BatchSize * Object.number_vertices;
		iOffset += hw_BatchSize * Object.number_indices;
	}
}
