#include "stdafx.h"
#include "ImGUI_Loader.h"

#include "Level.h"
#include "xr_level_controller.h"
#include "../xrEngine/xr_input.h"
#include "ImUtils.h"

static bool isAlt = false;
ENGINE_API extern EditorStage imgui_stage;

bool IsEditorActive()
{
    if (g_current_renderer != 4)
        return false;

    if (g_dedicated_server)
        return false;

    return imgui_stage == EditorStage::Full || (imgui_stage == EditorStage::Light && isAlt);
}

bool IsEditor() { return imgui_stage != EditorStage::None; }


void ShowWeatherEditor(bool& show);
void ShowLog(bool& show);

bool show_weather_window = false;
bool show_spawn_window = false;
bool show_logs = false;

void ShowEditor()
{ 
    if (g_current_renderer != 4)
        return;
    
    ImguiWnd wnd("Main");
    if (wnd.Collapsed)
        return;

    ImGui::Text(u8"Advanced X-Ray Editor");
 
    auto io = ImGui::GetIO();
    
    ImGui::Checkbox("Weather Editor", &show_weather_window );
    ImGui::Checkbox("Spawn Meny", &show_spawn_window);
    ImGui::Checkbox("Logs", &show_logs);

    if (ImGui::Button("Atmosfear 3 Options (Server)"))
    {
        luabind::functor<void>	funct;
        ai().script_engine().functor("atmosfear.OnButton_af_options_clicked", funct);
        funct();
    }

    if (show_weather_window)
        ShowWeatherEditor(show_weather_window);

    if (show_spawn_window)
        RenderSpawnManagerWindow(show_spawn_window);

    if (show_logs)
        ShowLog(show_logs);

    bool full = imgui_stage == EditorStage::Full;
    //if (ImGui::Checkbox("Active", &full))
    //    imgui_stage = full ? EditorStage::Full : EditorStage::None;
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

bool isRControl = false, isLControl = false, isRShift = false, isLShift = false;
bool Editor_KeyPress(int key)
{
     if (g_current_renderer != 4)
        return false;
 
    if (g_dedicated_server)
        return false;

    if (!Level().game || !Level().game->local_player)
        return false;

    if (!Level().game->local_player->testFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS))
    {
        imgui_stage = EditorStage::None;
        return false;
    }

    switch (key)
    {
        case (DIK_ESCAPE):
        {
            imgui_stage = EditorStage::None;
        }
        break;

        case (DIK_F12):
        {
            if (imgui_stage == EditorStage::Full)
                imgui_stage = EditorStage::None;
            else
                imgui_stage = EditorStage::Full;
         }break;
    }

    if (!IsEditorActive())
        return false;

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = true;

    switch (key)
    {

    case DIK_RALT:
    case DIK_LALT:
    case DIK_F12:
        break;
    case DIK_RCONTROL:
        isRControl = true;
        io.KeyCtrl = true;
        break;
    case DIK_LCONTROL:
        isLControl = true;
        io.KeyCtrl = true;
        break;
    case DIK_RSHIFT:
        isRShift = true;
        io.KeyShift = true;
        break;
    case DIK_LSHIFT:
        isLShift = true;
        io.KeyShift = true;
        break;
    case MOUSE_1:
        io.MouseDown[0] = true;
        break;
    case MOUSE_2:
        io.MouseDown[1] = true;
        break;
    case MOUSE_3:
        io.MouseDown[2] = true;
        break;
    case DIK_NUMPAD0:
        io.AddInputCharacter('0');
        break;
    case DIK_NUMPAD1:
        io.AddInputCharacter('1');
        break;
    case DIK_NUMPAD2:
        io.AddInputCharacter('2');
        break;
    case DIK_NUMPAD3:
        io.AddInputCharacter('3');
        break;
    case DIK_NUMPAD4:
        io.AddInputCharacter('4');
        break;
    case DIK_NUMPAD5:
        io.AddInputCharacter('5');
        break;
    case DIK_NUMPAD6:
        io.AddInputCharacter('6');
        break;
    case DIK_NUMPAD7:
        io.AddInputCharacter('7');
        break;
    case DIK_NUMPAD8:
        io.AddInputCharacter('8');
        break;
    case DIK_NUMPAD9:
        io.AddInputCharacter('9');
        break;

    default:
        if (key < 512)
            io.KeysDown[key] = true;
        if (key == DIK_SPACE && (pInput->iGetAsyncKeyState(DIK_RWIN) || pInput->iGetAsyncKeyState(DIK_LWIN)))
            ActivateKeyboardLayout((HKL)HKL_NEXT, 0);
        else {
            uint16_t ch[1];
            int n = pInput->scancodeToChar(key, ch);
            if (n > 0) {
                wchar_t buf;
                MultiByteToWideChar(CP_ACP, 0, (char*)ch, n, &buf, 1);
                io.AddInputCharacter(buf);
            }
        }
    }
    return true;
}

bool Editor_KeyRelease(int key)
{
    if (g_current_renderer != 4)
        return false;


    if (g_dedicated_server)
        return false;

    /*
    if (key == DIK_RALT || key == DIK_LALT)
    {
        isAlt = false;
        stage = EditorStage::None;
    }
    */

    bool active = IsEditorActive();

    ImGuiIO& io = ImGui::GetIO();
    if (!active)
        io.MouseDrawCursor = false;

    switch (key) {
    case DIK_RCONTROL:
        isRControl = false;
        io.KeyCtrl = isRControl || isLControl;
        break;
    case DIK_LCONTROL:
        isLControl = false;
        io.KeyCtrl = isRControl || isLControl;
        break;
    case DIK_RSHIFT:
        isRShift = false;
        io.KeyShift = isRShift || isLShift;
        break;
    case DIK_LSHIFT:
        isLShift = false;
        io.KeyShift = isRShift || isLShift;
        break;
    case MOUSE_1:
        io.MouseDown[0] = false;
        break;
    case MOUSE_2:
        io.MouseDown[1] = false;
        break;
    case MOUSE_3:
        io.MouseDown[2] = false;
        break;
    default:
        if (key < 512)
            io.KeysDown[key] = false;
    }
    return active;
}

bool Editor_KeyHold(int key)
{
    if (g_current_renderer != 4)
        return false;

    if (!IsEditorActive())
        return false;
    return true;
}

bool Editor_MouseMove(int dx, int dy)
{
    if (g_current_renderer != 4)
        return false;

    if (!IsEditorActive())
        return false;

    ImGuiIO& io = ImGui::GetIO();
    POINT p;
    GetCursorPos(&p);
    io.MousePos.x = p.x;
    io.MousePos.y = p.y;
    return true;
}

static int s_direction{};

bool Editor_MouseWheel(int direction)
{
    if (g_current_renderer != 4)
        return false;

    if (!IsEditorActive())
        return false;

    s_direction = direction;

    return true;
}

void Editor_OnFrame()
{
    if (g_current_renderer != 4)
        return;

    if (s_direction)
    {
        ImGuiIO& io = ImGui::GetIO();

        io.MouseWheel += s_direction > 0 ? +1.0f : -1.0f;
        s_direction = 0;
    }
}


xr_string toUtf8(const char* s)
{
    // TO ACP
    static xr_vector<wchar_t> buf;
    int n = MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
    buf.resize(n);
    MultiByteToWideChar(CP_ACP, 0, s, -1, &buf[0], buf.size());

    // TO UTF8
    xr_string result;
    n = WideCharToMultiByte(CP_UTF8, 0, &buf[0], buf.size(), nullptr, 0, nullptr, nullptr);
    result.resize(n);
    n = WideCharToMultiByte(CP_UTF8, 0, &buf[0], buf.size(), &result[0], result.size(), nullptr, nullptr);
    return result;
}