#include "stdafx.h"
#include "pch_script.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_animation_manager.h"
#include "inventory.h"
#include "../xrCore/_flags.h"

#include "ai_stalker_net_state.h"

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
	//u_last_legs_motion_idx = u16(-1);
	//u_last_head_motion_idx = u16(-1);
	//u_last_torso_motion_idx = u16(-1);
}