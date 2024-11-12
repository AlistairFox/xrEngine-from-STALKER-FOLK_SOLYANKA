#include "stdafx.h"
#include "xrCompress.h"

#ifndef MOD_COMPRESS
	extern int				ProcessDifference();
#endif

int __cdecl main	(int argc, char* argv[])
{
	Debug._initialize	(false);
	Core._initialize	("xrCompress",0,FALSE);
	printf				("\n\n");

	LPCSTR params = GetCommandLine();
	xrCompressor		C;

	C.SetStoreFiles(NULL!=strstr(params,"-store"));
	C.SetStoreDDS(NULL != strstr(params, "-dds_store"));

#ifndef MOD_COMPRESS
	if(strstr(params,"-diff"))
	{
		ProcessDifference	();
	}

	if (strstr(params, "-unpack"))
	{


		LPCSTR						fsgame_ltx_name = "-fsltx ";
		string_path					fsgame = "";
		if (strstr(params, fsgame_ltx_name))
		{
			int						sz = xr_strlen(fsgame_ltx_name);
			sscanf(strstr(params, fsgame_ltx_name) + sz, "%[^ ] ", fsgame);
		}

		Core._initialize("xrCompress_Unpack", 0, TRUE, fsgame[0] ? fsgame : "NULL");

		FS_FileSet set;
		FS.file_list(set, "$game_data$");

		FS_FileSet set_mp;
		FS.file_list(set_mp, "$game_arch_mp$");

		Msg("Start Save Files: %d", set.size());
		printf("Files Size: %d \n", set.size());


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
			else
			{
				Msg("Cant Read: %s", s.name.c_str());
			}

			FS.r_close(r);
		}
 
	}
	else
#endif
	{
#ifndef MOD_COMPRESS
		if (argc<2)	
		{
			printf("ERROR: u must pass folder name as parameter.\n");
			printf("-diff /? option to get information about creating difference.\n");
			printf("-fast	- fast compression.\n");
			printf("-store	- store files. No compression.\n");
			printf("-ltx <file_name.ltx> - pathes to compress.\n");
			printf("\n");
			printf("LTX format:\n");
			printf("	[config]\n");
			printf("	;<path>     = <recurse>\n");
			printf("	.\\         = false\n");
			printf("	textures    = true\n");
			
			Core._destroy();
			return 3;
		}
#endif

		string_path		folder;		
		strconcat		(sizeof(folder),folder,argv[1],"\\");
		_strlwr_s		(folder,sizeof(folder));
		printf			("\nCompressing files (%s)...\n\n",folder);

		FS._initialize	(CLocatorAPI::flTargetFolderOnly, folder);
		FS.append_path	("$working_folder$","",0,false);

		C.SetFastMode	(NULL!=strstr(params,"-fast"));
		C.SetTargetName	(argv[1]);
		

		u16 size = 0;
		LPCSTR size_archive = strstr(params, "-max_size ") + 10;
		sscanf(size_archive, "%d", &size);
		C.SetMaxVolumeSize(1024 * 1024 * size);


		LPCSTR p		= strstr(params,"-ltx");

		if(0!=p)
		{
			string64				ltx_name;
			sscanf					(strstr(params,"-ltx ")+5,"%[^ ] ", ltx_name);

			CInifile ini			(ltx_name);
			printf					("Processing LTX...\n");
			C.ProcessLTX			(ini);
		}
		else
		{
			string64				header_name;
			sscanf					(strstr(params,"-header ")+8, "%[^ ] ", header_name);
			C.SetPackHeaderName		(header_name);
			C.ProcessTargetFolder	();
		}
	}

	

	Core._destroy		();
	return 0;
}
