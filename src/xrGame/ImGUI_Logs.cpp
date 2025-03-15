#include "StdAfx.h"
 
#include "ImUtils.h"
#include "ImGUI_Loader.h"

// Log Menu
bool bOnlyError = false;
bool bAutoScroll = false;
bool bUseText = false;

#pragma optimize("", off)
#define RGBAColor(r,g,b,a) r/(float)255, g/(float)255, b/(float)255, a/(float)255



string512 search_str;
const ImVec4 getLogColor_new(const char* text)
{
	if (text == nullptr || xr_strlen(text) == 0)
		return ImVec4(RGBAColor(230, 230, 230, 255));

	xr_string TextEx = text;
	TextEx = TextEx.RemoveWhitespaces();
	size_t Pos = TextEx.find('|');

	while (Pos != xr_string::npos)
	{
		TextEx.erase(Pos, 1);
		Pos = TextEx.find('|');
	}

	char Word = TextEx[0];

	switch (Word)
	{
	case '~': return ImVec4(RGBAColor(248, 248, 49, 255));
	case '!': return ImVec4(RGBAColor(204, 102, 102, 255));
	case '@': return ImVec4(RGBAColor(125, 125, 241, 255));
	case '#': return ImVec4(RGBAColor(0, 222, 205, 155));
	case '%': return ImVec4(RGBAColor(202, 85, 219, 155));
	case '$': return ImVec4(RGBAColor(172, 172, 255, 255));
	case '*': return ImVec4(RGBAColor(248, 248, 49, 255));
	case '^': return ImVec4(RGBAColor(100, 246, 121, 255));
	case '&': return ImVec4(RGBAColor(255, 255, 0, 255));
	case '-': return ImVec4(RGBAColor(0, 255, 0, 255));
	case '+': return ImVec4(RGBAColor(84, 255, 255, 255));
	case '=': return ImVec4(RGBAColor(205, 205, 105, 255));
	case '/': return ImVec4(RGBAColor(146, 146, 252, 255));
	}

	return ImVec4(RGBAColor(230, 230, 230, 255));
}

const ImVec4 getLogColor(const char& c)
{
	switch (c)
	{
	case '~': return ImVec4(RGBAColor(248, 248, 49, 255));
	case '!': return ImVec4(RGBAColor(204, 102, 102, 255));
	case '@': return ImVec4(RGBAColor(125, 125, 241, 255));
	case '#': return ImVec4(RGBAColor(0, 222, 205, 155));
	case '$': return ImVec4(RGBAColor(172, 172, 255, 255));
	case '%': return ImVec4(RGBAColor(202, 85, 219, 155));
	case '^': return ImVec4(RGBAColor(100, 246, 121, 255));
	case '&': return ImVec4(RGBAColor(255, 255, 0, 255));
	case '*': return ImVec4(RGBAColor(187, 187, 187, 255));
	case '-': return ImVec4(RGBAColor(0, 255, 0, 255));
	case '+': return ImVec4(RGBAColor(84, 255, 255, 255));
	case '=': return ImVec4(RGBAColor(205, 205, 105, 255));
	case '/': return ImVec4(RGBAColor(146, 146, 252, 255));
	default: return ImVec4(RGBAColor(230, 230, 230, 255));
	}
}

void ShowLog(bool& show)
{
	// ImGui::SetNextWindowSize(ImVec2(800, 600), 0);

	ImguiWnd wnd("Окно Лога", &show);
	if (wnd.Collapsed)
		return;


	ImGui::Checkbox("only_error", &bOnlyError);
	ImGui::Checkbox("scrool_auto", &bAutoScroll);
	ImGui::Checkbox("use_search", &bUseText);

	if (ImGui::Button("ClearLog"))
	{
		LogFile->clear_not_free();
		FlushLog();
		Msg("* Log file has been cleaned successfully!");
	};

	if (bUseText)
		ImGui::InputText("#searcher", search_str, 512);

	ImGui::Spacing();
	if (ImGui::BeginChild("Log"))
	{
		for (auto Str : *LogFile)
		{
			if (bUseText)
			{
				if (strstr(Str.c_str(), search_str) != 0)
				{
					const char* s = Str.c_str();
					ImVec4 Color = getLogColor_new(s);
					ImGui::TextColored(Color, Str.c_str());
				}
			}
			else
				if (bOnlyError)
				{
					if (strncmp(Str.c_str(), "###", 3) == 0)
					{
						ImGui::Text(Str.c_str());
					}
					else if (strncmp(Str.c_str(), "~ ", 2) == 0)
					{
						ImVec4 Color = { 1,1,0,1 };
						ImGui::TextColored(Color, Str.c_str());
					}
					else
						if (strncmp(Str.c_str(), "* ", 2) == 0)
						{
							ImVec4 Color = { 0,1,0,1 };
							ImGui::TextColored(Color, Str.c_str());
						}
						else
							if (strncmp(Str.c_str(), "! ", 2) == 0)
							{
								ImVec4 Color = { 1,0,0,1 };
								ImGui::TextColored(Color, Str.c_str());
							}
				}
				else
				{
					ImVec4 Color = { 1,1,1,1 };

					if (strncmp(Str.c_str(), "###", 3) == 0)
					{
						Color = { 1,0,0,1 };
					}
					else if (strncmp(Str.c_str(), "~ ", 2) == 0)
					{
						Color = { 1,1,0,1 };
						ImGui::TextColored(Color, Str.c_str());
					}
					else if (strncmp(Str.c_str(), "! ", 2) == 0)
					{
						Color = { 1,0,0,1 };
					}
					else if (strncmp(Str.c_str(), "* ", 2) == 0)
					{
						Color = { 0,1,0,1 };
					}
					else if (strncmp(Str.c_str(), "##@", 3) == 0)
					{
						Color = { 1,1,0,1 };
					}
					else if (strncmp(Str.c_str(), "---", 3) == 0)
					{
						Color = { 0,1,0,1 };
					}

					try
					{
						ImGui::TextColored(Color, Str.c_str());
					}
					catch (...)
					{
						//	Msg("IMgui Crushed: %s", Str.c_str());
					}
				}
		}

		if (bAutoScroll)
			ImGui::SetScrollHereY();

	}
	ImGui::EndChild();
}

