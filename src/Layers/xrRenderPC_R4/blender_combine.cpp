#include "stdafx.h"
#pragma hdrstop

#include "Blender_combine.h"

CBlender_combine::CBlender_combine	()	{	description.CLS		= 0;	}
CBlender_combine::~CBlender_combine	()	{	}

void	CBlender_combine::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	switch (C.iElement)
	{
	case 0:	// combine
		C.r_Pass("combine_1", "combine_1_nomsaa", false, false, false, true, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA);	//. MRT-blend?
		C.r_Stencil(true, D3D11_COMPARISON_LESS_EQUAL, 0xff, 0x00);	// stencil should be >= 1
		C.r_StencilRef		(0x01);

		C.r_dx10Texture		("s_position",		r2_RT_P				);
		C.r_dx10Texture		("s_normal",		r2_RT_N				);
		C.r_dx10Texture		("s_diffuse",		r2_RT_albedo		);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum			);
		C.r_dx10Texture		("s_depth",			r2_RT_depth			);
		C.r_dx10Texture		("s_tonemap",		r2_RT_luminance_cur	);
		C.r_dx10Texture		("s_material",		r2_material			);
		C.r_dx10Texture		("env_s0",			r2_T_envs0			);
		C.r_dx10Texture		("env_s1",			r2_T_envs1			);
		C.r_dx10Texture		("sky_s0",			r2_T_sky0			);
		C.r_dx10Texture		("sky_s1",			r2_T_sky1			);
		C.r_dx10Texture		("s_occ",			r2_RT_ssao_temp		);
		C.r_dx10Texture		("s_half_depth",	r2_RT_half_depth	);
		C.r_dx10Texture("s_hbao_plus", r2_RT_ssao_temp);

		jitter(C);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 1:	// aa-edge-detection + AA :)
		C.r_Pass("stub_notransform_aa_AA", "combine_2_AA", false, false, false);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_image",			r2_RT_generic0);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Texture		("s_distort",		r2_RT_generic1);

		C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
		C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
		C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 2:	// non-AA
		//	Can use simpler VS (need only Tex0)
		C.r_Pass("stub_notransform_aa_AA", "combine_2_NAA", false, false, false);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_image",			r2_RT_generic0);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Texture		("s_distort",		r2_RT_generic1);
		C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
		C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
		C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 3:	// aa-edge-detection + AA :) + DISTORTION
		C.r_Pass("stub_notransform_aa_AA", "combine_2_AA_D", false, false, false);


		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_image",			r2_RT_generic0);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Texture		("s_distort",		r2_RT_generic1);
		C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
		C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
		C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 4:	// non-AA + DISTORTION
		//	Can use simpler VS (need only Tex0)
		C.r_Pass			("stub_notransform_aa_AA","combine_2_NAA_D",	false,	false,	false);


		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_image",			r2_RT_generic0);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Texture		("s_distort",		r2_RT_generic1);
		C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
		C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
		C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 5:	// post-processing
		break;
	}
}

CBlender_combine_msaa::CBlender_combine_msaa	()	{	description.CLS		= 0;	}
CBlender_combine_msaa::~CBlender_combine_msaa	()	{	}

//	TODO: DX10: Implement CBlender_combine::Compile
void	CBlender_combine_msaa::Compile(CBlender_Compile& C)
{
   IBlender::Compile		(C);

   if( Name )
      ::Render->m_MSAASample = atoi( Definition );
   else
      ::Render->m_MSAASample = -1;

   switch (C.iElement)
   {
   case 0:	// combine
	   C.r_Pass("combine_1", "combine_1_msaa", false, false, false, true, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA);	//. MRT-blend?
	  C.r_Stencil(true, D3D11_COMPARISON_LESS_EQUAL, 0xff, 0x00);	// stencil should be >= 1
      C.r_StencilRef		(0x01);


      C.r_dx10Texture		("s_position",		r2_RT_P				);
      C.r_dx10Texture		("s_normal",		r2_RT_N				);
      C.r_dx10Texture		("s_diffuse",		r2_RT_albedo		);
      C.r_dx10Texture		("s_accumulator",	r2_RT_accum			);
      C.r_dx10Texture		("s_depth",			r2_RT_depth			);
      C.r_dx10Texture		("s_tonemap",		r2_RT_luminance_cur	);
      C.r_dx10Texture		("s_material",		r2_material			);
	  C.r_dx10Texture		("s_occ",			r2_RT_ssao_temp		);
	  C.r_dx10Texture		("s_half_depth",	r2_RT_half_depth	);
      C.r_dx10Texture		("env_s0",			r2_T_envs0			);
      C.r_dx10Texture		("env_s1",			r2_T_envs1			);
      C.r_dx10Texture		("sky_s0",			r2_T_sky0			);
      C.r_dx10Texture		("sky_s1",			r2_T_sky1			);
	  C.r_dx10Texture("s_hbao_plus", r2_RT_ssao_temp);


      jitter(C);

      C.r_dx10Sampler		("smp_nofilter");
      C.r_dx10Sampler		("smp_material");
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 1:	// aa-edge-detection + AA :)
      C.r_Pass			("stub_notransform_aa_AA","combine_2_AA",		false,	false,	false);


      C.r_dx10Texture		("s_position",		r2_RT_P);
      C.r_dx10Texture		("s_normal",		r2_RT_N);
      C.r_dx10Texture		("s_image",			r2_RT_generic0);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Texture		("s_distort",		r2_RT_generic1_r);
	  C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
	  C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
	  C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

      C.r_dx10Sampler		("smp_nofilter");
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 2:	// non-AA
      //	Can use simpler VS (need only Tex0)
      C.r_Pass			("stub_notransform_aa_AA","combine_2_NAA",	false,	false,	false);


      C.r_dx10Texture		("s_position",		r2_RT_P);
      C.r_dx10Texture		("s_normal",		r2_RT_N);
      C.r_dx10Texture		("s_image",			r2_RT_generic0);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Texture		("s_distort",		r2_RT_generic1_r);
	  C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
	  C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
	  C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

      C.r_dx10Sampler		("smp_nofilter");
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 3:	// aa-edge-detection + AA :) + DISTORTION
      C.r_Pass			("stub_notransform_aa_AA","combine_2_AA_D",	false,	false,	false);


      C.r_dx10Texture		("s_position",		r2_RT_P);
      C.r_dx10Texture		("s_normal",		r2_RT_N);
      C.r_dx10Texture		("s_image",			r2_RT_generic0);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Texture		("s_distort",		r2_RT_generic1_r);
	  C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
	  C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
	  C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

      C.r_dx10Sampler		("smp_nofilter");
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 4:	// non-AA + DISTORTION
      //	Can use simpler VS (need only Tex0)
      C.r_Pass			("stub_notransform_aa_AA","combine_2_NAA_D",	false,	false,	false);


      C.r_dx10Texture		("s_position",		r2_RT_P);
      C.r_dx10Texture		("s_normal",		r2_RT_N);
      C.r_dx10Texture		("s_image",			r2_RT_generic0);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Texture		("s_distort",		r2_RT_generic1_r);
	  C.r_dx10Texture("s_blur_2", r2_RT_blur_2);
	  C.r_dx10Texture("s_blur_4", r2_RT_blur_4);
	  C.r_dx10Texture("s_blur_8", r2_RT_blur_8);

      C.r_dx10Sampler		("smp_nofilter");
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 5:	// post-processing
      break;
   }
   ::Render->m_MSAASample = -1;
}    
