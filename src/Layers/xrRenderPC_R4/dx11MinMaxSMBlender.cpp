#include "stdafx.h"
#include "dx11MinMaxSMBlender.h"

void CBlender_createminmax::Compile(CBlender_Compile& C)
{
	IBlender::Compile	(C);

	switch (C.iElement) 
	{
   case 0:
		C.r_Pass	("stub_notransform_2uv", "create_minmax_sm", false,	false,	false, false);
		C.PassSET_ZB		(false,false,false	);

		C.r_dx10Texture		("s_smap",		r2_RT_smap_depth);

		C.r_dx10Sampler		("smp_nofilter");

		C.r_End				();

		break;
	}

}