#ifndef SH_RT_H
#define SH_RT_H
#pragma once

enum VIEW_TYPE
{
	//SRV = 1 << 1, // ShaderResource
	//RTV = 1 << 2, // RenderTarget
	//DSV = 1 << 3, // DepthStencil
	//UAV = 1 << 4, // UnorderedAcces
	SRV_RTV,		// ShaderResource & RenderTarget
	SRV_RTV_UAV,	// ShaderResource & RenderTarget & UnorderedAcces
	SRV_RTV_DSV,	// ShaderResource & RenderTarget & DepthStencil
	SRV_DSV,		// ShaderResource & DepthStencil
};

//////////////////////////////////////////////////////////////////////////
class		CRT		:	public xr_resource_named	{
public:
	CRT();
	~CRT();

	void	create(LPCSTR Name, u32 w, u32 h, DXGI_FORMAT f, VIEW_TYPE view, u32 s);

	void	destroy();
	void	reset_begin();
	void	reset_end();
	IC BOOL	valid()	{ return !!pTexture; }

public:
	ID3DTexture2D*			pSurface;
	ID3DRenderTargetView*	pRT;

	ref_texture				pTexture;

	ID3DDepthStencilView* pZRT;
	ID3D11UnorderedAccessView* pUAView;
	DXGI_FORMAT format;
	VIEW_TYPE					view;
	u32							samples;

	u64						_order;
	u32						dwWidth;
	u32						dwHeight;
};
struct 		resptrcode_crt : public resptr_base<CRT>
{
	void				create(LPCSTR Name, u32 w, u32 h, DXGI_FORMAT f, VIEW_TYPE view, u32 s);

	void				destroy() { _set(NULL); }
};
typedef	resptr_core<CRT, resptrcode_crt>		ref_rt;

#endif // SH_RT_H
