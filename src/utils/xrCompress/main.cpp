#include "stdafx.h"
#include "xrCompress.h"
  
#include <iostream>
#include <string>
#include <windows.h>
#include <locale>
  
std::string ToUTF8(const std::wstring& wideString)
{
	if (wideString.empty()) return std::string();
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8String(sizeNeeded - 1, 0); // -1, чтобы не включать завершающий нулевой символ
	WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, &utf8String[0], sizeNeeded, nullptr, nullptr);
	return utf8String;
}

int __cdecl main(int argc, char* argv[])
{
	Debug._initialize(false);

	LPCSTR params = GetCommandLine();

	bool use_unpack = false;

	int response = MessageBox(
		NULL,								// Родительское окно (NULL означает, что его нет)
		"Распаковать?",      // Сообщение
		"да: Распаковать, нет: Запаковать",					// Заголовок окна
		MB_YESNO | MB_ICONQUESTION     // Кнопки (Да/Нет) и иконка вопроса
	);
	std::string filename = "gamedata", filearchivename = "packed", folder_out = "resources";
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	std::wcin.imbue(std::locale("")); // Устанавливаем локаль для ввода
	std::wcout.imbue(std::locale("")); // Устанавливаем локаль для вывода

	bool use_encript = false;

	switch (response)
	{
	case IDYES:
		use_unpack = true;
		break;
	case IDNO:
		use_unpack = false;
		std::cout << ToUTF8(L"Введите имя папки которую нужно запаковать: ");
		std::cin >> filename;

		std::cout << ToUTF8(L"Введите имя папки в которую запишутся архивы: ");
		std::cin >> folder_out;

		std::cout << ToUTF8(L"Введите имя архива: ");
		std::cin >> filearchivename;
  
		break;
	}

	 
	if (use_unpack)
	{
		string_path	fsgame = "fsgame.ltx";
		Core._initialize("xrCompress_Unpack", 0, TRUE, fsgame[0] ? fsgame : "NULL");

		FS_FileSet set;
		FS.file_list(set, "$game_data$");

		FS_FileSet set_mp;
		FS.file_list(set_mp, "$game_arch_mp$");


		for (auto s : set)
		{
			string_path file, file2;
			FS.update_path(file, "$game_data$", s.name.c_str());

			if (!FS.path_exist("$game_unpacked$"))
			{
				FS.append_path("$game_unpacked$", "", "unpacked", 0);
				FS.update_path(file2, "$game_unpacked$", s.name.c_str());
			}
			else
				FS.update_path(file2, "$game_unpacked$", s.name.c_str());

			IReader* r = FS.r_open(file);
			if (r)
			{
				printf("S: %s \n", s.name.c_str());
				Msg("S: %s", s.name.c_str());

				IWriter* w = FS.w_open(file2);
				w->w(r->pointer(), r->length());
				FS.w_close(w);
			}
			FS.r_close(r);
		}
	}
	else
	{
		Core._initialize("xrCompress", 0, FALSE);
		printf("xrCore Is Initialized \n\n");

		xrCompressor		C;
		C.SetStoreFiles(true);
		C.SetArchiveName(filearchivename.c_str());
		C.SetOutFolder(folder_out.c_str());
		printf("ArgV: %s", argv[1]);

		string_path		folder;
		strconcat(sizeof(folder), folder, filename.c_str(), "\\");
		_strlwr_s(folder, sizeof(folder));

		printf("\nCompressing files (%s)...\n\n", folder);

		FS._initialize(CLocatorAPI::flTargetFolderOnly, folder);
		FS.append_path("$working_folder$", "", 0, false);

		C.SetFastMode(false);
		C.SetTargetName(filename.c_str());
		C.SetMaxVolumeSize(1024 * 1024 * 1024);
   
		CInifile* ini = xr_new<CInifile> ("build_all.ltx");
		if (!ini)
			R_ASSERT(ini, "Ini file is no finded");
		
		printf("Processing LTX...\n");
		C.ProcessLTX(*ini);	 
	}

	Msg("Working Process Destroy");

	Core._destroy();
	return 0;

}