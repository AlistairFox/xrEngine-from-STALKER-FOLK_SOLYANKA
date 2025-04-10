#include "stdafx.h"
#include "LocatorAPI.h"
#include "FS_internal.h"
#include "stream_reader.h"
#include "data_archive.h"
#pragma warning (disable: 4996)


#include "zlib/zlib-ng.h"
#pragma comment(lib, "zlibstatic-ng.lib")

// Reader
IReader* open_chunk(void* ptr, u32 ID)
{
	BOOL			res;
	u32				dwType, dwSize;
	DWORD			read_byte;
	u32 pt = SetFilePointer(ptr, 0, 0, FILE_BEGIN); VERIFY(pt != INVALID_SET_FILE_POINTER);
	while (true) 
	{
		res = ReadFile(ptr, &dwType, 4, &read_byte, 0);
		if (read_byte == 0)
			return NULL;

		res = ReadFile(ptr, &dwSize, 4, &read_byte, 0);
		if (read_byte == 0)
			return NULL;

		if ((dwType & (~CFS_CompressMark)) == ID)
		{
			u8* src_data	= xr_alloc<u8>(dwSize);
			res				= ReadFile	(ptr,src_data,dwSize,&read_byte,0); VERIFY(res&&(read_byte==dwSize));
 			return			xr_new<CTempReader>(src_data,dwSize,0);
		}
		else
		{
			pt = SetFilePointer(ptr, dwSize, 0, FILE_CURRENT);
			if (pt == INVALID_SET_FILE_POINTER) return 0;
		}
	}
	return 0;
};

void CLocatorAPI::Register(LPCSTR name, u32 vfs, u32 crc, u32 ptr, u32 size_real, u32 size_compressed, u32 modif)
{
	string256			temp_file_name;
	xr_strcpy(temp_file_name, sizeof(temp_file_name), name);
	xr_strlwr(temp_file_name);

	// Register file
	file				desc;
	desc.name = temp_file_name;
	desc.vfs = vfs;
	desc.crc = crc;
	desc.ptr = ptr;
	desc.size_real = size_real;
	desc.size_compressed = size_compressed;
	desc.modif = modif & (~u32(0x3));

	files_it			I = m_files.find(desc);
	if (I != m_files.end())
	{
		desc.name = I->name;
		const_cast<file&>(*I) = desc;
		return;
	}
	else  
		desc.name = xr_strdup(desc.name);
 
	// otherwise insert file
	m_files.insert(desc);

	// Try to register folder(s)
	string_path			temp;
	xr_strcpy(temp, sizeof(temp), desc.name);
	string_path			path;
	string_path			folder;
	while (temp[0])
	{
		_splitpath(temp, path, folder, 0, 0);
		xr_strcat(path, folder);
		if (!exist(path))
		{
			desc.name = xr_strdup(path);
			desc.vfs = 0xffffffff;
			desc.ptr = 0;
			desc.size_real = 0;
			desc.size_compressed = 0;
			desc.modif = u32(-1);
			std::pair<files_it, bool> I = m_files.insert(desc);

			R_ASSERT(I.second);
		}
		xr_strcpy(temp, sizeof(temp), folder);
		if (xr_strlen(temp))		temp[xr_strlen(temp) - 1] = 0;
	}
}

void CLocatorAPI::file_from_archive(IReader*& R, LPCSTR fname, const file& desc)
{
	// Archived one
	archive& A = m_archives[desc.vfs];
	u32 start = (desc.ptr / dwAllocGranularity) * dwAllocGranularity;
	u32 end = (desc.ptr + desc.size_compressed) / dwAllocGranularity;
	if ((desc.ptr + desc.size_compressed) % dwAllocGranularity)	end += 1;
	end *= dwAllocGranularity;
	if (end > A.size)				end = A.size;
	u32 sz = (end - start);
	u8* ptr = (u8*)MapViewOfFile(A.hSrcMap, FILE_MAP_READ, 0, start, sz); VERIFY3(ptr, "cannot create file mapping on file", fname);

	string512					temp;
	xr_sprintf(temp, sizeof(temp), "%s:%s", *A.path, fname);

	u32 ptr_offs = desc.ptr - start;
	if (desc.size_real == desc.size_compressed)
	{
		R = xr_new<CPackReader>(ptr, ptr + ptr_offs, desc.size_real);
		return;
	}

	// Compressed LZO
	// u8* data_real = xr_alloc<u8>(desc.size_real);
	// rtc_decompress(data_real, desc.size_real, ptr + ptr_offs, desc.size_compressed);
	
	// ZLIB
	size_t real = desc.size_real;
	size_t compressed = desc.size_compressed;

	u8* data_real = xr_alloc<u8>(desc.size_real);
	R_ASSERT(Z_OK == zng_uncompress2(data_real, &real, ptr + ptr_offs, &compressed));

	R = xr_new<CTempReader>(data_real, desc.size_real, 0);
	UnmapViewOfFile(ptr);
}

void CLocatorAPI::LoadArchive(archive& A, LPCSTR entrypoint)
{
	// Create base path
	string_path					fs_entry_point;
	fs_entry_point[0] = 0;
	if (A.header)
	{
		shared_str read_path = A.header->r_string("header", "entry_point");
		if (0 == stricmp(read_path.c_str(), "gamedata"))
		{
			read_path = "$fs_root$";
			PathPairIt P = pathes.find(read_path.c_str());
			if (P != pathes.end())
			{
				FS_Path* root = P->second;
				//				R_ASSERT3				(root, "path not found ", read_path.c_str());
				xr_strcpy(fs_entry_point, sizeof(fs_entry_point), root->m_Path);
			}
			xr_strcat(fs_entry_point, "gamedata\\");
		}
		else
		{
			string256			alias_name;
			alias_name[0] = 0;
			R_ASSERT2(*read_path.c_str() == '$', read_path.c_str());

			int count = sscanf(read_path.c_str(), "%[^\\]s", alias_name);
			R_ASSERT2(count == 1, read_path.c_str());

			PathPairIt P = pathes.find(alias_name);

			if (P != pathes.end())
			{
				FS_Path* root = P->second;
				//			R_ASSERT3			(root, "path not found ", alias_name);
				xr_strcpy(fs_entry_point, sizeof(fs_entry_point), root->m_Path);
			}
			xr_strcat(fs_entry_point, sizeof(fs_entry_point), read_path.c_str() + xr_strlen(alias_name) + 1);
		}

	}
	else
	{
		R_ASSERT2(0, "unsupported");
		xr_strcpy(fs_entry_point, sizeof(fs_entry_point), A.path.c_str());
		if (strext(fs_entry_point))
			*strext(fs_entry_point) = 0;
	}

	if (entrypoint)
		xr_strcpy(fs_entry_point, sizeof(fs_entry_point), entrypoint);

	// Read FileSystem
	A.open();

	IReader* hdr = hdr = open_chunk(A.hSrcFile, 1000);
	R_ASSERT(hdr);
	
//	Msg("Header is: %p", hdr);

	while (!hdr->eof())
	{
		DescriptData data;
		hdr->r(&data, sizeof(data));

 		string_path		name, full;
		xr_strcpy(name, data.file_path);
		strconcat(sizeof(full), full, fs_entry_point, name);

//		Msg("Read File: %s", full);
  		Register(full, A.vfs_idx, data.crc, data.ptr, data.size_real, data.size_compressed, 0);
	}
	hdr->close();

}

void CLocatorAPI::ProcessArchive(LPCSTR _path)
{
	// find existing archive
	shared_str path = _path;

	for (archives_it it = m_archives.begin(); it != m_archives.end(); ++it)
		if (it->path == path)
			return;

	m_archives.push_back(archive());
	archive& A = m_archives.back();
	A.vfs_idx = m_archives.size() - 1;
	A.path = path;

	A.open();

	// Read header
	BOOL bProcessArchiveLoading = TRUE;
	IReader* hdr = open_chunk(A.hSrcFile, CFS_HeaderChunkID);

	if (hdr)
	{
		A.header = xr_new<CInifile>(hdr, "archive_header");
		hdr->close();
		bProcessArchiveLoading = A.header->r_bool("header", "auto_load");
	}

	if (bProcessArchiveLoading)
		LoadArchive(A);
	else
		A.close();
}


void CLocatorAPI::file_from_archive(CStreamReader*& R, LPCSTR fname, const file& desc)
{
	archive& A = m_archives[desc.vfs];
	R_ASSERT2(
		desc.size_compressed == desc.size_real,
		make_string(
			"cannot use stream reading for compressed data %s, do not compress data to be streamed",
			fname
		)
	);

	R = xr_new<CStreamReader>();
	R->construct(
		A.hSrcMap,
		desc.ptr,
		desc.size_compressed,
		A.size,
		BIG_FILE_READER_WINDOW_SIZE
	);
}