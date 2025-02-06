#pragma once


ENGINE_API int g_current_renderer;

void ShowEditor();
bool IsEditor();
void Editor_OnFrame();


bool Editor_KeyPress(int key);
bool Editor_KeyRelease(int key);
bool Editor_KeyHold(int key);
bool Editor_MouseMove(int dx, int dy);
bool Editor_MouseWheel(int direction);
 
#pragma warning(disable:4995)
#include <imgui.h>
#pragma warning(default:4995)

namespace luabind {
    class object;
}

enum TypesNetworkAdmin
{
    eCoopType = 0,
    eAdminType = 1
};

struct ImguiWnd
{
    ImguiWnd(const char* name, bool* pOpen = nullptr) { Collapsed = !ImGui::Begin(name, pOpen); }
    ~ImguiWnd() { ImGui::End(); }

    bool Collapsed;
};


struct ImguiWndFlags
{
    ImguiWndFlags(const char* name, bool* pOpen, ImGuiWindowFlags& flags) { Collapsed = !ImGui::Begin(name, pOpen, flags); }
    ~ImguiWndFlags() { ImGui::End(); }

    bool Collapsed;
};

struct ImguiChieldWnd
{
    ImguiChieldWnd(const char* name, float posX, float posY) { Collapsed = !ImGui::BeginChild(name, ImVec2(posX, posY)); }
    void EndChield() { ImGui::EndChild(); }
    ~ImguiChieldWnd() { ImGui::EndChild(); }

    bool Collapsed;
};

xr_string to_string(const luabind::object& o, xr_string offset = xr_string());
xr_string toUtf8(const char* s);
xr_string Utf8ToWin1251(const char* s);

void MsgToALL(char* s, ...);