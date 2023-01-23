#include "stdafx.h"
#include "Text_Console.h"
#include "line_editor.h"

extern char const * const		ioc_prompt;
extern char const * const		ch_cursor;
int g_svTextConsoleUpdateRate = 1;

CTextConsole::CTextConsole()
{
	m_pMainWnd    = NULL;
	m_hConsoleWnd = NULL;
	m_hLogWnd     = NULL;
	m_hLogWndFont = NULL;

	m_bScrollLog  = true;
	m_dwStartLine = 0;

	m_bNeedUpdate      = false;
	m_dwLastUpdateTime = Device.dwTimeGlobal;
	m_last_time        = Device.dwTimeGlobal;
}

CTextConsole::~CTextConsole()
{
	m_pMainWnd = NULL;
}

//-------------------------------------------------------------------------------------------
LRESULT CALLBACK TextConsole_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void	CTextConsole::CreateConsoleWnd()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	//----------------------------------
	RECT cRc;
	GetClientRect(*m_pMainWnd, &cRc);
	INT lX = cRc.left;
	INT lY = cRc.top;
	INT lWidth = cRc.right - cRc.left;
	INT lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	const char*	wndclass ="TEXT_CONSOLE";

	// Register the windows class
	WNDCLASS wndClass = { 0, TextConsole_WndProc, 0, 0, hInstance,
		NULL,
		LoadCursor( hInstance, IDC_ARROW ),
		GetStockBrush(GRAY_BRUSH),
		NULL, wndclass };
	RegisterClass( &wndClass );

	// Set the window's initial style
	u32 dwWindowStyle = WS_OVERLAPPED | WS_CHILD | WS_VISIBLE;// | WS_CLIPSIBLINGS;// | WS_CLIPCHILDREN;

	// Set the window's initial width
	RECT rc;
	SetRect			( &rc, lX, lY, lWidth, lHeight );
//	AdjustWindowRect( &rc, dwWindowStyle, FALSE );

	// Create the render window
	m_hConsoleWnd = CreateWindow( wndclass, "XRAY Text Console", dwWindowStyle,
		lX, lY,
		lWidth, lHeight, *m_pMainWnd,
		0, hInstance, 0L );
	//---------------------------------------------------------------------------
	R_ASSERT2(m_hConsoleWnd, "Unable to Create TextConsole Window!");
};
//-------------------------------------------------------------------------------------------
LRESULT CALLBACK TextConsole_LogWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void	CTextConsole::CreateLogWnd()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	//----------------------------------
	RECT cRc;
	GetClientRect(m_hConsoleWnd, &cRc);
	INT lX = cRc.left;
	INT lY = cRc.top;
	INT lWidth = cRc.right - cRc.left;
	INT lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	const char*	wndclass ="TEXT_CONSOLE_LOG_WND";

	// Register the windows class
	WNDCLASS wndClass = { 0, TextConsole_LogWndProc, 0, 0, hInstance,
		NULL,
		LoadCursor( NULL, IDC_ARROW ),
		GetStockBrush(BLACK_BRUSH),
		NULL, wndclass };
	RegisterClass( &wndClass );

	// Set the window's initial style
	u32 dwWindowStyle = WS_OVERLAPPED | WS_CHILD | WS_VISIBLE;// | WS_CLIPSIBLINGS;
//	u32 dwWindowStyleEx = WS_EX_CLIENTEDGE;

	// Set the window's initial width
	RECT rc;
	SetRect			( &rc, lX, lY, lWidth, lHeight );
//	AdjustWindowRect( &rc, dwWindowStyle, FALSE );

	// Create the render window
	m_hLogWnd = CreateWindow(wndclass, "XRAY Text Console Log", dwWindowStyle,
		lX, lY,
		lWidth, lHeight, m_hConsoleWnd,
		0, hInstance, 0L );
	//---------------------------------------------------------------------------
	R_ASSERT2(m_hLogWnd, "Unable to Create TextConsole Window!");
	//---------------------------------------------------------------------------
	ShowWindow(m_hLogWnd, SW_SHOW); 
	UpdateWindow(m_hLogWnd);
	//-----------------------------------------------
	LOGFONT lf; 
	lf.lfHeight = -12; 
	lf.lfWidth = 0;
	lf.lfEscapement = 0; 
	lf.lfOrientation = 0; 
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = 0; 
	lf.lfUnderline = 0; 
	lf.lfStrikeOut = 0; 
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_STRING_PRECIS;
	lf.lfClipPrecision = CLIP_STROKE_PRECIS;	
	lf.lfQuality = DRAFT_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	xr_sprintf(lf.lfFaceName,sizeof(lf.lfFaceName),"");

	m_hLogWndFont = CreateFontIndirect(&lf);
	R_ASSERT2(m_hLogWndFont, "Unable to Create Font for Log Window");
	//------------------------------------------------
	m_hDC_LogWnd = GetDC(m_hLogWnd);
	R_ASSERT2(m_hDC_LogWnd, "Unable to Get DC for Log Window!");
	//------------------------------------------------
	m_hDC_LogWnd_BackBuffer = CreateCompatibleDC(m_hDC_LogWnd);
	R_ASSERT2(m_hDC_LogWnd_BackBuffer, "Unable to Create Compatible DC for Log Window!");
	//------------------------------------------------
	GetClientRect(m_hLogWnd, &cRc);
	lWidth = cRc.right - cRc.left;
	lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	m_hBB_BM = CreateCompatibleBitmap(m_hDC_LogWnd, lWidth, lHeight);
	R_ASSERT2(m_hBB_BM, "Unable to Create Compatible Bitmap for Log Window!");
	//------------------------------------------------
	m_hOld_BM = (HBITMAP)SelectObject(m_hDC_LogWnd_BackBuffer, m_hBB_BM);
	//------------------------------------------------
	m_hPrevFont = (HFONT)SelectObject(m_hDC_LogWnd_BackBuffer, m_hLogWndFont);
	//------------------------------------------------
	SetTextColor(m_hDC_LogWnd_BackBuffer, RGB(255, 255, 255));
	SetBkColor(m_hDC_LogWnd_BackBuffer, RGB(1, 1, 1));
	//------------------------------------------------
	m_hBackGroundBrush = GetStockBrush(BLACK_BRUSH);
}

void CTextConsole::Initialize()
{
	inherited::Initialize();
	if (!strstr(Core.Params, "-hide"))
	{
		m_pMainWnd         = &Device.m_hWnd;
		m_dwLastUpdateTime = Device.dwTimeGlobal;
		m_last_time        = Device.dwTimeGlobal;

	
		CreateConsoleWnd();
		CreateLogWnd();
		ShowWindow(m_hConsoleWnd, SW_SHOW);
		UpdateWindow(m_hConsoleWnd);
	}
 
	m_server_info.ResetData();
}

void CTextConsole::Destroy()
{
	inherited::Destroy();	

	SelectObject( m_hDC_LogWnd_BackBuffer, m_hPrevFont );
	SelectObject( m_hDC_LogWnd_BackBuffer, m_hOld_BM );

	if ( m_hBB_BM )           DeleteObject( m_hBB_BM );
	if ( m_hOld_BM )          DeleteObject( m_hOld_BM );
	if ( m_hLogWndFont )      DeleteObject( m_hLogWndFont );
	if ( m_hPrevFont )        DeleteObject( m_hPrevFont );
	if ( m_hBackGroundBrush ) DeleteObject( m_hBackGroundBrush );

	ReleaseDC( m_hLogWnd, m_hDC_LogWnd_BackBuffer );
	ReleaseDC( m_hLogWnd, m_hDC_LogWnd );

	DestroyWindow( m_hLogWnd );
	DestroyWindow( m_hConsoleWnd );
}

void CTextConsole::OnRender() {} //disable ÑConsole::OnRender()

void CTextConsole::OnPaint()
{		 
	if (strstr(Core.Params, "-hide"))
		return;

	RECT wRC;
	PAINTSTRUCT ps;
	BeginPaint( m_hLogWnd, &ps );

	if ( /*m_bNeedUpdate*/ Device.dwFrame % 2 )
	{
//		m_dwLastUpdateTime = Device.dwTimeGlobal;
//		m_bNeedUpdate = false;
		
		GetClientRect( m_hLogWnd, &wRC );
		DrawLog( m_hDC_LogWnd_BackBuffer, &wRC );
	}
	else
	{
		wRC = ps.rcPaint;
	}
	
	
	BitBlt(	m_hDC_LogWnd,
			wRC.left, wRC.top,
			wRC.right - wRC.left, wRC.bottom - wRC.top,
			m_hDC_LogWnd_BackBuffer,
			wRC.left, wRC.top,
			SRCCOPY); //(FullUpdate) ? SRCCOPY : NOTSRCCOPY);
/*
	Msg ("URect - %d:%d - %d:%d", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
*/
	EndPaint( m_hLogWnd, &ps );
}

extern u32 shedule_time_ms = 0;
extern u32 updataCL_time_ms = 0;
extern u32 shedule_count = 0;

u32 time_update = 0;
string128 updateCL;
string128 updateSH;
string128 updateSH_count;

float last_fps = 0;

#include "IGame_Persistent.h"

void CTextConsole::DrawLog( HDC hDC, RECT* pRect )
{
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);

	RECT wRC = *pRect;
	GetClientRect(m_hLogWnd, &wRC);
	FillRect(hDC, &wRC, m_hBackGroundBrush);

	int Width = wRC.right - wRC.left;
	int Height = wRC.bottom - wRC.top;
	wRC = *pRect;
	
	int y_top_max = (int)(0.48f * Height);

	//---------------------------------------------------------------------------------
	LPCSTR s_edt = ec().str_edit();
	LPCSTR s_cur = ec().str_before_cursor();

	u32 cur_len = xr_strlen( s_cur ) + xr_strlen( ch_cursor ) + 1;
	PSTR buf = (PSTR)_alloca( cur_len * sizeof(char) );
	xr_strcpy( buf, cur_len, s_cur );
	xr_strcat( buf, cur_len, ch_cursor );
	buf[cur_len-1] = 0;

	u32 cur0_len = xr_strlen( s_cur );

	int xb = 25;
	
	SetTextColor( hDC, RGB(255, 255, 255) );
	TextOut( hDC, xb, Height-tm.tmHeight-1, buf, cur_len-1 );
	buf[ cur0_len ] = 0;
	
	SetTextColor(hDC, RGB(0, 0, 0));
	TextOut( hDC, xb, Height-tm.tmHeight-1, buf, cur0_len );


	SetTextColor( hDC, RGB(255, 255, 255) );
	TextOut( hDC, 0, Height-tm.tmHeight-3, ioc_prompt, xr_strlen(ioc_prompt) ); // ">>> "

	SetTextColor( hDC, (COLORREF)bgr2rgb(get_mark_color( mark11 )) );
	TextOut( hDC, xb, Height-tm.tmHeight-3, s_edt, xr_strlen(s_edt) );

	SetTextColor( hDC, RGB(205, 205, 225) );
	u32 log_line = LogFile->size()-1;
	string16 q, q2;
	itoa( log_line, q, 10 );
	xr_strcpy( q2, sizeof(q2), "[" );
	xr_strcat( q2, sizeof(q2), q );
	xr_strcat( q2, sizeof(q2), "]" );
	u32 qn = xr_strlen( q2 );

	TextOut( hDC, Width - 8 * qn, Height-tm.tmHeight-tm.tmHeight, q2, qn );

	int ypos = Height - tm.tmHeight - tm.tmHeight;

	if (!psDeviceFlags.test(rsProfiler))
	{
		for (int i = LogFile->size() - 1 - scroll_delta; i >= 0; --i)
		{
			ypos -= tm.tmHeight;
			if (ypos < y_top_max)
			{
				break;
			}

			LPCSTR ls = ((*LogFile)[i]).c_str();

			if (!ls)
			{
				continue;
			}
			Console_mark cm = (Console_mark)ls[0];
			COLORREF     c2 = (COLORREF)bgr2rgb(get_mark_color(cm));
			SetTextColor(hDC, c2);
			u8 b = (is_mark(cm)) ? 2 : 0;
			LPCSTR pOut = ls + b;

			BOOL res = TextOut(hDC, 10, ypos, pOut, xr_strlen(pOut));
			if (!res)
			{
				R_ASSERT2(0, "TextOut(..) return NULL");
			}
		}

		if (g_pGameLevel && last_fps != Device.Statistic->fFPS)
		{
			last_fps = Device.Statistic->fFPS;
			
			string32 fps = {0};
			itoa(u32(last_fps), fps, 10);
			
			string32 full_fps = {0};
			xr_strcpy(full_fps, "FPS: ");
			xr_strcat(full_fps, fps);
			
			if (last_fps < 30)
			{
				SetTextColor(hDC, RGB(255, 0, 0));
				TextOut(hDC, 10, 5, full_fps, xr_strlen(full_fps));
				//m_server_info.AddItem("FPS: ", fps, RGB(255, 0, 0));
			}
			else
			{
				SetTextColor(hDC, RGB(0, 255, 0));
				TextOut(hDC, 10, 5, full_fps, xr_strlen(full_fps));
				//m_server_info.AddItem("FPS: ", fps, RGB(0, 255, 0));
			}	
		}


		if (g_pGameLevel && (Device.dwTimeGlobal - m_last_time > 500))
		{
			m_last_time = Device.dwTimeGlobal;

			m_server_info.ResetData();
			g_pGameLevel->GetLevelInfo(&m_server_info);

			m_server_info.AddItem("updateCL", updateCL, RGB(0, 255, 255));
			m_server_info.AddItem("updateSH", updateSH, RGB(0, 255, 255));
			m_server_info.AddItem("updateSH_count", updateSH_count, RGB(0, 255, 255));

			if (g_pGamePersistent && &g_pGamePersistent->Environment())
			{
				if (g_pGamePersistent->Environment().PrewWeatherName.size() > 0)
				{
					string128 weather;
					xr_strcpy(weather, "[");
					xr_strcat(weather, g_pGamePersistent->Environment().PrewWeatherName.c_str());
					xr_strcat(weather, "]");

					m_server_info.AddItem("weather1", weather, RGB(255, 0, 255));
				}
				if (g_pGamePersistent->Environment().CurrentWeatherName.size() > 0)
				{
					string128 weather2;
					xr_strcpy(weather2, "[");
					xr_strcat(weather2, g_pGamePersistent->Environment().CurrentWeatherName.c_str());
					xr_strcat(weather2, "]");

					m_server_info.AddItem("weather2", weather2, RGB(255, 0, 255));
				}
			}
		}


		if (g_pGameLevel && (Device.dwTimeGlobal - time_update > 1000))
		{
			string64 tmp = { 0 };
			xr_strcpy(updateCL, "[");
			xr_strcat(updateCL, itoa(updataCL_time_ms, tmp, 10));
			xr_strcat(updateCL, "]");

			xr_strcpy(updateSH, "[");
			xr_strcat(updateSH, itoa(shedule_time_ms, tmp, 10));
			xr_strcat(updateSH, "]");

			xr_strcpy(updateSH_count, "[");
			xr_strcat(updateSH_count, itoa(shedule_count, tmp, 10));
			xr_strcat(updateSH_count, "]");

			time_update = Device.dwTimeGlobal;
			updataCL_time_ms = 0;
			shedule_time_ms = 0;
			shedule_count = 0;
		}
		 


		bool right = false;
		ypos = 25;
		for (u32 i = 0; i < m_server_info.Size(); ++i)
		{
			SetTextColor(hDC, m_server_info[i].color);

			if (!right)
				TextOut(hDC, 10, ypos, m_server_info[i].name, xr_strlen(m_server_info[i].name));
			else 
				TextOut(hDC, 300, ypos, m_server_info[i].name, xr_strlen(m_server_info[i].name));

			ypos += tm.tmHeight;
			if (ypos > y_top_max - 40)
			{
				right = true;
				ypos = 25;
				//break;
			}
		}
	}
	else
	{
		if (g_pGameLevel && (Device.dwTimeGlobal - m_last_time > 100))
		{
			m_last_time = Device.dwTimeGlobal;

			m_server_info.ResetData();
			
			g_pGameLevel->GetLevelInfo(&m_server_info);
		}


		bool right = false;

		ypos = 15;
		for (u32 i = 0; i < m_server_info.Size(); ++i)
		{
			SetTextColor(hDC, m_server_info[i].color);
			
			if (!right)
				TextOut(hDC, 10, ypos, m_server_info[i].name, xr_strlen(m_server_info[i].name));
			else 
				TextOut(hDC, 550, ypos, m_server_info[i].name, xr_strlen(m_server_info[i].name));

			ypos += tm.tmHeight;

			if (ypos > Height - 80)
			{
				ypos = 5;
				right = true;
				//break;
			}
			 
		}
	}

	
}
/*
void CTextConsole::IR_OnKeyboardPress( int dik ) !!!!!!!!!!!!!!!!!!!!!
{
	m_bNeedUpdate = true;
	inherited::IR_OnKeyboardPress( dik );
}
*/
void CTextConsole::OnFrame()
{
	if (strstr(Core.Params, "-hide"))
		return;

	inherited::OnFrame();
/*	if ( !m_bNeedUpdate && m_dwLastUpdateTime + 1000/g_svTextConsoleUpdateRate > Device.dwTimeGlobal )
	{
		return;
	}
*/	InvalidateRect( m_hConsoleWnd, NULL, FALSE );
	SetCursor( LoadCursor( NULL, IDC_ARROW ) );	
//	m_bNeedUpdate = true;
}
