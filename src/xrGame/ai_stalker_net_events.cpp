#include "stdafx.h"
#include "pch_script.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_animation_manager.h"
#include "inventory.h"
#include "Actor.h" 			 
#include "ai_stalker_net_state.h"
#include "animation_movement_controller.h"
 
// для синхры анимок 
void CAI_Stalker::OnAnimationUpdate(MotionID motion, CBlend* blend, bool mix_anims, bool is_global, bool anim_controller, bool isLocal, bool isLegs, Fmatrix* matrix)
{
	if (!blend)
		return;

	NET_Packet packet;
	Game().u_EventGen(packet, GE_STALKER_ANIMATION, this->ID());
	packet.w(&motion, sizeof(MotionID));
	packet.w_u8(mix_anims);

 	packet.w_u8(is_global);
	packet.w_u8(anim_controller);
	packet.w_u8(isLocal);
	packet.w_u8(isLegs);
	 
	if (anim_controller && matrix)
	{
		packet.w_u8(1);
		packet.w_matrix(*matrix);
	}
	else
	{
		packet.w_u8(0);
	}

	Game().u_EventSend(packet);
}

/*
bone_or_part:
0: legs
1: torso
2: head
*/


void CAI_Stalker::EventAnimation(NET_Packet& P)
{
	if (OnServer() || !g_Alive())
		return;
 
	if (!Level().game_configured)
	{
		Msg("Game is not configured");
		return;
	}
 
	IKinematicsAnimated* ka = smart_cast<IKinematicsAnimated*>(Visual());
	if (!ka)
		return;

	if (!RDEVICE.b_isLoadTextures || !RDEVICE.b_is_Ready || !isLoaded)
	{
		Msg("Skip Process Event: NPC: %d, IsLoaded: %d", ID(), isLoaded);
		return;
	}

	ka->SetNPC(true);

	MotionID motion;
	bool mix_anims, is_global, anim_controller, isLocal, isLegs;
	P.r(&motion, sizeof(MotionID));
	mix_anims = P.r_u8();

	is_global = P.r_u8();
	anim_controller = P.r_u8();
	isLocal = P.r_u8();
	isLegs = P.r_u8();
	
	bool use_matrix = P.r_u8();
	if (use_matrix)
  		P.r_matrix(target_matrix);
 	
	CBlend* m_blend = 0;
	if (!is_global)
	{
		// Msg("Event Animation Parts [%s]", cName().c_str());
		if (motion.slot == 0)
			last_legs_idx = motion.idx;
		if (motion.slot == 1)
			last_torso_idx = motion.idx;
		if (motion.slot == 2)
			last_head_idx = motion.idx;

		last_script_idx = -1;
		last_global_idx = -1;

		if (animation_movement())
			animation_movement()->stop();
 
 		m_blend = ka->PlayCycle(motion, mix_anims);
	}
	else 
	{
 		last_legs_idx  = -1;
 		last_torso_idx = -1;
 		last_head_idx  = -1;

		last_script_idx = motion.idx;
		last_global_idx = motion.idx;

		// Msg("Event Animation SCRIPT [%s]", cName().c_str());


		for (u16 i = 0; i < MAX_PARTS; ++i)
		{
			CBlend* blend = 0;
			if (!m_blend)
			{
				blend = ka->LL_PlayCycle(i, motion, mix_anims, 0, 0);

				if (blend && !m_blend)
					m_blend = blend;

				if (anim_controller)
				{
					create_anim_mov_ctrl(blend, &target_matrix, isLocal);
				}
				else
				{
					if (animation_movement() && is_global)
						animation_movement()->stop();
				}
			}
			else
				ka->LL_PlayCycle(i, motion, mix_anims, 0, 0);
		}
	}	 

	CStepManager::on_animation_start(motion, m_blend);
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