// dx10HW.cpp: implementation of the DX10 specialisation of CHW.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#pragma warning(disable:4995)
#include <dxsdk/d3dx9.h>
#pragma warning(default:4995)
#include "../xrRender/HW.h"
#include "../../xrEngine/XR_IOConsole.h"
#include "../../Include/xrAPI/xrAPI.h"

#include "StateManager\dx10SamplerStateCache.h"
#include "StateManager\dx10StateCache.h"

#ifndef _EDITOR
void	fill_vid_mode_list(CHW* _hw);
void	free_vid_mode_list();

void	fill_render_mode_list();
void	free_render_mode_list();
#else
void	fill_vid_mode_list(CHW* _hw) {}
void	free_vid_mode_list() {}
void	fill_render_mode_list() {}
void	free_render_mode_list() {}
#endif

CHW			HW;


#pragma warning(disable:4995)
// Imgui
#include <imgui.h>
#include "backends\imgui_impl_dx11.h"
#include "backends\imgui_impl_win32.h"		 

#pragma comment(lib, "imgui.lib")
#pragma warning(default:4995)



CHW::CHW() :
	//	hD3D(NULL),
		//pD3D(NULL),
	m_pAdapter(0),
	pDevice(NULL),
	m_move_window(true)
	//pBaseRT(NULL),
	//pBaseZB(NULL)
{
	Device.seqAppActivate.Add(this);
	Device.seqAppDeactivate.Add(this);
}

CHW::~CHW()
{
	Device.seqAppActivate.Remove(this);
	Device.seqAppDeactivate.Remove(this);
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CHW::CreateD3D()
{

	IDXGIFactory* pFactory;
	R_CHK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory)));

	m_pAdapter = 0;
	m_bUsePerfhud = false;

	if (!m_pAdapter)
		pFactory->EnumAdapters(0, &m_pAdapter);

	pFactory->Release();

}

void CHW::DestroyD3D()
{
	//_RELEASE					(this->pD3D);

	_SHOW_REF("refCount:m_pAdapter", m_pAdapter);
	_RELEASE(m_pAdapter);

	//	FreeLibrary(hD3D);
}

void CHW::CreateDevice(HWND m_hWnd, bool move_window)
{
	m_move_window = move_window;
	CreateD3D();



	// TODO: DX10: Create appropriate initialization

	// General - select adapter and device
	BOOL  bWindowed = !psDeviceFlags.is(rsFullscreen);

	m_DriverType = Caps.bForceGPU_REF ?
		D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_HARDWARE;

	if (m_bUsePerfhud)
		m_DriverType = D3D_DRIVER_TYPE_REFERENCE;

	// Display the name of video board
	DXGI_ADAPTER_DESC Desc;
	R_CHK(m_pAdapter->GetDesc(&Desc));
	//	Warning: Desc.Description is wide string
	Msg("* GPU [vendor:%X]-[device:%X]: %S", Desc.VendorId, Desc.DeviceId, Desc.Description);


	Caps.id_vendor = Desc.VendorId;
	Caps.id_device = Desc.DeviceId;

	// Set up the presentation parameters
	DXGI_SWAP_CHAIN_DESC& sd = m_ChainDesc;
	ZeroMemory(&sd, sizeof(sd));

	selectResolution(sd.BufferDesc.Width, sd.BufferDesc.Height, bWindowed);


	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferCount = 1;

	// Multisample
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = bWindowed;

	if (bWindowed)
	{
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
	}
	else
	{
		sd.BufferDesc.RefreshRate = selectRefresh(sd.BufferDesc.Width, sd.BufferDesc.Height, sd.BufferDesc.Format);
	}

	//	Additional set up
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	UINT createDeviceFlags = 0;
#ifdef DEBUG
	//createDeviceFlags |= D3Dxx_CREATE_DEVICE_DEBUG;
#endif
	HRESULT R;
	// Create the device
	//	DX10 don't need it?
	//u32 GPU		= selectGPU();
	D3D_FEATURE_LEVEL pFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	R = D3D11CreateDeviceAndSwapChain(0,//m_pAdapter,//What wrong with adapter??? We should use another version of DXGI?????
		m_DriverType,
		NULL,
		createDeviceFlags,
		pFeatureLevels,
		sizeof(pFeatureLevels) / sizeof(pFeatureLevels[0]),
		D3D11_SDK_VERSION,
		&sd,
		&m_pSwapChain,
		&pDevice,
		&FeatureLevel,
		&pContext);


	if (FAILED(R))
	{
		// Fatal error! Cannot create rendering device AT STARTUP !!!
		Msg("Failed to initialize graphics hardware.\n"
			"Please try to restart the game.\n"
			"CreateDevice returned 0x%08x", R
		);
		FlushLog();
		MessageBox(NULL, "Failed to initialize graphics hardware.\nPlease try to restart the game.", "Error!", MB_OK | MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), 0);
	};
	R_CHK(R);

	_SHOW_REF("* CREATE: DeviceREF:", HW.pDevice);


	//if (ps_r_ssao == SSAO_HBAO_PLUS)
	{
		GFSDK_SSAO_CustomHeap CustomHeap;
		CustomHeap.new_ = ::operator new;
		CustomHeap.delete_ = ::operator delete;
		GFSDK_SSAO_CreateContext_D3D11(pDevice, &pSSAO, &CustomHeap, GFSDK_SSAO_Version());

		if (pSSAO)
		{
			Msg("*pSSAO HAS CONTEXT");
		}
	}



	//	Create render target and depth-stencil views here
	UpdateViews();

	//u32	memory									= pDevice->GetAvailableTextureMem	();
	size_t	memory = Desc.DedicatedVideoMemory;
	Msg("*     Texture memory: %d M", memory / (1024 * 1024));
	//Msg		("*          DDI-level: %2.1f",		float(D3DXGetDriverLevel(pDevice))/100.f);
#ifndef _EDITOR
	updateWindowProps(m_hWnd);
	fill_vid_mode_list(this);
#endif

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::SetAllocatorFunctions(
		[](size_t size, void* /*user_data*/)
		{
			return xr_malloc(size);
		},
		[](void* ptr, void* /*user_data*/)
		{
			xr_free(ptr);
		}
	);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplDX11_Init(pDevice, pContext);
	ImGui_ImplWin32_Init(m_hWnd);
}

void CHW::DestroyDevice()
{
	ImGui_ImplDX11_Shutdown();
	//	Destroy state managers
	StateManager.Reset();
	RSManager.ClearStateArray();
	DSSManager.ClearStateArray();
	BSManager.ClearStateArray();
	SSManager.ClearStateArray();

	_SHOW_REF("refCount:pBaseZB", pBaseZB);
	_RELEASE(pBaseZB);

	_SHOW_REF("refCount:pBaseRT", pBaseRT);
	_RELEASE(pBaseRT);

	//	Must switch to windowed mode to release swap chain
	if (!m_ChainDesc.Windowed) m_pSwapChain->SetFullscreenState(FALSE, NULL);
	_SHOW_REF("refCount:m_pSwapChain", m_pSwapChain);
	_RELEASE(m_pSwapChain);

	_RELEASE(pContext);

	if (pSSAO)
		_RELEASE(pSSAO);

	pDepthStencil->Release();

	_SHOW_REF("DeviceREF:", HW.pDevice);
	_RELEASE(HW.pDevice);


	DestroyD3D();

#ifndef _EDITOR
	free_vid_mode_list();
#endif
}

//////////////////////////////////////////////////////////////////////
// Resetting device
//////////////////////////////////////////////////////////////////////
void CHW::Reset(HWND hwnd)
{
	ImGui_ImplDX11_InvalidateDeviceObjects();

	DXGI_SWAP_CHAIN_DESC& cd = m_ChainDesc;

	BOOL	bWindowed = !psDeviceFlags.is(rsFullscreen);

	cd.Windowed = bWindowed;

	m_pSwapChain->SetFullscreenState(!bWindowed, NULL);

	DXGI_MODE_DESC& desc = m_ChainDesc.BufferDesc;

	selectResolution(desc.Width, desc.Height, bWindowed);

	if (bWindowed)
	{
		desc.RefreshRate.Numerator = 60;
		desc.RefreshRate.Denominator = 1;
	}
	else
		desc.RefreshRate = selectRefresh(desc.Width, desc.Height, desc.Format);

	CHK_DX(m_pSwapChain->ResizeTarget(&desc));

	_SHOW_REF("refCount:pBaseZB", pBaseZB);
	_SHOW_REF("refCount:pBaseRT", pBaseRT);

	_RELEASE(pBaseZB);
	_RELEASE(pBaseRT);

	CHK_DX(m_pSwapChain->ResizeBuffers(
		cd.BufferCount,
		desc.Width,
		desc.Height,
		desc.Format,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	UpdateViews();


	updateWindowProps(hwnd);

}

void CHW::selectResolution(u32& dwWidth, u32& dwHeight, BOOL bWindowed)
{
	fill_vid_mode_list(this);

	if (bWindowed)
	{
		dwWidth = psCurrentVidMode[0];
		dwHeight = psCurrentVidMode[1];
	}
	else //check
	{
		string64					buff;
		xr_sprintf(buff, sizeof(buff), "%dx%d", psCurrentVidMode[0], psCurrentVidMode[1]);

		if (_ParseItem(buff, vid_mode_token) == u32(-1)) //not found
		{ //select safe
			xr_sprintf(buff, sizeof(buff), "vid_mode %s", vid_mode_token[0].name);
			Console->Execute(buff);
		}

		dwWidth = psCurrentVidMode[0];
		dwHeight = psCurrentVidMode[1];
	}
}

//	TODO: DX10: check if we need these
/*
u32	CHW::selectPresentInterval	()
{
	D3DCAPS9	caps;
	pD3D->GetDeviceCaps(DevAdapter,DevT,&caps);

	if (!psDeviceFlags.test(rsVSync))
	{
		if (caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
			return D3DPRESENT_INTERVAL_IMMEDIATE;
		if (caps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE)
			return D3DPRESENT_INTERVAL_ONE;
	}
	return D3DPRESENT_INTERVAL_DEFAULT;
}

u32 CHW::selectGPU ()
{
	if (Caps.bForceGPU_SW) return D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	D3DCAPS9	caps;
	pD3D->GetDeviceCaps(DevAdapter,DevT,&caps);

	if(caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		if (Caps.bForceGPU_NonPure)	return D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else {
			if (caps.DevCaps&D3DDEVCAPS_PUREDEVICE) return D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE;
			else return D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
		// return D3DCREATE_MIXED_VERTEXPROCESSING;
	} else return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
}
*/

void CHW::OnAppActivate()
{
	if (m_pSwapChain && !m_ChainDesc.Windowed)
	{
		ShowWindow(m_ChainDesc.OutputWindow, SW_RESTORE);
		m_pSwapChain->SetFullscreenState(TRUE, NULL);
	}
}

void CHW::OnAppDeactivate()
{
	if (m_pSwapChain && !m_ChainDesc.Windowed)
	{
		m_pSwapChain->SetFullscreenState(FALSE, NULL);
		ShowWindow(m_ChainDesc.OutputWindow, SW_MINIMIZE);
	}
}

DXGI_RATIONAL CHW::selectRefresh(u32 dwWidth, u32 dwHeight, DXGI_FORMAT fmt)
{
	DXGI_RATIONAL res;

	res.Numerator = Device.GetMonitorRefresh();
	res.Denominator = 1;

	float CurrentFreq = Device.GetMonitorRefresh();

	xr_vector<DXGI_MODE_DESC> modes;

	IDXGIOutput* pOutput;
	m_pAdapter->EnumOutputs(0, &pOutput);
	VERIFY(pOutput);

	UINT num = 0;
	DXGI_FORMAT format = fmt;
	UINT flags = 0;

	// Get the number of display modes available
	pOutput->GetDisplayModeList(format, flags, &num, 0);

	// Get the list of display modes
	modes.resize(num);
	pOutput->GetDisplayModeList(format, flags, &num, &modes.front());

	_RELEASE(pOutput);

	for (u32 i = 0; i < num; ++i)
	{
		DXGI_MODE_DESC& desc = modes[i];

		if ((desc.Width == dwWidth)
			&& (desc.Height == dwHeight)
			)
		{
			VERIFY(desc.RefreshRate.Denominator);
			float TempFreq = float(desc.RefreshRate.Numerator) / float(desc.RefreshRate.Denominator);
			if (TempFreq > CurrentFreq)
			{
				CurrentFreq = TempFreq;
				res = desc.RefreshRate;
			}
		}
	}

	refresh_rate = 1.f / CurrentFreq;

	return res;
}

void CHW::updateWindowProps_Position(HWND m_hWnd, u32 X, u32 Y, u32 SizeX, u32 SizeY)
{
	if (psDeviceFlags.is(rsFullscreen))
		return;

	u32		dwWindowStyle = 0;
	// Set window properties depending on what mode were in.

	if (m_move_window)
	{
		Msg("Update Window Pos : X: %u, Y: %u", X, Y);

		SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle = (WS_BORDER | WS_DLGFRAME | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX));

		RECT			m_rcWindowBounds;

		SetRect(&m_rcWindowBounds,
			X,
			Y,
			X + m_ChainDesc.BufferDesc.Width,
			Y + m_ChainDesc.BufferDesc.Height);

		AdjustWindowRect(&m_rcWindowBounds, dwWindowStyle, FALSE);

		SetWindowPos(m_hWnd,
			HWND_NOTOPMOST,
			m_rcWindowBounds.left,
			m_rcWindowBounds.top,
			(m_rcWindowBounds.right - m_rcWindowBounds.left),
			(m_rcWindowBounds.bottom - m_rcWindowBounds.top),
			SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_DRAWFRAME);
	}

	ShowCursor(FALSE);
	SetForegroundWindow(m_hWnd);

}

void CHW::updateWindowProps(HWND m_hWnd)
{
	//	BOOL	bWindowed				= strstr(Core.Params,"-dedicated") ? TRUE : !psDeviceFlags.is	(rsFullscreen);
	BOOL	bWindowed = !psDeviceFlags.is(rsFullscreen);

	u32		dwWindowStyle = 0;
	// Set window properties depending on what mode were in.
	if (bWindowed) {
		if (m_move_window) {
			if (strstr(Core.Params, "-no_dialog_header"))
				SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle = (WS_BORDER | WS_VISIBLE));
			else
				SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle = (WS_BORDER | WS_DLGFRAME | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX));
			// When moving from fullscreen to windowed mode, it is important to
			// adjust the window size after recreating the device rather than
			// beforehand to ensure that you get the window size you want.  For
			// example, when switching from 640x480 fullscreen to windowed with
			// a 1000x600 window on a 1024x768 desktop, it is impossible to set
			// the window size to 1000x600 until after the display mode has
			// changed to 1024x768, because windows cannot be larger than the
			// desktop.

			RECT			m_rcWindowBounds;
			BOOL			bCenter = FALSE;
			BOOL			bRight = FALSE;

			if (strstr(Core.Params, "-center_screen"))	bCenter = TRUE;
			if (strstr(Core.Params, "-right_screen")) bRight = TRUE;

			if (bCenter) {
				RECT				DesktopRect;

				GetClientRect(GetDesktopWindow(), &DesktopRect);

				SetRect(&m_rcWindowBounds,
					(DesktopRect.right - m_ChainDesc.BufferDesc.Width) / 2,
					(DesktopRect.bottom - m_ChainDesc.BufferDesc.Height) / 2,
					(DesktopRect.right + m_ChainDesc.BufferDesc.Width) / 2,
					(DesktopRect.bottom + m_ChainDesc.BufferDesc.Height) / 2);
			}
			else
				if (bRight)
				{
					RECT				DesktopRect;

					GetClientRect(GetDesktopWindow(), &DesktopRect);

					SetRect(&m_rcWindowBounds,
						1024,
						0,
						m_ChainDesc.BufferDesc.Width + 1024,
						m_ChainDesc.BufferDesc.Height);
				}
				else
				{
					SetRect(&m_rcWindowBounds,
						0,
						0,
						m_ChainDesc.BufferDesc.Width,
						m_ChainDesc.BufferDesc.Height);
				};

			AdjustWindowRect(&m_rcWindowBounds, dwWindowStyle, FALSE);

			SetWindowPos(m_hWnd,
				HWND_NOTOPMOST,
				m_rcWindowBounds.left,
				m_rcWindowBounds.top,
				(m_rcWindowBounds.right - m_rcWindowBounds.left),
				(m_rcWindowBounds.bottom - m_rcWindowBounds.top),
				SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_DRAWFRAME);
		}
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle = (WS_POPUP | WS_VISIBLE));
	}

	ShowCursor(FALSE);
	SetForegroundWindow(m_hWnd);
}


struct _uniq_mode
{
	_uniq_mode(LPCSTR v) :_val(v) {}
	LPCSTR _val;
	bool operator() (LPCSTR _other) { return !stricmp(_val, _other); }
};

#ifndef _EDITOR

void free_vid_mode_list()
{
	for (int i = 0; vid_mode_token[i].name; i++)
	{
		xr_free(vid_mode_token[i].name);
	}
	xr_free(vid_mode_token);
	vid_mode_token = NULL;
}

void fill_vid_mode_list(CHW* _hw)
{
	if (vid_mode_token != NULL)		return;
	xr_vector<LPCSTR>	_tmp;
	xr_vector<DXGI_MODE_DESC>	modes;

	IDXGIOutput* pOutput;
	//_hw->m_pSwapChain->GetContainingOutput(&pOutput);
	_hw->m_pAdapter->EnumOutputs(0, &pOutput);
	VERIFY(pOutput);

	UINT num = 0;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT flags = 0;

	// Get the number of display modes available
	pOutput->GetDisplayModeList(format, flags, &num, 0);

	// Get the list of display modes
	modes.resize(num);
	pOutput->GetDisplayModeList(format, flags, &num, &modes.front());

	_RELEASE(pOutput);

	for (u32 i = 0; i < num; ++i)
	{
		DXGI_MODE_DESC& desc = modes[i];
		string32		str;

		if (desc.Width < 800)
			continue;

		xr_sprintf(str, sizeof(str), "%dx%d", desc.Width, desc.Height);

		if (_tmp.end() != std::find_if(_tmp.begin(), _tmp.end(), _uniq_mode(str)))
			continue;

		_tmp.push_back(NULL);
		_tmp.back() = xr_strdup(str);
	}



	//	_tmp.push_back				(NULL);
	//	_tmp.back()					= xr_strdup("1024x768");

	u32 _cnt = _tmp.size() + 1;

	vid_mode_token = xr_alloc<xr_token>(_cnt);

	vid_mode_token[_cnt - 1].id = -1;
	vid_mode_token[_cnt - 1].name = NULL;

#ifdef DEBUG
	Msg("Available video modes[%d]:", _tmp.size());
#endif // DEBUG
	for (u32 i = 0; i < _tmp.size(); ++i)
	{
		vid_mode_token[i].id = i;
		vid_mode_token[i].name = _tmp[i];
#ifdef DEBUG
		Msg("[%s]", _tmp[i]);
#endif // DEBUG
	}

}

void CHW::UpdateViews()
{
	DXGI_SWAP_CHAIN_DESC& sd = m_ChainDesc;
	HRESULT R;

	// Create a render target view
	//R_CHK	(pDevice->GetRenderTarget			(0,&pBaseRT));
	ID3DTexture2D* pBuffer;
	R = m_pSwapChain->GetBuffer(0, __uuidof(ID3DTexture2D), (LPVOID*)&pBuffer);
	R_CHK(R);
	R = pDevice->CreateRenderTargetView(pBuffer, NULL, &pBaseRT);
	R_CHK(R);

	pBuffer->Release();

	//	Create Depth/stencil buffer
	//	HACK: DX10: hard depth buffer format
	//R_CHK	(pDevice->GetDepthStencilSurface	(&pBaseZB));

	D3D_TEXTURE2D_DESC descDepth;
	descDepth.Width = sd.BufferDesc.Width;
	descDepth.Height = sd.BufferDesc.Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS; // DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D_USAGE_DEFAULT;
	descDepth.BindFlags = D3D_BIND_DEPTH_STENCIL | D3D_BIND_SHADER_RESOURCE;;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	R = pDevice->CreateTexture2D(&descDepth,       // Texture desc
		NULL,                  // Initial data
		&pDepthStencil); // [out] Texture
	R_CHK(R);

	//	Create Depth/stencil view
	//R = pDevice->CreateDepthStencilView( pDepthStencil, NULL, &pBaseZB );
	//R_CHK(R);
	D3D_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	R_CHK(pDevice->CreateDepthStencilView(pDepthStencil, &dsvDesc, &pBaseZB)); // read & wtire DSV



	// Shader resource view
	D3D_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};
	depthSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthSRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MipLevels = 1;
	depthSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
	R_CHK(pDevice->CreateShaderResourceView(pDepthStencil, &depthSRVDesc, &pBaseDepthReadSRV)); // read SRV

	if (pBaseDepthReadSRV)
	{
		Msg("* Shader Resource: pBaseDepthReadSRV Created");
	}


}
#endif
