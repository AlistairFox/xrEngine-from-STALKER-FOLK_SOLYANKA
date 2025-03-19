#include "stdafx.h"
#include "../xrEngine/XR_IOConsole.h"
#include "../xrEngine/xr_ioc_cmd.h"

#include "Level.h"
//jsonxx

#include <fstream>;
#include "../jsonxx/jsonxx.h"

#include <fstream>
#include <iostream>

using namespace jsonxx;
//jsonxx


/*
	Register Accounts
*/

class CCC_AdmRegister : public IConsole_Command {
public:
	CCC_AdmRegister(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			string128 login;
			string128 password;
			int admin;

			if (sscanf_s(args, "%s %s %u", &login, sizeof(login), &password, sizeof(password), &admin) != 3)
			{
				Msg("Login and pass not good format");
				return;
			}

			Msg("LoginRegister [%s]", login);
			Msg("PasswordRegister [%s]", password);
			Msg("admin [%d]", admin);


			Object jsonObj;

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

				Array array_table;

				Object tab;

				u32 rand = Random.randI(1, 10000);

				tab << "login:" << Value(login);
				tab << "password:" << Value(password);
				tab << "admin:" << Number(admin);

				if (jsonObj.has<Array>("USERS"))
				{
					for (int i = 0; i != jsonObj.get<Array>("USERS").size(); i++)
					{
						bool find = jsonObj.get<Array>("USERS").get<Object>(i).has<String>("login:");
						if (find)
						{
							std::string login_cmp = jsonObj.get<Array>("USERS").get<Object>(i).get<String>("login:");
							bool compere = xr_strcmp(shared_str(login_cmp.c_str()), shared_str(login));

							if (!compere)
							{
								Msg("--- Логин [%s] Занят укажите другой значение", login);
								return;
							}
						}
					}

					jsonObj.get<Array>("USERS") << tab;
				}
				else
				{
					array_table << tab;

					jsonObj << "USERS" << array_table;
				}

				std::ofstream outfile(path_xray);

				if (outfile.is_open())
				{
					outfile.write(jsonObj.json().c_str(), jsonObj.json().size());
				}

				outfile.close();

			}
		}
		else
		{
			string128 login;
			string128 password;
			int admin = 0;

			if (sscanf_s(args, "%s %s %d", &login, sizeof(login), &password, sizeof(password), &admin) != 3)
			{
				Msg("Login and pass not good format");
				return;
			}

			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "reg %s %s %d", login, password, admin);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
};

class CCC_AdmDelateUser : public IConsole_Command {
public:
	CCC_AdmDelateUser(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			string128 login;

			if (sscanf_s(args, "%s", &login, sizeof(login)) != 1)
			{
				Msg("Login not good format");
				return;
			}

			Msg("LoginRegister [%s]", login);

			Object jsonObj;

			if (FS.path_exist("$mp_saves$"));
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

				Array jsonArray;

				if (jsonObj.has<Array>("USERS"))
					jsonArray = jsonObj.get<Array>("USERS");

				for (int i = 0; i < jsonArray.size(); i++)
				{
					Object tab = jsonArray.get<Object>(i);

					if (tab.has<String>("login:"))
					{
						std::string login_str = tab.get<String>("login:");
						bool check = std::strcmp(login_str.c_str(), login);

						Msg("Finded[%s]", check ? "true" : "false");
						Msg("Finders[%s][%s]", login, login_str.c_str());

						if (!check)
						{
							jsonObj.get<Array>("USERS").get<Object>(i).reset();
							//				jsonObj.get<Array>("USERS").values().at(i);
						}

					}
				}

				std::ofstream outfile(path_xray);

				if (outfile.is_open())
				{
					outfile.write(jsonObj.json().c_str(), jsonObj.json().size());
				}

				outfile.close();

				for (auto pl : Game().players)
				{
					if (xr_strcmp(pl.second->m_account.name_login(), login) == 0)
					{
						xrClientData* tmpxrclient = static_cast<xrClientData*>(Level().Server->GetClientByID(pl.first));
						Level().Server->DisconnectClient(tmpxrclient, "st_kicked_by_server");

						string_path path;
						FS.file_delete("$mp_saves_file$", login);
						break;
					}
				}
			}
		}
		else
		{
			string128 login;

			if (sscanf_s(args, "%s", &login, sizeof(login)) != 1)
			{
				Msg("Login not good format");
				return;
			}

			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "unreg %s", login);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
};

class CCC_GiveSelfAdmin : public IConsole_Command
{
public:
	CCC_GiveSelfAdmin(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; }
	virtual void Execute(LPCSTR args)
	{
		if (!Level().game)
			return;
		NET_Packet packet;
		Game().u_EventGen(packet, GE_GAME_EVENT, -1);
		packet.w_u16(GAME_EVENT_ADMIN_RIGHTS);
		packet.w_u8(1);
		packet.w_clientID(Level().game->local_svdpnid);
		Game().u_EventSend(packet);
	}
};

class CCC_RemoveSelfAdmin : public IConsole_Command
{
public:
	CCC_RemoveSelfAdmin(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; }
	virtual void Execute(LPCSTR args)
	{
		if (!Level().game)
			return;
		NET_Packet packet;
		Game().u_EventGen(packet, GE_GAME_EVENT, -1);
		packet.w_u16(GAME_EVENT_ADMIN_RIGHTS);
		packet.w_u8(0);
		packet.w_clientID(Level().game->local_svdpnid);
		Game().u_EventSend(packet);
	}
};
 
/* END REG ACC*/

void register_console_registers()
{
	//ADMIN		(SE7kILLS)
	CMD1(CCC_AdmRegister,   "register_new_acc");
	CMD1(CCC_AdmDelateUser, "unregister_acc");

	// Для тетсов только дает радминку
	// CMD1(CCC_GiveSelfAdmin,		"ra_give_self");
	// CMD1(CCC_RemoveSelfAdmin,	"ra_remove_self");
}