#include "stdafx.h"
#include "../xrEngine/XR_IOConsole.h"
#include "../xrEngine/xr_ioc_cmd.h"
#include "game_sv_mp.h"
#include "game_sv_freemp.h"

#include "Level.h"
#include "Actor.h"
extern char const* exclude_raid_from_args(LPCSTR args, LPSTR dest, size_t dest_size);


// MOENY TRANSFERING
class CCC_GiveMoneyToPlayer : public IConsole_Command {
public:
	CCC_GiveMoneyToPlayer(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void	Execute(LPCSTR args)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_mp* srv = smart_cast<game_sv_mp*>(Level().Server->game);
		if (!srv) return;

		string1024 buff;
		exclude_raid_from_args(args, buff, sizeof(buff));

		ClientID client_id(0);
		u32 tmp_client_id;
		s32 money;

		if (sscanf_s(buff, "%u %d", &tmp_client_id, &money) != 2)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Give money to player. Format: \"sv_give_money <player session id> <money>\"");
			return;
		}
		client_id.set(tmp_client_id);

		xrClientData* CL = static_cast<xrClientData*>(Level().Server->GetClientByID(client_id));

		if (CL && CL->ps && (CL != Level().Server->GetServerClient()))
		{
			s64 total_money = CL->ps->money_for_round;
			total_money += money;

			if (total_money < 0)
				total_money = 0;

			if (total_money > std::numeric_limits<s32>().max())
			{
				Msg("! The limit of the maximum amount of money has been exceeded");
				return;
			}

			CL->ps->money_for_round = s32(total_money);
			srv->signal_Syncronize();
		}
		else
		{
			Msg("! Can't give money to client %u", client_id.value());
		}
	}
};

class CCC_TransferMoney : public IConsole_Command {
public:
	CCC_TransferMoney(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void	Execute(LPCSTR args)
	{
		string4096 buff;
		xr_strcpy(buff, args);

		static _locale_t current_locale = _create_locale(LC_ALL, "");

		u32 len = xr_strlen(buff);
		if (0 == len)
			return;

		if (IsGameTypeSingle()) return;
		if (g_dedicated_server) return;
		if (!(&Level())) return;
		if (!(&Game())) return;

		if (!Game().local_player)
			return;

		if (Game().local_player && Game().local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
			return;

		CActor* pActor = smart_cast<CActor*>(Level().CurrentControlEntity());
		if (!pActor || !pActor->is_alive())
			return;

		string128 name;
		s32 money;

		if (sscanf_s(args, "%d %s", &money, &name, sizeof(name)) != 2)
		{
			Msg("! Transfer money to player. Format: \"g_transfer_money <money> <player name>\"");
			return;
		}
		if (Game().local_player->money_for_round < money)
		{
			Msg("! You don't have enough money!");
			return;
		}

		_strlwr_l(_Trim(name), current_locale);

		bool wasSent = false;

		for (auto& player : Game().players)
		{
			game_PlayerState* ps = player.second;
			if (ps->GameID == Game().local_player->GameID)
			{
				continue;
			}

			string128 player_name;
			xr_strcpy(player_name, ps->getName());
			_strlwr_l(player_name, current_locale);

			if (xr_strcmp(player_name, name) == 0)
			{
				NET_Packet					P;
				Game().u_EventGen(P, GE_GAME_EVENT, pActor->ID());
				P.w_u16(GAME_EVENT_TRANSFER_MONEY);
				P.w_clientID(player.first); // to
				P.w_s32(money); // money
				Game().u_EventSend(P);
				wasSent = true;
				break;
			}
		}

		if (wasSent)
		{
			Msg("- The money was transferred successfully!");
		}
		else
		{
			Msg("! Failed to send money to player with name: \"%s\".", name);
		}
	}
};

class CCC_Set_Money_to_client : public IConsole_Command {
public:
	CCC_Set_Money_to_client(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };
	virtual void		Execute(LPCSTR args)
	{
		string256 args_new;
		exclude_raid_from_args(args, args_new, sizeof(args_new));
		u32 read_id = 0;
		u32 money = 0;
		u32 count = sscanf(args_new, "%u, %u", &read_id, &money);

		if (count == 1 && OnClient())
		{
			sscanf(args_new, "%u", &money);
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);

			string128 str;
			xr_sprintf(str, "sv_set_money %u %u", Level().GetClientID(), money);

			P.w_stringZ(str);

			Level().Send(P, net_flags(TRUE, TRUE));
			return;
		}

		if (OnServer())
		{
			game_sv_freemp* freemp = smart_cast<game_sv_freemp*>(Level().Server->game);
			if (freemp)
			{
				ClientID id;
				id.set(read_id);

				game_PlayerState* ps = freemp->get_id(id);
				if (ps)
					freemp->SetMoneyToPlayer(ps, money);
			}
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);

			string128 str;
			xr_sprintf(str, "sv_set_money %u %u", read_id, money);

			P.w_stringZ(str);

			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
	virtual void Save(IWriter* F) {};
};

class CCC_ADD_Money_to_client_self : public IConsole_Command {
public:
	CCC_ADD_Money_to_client_self(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };
	virtual void		Execute(LPCSTR args)
	{
		NET_Packet		P;
		P.w_begin(M_REMOTE_CONTROL_CMD);

		string128 str;
		xr_sprintf(str, "sv_give_money %u %s", Level().GetClientID(), args);

		P.w_stringZ(str);

		Level().Send(P, net_flags(TRUE, TRUE));
	}
	virtual void Save(IWriter* F) {};
};



/**

	ADM_FETURES

**/

class CCC_SetNoClipForPlayer : public IConsole_Command {
public:
	CCC_SetNoClipForPlayer(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void	Execute(LPCSTR args)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_mp* srv = smart_cast<game_sv_mp*>(Level().Server->game);
		if (!srv) return;

		string1024 buff;
		exclude_raid_from_args(args, buff, sizeof(buff));

		ClientID client_id(0);
		u32 tmp_client_id;
		u32 value;
		if (sscanf_s(buff, "%u %u", &tmp_client_id, &value) != 2)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Set no clip for player. Format: \"sv_set_no_clip <player session id> <value>\"");
			return;
		}
		client_id.set(tmp_client_id);

		xrClientData* CL = static_cast<xrClientData*>(Level().Server->GetClientByID(client_id));
		if (CL && CL->ps && (CL != Level().Server->GetServerClient()) && (value == 0 || value == 1))
		{
			if (value) CL->ps->setFlag(GAME_PLAYER_MP_NO_CLIP);
			else CL->ps->resetFlag(GAME_PLAYER_MP_NO_CLIP);
			srv->signal_Syncronize();
		}
		else
		{
			Msg("! Can't set no clip for player with client id %u", client_id.value());
		}
	}
};

class CCC_SetInvisForPlayer : public IConsole_Command {
public:
	CCC_SetInvisForPlayer(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void	Execute(LPCSTR args)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_mp* srv = smart_cast<game_sv_mp*>(Level().Server->game);
		if (!srv) return;

		string1024 buff;
		exclude_raid_from_args(args, buff, sizeof(buff));

		ClientID client_id(0);
		u32 tmp_client_id;
		u32 value;
		if (sscanf_s(buff, "%u %u", &tmp_client_id, &value) != 2)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Set no clip for player. Format: \"sv_set_invis <player session id> <value>\"");
			return;
		}
		client_id.set(tmp_client_id);

		xrClientData* CL = static_cast<xrClientData*>(Level().Server->GetClientByID(client_id));
		if (CL && CL->ps && (CL != Level().Server->GetServerClient()) && (value == 0 || value == 1))
		{
			if (value) CL->ps->setFlag(GAME_PLAYER_MP_INVIS);
			else CL->ps->resetFlag(GAME_PLAYER_MP_INVIS);
			srv->signal_Syncronize();
		}
		else
		{
			Msg("! Can't set invis for player with client id %u", client_id.value());
		}
	}
};

class CCC_MovePlayerToRPoint : public IConsole_Command {
public:
	CCC_MovePlayerToRPoint(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void	Execute(LPCSTR args)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_mp* sv_game = smart_cast<game_sv_mp*>(Level().Server->game);
		if (!sv_game) return;

		string1024 buff;
		exclude_raid_from_args(args, buff, sizeof(buff));

		ClientID client_id(0);
		u32 tmp_client_id;
		if (sscanf_s(buff, "%u", &tmp_client_id) != 1)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Move player to rpoint. Format: \"sv_move_player_to_rpoint <player session id>\"");
			return;
		}
		client_id.set(tmp_client_id);

		xrClientData* CL = static_cast<xrClientData*>(Level().Server->GetClientByID(client_id));
		if (!CL || !CL->net_Ready || !CL->owner || !CL->ps || CL->ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
		{
			Msg("! Can't move player to rpoint %u", client_id.value());
			return;
		}

		xr_vector<RPoint>& rpoints = sv_game->rpoints[CL->ps->team];
		RPoint& rp = rpoints[::Random.randI(rpoints.size())];

		sv_game->TeleportPlayerTo(client_id, rp.P, rp.A);
	}
};

class CCC_SetGodModForPlayer : public IConsole_Command {
public:
	CCC_SetGodModForPlayer(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void	Execute(LPCSTR args)
	{
		if (!g_pGameLevel || !Level().Server) return;

		game_sv_mp* srv = smart_cast<game_sv_mp*>(Level().Server->game);
		if (!srv) return;

		string1024 buff;
		exclude_raid_from_args(args, buff, sizeof(buff));

		ClientID client_id(0);
		u32 tmp_client_id;

		if (sscanf_s(buff, "%u", &tmp_client_id) != 1)
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Set god mode for player. Format: \"sv_set_god_mode <player session id>\" ");
			return;
		}

		client_id.set(tmp_client_id);

		xrClientData* CL = static_cast<xrClientData*>(Level().Server->GetClientByID(client_id));

		if (CL && CL->ps && (CL != Level().Server->GetServerClient()))
		{
			if (CL->ps->testFlag(GAME_PLAYER_MP_GOD_MODE))
				CL->ps->resetFlag(GAME_PLAYER_MP_GOD_MODE);
			else
				CL->ps->setFlag(GAME_PLAYER_MP_GOD_MODE);

			srv->signal_Syncronize();
		}
		else
		{
			Msg("! Can't set god mode for player with client id %u", client_id.value());
		}
	}
};

class CCC_AdmNoClip : public IConsole_Command {
public:
	CCC_AdmNoClip(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		u32 value = u32(-1);

		if (sscanf(args, "%u", &value) == 1 && (value == 0 || value == 1))
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "sv_set_no_clip %u %u", Game().local_svdpnid.value(), value);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
		else
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Set noclip for self. Format: \"adm_no_clip [0,1]\"");
			return;
		}
	}
};

class CCC_AdmInvis : public IConsole_Command {
public:
	CCC_AdmInvis(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		u32 value = u32(-1);

		if (sscanf(args, "%u", &value) == 1 && (value == 0 || value == 1))
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "sv_set_invis %u %u", Game().local_svdpnid.value(), value);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
		else
		{
			Msg("! ERROR: bad command parameters.");
			Msg("Set noclip for self. Format: \"adm_invis [0,1]\"");
			return;
		}
	}
};

class CCC_AdmGodMode : public IConsole_Command {
public:
	CCC_AdmGodMode(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		NET_Packet		P;
		P.w_begin(M_REMOTE_CONTROL_CMD);
		string128 str;
		xr_sprintf(str, "sv_set_god_mode %u", Game().local_svdpnid.value());
		P.w_stringZ(str);
		Level().Send(P, net_flags(TRUE, TRUE));
	}
};

class CCC_UnlimatedAmmo : public IConsole_Command
{
public:
	CCC_UnlimatedAmmo(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			if (!g_pGameLevel || !Level().Server) return;

			game_sv_freemp* srv = smart_cast<game_sv_freemp*>(Level().Server->game);
			if (!srv) return;

			u32 tmp_cl;
			ClientID clientID;

			sscanf(args, "%u", &tmp_cl);
			clientID.set(tmp_cl);

			xrClientData* CL = (xrClientData*)Level().Server->GetClientByID(clientID);

			if (!CL)
				return;

			if (CL && CL->ps && (CL != Level().Server->GetServerClient()))
			{
				if (CL->ps->testFlag(GAME_PLAYER_MP_UNLIMATED_AMMO))
					CL->ps->resetFlag(GAME_PLAYER_MP_UNLIMATED_AMMO);
				else
					CL->ps->setFlag(GAME_PLAYER_MP_UNLIMATED_AMMO);

				srv->signal_Syncronize();
			}
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_unlimited_ammo %u", Game().local_svdpnid);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}

	}

};

enum surges {
	Surge = 1,
	Fallout = 2,
	PsiStorm = 3
};

class CCC_AdmSurgeStart : public IConsole_Command {
public:
	CCC_AdmSurgeStart(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			Msg("Start Surge Storm");
			luabind::functor<void> funct;
			if (ai().script_engine().functor("mp_events.surge_run", funct))
				funct();

			// NET_Packet		P;
			// P.w_begin(M_SCRIPT_EVENT);
			// P.w_u8(8);
			// P.w_u8(Surge);
			// Level().Send(P, net_flags(TRUE, TRUE));
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_surge");
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
};

class CCC_AdmFalloutStart : public IConsole_Command {
public:
	CCC_AdmFalloutStart(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			Msg("Start Fallout Storm");
			luabind::functor<void> funct;
			if (ai().script_engine().functor("mp_events.fallout_run", funct))
				funct();

			// NET_Packet		P;
			// P.w_begin(M_SCRIPT_EVENT);
			// P.w_u8(8);
			// P.w_u8(Fallout);
			// Level().Send(P, net_flags(TRUE, TRUE));
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_fallout");
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
};

class CCC_AdmPsiStormStart : public IConsole_Command {
public:
	CCC_AdmPsiStormStart(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			Msg("Start Psi Storm");
			// NET_Packet		P;
			// P.w_begin(M_SCRIPT_EVENT);
			// P.w_u8(8);
			// P.w_u8(PsiStorm);
			// Level().Send(P, net_flags(TRUE, TRUE));

			luabind::functor<void> funct;
			if (ai().script_engine().functor("mp_events.psi_storm_run", funct))
				funct();
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_psi_storm");
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
};

// TELEPORTS

// TELEPORTS

class CCC_TeleportToPosition : public IConsole_Command
{
public:
	CCC_TeleportToPosition(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };


	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			u32 Client_ID;
			Fvector pos;
			ClientID id;
			float x = 0, y = 0, z = 0;
			int ret = sscanf(args, "%u, %f, %f, %f", &Client_ID, &x, &y, &z);

			if (ret != 4)
			{
				Msg("ERROR EXECUTE TELEPORT TO POS ClientID, x,y,z");
				return;
			}

			pos.set(x, y, z);
			id.set(Client_ID);

			if (!g_pGameLevel || !Level().Server) return;

			game_sv_mp* sv_game = smart_cast<game_sv_mp*>(Level().Server->game);
			if (!sv_game) return;

			sv_game->TeleportPlayerTo(id, pos);

		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_teleport %u %s", Game().local_svdpnid, args);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}

	}
};

class CCC_TeleportPlayer : public IConsole_Command
{
public:
	CCC_TeleportPlayer(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };


	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{

			if (!g_pGameLevel || !Level().Server) return;

			u32 Client_ID, Second_ID;
			Fvector pos;
			ClientID id, sec_id;

			int ret = sscanf(args, "%u, %u", &Client_ID, &Second_ID);

			if (ret != 2)
			{
				Msg("sv Error run cmd (ClientID, SecondID)");
				return;
			}

			id.set(Client_ID);
			sec_id.set(Second_ID);

			game_sv_mp* sv_game = smart_cast<game_sv_mp*>(Level().Server->game);

			if (!sv_game) return;

			if (sv_game)
			{
				for (auto pl : Game().players)
				{
					if (pl.first == sec_id)
					{
						CObject* o = Level().Objects.net_Find(pl.second->GameID);
						if (o)
						{
							pos.set(o->Position());
							sv_game->TeleportPlayerTo(id, pos);
						}
						break;
					}
				}
			}
		}
		else
		{
			u32 client_ID;
			int ret = sscanf(args, "%u", &client_ID);

			if (ret != 1)
			{
				Msg("Error cmd(ClientID) ");
				return;
			}

			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_teleport_player %u, %u", Game().local_svdpnid, client_ID);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}

	}
};

class CCC_TeleportToPlayer : public IConsole_Command
{
public:
	CCC_TeleportToPlayer(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };
	virtual void Execute(LPCSTR args)
	{
		NET_Packet		P;
		P.w_begin(M_REMOTE_CONTROL_CMD);
		string128 str;
		xr_sprintf(str, "adm_teleport_player %u %u", args, Game().local_svdpnid);
		P.w_stringZ(str);
		Level().Send(P, net_flags(TRUE, TRUE));
	}

};

class CCC_ChangeTeamPlayer : public IConsole_Command
{
public:
	CCC_ChangeTeamPlayer(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = false; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			string128 tmp;
			exclude_raid_from_args(args, tmp, sizeof(tmp));

			int team;
			u32 clid;

			sscanf(tmp, "%u %u", &clid, &team);
			ClientID id; id.set(clid);

			xrClientData* data = (xrClientData*)Level().Server->GetClientByID(id);
			if (data)
			{
				data->ps->team = team;
				Level().Server->game->signal_Syncronize();
			}
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_set_team %s", args);
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}
};

class CCC_AdmSurgeStop : public IConsole_Command
{
public:
	CCC_AdmSurgeStop(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };

	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			NET_Packet		P;
			P.w_begin(M_SCRIPT_EVENT);
			P.w_u8(9);
			P.w_u8(1);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_surge_stop");
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}

};

class CCC_AdmFalloutStop : public IConsole_Command
{
public:
	CCC_AdmFalloutStop(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			NET_Packet		P;
			P.w_begin(M_SCRIPT_EVENT);
			P.w_u8(9);
			P.w_u8(2);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_fallout_stop");
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}

};

class CCC_AdmPsiStormStop : public IConsole_Command
{
public:
	CCC_AdmPsiStormStop(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = true; };
	virtual void Execute(LPCSTR args)
	{
		if (OnServer())
		{
			NET_Packet		P;
			P.w_begin(M_SCRIPT_EVENT);
			P.w_u8(9);
			P.w_u8(3);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
		else
		{
			NET_Packet		P;
			P.w_begin(M_REMOTE_CONTROL_CMD);
			string128 str;
			xr_sprintf(str, "adm_psi_storm_stop");
			P.w_stringZ(str);
			Level().Send(P, net_flags(TRUE, TRUE));
		}
	}

};



extern BOOL		g_cl_draw_mp_statistic;
extern int		MAX_DISTANCE_FIND_GRAPH = 350;

extern float	Shedule_Scale_Objects = 4;
extern float	Shedule_Scale_AI_Stalker = 1;
extern float	Shedule_Zone = 1;

extern float	Shedule_Events = 0.1f;

extern int		SyncAlifeCount = 10;
void register_console_admin()
{
	CMD4(CCC_Integer, "sync_alife_objects", &SyncAlifeCount, 10, 1000);

	CMD4(CCC_Float, "_shedule_ai", &Shedule_Scale_AI_Stalker, 0, 20);
	CMD4(CCC_Float, "_shedule_objects", &Shedule_Scale_Objects, 0, 20);
	CMD4(CCC_Float, "_shedule_zones", &Shedule_Zone, 0, 20);

	// DRAW STATISTIC
	CMD4(CCC_Integer, "draw_mp_statistic", &g_cl_draw_mp_statistic, 0, 1);

	// client side
	CMD1(CCC_TransferMoney, "g_transfer_money");
 
	CMD1(CCC_ADD_Money_to_client_self, "g_money");

	CMD1(CCC_AdmNoClip, "adm_no_clip");
	CMD1(CCC_AdmInvis, "adm_invis");
	CMD1(CCC_AdmGodMode, "adm_god_mode");
	CMD1(CCC_UnlimatedAmmo, "adm_unlimited_ammo");

	CMD1(CCC_AdmSurgeStart, "adm_surge");
	CMD1(CCC_AdmPsiStormStart, "adm_psi_storm");
	CMD1(CCC_AdmFalloutStart, "adm_fallout");

	CMD1(CCC_AdmSurgeStop, "adm_surge_stop");
	CMD1(CCC_AdmFalloutStop, "adm_fallout_stop");
	CMD1(CCC_AdmPsiStormStop, "adm_psi_storm_stop");

 	// server side
	CMD1(CCC_SetNoClipForPlayer, "sv_set_no_clip");
	CMD1(CCC_SetInvisForPlayer, "sv_set_invis");
	CMD1(CCC_SetGodModForPlayer, "sv_set_god_mode");

	CMD1(CCC_MovePlayerToRPoint, "sv_move_player_to_rpoint");
	CMD1(CCC_GiveMoneyToPlayer, "sv_give_money"); 
 	CMD1(CCC_Set_Money_to_client, "sv_set_money");

	// TELEPORTS

	CMD1(CCC_TeleportPlayer, "adm_teleport_to_player");
	CMD1(CCC_TeleportToPlayer, "adm_teleport_player");

	CMD1(CCC_TeleportToPosition, "adm_teleport");
	CMD1(CCC_ChangeTeamPlayer, "adm_set_team");




}