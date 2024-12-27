#pragma once
#include "net_shared.h"
#include "NET_Common.h"
#include "../xrCore/fastdelegate.h"

class IClient;

class PlayersMonitor
{
private:
	typedef xr_vector<IClient*>	players_collection_t;
	xrCriticalSection			csPlayers;
	players_collection_t		net_Players;
	players_collection_t		net_Players_disconnected;
	bool						now_iterating_in_net_players;
	bool						now_iterating_in_net_players_disconn;

public:
	PlayersMonitor()
	{
		now_iterating_in_net_players = false;
		now_iterating_in_net_players_disconn = false;
	}
 
	template<typename ActionFunctor>
	void ForEachClientDo(ActionFunctor & functor)
	{
 		csPlayers.Enter();
		now_iterating_in_net_players = true;
		for (auto& PS : net_Players)
		{
			// VERIFY2(PS != NULL, "IClient ptr is NULL");
			functor(PS);
		}
		now_iterating_in_net_players = false;
		csPlayers.Leave();
	}
	
	void ForEachClientDo(fastdelegate::FastDelegate1<IClient*, void> & fast_delegate)
	{
		csPlayers.Enter();
		now_iterating_in_net_players = true;
		for (auto& PS : net_Players)
		{
			// VERIFY2(PS != NULL, "IClient ptr is NULL");
			fast_delegate(PS);
		}
		now_iterating_in_net_players = false;
		csPlayers.Leave();
	}
	
	template<typename SearchPredicate, typename ActionFunctor>
	u32	ForFoundClientsDo(SearchPredicate const & predicate, ActionFunctor & functor)
	{
		u32 ret_count = 0;
		csPlayers.Enter();
		now_iterating_in_net_players = true;

		players_collection_t::iterator players_endi = net_Players.end();
		players_collection_t::iterator temp_iter = std::find_if(
			net_Players.begin(),
			players_endi,
			predicate);

		while (temp_iter != players_endi)
		{
			VERIFY2(*temp_iter != NULL, "IClient ptr is NULL");
			functor(*temp_iter);
			temp_iter = std::find_if(++temp_iter, players_endi, predicate);
		}
		now_iterating_in_net_players = false;
 		csPlayers.Leave();

		return ret_count;
	}

	template<typename SearchPredicate>
	IClient*	FindAndEraseClient(SearchPredicate const & predicate)
	{
		csPlayers.Enter();

		VERIFY(!now_iterating_in_net_players);
		now_iterating_in_net_players = true;

		players_collection_t::iterator client_iter = std::find_if(
			net_Players.begin(),
			net_Players.end(),
			predicate);
		IClient* ret_client = NULL;
		if (client_iter != net_Players.end())
		{
			ret_client = *client_iter;
			net_Players.erase(client_iter);
		}
		now_iterating_in_net_players = false;

		csPlayers.Leave();
		return ret_client;
	}

	template<typename SearchPredicate>
	IClient*	GetFoundClient(SearchPredicate const & predicate)
	{
 		csPlayers.Enter();
 
		players_collection_t::iterator client_iter = std::find_if(
			net_Players.begin(),
			net_Players.end(),
			predicate);
		IClient* ret_client = NULL;
		if (client_iter != net_Players.end())
		{
			ret_client = *client_iter;
		}

		csPlayers.Leave();
		return ret_client;
	}

	void		AddNewClient(IClient* new_client)
	{
  		csPlayers.Enter();
		VERIFY(!now_iterating_in_net_players);
		net_Players.push_back(new_client);
		csPlayers.Leave();
	}

	u32			ClientsCount()
	{
 		csPlayers.Enter();
 		u32 ret_count = net_Players.size();
 		csPlayers.Leave();
		return ret_count;
	}
};  
