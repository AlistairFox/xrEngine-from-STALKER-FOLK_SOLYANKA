#include "stdafx.h"
#pragma hdrstop

#include "Blender_Particle.h"

#define					oBlendCount	6

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlender_Particle::CBlender_Particle()
{
	description.CLS		= B_PARTICLE;
	description.version	= 0;
	oBlend.Count		= oBlendCount;
	oBlend.IDselected	= 0;
	oAREF.value			= 32;
	oAREF.min			= 0;
	oAREF.max			= 255;
	oClamp.value		= TRUE;
}

CBlender_Particle::~CBlender_Particle()
{
	
}

void	CBlender_Particle::Save	( IWriter& fs	)
{
	IBlender::Save	(fs);

	// Blend mode
	xrP_TOKEN::Item	I;
	xrPWRITE_PROP	(fs,"Blending",	xrPID_TOKEN,     oBlend);
	I.ID = 0; xr_strcpy(I.str,"SET");			fs.w		(&I,sizeof(I));
	I.ID = 1; xr_strcpy(I.str,"BLEND");		fs.w		(&I,sizeof(I));
	I.ID = 2; xr_strcpy(I.str,"ADD");			fs.w		(&I,sizeof(I));
	I.ID = 3; xr_strcpy(I.str,"MUL");			fs.w		(&I,sizeof(I));
	I.ID = 4; xr_strcpy(I.str,"MUL_2X");		fs.w		(&I,sizeof(I));
	I.ID = 5; xr_strcpy(I.str,"ALPHA-ADD");	fs.w		(&I,sizeof(I));
	
	// Params
	xrPWRITE_PROP		(fs,"Texture clamp",xrPID_BOOL,		oClamp);
	xrPWRITE_PROP		(fs,"Alpha ref",	xrPID_INTEGER,	oAREF);
}

void	CBlender_Particle::Load	( IReader& fs, u16 version)
{
	IBlender::Load		(fs,version);

	xrPREAD_PROP		(fs,xrPID_TOKEN,		oBlend);	oBlend.Count =   oBlendCount;
	xrPREAD_PROP		(fs,xrPID_BOOL,			oClamp);
	xrPREAD_PROP		(fs,xrPID_INTEGER,		oAREF);
}


void	CBlender_Particle::Compile	(CBlender_Compile& C)
{
	IBlender::Compile		(C);
	switch	(C.iElement) 
	{
	case SE_R2_NORMAL_HQ: 	// deffer
	case SE_R2_NORMAL_LQ: 	// deffer
		switch (oBlend.IDselected)
		{
		case 0:	C.r_Pass("deffer_particle", "deffer_particle", false, true, true, true, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, false, 200);	break;	// SET
		case 1: C.r_Pass("particle", "particle", false, true, false, true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, true, 0);	break;	// BLEND
		case 2:	C.r_Pass("particle", "particle", false, true, false, true, D3D11_BLEND_ONE, D3D11_BLEND_ONE, true, 0);	break;	// ADD
		case 3:	C.r_Pass("particle", "particle", false, true, false, true, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, true, 0);	break;	// MUL
		case 4:	C.r_Pass("particle", "particle", false, true, false, true, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, true, 0);	break;	// MUL_2X
		case 5:	C.r_Pass("particle", "particle", false, true, false, true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, true, 0);	break;	// ALPHA-ADD
		};
		{
			C.r_dx10Texture		("s_base",	C.L_textures[0]);
			u32 hSampler = 	C.r_dx10Sampler("smp_base");
			if (oClamp.value&&(hSampler!=(u32)-1)) 
				C.i_dx10Address( hSampler, D3DTADDRESS_CLAMP);		
			C.r_dx10Texture		("s_position",	"$user$position");
			C.r_dx10Sampler		("smp_nofilter");
		}
		C.r_End				();
		break;
	case SE_R2_SHADOW:		// smap
		// HARD or SOFT: shadow-map
		switch (oBlend.IDselected)
		{
		case 0:	C.r_Pass("particle", "particle", false, true, true, false, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, true, 200);
			C.r_ColorWriteEnable(false, false, false, false);
			break;	// SET
		case 1: C.r_Pass("particle-clip", "particle_s-blend", false, true, false, true, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, true, 0);	break;	// BLEND
		case 2:	C.r_Pass("particle-clip", "particle_s-add", false, true, false, true, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, true, 0);	break;	// ADD
		case 3:	C.r_Pass("particle-clip", "particle_s-mul", false, true, false, true, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, true, 0);	break;	// MUL
		case 4:	C.r_Pass("particle-clip", "particle_s-mul", false, true, false, true, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, true, 0);	break;	// MUL_2X
		case 5:	C.r_Pass("particle-clip", "particle_s-aadd", false, true, false, true, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, true, 0);	break;	// ALPHA-ADD
		};
		{
			C.r_dx10Texture		("s_base",	C.L_textures[0]);
			u32 hSampler = 	C.r_dx10Sampler("smp_base");
			if (oClamp.value&&(hSampler!=(u32)-1)) 
				C.i_dx10Address( hSampler, D3DTADDRESS_CLAMP);		
			C.r_dx10Texture		("s_position",	"$user$position");
			C.r_dx10Sampler		("smp_nofilter");
		}
		C.r_End				();
		break;
	case 4: 	// deffer-EMAP
		break;
	};
}
