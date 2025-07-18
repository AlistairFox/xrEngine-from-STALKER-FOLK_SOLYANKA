#include "stdafx.h"
#include "../xrRender/resourcemanager.h"
#include "blender_light_occq.h"
#include "blender_light_mask.h"
#include "blender_light_direct.h"
#include "blender_light_point.h"
#include "blender_light_spot.h"
#include "blender_light_reflected.h"
#include "blender_combine.h"
#include "blender_bloom_build.h"
#include "blender_luminance.h"
#include "blender_ssao.h"
#include "dx11MinMaxSMBlender.h"
#include "dx11HDAOCSBlender.h"
#include "../xrRenderDX10/msaa/dx10MSAABlender.h"
#include "../xrRenderDX10/DX10 Rain/dx10RainBlender.h"
#include "blender_hud_blood.h"
#include "blender_hud_stamina.h"
#include "blender_hud_bleeding.h"
#include "blender_hud_satiety.h"
#include "blender_hud_thirst.h"
#include "blender_lens_flares.h"
#include "blender_ss_sunshafts.h"
#include "blender_blur.h"

#include "../xrRender/dxRenderDeviceRender.h"

#include <dxsdk/D3DX10Tex.h>
#include "blender_gasmask_drops.h"
#include "blender_gasmask_dudv.h"


void	CRenderTarget::u_setrt			(const ref_rt& _1, const ref_rt& _2, const ref_rt& _3, ID3DDepthStencilView* zb)
{
	VERIFY									(_1||zb);
	if (_1)
	{
		dwWidth = _1->dwWidth;
		dwHeight = _1->dwHeight;
	}
	else
	{
		D3D_DEPTH_STENCIL_VIEW_DESC	desc;
		zb->GetDesc(&desc);

      if( !RImplementation.o.dx10_msaa )
         VERIFY(desc.ViewDimension==D3D_DSV_DIMENSION_TEXTURE2D);

		ID3DResource *pRes;

		zb->GetResource( &pRes);

		ID3DTexture2D *pTex = (ID3DTexture2D *)pRes;

		D3D_TEXTURE2D_DESC	TexDesc;

		pTex->GetDesc(&TexDesc);

		dwWidth = TexDesc.Width;
		dwHeight = TexDesc.Height;
		_RELEASE( pRes );
	}

	if (_1) RCache.set_RT(_1->pRT,	0); else RCache.set_RT(NULL,0);
	if (_2) RCache.set_RT(_2->pRT,	1); else RCache.set_RT(NULL,1);
	if (_3) RCache.set_RT(_3->pRT,	2); else RCache.set_RT(NULL,2);
	RCache.set_ZB							(zb);
//	RImplementation.rmNormal				();
}

D3D_VIEWPORT custom_viewport[1] = { 0, 0, 0, 0, 0.f, 1.f };

void CRenderTarget::set_viewport_size(ID3DDeviceContext* dev, float w, float h)
{
	custom_viewport[0].Width = w;
	custom_viewport[0].Height = h;
	dev->RSSetViewports(1, custom_viewport);
}

void	CRenderTarget::u_setrt			(const ref_rt& _1, const ref_rt& _2, ID3DDepthStencilView* zb)
{
	VERIFY									(_1||zb);
	if (_1)
	{
		dwWidth = _1->dwWidth;
		dwHeight = _1->dwHeight;
	}
	else
	{
		D3D_DEPTH_STENCIL_VIEW_DESC	desc;
		zb->GetDesc(&desc);
      if( ! RImplementation.o.dx10_msaa )
		   VERIFY(desc.ViewDimension==D3D_DSV_DIMENSION_TEXTURE2D);

		ID3DResource *pRes;

		zb->GetResource( &pRes);

		ID3DTexture2D *pTex = (ID3DTexture2D *)pRes;

		D3D_TEXTURE2D_DESC	TexDesc;

		pTex->GetDesc(&TexDesc);

		dwWidth = TexDesc.Width;
		dwHeight = TexDesc.Height;
		_RELEASE( pRes );
	}

	if (_1) RCache.set_RT(_1->pRT,	0); else RCache.set_RT(NULL,0);
	if (_2) RCache.set_RT(_2->pRT,	1); else RCache.set_RT(NULL,1);
	RCache.set_ZB							(zb);
//	RImplementation.rmNormal				();
}

void	CRenderTarget::u_setrt			(u32 W, u32 H, ID3DRenderTargetView* _1, ID3DRenderTargetView* _2, ID3DRenderTargetView* _3, ID3DDepthStencilView* zb)
{
	//VERIFY									(_1);
	dwWidth									= W;
	dwHeight								= H;
	//VERIFY									(_1);
	RCache.set_RT							(_1,	0);
	RCache.set_RT							(_2,	1);
	RCache.set_RT							(_3,	2);
	RCache.set_ZB							(zb);
//	RImplementation.rmNormal				();
}

void	CRenderTarget::u_stencil_optimize	(eStencilOptimizeMode eSOM)
{
	//	TODO: DX10: remove half pixel offset?
	VERIFY	(RImplementation.o.nvstencil);
	//RCache.set_ColorWriteEnable	(FALSE);
	u32		Offset;
	float	_w					= float(Device.dwWidth);
	float	_h					= float(Device.dwHeight);
	u32		C					= color_rgba	(255,255,255,255);
	float	eps					= 0;
	float	_dw					= 0.5f;
	float	_dh					= 0.5f;
	FVF::TL* pv					= (FVF::TL*) RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
	pv->set						(-_dw,		_h-_dh,		eps,	1.f, C, 0, 0);	pv++;
	pv->set						(-_dw,		-_dh,		eps,	1.f, C, 0, 0);	pv++;
	pv->set						(_w-_dw,	_h-_dh,		eps,	1.f, C, 0, 0);	pv++;
	pv->set						(_w-_dw,	-_dh,		eps,	1.f, C, 0, 0);	pv++;
	RCache.Vertex.Unlock		(4,g_combine->vb_stride);
	RCache.set_Element			(s_occq->E[1]	);

	switch(eSOM)
	{
	case SO_Light:
		StateManager.SetStencilRef(dwLightMarkerID);
		break;
	case SO_Combine:
		StateManager.SetStencilRef(0x01);
		break;
	default:
		VERIFY(!"CRenderTarget::u_stencil_optimize. switch no default!");
	}	

	RCache.set_Geometry			(g_combine		);
	RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	
}

// 2D texgen (texture adjustment matrix)
void	CRenderTarget::u_compute_texgen_screen	(Fmatrix& m_Texgen)
{
	//float	_w						= float(Device.dwWidth);
	//float	_h						= float(Device.dwHeight);
	//float	o_w						= (.5f / _w);
	//float	o_h						= (.5f / _h);
	Fmatrix			m_TexelAdjust		= 
	{
		0.5f,				0.0f,				0.0f,			0.0f,
		0.0f,				-0.5f,				0.0f,			0.0f,
		0.0f,				0.0f,				1.0f,			0.0f,
		//	Removing half pixel offset
		//0.5f + o_w,			0.5f + o_h,			0.0f,			1.0f
		0.5f,				0.5f ,				0.0f,			1.0f
	};
	m_Texgen.mul	(m_TexelAdjust,RCache.xforms.m_wvp);
}

// 2D texgen for jitter (texture adjustment matrix)
void	CRenderTarget::u_compute_texgen_jitter	(Fmatrix&		m_Texgen_J)
{
	// place into	0..1 space
	Fmatrix			m_TexelAdjust		= 
	{
		0.5f,				0.0f,				0.0f,			0.0f,
		0.0f,				-0.5f,				0.0f,			0.0f,
		0.0f,				0.0f,				1.0f,			0.0f,
		0.5f,				0.5f,				0.0f,			1.0f
	};
	m_Texgen_J.mul	(m_TexelAdjust,RCache.xforms.m_wvp);

	// rescale - tile it
	float	scale_X			= float(Device.dwWidth)	/ float(TEX_jitter);
	float	scale_Y			= float(Device.dwHeight)/ float(TEX_jitter);
	//float	offset			= (.5f / float(TEX_jitter));
	m_TexelAdjust.scale			(scale_X,	scale_Y,1.f	);
	//m_TexelAdjust.translate_over(offset,	offset,	0	);
	m_Texgen_J.mulA_44			(m_TexelAdjust);
}

u8		fpack			(float v)				{
	s32	_v	= iFloor	(((v+1)*.5f)*255.f + .5f);
	clamp	(_v,0,255);
	return	u8(_v);
}
u8		fpackZ			(float v)				{
	s32	_v	= iFloor	(_abs(v)*255.f + .5f);
	clamp	(_v,0,255);
	return	u8(_v);
}
Fvector	vunpack			(s32 x, s32 y, s32 z)	{
	Fvector	pck;
	pck.x	= (float(x)/255.f - .5f)*2.f;
	pck.y	= (float(y)/255.f - .5f)*2.f;
	pck.z	= -float(z)/255.f;
	return	pck;
}
Fvector	vunpack			(Ivector src)			{
	return	vunpack	(src.x,src.y,src.z);
}
Ivector	vpack			(Fvector src)
{
	Fvector			_v;
	int	bx			= fpack	(src.x);
	int by			= fpack	(src.y);
	int bz			= fpackZ(src.z);
	// dumb test
	float	e_best	= flt_max;
	int		r=bx,g=by,b=bz;
#ifdef DEBUG
	int		d=0;
#else
	int		d=3;
#endif
	for (int x=_max(bx-d,0); x<=_min(bx+d,255); x++)
	for (int y=_max(by-d,0); y<=_min(by+d,255); y++)
	for (int z=_max(bz-d,0); z<=_min(bz+d,255); z++)
	{
		_v				= vunpack(x,y,z);
		float	m		= _v.magnitude();
		float	me		= _abs(m-1.f);
		if	(me>0.03f)	continue;
		_v.div	(m);
		float	e		= _abs(src.dotproduct(_v)-1.f);
		if (e<e_best)	{
			e_best		= e;
			r=x,g=y,b=z;
		}
	}
	Ivector		ipck;
	ipck.set	(r,g,b);
	return		ipck;
}

void	generate_jitter	(DWORD*	dest, u32 elem_count)
{
	const	int		cmax		= 8;
	svector<Ivector2,cmax>		samples;
	while (samples.size()<elem_count*2)
	{
		Ivector2	test;
		test.set	(::Random.randI(0,256),::Random.randI(0,256));
		BOOL		valid = TRUE;
		for (u32 t=0; t<samples.size(); t++)
		{
			int		dist	= _abs(test.x-samples[t].x)+_abs(test.y-samples[t].y);
			if (dist<32)	{
				valid		= FALSE;
				break;
			}
		}
		if (valid)	samples.push_back	(test);
	}
	for	(u32 it=0; it<elem_count; it++, dest++)
		*dest	= color_rgba(samples[2*it].x,samples[2*it].y,samples[2*it+1].y,samples[2*it+1].x);
}

CRenderTarget::CRenderTarget		()
{
   u32 SampleCount = 1;

   if (ps_r_ssao_mode!=2/*hdao*/)
	   ps_r_ssao = _min(ps_r_ssao, 3);

	RImplementation.o.ssao_ultra		= ps_r_ssao>3;
   if( RImplementation.o.dx10_msaa )
      SampleCount    = RImplementation.o.dx10_msaa_samples;

#ifdef DEBUG
	Msg			("MSAA samples = %d", SampleCount );
	if( RImplementation.o.dx10_msaa_opt )
		Msg		("dx10_MSAA_opt = on" );
	if( RImplementation.o.dx10_gbuffer_opt )
		Msg		("dx10_gbuffer_opt = on" );
#endif // DEBUG
	param_blur			= 0.f;
	param_gray			= 0.f;
	param_noise			= 0.f;
	param_duality_h		= 0.f;
	param_duality_v		= 0.f;
	param_noise_fps		= 25.f;
	param_noise_scale	= 1.f;

	im_noise_time		= 1/100;
	im_noise_shift_w	= 0;
	im_noise_shift_h	= 0;

	param_color_base	= color_rgba(127,127,127,	0);
	param_color_gray	= color_rgba(85,85,85,		0);
	//param_color_add		= color_rgba(0,0,0,			0);
	param_color_add.set( 0.0f, 0.0f, 0.0f );

	needClearAccumulator = true;
	dxRenderDeviceRender::Instance().Resources->Evict			();

	// Blenders
	b_occq					= xr_new<CBlender_light_occq>			();
	b_accum_mask			= xr_new<CBlender_accum_direct_mask>	();
	b_accum_direct			= xr_new<CBlender_accum_direct>			();
	b_accum_point			= xr_new<CBlender_accum_point>			();
	b_accum_spot			= xr_new<CBlender_accum_spot>			();
	b_accum_reflected		= xr_new<CBlender_accum_reflected>		();
	b_bloom					= xr_new<CBlender_bloom_build>			();
	if( RImplementation.o.dx10_msaa )
	{
		b_bloom_msaa			= xr_new<CBlender_bloom_build_msaa>		();
		b_postprocess_msaa	= xr_new<CBlender_postprocess_msaa>	();
	}
	b_luminance				= xr_new<CBlender_luminance>		();
	b_combine				= xr_new<CBlender_combine>			();
	b_ssao					= xr_new<CBlender_SSAO_noMSAA>		();
	b_sunshafts = new CBlender_sunshafts();
	b_blur = xr_new<CBlender_blur>();

	//HUD BLOOD
	b_hud_blood = xr_new<CBlender_Hud_Blood>();

	//HUD STAMINA
	b_hud_power = xr_new<CBlender_Hud_Stamina>();

	//HUD BLEEDING
	b_hud_bleeding = xr_new<CBlender_Hud_Bleeding>();

	//Hud Satiety
	b_hud_satiety = xr_new<CBlender_Hud_Satiety>();

	//hud thirst
	b_hud_thirst = xr_new<CBlender_Hud_Thirst>();

	//SFZ Lens Flares
	b_lfx = xr_new<CBlender_LFX>();

	// Screen Space Shaders Stuff
	b_ssfx_ssr = xr_new<CBlender_ssfx_ssr>(); // [Ascii1457] SSS new Phase
	b_ssfx_volumetric_blur = xr_new<CBlender_ssfx_volumetric_blur>(); // [Ascii1457] SSS new Phase

	b_gasmask_drops = xr_new<CBlender_gasmask_drops>();
	b_gasmask_dudv = xr_new<CBlender_gasmask_dudv>();

	// HDAO
	b_hdao_cs               = xr_new<CBlender_CS_HDAO>			();
	if( RImplementation.o.dx10_msaa )
	{
		b_hdao_msaa_cs      = xr_new<CBlender_CS_HDAO_MSAA>     ();
	}

	if( RImplementation.o.dx10_msaa )
	{
		int bound = RImplementation.o.dx10_msaa_samples;

		if( RImplementation.o.dx10_msaa_opt )
			bound = 1;

		for( int i = 0; i < bound; ++i )
		{
			static LPCSTR SampleDefs[] = { "0","1","2","3","4","5","6","7" };
			b_combine_msaa[i]							= xr_new<CBlender_combine_msaa>					();
			b_accum_mask_msaa[i]						= xr_new<CBlender_accum_direct_mask_msaa>		();
			b_accum_direct_msaa[i]					= xr_new<CBlender_accum_direct_msaa>			();
			b_accum_direct_volumetric_msaa[i]			= xr_new<CBlender_accum_direct_volumetric_msaa>	();
			//b_accum_direct_volumetric_sun_msaa[i]	= xr_new<CBlender_accum_direct_volumetric_sun_msaa>			();
			b_accum_spot_msaa[i]		 				= xr_new<CBlender_accum_spot_msaa>			();
			b_accum_volumetric_msaa[i]				= xr_new<CBlender_accum_volumetric_msaa>	();
			b_accum_point_msaa[i]						= xr_new<CBlender_accum_point_msaa>			();
			b_accum_reflected_msaa[i]					= xr_new<CBlender_accum_reflected_msaa>		();
			b_ssao_msaa[i]							= xr_new<CBlender_SSAO_MSAA>				();
			static_cast<CBlender_accum_direct_mask_msaa*>( b_accum_mask_msaa[i] )->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_accum_direct_volumetric_msaa*>(b_accum_direct_volumetric_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			//static_cast<CBlender_accum_direct_volumetric_sun_msaa*>(b_accum_direct_volumetric_sun_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_accum_direct_msaa*>(b_accum_direct_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_accum_volumetric_msaa*>(b_accum_volumetric_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_accum_spot_msaa*>(b_accum_spot_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_accum_point_msaa*>(b_accum_point_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_accum_reflected_msaa*>(b_accum_reflected_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_combine_msaa*>(b_combine_msaa[i])->SetDefine( "ISAMPLE", SampleDefs[i]);
			static_cast<CBlender_SSAO_MSAA*>(b_ssao_msaa[i])->SetDefine("ISAMPLE", SampleDefs[i]);
		}
	}
	//	NORMAL
	{
		u32		w=Device.dwWidth, h=Device.dwHeight;

		rt_Position.create(r2_RT_P, w,h, DXGI_FORMAT_R16G16B16A16_FLOAT, SRV_RTV, SampleCount);

		if( RImplementation.o.dx10_msaa )
			rt_MSAADepth.create(r2_RT_MSAAdepth, w,h, DXGI_FORMAT_D24_UNORM_S8_UINT, SRV_DSV, SampleCount);

		if( !RImplementation.o.dx10_gbuffer_opt )
			rt_Normal.create(r2_RT_N, w,h, DXGI_FORMAT_R16G16B16A16_FLOAT, SRV_RTV, SampleCount);

		// select albedo & accum
		//if (RImplementation.o.mrtmixdepth)	
		{
			// NV50
			rt_Color.create(r2_RT_albedo, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, SampleCount);
			rt_Accumulator.create(r2_RT_accum, w,h,  DXGI_FORMAT_R16G16B16A16_FLOAT, SRV_RTV, SampleCount);
		}
		/*else
		{
			// can't - mix-depth
			if (RImplementation.o.fp16_blend) {
				// NV40
				if( !RImplementation.o.dx10_gbuffer_opt )
				{
					rt_Color.create(r2_RT_albedo, vp_params_main_secondary, D3DFMT_A16B16G16R16F, SampleCount);	// expand to full
					rt_Accumulator.create(r2_RT_accum, vp_params_main_secondary, D3DFMT_A16B16G16R16F, SampleCount);
				}
				else
				{
					rt_Color.create(r2_RT_albedo, vp_params_main_secondary, D3DFMT_A8R8G8B8, SampleCount);	// expand to full
					rt_Accumulator.create(r2_RT_accum, vp_params_main_secondary, D3DFMT_A16B16G16R16F, SampleCount);
				}
			} else {
				// R4xx, no-fp-blend,-> albedo_wo
				VERIFY						(RImplementation.o.albedo_wo);
				rt_Color.create(r2_RT_albedo, vp_params_main_secondary, D3DFMT_A8R8G8B8, SampleCount);	// normal
				rt_Accumulator.create(r2_RT_accum, vp_params_main_secondary, D3DFMT_A16B16G16R16F, SampleCount);
				rt_Accumulator_temp.create(r2_RT_accum_temp, vp_params_main_secondary, D3DFMT_A16B16G16R16F, SampleCount);
			}
		}*/

		// generic(LDR) RTs
		rt_Generic_0.create(r2_RT_generic0, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);
		rt_Generic_1.create(r2_RT_generic1, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);
		rt_ui_pda.create(r2_RT_ui, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);

		// RT Blur
		rt_blur_h_2.create(r2_RT_blur_h_2, w/2,h/2, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);
		rt_blur_2.create(r2_RT_blur_2, w/2,h/2, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);

		rt_blur_h_4.create(r2_RT_blur_h_4, w/4,h/4, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);
		rt_blur_4.create(r2_RT_blur_4, w/4,h/4, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);

		rt_blur_h_8.create(r2_RT_blur_h_8, w/8,h/8, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);
		rt_blur_8.create(r2_RT_blur_8, w/8,h/8, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);

		// Screen Space Shaders Stuff
		rt_ssfx.create(r2_RT_ssfx, w,h, DXGI_FORMAT_B8G8R8A8_UNORM, SRV_RTV,1); // Generic RT
		rt_ssfx_temp.create(r2_RT_ssfx_temp, w,h, DXGI_FORMAT_B8G8R8A8_UNORM, SRV_RTV, 1); // Temp RT
		rt_ssfx_temp2.create(r2_RT_ssfx_temp2, w,h, DXGI_FORMAT_B8G8R8A8_UNORM, SRV_RTV, 1); // Temp RT 8B
		rt_ssfx_accum.create(r2_RT_ssfx_accum, w,h, DXGI_FORMAT_R16G16B16A16_FLOAT,SRV_RTV, SampleCount); // Temp RT 16B

		rt_ssfx_hud.create(r2_RT_ssfx_hud, w,h, DXGI_FORMAT_R8_UNORM, SRV_RTV, 1); // Temp RT 8B

		if (RImplementation.o.dx10_msaa)
			rt_Generic_temp.create("$user$generic_temp", w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, SampleCount);
		else
			rt_Generic_temp.create("$user$generic_temp", w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);

		if( RImplementation.o.dx10_msaa )
		{
			rt_Generic_0_r.create(r2_RT_generic0_r, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, SampleCount);
			rt_Generic_1_r.create(r2_RT_generic1_r, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, SampleCount);
			rt_Generic.create(r2_RT_generic, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);
		}

		// RT - KD
		rt_sunshafts_0.create(r2_RT_sunshafts0, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);
		rt_sunshafts_1.create(r2_RT_sunshafts1, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);

		rt_Generic.create(r2_RT_generic, w,h, DXGI_FORMAT_R8G8B8A8_UNORM, SRV_RTV, 1);

		rt_Generic_2.create			(r2_RT_generic2, w,h, DXGI_FORMAT_R16G16B16A16_FLOAT, SRV_RTV, SampleCount );
	}

	s_sunshafts.create(b_sunshafts, "r2\\sunshafts");

	s_blur.create(b_blur, "r2\\blur");
	// OCCLUSION
	s_occq.create					(b_occq,		"r2\\occq");

	// Screen Space Shaders Stuff
	s_ssfx_ssr.create(b_ssfx_ssr, "r2\\ssfx_ssr"); // SSR
	s_ssfx_volumetric_blur.create(b_ssfx_volumetric_blur, "r2\\ssfx_volumetric_blur"); // Volumetric Blur

	//Hud Blood
	s_hud_blood.create(b_hud_blood, "r3\\hud_blood");

	//Hud Stamina
	s_hud_power.create(b_hud_power, "r3\\hud_power");

	//Hud Bleeding
	s_hud_bleeding.create(b_hud_bleeding, "r3\\hud_bleeding");

	//hud satiety
	s_hud_satiety.create(b_hud_satiety, "r3\\hud_power");

	//hud thirst
	s_hud_thirst.create(b_hud_thirst, "r3\\hud_power");

	//SFZ Lens Flares
	s_lfx.create(b_lfx, "r3\\lfx");
	g_lfx.create(FVF::F_V, RCache.Vertex.Buffer(), RCache.QuadIB);

	s_gasmask_drops.create(b_gasmask_drops, "r2\\gasmask_drops");
	s_gasmask_dudv.create(b_gasmask_dudv, "r2\\gasmask_dudv");

	// DIRECT (spot)
	DXGI_FORMAT						depth_format	= (DXGI_FORMAT)RImplementation.o.HW_smap_FORMAT;

	if (RImplementation.o.HW_smap)
	{

		u32	size					=RImplementation.o.smapsize	;
		rt_smap_depth.create(r2_RT_smap_depth, size, size, depth_format, SRV_DSV, 1);

		if (RImplementation.o.dx10_minmax_sm)
		{
			rt_smap_depth_minmax.create(r2_RT_smap_depth_minmax, size/4, size/4, DXGI_FORMAT_D32_FLOAT, SRV_DSV, 1);
			CBlender_createminmax TempBlender;
			s_create_minmax_sm.create( &TempBlender, "null" );
		}

		//rt_smap_surf.create			(r2_RT_smap_surf,			size,size,nullrt		);
		//rt_smap_ZB					= NULL;
		s_accum_mask.create			(b_accum_mask,				"r3\\accum_mask");
		s_accum_direct.create		(b_accum_direct,			"r3\\accum_direct");


		if( RImplementation.o.dx10_msaa )
		{
			int bound = RImplementation.o.dx10_msaa_samples;

			if( RImplementation.o.dx10_msaa_opt )
				bound = 1;

			for( int i = 0; i < bound; ++i )
			{
				s_accum_direct_msaa[i].create		(b_accum_direct_msaa[i],			"r3\\accum_direct");
				s_accum_mask_msaa[i].create		(b_accum_mask_msaa[i],			"r3\\accum_direct");
			}
		}

		s_accum_direct_volumetric.create("accum_volumetric_sun_nomsaa");

		if (RImplementation.o.dx10_minmax_sm)
			s_accum_direct_volumetric_minmax.create("accum_volumetric_sun_nomsaa_minmax");

		if( RImplementation.o.dx10_msaa )
		{
			static LPCSTR snames[] = { "accum_volumetric_sun_msaa0",
				"accum_volumetric_sun_msaa1",
				"accum_volumetric_sun_msaa2",
				"accum_volumetric_sun_msaa3",
				"accum_volumetric_sun_msaa4",
				"accum_volumetric_sun_msaa5",
				"accum_volumetric_sun_msaa6",
				"accum_volumetric_sun_msaa7" };
			int bound = RImplementation.o.dx10_msaa_samples;

			if( RImplementation.o.dx10_msaa_opt )
				bound = 1;

			for( int i = 0; i < bound; ++i )
			{
				//s_accum_direct_volumetric_msaa[i].create		(b_accum_direct_volumetric_sun_msaa[i],			"r3\\accum_direct");
				s_accum_direct_volumetric_msaa[i].create		(snames[i]);
			}
		}

	}
	else
	{
		VERIFY(!"Use HW SMAPs only!");
	}

	//	RAIN
	//	TODO: DX10: Create resources only when DX10 rain is enabled.
	//	Or make DX10 rain switch dynamic?
	{
		CBlender_rain	TempBlender;
		s_rain.create( &TempBlender, "null");

		if( RImplementation.o.dx10_msaa )
		{
			static LPCSTR SampleDefs[] = { "0","1","2","3","4","5","6","7" };
			CBlender_rain_msaa	TempBlender[8];

			int bound = RImplementation.o.dx10_msaa_samples;

			if( RImplementation.o.dx10_msaa_opt )
				bound = 1;

			for( int i = 0; i < bound; ++i )
			{
				TempBlender[i].SetDefine( "ISAMPLE", SampleDefs[i] );
				s_rain_msaa[i].create( &TempBlender[i], "null");
				s_accum_spot_msaa[i].create			(b_accum_spot_msaa[i],				"r2\\accum_spot_s",	"lights\\lights_spot01");
				s_accum_point_msaa[i].create		(b_accum_point_msaa[i],			"r2\\accum_point_s");
				//s_accum_volume_msaa[i].create(b_accum_direct_volumetric_msaa[i], "lights\\lights_spot01");
				s_accum_volume_msaa[i].create(b_accum_volumetric_msaa[i], "lights\\lights_spot01");
				s_combine_msaa[i].create					(b_combine_msaa[i],					"r2\\combine");
			}
		}
	}

	if( RImplementation.o.dx10_msaa )
	{
		CBlender_msaa TempBlender;

		s_mark_msaa_edges.create( &TempBlender, "null" );
	}

	// POINT
	{
		s_accum_point.create		(b_accum_point,				"r2\\accum_point_s");
		accum_point_geom_create		();
		g_accum_point.create		(D3DFVF_XYZ,				g_accum_point_vb, g_accum_point_ib);
		accum_omnip_geom_create		();
		g_accum_omnipart.create		(D3DFVF_XYZ,				g_accum_omnip_vb, g_accum_omnip_ib);
	}

	// SPOT
	{
		s_accum_spot.create			(b_accum_spot,				"r2\\accum_spot_s",	"lights\\lights_spot01");
		accum_spot_geom_create		();
		g_accum_spot.create			(D3DFVF_XYZ,				g_accum_spot_vb, g_accum_spot_ib);
	}

	{
		s_accum_volume.create("accum_volumetric", "lights\\lights_spot01");
		accum_volumetric_geom_create();
		g_accum_volumetric.create( D3DFVF_XYZ, g_accum_volumetric_vb, g_accum_volumetric_ib);
	}


	// REFLECTED
	{
		s_accum_reflected.create	(b_accum_reflected,			"r2\\accum_refl");
		if( RImplementation.o.dx10_msaa )
		{
			int bound = RImplementation.o.dx10_msaa_samples;

			if( RImplementation.o.dx10_msaa_opt )
				bound = 1;

			for( int i = 0; i < bound; ++i )
			{
				s_accum_reflected_msaa[i].create( b_accum_reflected_msaa[i], "null");
			}
		}
	}


	// BLOOM
	{
		DXGI_FORMAT	fmt				= DXGI_FORMAT_R8G8B8A8_UNORM;			//;		// D3DFMT_X8R8G8B8
		u32	w=BLOOM_size_X, h=BLOOM_size_Y;
		u32 fvf_build				= D3DFVF_XYZRHW|D3DFVF_TEX4|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1)|D3DFVF_TEXCOORDSIZE2(2)|D3DFVF_TEXCOORDSIZE2(3);
		u32 fvf_filter				= (u32)D3DFVF_XYZRHW|D3DFVF_TEX8|D3DFVF_TEXCOORDSIZE4(0)|D3DFVF_TEXCOORDSIZE4(1)|D3DFVF_TEXCOORDSIZE4(2)|D3DFVF_TEXCOORDSIZE4(3)|D3DFVF_TEXCOORDSIZE4(4)|D3DFVF_TEXCOORDSIZE4(5)|D3DFVF_TEXCOORDSIZE4(6)|D3DFVF_TEXCOORDSIZE4(7);
		rt_Bloom_1.create(r2_RT_bloom1, w,h, fmt, SRV_RTV, 1);
		rt_Bloom_2.create(r2_RT_bloom2, w, h, fmt, SRV_RTV, 1);
		g_bloom_build.create		(fvf_build,		RCache.Vertex.Buffer(), RCache.QuadIB);
		g_bloom_filter.create		(fvf_filter,	RCache.Vertex.Buffer(), RCache.QuadIB);
		s_bloom_dbg_1.create		("effects\\screen_set",		r2_RT_bloom1);
		s_bloom_dbg_2.create		("effects\\screen_set",		r2_RT_bloom2);
		s_bloom.create				(b_bloom,					"r2\\bloom");
		if( RImplementation.o.dx10_msaa )
		{
			s_bloom_msaa.create		 (b_bloom_msaa,					"r2\\bloom");
			s_postprocess_msaa.create(b_postprocess_msaa,			"r2\\post");
		}
		f_bloom_factor				= 0.5f;
	}


	// TONEMAP
	{
		rt_LUM_64.create(r2_RT_luminance_t64, 64, 64, DXGI_FORMAT_R16G16B16A16_FLOAT, SRV_RTV, 1);
		rt_LUM_8.create(r2_RT_luminance_t8, 8, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, SRV_RTV, 1);
		s_luminance.create			(b_luminance,				"r2\\luminance");
		f_luminance_adapt			= 0.5f;

		t_LUM_src.create			(r2_RT_luminance_src);
		t_LUM_dest.create			(r2_RT_luminance_cur);

		// create pool
		for (u32 it=0; it<HW.Caps.iGPUNum*2; it++)	{
			string256					name;
			xr_sprintf						(name,"%s_%d",	r2_RT_luminance_pool,it	);
			rt_LUM_pool[it].create(name, 1, 1, DXGI_FORMAT_R32_FLOAT, SRV_RTV, 1);
			//u_setrt						(rt_LUM_pool[it],	0,	0,	0			);
			//CHK_DX						(HW.pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET,	0x7f7f7f7f,	1.0f, 0L));
			FLOAT ColorRGBA[4] = { 127.0f/255.0f, 127.0f/255.0f, 127.0f/255.0f, 127.0f/255.0f};
			HW.pContext->ClearRenderTargetView(rt_LUM_pool[it]->pRT, ColorRGBA);
		}
		u_setrt						( Device.dwWidth,Device.dwHeight,HW.pBaseRT,NULL,NULL,HW.pBaseZB);
	}

	// HBAO
	if (RImplementation.o.hbao_plus)
	{
		s_ssao.create(b_ssao, "r2\\ssao");
	}
	else
	if (RImplementation.o.ssao_opt_data)
	{
		u32		w = 0;
		u32		h = 0;
		if (RImplementation.o.ssao_half_data)
		{
			w = Device.dwWidth / 2;
			h = Device.dwHeight / 2;
		}
		else
		{
			w = Device.dwWidth;
			h = Device.dwHeight;
		}

		DXGI_FORMAT	fmt = DXGI_FORMAT_D32_FLOAT;
		rt_half_depth.create(r2_RT_half_depth, w, h, fmt, SRV_DSV, 1);

		s_ssao.create				(b_ssao, "r2\\ssao");
	}

	//if (RImplementation.o.ssao_blur_on)
	//{
	//	u32		w = Device.dwWidth, h = Device.dwHeight;
	//	rt_ssao_temp.create			(r2_RT_ssao_temp, w, h, D3DFMT_G16R16F, SampleCount);
	//	s_ssao.create				(b_ssao, "r2\\ssao");

	//	if( RImplementation.o.dx10_msaa )
	//	{
	//		int bound = RImplementation.o.dx10_msaa_opt ? 1 : RImplementation.o.dx10_msaa_samples;

	//		for( int i = 0; i < bound; ++i )
	//		{
	//			s_ssao_msaa[i].create( b_ssao_msaa[i], "null");
	//		}
	//	}
	//}

	// HDAO
	if (RImplementation.o.hbao_plus)
	{
		u32		w = Device.dwWidth, h = Device.dwHeight;
		rt_ssao_temp.create(r2_RT_ssao_temp, w, h, DXGI_FORMAT_R16_FLOAT, SRV_RTV, 1);
		rt_HBAO_plus_normal.create(r2_RT_HBAO_plus_normal, w, h, DXGI_FORMAT_R16G16B16A16_SNORM,SRV_RTV, 1);
	}
	else
	if( RImplementation.o.ssao_hdao && RImplementation.o.ssao_ultra)
	{
		u32		w = Device.dwWidth, h = Device.dwHeight;
		rt_ssao_temp.create(r2_RT_ssao_temp, w, h, DXGI_FORMAT_R16_FLOAT, SRV_RTV, 1);
		s_hdao_cs.create			(b_hdao_cs, "r2\\ssao");
		if( RImplementation.o.dx10_msaa )
		{
			s_hdao_cs_msaa.create			(b_hdao_msaa_cs, "r2\\ssao");
		}
	}

	// COMBINE
	{
		static D3DVERTEXELEMENT9 dwDecl[] =
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos+uv
			D3DDECL_END()
		};
		s_combine.create					(b_combine,					"r2\\combine");
		s_combine_volumetric.create			("combine_volumetric");
		s_combine_dbg_0.create				("effects\\screen_set",		r2_RT_smap_surf		);	
		s_combine_dbg_1.create				("effects\\screen_set",		r2_RT_luminance_t8	);
		s_combine_dbg_Accumulator.create	("effects\\screen_set",		r2_RT_accum			);
		g_combine_VP.create					(dwDecl,		RCache.Vertex.Buffer(), RCache.QuadIB);
		g_combine.create					(FVF::F_TL,		RCache.Vertex.Buffer(), RCache.QuadIB);
		g_combine_2UV.create				(FVF::F_TL2uv,	RCache.Vertex.Buffer(), RCache.QuadIB);
		g_combine_cuboid.create				(dwDecl,	RCache.Vertex.Buffer(), RCache.Index.Buffer());

		u32 fvf_aa_blur				= D3DFVF_XYZRHW|D3DFVF_TEX4|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1)|D3DFVF_TEXCOORDSIZE2(2)|D3DFVF_TEXCOORDSIZE2(3);
		g_aa_blur.create			(fvf_aa_blur,	RCache.Vertex.Buffer(), RCache.QuadIB);

		u32 fvf_aa_AA				= D3DFVF_XYZRHW|D3DFVF_TEX7|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1)|D3DFVF_TEXCOORDSIZE2(2)|D3DFVF_TEXCOORDSIZE2(3)|D3DFVF_TEXCOORDSIZE2(4)|D3DFVF_TEXCOORDSIZE4(5)|D3DFVF_TEXCOORDSIZE4(6);
		g_aa_AA.create				(fvf_aa_AA,		RCache.Vertex.Buffer(), RCache.QuadIB);

		u32 fvf_KD = D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0);
		g_KD.create(fvf_KD, RCache.Vertex.Buffer(), RCache.QuadIB);

		t_envmap_0.create			(r2_T_envs0);
		t_envmap_1.create			(r2_T_envs1);
	}

	// Build textures
	{
		// Testure for async sreenshots
		{
			D3D_TEXTURE2D_DESC	desc;
			desc.Width = Device.dwWidth;
			desc.Height = Device.dwHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
			desc.Usage = D3D_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			R_CHK( HW.pDevice->CreateTexture2D(&desc, 0, &t_ss_async) );
		}
		// Build material(s)
		{
			//	Create immutable texture. 
			//	So we need to init data _before_ the creation.
			// Surface
			//R_CHK						(D3DXCreateVolumeTexture(HW.pDevice,TEX_material_LdotN,TEX_material_LdotH,4,1,0,D3DFMT_A8L8,D3DPOOL_MANAGED,&t_material_surf));
			//t_material					= dxRenderDeviceRender::Instance().Resources->_CreateTexture(r2_material);
			//t_material->surface_set		(t_material_surf);
			//	Use DXGI_FORMAT_R8G8_UNORM

			u16	tempData[TEX_material_LdotN*TEX_material_LdotH*TEX_material_Count];

			D3D_TEXTURE3D_DESC	desc;
			desc.Width = TEX_material_LdotN;
			desc.Height = TEX_material_LdotH;
			desc.Depth	= TEX_material_Count;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8_UNORM;
			desc.Usage = D3D_USAGE_IMMUTABLE;
			desc.BindFlags = D3D_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			D3D_SUBRESOURCE_DATA	subData;

			subData.pSysMem = tempData;
			subData.SysMemPitch = desc.Width*2;
			subData.SysMemSlicePitch = desc.Height*subData.SysMemPitch;

			// Fill it (addr: x=dot(L,N),y=dot(L,H))
			//D3DLOCKED_BOX				R;
			//R_CHK						(t_material_surf->LockBox	(0,&R,0,0));
			for (u32 slice=0; slice<TEX_material_Count; slice++)
			{
				for (u32 y=0; y<TEX_material_LdotH; y++)
				{
					for (u32 x=0; x<TEX_material_LdotN; x++)
					{
						u16*	p	=	(u16*)		
							(LPBYTE (subData.pSysMem) 
							+ slice*subData.SysMemSlicePitch 
							+ y*subData.SysMemPitch + x*2);
						float	ld	=	float(x)	/ float	(TEX_material_LdotN-1);
						float	ls	=	float(y)	/ float	(TEX_material_LdotH-1) + EPS_S;
						ls			*=	powf(ld,1/32.f);
						float	fd,fs;

						switch	(slice)
						{
						case 0:	{ // looks like OrenNayar
							fd	= powf(ld,0.75f);		// 0.75
							fs	= powf(ls,16.f)*.5f;
								}	break;
						case 1:	{// looks like Blinn
							fd	= powf(ld,0.90f);		// 0.90
							fs	= powf(ls,24.f);
								}	break;
						case 2:	{ // looks like Phong
							fd	= ld;					// 1.0
							fs	= powf(ls*1.01f,128.f	);
								}	break;
						case 3:	{ // looks like Metal
							float	s0	=	_abs	(1-_abs	(0.05f*_sin(33.f*ld)+ld-ls));
							float	s1	=	_abs	(1-_abs	(0.05f*_cos(33.f*ld*ls)+ld-ls));
							float	s2	=	_abs	(1-_abs	(ld-ls));
							fd		=	ld;				// 1.0
							fs		=	powf	(_max(_max(s0,s1),s2), 24.f);
							fs		*=	powf	(ld,1/7.f);
								}	break;
						default:
							fd	= fs = 0;
						}
						s32		_d	=	clampr	(iFloor	(fd*255.5f),	0,255);
						s32		_s	=	clampr	(iFloor	(fs*255.5f),	0,255);
						if ((y==(TEX_material_LdotH-1)) && (x==(TEX_material_LdotN-1)))	{ _d = 255; _s=255;	}
						*p			=	u16		(_s*256 + _d);
					}
				}
			}
			//R_CHK		(t_material_surf->UnlockBox	(0));

			R_CHK(HW.pDevice->CreateTexture3D(&desc, &subData, &t_material_surf));
			t_material					= dxRenderDeviceRender::Instance().Resources->_CreateTexture(r2_material);
			t_material->surface_set		(t_material_surf);
			//R_CHK						(D3DXCreateVolumeTexture(HW.pDevice,TEX_material_LdotN,TEX_material_LdotH,4,1,0,D3DFMT_A8L8,D3DPOOL_MANAGED,&t_material_surf));
			//t_material					= dxRenderDeviceRender::Instance().Resources->_CreateTexture(r2_material);
			//t_material->surface_set		(t_material_surf);

			// #ifdef DEBUG
			// R_CHK	(D3DXSaveTextureToFile	("x:\\r2_material.dds",D3DXIFF_DDS,t_material_surf,0));
			// #endif
		}

		// Build noise table
		if (1)
		{
			// Surfaces
			//D3DLOCKED_RECT				R[TEX_jitter_count];

			//for (int it=0; it<TEX_jitter_count; it++)
			//{
			//	string_path					name;
			//	xr_sprintf						(name,"%s%d",r2_jitter,it);
			//	R_CHK	(D3DXCreateTexture	(HW.pDevice,TEX_jitter,TEX_jitter,1,0,D3DFMT_Q8W8V8U8,D3DPOOL_MANAGED,&t_noise_surf[it]));
			//	t_noise[it]					= dxRenderDeviceRender::Instance().Resources->_CreateTexture	(name);
			//	t_noise[it]->surface_set	(t_noise_surf[it]);
			//	R_CHK						(t_noise_surf[it]->LockRect	(0,&R[it],0,0));
			//}
			//	Use DXGI_FORMAT_R8G8B8A8_SNORM

			static const int sampleSize = 4;
			u32	tempData[TEX_jitter_count][TEX_jitter*TEX_jitter];

			D3D_TEXTURE2D_DESC	desc;
			desc.Width = TEX_jitter;
			desc.Height = TEX_jitter;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
			//desc.Usage = D3D_USAGE_IMMUTABLE;
			desc.Usage = D3D_USAGE_DEFAULT;
			desc.BindFlags = D3D_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			D3D_SUBRESOURCE_DATA	subData[TEX_jitter_count];

			for (int it=0; it<TEX_jitter_count-1; it++)
			{
				subData[it].pSysMem = tempData[it];
				subData[it].SysMemPitch = desc.Width*sampleSize;
			}

			// Fill it,
			for (u32 y=0; y<TEX_jitter; y++)
			{
				for (u32 x=0; x<TEX_jitter; x++)
				{
					DWORD	data	[TEX_jitter_count-1];
					generate_jitter	(data,TEX_jitter_count-1);
					for (u32 it=0; it<TEX_jitter_count-1; it++)
					{
						u32*	p	=	(u32*)	
							(LPBYTE (subData[it].pSysMem) 
							+ y*subData[it].SysMemPitch 
							+ x*4);

						*p	=	data	[it];
					}
				}
			}

			//for (int it=0; it<TEX_jitter_count; it++)	{
			//	R_CHK						(t_noise_surf[it]->UnlockRect(0));
			//}

			for (int it=0; it<TEX_jitter_count-1; it++)
			{
				string_path					name;
				xr_sprintf						(name,"%s%d",r2_jitter,it);
				//R_CHK	(D3DXCreateTexture	(HW.pDevice,TEX_jitter,TEX_jitter,1,0,D3DFMT_Q8W8V8U8,D3DPOOL_MANAGED,&t_noise_surf[it]));
				R_CHK( HW.pDevice->CreateTexture2D(&desc, &subData[it], &t_noise_surf[it]) );
				t_noise[it]					= dxRenderDeviceRender::Instance().Resources->_CreateTexture	(name);
				t_noise[it]->surface_set	(t_noise_surf[it]);
				//R_CHK						(t_noise_surf[it]->LockRect	(0,&R[it],0,0));
			}

			float tempDataHBAO[TEX_jitter*TEX_jitter*4];

			// generate HBAO jitter texture (last)
			D3D_TEXTURE2D_DESC	descHBAO;
			descHBAO.Width = TEX_jitter;
			descHBAO.Height = TEX_jitter;
			descHBAO.MipLevels = 1;
			descHBAO.ArraySize = 1;
			descHBAO.SampleDesc.Count = 1;
			descHBAO.SampleDesc.Quality = 0;
			descHBAO.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			//desc.Usage = D3D_USAGE_IMMUTABLE;
			descHBAO.Usage = D3D_USAGE_DEFAULT;
			descHBAO.BindFlags = D3D_BIND_SHADER_RESOURCE;
			descHBAO.CPUAccessFlags = 0;
			descHBAO.MiscFlags = 0;

			it = TEX_jitter_count-1;
			subData[it].pSysMem = tempDataHBAO;
			subData[it].SysMemPitch = descHBAO.Width*sampleSize * sizeof(float);

			// Fill it,
			for (u32 y=0; y<TEX_jitter; y++)
			{
				for (u32 x=0; x<TEX_jitter; x++)
				{
					float numDir = 1.0f;
					switch (ps_r_ssao)
					{
					case 1: numDir = 4.0f; break;
					case 2: numDir = 6.0f; break;
					case 3: numDir = 8.0f; break;
					case 4: numDir = 8.0f; break;
					}
					float angle = 2 * PI * ::Random.randF(0.0f, 1.0f) / numDir;
					float dist = ::Random.randF(0.0f, 1.0f);

					float *p	=	(float*)	
						(LPBYTE (subData[it].pSysMem) 
						+ y*subData[it].SysMemPitch 
						+ x*4*sizeof(float));
					*p = (float)(_cos(angle));
					*(p+1) = (float)(_sin(angle));
					*(p+2) = (float)(dist);
					*(p+3) = 0;
				}
			}			

			string_path					name;
			xr_sprintf						(name,"%s%d",r2_jitter,it);
			//R_CHK	(D3DXCreateTexture	(HW.pDevice,TEX_jitter,TEX_jitter,1,0,D3DFMT_Q8W8V8U8,D3DPOOL_MANAGED,&t_noise_surf[it]));
			R_CHK( HW.pDevice->CreateTexture2D(&descHBAO, &subData[it], &t_noise_surf[it]) );
			t_noise[it]					= dxRenderDeviceRender::Instance().Resources->_CreateTexture	(name);
			t_noise[it]->surface_set	(t_noise_surf[it]);


			//	Create noise mipped
			{
				//	Autogen mipmaps
				desc.MipLevels = 0;
				R_CHK( HW.pDevice->CreateTexture2D(&desc, 0, &t_noise_surf_mipped) );
				t_noise_mipped = dxRenderDeviceRender::Instance().Resources->_CreateTexture(r2_jitter_mipped);
				t_noise_mipped->surface_set(t_noise_surf_mipped);

				//	Update texture. Generate mips.

				HW.pContext->CopySubresourceRegion( t_noise_surf_mipped, 0, 0, 0, 0, t_noise_surf[0], 0, 0 );

				D3DX11FilterTexture(HW.pContext, t_noise_surf_mipped, 0, D3DX10_FILTER_POINT);
			}
		}
	}

	// PP
	s_postprocess.create				("postprocess");
	g_postprocess.create				(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX3,RCache.Vertex.Buffer(),RCache.QuadIB);

	// Menu
	s_menu.create						("distort");
	g_menu.create						(FVF::F_TL,RCache.Vertex.Buffer(),RCache.QuadIB);

	// 
	dwWidth		= Device.dwWidth;
	dwHeight	= Device.dwHeight;
}

CRenderTarget::~CRenderTarget	()
{
	_RELEASE					(t_ss_async);

	// Textures
	t_material->surface_set		(NULL);

#ifdef DEBUG
	_SHOW_REF					("t_material_surf",t_material_surf);
#endif // DEBUG
	_RELEASE					(t_material_surf);

	t_LUM_src->surface_set		(NULL);
	t_LUM_dest->surface_set		(NULL);

#ifdef DEBUG
	ID3DBaseTexture*	pSurf = 0;

	pSurf = t_envmap_0->surface_get();
	if (pSurf) pSurf->Release();
	_SHOW_REF("t_envmap_0 - #small",pSurf);

	pSurf = t_envmap_1->surface_get();
	if (pSurf) pSurf->Release();
	_SHOW_REF("t_envmap_1 - #small",pSurf);
	//_SHOW_REF("t_envmap_0 - #small",t_envmap_0->pSurface);
	//_SHOW_REF("t_envmap_1 - #small",t_envmap_1->pSurface);
#endif // DEBUG
	t_envmap_0->surface_set		(NULL);
	t_envmap_1->surface_set		(NULL);
	t_envmap_0.destroy			();
	t_envmap_1.destroy			();

	//	TODO: DX10: Check if we need old style SMAPs
//	_RELEASE					(rt_smap_ZB);

	// Jitter
	for (int it=0; it<TEX_jitter_count; it++)	{
		t_noise	[it]->surface_set	(NULL);
#ifdef DEBUG
		_SHOW_REF("t_noise_surf[it]",t_noise_surf[it]);
#endif // DEBUG
		_RELEASE					(t_noise_surf[it]);
	}

	t_noise_mipped->surface_set	(NULL);
#ifdef DEBUG
	_SHOW_REF("t_noise_surf_mipped",t_noise_surf_mipped);
#endif // DEBUG
	_RELEASE					(t_noise_surf_mipped);

	// 
	accum_spot_geom_destroy		();
	accum_omnip_geom_destroy	();
	accum_point_geom_destroy	();
	accum_volumetric_geom_destroy();

	// Blenders
	xr_delete					(b_combine				);
	xr_delete					(b_luminance			);
	xr_delete					(b_bloom				);
	xr_delete					(b_accum_reflected		);
	xr_delete					(b_accum_spot			);
	xr_delete					(b_accum_point			);
	xr_delete					(b_accum_direct			);
	xr_delete					(b_ssao					);
	xr_delete					(b_blur					);
	xr_delete					(b_hud_blood			); //Hud Blood
	xr_delete					(b_hud_power			); //Hud Stamina
	xr_delete					(b_hud_bleeding			); //Hud Bleeding
	xr_delete					(b_hud_satiety			); //satiety
	xr_delete					(b_hud_thirst			); //thirst
	xr_delete(b_sunshafts);
	xr_delete(b_lfx); //SFZ Lens Flares
	xr_delete(b_ssfx_ssr); // [Ascii1457] SSS new Phase
	xr_delete(b_ssfx_volumetric_blur); // [Ascii1457] SSS new Phase
	xr_delete(b_gasmask_drops);
	xr_delete(b_gasmask_dudv);

   if( RImplementation.o.dx10_msaa )
   {
      int bound = RImplementation.o.dx10_msaa_samples;

      if( RImplementation.o.dx10_msaa_opt )
         bound = 1;

	  for( int i = 0; i < bound; ++i )
	  {
		  xr_delete					(b_combine_msaa[i]);
		  xr_delete					(b_accum_direct_msaa[i]);
		  xr_delete					(b_accum_mask_msaa[i]);
		  xr_delete					(b_accum_direct_volumetric_msaa[i]);
		  //xr_delete					(b_accum_direct_volumetric_sun_msaa[i]);
		  xr_delete					(b_accum_spot_msaa[i]);
		  xr_delete					(b_accum_volumetric_msaa[i]);
		  xr_delete					(b_accum_point_msaa[i]);
		  xr_delete					(b_accum_reflected_msaa[i]);
		  xr_delete					(b_ssao_msaa[i]);
	  }
   }
	xr_delete					(b_accum_mask			);
	xr_delete					(b_occq					);
	xr_delete					(b_hdao_cs				);
	if( RImplementation.o.dx10_msaa )
	{
        xr_delete( b_hdao_msaa_cs );
    }
}

void CRenderTarget::reset_light_marker( bool bResetStencil)
{
	dwLightMarkerID = 5;
	if (bResetStencil)
	{
		u32		Offset;
		float	_w					= float(Device.dwWidth);
		float	_h					= float(Device.dwHeight);
		u32		C					= color_rgba	(255,255,255,255);
		float	eps					= 0;
		float	_dw					= 0.5f;
		float	_dh					= 0.5f;
		FVF::TL* pv					= (FVF::TL*) RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set						(-_dw,		_h-_dh,		eps,	1.f, C, 0, 0);	pv++;
		pv->set						(-_dw,		-_dh,		eps,	1.f, C, 0, 0);	pv++;
		pv->set						(_w-_dw,	_h-_dh,		eps,	1.f, C, 0, 0);	pv++;
		pv->set						(_w-_dw,	-_dh,		eps,	1.f, C, 0, 0);	pv++;
		RCache.Vertex.Unlock		(4,g_combine->vb_stride);
		RCache.set_Element			(s_occq->E[2]	);
		RCache.set_Geometry			(g_combine		);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}
}

void CRenderTarget::increment_light_marker()
{
	dwLightMarkerID += 2;

	//if (dwLightMarkerID>10)
	const u32 iMaxMarkerValue = RImplementation.o.dx10_msaa ? 127 : 255;
	
	if ( dwLightMarkerID > iMaxMarkerValue )
		reset_light_marker(true);
}

bool CRenderTarget::need_to_render_sunshafts()
{
	if (!ps_r_sun_shafts)
		return false;

	{
		CEnvDescriptor&	E = *g_pGamePersistent->Environment().CurrentEnv;
		float fValue = E.m_fSunShaftsIntensity;
		//	TODO: add multiplication by sun color here
		if (fValue<0.0001) return false;
	}

	return true;
}

bool CRenderTarget::use_minmax_sm_this_frame()
{
	switch(RImplementation.o.dx10_minmax_sm)
	{
	case CRender::MMSM_ON:
		return true;
	case CRender::MMSM_AUTO:
		return need_to_render_sunshafts();
	case CRender::MMSM_AUTODETECT:
		{
			u32 dwScreenArea = 
				HW.m_ChainDesc.BufferDesc.Width*
				HW.m_ChainDesc.BufferDesc.Height;

			if ( ( dwScreenArea >=RImplementation.o.dx10_minmax_sm_screenarea_threshold))
				return need_to_render_sunshafts();
			else 
				return false;
		}
		
	default:
		return false;
	}

}
