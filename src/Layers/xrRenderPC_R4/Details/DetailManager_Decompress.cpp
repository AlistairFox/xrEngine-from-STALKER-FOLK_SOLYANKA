#include "stdafx.h"
#pragma hdrstop

#include "DetailManager.h"
#include "../xrRender/cl_intersect.h"

//--------------------------------------------------- Decompression
IC float	Interpolate(float* base, u32 x, u32 y, u32 size)
{
	float	f = float(size);
	float	fx = float(x) / f; float ifx = 1.f - fx;
	float	fy = float(y) / f; float ify = 1.f - fy;

	float	c01 = base[0] * ifx + base[1] * fx;
	float	c23 = base[2] * ifx + base[3] * fx;

	float	c02 = base[0] * ify + base[2] * fy;
	float	c13 = base[1] * ify + base[3] * fy;

	float	cx = ify * c01 + fy * c23;
	float	cy = ifx * c02 + fx * c13;
	return	(cx + cy) / 2;
}

IC bool		InterpolateAndDither(float* alpha255, u32 x, u32 y, u32 sx, u32 sy, u32 size, int dither[16][16])
{
	clamp(x, (u32)0, size - 1);
	clamp(y, (u32)0, size - 1);
	int		c = iFloor(Interpolate(alpha255, x, y, size) + .5f);
	clamp(c, 0, 255);

	u32	row = (y + sy) % 16;
	u32	col = (x + sx) % 16;
	return	c > dither[col][row];
}

#include "../../xrEngine/GameMtlLib.h"

static void correction_orientation(const Fvector& pos, const Fvector& dir, const Fvector& ground_normal, float& target_angle)
{
	Fplane Plane_;
	Plane_.build(pos, ground_normal);

	Fvector				position_on_plane;
	Plane_.project(position_on_plane, pos);

	// находим проекцию точки, лежащей на векторе текущего направления
	Fvector				dir_point, proj_point;
	dir_point.mad(position_on_plane, dir, 1.f);
	Plane_.project(proj_point, dir_point);

	// получаем искомый вектор направления
	Fvector				target_dir;
	target_dir.sub(proj_point, position_on_plane);

	// изменяем текущий угол Эйлера
	target_angle = target_dir.getP();
}

static void ground_correction(Fmatrix& xform_, const Fvector& ground_normal)
{
	Fvector saved_pos = xform_.c;
	float h_, p_, b_;
	xform_.getHPB(h_, p_, b_);
	correction_orientation(xform_.c, xform_.k, ground_normal, p_);
	correction_orientation(xform_.c, xform_.i, ground_normal, b_);
	xform_.setHPB(h_, p_, -b_);
	xform_.c = saved_pos;
}
//#define		DBG_SWITCHOFF_RANDOMIZE
void		CDetailManager::cache_Decompress(Slot* S)
{
	VERIFY(S);
	Slot& D = *S;
	D.type = stReady;
	if (D.empty)		return;

	DetailSlot& DS = QueryDB(D.sx, D.sz);

	// Select polygons
	Fvector		bC, bD;
	D.vis.box.get_CD(bC, bD);

	xrc.box_options(CDB::OPT_FULL_TEST);
	xrc.box_query(g_pGameLevel->ObjectSpace.GetStaticModel(), bC, bD);
	u32	triCount = xrc.r_count();
	CDB::TRI* tris = g_pGameLevel->ObjectSpace.GetStaticTris();
	Fvector* verts = g_pGameLevel->ObjectSpace.GetStaticVerts();

	if (0 == triCount)	return;

	// Build shading table
	float		alpha255[dm_obj_in_slot][4];
	for (int i = 0; i < dm_obj_in_slot; i++)
	{
		alpha255[i][0] = 255.f * float(DS.palette[i].a0) / 15.f;
		alpha255[i][1] = 255.f * float(DS.palette[i].a1) / 15.f;
		alpha255[i][2] = 255.f * float(DS.palette[i].a2) / 15.f;
		alpha255[i][3] = 255.f * float(DS.palette[i].a3) / 15.f;
	}

	// Prepare to selection
	float		density = ps_r__Detail_density;
	float		jitter = density / 1.7f;
	u32			d_size = iCeil(dm_slot_size / density);
	svector<int, dm_obj_in_slot>		selected;

	u32 p_rnd = D.sx * D.sz; // нужно для того чтобы убрать полосы(ряды)
	CRandom				r_selection(0x12071980 ^ p_rnd);
	CRandom				r_jitter(0x12071980 ^ p_rnd);
	CRandom				r_yaw(0x12071980 ^ p_rnd);
	CRandom				r_scale(0x12071980 ^ p_rnd);

	// Prepare to actual-bounds-calculations
	Fbox				Bounds;
	Bounds.invalidate();

	// Decompressing itself
	for (u32 z = 0; z <= d_size; z++)
	{
		for (u32 x = 0; x <= d_size; x++)
		{
			// shift
			u32 shift_x = r_jitter.randI(16);
			u32 shift_z = r_jitter.randI(16);

			// Iterpolate and dither palette
			selected.clear();


			if ((DS.id0 != DetailSlot::ID_Empty) && InterpolateAndDither(alpha255[0], x, z, shift_x, shift_z, d_size, dither))	selected.push_back(0);
			if ((DS.id1 != DetailSlot::ID_Empty) && InterpolateAndDither(alpha255[1], x, z, shift_x, shift_z, d_size, dither))	selected.push_back(1);
			if ((DS.id2 != DetailSlot::ID_Empty) && InterpolateAndDither(alpha255[2], x, z, shift_x, shift_z, d_size, dither))	selected.push_back(2);
			if ((DS.id3 != DetailSlot::ID_Empty) && InterpolateAndDither(alpha255[3], x, z, shift_x, shift_z, d_size, dither))	selected.push_back(3);

			// Select
			if (selected.empty())	continue;

			u32 index;
			if (selected.size() == 1)
				index = selected[0];
			else			
				index = selected[r_selection.randI(selected.size())];
 
			const CDetail& Dobj = objects[DS.r_id(index)];
			CDetail::SlotItem	Item = CDetail::SlotItem();
			// Position (XZ)
			float		rx = (float(x) / float(d_size)) * dm_slot_size + D.vis.box.min.x;
			float		rz = (float(z) / float(d_size)) * dm_slot_size + D.vis.box.min.z;
			
			Fvector		Item_P;
			Item_P.set(rx + r_jitter.randFs(jitter), D.vis.box.max.y, rz + r_jitter.randFs(jitter));

			// Position (Y)
			float y = D.vis.box.min.y - 5;
			Fvector	dir; dir.set(0, -1, 0);
			Fvector normal; normal.set(0, 1, 0);
			float		r_u, r_v, r_range;
			bool no_push = false;
			for (u32 tid = 0; tid < triCount; tid++)
			{
				CDB::TRI& T = tris[xrc.r_begin()[tid].id];
				SGameMtl* mtl = GMLib.GetMaterialByIdx(T.material);

				if (mtl->Flags.test(SGameMtl::flPassable))
					continue;

				//Detect sector
				// CSector* sector = (CSector*)RImplementation.getSector(T.sector);
				// if (sector != RImplementation.pOutdoorSector)
				// {
				// 	no_push = true;
				// 	continue;
				// }

				Fvector		Tv[3] = { verts[T.verts[0]],verts[T.verts[1]],verts[T.verts[2]] };
				if (CDB::TestRayTri(Item_P, dir, Tv, r_u, r_v, r_range, TRUE))
				{
					if (r_range >= 0) {
						float y_test = Item_P.y - r_range;
						if (y_test > y)	y = y_test;
					}
					normal.mknormal(verts[T.verts[0]], verts[T.verts[1]], verts[T.verts[2]]);
				}
			}

			if (no_push)
				continue;
			if (y < D.vis.box.min.y)		
				continue;
			Item_P.y = y;

			// Angles and scale
			Item.scale = r_scale.randF(Dobj.m_fMinScale * 0.5f, Dobj.m_fMaxScale * 0.9f);

			// X-Form BBox
			Fmatrix		mScale, mXform, mRotY;
			Fbox		ItemBB;



			mRotY.rotateY(r_yaw.randF(0, PI_MUL_2));

			mRotY.translate_over(Item_P);
			mScale.scale(Item.scale, Item.scale, Item.scale);
			mXform.mul_43(mRotY, mScale);
			ItemBB.xform(Dobj.bv_bb, mXform);
			Bounds.merge(ItemBB);

			Item.c_hemi = DS.r_qclr(DS.c_hemi, 15);

			// Vis-sorting
			if (Dobj.m_Flags.is(DO_NO_WAVING))
				Item.vis_ID = 0;
			else
				Item.vis_ID = Random.randI(1, 3);

			//чтобы (только) листики травы ложились на поверхность террейна
			ground_correction(mRotY, normal);

			// se7kills (v2v3v4) подсказал сделать матрицу в HPB
			mRotY.getHPB(Item.Direction);
			Item.Position = mRotY.c;

			// Save it
			D.G[index].items.emplace_back(std::make_shared<CDetail::SlotItem>(Item));
		}
	}

	// Update bounds to more tight and real ones
	D.vis.clear();
	D.vis.box.set(Bounds);
	D.vis.box.getsphere(D.vis.sphere.P, D.vis.sphere.R);
}
