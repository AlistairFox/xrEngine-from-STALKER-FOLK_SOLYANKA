#include "stdafx.h"

#pragma warning(disable: 4995)
#pragma warning(disable: 4996)
#include "xrCompress.h"
#include "mutex"
#include "ppl.h"

#define CFS_ARCHIVE_SE7	2610
 
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

	for (xr_vector<shared_str>::iterator it=exclude_exts.begin(); it!=exclude_exts.end(); ++it)
		if (PatternMatch(p_ext,it->c_str()))
			return true;

	return false;
}

bool xrCompressor::testVFS(LPCSTR path)
{
	if (bStoreFiles)
		return			(true);


	string256			p_ext;
	_splitpath(path, 0, 0, 0, p_ext);
 
	if (!stricmp(p_ext,".xml"))
		return			(false);

	if (!stricmp(p_ext,".ltx"))
		return			(FALSE);

	if (!stricmp(p_ext,".script"))
		return			(FALSE);
	//new SE7  TEXTURES
	// if (!bStoreDDS)
	// if (!stricmp(p_ext, ".dds"))
	// 	return			(FALSE);

	//LEVELS
	//if (!stricmp(p_ext, ".geom"))
	//	return (FALSE);

	//if (!stricmp(p_ext, ".geomx"))
	//	return (FALSE);

	//if (!stricmp(p_ext, ".cform"))
	//	return (FALSE);

	//OGF
	// if (!stricmp(p_ext, ".ogf"))
	// 	return (FALSE);
	// 
	// if (!stricmp(p_ext, ".omf"))
	// 	return (FALSE);
	// 
	// //SOUND
	// if (!stricmp(p_ext, ".ogg"))
	// 	return (FALSE);
 
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

std::mutex lock_processfile;


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
		// fs_pack_writer->w(data.data(), c_size_real);

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
				//fs_pack_writer->w(CompressedData, size_compressed);
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
			//fs_pack_writer->w(data, size_real);
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
	// Write description
	// write_file_header		(path, c_crc32, c_ptr, c_size_real, c_size_compressed);
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

	//write pack header without compression
 

	if(config_ltx && config_ltx->section_exist("header"))
	{
		CMemoryWriter			W;
		CInifile::Sect&	S		= config_ltx->r_section("header");
		CInifile::SectCIt it	= S.Data.begin();
		CInifile::SectCIt it_e	= S.Data.end();
		string4096				buff;
		xr_sprintf					(buff,"[%s]",S.Name.c_str());
		W.w_string				(buff);
		for(;it!=it_e;++it)
		{
			const CInifile::Item& I	= *it;
			xr_sprintf					(buff, "%s = %s", I.first.c_str(), I.second.c_str());
			W.w_string				(buff);
		}
		W.seek						(0);
		IReader	R(W.pointer(), W.size());

		printf						("...Writing pack header\n");
		fs_pack_writer->open_chunk	(CFS_HeaderChunkID);					//CFS_HeaderChunkID
		fs_pack_writer->w			(R.pointer(), R.length());
		fs_pack_writer->close_chunk	();
	}
	else
	if(pPackHeader)
	{
		printf						("...Writing pack header\n");
		fs_pack_writer->open_chunk	(CFS_HeaderChunkID);
		fs_pack_writer->w			(pPackHeader->pointer(), pPackHeader->length());
		fs_pack_writer->close_chunk	();
	}else
		printf			("...Pack header not found\n");

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

	fs_pack_writer->w_chunk		(CFS_ARCHIVE_SE7 /*1*/ | CFS_CompressMark, fs_desc.pointer(), fs_desc.size());


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

		xr_vector<PackedData> toPack[16];
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

		concurrency::parallel_for(size_t(0), size_t(16), [&](size_t ID)
			{
				FunctionMT(ID);
			});

		u32 TotalFiles = 0;
		for (auto i = 0; i < 16; i++)
		{
			TotalFiles += toPack[i].size();
		}

		int ID = 0;
		for (auto i = 0; i < 16; i++)
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

bool xrCompressor::IsFolderAccepted(CInifile& ltx, LPCSTR path, BOOL& recurse)
{
	// exclude folders
	if( ltx.section_exist("exclude_folders") )
	{
		CInifile::Sect& ef_sect	= ltx.r_section("exclude_folders");
		for (CInifile::SectCIt ef_it=ef_sect.Data.begin(); ef_it!=ef_sect.Data.end(); ef_it++)
		{
			recurse	= CInifile::IsBOOL(ef_it->second.c_str());
			if (recurse)	
			{
				if (path==strstr(path,ef_it->first.c_str()))	
					return false;
			}else
			{
				if (0==xr_strcmp(path,ef_it->first.c_str()))	
					return false;
			}
		}
	}
	return true;
}

void xrCompressor::ProcessLTX(CInifile& ltx)
{
	config_ltx	=&ltx;

	if (ltx.line_exist("options","exclude_exts"))
		_SequenceToList(exclude_exts, ltx.r_string("options","exclude_exts"));

	files_list				= xr_new< xr_vector<char*> >();
	folders_list			= xr_new< xr_vector<char*> >();

	if(ltx.section_exist("include_folders"))
	{
		CInifile::Sect& if_sect	= ltx.r_section("include_folders");

		for (CInifile::SectCIt if_it=if_sect.Data.begin(); if_it!=if_sect.Data.end(); ++if_it)
		{
			BOOL ifRecurse		= CInifile::IsBOOL(if_it->second.c_str());
			u32 folder_mask		= FS_ListFolders | (ifRecurse?0:FS_RootOnly);

			string_path path;
			LPCSTR _path		= 0==xr_strcmp(if_it->first.c_str(),".\\")?"":if_it->first.c_str();
			xr_strcpy			(path,_path);
			u32 path_len		= xr_strlen(path);
			if ((0!=path_len)&&(path[path_len-1]!='\\')) xr_strcat(path,"\\");

			Msg					("");
			Msg					("Processing folder: '%s'",path);

			BOOL efRecurse;
			BOOL val			= IsFolderAccepted(ltx,path,efRecurse);
			if (val || (!val&&!efRecurse))
			{ 
				if (val)		
					GatherFiles	(path);

				xr_vector<char*>*	i_fl_list	= FS.file_list_open	("$target_folder$",path,folder_mask);
				if (!i_fl_list)
				{
					Msg			("ERROR: Unable to open folder list:", path);
					continue;
				}

				xr_vector<char*>::iterator it	= i_fl_list->begin();
				xr_vector<char*>::iterator itE	= i_fl_list->end();
				for (;it!=itE;++it)
				{ 
					xr_string tmp_path	= xr_string(path)+xr_string(*it);
					bool val		= IsFolderAccepted(ltx,tmp_path.c_str(),efRecurse);
					if (val)
					{
						folders_list->push_back(xr_strdup(tmp_path.c_str()));
						Msg			("+F: %s",tmp_path.c_str());
						// collect files
						if (ifRecurse) 
							GatherFiles (tmp_path.c_str());
					}else
					{
						Msg			("-F: %s",tmp_path.c_str());
					}
				}
				FS.file_list_close	(i_fl_list);
			}else
			{
				Msg					("-F: %s",path);
			}
		}
	}

	
	if(ltx.section_exist("include_files"))
	{
		CInifile::Sect& if_sect	= ltx.r_section("include_files");
		for (CInifile::SectCIt if_it=if_sect.Data.begin(); if_it!=if_sect.Data.end(); ++if_it)
		{
		  files_list->push_back	(xr_strdup(if_it->first.c_str()));
		}	
	}

	PerformWork	();

	// free
	xr_vector<char*>::iterator it	= files_list->begin();
	xr_vector<char*>::iterator itE	= files_list->end();
	for (;it!=itE;++it) 
		xr_free(*it);
	xr_delete(files_list);

	it				= folders_list->begin();
	itE				= folders_list->end();
	for (;it!=itE;++it) 
		xr_free(*it);
	xr_delete(folders_list);

	exclude_exts.clear_and_free();
}
