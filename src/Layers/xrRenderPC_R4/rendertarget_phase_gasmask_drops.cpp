#include "stdafx.h"

void CRenderTarget::phase_gasmask_drops()
{
	Fvector3 Params = g_pGamePersistent->GetRainDropsParams();
	float HudGlassElement = g_pGamePersistent->GetHudGlassElement();
	//Constants
	u32 Offset = 0;
	u32 C = color_rgba(0, 0, 0, 255);

	float d_Z = EPS_S;
	float d_W = 1.0f;
	float w = float(Device.dwWidth);
	float h = float(Device.dwHeight);

	Fvector2 p0, p1;
	p0.set(0.0f, 0.0f);
	p1.set(1.0f, 1.0f);

	//////////////////////////////////////////////////////////////////////////
	//Set MSAA/NonMSAA rendertarget
	ref_rt& dest_rt = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;
	u_setrt(dest_rt, nullptr, nullptr, nullptr);	

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	//Fill vertex buffer
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, float(h), d_Z, d_W, C, p0.x, p1.y); pv++;
	pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(float(w), float(h), d_Z, d_W, C, p1.x, p1.y); pv++;
	pv->set(float(w), 0, d_Z, d_W, C, p1.x, p0.y); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	//Set pass
	RCache.set_Element(s_gasmask_drops->E[0]);

	//Set parameters
	RCache.set_c("drops_control", Params.x, Params.y, Params.z, 0);
	RCache.set_c("mask_control", HudGlassElement, 1.f, 1.f, 1.f);

	//Set geometry
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
};
