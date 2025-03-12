

#include <stdafx.h>	
#include "../../xrEngine/render.h"

#include "../xrRender/FBasicVisual.h"

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

#define BASE_FOV 73.5f

IC float GetDistFromCamera(const Fvector& from_position)
// Aproximate, adjusted by fov, distance from camera to position (For right work when looking though binoculars and scopes)
{
	float distance = Device.vCameraPosition.distance_to(from_position);
	float fov_K = BASE_FOV / Device.fFOV;
	float adjusted_distane = distance / fov_K;

	return adjusted_distane;
}

extern int opt_static;
extern int opt_dynamic;

bool IsValuableToRender(dxRender_Visual* pVisual, bool isStatic, bool sm, Fmatrix& transform_matrix, bool ignore = false)
{
	//	if (ignore)
	//		return true;

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

		if (sm) // Highest cut off for shadow map		 //&& !!psDeviceFlags2.test(rsOptShadowGeom)
		{
			if (sphere_volume < 50000.f && adjusted_distane > 160)
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
		}

		if (isStatic)
		{
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