#pragma once

struct DescriptData
{
	char file_path[512];
	u32 crc;
	u32 ptr;
	u32 size_real;
	u32 size_compressed;
};