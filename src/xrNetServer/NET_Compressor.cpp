// NET_Compressor.cpp: implementation of the NET_Compressor class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "NET_Common.h"
#include "NET_Compressor.h"
 
#if NET_USE_COMPRESSION

#	ifdef DEBUG
#		pragma warning(push)
#		pragma warning(disable:4995)
#		include <malloc.h>
#		pragma warning(pop)
#	endif // DEBUG

#	include <boost/crc.hpp>

#	if NET_USE_LZO_COMPRESSION
#		define	ENCODE	rtc9_compress
#		define	DECODE	rtc9_decompress
#	else // NET_USE_LZO_COMPRESSION
#		include "../xrCore/ppmd_compressor.h"
#		define	ENCODE	ppmd_compress
#		define	DECODE	ppmd_decompress
#	endif // NET_USE_LZO_COMPRESSION

#endif // NET_USE_COMPRESSION

static FILE*    RawTrafficDump          = NULL;
static FILE*    CompressionDump         = NULL;
 
// size of range encoding code values

#define PPM_CODE_BITS		32
#define PPM_TOP_VALUE       ((NET_Compressor::code_value)1 << (PPM_CODE_BITS-1))

#define SHIFT_BITS		    (PPM_CODE_BITS - 9)
#define EXTRA_BITS		    ((PPM_CODE_BITS-2) % 8 + 1)
#define PPM_BOTTOM_VALUE    (PPM_TOP_VALUE >> 8)
 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NET_Compressor::NET_Compressor()
{
}

NET_Compressor::~NET_Compressor()
{
	if( CompressionDump )
	{
	    fclose( CompressionDump );
	    CompressionDump = NULL;
    }
    if( RawTrafficDump )
    {
        fclose( RawTrafficDump );
        RawTrafficDump = NULL;
    }
}

u16 NET_Compressor::compressed_size	(const u32 &count)
{
	return			((u16)count);
}

XRNETSERVER_API BOOL g_net_compressor_enabled		= FALSE;
XRNETSERVER_API BOOL g_net_compressor_gather_stats	= FALSE;

u16 NET_Compressor::Compress(BYTE* dest, const u32 &dest_size, BYTE* src, const u32 &count)
{
	SCompressorStats::SStatPacket* _p = NULL;
	bool b_compress_packet = (count>36);
	if(g_net_compressor_gather_stats && b_compress_packet)
	{
		_p								= m_stats.get(count);
		_p->hit_count						+= 1;
		m_stats.total_uncompressed_bytes	+= count;
	}

	CopyMemory(dest,src,count);
	return (u16(count));
}



u16 NET_Compressor::Decompress	(BYTE* dest, const u32 &dest_size, BYTE* src, const u32 &count)
{
	CopyMemory(dest,src,count);
	return (u16(count));
}
 