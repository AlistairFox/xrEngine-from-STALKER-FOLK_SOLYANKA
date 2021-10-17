#include "StdAfx.h"
#include "game_sv_freemp.h"


void game_sv_freemp::OnPlayerUIContacts(NET_Packet& P, ClientID const& clientID)
{
	u16 player;
	P.r_u16(player);
	ClientID cl_leader;
	P.r_clientID(cl_leader);

	u32 leader = cl_leader.value();
	xrClientData* data = (xrClientData*)get_client(player);
	xrClientData* data_leader = (xrClientData*)server().ID_to_client(cl_leader);

	if (!data)
		return;

	if (!data_leader)
		return;

	teamPlayers[leader].leader = leader;

	if (teamPlayers[leader].cur_players == 0)
	{
		teamPlayers[leader].cur_players = 1;
		u32 leader_gameid = data_leader->ps->GameID;

		teamPlayers[leader].players[0].GameID = leader_gameid;
		teamPlayers[leader].players[0].Client = cl_leader;

		Msg("OnPlayerUIContacts Leader GameID [%d] Leader[%u] UI [%d]", leader_gameid, leader, teamPlayers[leader].cur_players);
	}

	if (teamPlayers[leader].cur_players > 3)
	{
		return;
	}

	teamPlayers[leader].players[teamPlayers[leader].cur_players].GameID = player;
	teamPlayers[leader].players[teamPlayers[leader].cur_players].Client = data->ID;

	Msg("OnPlayerUIContacts GameID [%d] ClientID [%d] UI [%d]", player, data->ID, teamPlayers[leader].cur_players);

	teamPlayers[leader].cur_players += 1;

	for (auto pl : teamPlayers[leader].players)
	{
		if (pl.Client != 0)
			OnPlayerUIContactsRecvestUpdate(pl.Client, leader);
	}
}

void game_sv_freemp::OnPlayerUIContactsRecvest(NET_Packet& P, ClientID const& clientID)
{
	Msg("OnPlayerUIContactsRecvest");
	u16 actor;
	ClientID who;
	ClientID id_client_to_send;

	P.r_clientID(id_client_to_send);
	P.r_u16(actor);
	P.r_clientID(who);

	xrClientData* data = server().ID_to_client(who);

	if (!data)
		return;

	NET_Packet packet;
	GenerateGameMessage(packet);
	packet.w_u32(GE_UI_PDA);
	packet.w_u8(0);
	packet.w_clientID(who);
	//packet.w_u8(ui);

	server().SendTo(id_client_to_send, packet, net_flags(true, true));
}

void game_sv_freemp::OnPlayerUIContactsRecvestUpdate(ClientID Client, u32 leader)
{
	NET_Packet packet;
	GenerateGameMessage(packet);
	packet.w_u32(GE_UI_PDA);
	packet.w_u8(1);
	packet.w(&teamPlayers[leader], sizeof(Team));

	Msg("leader[%u] client[%u]", leader, Client);

	server().SendTo(Client, packet, net_flags(true, true));
}