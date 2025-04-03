#include "stdafx.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "xrserver_objects.h"
#include "xrServer_Objects_Alife_Monsters.h"
#include "Level.h"


void xrServer::Perform_connect_spawn(CSE_Abstract* E, xrClientData* CL, NET_Packet& P)
{
	P.B.count = 0;
	xr_vector<u16>::iterator it = std::find(conn_spawned_ids.begin(), conn_spawned_ids.end(), E->ID);
	if(it != conn_spawned_ids.end())
	{
//.		Msg("Rejecting redundant SPAWN data [%d]", E->ID);
		return;
	}
	
	conn_spawned_ids.push_back(E->ID);
	
	if (E->net_Processed)						return;
	if (E->s_flags.is(M_SPAWN_OBJECT_PHANTOM))	return;


	// Connectivity order
	CSE_Abstract* Parent = ID_to_entity	(E->ID_Parent);
	if (Parent)		Perform_connect_spawn	(Parent,CL,P);

	// Process
	Flags16			save = E->s_flags;
	//-------------------------------------------------
	E->s_flags.set	(M_SPAWN_UPDATE,TRUE);
	if (0==E->owner)	
	{
		// PROCESS NAME; Name this entity
		if (E->s_flags.is(M_SPAWN_OBJECT_ASPLAYER))
		{
			CL->owner			= E;
			VERIFY				(CL->ps);
			E->set_name_replace	(CL->ps->getName());
		}

		// Associate
		E->owner		= CL;
		E->Spawn_Write	(P,TRUE	);
		E->UPDATE_Write	(P);

		CSE_ALifeObject*	object = smart_cast<CSE_ALifeObject*>(E);
		VERIFY				(object);
		if (!object->keep_saved_data_anyway())
			object->client_data.clear	();
	}
	else				
	{
		E->Spawn_Write	(P, FALSE);
		E->UPDATE_Write	(P);
//		CSE_ALifeObject*	object = smart_cast<CSE_ALifeObject*>(E);
//		VERIFY				(object);
//		VERIFY				(object->client_data.empty());
	}
	//-----------------------------------------------------
	E->s_flags			= save;
	SendTo				(CL->ID,P,net_flags(TRUE,TRUE));
	E->net_Processed	= TRUE;
}

void xrServer::SendConfigFinished(ClientID const & clientId)
{
	NET_Packet	P;
	P.w_begin	(M_SV_CONFIG_FINISHED);
	SendTo		(clientId, P, net_flags(TRUE,TRUE));
}
#include "script_ini_file.h"

void xrServer::SendConnectionData(IClient* _CL)
{
	conn_spawned_ids.clear();
	xrClientData*	CL				= (xrClientData*)_CL;
	NET_Packet		P;
	// Replicate current entities on to this client
	xrS_entities::iterator	I=entities.begin(),E=entities.end();
	for (; I!=E; ++I)						
		I->second->net_Processed	= FALSE;
	
	std::string name;
	name.append("spawns_ini.ltx");

	for (I=entities.begin(); I!=E; ++I)		
	{
		Perform_connect_spawn(I->second, CL, P);
	}

	// Start to send server logo and rules
	SendServerInfoToClient			(CL->ID);

/*
	Msg("--- Our sended SPAWN IDs:");
	xr_vector<u16>::iterator it = conn_spawned_ids.begin();
	for (; it != conn_spawned_ids.end(); ++it)
	{
		Msg("%d", *it);
	}
	Msg("---- Our sended SPAWN END");
*/
};

void xrServer::OnCL_Connected		(IClient* _CL)
{
	xrClientData*	CL				= (xrClientData*)_CL;
	CL->net_Accepted = TRUE;
	/*if (Level().IsDemoPlay())
	{
		Level().StartPlayDemo();
		return;
	};*/
///	Server_Client_Check(CL);
	//csPlayers.Enter					();	//sychronized by a parent call
	Export_game_type(CL);
	Perform_game_export();
	SendConnectionData(CL);

	VERIFY2(CL->ps, "Player state not created");
	if (!CL->ps)
	{
		Msg("! ERROR: Player state not created - incorect message sequence!");
		return;
	}

	game->OnPlayerConnect(CL->ID);	
}

void	xrServer::SendConnectResult(IClient* CL, u8 res, u8 res1, char* ResultStr)
{
	NET_Packet	P;
	P.w_begin	(M_CLIENT_CONNECT_RESULT);
	P.w_u8		(res);
	P.w_u8		(res1);
	P.w_stringZ	(ResultStr);
	P.w_clientID(CL->ID);

	if (SV_Client && SV_Client == CL)
		P.w_u8(1);
	else
		P.w_u8(0);
	P.w_stringZ(Level().m_caServerOptions);
	
	SendTo		(CL->ID, P);

	if (!res)			//need disconnect 
	{
#ifdef MP_LOGGING
		Msg("* Server disconnecting client, resaon: %s", ResultStr);
#endif
		Flush_Clients_Buffers	();
		DisconnectClient		(CL, ResultStr);
	}

	if (Level().IsDemoPlay())
	{
		Level().StartPlayDemo();

		return;
	}
	
};

void xrServer::SendProfileCreationError(IClient* CL, char const * reason)
{
	VERIFY					(CL);
	
	NET_Packet	P;
	P.w_begin				(M_CLIENT_CONNECT_RESULT);
	P.w_u8					(0);
	P.w_u8					(ecr_profile_error);
	P.w_stringZ				(reason);
	P.w_clientID			(CL->ID);
	SendTo					(CL->ID, P);
	if (CL != GetServerClient())
	{
		Flush_Clients_Buffers	();
		DisconnectClient		(CL, reason);
	}
}

//this method response for client validation on connect state (CLevel::net_start_client2)
//the first validation is CDKEY, then gamedata checksum (NeedToCheckClient_BuildVersion), then 
//banned or not...
//WARNING ! if you will change this method see M_AUTH_CHALLENGE event handler
void xrServer::Check_GameSpy_CDKey_Success			(IClient* CL)
{
	if (NeedToCheckClient_BuildVersion(CL))
		return;
	//-------------------------------------------------------------
	RequestClientDigest(CL);
};

BOOL	g_SV_Disable_Auth_Check = FALSE;

#include <fstream>;
#include "../jsonxx/jsonxx.h"

#include <fstream>
#include <iostream>

using namespace jsonxx;

bool xrServer::NeedToCheckClient_BuildVersion		(IClient* CL)	
{
/*#ifdef DEBUG

	return false; 

#endif*/
	xrClientData* tmp_client	= smart_cast<xrClientData*>(CL);
	VERIFY						(tmp_client);
	PerformSecretKeysSync		(tmp_client);


	//if (g_SV_Disable_Auth_Check) return false;

	CL->flags.bVerified = FALSE;
	NET_Packet	P;
	P.w_begin	(M_AUTH_CHALLENGE);
	SendTo		(CL->ID, P);
	return true;
};



void xrServer::OnBuildVersionRespond(IClient* CL, NET_Packet& P)
{
	u16 Type;
	P.r_begin(Type);
	u64 _our = FS.auth_get();
	u64 _him = P.r_u64();

	// if (_our != _him)
	// {
	// 	SendConnectResult(CL, 0, ecr_data_verification_failed, "gamedata вмешательство !!!");	
	// 	return;
	// }

	shared_str login ; 
	P.r_stringZ(login);
	shared_str password;
	P.r_stringZ(password);
	string_path path_xray; // logins
	FS.update_path(path_xray, "$mp_saves_logins$", "logins.ltx"); // logins
	CInifile* file = xr_new<CInifile>(path_xray, true); // logins

	if (file->section_exist(CL->name.c_str()))
	{
		const auto& section = file->r_section(CL->name.c_str());

		for (const auto& line : section.Data)
		if (!strcmp(line.first.c_str(), "banned"))
			SendConnectResult(CL, 0, ecr_have_been_banned, "Вас забанили за читы. Пошёл нахуй.");
	}

#ifdef USE_DEBUG_AUTH
	Msg("_our = %d", _our);
	Msg("_him = %d", _him);
	_our = MP_DEBUG_AUTH;
#endif // USE_DEBUG_AUTH

	Object jsonObj;
	bool finded = false;
	std::string nick = { 0 };
	int admin = 0;

	if (FS.path_exist("$mp_saves$"))
	{
		string_path path_xray;

		FS.update_path(path_xray, "$mp_saves$", "players.json");

		std::ifstream input(path_xray);

		if (input.is_open())
		{
			std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

			jsonObj.parse(str);
		}

		input.close();
		
		if (jsonObj.has<Array>("USERS"))
		{
			Array users = jsonObj.get<Array>("USERS");

			for (int i = 0; i < users.size(); i++)
			{
				Object name = users.get<Object>(i);
				std::string name_login = { 0 };
				std::string name_password = { 0 };

				if (name.has<String>("login:"))
					name_login = name.get<String>("login:");

				if (name.has<String>("password:"))
					name_password = name.get<String>("password:");

				if (!std::strcmp(name_login.c_str(), login.c_str()))
				{
					if (!std::strcmp(name_password.c_str(), password.c_str()))
					{
						finded = true;
						
						if (name.has<Number>("admin:"))
							admin = name.get<Number>("admin:");
						if (name.has<String>("nick:"))
							nick = name.get<String>("nick:");
						
						 
					}
				}
			}
		}
	} 
	
	if (CL->flags.bLocal && !g_dedicated_server)
	{
		SendConnectResult(CL, 0, ecr_data_verification_failed, "!!! Нельзя запустить сервер с клиента");
		return;
	}

    if (!finded && !CL->flags.bLocal)
	{
 		SendConnectResult( CL, 0, ecr_data_verification_failed, "НЕВЕРНЫЕ ДАННЫЕ ДЛЯ ВХОДА" );
	}
	else 
	{
		bool bAccessUser = false;
		string512 res_check;
		
		/*
		if ( !CL->flags.bLocal )
		{
			bAccessUser	= Check_ServerAccess( CL, res_check );
		}
				
		if( CL->flags.bLocal || bAccessUser )
		{
			//Check_BuildVersion_Success( CL );
			RequestClientDigest(CL);
		}
		
		else
		{
			Msg("* Client 0x%08x has an incorrect password", CL->ID.value());
			xr_strcat( res_check, "Invalid password.");
			SendConnectResult( CL, 0, ecr_password_verification_failed, res_check );
		}
		*/

		RequestClientDigest(CL);

		xrClientData* data = ID_to_client(CL->ID);
		
		if (data->ps)
		{
			if (game->Type() == eGameIDFreeMp)
				data->ps->m_account.set_player_name(login.c_str());
			else 
				data->ps->m_account.set_player_name(nick.c_str());		

			data->ps->m_account.set_player_login(login.c_str());
			data->ps->m_account.set_player_password(password.c_str());

			data->ps->m_account.set_player_save(login.c_str());
			
			if (admin == 1)
			{
				//admin 
				data->m_admin_rights.m_has_admin_rights = TRUE;
				data->m_admin_rights.m_dwLoginTime = Device.dwTimeGlobal;
				data->ps->setFlag(GAME_PLAYER_HAS_ADMIN_RIGHTS);
				Msg("# User [%s] logged as remote administrator.", login.c_str());

			}

			game->signal_Syncronize();
		}
			
	}
};

void xrServer::Check_BuildVersion_Success			( IClient* CL )
{
	CL->flags.bVerified = TRUE;
	SendConnectResult(CL, 1, 0, "All Ok");
};