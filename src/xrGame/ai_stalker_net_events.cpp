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

void CAI_Stalker::OnEventUpdate(MotionID motion, CBlend* blend, bool mix_anims, float pos)
{
	//Msg("OnEventUpdate [%d], part[%d]", motion.idx, blend->bone_or_part);
 

	if (blend->bone_or_part == 1)
	{
		if (torso_anim_id > 254)
			torso_anim_id = 0;

		torso_anim_id += 1;
		motion_torso = motion;
		torso_loop = mix_anims;
		pos_torso = pos;
	}

	if (blend->bone_or_part == 0)
	{
		if (legs_anim_id > 254)
			legs_anim_id = 0;

		legs_anim_id += 1;
		motion_legs = motion;
		legs_loop = mix_anims;
		pos_legs = pos;
	}

	if (blend->bone_or_part == 2)
	{
		if (head_anim_id > 254)
			head_anim_id = 0;

		head_anim_id += 1;
		motion_head = motion;
		head_loop = mix_anims;
		pos_head = pos;
	}	
}

void CAI_Stalker::OnEventAnimationsRecived(NET_Packet packet)
{ 		  
	MotionID id;
	u16 slot;
	float amount;
	packet.r(&id, sizeof(id));
	packet.r_u16(slot);
 
	IKinematicsAnimated* ka = Visual()->dcast_PKinematicsAnimated();

	if (!ka)
		return;

	bool has_slot_for_anim = false;

	if (ka->LL_PartBlendsCount(0) < 6 && ka->LL_PartBlendsCount(1) < 6 &&  ka->LL_PartBlendsCount(2) < 6)
			has_slot_for_anim = true;

	if (has_slot_for_anim)
		ka->LL_PlayCycle(slot, id, true, 0, 0, 0);
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