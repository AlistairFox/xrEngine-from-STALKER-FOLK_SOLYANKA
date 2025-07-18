#if !defined _INCDEF_NETCOMMON_H_
#define _INCDEF_NETCOMMON_H_
#pragma once
//==============================================================================

struct GameDescriptionData
{
	string128	map_name = "";
	string128	map_version = "";
	string512	download_url = "";
    string256   spawn_name = "";
};
    
#define NET_MERGE_PACKETS               1
#define NET_TAG_MERGED                  0xE1
#define NET_TAG_NONMERGED               0xE0

#define NET_USE_COMPRESSION             0
#define NET_TAG_COMPRESSED              0xC1
#define NET_TAG_NONCOMPRESSED           0xC0

#define NET_USE_LZO_COMPRESSION         1
#define NET_USE_COMPRESSION_CRC         1

#define NET_LOG_PACKETS                 0
#define NET_LOG_COMPRESSION             0
#define NET_DUMP_COMPRESSION            0


#define NET_GUARANTEEDPACKET_DEFAULT    0
#define NET_GUARANTEEDPACKET_IGNORE     1
#define NET_GUARANTEEDPACKET_SEPARATE   2

#define	GAMESPY_QR2_BASEPORT				5445
#define GAMESPY_BROWSER_MAX_UPDATES			20

#define START_PORT							0
#define END_PORT							65535
#define START_PORT_LAN						GAMESPY_QR2_BASEPORT
#define START_PORT_LAN_SV					START_PORT_LAN + 1
#define START_PORT_LAN_CL					START_PORT_LAN + 2
#define END_PORT_LAN						START_PORT_LAN + 250//GameSpy only process 500 ports

extern XRNETSERVER_API int psNET_GuaranteedPacketMode;

/*#ifdef DEBUG
void PrintParsedPacket(const char* message, u16 message_type, const void* packet_data, u32 packet_size);
#endif*/

//==============================================================================

class XRNETSERVER_API
MultipacketSender
{
public:
                    MultipacketSender();
    virtual         ~MultipacketSender() {}

    void            SendPacket( const void* packet_data, u32 packet_sz, u32 flags, u32 timeout );
    void            FlushSendBuffer( u32 timeout );


protected:

    virtual void    _SendTo_LL( const void* data, u32 size, u32 flags, u32 timeout ) =0;


private:

    struct Buffer;


    void            _FlushSendBuffer( u32 timeout, Buffer* buf );

    struct
    Buffer
    {
                    Buffer() : last_flags(0) { buffer.B.count = 0; }
                    
        NET_Packet  buffer;
        u32         last_flags;
    };

    Buffer              _buf;
    Buffer              _gbuf;
    xrCriticalSection   _buf_cs;
};


//==============================================================================

class XRNETSERVER_API
MultipacketReciever
{
public:

    virtual         ~MultipacketReciever() {}

    void            RecievePacket( const void* packet_data, u32 packet_sz, u32 param=0 );


protected:

    virtual void    _Recieve( const void* data, u32 data_size, u32 param ) =0;
};


//==============================================================================

struct SClientConnectData
{
	ClientID		clientID;
	string64		name;
	string64		pass;
	u32				process_id;

	SClientConnectData()
	{
		name[0] = pass[0] = 0;
		process_id = 0;
	}
};

//==============================================================================

#endif // _INCDEF_NETCOMMON_H_

