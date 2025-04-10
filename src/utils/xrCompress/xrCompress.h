#pragma once
#ifndef XR_COMPRESS_H_INCLUDED
#define XR_COMPRESS_H_INCLUDED
#include "..\..\xrCore\data_archive.h"

class xrCompressor
{
	struct PackedData
	{
		char		FileName[512];
		u32			c_crc32 = 0;
		u32			c_size_real = 0;
		u32			c_size_compressed = 0;

		xr_vector<u8> packed_data;
	};
public:
	u8							level_compression;

private:
	bool						bFast;
	bool						bStoreFiles;
	bool						bStoreDDS;
	IWriter*					fs_pack_writer;
	CMemoryWriter				fs_desc;
	shared_str					target_name;
	IReader*					pPackHeader;
	CInifile*					config_ltx;
	xr_vector<char*>*			files_list;
	xr_vector<char*>*			folders_list;
 
	xr_vector<shared_str>		exclude_exts;
	bool	testSKIP			(LPCSTR path);
	bool	testEqual			(LPCSTR path, IReader* base);
	bool	testVFS				(LPCSTR path);

	bool	IsFolderAccepted	(CInifile& ltx, LPCSTR path, BOOL& recurse);
	
	void	GatherFiles			(LPCSTR folder);

	void	write_file_header	(LPCSTR file_name, const u32 &crc, const u32 &ptr, const u32 &size_real, const u32 &size_compressed);
	void	ClosePack			();
	void	OpenPack			(LPCSTR tgt_folder, int num);
	
	void	PerformWork			();
 	void	ProcessFile			(LPCSTR path, xr_vector<PackedData>& toPack, size_t& Accum);


	u32						bytesSRC;
	u32						bytesDST;
	u32						filesTOTAL;
	u32						filesSKIP;
	u32						filesVFS;

  	u32						dwTimeStart;

	u32						XRP_MAX_SIZE;

	shared_str				ArchiveName, OutFolder;

public:


	void	SetArchiveName(LPCSTR n) { ArchiveName._set(n); }
	void	SetOutFolder(LPCSTR v) { OutFolder._set(v); }

			xrCompressor		();
			~xrCompressor		();
	void	SetFastMode			(bool b)					{bFast=b;}
	void	SetStoreFiles		(bool b)					{bStoreFiles=b;}
	void	SetMaxVolumeSize	(u32 sz)					{XRP_MAX_SIZE=sz;}
	void	SetTargetName		(LPCSTR n)					{target_name=n;}
	void	SetPackHeaderName	(LPCSTR n);
	void	SetStoreDDS(bool b) { bStoreDDS = b; };

	void	ProcessLTX			(CInifile& ini);
	void	ProcessTargetFolder	();
};

#endif