
#include "stdafx.h"
#include "NET_Common.h"

//------------------------------------------------------------------------------

#pragma pack( push )
#pragma pack( 1 )
struct MultipacketHeader
{
    u8  tag;
    u16 unpacked_size;
};
#pragma pack( pop )

//------------------------------------------------------------------------------

static NET_Compressor   Compressor;
static const unsigned   MaxMultipacketSize          = NET_PacketSizeLimit + 128;

XRNETSERVER_API int     psNET_GuaranteedPacketMode  = NET_GUARANTEEDPACKET_DEFAULT;

MultipacketSender::MultipacketSender()
{
}

void MultipacketSender::SendPacket( const void* packet_data, u32 packet_sz, u32 flags, u32 timeout )
{
    _buf_cs.Enter();
	
    Buffer* buf = &_buf;

    switch( psNET_GuaranteedPacketMode )
    {
        case NET_GUARANTEEDPACKET_IGNORE :
        {
            flags &= ~DPNSEND_GUARANTEED;
        }   break;

        case NET_GUARANTEEDPACKET_SEPARATE :
        {
            if( flags & DPNSEND_GUARANTEED )
                buf = &_gbuf;
        }   break;
    }
        
    u32 old_flags = (buf->last_flags) & (~DPNSEND_IMMEDIATELLY);
    u32 new_flags = flags & (~DPNSEND_IMMEDIATELLY);

    if(     (buf->buffer.B.count + packet_sz + sizeof(u16) >= NET_PacketSizeLimit)
        ||  (old_flags != new_flags)
        ||  (flags & DPNSEND_IMMEDIATELLY)
      )
    {
        _FlushSendBuffer( timeout, buf );
    }

    buf->buffer.w_u16( (u16)packet_sz );
    buf->buffer.w( packet_data, packet_sz );
    buf->last_flags = flags;

    if( flags & DPNSEND_IMMEDIATELLY )
        _FlushSendBuffer( timeout, buf );
  
    _buf_cs.Leave();
}

//------------------------------------------------------------------------------

void MultipacketSender::FlushSendBuffer( u32 timeout )
{
    _buf_cs.Enter();
    
    _FlushSendBuffer( timeout, &_buf );
    _FlushSendBuffer( timeout, &_gbuf );
    
    _buf_cs.Leave();
}

//------------------------------------------------------------------------------

void MultipacketSender::_FlushSendBuffer( u32 timeout, Buffer* buf )
{
    // expected to be called between '_buf_cs' enter/leave
    if( buf->buffer.B.count )
    {
        // compress data
        unsigned            comp_sz     = Compressor.compressed_size( buf->buffer.B.count );        
        u8                  packet_data [MaxMultipacketSize];
        MultipacketHeader*  header      = (MultipacketHeader*) packet_data;
 
        comp_sz = Compressor.Compress(  
            packet_data+sizeof(MultipacketHeader), 
            sizeof(packet_data)-sizeof(MultipacketHeader), 
            buf->buffer.B.data, buf->buffer.B.count 
        );

        header->tag             = NET_TAG_MERGED;
        header->unpacked_size   = (u16) buf->buffer.B.count;
	        
        _SendTo_LL( packet_data, comp_sz+sizeof(MultipacketHeader), buf->last_flags, timeout );
        buf->buffer.B.count = 0;        
    } 
}

//------------------------------------------------------------------------------

void MultipacketReciever::RecievePacket( const void* packet_data, u32 packet_sz, u32 param )
{
    MultipacketHeader*  header = (MultipacketHeader*) packet_data;
    u8 data[MaxMultipacketSize];

    if ( header->tag != NET_TAG_MERGED  &&  header->tag != NET_TAG_NONMERGED )
       return;

    Compressor.Decompress( data, sizeof(data), (u8*)packet_data+sizeof(MultipacketHeader), packet_sz-sizeof(MultipacketHeader) );
     
    bool    is_multi_packet = header->tag == NET_TAG_MERGED;
    u32     processed_sz    = 0;
    u8*     dat             = data;

    while( processed_sz < header->unpacked_size )
    {
        u32     size = (is_multi_packet) ? u32(*((u16*) dat)) : header->unpacked_size;

        if( is_multi_packet )
            dat += sizeof(u16);
		_Recieve( dat, size, param );

        dat          += size;
        processed_sz += size + ((is_multi_packet) ? sizeof(u16) : 0);
    }
}

void XRNETSERVER_API DumpNetCompressorStats(bool brief)
{
}
