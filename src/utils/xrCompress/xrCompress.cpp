#include "stdafx.h"

#pragma warning(disable: 4995)
#pragma warning(disable: 4996)
#include "xrCompress.h"
#include "mutex"
#include "ppl.h"

#define CFS_ARCHIVE_SE7	2610

std::mutex lock_processfile;
 
xrCompressor::xrCompressor() : fs_pack_writer(NULL),bFast(false),files_list(NULL),folders_list(NULL),bStoreFiles(false),pPackHeader(NULL),config_ltx(NULL)
{
	bytesSRC		= 0;
	bytesDST		= 0;
	filesTOTAL		= 0;
	filesSKIP		= 0;
	filesVFS		= 0;

	dwTimeStart		= 0;

	XRP_MAX_SIZE	= 1024*1024*640; // bytes (640Mb)
}

xrCompressor::~xrCompressor()
{
	if(pPackHeader)
		FS.r_close	(pPackHeader);
}

bool is_tail(LPCSTR name, LPCSTR tail, const u32 tlen)
{
	LPCSTR p			= strstr(name,tail);
	if(!p)				return false;

	u32 nlen			= xr_strlen(name);
	return				(p==name+nlen-tlen);
		
}

bool xrCompressor::testSKIP(LPCSTR path)
{
	string256			p_name;
	string256			p_ext;
	_splitpath			(path, 0, 0, p_name, p_ext );

	if (strstr(path,"textures\\lod\\"))				return true;
	if (strstr(path,"textures\\det\\"))				return true;

	if (	stricmp(p_ext,".thm") && 
			strstr(path,"textures\\terrain\\terrain_") && 
			!is_tail(p_name,"_mask",5) 
		)
		return true;

	if (strstr(path,"textures\\") && is_tail(p_name, "_nmap",5) && !strstr(p_name, "water_flowing_nmap") )
		return true;
	
	if (0==stricmp(p_name,"build")) 
	{
		if (0==stricmp(p_ext,".aimap")	)	return true;
		if (0==stricmp(p_ext,".cform")	)	return true;
		if (0==stricmp(p_ext,".details"))	return true;
		if (0==stricmp(p_ext,".prj")	)	return true;
		if (0==stricmp(p_ext,".lights")	)	return false;
	}
	if (0==stricmp(p_name,"do_light") && 0==stricmp(p_ext,".ltx"))	return true;

	if (0==stricmp(p_ext,".txt"))	return true;
	if (0==stricmp(p_ext,".tga"))	return true;
	if (0==stricmp(p_ext,".db"))	return true;
	if (0==stricmp(p_ext,".smf"))	return true;

	if ('~'==p_ext[1])				return true;
	if ('_'==p_ext[1])				return true;
	if (0==stricmp(p_ext,".vcproj"))	return true;
	if (0==stricmp(p_ext,".sln"))	return true;
	if (0==stricmp(p_ext,".old"))	return true;
	if (0==stricmp(p_ext,".rc"))	return true;
 
	return false;
}

bool xrCompressor::testVFS(LPCSTR path)
{
	string256			p_ext;
	_splitpath(path, 0, 0, 0, p_ext);
 
	if (!stricmp(p_ext,".xml"))
		return			(false);

	if (!stricmp(p_ext,".ltx"))
		return			(FALSE);

	if (!stricmp(p_ext,".script"))
		return			(FALSE);
	
	
	if (!stricmp(p_ext, ".dds"))
	 	return			(FALSE);

	//LEVELS
	if (!stricmp(p_ext, ".geom"))
		return (FALSE);

	if (!stricmp(p_ext, ".geomx"))
		return (FALSE);

	if (!stricmp(p_ext, ".cform"))
		return (FALSE);

	//OGF
	if (!stricmp(p_ext, ".ogf"))
	 	return (FALSE);
	
	 if (!stricmp(p_ext, ".omf"))
	 	return (FALSE);

	//SOUND
	if (!stricmp(p_ext, ".ogg"))
		return (FALSE);
 
	return				(TRUE);
}

bool xrCompressor::testEqual(LPCSTR path, IReader* base)
{
	bool res			= false;
	IReader*	test	= FS.r_open	(path);

	if(test->length() == base->length())
	{
		if( 0==memcmp(test->pointer(),base->pointer(),base->length()) )
			res			= TRUE;
	}
	FS.r_close			(test);
	return				res;
}

void MsgComressor(const char* format, ...)
{
	va_list		mark;
	string2048	text_buffer;
	va_start(mark, format);
	int sz = _vsnprintf(text_buffer, sizeof(text_buffer) - 1, format, mark);
	text_buffer[sizeof(text_buffer) - 1] = 0;
	va_end(mark);

	printf(text_buffer);
	printf("\n");
	Msg(text_buffer);
}



void xrCompressor::ProcessFile(LPCSTR path, xr_vector<PackedData>& toPack, size_t& Accum)
{
	filesTOTAL++;
 	string_path		fn;
	strconcat(sizeof(fn), fn, target_name.c_str(), "\\", path);
	IReader* src = FS.r_open(fn);

	if (testSKIP(path) || ::GetFileAttributes(fn) == u32(-1) || 0 == src)
	{
		filesSKIP++;
		MsgComressor("Skip: %s", path);
		return;
	}

	u32			c_crc32 = crc32(src->pointer(), src->length());
	u32			c_ptr = 0;
	u32			c_size_real = 0;
	u32			c_size_compressed = 0;

	u8* ptr = (u8*)src->pointer();
	bytesSRC += src->length();

	xr_vector<u8> data(src->length());
	CopyMemory(data.data(), ptr, src->length());

	c_ptr = fs_pack_writer->tell();
	c_size_real = src->length();

	PackedData data_toPack;

	xr_strcpy(data_toPack.FileName, path);

	if (testVFS(path))
	{
		filesVFS++;
		c_size_compressed = c_size_real;
		MsgComressor("VFSS: %7u kb to %7u kb | Rate: %3.0f | File: %s", c_size_real / 1024, c_size_compressed / 1024, (float(c_size_compressed) / float(c_size_real)) * 100.0f, path);
 		data_toPack.c_crc32 = c_crc32;
		data_toPack.c_size_compressed = c_size_compressed;
		data_toPack.c_size_real = c_size_real;
		data_toPack.packed_data = data;
	}
	else
	{
		bool noCompress = false;
		if (c_size_real != 0)
		{
			// zlib
			size_t size_compressed = zng_compressBound(c_size_real);
			u8* CompressedData = xr_alloc<u8>(size_compressed);
			R_ASSERT(Z_OK == zng_compress2(CompressedData, &size_compressed, data.data(), c_size_real, level_compression));
			c_size_compressed = size_compressed;

			if ((c_size_compressed + 16) >= c_size_real)
			{
				noCompress = true;
			}
			else
			{
				// Compressed OK - optimize
 				data_toPack.packed_data.resize(c_size_compressed);
				CopyMemory(data_toPack.packed_data.data(), CompressedData, c_size_compressed);
				data_toPack.c_crc32 = c_crc32;
				data_toPack.c_size_compressed = c_size_compressed;
				data_toPack.c_size_real = c_size_real;
				xr_delete(CompressedData);
 				MsgComressor("Zlib: %7u kb to %7u kb | Rate: %3.0f | File: %s", c_size_real / 1024, c_size_compressed / 1024, (float(c_size_compressed) / float(c_size_real)) * 100.0f, path);
			}
		}
		else
			noCompress = true;

		if (noCompress)
		{
			filesVFS++;
			c_size_compressed = c_size_real;
			data_toPack.c_crc32 = c_crc32;
			data_toPack.c_size_compressed = c_size_compressed;
			data_toPack.c_size_real = c_size_real;
			data_toPack.packed_data = data;
			MsgComressor("VFSS: %7u kb to %7u kb | Rate: %3.0f | File: %s", c_size_real / 1024, c_size_compressed / 1024, (float(c_size_compressed) / float(c_size_real)) * 100.0f, path);
		}
	}

	lock_processfile.lock();
	Accum += data_toPack.c_size_compressed;
	lock_processfile.unlock();

	toPack.push_back(data_toPack);
	FS.r_close(src);
}



void xrCompressor::write_file_header(LPCSTR file_name, const u32 &crc, const u32 &ptr, const u32 &size_real, const u32 &size_compressed)
{
	DescriptData data;
 	xr_strcpy(data.file_path, file_name);
	data.crc = crc;
	data.ptr = ptr;
	data.size_compressed = size_compressed;
	data.size_real = size_real;	 

	fs_desc.w			(&data, sizeof(data) );
}

void xrCompressor::OpenPack(LPCSTR tgt_folder, int num)
{
	VERIFY			(0==fs_pack_writer);

	string_path		fname = { 0 };
	string128		s_num = { 0 };

	xr_strcat(fname, OutFolder.c_str());
	xr_strcat(fname, "\\");

	string64				c_name = { 0 };
	xr_strcat(fname, ArchiveName.c_str());

	xr_strcat(fname, ".db");
	xr_strcat(fname, itoa(num, s_num, 10));

	Msg("Open Pack: %s", fname);

	fs_pack_writer	= FS.w_open	(fname);
	fs_desc.clear	();
 
	bytesSRC		= 0;
	bytesDST		= 0;
	filesTOTAL		= 0;
	filesSKIP		= 0;
	filesVFS		= 0;
 
	dwTimeStart		= timeGetTime();

	char* text_ltx = 
		"[header]							  \n"
		"auto_load = true					  \n"
		"level_name = single				  \n"
		"level_ver = 1.0					  \n"
		"entry_point = $fs_root$\\gamedata\\  \n"
		"creator = \"\"						  \n"
		"link = \"\"						  \n";

	MsgComressor("LTX: %s", text_ltx);

	IReader * file = new IReader(text_ltx, xr_strlen(text_ltx));

	config_ltx = new CInifile(file);
 
	//write pack header without compression
 	//if(config_ltx && config_ltx->section_exist("header"))
	{
		CMemoryWriter			W;
		// CInifile::Sect&	S		= config_ltx->r_section("header");
 		
		// string4096				buff;
		// xr_sprintf				(buff,"[%s]", S.Name.c_str());
		// W.w_string				(buff);
	 	// 
		// for(auto & items : S.Data)
		// {
		// 	xr_sprintf					(buff, "%s = %s", items.first.c_str(), items.second.c_str());
		// 	W.w_string				(buff);
 		// }
		// W.seek						(0);

		W.w_string(" [header] ");
		W.w_string("auto_load = true");
		W.w_string("level_name = single");
		W.w_string("level_ver = 1.0");
		W.w_string("entry_point = $fs_root$\\gamedata\\");
		W.w_string("creator = \"\"	");
		W.w_string("link = \"\" ");

		IReader	R(W.pointer(), W.size());

		printf						("...Writing pack header\n");

		IReader* test = new IReader(W.pointer(), W.size());
		if (test)
		{
			while (test->elapsed())
			{
				xr_string s;
				test->r_string(s);
				MsgComressor("Writed: %s", s.c_str());
			}
		}

		fs_pack_writer->open_chunk	(CFS_HeaderChunkID);					//CFS_HeaderChunkID
		fs_pack_writer->w			(R.pointer(), R.length());
		fs_pack_writer->close_chunk	();
	}
	//else
	//if(pPackHeader)
	//{
	//	printf						("...Writing pack header\n");
	//	fs_pack_writer->open_chunk	(CFS_HeaderChunkID);
	//	fs_pack_writer->w			(pPackHeader->pointer(), pPackHeader->length());
	//	fs_pack_writer->close_chunk	();
	//}else
	//	printf			("...Pack header not found\n");

//	g_dummy_stuff	= _dummy_stuff_subst;

	fs_pack_writer->open_chunk	(0);
}
void xrCompressor::SetPackHeaderName(LPCSTR n)
{
	pPackHeader		= FS.r_open	(n);
	R_ASSERT2		(pPackHeader, n);
}

   
void xrCompressor::ClosePack()
{
	fs_pack_writer->close_chunk	(); 

	// save list
	bytesDST		= fs_pack_writer->tell	();
	Msg				("...Writing pack desc");

	fs_pack_writer->w_chunk		( 1000 , fs_desc.pointer(), fs_desc.size());


	Msg				("Data size: %d. Desc size: %d.",bytesDST,fs_desc.size());
	FS.w_close		(fs_pack_writer);
	Msg				("Pack saved.");
}

void xrCompressor::PerformWork()
{
	MsgComressor("~~~ File List: %u", files_list->size());
	if (!files_list->empty() && target_name.size())
	{
		string256		caption;

		int pack_num = 0;
		OpenPack(target_name.c_str(), pack_num++);

		for (u32 it = 0; it < folders_list->size(); it++)
			write_file_header((*folders_list)[it], 0, 0, 0, 0);

		xr_vector<PackedData> toPack[32];
		xr_vector<PackedData> toPackGlobal;
		size_t DataAccumulated = 0;
		std::atomic<int> CurrentPos = 0;

		auto FunctionMT = [&](int ID)
		{
			for (;;)
			{
				int CurrentFile = CurrentPos.load();
				if (CurrentFile >= files_list->size())
					break;
				CurrentPos.fetch_add(1);

				auto File = (*files_list)[CurrentFile];
				ProcessFile(File, toPack[ID], DataAccumulated);


				// Window Info 
				xr_sprintf(caption, "Compress files: Accumulated:%u | %d/%d - %d%%", toPack[ID].size(), CurrentFile, files_list->size(), (it * 100) / files_list->size());
				SetWindowText(GetConsoleWindow(), caption);
			}
		};

 		xr_vector<std::thread*> threads_work;
  		for (auto I = 0; I < max_threads; I++)
			threads_work.push_back(new std::thread(FunctionMT, I));
	
		for (auto I = 0; I < max_threads; I++)
			threads_work[I]->join();

		//concurrency::parallel_for(size_t(0), size_t(max_threads), [&](size_t ID)
		//{
		//	FunctionMT(ID);
		//});

		u32 TotalFiles = 0;
		for (auto i = 0; i < max_threads; i++)
		{
			TotalFiles += toPack[i].size();
		}

		int ID = 0;
		for (auto i = 0; i < max_threads; i++)
		{
			for (auto P : toPack[i])
			{
				if (fs_pack_writer->tell() > XRP_MAX_SIZE)
				{
					DataAccumulated = 0;
					ClosePack();
					OpenPack(target_name.c_str(), pack_num++);
				}

				u32 c_ptr = fs_pack_writer->tell();
				fs_pack_writer->w(P.packed_data.data(), P.c_size_compressed);
				write_file_header(P.FileName, P.c_crc32, c_ptr, P.c_size_real, P.c_size_compressed);
				xr_sprintf(caption, "Export: R: %7u, C: %7u : %s", P.c_size_real, P.c_size_compressed, P.FileName);
				SetWindowText(GetConsoleWindow(), caption);
				ID++;
			}
		}

		ClosePack();
	}
	else
	{
		Msg("ERROR: folder not found : %s", *target_name);
	}
}

void xrCompressor::ProcessTargetFolder()
{
	// collect files
	files_list			= FS.file_list_open	("$target_folder$",FS_ListFiles);
	R_ASSERT2			(files_list,	"Unable to open folder!!!");
	// collect folders
	folders_list		= FS.file_list_open	("$target_folder$",FS_ListFolders);
	R_ASSERT2			(folders_list,	"Unable to open folder!!!");
	// compress
	PerformWork			();
	// free lists
	FS.file_list_close	(folders_list);
	FS.file_list_close	(files_list);
}

void xrCompressor::GatherFiles(LPCSTR path)
{
	xr_vector<char*>*	i_list	= FS.file_list_open	("$target_folder$",path,FS_ListFiles|FS_RootOnly);
	if (!i_list){
		Msg				("ERROR: Unable to open file list:%s", path);
		return;
	}
	xr_vector<char*>::iterator it	= i_list->begin();
	xr_vector<char*>::iterator itE	= i_list->end();
	for (;it!=itE;++it)
	{
		xr_string		tmp_path	= xr_string(path)+xr_string(*it);
		if (!testSKIP(tmp_path.c_str()))
		{
			files_list->push_back	(xr_strdup(tmp_path.c_str()));
		}else{
			Msg				("-f: %s",tmp_path.c_str());
		}
	}
	FS.file_list_close	(i_list);
}
 
void xrCompressor::ProcessStart()
{
	files_list				= xr_new< xr_vector<char*> >();
	folders_list			= xr_new< xr_vector<char*> >();
 
 	u32 folder_mask		= FS_ListFolders;

	string_path path;
	LPCSTR _path		= "";
	xr_strcpy			(path,_path);
	u32 path_len		= xr_strlen(path);
	if ((0!=path_len)&&(path[path_len-1]!='\\')) xr_strcat(path,"\\");

	Msg					("");
	Msg					("Processing folder: '%s'",path);

	GatherFiles	(path);

	xr_vector<char*>*	i_fl_list	=  FS.file_list_open	("$target_folder$", path, folder_mask);
 	for (auto& file : *i_fl_list)
	{ 
		xr_string tmp_path	= xr_string(path) + xr_string(file);
		folders_list->push_back(xr_strdup(tmp_path.c_str()));
		Msg			("+F: %s",tmp_path.c_str());
		
		// collect files
 		GatherFiles (tmp_path.c_str());
	}
	FS.file_list_close	(i_fl_list);

	PerformWork	();

	// free
	for (auto& list : *files_list)
		xr_free(list);
	xr_delete(files_list);
 
	for (auto& list : *folders_list)
		xr_free(list);
	xr_delete(folders_list);
	 
}
