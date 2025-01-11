#include "stdafx.h"
#include "device.h"



  
#ifndef DEDICATED_SERVER
#include "IGame_Persistent.h"
extern ENGINE_API EditorStage imgui_stage = EditorStage::None;


// need for imgui
static INT64 g_Time = 0;
static INT64 g_TicksPerSecond = 0;

extern int g_current_renderer;

#include "backends\imgui_impl_dx11.h"
#include "backends\imgui_impl_dx9.h"
#include "backends\imgui_impl_win32.h"


#pragma warning(disable:4995)
#include <imgui.h>
#pragma warning(default:4995)


void ImGui_NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	RECT rect;
	GetClientRect(Device.m_hWnd, &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	if (g_TicksPerSecond == 0) {
		QueryPerformanceFrequency((LARGE_INTEGER*)&g_TicksPerSecond);
		QueryPerformanceCounter((LARGE_INTEGER*)&g_Time);
	}
	// Setup time step
	INT64 current_time;
	QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
	io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
	g_Time = current_time;

	// Start the frame
	if (g_current_renderer == 4)
		ImGui_ImplDX11_NewFrame();
	else
		ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	g_pGamePersistent->EditorOnFrame();

	ImGui::NewFrame();
}
 
void ImGui_EndFrame()
{
	ImGui::EndFrame();
}

void ImGUI_DeviceCreate()
{
	string_path path;
	FS.update_path(path, "$app_data_root$", "imgui.ini");

	ImGui::GetIO().IniFilename = xr_strdup(path);//path.c_str();
}

void ImGUI_DeviceDestroy()
{
	xr_free(ImGui::GetIO().IniFilename);
}
#else 
void ImGui_NewFrame()
{
	 
}

void ImGui_EndFrame()
{
	 
}

void ImGUI_DeviceCreate()
{
	 
}

void ImGUI_DeviceDestroy()
{
 
}
#endif 