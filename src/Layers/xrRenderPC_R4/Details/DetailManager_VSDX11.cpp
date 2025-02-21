#include "stdafx.h"
#include "DetailManager.h"

#include "../../xrEngine/IGame_Persistent.h"
#include "../../xrEngine/Environment.h"

#include "../xrRenderDX10/dx10BufferUtils.h"



const int quant = 16384;

short QC(float v)
{
	int t = iFloor(v * float(quant)); clamp(t, -32768, 32767);
	return short(t & 0xffff);
}

static D3DVERTEXELEMENT9 dwDecl[] =
{
	// 2й параметр оффсет в байтах
	{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos
	{ 0, 12, D3DDECLTYPE_SHORT4,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },	// uvt mid

	D3DDECL_END()
};

#pragma pack(push,1)
struct	vertHW
{
	float		x, y, z;
	short		u, v, t;
	short			mid;
};
#pragma pack(pop)

struct ButhingData
{
	float H, P, B, scale;
	float X, Y, Z, hemi;
};



void CDetailManager::hw_Load()
{
	hw_Load_Geom();
}


void CDetailManager::hw_Load_Geom()
{
	// Analyze batch-size
#define USED_TYPE u32

	Msg("CDetail VSDX11: Buthes: %u", hw_BatchSize);

	if (hw_BatchSize < 64)
		hw_BatchSize = 64;

	if (hw_BatchSize > 2000)
		hw_BatchSize = 2000;

	// Pre-process objects
	u32			dwVerts = 0;
	u32			dwIndices = 0;
	for (u32 o = 0; o < objects.size(); o++)
	{
		const CDetail& D = objects[o];
		dwVerts += D.number_vertices * hw_BatchSize;
		dwIndices += D.number_indices * hw_BatchSize;
	}

	// По всей видимости юзается 16 бит 

	u32			vSize = sizeof(vertHW);
	Msg("* [DETAILS] verticies: %u dwIndices: %u, sizeof vertHW(%U)", dwVerts, dwIndices / 3, vSize);
	Msg("* [DETAILS] Batch(%d), VB(%dK), IB(%dK)", hw_BatchSize, (dwVerts * vSize) / 1024, (dwIndices * sizeof(USED_TYPE)) / 1024);


	// Fill VB
	{
		vertHW* pV{};
		vertHW* pVOriginal;
		pVOriginal = xr_alloc<vertHW>(dwVerts);
		pV = pVOriginal;

		for (u32 o = 0; o < objects.size(); o++)
		{
			const CDetail& D = objects[o];
			for (u32 batch = 0; batch < hw_BatchSize; batch++)
			{
				u32 mid = batch; // * c_size

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
		xr_free(pVOriginal);
	}

	// Fill IB
	{
		u32 value;

		int& i = ((int&)value);

		USED_TYPE* pI{};
		USED_TYPE* pIOriginal;
		pIOriginal = xr_alloc<USED_TYPE>(dwIndices);
		pI = pIOriginal;

		for (u32 o = 0; o < objects.size(); o++)
		{
			const CDetail& D = objects[o];
			USED_TYPE		offset = 0;

			for (u32 batch = 0; batch < hw_BatchSize; batch++)
			{
				for (u32 i = 0; i < u32(D.number_indices); i++)
					*pI++ = USED_TYPE(USED_TYPE(D.indices[i]) + USED_TYPE(offset));
				offset = USED_TYPE(offset + USED_TYPE(D.number_vertices));
			}
		}
		R_CHK(dx10BufferUtils::CreateIndexBuffer(&hw_IB, pIOriginal, dwIndices * sizeof(USED_TYPE)));
		xr_free(pIOriginal);
	}

	// Declare geometry
	hw_Geom.create(dwDecl, hw_VB, hw_IB);

}

void CDetailManager::hw_Unload()
{
	hw_Geom.destroy();
	_RELEASE(hw_IB);
	_RELEASE(hw_VB);
}


extern float PlayerDetailsResize;
extern int UseInstanceing = 0;

void CDetailManager::hw_Render(light* L)
{
	if (0 == dtFS)				
		return;

	Device.Statistic->RenderDUMP_DT_Render.Begin();

	RCache.set_CullMode(CULL_NONE);
	RCache.set_xform_world(Fidentity);
	RCache.set_Geometry_32BIT(hw_Geom);

	float scale = 1.f / float(quant);
	Fvector4 wave, wave_old, consts;

	// Wave0
	wave.set(1.f / 5.f, 1.f / 7.f, 1.f / 3.f, m_time_pos);
	wave_old.set(1.f / 5.f, 1.f / 7.f, 1.f / 3.f, m_time_pos_old);
	consts.set(scale, scale, ps_r__Detail_l_aniso, ps_r__Detail_l_ambient);
	hw_Render_Buthing(consts, wave.div(PI_MUL_2), wave_dir1, 1, SE_R2_NORMAL_HQ, L); //  wave_old.div(PI_MUL_2), wave_dir1_old,

	// Wave1
	wave.set(1.f / 3.f, 1.f / 7.f, 1.f / 5.f, m_time_pos);
	wave_old.set(1.f / 3.f, 1.f / 7.f, 1.f / 5.f, m_time_pos_old);
	hw_Render_Buthing(consts, wave.div(PI_MUL_2), wave_dir2, 2, SE_R2_NORMAL_HQ, L); // wave_old.div(PI_MUL_2), wave_dir2_old, 

	// Still
	consts.set(scale, scale, scale, 1.f);
	hw_Render_Buthing(consts, wave.div(PI_MUL_2), wave_dir2, 0, SE_R2_NORMAL_LQ, L); //  wave_old.div(PI_MUL_2), wave_dir2_old,

	RCache.set_CullMode(CULL_CCW);

	Device.Statistic->RenderDUMP_DT_Render.End();
}



extern float PlayerDetailsResize;


void CDetailManager::hw_Render_Buthing(
	const Fvector4& consts,
	const Fvector4& wave,
	const Fvector4& wind,
	u32 var_id, u32 lod_id, light* L)
{

	if (RImplementation.phase == CRender::PHASE_SMAP && var_id == 0)
		return;

	static shared_str strConsts("consts");
	static shared_str strWave("wave");
	static shared_str strDir2D("dir2D");
	static shared_str strArray("array");


	// Matrices and offsets
	u32 vOffset = 0;
	u32 iOffset = 0;

	// Iterate
	for (CDetail& Object : objects)
	{
		for (u32 iPass = 0; iPass < Object.shader->E[lod_id]->passes.size(); ++iPass)
		{
			// Setup matrices + colors (and flush it as necessary)
			RCache.set_Element(Object.shader->E[lod_id], iPass);

			RImplementation.apply_lmaterial();

			//	This could be cached in the corresponding consatant buffer
			//	as it is done for DX9
			RCache.set_c(strConsts, consts);
			RCache.set_c(strWave, wave);
			RCache.set_c(strDir2D, wind);

			u32 dwBatch = 0;
			for (auto& S : Object.m_items[var_id][render_key])
			{
				CDetail::SlotItem& Instance = *S.get();
				if (RImplementation.phase == CRender::PHASE_SMAP && L)
				{
					if (L->position.distance_to_sqr(Instance.Position) >= _sqr(L->range))
						continue;
				}

				static ButhingData* b_data = NULL;

				if (dwBatch == 0)
				{
					RCache.get_ConstantDirect(strArray, hw_BatchSize * sizeof(ButhingData), (void**)&b_data, 0, 0);
				}

				if (!b_data)
				{
					Msg("! Не найден Шейдерный буфер: {'array'} (трава сломана) ");
					continue;
				}
				float scale = Instance.RenderScale * PlayerDetailsResize;
				b_data[dwBatch] =
				{
					Instance.Direction.x,  Instance.Direction.y, Instance.Direction.z, scale,
					Instance.Position.x, Instance.Position.y, Instance.Position.z, Instance.c_hemi
				};
				 
				dwBatch++;

				if (dwBatch >= hw_BatchSize)
				{
					// flush
					u32 dwCNT_verts = dwBatch * Object.number_vertices;
					u32 dwCNT_prims = (dwBatch * Object.number_indices) / 3;
					RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, dwCNT_verts, iOffset, dwCNT_prims);
					dwBatch = 0;
				}

			}
			// flush if nessecary
			if (dwBatch > 0 && dwBatch < hw_BatchSize)
			{
				u32 dwCNT_verts = dwBatch * Object.number_vertices;
				u32 dwCNT_prims = (dwBatch * Object.number_indices) / 3;
				RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, dwCNT_verts, iOffset, dwCNT_prims);
				dwBatch = 0;
			}
		}
		vOffset += hw_BatchSize * Object.number_vertices;
		iOffset += hw_BatchSize * Object.number_indices;

	}
} 