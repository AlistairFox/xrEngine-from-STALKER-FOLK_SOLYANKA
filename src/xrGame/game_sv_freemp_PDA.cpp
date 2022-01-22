#include "StdAfx.h"
#include "game_sv_freemp.h"


void game_sv_freemp::OnPlayerUIContacts(NET_Packet& P, ClientID const& clientID)
{
	ClientID player;
	P.r_clientID(player);

	ClientID leader;
	P.r_clientID(leader);

	if (teamPlayers[leader].cur_players == 0)
	{
		teamPlayers[leader].cur_players = 1;
		teamPlayers[leader].players[0] = leader;
		teamPlayers[leader].ClientLeader = leader.value();
	}

	if (teamPlayers[leader].cur_players > 3)
	{
		return;
	}

 	teamPlayers[leader].players[teamPlayers[leader].cur_players] = player;
	teamPlayers[leader].cur_players += 1;

	//Msg("[OnPlayerUIContacts] LeaderID [%u] Players [%d]", leader.value(), teamPlayers[leader].cur_players + 1);
	//Msg("[OnPlayerUIContacts] ClientID [%u] players [%d]", player.value(), teamPlayers[leader].cur_players + 1);

	for (auto pl : teamPlayers[leader].players)
	{
		if (pl != 0)
			OnPlayerUIContactsRecvestUpdate(pl, leader);
	}
}

void game_sv_freemp::OnPlayerUIContactsRecvest(NET_Packet& P, ClientID const& clientID)
{
	//Msg("OnPlayerUIContactsRecvest");
	
 	ClientID who;
	ClientID id_client_to_send;

	P.r_clientID(id_client_to_send);
 	P.r_clientID(who);

	xrClientData* data = server().ID_to_client(who);

	if (!data)
		return;

	NET_Packet packet;
	GenerateGameMessage(packet);
	packet.w_u32(GE_UI_PDA);
	packet.w_u8(0);
	packet.w_clientID(who);
 
	server().SendTo(id_client_to_send, packet, net_flags(true, true));
}

void game_sv_freemp::OnPlayerUIContactsRecvestUpdate(ClientID Client, ClientID leader)
{
	NET_Packet packet;
	GenerateGameMessage(packet);
	packet.w_u32(GE_UI_PDA);
	packet.w_u8(1);
	packet.w(&teamPlayers[leader] , sizeof(Team));
	server().SendTo(Client, packet, net_flags(true, true));
}
 

void game_sv_freemp::OnPlayerUIContactsRemoveUser(ClientID Client, ClientID Leader)
{
	//Msg("remove user [%u] / leader[%u]", Client, Leader);

	ClientID ids[4];

	u32 id = 0;
	for (auto player : teamPlayers[Leader].players)
	{
		ids[id] = player;
		if (Client == player)
		{
			teamPlayers[Leader].players[id] = 0;
			teamPlayers[Leader].cur_players -= 1;
		}
 
		id += 1;
	}	
	if (Client == Leader)
	{
		teamPlayers.erase(Leader); 
	}

	for (auto pl : ids)
	{
		OnPlayerUIContactsRecvestUpdate(pl, Leader);
	}
}


