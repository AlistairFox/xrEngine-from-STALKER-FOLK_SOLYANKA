#include "stdafx.h"
#pragma hdrstop

#include "Blender_light_occq.h"

CBlender_light_occq::CBlender_light_occq	()	{	description.CLS		= 0;	}
CBlender_light_occq::~CBlender_light_occq	()	{	}
//	TODO: DX10: if nesessary for NV stencil optimisation implement pass 1
void	CBlender_light_occq::Compile(CBlender_Compile& C)
{
	IBlender::Compile	(C);

	switch (C.iElement) 
	{
	case 0:			// occlusion testing
		C.r_Pass	("dumb", "dumb",false,true,false,false);
		C.r_End		();
		//	Color write as well as culling and stencil are set up manually in code.
		break;
	case 1:			// NV40 optimization :)
		C.r_Pass	("stub_notransform_t", "dumb",false,false,false,false);
		C.r_ColorWriteEnable	(false,false,false,false);
		C.r_CullMode(D3D11_CULL_NONE);
		C.r_Stencil(true, D3D11_COMPARISON_LESS_EQUAL, 0xff, 0x00);	// keep/keep/keep
		C.r_End		();
		break;
	case 2:			// Stencil clear in case we've ran out of markers.
		C.r_Pass	("stub_notransform_t", "dumb",false,false,false,false);
		C.r_ColorWriteEnable	(false,false,false,false);
		C.r_CullMode(D3D11_CULL_NONE);
		if( RImplementation.o.dx10_msaa )
			C.r_Stencil(true, D3D11_COMPARISON_ALWAYS, 0x00, 0x7E, D3D11_STENCIL_OP_ZERO, D3D11_STENCIL_OP_ZERO, D3D11_STENCIL_OP_ZERO);
		else
		{
			//	Clear all bits except the last one
			C.r_Stencil(true, D3D11_COMPARISON_ALWAYS, 0x00, 0xFE, D3D11_STENCIL_OP_ZERO, D3D11_STENCIL_OP_ZERO, D3D11_STENCIL_OP_ZERO);
		}
		C.r_End		();
		break;
	}
}
