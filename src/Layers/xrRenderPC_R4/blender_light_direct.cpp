#include "stdafx.h"
#pragma hdrstop

#include "Blender_light_direct.h"

CBlender_accum_direct::CBlender_accum_direct	()	{ description.CLS		= 0;	}
CBlender_accum_direct::~CBlender_accum_direct	()	{	}

void	CBlender_accum_direct::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

//	BOOL	b_HW_smap		= RImplementation.o.HW_smap;
//	BOOL	b_HW_PCF		= RImplementation.o.HW_smap_PCF;
	BOOL		blend		= false;	//RImplementation.o.fp16_blend;
	D3D11_BLEND	dest = blend ? D3D11_BLEND_ONE : D3D11_BLEND_ZERO;
	if (RImplementation.o.sunfilter)	{ blend = false;}

	switch (C.iElement)
	{
	case SE_SUN_NEAR:		// near pass - enable Z-test to perform depth-clipping
	case SE_SUN_MIDDLE:		// middle pass - enable Z-test to perform depth-clipping
		//	FVF::TL2uv
		C.r_Pass("accum_sun", "accum_sun_near_nomsaa_nominmax", false, true, false, blend, D3D11_BLEND_ONE, dest);

		C.r_CullMode(D3D11_CULL_NONE);
		C.PassSET_ZB		(true,false,true	);	// force inverted Z-Buffer

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);
		C.r_dx10Texture		("s_smap_minmax",	r2_RT_smap_depth_minmax);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");

		C.r_End				();
		break;
	case SE_SUN_FAR:		// far pass, only stencil clipping performed

		C.r_Pass("accum_sun", "accum_sun_far_nomsaa", false, true, false, blend, D3D11_BLEND_ONE, dest);
		C.r_CullMode(D3D11_CULL_NONE);
		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		{
			u32 s = C.r_dx10Sampler("smp_smap");
			C.i_dx10Address		(s, D3DTADDRESS_BORDER);
			C.i_dx10BorderColor	(s, D3DCOLOR_ARGB(255, 255, 255, 255));
		}

		C.r_End				();
		break;
	case SE_SUN_LUMINANCE:	// luminance pass

		C.r_Pass			("stub_notransform_aa_AA","accum_sun_nomsaa",		false,	false,	false);
		C.r_CullMode		(D3D11_CULL_NONE);


		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_smap",			r2_RT_generic0);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		jitter				(C);
		C.r_End				();
		break;

		//	SE_SUN_NEAR for min/max
	case SE_SUN_NEAR_MINMAX:		// near pass - enable Z-test to perform depth-clipping
		//	FVF::TL2uv
		C.r_Pass("accum_sun", "accum_sun_near_nomsaa_minmax", false, true, false, blend, D3D11_BLEND_ONE, dest);
		C.r_CullMode(D3D11_CULL_NONE);
		C.PassSET_ZB		(true,false,true	);	// force inverted Z-Buffer

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);
		C.r_dx10Texture		("s_smap_minmax",	r2_RT_smap_depth_minmax);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");

		C.r_End				();
		break;
	}
}


CBlender_accum_direct_msaa::CBlender_accum_direct_msaa	()	{		Name = 0; Definition = 0; description.CLS		= 0;	}
CBlender_accum_direct_msaa::~CBlender_accum_direct_msaa	()	{	}

//	TODO: DX10:	implement CBlender_accum_direct::Compile
void	CBlender_accum_direct_msaa::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

   if( Name )
      ::Render->m_MSAASample = atoi( Definition );
   else
      ::Render->m_MSAASample = -1;


	BOOL		blend		= false;
	D3D11_BLEND	dest = blend ? D3D11_BLEND_ONE : D3D11_BLEND_ZERO;
	if (RImplementation.o.sunfilter) { blend = false; }

	switch (C.iElement)
	{
	case SE_SUN_NEAR:		// near pass - enable Z-test to perform depth-clipping
	case SE_SUN_MIDDLE:		// middle pass - enable Z-test to perform depth-clipping
		//	FVF::TL2uv

		C.r_Pass("accum_sun", "accum_sun_near_msaa_nominmax", false, true, false, blend, D3D11_BLEND_ONE, dest);
		C.r_CullMode(D3D11_CULL_NONE);
		C.PassSET_ZB		(true,false,true	);	// force inverted Z-Buffer


		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");

		C.r_End				();
		break;
	case SE_SUN_FAR:		// far pass, only stencil clipping performed

		C.r_Pass("accum_sun", "accum_sun_far_msaa", false, true, false, blend, D3D11_BLEND_ONE, dest);
		C.r_CullMode(D3D11_CULL_NONE);


		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		{
			u32 s = C.r_dx10Sampler("smp_smap");
			C.i_dx10Address		(s, D3DTADDRESS_BORDER);
			C.i_dx10BorderColor	(s, D3DCOLOR_ARGB(255, 255, 255, 255));
		}

		C.r_End				();
		break;
	case SE_SUN_LUMINANCE:	// luminance pass

		C.r_Pass			("stub_notransform_aa_AA","accum_sun_msaa",		false,	false,	false);
		C.r_CullMode		(D3D11_CULL_NONE);


		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_smap",			r2_RT_generic0);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		jitter				(C);
		C.r_End				();
		break;

		
		//	SE_SUN_NEAR for minmax
	case SE_SUN_NEAR_MINMAX:		// near pass - enable Z-test to perform depth-clipping
		C.r_Pass("accum_sun", "accum_sun_near_msaa_minmax", false, true, false, blend, D3D11_BLEND_ONE, dest);
		C.r_CullMode(D3D11_CULL_NONE);
		C.PassSET_ZB		(true,false,true	);	// force inverted Z-Buffer

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);
		C.r_dx10Texture		("s_smap_minmax",	r2_RT_smap_depth_minmax);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");

		C.r_End				();
		break;

	}
   ::Render->m_MSAASample = -1;
}

CBlender_accum_direct_volumetric_msaa::CBlender_accum_direct_volumetric_msaa	()	{		Name = 0; Definition = 0; description.CLS		= 0;	}
CBlender_accum_direct_volumetric_msaa::~CBlender_accum_direct_volumetric_msaa	()	{	}

//	TODO: DX10:	implement CBlender_accum_direct::Compile
void	CBlender_accum_direct_volumetric_msaa::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

   if( Name )
      ::Render->m_MSAASample = atoi( Definition );
   else
      ::Render->m_MSAASample = -1;

	BOOL		blend		= false;	//RImplementation.o.fp16_blend;
	D3D11_BLEND	dest = blend ? D3D11_BLEND_ONE : D3D11_BLEND_ZERO;
	if (RImplementation.o.sunfilter)	{ blend = false; }

	switch (C.iElement)
		{
		case 0:		// near pass - enable Z-test to perform depth-clipping
			C.r_Pass("accum_sun", "accum_volumetric_sun_msaa", false, true, false, blend, D3D11_BLEND_ONE, dest);
				C.r_dx10Texture		("s_lmap",			C.L_textures[0]);
				C.r_dx10Texture	("s_smap",			r2_RT_smap_depth);
				 C.r_dx10Texture	("s_noise", "fx\\fx_noise");

         C.r_dx10Sampler		("smp_rtlinear");
				 C.r_dx10Sampler		("smp_linear");
				 C.r_dx10Sampler		("smp_smap");
         C.r_End				();
			break;
		}
   ::Render->m_MSAASample = -1;
}

CBlender_accum_direct_volumetric_sun_msaa::CBlender_accum_direct_volumetric_sun_msaa	()	{		Name = 0; Definition = 0; description.CLS		= 0;	}
CBlender_accum_direct_volumetric_sun_msaa::~CBlender_accum_direct_volumetric_sun_msaa	()	{	}

//	TODO: DX10:	implement CBlender_accum_direct::Compile
void	CBlender_accum_direct_volumetric_sun_msaa::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

   if( Name )
      ::Render->m_MSAASample = atoi( Definition );
   else
      ::Render->m_MSAASample = -1;

	switch (C.iElement)
		{
		case 0:		// near pass - enable Z-test to perform depth-clipping
			C.r_Pass("accum_sun", "accum_volumetric_sun_msaa", false, false, false, true, D3D11_BLEND_ONE, D3D11_BLEND_ONE, false, 0);
         C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);
         C.r_dx10Texture		("s_position",		r2_RT_P);
         jitter				(C);

         C.r_dx10Sampler		("smp_nofilter");
         C.r_dx10Sampler		("smp_smap");
         C.r_End				();
			break;
		}
   ::Render->m_MSAASample = -1;
}

