#include "stdafx.h"
#include "pch_script.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_animation_manager.h"
#include "inventory.h"
#include "Actor.h" 			 
#include "ai_stalker_net_state.h"

// Сохранение текущей анимки и нумерация (для синхры анимок)
void CAI_Stalker::OnAnimationUpdate(MotionID motion, CBlend* blend, bool mix_anims, bool anim_ctrl , float pos)
{
	if (blend->bone_or_part == 0)
	{
		if (legs_num > 254)
			legs_num = 0;

		legs_num += 1;
		legs_motion = motion;
		legs_loop = mix_anims;
		legs_pos = pos;
		legs_anim_ctrl = anim_ctrl;
		blend_legs = blend;
	}

	if (blend->bone_or_part == 1)
	{
		if (torso_num > 254)
			torso_num = 0;

		torso_num += 1;
		torso_motion = motion;
		torso_loop = mix_anims;
		torso_pos = pos;
		torso_anim_ctrl = anim_ctrl;
		blend_torso = blend;
	}

	if (blend->bone_or_part == 2)
	{
		if (head_num > 254)
			head_num = 0;

		head_num += 1;
		head_motion = motion;
		head_loop = mix_anims;
		head_pos = pos;
		head_anim_ctrl = anim_ctrl;
		blend_head = blend;
	}	
}

//Релиз тела
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
			return false;

		if (Level().timeServer() - m_last_player_detection_time < 5000)
			return false;

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
 
		if (this->Position().distance_to_sqr(pObject->Position()) < distance_sqr)
		{
			have = true;
			break;
		}
	}

	return have;
}