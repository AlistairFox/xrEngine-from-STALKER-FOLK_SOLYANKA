#include "stdafx.h"

#include "fhierrarhyvisual.h"
#include "SkeletonCustom.h"
#include "../../xrEngine/fmesh.h"
#include "../../xrEngine/irenderable.h"

#include "flod.h"
#include "particlegroup.h"
#include "FTreeVisual.h"

using	namespace R_dsgraph;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Scene graph actual insertion and sorting ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
float		r_ssaDISCARD;
float		r_ssaDONTSORT;
float		r_ssaLOD_A,			r_ssaLOD_B;
float		r_ssaGLOD_start,	r_ssaGLOD_end;
float		r_ssaHZBvsTEX;

ICF	float	CalcSSA				(float& distSQ, Fvector& C, dxRender_Visual* V)
{
	float R	= V->vis.sphere.R + 0;
	distSQ	= Device.vCameraPosition.distance_to_sqr(C)+EPS;
	return	R/distSQ;
}
ICF	float	CalcSSA				(float& distSQ, Fvector& C, float R)
{
	distSQ	= Device.vCameraPosition.distance_to_sqr(C)+EPS;
	return	R/distSQ;
}

ICF    float    CalcHudSSA(float& distSQ, Fvector& C, dxRender_Visual* V)
{
	float R = V->vis.sphere.R + 0;
	distSQ = Fvector().set(0.f, 0.f, 0.f).distance_to_sqr(C) + EPS;
	return    R / distSQ;
}

void R_dsgraph_structure::r_dsgraph_insert_dynamic	(dxRender_Visual *pVisual, Fvector& Center)
{
	CRender&	RI			=	RImplementation;

	if (pVisual->vis.marker	==	RI.marker)	return	;
	pVisual->vis.marker		=	RI.marker			;

	float distSQ			;
	float SSA;

	if (!RI.val_bHUD)
		SSA = CalcSSA(distSQ, Center, pVisual);
	else
		SSA = CalcHudSSA(distSQ, Center, pVisual);

	if (SSA<=r_ssaDISCARD)		return;

	// Distortive geometry should be marked and R2 special-cases it
	// a) Allow to optimize RT order
	// b) Should be rendered to special distort buffer in another pass
	VERIFY						(pVisual->shader._get());
	ShaderElement*		sh_d	= &*pVisual->shader->E[4];
	if (RImplementation.o.distortion && sh_d && sh_d->flags.bDistort && pmask[sh_d->flags.iPriority/2]) {
		mapSorted_Node* N		= mapDistort.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= sh_d;		// 4=L_special
	}

	// Select shader
	ShaderElement*	sh		=	RImplementation.rimp_select_sh_dynamic	(pVisual,distSQ);
	if (0==sh)								return;
	if (!pmask[sh->flags.iPriority/2])		return;

	// Create common node
	// NOTE: Invisible elements exist only in R1
	_MatrixItem		item	= {SSA,RI.val_pObject,pVisual,*RI.val_pTransform};

	// HUD rendering
	if (RI.val_bHUD)			
	{
		if (sh->flags.bStrictB2F)	
		{
			if (sh->flags.bEmissive) {
				mapSorted_Node* N = mapHUDEmissive.insertInAnyWay(distSQ);
				N->val.ssa = SSA;
				N->val.pObject = RI.val_pObject;
				N->val.pVisual = pVisual;
				N->val.Matrix = *RI.val_pTransform;
				N->val.se = &*pVisual->shader->E[4];		// 4=L_special
			}
			mapSorted_Node* N = mapHUDSorted.insertInAnyWay(distSQ);
			N->val.ssa = SSA;
			N->val.pObject = RI.val_pObject;
			N->val.pVisual = pVisual;
			N->val.Matrix = *RI.val_pTransform;
			N->val.se = sh;
			return;
		} 
		else 
		{
			mapHUD_Node* N			= mapHUD.insertInAnyWay		(distSQ);
			N->val.ssa				= SSA;
			N->val.pObject			= RI.val_pObject;
			N->val.pVisual			= pVisual;
			N->val.Matrix			= *RI.val_pTransform;
			N->val.se				= sh;

			if (sh->flags.bEmissive) 
			{
				mapSorted_Node* N		= mapHUDEmissive.insertInAnyWay	(distSQ);
				N->val.ssa				= SSA;
				N->val.pObject			= RI.val_pObject;
				N->val.pVisual			= pVisual;
				N->val.Matrix			= *RI.val_pTransform;
				N->val.se				= &*pVisual->shader->E[4];		// 4=L_special
			}

			return;
		}
	}

	// Shadows registering
	if (RI.val_bInvisible)		return;

	// strict-sorting selection
	if (sh->flags.bStrictB2F)	{
		mapSorted_Node* N		= mapSorted.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= sh;
		return;
	}


	// Emissive geometry should be marked and R2 special-cases it
	// a) Allow to skeep already lit pixels
	// b) Allow to make them 100% lit and really bright
	// c) Should not cast shadows
	// d) Should be rendered to accumulation buffer in the second pass
	if (sh->flags.bEmissive) {
		mapSorted_Node* N		= mapEmissive.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= &*pVisual->shader->E[4];		// 4=L_special
	}
	if (sh->flags.bWmark	&& pmask_wmark)	{
		mapSorted_Node* N		= mapWmark.insertInAnyWay		(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= sh;							
		return					;
	}


	for ( u32 iPass = 0; iPass<sh->passes.size(); ++iPass)
	{
		// the most common node
		//SPass&						pass	= *sh->passes.front	();
		//mapMatrix_T&				map		= mapMatrix			[sh->flags.iPriority/2];
		SPass&						pass	= *sh->passes[iPass];
		mapMatrix_T&				map		= mapMatrixPasses	[sh->flags.iPriority/2][iPass];
		

#ifdef USE_RESOURCE_DEBUGGER
		mapMatrixVS::TNode*			Nvs		= map.insert		(pass.vs);
		mapMatrixGS::TNode*			Ngs		= Nvs->val.insert	(pass.gs);
		mapMatrixPS::TNode*			Nps		= Ngs->val.insert	(pass.ps);
#else
		mapMatrixVS::TNode*			Nvs		= map.insert		(&*pass.vs);
		mapMatrixGS::TNode*			Ngs		= Nvs->val.insert	(pass.gs->gs);
		mapMatrixPS::TNode*			Nps		= Ngs->val.insert	(pass.ps->ps);
#endif


#	ifdef USE_RESOURCE_DEBUGGER
		Nps->val.hs = pass.hs;
		Nps->val.ds = pass.ds;
		mapMatrixCS::TNode*			Ncs		= Nps->val.mapCS.insert	(pass.constants._get());
#	else
		Nps->val.hs = pass.hs->sh;
		Nps->val.ds = pass.ds->sh;
		mapMatrixCS::TNode*			Ncs		= Nps->val.mapCS.insert	(pass.constants._get());
#	endif

		mapMatrixStates::TNode*		Nstate	= Ncs->val.insert	(pass.state->state);
		mapMatrixTextures::TNode*	Ntex	= Nstate->val.insert(pass.T._get());
		mapMatrixItems&				items	= Ntex->val;
		items.push_back						(item);

		// Need to sort for HZB efficient use
		if (SSA>Ntex->val.ssa)		{ Ntex->val.ssa = SSA;
		if (SSA>Nstate->val.ssa)	{ Nstate->val.ssa = SSA;
		if (SSA>Ncs->val.ssa)		{ Ncs->val.ssa = SSA;
		if (SSA>Nps->val.mapCS.ssa)		{ Nps->val.mapCS.ssa = SSA;


		if (SSA>Ngs->val.ssa)		{ Ngs->val.ssa = SSA;

		if (SSA>Nvs->val.ssa)		{ Nvs->val.ssa = SSA;

							} 
						} 
					} 
				} 
			} 
		}
	}

	if (val_recorder)			{
		Fbox3		temp		;
		Fmatrix&	xf			= *RI.val_pTransform;
		temp.xform	(pVisual->vis.box,xf);
		val_recorder->push_back	(temp);
	}
}

void R_dsgraph_structure::r_dsgraph_insert_static	(dxRender_Visual *pVisual)
{
	CRender&	RI				=	RImplementation;

	if (pVisual->vis.marker		==	RI.marker)	return	;
	pVisual->vis.marker			=	RI.marker			;

	float distSQ;
	float SSA					=	CalcSSA		(distSQ,pVisual->vis.sphere.P,pVisual);
	if (SSA<=r_ssaDISCARD)		return;

	// Distortive geometry should be marked and R2 special-cases it
	// a) Allow to optimize RT order
	// b) Should be rendered to special distort buffer in another pass
	VERIFY						(pVisual->shader._get());
	ShaderElement*		sh_d	= &*pVisual->shader->E[4];
	if (RImplementation.o.distortion && sh_d && sh_d->flags.bDistort && pmask[sh_d->flags.iPriority/2]) {
		mapSorted_Node* N		= mapDistort.insertInAnyWay		(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= NULL;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= Fidentity;
		N->val.se				= &*pVisual->shader->E[4];		// 4=L_special
	}

	// Select shader
	ShaderElement*		sh		= RImplementation.rimp_select_sh_static(pVisual,distSQ);
	if (0==sh)								return;
	if (!pmask[sh->flags.iPriority/2])		return;

	// Water rendering
	if (sh->flags.isWater)
	{
		mapWater_Node* N = mapWater.insertInAnyWay(distSQ);
		N->val.ssa = SSA;
		N->val.pObject = NULL;
		N->val.pVisual = pVisual;
		N->val.Matrix = Fidentity;
		N->val.se = sh;
		return;
	}

	// strict-sorting selection
	if (sh->flags.bStrictB2F) {
		mapSorted_Node* N			= mapSorted.insertInAnyWay(distSQ);
		N->val.pObject				= NULL;
		N->val.pVisual				= pVisual;
		N->val.Matrix				= Fidentity;
		N->val.se					= sh;
		return;
	}

	// Emissive geometry should be marked and R2 special-cases it
	// a) Allow to skeep already lit pixels
	// b) Allow to make them 100% lit and really bright
	// c) Should not cast shadows
	// d) Should be rendered to accumulation buffer in the second pass
	if (sh->flags.bEmissive) {
		mapSorted_Node* N		= mapEmissive.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= NULL;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= Fidentity;
		N->val.se				= &*pVisual->shader->E[4];		// 4=L_special
	}
	if (sh->flags.bWmark	&& pmask_wmark)	{
		mapSorted_Node* N		= mapWmark.insertInAnyWay		(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= NULL;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= Fidentity;
		N->val.se				= sh;							
		return					;
	}


	if	(val_feedback && counter_S==val_feedback_breakp)	val_feedback->rfeedback_static(pVisual);

	counter_S					++;

	if (sh->flags.bLandscape && RI.phase == CRender::PHASE_NORMAL)
	{
		mapLandscape_Node* N = mapLandscape.insertInAnyWay(distSQ);
		N->val.ssa = SSA;
		N->val.pObject = NULL;
		N->val.pVisual = pVisual;
		N->val.Matrix = Fidentity;
		N->val.se = sh;
		return;
	}

	for ( u32 iPass = 0; iPass<sh->passes.size(); ++iPass)
	{
		//SPass&						pass	= *sh->passes.front	();
		//mapNormal_T&				map		= mapNormal			[sh->flags.iPriority/2];
		SPass&						pass	= *sh->passes[iPass];
		mapNormal_T&				map		= mapNormalPasses[sh->flags.iPriority/2][iPass];

#ifdef USE_RESOURCE_DEBUGGER
		mapNormalVS::TNode*			Nvs		= map.insert		(pass.vs);
		mapNormalGS::TNode*			Ngs		= Nvs->val.insert	(pass.gs);
		mapNormalPS::TNode*			Nps		= Ngs->val.insert	(pass.ps);
#else // USE_RESOURCE_DEBUGGER

		mapNormalVS::TNode*			Nvs		= map.insert		(&*pass.vs);
		mapNormalGS::TNode*			Ngs		= Nvs->val.insert	(pass.gs->gs);
		mapNormalPS::TNode*			Nps		= Ngs->val.insert	(pass.ps->ps);

#endif // USE_RESOURCE_DEBUGGER


#	ifdef USE_RESOURCE_DEBUGGER
		Nps->val.hs = pass.hs;
		Nps->val.ds = pass.ds;
		mapNormalCS::TNode*			Ncs		= Nps->val.mapCS.insert	(pass.constants._get());
#	else
		Nps->val.hs = pass.hs->sh;
		Nps->val.ds = pass.ds->sh;
		mapNormalCS::TNode*			Ncs		= Nps->val.mapCS.insert	(pass.constants._get());
#	endif

		mapNormalStates::TNode*		Nstate	= Ncs->val.insert	(pass.state->state);
		mapNormalTextures::TNode*	Ntex	= Nstate->val.insert(pass.T._get());
		mapNormalItems&				items	= Ntex->val;
		_NormalItem					item	= {SSA,pVisual};
		items.push_back						(item);

		// Need to sort for HZB efficient use
		if (SSA>Ntex->val.ssa)		{ Ntex->val.ssa = SSA;
		if (SSA>Nstate->val.ssa)	{ Nstate->val.ssa = SSA;
		if (SSA>Ncs->val.ssa)		{ Ncs->val.ssa = SSA;
		if (SSA>Nps->val.mapCS.ssa)		{ Nps->val.mapCS.ssa = SSA;
		if (SSA>Ngs->val.ssa)		{ Ngs->val.ssa = SSA;
		if (SSA>Nvs->val.ssa)		{ Nvs->val.ssa = SSA;
		} } } } } }
	}


	if (val_recorder)			{
		val_recorder->push_back	(pVisual->vis.box	);
	}

}

	// Static geometry optimization
#define O_S_L1_S_LOW    10.f // geometry 3d volume size
#define O_S_L1_D_LOW    150.f // distance, after which it is not rendered
#define O_S_L2_S_LOW    100.f
#define O_S_L2_D_LOW    200.f
#define O_S_L3_S_LOW    500.f
#define O_S_L3_D_LOW    250.f
#define O_S_L4_S_LOW    2500.f
#define O_S_L4_D_LOW    350.f
#define O_S_L5_S_LOW    7000.f
#define O_S_L5_D_LOW    400.f

#define O_S_L1_S_MED    25.f
#define O_S_L1_D_MED    50.f
#define O_S_L2_S_MED    200.f
#define O_S_L2_D_MED    150.f
#define O_S_L3_S_MED    1000.f
#define O_S_L3_D_MED    200.f
#define O_S_L4_S_MED    2500.f
#define O_S_L4_D_MED    300.f
#define O_S_L5_S_MED    7000.f
#define O_S_L5_D_MED    400.f

#define O_S_L1_S_HII    50.f
#define O_S_L1_D_HII    50.f
#define O_S_L2_S_HII    400.f
#define O_S_L2_D_HII    150.f
#define O_S_L3_S_HII    1500.f
#define O_S_L3_D_HII    200.f
#define O_S_L4_S_HII    5000.f
#define O_S_L4_D_HII    300.f
#define O_S_L5_S_HII    20000.f
#define O_S_L5_D_HII    350.f

#define O_S_L1_S_ULT    50.f
#define O_S_L1_D_ULT    35.f
#define O_S_L2_S_ULT    500.f
#define O_S_L2_D_ULT    125.f
#define O_S_L3_S_ULT    1750.f
#define O_S_L3_D_ULT    175.f
#define O_S_L4_S_ULT    5250.f
#define O_S_L4_D_ULT    250.f
#define O_S_L5_S_ULT    25000.f
#define O_S_L5_D_ULT    300.f

// Dyn geometry optimization

#define O_D_L1_S_LOW    1.f // geometry 3d volume size
#define O_D_L1_D_LOW    80.f // distance, after which it is not rendered
#define O_D_L2_S_LOW    3.f
#define O_D_L2_D_LOW    150.f
#define O_D_L3_S_LOW    4000.f
#define O_D_L3_D_LOW    250.f

#define O_D_L1_S_MED    1.f
#define O_D_L1_D_MED    40.f
#define O_D_L2_S_MED    4.f
#define O_D_L2_D_MED    100.f
#define O_D_L3_S_MED    4000.f
#define O_D_L3_D_MED    200.f

#define O_D_L1_S_HII    1.4f
#define O_D_L1_D_HII    30.f
#define O_D_L2_S_HII    4.f
#define O_D_L2_D_HII    80.f
#define O_D_L3_S_HII    4000.f
#define O_D_L3_D_HII    150.f

#define O_D_L1_S_ULT    2.0f
#define O_D_L1_D_ULT    30.f
#define O_D_L2_S_ULT    8.f
#define O_D_L2_D_ULT    50.f
#define O_D_L3_S_ULT    4000.f
#define O_D_L3_D_ULT    110.f

	Fvector4 o_optimize_static_l1_dist = { O_S_L1_D_LOW, O_S_L1_D_MED, O_S_L1_D_HII, O_S_L1_D_ULT };
	Fvector4 o_optimize_static_l1_size = { O_S_L1_S_LOW, O_S_L1_S_MED, O_S_L1_S_HII, O_S_L1_S_ULT };
	Fvector4 o_optimize_static_l2_dist = { O_S_L2_D_LOW, O_S_L2_D_MED, O_S_L2_D_HII, O_S_L2_D_ULT };
	Fvector4 o_optimize_static_l2_size = { O_S_L2_S_LOW, O_S_L2_S_MED, O_S_L2_S_HII, O_S_L2_S_ULT };
	Fvector4 o_optimize_static_l3_dist = { O_S_L3_D_LOW, O_S_L3_D_MED, O_S_L3_D_HII, O_S_L3_D_ULT };
	Fvector4 o_optimize_static_l3_size = { O_S_L3_S_LOW, O_S_L3_S_MED, O_S_L3_S_HII, O_S_L3_S_ULT };
	Fvector4 o_optimize_static_l4_dist = { O_S_L4_D_LOW, O_S_L4_D_MED, O_S_L4_D_HII, O_S_L4_D_ULT };
	Fvector4 o_optimize_static_l4_size = { O_S_L4_S_LOW, O_S_L4_S_MED, O_S_L4_S_HII, O_S_L4_S_ULT };
	Fvector4 o_optimize_static_l5_dist = { O_S_L5_D_LOW, O_S_L5_D_MED, O_S_L5_D_HII, O_S_L5_D_ULT };
	Fvector4 o_optimize_static_l5_size = { O_S_L5_S_LOW, O_S_L5_S_MED, O_S_L5_S_HII, O_S_L5_S_ULT };

	Fvector4 o_optimize_dynamic_l1_dist = { O_D_L1_D_LOW, O_D_L1_D_MED, O_D_L1_D_HII, O_D_L1_D_ULT };
	Fvector4 o_optimize_dynamic_l1_size = { O_D_L1_S_LOW, O_D_L1_S_MED, O_D_L1_S_HII, O_D_L1_S_ULT };
	Fvector4 o_optimize_dynamic_l2_dist = { O_D_L2_D_LOW, O_D_L2_D_MED, O_D_L2_D_HII, O_D_L2_D_ULT };
	Fvector4 o_optimize_dynamic_l2_size = { O_D_L2_S_LOW, O_D_L2_S_MED, O_D_L2_S_HII, O_D_L2_S_ULT };
	Fvector4 o_optimize_dynamic_l3_dist = { O_D_L3_D_LOW, O_D_L3_D_MED, O_D_L3_D_HII, O_D_L3_D_ULT };
	Fvector4 o_optimize_dynamic_l3_size = { O_D_L3_S_LOW, O_D_L3_S_MED, O_D_L3_S_HII, O_D_L3_S_ULT };

#define BASE_FOV 67.f

	IC float GetDistFromCamera(const Fvector& from_position)
		// Aproximate, adjusted by fov, distance from camera to position (For right work when looking though binoculars and scopes)
	{
		float distance = Device.vCameraPosition.distance_to(from_position);
		float fov_K = BASE_FOV / Device.fFOV;
		float adjusted_distane = distance / fov_K;

		return adjusted_distane;
	}

	IC bool IsValuableToRender(dxRender_Visual* pVisual, bool isStatic, bool sm, Fmatrix& transform_matrix)
	{
		if ((isStatic && opt_static >= 1) || (!isStatic && opt_dynamic >= 1))
		{
			float sphere_volume = pVisual->getVisData().sphere.volume();

			float adjusted_distane = 0;

			if (isStatic)
				adjusted_distane = GetDistFromCamera(pVisual->vis.sphere.P);
			else
				// dynamic geometry position needs to be transformed by transform matrix, to get world coordinates, dont forget ;)
			{
				Fvector pos;
				transform_matrix.transform_tiny(pos, pVisual->vis.sphere.P);

				adjusted_distane = GetDistFromCamera(pos);
			}

			if (sm && !!psDeviceFlags2.test(rsOptShadowGeom)) // Highest cut off for shadow map
			{
				if (sphere_volume < 50000.f && adjusted_distane > 160);
					// don't need geometry behind the farest sun shadow cascade
					return false;

				if ((sphere_volume < o_optimize_static_l1_size.z) && (adjusted_distane > o_optimize_static_l1_dist.z))
					return false;
				else if ((sphere_volume < o_optimize_static_l2_size.z) && (adjusted_distane > o_optimize_static_l2_dist.z))
					return false;
				else if ((sphere_volume < o_optimize_static_l3_size.z) && (adjusted_distane > o_optimize_static_l3_dist.z))
					return false;
				else if ((sphere_volume < o_optimize_static_l4_size.z) && (adjusted_distane > o_optimize_static_l4_dist.z))
					return false;
				else if ((sphere_volume < o_optimize_static_l5_size.z) && (adjusted_distane > o_optimize_static_l5_dist.z))
					return false;

				return true;
			}

			if (isStatic)
			{
				if (pVisual->Type == MT_LOD || pVisual->Type == MT_TREE_PM || pVisual->Type == MT_TREE_ST)

					return true;

				if (opt_static == 2)
				{
					if ((sphere_volume < o_optimize_static_l1_size.y) && (adjusted_distane > o_optimize_static_l1_dist.y))
						return false;
					else if ((sphere_volume < o_optimize_static_l2_size.y) && (adjusted_distane > o_optimize_static_l2_dist.
						y))
						return false;
					else if ((sphere_volume < o_optimize_static_l3_size.y) && (adjusted_distane > o_optimize_static_l3_dist.
						y))
						return false;
					else if ((sphere_volume < o_optimize_static_l4_size.y) && (adjusted_distane > o_optimize_static_l4_dist.
						y))
						return false;
					else if ((sphere_volume < o_optimize_static_l5_size.y) && (adjusted_distane > o_optimize_static_l5_dist.
						y))
						return false;
				}
				else if (opt_static == 3)
				{
					if ((sphere_volume < o_optimize_static_l1_size.z) && (adjusted_distane > o_optimize_static_l1_dist.z))
						return false;
					else if ((sphere_volume < o_optimize_static_l2_size.z) && (adjusted_distane > o_optimize_static_l2_dist.
						z))
						return false;
					else if ((sphere_volume < o_optimize_static_l3_size.z) && (adjusted_distane > o_optimize_static_l3_dist.
						z))
						return false;
					else if ((sphere_volume < o_optimize_static_l4_size.z) && (adjusted_distane > o_optimize_static_l4_dist.
						z))
						return false;
					else if ((sphere_volume < o_optimize_static_l5_size.z) && (adjusted_distane > o_optimize_static_l5_dist.
						z))
						return false;
				}
				else if (opt_static == 4)
				{
					if ((sphere_volume < o_optimize_static_l1_size.w) && (adjusted_distane > o_optimize_static_l1_dist.w))
						return false;
					else if ((sphere_volume < o_optimize_static_l2_size.w) && (adjusted_distane > o_optimize_static_l2_dist.
						w))
						return false;
					else if ((sphere_volume < o_optimize_static_l3_size.w) && (adjusted_distane > o_optimize_static_l3_dist.
						w))
						return false;
					else if ((sphere_volume < o_optimize_static_l4_size.w) && (adjusted_distane > o_optimize_static_l4_dist.
						w))
						return false;
					else if ((sphere_volume < o_optimize_static_l5_size.w) && (adjusted_distane > o_optimize_static_l5_dist.
						w))
						return false;
				}
				else
				{
					if ((sphere_volume < o_optimize_static_l1_size.x) && (adjusted_distane > o_optimize_static_l1_dist.x))
						return false;
					else if ((sphere_volume < o_optimize_static_l2_size.x) && (adjusted_distane > o_optimize_static_l2_dist.
						x))
						return false;
					else if ((sphere_volume < o_optimize_static_l3_size.x) && (adjusted_distane > o_optimize_static_l3_dist.
						x))
						return false;
					else if ((sphere_volume < o_optimize_static_l4_size.x) && (adjusted_distane > o_optimize_static_l4_dist.
						x))
						return false;
					else if ((sphere_volume < o_optimize_static_l5_size.x) && (adjusted_distane > o_optimize_static_l5_dist.
						x))
						return false;
				}
			}
			else
			{
				if (opt_dynamic == 2)
				{
					if ((sphere_volume < o_optimize_dynamic_l1_size.y) && (adjusted_distane > o_optimize_dynamic_l1_dist.y))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l2_size.y) && (adjusted_distane >
						o_optimize_dynamic_l2_dist.y))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l3_size.y) && (adjusted_distane >
						o_optimize_dynamic_l3_dist.y))
						return false;
				}
				else if (opt_dynamic == 3)
				{
					if ((sphere_volume < o_optimize_dynamic_l1_size.z) && (adjusted_distane > o_optimize_dynamic_l1_dist.z))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l2_size.z) && (adjusted_distane >
						o_optimize_dynamic_l2_dist.z))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l3_size.z) && (adjusted_distane >
						o_optimize_dynamic_l3_dist.z))
						return false;
				}
				else if (opt_dynamic == 4)
				{
					if ((sphere_volume < o_optimize_dynamic_l1_size.w) && (adjusted_distane > o_optimize_dynamic_l1_dist.w))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l2_size.w) && (adjusted_distane >
						o_optimize_dynamic_l2_dist.w))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l3_size.w) && (adjusted_distane >
						o_optimize_dynamic_l3_dist.w))
						return false;
				}
				else
				{
					if ((sphere_volume < o_optimize_dynamic_l1_size.x) && (adjusted_distane > o_optimize_dynamic_l1_dist.x))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l2_size.x) && (adjusted_distane >
						o_optimize_dynamic_l2_dist.x))
						return false;
					else if ((sphere_volume < o_optimize_dynamic_l3_size.x) && (adjusted_distane >
						o_optimize_dynamic_l3_dist.x))
						return false;
				}
			}
		}

		return true;
	}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRender::add_leafs_Dynamic	(dxRender_Visual *pVisual)
{
	if (!pVisual)				return;

	if (!IsValuableToRender(pVisual, false, phase == 1, *val_pTransform))
		return;

	// Visual is 100% visible - simply add it
	xr_vector<dxRender_Visual*>::iterator I,E;	// it may be useful for 'hierrarhy' visual

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG	= (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++)	{
				PS::CParticleGroup::SItem&			I		= *i_it;
				if (I._effect)		add_leafs_Dynamic		(I._effect);
				for (xr_vector<dxRender_Visual*>::iterator pit = I._children_related.begin();	pit!=I._children_related.end(); pit++)	add_leafs_Dynamic(*pit);
				for (xr_vector<dxRender_Visual*>::iterator pit = I._children_free.begin();		pit!=I._children_free.end();	pit++)	add_leafs_Dynamic(*pit);
			}
		}
		return;
	case MT_HIERRARHY:
		{
			// Add all children, doesn't perform any tests
			FHierrarhyVisual* pV = (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end	();
			for (; I!=E; I++)	add_leafs_Dynamic	(*I);
		}
		return;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV			= (CKinematics*)pVisual;
			BOOL	_use_lod			= FALSE	;
			if (pV->m_lod)				
			{
				Fvector							Tpos;	float		D;
				val_pTransform->transform_tiny	(Tpos, pV->vis.sphere.P);
				float		ssa		=	CalcSSA	(D,Tpos,pV->vis.sphere.R/2.f);	// assume dynamics never consume full sphere
				if (ssa<r_ssaLOD_A)	_use_lod	= TRUE;
			}
			if (_use_lod)				
			{
				add_leafs_Dynamic			(pV->m_lod)		;
			} else {
				pV->CalculateBones			(TRUE);
				pV->CalculateWallmarks		();		//. bug?
				I = pV->children.begin		();
				E = pV->children.end		();
				for (; I!=E; I++)	add_leafs_Dynamic	(*I);
			}
		}
		return;
	default:
		{
			// General type of visual
			// Calculate distance to it's center
			Fvector							Tpos;
			val_pTransform->transform_tiny	(Tpos, pVisual->vis.sphere.P);
			r_dsgraph_insert_dynamic		(pVisual,Tpos);
		}
		return;
	}
}

void CRender::add_leafs_Static(dxRender_Visual *pVisual)
{
	if (!HOM.visible(pVisual->vis))		return;

	if (!IsValuableToRender(pVisual, true, phase == 1, *val_pTransform))
		return;

	// Visual is 100% visible - simply add it
	xr_vector<dxRender_Visual*>::iterator I,E;	// it may be usefull for 'hierrarhy' visuals

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG = (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++){
				PS::CParticleGroup::SItem&			I		= *i_it;
				if (I._effect)		add_leafs_Dynamic		(I._effect);
				for (xr_vector<dxRender_Visual*>::iterator pit = I._children_related.begin();	pit!=I._children_related.end(); pit++)	add_leafs_Dynamic(*pit);
				for (xr_vector<dxRender_Visual*>::iterator pit = I._children_free.begin();		pit!=I._children_free.end();	pit++)	add_leafs_Dynamic(*pit);
			}
		}
		return;
	case MT_HIERRARHY:
		{
			// Add all children, doesn't perform any tests
			FHierrarhyVisual* pV	= (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end	();
			for (; I!=E; I++)		add_leafs_Static (*I);
		}
		return;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV		= (CKinematics*)pVisual;
			pV->CalculateBones		(TRUE);
			I = pV->children.begin	();
			E = pV->children.end	();
			for (; I!=E; I++)		add_leafs_Static	(*I);
		}
		return;
	case MT_LOD:
		{
			FLOD		* pV	=		(FLOD*) pVisual;
			float		D;
			float		ssa		=		CalcSSA(D,pV->vis.sphere.P,pV);
			ssa					*=		pV->lod_factor;
			if (ssa<r_ssaLOD_A)
			{
				if (ssa<r_ssaDISCARD)	return;
				mapLOD_Node*	N	=	mapLOD.insertInAnyWay(D);
				N->val.ssa			=	ssa;
				N->val.pVisual		=	pVisual;
			}
			if (ssa>r_ssaLOD_B || phase==PHASE_SMAP)
			{
				// Add all children, doesn't perform any tests
				I = pV->children.begin	();
				E = pV->children.end	();
				for (; I!=E; I++)	add_leafs_Static (*I);
			}
		}
		return;
	case MT_TREE_PM:
	case MT_TREE_ST:
		{
			// General type of visual
			r_dsgraph_insert_static		(pVisual);
		}
		return;
	default:
		{
			// General type of visual
			r_dsgraph_insert_static		(pVisual);
		}
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CRender::add_Dynamic(dxRender_Visual *pVisual, u32 planes)
{

	if (!IsValuableToRender(pVisual, false, phase == 1, *val_pTransform))
		return FALSE;

	// Check frustum visibility and calculate distance to visual's center
	Fvector		Tpos;	// transformed position
	EFC_Visible	VIS;

	val_pTransform->transform_tiny	(Tpos, pVisual->vis.sphere.P);
	VIS = View->testSphere			(Tpos, pVisual->vis.sphere.R,planes);
	if (fcvNone==VIS) return FALSE	;

	// If we get here visual is visible or partially visible
	xr_vector<dxRender_Visual*>::iterator I,E;	// it may be usefull for 'hierrarhy' visuals

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG = (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++)
			{
				PS::CParticleGroup::SItem&			I		= *i_it;
				if (fcvPartial==VIS) 
				{
					if (I._effect)		add_Dynamic				(I._effect,planes);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_related.begin();	pit!=I._children_related.end(); pit++)	add_Dynamic(*pit,planes);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_free.begin();		pit!=I._children_free.end();	pit++)	add_Dynamic(*pit,planes);
				} else 
				{
					if (I._effect)		add_leafs_Dynamic		(I._effect);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_related.begin();	pit!=I._children_related.end(); pit++)	add_leafs_Dynamic(*pit);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_free.begin();		pit!=I._children_free.end();	pit++)	add_leafs_Dynamic(*pit);
				}
			}
		}
		break;
	case MT_HIERRARHY:
		{
			// Add all children
			FHierrarhyVisual* pV = (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end	();
			if (fcvPartial==VIS) 
			{
				for (; I!=E; I++)	add_Dynamic			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Dynamic	(*I);
			}
		}
		break;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV			= (CKinematics*)pVisual;
			BOOL	_use_lod			= FALSE	;
			if (pV->m_lod)				
			{
				Fvector							Tpos;	float		D;
				val_pTransform->transform_tiny	(Tpos, pV->vis.sphere.P);
				float		ssa		=	CalcSSA	(D,Tpos,pV->vis.sphere.R/2.f);	// assume dynamics never consume full sphere
				if (ssa<r_ssaLOD_A)	_use_lod	= TRUE		;
			}
			if (_use_lod)
			{
				add_leafs_Dynamic			(pV->m_lod)		;
			} else 
			{
				pV->CalculateBones			(TRUE);
				pV->CalculateWallmarks		();		//. bug?
				I = pV->children.begin		();
				E = pV->children.end		();
				for (; I!=E; I++)	add_leafs_Dynamic	(*I);
			}
			/*
			I = pV->children.begin		();
			E = pV->children.end		();
			if (fcvPartial==VIS) {
				for (; I!=E; I++)	add_Dynamic			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Dynamic	(*I);
			}
			*/
		}
		break;
	default:
		{
			// General type of visual
			r_dsgraph_insert_dynamic(pVisual,Tpos);
		}
		break;
	}
	return TRUE;
}

void CRender::add_Static(dxRender_Visual *pVisual, u32 planes)
{
	if (!IsValuableToRender(pVisual, true, phase == 1, *val_pTransform))
		return;

	// Check frustum visibility and calculate distance to visual's center
	EFC_Visible	VIS;
	vis_data&	vis			= pVisual->vis;
	VIS = View->testSAABB	(vis.sphere.P,vis.sphere.R,vis.box.data(),planes);
	if (fcvNone==VIS)		
		return;

	if (!HOM.visible(vis))	
		return;

	// If we get here visual is visible or partially visible
	xr_vector<dxRender_Visual*>::iterator I,E;	// it may be usefull for 'hierrarhy' visuals

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG = (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++){
				PS::CParticleGroup::SItem&			I		= *i_it;
				if (fcvPartial==VIS) {
					if (I._effect)		add_Dynamic				(I._effect,planes);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_related.begin();	pit!=I._children_related.end(); pit++)	add_Dynamic(*pit,planes);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_free.begin();		pit!=I._children_free.end();	pit++)	add_Dynamic(*pit,planes);
				} else {
					if (I._effect)		add_leafs_Dynamic		(I._effect);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_related.begin();	pit!=I._children_related.end(); pit++)	add_leafs_Dynamic(*pit);
					for (xr_vector<dxRender_Visual*>::iterator pit = I._children_free.begin();		pit!=I._children_free.end();	pit++)	add_leafs_Dynamic(*pit);
				}
			}
		}
		break;
	case MT_HIERRARHY:
		{
			// Add all children
			FHierrarhyVisual* pV = (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end		();
			if (fcvPartial==VIS) {
				for (; I!=E; I++)	add_Static			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Static	(*I);
			}
		}
		break;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV		= (CKinematics*)pVisual;
			pV->CalculateBones		(TRUE);
			I = pV->children.begin	();
			E = pV->children.end	();
			if (fcvPartial==VIS) {
				for (; I!=E; I++)	add_Static			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Static	(*I);
			}
		}
		break;
	case MT_LOD:
		{
			FLOD		* pV	= (FLOD*) pVisual;
			float		D;
			float		ssa		= CalcSSA	(D,pV->vis.sphere.P,pV);
			ssa					*= pV->lod_factor;
			if (ssa<r_ssaLOD_A)	
			{
				if (ssa<r_ssaDISCARD)	return;
				mapLOD_Node*	N		= mapLOD.insertInAnyWay(D);
				N->val.ssa				= ssa;
				N->val.pVisual			= pVisual;
			}

			if (ssa>r_ssaLOD_B || phase==PHASE_SMAP)
			{
				// Add all children, perform tests
				I = pV->children.begin	();
				E = pV->children.end	();
				for (; I!=E; I++)	add_leafs_Static	(*I);
			}
		}
		break;
	case MT_TREE_ST:
	case MT_TREE_PM:
		{
			// General type of visual
			r_dsgraph_insert_static		(pVisual);
		}
		return;
	default:
		{
			// General type of visual
			r_dsgraph_insert_static		(pVisual);
		}
		break;
	}
}
