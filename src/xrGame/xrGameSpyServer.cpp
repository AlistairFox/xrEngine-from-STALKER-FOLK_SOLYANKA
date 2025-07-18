#include "stdafx.h"
#include "xrMessages.h"
#include "xrGameSpyServer.h"
#include "../xrEngine/igame_persistent.h"

#include "GameSpy/GameSpy_Base_Defs.h"
#include "GameSpy/GameSpy_Available.h"

//#define DEMO_BUILD

xrGameSpyServer::xrGameSpyServer()
{
	m_iReportToMasterServer = 0;
	m_bQR2_Initialized = FALSE;
	m_bCDKey_Initialized = FALSE;
	m_bCheckCDKey = false;
	ServerFlags.set( server_flag_all, 0 );
	iGameSpyBasePort = 0;
}

xrGameSpyServer::~xrGameSpyServer()
{
	CDKey_ShutDown();
	QR2_ShutDown();
}

bool	xrGameSpyServer::HasPassword()	{ return !!ServerFlags.test(server_flag_password); }
bool	xrGameSpyServer::HasProtected()	{ return !!ServerFlags.test(server_flag_protected); }

//----------- xrGameSpyClientData -----------------------
IClient*		xrGameSpyServer::client_Create		()
{
	return xr_new<xrGameSpyClientData> ();
}
xrGameSpyClientData::xrGameSpyClientData	():xrClientData()
{
	m_bCDKeyAuth = false;
	m_iCDKeyReauthHint = 0;
}
void	xrGameSpyClientData::Clear()
{
	inherited::Clear();

	m_pChallengeString[0] = 0;
	m_bCDKeyAuth = false;
	m_iCDKeyReauthHint = 0;
};

xrGameSpyClientData::~xrGameSpyClientData()
{
	m_pChallengeString[0] = 0;
	m_bCDKeyAuth = false;
	m_iCDKeyReauthHint = 0;
}
//-------------------------------------------------------
#include "Level.h"

xrGameSpyServer::EConnect xrGameSpyServer::Connect(shared_str &session_name, GameDescriptionData & game_descr)
{
	EConnect res = inherited::Connect(session_name, game_descr);
	if (res!=ErrNoError) return res;

	if ( 0 == *(game->get_option_s		(*session_name,"hname",NULL)))
	{
		string1024	CompName;
		DWORD		CompNameSize = 1024;
		if (GetComputerName(CompName, &CompNameSize)) 
			HostName	= CompName;
	}
	else
		HostName	= game->get_option_s		(*session_name,"hname",NULL);
	
	if (0 != *(game->get_option_s		(*session_name,"psw",NULL)))
		Password._set(game->get_option_s		(*session_name,"psw",NULL));
	MapName	= Level().name();

	m_iReportToMasterServer		= game->get_option_i		(*session_name,"public",0);
	m_iMaxPlayers				= game->get_option_i		(*session_name,"maxplayers",32);
	 
	//--------------------------------------------//
	if (game->Type() != eGameIDSingle) 
	{
		//----- Check for Backend Services ---
		CGameSpy_Available GSA;
		shared_str result_string;
		if (!GSA.CheckAvailableServices(result_string))
		{
			Msg(*result_string);
		};

		//------ Init of QR2 SDK -------------
		iGameSpyBasePort = game->get_option_i(*session_name, "portgs", -1);
		QR2_Init(iGameSpyBasePort);
	};

	return res;
}

void			xrGameSpyServer::Update				()
{
	inherited::Update();

	if (m_bQR2_Initialized)
	{
		m_QR2.Think(NULL);
	};

	if (m_bCDKey_Initialized)
	{
		m_GCDServer.Think();
	};
}

int				xrGameSpyServer::GetPlayersCount()
{
	int NumPlayers = net_players.ClientsCount();
	if (!g_dedicated_server || NumPlayers < 1) return NumPlayers;
	return NumPlayers - 1;
};

bool			xrGameSpyServer::NeedToCheckClient_GameSpy_CDKey	(IClient* CL)
{
	if (!m_bCDKey_Initialized || (CL == GetServerClient() && g_dedicated_server))
	{
		return false;
	};

	SendChallengeString_2_Client(CL);

	return true;
};

void			xrGameSpyServer::OnCL_Disconnected	(IClient* _CL)
{
	inherited::OnCL_Disconnected(_CL);

	if (m_bCDKey_Initialized)
	{
		Msg("Server : Disconnecting Client");
		m_GCDServer.DisconnectUser(int(_CL->ID.value()));
	};
}

u32				xrGameSpyServer::OnMessage(NET_Packet& P, ClientID sender)			// Non-Zero means broadcasting with "flags" as returned
{
	u16			type;
	P.r_begin	(type);

	xrGameSpyClientData* CL		= (xrGameSpyClientData*)ID_to_client(sender);

	switch (type)
	{
	case M_GAMESPY_CDKEY_VALIDATION_CHALLENGE_RESPOND:
		{
			string128 ResponseStr;
			P.r_stringZ(ResponseStr);
			
			if (!CL->m_bCDKeyAuth)
			{
#ifndef MASTER_GOLD
				Msg("Server : Respond accepted, Authenticate client.");
#endif // #ifndef MASTER_GOLD
				m_GCDServer.AuthUser(int(CL->ID.value()), CL->m_cAddress.m_data.data, CL->m_pChallengeString, ResponseStr, this);
				xr_strcpy(CL->m_guid,128,this->GCD_Server()->GetKeyHash(CL->ID.value()));
			}
			else
			{
				Msg("Server : Respond accepted, ReAuthenticate client.");
				m_GCDServer.ReAuthUser(int(CL->ID.value()), CL->m_iCDKeyReauthHint, ResponseStr);
			}

			return (0);
		}break;
	}

	return	inherited::OnMessage(P, sender);
};

bool xrGameSpyServer::Check_ServerAccess( IClient* CL, string512& reason )
{
	if( !HasProtected() )
	{
		xr_strcpy( reason, "Access successful by server. " );
		return true;
	}
	//CL->pass to check the server password
	return true;
}

void xrGameSpyServer::Assign_ServerType( string512& res )
{
	string_path		fn;
	FS.update_path( fn, "$app_data_root$", "server_users.ltx" );
	if( FS.exist(fn) )
	{
		CInifile inif( fn );
		if( inif.section_exist( "users" ) )
		{
			if( inif.line_count( "users" ) != 0 )
			{
				ServerFlags.set( server_flag_protected, 1 );
				xr_strcpy( res, "# Server started as protected, using users list." );
				Msg( res );
				return;
			}else{
				xr_strcpy( res, "Users count in list is null." );
			}
		}else{
			xr_strcpy( res, "Section [users] not found." );
		}
	}else{
		xr_strcpy( res, "File <server_users.ltx> not found in folder <$app_data_root$>." );
	}// if FS.exist(fn)

	Msg( res );
	ServerFlags.set( server_flag_protected, 0 );
	xr_strcpy( res, "# Server started without users list." );
	Msg( res );
}
#include "profiler.h"

void xrGameSpyServer::GetServerInfo( CServerInfo* si )
{
	if (psDeviceFlags.test(rsProfiler))
	{
		profiler().GetServerInfo(si);
		return;
	}

	string32 tmp, tmp2;

	string32 isPublicS;
	sprintf(isPublicS, "Server is Public: %u", m_iReportToMasterServer);
	si->AddItem (isPublicS, RGB(128, 128, 255));

	si->AddItem( "Server name", HostName.c_str(), RGB(128,128,255) );

	si->AddItem( "Map", Level().Server->game->name_map_alife().c_str(), RGB(255, 0, 128));
	
	xr_strcpy( tmp, itoa( GetPlayersCount(), tmp2, 10 ) );
	xr_strcat( tmp, " / ");
	xr_strcat( tmp, itoa( m_iMaxPlayers, tmp2, 10 ) );

	si->AddItem( "Players", tmp, RGB(255,128,255) );
	si->AddItem( "GameSpy port", itoa( iGameSpyBasePort, tmp, 10 ), RGB(200,5,155) );

	inherited::GetServerInfo( si );
}
