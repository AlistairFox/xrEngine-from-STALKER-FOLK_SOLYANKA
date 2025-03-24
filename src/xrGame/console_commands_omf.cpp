#include "stdafx.h"
#include "../xrEngine/XR_IOConsole.h"
#include "../xrEngine/xr_ioc_cmd.h"

#include "../xrEngine/Fmesh.h"

class CCC_OMFS : public IConsole_Command
{
public:
	CCC_OMFS(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; }
	virtual void Execute(LPCSTR args)
	{
		FS_FileSet files;
		FS.file_list(files, "$game_meshes$");

		string_path file_folder_userdata;
		FS.update_path(file_folder_userdata, "$app_data_root$", "");

		for (auto file : files)
		{
			string_path filep;
			FS.update_path(filep, "$game_meshes$", file.name.c_str());



			if (strstr(file.name.c_str(), ".ogf") != 0)
			{
				string_path file_exp;
				xr_strcpy(file_exp, file_folder_userdata);

				xr_strcat(file_exp, "ogf_refferences\\");

				xr_strcat(file_exp, file.name.c_str());


				IWriter* write = FS.w_open(file_exp);

				IReader* R = FS.r_open(filep);
				if (R)
				{
					Msg("Reading File: %s", filep);
					if (R->find_chunk(OGF_S_MOTION_REFS2))
					{
						u32 set_cnt = R->r_u32();

						string_path		nm;
						for (u32 k = 0; k < set_cnt; ++k)
						{
							R->r_stringZ(nm, sizeof(nm));
							xr_strcat(nm, ".omf");

							string128 tmp;
							sprintf_s(tmp, "[%u] OMF: %s", k, nm);
							write->w_string(tmp);
							//Msg("[%u] OMF: %s", k, nm);
						}
					}
				}
				FS.r_close(R);

				FS.w_close(write);
			}

		}
	}
};

class CCC_OMFS_HANDS : public IConsole_Command
{
public:
	CCC_OMFS_HANDS(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; }
	virtual void Execute(LPCSTR args)
	{
		FS_FileSet files;
		FS.file_list(files, "$game_meshes$");

		string_path file_folder_userdata;
		FS.update_path(file_folder_userdata, "$app_data_root$", "");

		for (auto file : files)
		{
			string_path filep;
			FS.update_path(filep, "$game_meshes$", file.name.c_str());

			if (strstr(file.name.c_str(), "dynamics\\weapons\\wpn_hand") != 0)
			{
				string_path file_exp;
				xr_strcpy(file_exp, file_folder_userdata);
				xr_strcat(file_exp, "hands_refferences\\");
				xr_strcat(file_exp, file.name.c_str());


				IReader* R = FS.r_open(filep);

				if (R)
				{
					if (R->find_chunk(OGF_S_MOTION_REFS2))
					{
						u32 set_cnt = R->r_u32();
						string_path		nm;
						xr_vector<shared_str> names;


						CInifile* write = xr_new<CInifile>(file_exp);
						for (auto S : names)
						{
							string128 tmp;
							sprintf_s(tmp, "%s", *S);
							write->w_string("OMF_REFS", "", tmp);
						}
						write->save_as();
						xr_delete(write);
					}
				}

				FS.r_close(R);


			}

		}
	}
};
 
extern int bDebugHud = 0;
void register_console_ogfs()
{
	CMD4(CCC_Integer, "debug_playerhud", &bDebugHud, 0, 1);

	CMD1(CCC_OMFS, "ogf_refs");
	CMD1(CCC_OMFS_HANDS, "ogfs_refs_hands");
}