#include "StdAfx.h"
#include "game_sv_freemp.h"

#include "string_table.h"

#define SQUAD_CAPACITY 4

//Squad
void game_sv_freemp::join_player_in_squad(NET_Packet& P, u16 id)
{
	Msg("--- Server[SQUAD]  join_player_in_squad: %d", id);

	u16 InviterId = P.r_u16();
	CSE_Abstract* first_member = get_entity_from_eid(InviterId);
	if (!first_member) return;
	game_PlayerState* ps_first_member = first_member->owner->ps;
	if (!ps_first_member) return;

	MP_Squad* squad = NULL;

	//if player already in squad - find it
	if (ps_first_member->MPSquadID != NULL)
	{
		squad = find_squad_by_squadid(ps_first_member->MPSquadID);
		if (squad == NULL) 
		{
			Msg("! SQUAD: Can't find your squad!");
			return;
		}
	}
	//if not create it
	else
	{
		squad = create_squad(ps_first_member);
		ps_first_member->MPSquadID = squad->id;
		signal_Syncronize();
		mp_squads.push_back(squad);
		Msg("* SQUAD: Squad successfully created!");
	}

	//check if we have max players in squad
	if (squad->players.size() == SQUAD_CAPACITY)
	{
		Msg("! SQUAD: You have maximum players in squad!");
	}

	//creating ps of player who wants to join in squad
	CSE_Abstract* new_member = get_entity_from_eid(id);
	game_PlayerState* ps_new_member = new_member->owner->ps;

	//if we created squad and there's no second player 
	//delete squad and null our MPSquadID
	if (!new_member || !ps_new_member)
	{
		if (squad->players.size() == 1)
		{
			squad->players.clear();
			delete_squad(ps_first_member->MPSquadID);
			ps_first_member->MPSquadID = 0;
			signal_Syncronize();
			return;
		}
		else
		{
			return;
		}
	}

	//if he is already in our squad deny invite
	if (check_player_in_squad(ps_new_member, squad)) {
		Msg("! SQUAD: Player already in your squad!");
		return;
	}

	//setting new MPSquadID and push this player to players vector
	ps_new_member->MPSquadID = squad->id;
	//say everyone that we joined in squad!
	signal_Syncronize();

	squad->players.push_back(ps_new_member);
	SendMpSuqadToMembers(squad);
}

void game_sv_freemp::delete_player_from_squad(u16 id)
{
	//Who kicked
	CSE_Abstract* WhoKicked = get_entity_from_eid(id);
	if (!WhoKicked) return;
	game_PlayerState* psWhoKicked = WhoKicked->owner->ps;
	if (!psWhoKicked) return;

	MP_Squad* squad = NULL;

	//find squad in squads vector
	squad = find_squad_by_squadid(psWhoKicked->MPSquadID);
	if (squad == NULL) {
		Msg("! SQUAD: Can't delete player because can't find your squad!");
		return;
	}

	delete_player_from_player_list(squad, psWhoKicked);

	if (WhoKicked->owner->ID == squad->squad_leader_cid)
	{
		find_new_squad_leader(squad->id);
	}

	if (squad->players.size() == 1)
	{
		u16 pLastPlayerID = squad->players[0]->GameID;

		squad->players[0]->MPSquadID = 0;
		signal_Syncronize();

		squad->players.clear();
		delete_squad(squad->id);

		CSE_Abstract* LastPlayer = get_entity_from_eid(pLastPlayerID);
		if (!LastPlayer)
			return;

		NET_Packet PLastPlayer;
		GenerateGameMessage(PLastPlayer);
		PLastPlayer.w_u32(GE_PDA_SQUAD_KICK_PLAYER);
		string32 msgLP;
		xr_sprintf(msgLP, CStringTable().translate("mp_squad_disbanded").c_str()); //"Отряд расформирован"
		PLastPlayer.w_stringZ(msgLP);
		m_server->SendTo(LastPlayer->owner->ID, PLastPlayer, net_flags(TRUE, TRUE));

		return;
	}
	else
		SendMpSuqadToMembers(squad);
}

void game_sv_freemp::delete_player_from_squad(NET_Packet& P, u16 id)
{
	//Who will be kicked
	u16 WhoKickedID = P.r_u16();
	CSE_Abstract* WhoKicked = get_entity_from_eid(WhoKickedID);
	if (!WhoKicked) return;
	game_PlayerState* psWhoKicked = WhoKicked->owner->ps;
	if (!psWhoKicked) return;

	//Initiator of kick
	CSE_Abstract* WhoKick = get_entity_from_eid(id);
	if (!WhoKick) return;
	game_PlayerState* psWhoKick = WhoKick->owner->ps;
	if (!psWhoKick) return;

	MP_Squad* squad = NULL;
	bool self_kick = false;

	//find squad in squads vector
	squad = find_squad_by_squadid(psWhoKicked->MPSquadID);
	if (squad == NULL) {
		Msg("! SQUAD: Can't delete player because can't find your squad!");
		return;
	}

	//check if kicker is squad leader or player himself
	if (WhoKick->owner->ID != squad->squad_leader_cid && psWhoKick->GameID != psWhoKicked->GameID)
	{
		Msg("! SQUAD: Can't delete player because you are not squad leader!");
		return;
	}

	delete_player_from_player_list(squad, psWhoKicked);

	if (WhoKicked->owner->ID == squad->squad_leader_cid)
	{
		find_new_squad_leader(squad->id);
	}

	if (psWhoKick->GameID == psWhoKicked->GameID)
	{
		NET_Packet P_;
		GenerateGameMessage(P_);
		P_.w_u32(GE_PDA_SQUAD_KICK_PLAYER);
		string32 msg;
		xr_sprintf(msg, CStringTable().translate("mp_squad_self_left").c_str()); //Вы вышли из отряда
		P_.w_stringZ(msg);
		m_server->SendTo(WhoKick->owner->ID, P_, net_flags(TRUE, TRUE));

		self_kick = true;
	}

	if (squad->players.size() == 1)
	{
		u16 pLastPlayerID = squad->players[0]->GameID;

		squad->players[0]->MPSquadID = 0;
		signal_Syncronize();

		squad->players.clear();
		delete_squad(squad->id);

		CSE_Abstract* LastPlayer = get_entity_from_eid(pLastPlayerID);
		if (!LastPlayer)
			return;

		if (!self_kick) 
		{
			NET_Packet P_;
			GenerateGameMessage(P_);
			P_.w_u32(GE_PDA_SQUAD_KICK_PLAYER);
			string32 msg;
			xr_sprintf(msg, CStringTable().translate("mp_squad_kick").c_str()); //Вас исключили из отряда
			P_.w_stringZ(msg);
			m_server->SendTo(WhoKicked->owner->ID, P_, net_flags(TRUE, TRUE));
		}

		NET_Packet PLastPlayer;
		GenerateGameMessage(PLastPlayer);
		PLastPlayer.w_u32(GE_PDA_SQUAD_KICK_PLAYER);
		string32 msgLP;
		xr_sprintf(msgLP, CStringTable().translate("mp_squad_disbanded").c_str()); //"Отряд расформирован"
		PLastPlayer.w_stringZ(msgLP);
		m_server->SendTo(LastPlayer->owner->ID, PLastPlayer, net_flags(TRUE, TRUE));

		return;
	}
	else
		SendMpSuqadToMembers(squad);

	if (self_kick)
		return;

	NET_Packet P_;
	GenerateGameMessage(P_);
	P_.w_u16(GE_PDA_SQUAD_KICK_PLAYER);
	string32 msg;
	xr_sprintf(msg, CStringTable().translate("mp_squad_kick").c_str()); //Вас исключили из отряда
	P_.w_stringZ(msg);
	m_server->SendTo(WhoKicked->owner->ID, P_, net_flags(TRUE, TRUE));

}

void game_sv_freemp::delete_player_from_player_list(MP_Squad* squad, game_PlayerState* pPlayer)
{
	if (squad == nullptr)
		return;

	xr_vector<game_PlayerState*>::const_iterator it = squad->players.begin();
	xr_vector<game_PlayerState*>::const_iterator it_e = squad->players.end();

	for (; it != it_e; ++it)
	{
		if ((*it)->GameID == pPlayer->GameID)
		{
			squad->players.erase(it);
			pPlayer->MPSquadID = 0;
			signal_Syncronize();
		}
	}
}

game_sv_freemp::MP_Squad* game_sv_freemp::create_squad(game_PlayerState* ps)
{
	ClientID client = get_entity_from_eid(ps->GameID)->owner->ID;

	MP_Squad* squad = xr_new<MP_Squad>();
	squad->squad_leader_cid = client;
	squad->id = ps->GameID;
	squad->players.push_back(ps);
	squad->current_map_point = NULL;
	// squad->current_quest = sv_tasks->FindTaskByClientID(client);

	return squad;
}

void game_sv_freemp::delete_squad(u16 squad_id)
{
	xr_vector<MP_Squad*>::const_iterator it = mp_squads.begin();
	xr_vector<MP_Squad*>::const_iterator it_e = mp_squads.end();

	for (; it != it_e; ++it)
	{
		if ((*it)->id == squad_id)
			mp_squads.erase(it);
	}
}

game_sv_freemp::MP_Squad* game_sv_freemp::find_squad_by_squadid(u16 id)
{
	xr_vector<MP_Squad*>::const_iterator it = mp_squads.begin();
	xr_vector<MP_Squad*>::const_iterator it_e = mp_squads.end();

	for (; it != it_e; ++it)
	{
		if ((*it)->id == id)
			return (*it);
	}
	return 0;
}

bool game_sv_freemp::check_player_in_squad(game_PlayerState* ps, MP_Squad* squad)
{
	xr_vector<game_PlayerState*>::const_iterator it = squad->players.begin();
	xr_vector<game_PlayerState*>::const_iterator it_e = squad->players.end();

	for (; it != it_e; ++it)
	{
		if ((*it)->GameID == ps->GameID)
			return true;
	}

	return false;
}

void game_sv_freemp::find_new_squad_leader(u16 squad_id)
{
	MP_Squad* squad = find_squad_by_squadid(squad_id);
	if (squad == NULL) {
		Msg("! SQUAD: Can't find your squad!");
		return;
	}

	if (squad->players.size() <= 1)
		return;

	xrClientData* data = (xrClientData*)get_client(squad->players[0]->GameID);
	if (!data)
		return;

	squad->squad_leader_cid = data->ID;

	NET_Packet P1;
	string64 msg;
	GenerateGameMessage(P1);
	P1.w_u32(GE_PDA_SQUAD_MAKE_LEADER);
	xr_sprintf(msg, CStringTable().translate("mp_squad_new_leader_self").c_str()); //Вы назначены новым лидером отряда
	P1.w_stringZ(msg);

	m_server->SendTo(data->ID, P1, net_flags(TRUE, TRUE));
}

void game_sv_freemp::SendMpSuqadToMembers(MP_Squad* squad)
{
	xr_vector<u16> ID;

	u8 capacity = (u8)squad->players.size();

	NET_Packet P;
	GenerateGameMessage(P);
	P.w_u32(GE_PDA_SQUAD_RESPOND_INVITE);
	P.w_clientID(squad->squad_leader_cid);
	P.w_u16(squad->id);
	P.w_u16(squad->current_map_point);
	P.w_stringZ(squad->current_quest);
	P.w_u8(capacity);
	string32 msg;
	xr_sprintf(msg, CStringTable().translate("mp_squad_update").c_str()); //Отряд обновлён
	P.w_stringZ(msg);

	for (u32 o_it = 0; o_it < squad->players.size(); o_it++)
	{
		u16 id = squad->players[o_it]->GameID;

		xrClientData* src = (xrClientData*)(get_client(id));
		if (!src)
			continue;

		for (u32 IDPL= 0; IDPL < capacity; IDPL++)
		{
			P.w_u16(squad->players[IDPL]->GameID);
		};

		m_server->SendTo(src->ID, P, net_flags(TRUE, TRUE));
	}
}

void game_sv_freemp::make_player_squad_leader(NET_Packet& P, u16 id)
{
	u16 NewLeaderID = P.r_u16();
	CSE_Abstract* NewLeader = get_entity_from_eid(NewLeaderID);
	if (!NewLeader)
		return;
	game_PlayerState* psNewLeader = NewLeader->owner->ps;
	if (!psNewLeader)
		return;

	CSE_Abstract* OldLeader = get_entity_from_eid(id);
	if (!OldLeader)
		return;
	game_PlayerState* psOldLeader = OldLeader->owner->ps;
	if (!psOldLeader)
		return;

	MP_Squad* squad = find_squad_by_squadid(psOldLeader->MPSquadID);
	if (squad == NULL) {
		Msg("! SQUAD: Can't find your squad!");
		return;
	}

	if (OldLeader->owner->ID != squad->squad_leader_cid)
	{
		Msg("! SQUAD: You are not squad leader!");
		return;
	}

	squad->squad_leader_cid = NewLeader->owner->ID;
	SendMpSuqadToMembers(squad);

	NET_Packet P_;
	GenerateGameMessage(P_);
	P_.w_u32(GE_PDA_SQUAD_MAKE_LEADER);
	string64 msg;
	xr_sprintf(msg, "%s %s %s", CStringTable().translate("mp_squad_make_leader_0").c_str(), psNewLeader->getName(), CStringTable().translate("mp_squad_make_leader_1").c_str()); //"Вы назначили %s лидером отряда"
	P_.w_stringZ(msg);
	m_server->SendTo(OldLeader->owner->ID, P_, net_flags(TRUE, TRUE));

	NET_Packet P1;
	GenerateGameMessage(P1);
	P1.w_u32(GE_PDA_SQUAD_MAKE_LEADER);
	xr_sprintf(msg, CStringTable().translate("mp_squad_make_leader_self").c_str()); //"Вы назначены новым лидером отряда"
	P1.w_stringZ(msg);

	m_server->SendTo(NewLeader->owner->ID, P1, net_flags(TRUE, TRUE));
}
