// HW.h: interface for the CHW class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HW_H__0E25CF4A_FFEC_11D3_B4E3_4854E82A090D__INCLUDED_)
#define AFX_HW_H__0E25CF4A_FFEC_11D3_B4E3_4854E82A090D__INCLUDED_
#pragma once

#include "hwcaps.h"

#ifndef _MAYA_EXPORT
#include "stats_manager.h"
#endif


class  CHW

	: public pureAppActivate,
	public pureAppDeactivate

{
	//	Functions section
	public:
		int maxRefreshRate;
		CHW();
		~CHW();

		void					CreateD3D();
		void					DestroyD3D();

		void					CreateDevice(HWND hw, bool move_window);

		void					DestroyDevice();

		void					Reset(HWND hw);

		void					selectResolution(u32& dwWidth, u32& dwHeight, BOOL bWindowed);
		u32						selectPresentInterval();
		u32						selectGPU();
		void					updateWindowProps(HWND hw);


		void	Validate(void) {};

		//	Variables section
		public:
			IDXGIAdapter* m_pAdapter;	//	pD3D equivalent
			ID3D11Device* pDevice;	//	combine with DX9 pDevice via typedef
			ID3D11DeviceContext* pContext;	//	combine with DX9 pDevice via typedef
			IDXGISwapChain* m_pSwapChain;
			ID3D11RenderTargetView* pBaseRT;	//	combine with DX9 pBaseRT via typedef
			ID3D11DepthStencilView* pBaseZB;

			CHWCaps					Caps;

			D3D_DRIVER_TYPE		m_DriverType;	//	DevT equivalent
			DXGI_SWAP_CHAIN_DESC	m_ChainDesc;	//	DevPP equivalent
			bool					m_bUsePerfhud;
			D3D_FEATURE_LEVEL		FeatureLevel;

			GFSDK_SSAO_Context_D3D* pSSAO;

			ID3DTexture2D* pDepthStencil;
			ID3DShaderResourceView* pBaseDepthReadSRV;

		#ifndef _MAYA_EXPORT
			stats_manager			stats_manager;
		#endif
			void			UpdateViews();
			DXGI_RATIONAL	selectRefresh(u32 dwWidth, u32 dwHeight, DXGI_FORMAT fmt);

			virtual	void	OnAppActivate();
			virtual void	OnAppDeactivate();
			virtual void	updateWindowProps_Position(HWND hw, u32 X, u32 Y, u32 SizeX, u32 SizeY);
		private:
			bool					m_move_window;
};

extern ECORE_API CHW		HW;

#endif // !defined(AFX_HW_H__0E25CF4A_FFEC_11D3_B4E3_4854E82A090D__INCLUDED_)
