#include "stdafx.h"
#include "pch_script.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_animation_manager.h"
#include "inventory.h"
#include "../xrCore/_flags.h"

#include "ai_stalker_net_state.h"
#include "Actor.h"

enum sync_flags
{
	animation_flag = (1 << 0),
	animation_update = (1 << 1),
};
 
void CAI_Stalker::OnEventAnimations(bool update)
{
	NET_Packet packet;
	Game().u_EventGen(packet, GE_STALKER_ANIMS, this->ID());
  	Game().u_EventSend(packet);
}

void CAI_Stalker::OnEventAnimationsRecived()
{	
	u_last_script_motion_idx = u16(-1);
}

extern    int        g_iCorpseRemove;
 

bool CAI_Stalker::NeedToDestroyObject() const
{
	if (IsGameTypeSingle() || OnClient())
	{
		return false;
	}
	else
	{
		if (g_Alive() || g_iCorpseRemove == -1 || TimePassedAfterDeath() < m_dwBodyRemoveTime)
		{
			return false;
		}

		if (Level().timeServer() - m_last_player_detection_time < 5000)
		{
			return false;
		}

		if (HavePlayersNearby(50.f))
		{
			// добавляем время на продление "жизни трупа", если был игрок рядом
			m_last_player_detection_time = Level().timeServer();
			return false;
		}

		return true;
	}
}

ALife::_TIME_ID CAI_Stalker::TimePassedAfterDeath()  const
{
	if (!g_Alive())
		return Level().timeServer() - GetLevelDeathTime();
	else
		return 0;
}

bool CAI_Stalker::HavePlayersNearby(float distance) const
{
	bool have = false;
	float distance_sqr = distance * distance;

	auto i = Game().players.begin();
	auto ie = Game().players.end();

	for (; i != ie; ++i)
	{
		game_PlayerState* ps = i->second;
		if (!ps) continue;

		u16 id = ps->GameID;

		CObject* pObject = Level().Objects.net_Find(id);
		CActor* actor = smart_cast<CActor*>(pObject);

		if (!pObject) continue;
		if (!actor) continue;

		if (id == Actor()->ID())
			continue;

		//Msg("ID [%d], DistPos[%.0f], Distance[%.0f]", id,this->Position().distance_to_sqr(pObject->Position()), distance_sqr);

		if (this->Position().distance_to_sqr(pObject->Position()) < distance_sqr)
		{
			have = true;
			break;
		}
	}

	return have;
}