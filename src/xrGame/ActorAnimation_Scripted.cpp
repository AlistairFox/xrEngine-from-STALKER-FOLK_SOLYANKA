#include "stdafx.h"
#include "Actor.h"
#include "ActorAnimation.h"
#include "actor_anim_defs.h"
#include "script_sound.h"
#include "ai_sounds.h"
#include "Inventory.h"

#include "../Include/xrRender/Kinematics.h"
#include "Weapon.h"

#include "ui/UIActorMenu.h"

bool CActor::ActorAnimationBlockedUpdate(u32 mstate_rl)
{
	/// Msg("Update MP ANIM: %s", MpAnimationMode()? "true" : "false");
 

	if (Level().CurrentControlEntity() == this)
	{
		if (MpAnimationMode() || NEED_EXIT)
		{
			SelectScriptAnimation(mstate_rl);
			script_animation_legs(mstate_rl);
			return true;
		}
	}
	else
	{
		if (MpAnimationMode())
		{
			if (ANIM_CAN_WALK)
			{
				SelectScriptAnimation_Walking(mstate_rl);
				script_animation_legs(mstate_rl);
			}
			return true;
		}
	}

	InputAnim = 0;
	OutAnim = 0;
	MidAnim = 0;

	return false;
}

bool CActor::MpAnimationMode() const
{
	if (!CanUseAnimationsMenu)
		return false;

	if (Level().CurrentControlEntity() == this)
	{
		if (!CanChange || !InPlay || !MidPlay || !OutPlay)
			return true;

		return ANIM_SELECTED > 0;
	}
	else
	{
		if (ANIM_CAN_WALK)
			return true;

		return !CanChange;
	}
}

void callbackAnim(CBlend* blend)
{
	CActor* act = (CActor*)blend->CallbackParam;
	if (act)
		act->CanChange = true;
}

CBlend* PlayMotionByParts(IKinematicsAnimated* sa, MotionID motion_ID, BOOL bMixIn, PlayCallback Callback, LPVOID CallbackParam);

void AnimationDataScripted(CActor* actor_scripted, MotionID& actor_selected, bool actor_can_walk, LPCSTR& MSTATE, CBlend* m_current_legs_blend)
{
	if (actor_scripted == nullptr)
		return;

	CGameFont* font = UI().Font().pFontArial14;

	font->SetHeightI(0.02f);
	font->OutSet(320, 50);
	font->SetColor(color_argb(255, 255, 128, 128));

	string128 legsidx = { 0 };
	xr_sprintf(legsidx, "LegsIDX: %u, LegsPlay: %d", actor_scripted->m_current_legs.idx);

	string128 LegsSel = { 0 };
	xr_sprintf(LegsSel, "LegsSel: %u", actor_selected.idx);

	string128 ActorCanWalk = { 0 };
	xr_sprintf(ActorCanWalk, "Can_walk: %d", actor_can_walk);

	string128 actor_MState = { 0 };
	xr_sprintf(actor_MState, "Mstate_rl: %s", MSTATE);

	string128 asel_valid = { 0 };
	xr_sprintf(asel_valid, "Valid: %d", actor_selected.valid());

	string128 current_data = { 0 };
	if (m_current_legs_blend != nullptr)
	{
		xr_sprintf(current_data, "Blend_StopAtEnd: %d, Blend_StopCall: %d, BlendPlayed: %d, BLendTime: %f, BlendTotal: %f, BlendAmount: %f",
			m_current_legs_blend->stop_at_end, m_current_legs_blend->stop_at_end_callback, m_current_legs_blend->playing,
			m_current_legs_blend->timeCurrent, m_current_legs_blend->timeTotal, m_current_legs_blend->blendAmount
		);
	}

	font->OutNext(legsidx);
	font->OutNext(LegsSel);
	font->OutNext(ActorCanWalk);
	font->OutNext(actor_MState);
	font->OutNext(asel_valid);

	if (m_current_legs_blend != nullptr)
		font->OutNext(current_data);
}


void legs_play_callback_script(CBlend* blend)
{
	CActor* object = (CActor*)blend->CallbackParam;
	VERIFY(object);

	object->m_current_legs.invalidate();
}

void AnimTorsoPlayCallBack_script(CBlend* blend)
{
	CActor* object = (CActor*)blend->CallbackParam;

	object->m_current_torso.invalidate();
}



#include "../xrEngine/motion.h"

void CActor::SelectScriptAnimation_Walking(u32 mstate_rl)
{
	if (!InitItems)
	{
		InitItems = true;
		StartAnimItems();
	}

	MotionID M_torso;

	if (mstate_rl & mcAnyMove)
	{
		M_torso = m_anims->m_script.walking_anims.m_animation_torso_walk[ANIM_CURRENT_PLAYED];
	}
	else
	{
		M_torso = m_anims->m_script.walking_anims.m_animation_torso_idle[ANIM_CURRENT_PLAYED];
	}

	if (m_current_torso_blend && m_current_torso_blend->stop_at_end)
		m_current_torso.invalidate();

	if (m_current_torso != M_torso && M_torso.valid())
	{
		m_current_torso_blend = smart_cast<IKinematicsAnimated*>(Visual())->PlayCycle(M_torso);

		m_current_torso = M_torso;
	}


	if (!m_current_legs_blend)
		return;

	if (!m_current_torso_blend)
		return;

	IKinematicsAnimated* skeleton_animated = smart_cast<IKinematicsAnimated*>(Visual());

	CMotionDef* motion0 = skeleton_animated->LL_GetMotionDef(m_current_torso);
	VERIFY(motion0);
	if (!(motion0->flags & esmSyncPart))
		return;

	CMotionDef* motion1 = skeleton_animated->LL_GetMotionDef(m_current_legs);
	VERIFY(motion1);

	if (!dynamic_cast<CMotionDef*>(motion1))
		return;

	if (!(motion1->flags & esmSyncPart))
		return;


	m_current_torso_blend->timeCurrent = m_current_legs_blend->timeCurrent / m_current_legs_blend->timeTotal * m_current_torso_blend->timeTotal;
}


void CActor::script_animation_legs(u32 mstate_rl)
{
	if (ANIM_CAN_WALK)
	{
		MotionID M_legs;

		if (mstate_rl & mcFwd)
			M_legs = m_anims->m_script.walking_anims.m_animation_fwd[ANIM_CURRENT_PLAYED];
		else if (mstate_rl & mcBack)
			M_legs = m_anims->m_script.walking_anims.m_animation_back[ANIM_CURRENT_PLAYED];
		else if (mstate_rl & mcLStrafe)
			M_legs = m_anims->m_script.walking_anims.m_animation_ls[ANIM_CURRENT_PLAYED];
		else if (mstate_rl & mcRStrafe)
			M_legs = m_anims->m_script.walking_anims.m_animation_rs[ANIM_CURRENT_PLAYED];
		else
			M_legs = m_anims->m_script.walking_anims.m_animation_idle[ANIM_CURRENT_PLAYED];

		/*
		LPCSTR MSTATE;
		if (mstate_rl & mcFwd)
			MSTATE = "mcFWD";
		else if (mstate_rl & mcBack)
			MSTATE = "mcBACK";
		else if (mstate_rl & mcLStrafe)
			MSTATE = "mcLStrafe";
		else if (mstate_rl & mcRStrafe)
			MSTATE = "mcRStrafe";
		else
			MSTATE = "mcIDLE_safe";

		if (OnClient())
			AnimationDataScripted(this, M_legs, can_walk, MSTATE, m_current_legs_blend);
 		*/

		IKinematicsAnimated* k = smart_cast<IKinematicsAnimated*>(Visual());

		if (m_current_legs_blend && m_current_legs_blend->stop_at_end)
			m_current_legs.invalidate();


		if (m_current_legs != M_legs && M_legs.valid() || !m_current_legs_blend)
		{
			float pos = 0.f;
			VERIFY(!m_current_legs_blend || !fis_zero(m_current_legs_blend->timeTotal));

			if ((mstate_real & mcAnyMove) && (mstate_old & mcAnyMove) && m_current_legs_blend)
				pos = fmod(m_current_legs_blend->timeCurrent, m_current_legs_blend->timeTotal) / m_current_legs_blend->timeTotal;


			IKinematicsAnimated* ka = smart_cast<IKinematicsAnimated*>(Visual());
			m_current_legs_blend = PlayMotionByParts(ka, M_legs, TRUE, legs_play_callback_script, this);

			//if ((!(mstate_old&mcAnyMove))&&(mstate_real&mcAnyMove))
			//	pos						= 0.5f; 

			if (m_current_legs_blend)
			{
				m_current_legs_blend->timeCurrent = m_current_legs_blend->timeTotal * pos;
				m_current_legs_blend->stop_at_end_callback = true;
			}

			m_current_legs = M_legs;

			CStepManager::on_animation_start(M_legs, m_current_legs_blend);
		}
	}
}

// OLDER  CODE

void SActorStateAnimation::CreateAnimationsScripted(IKinematicsAnimated* K)
{
	string_path filepath;
	FS.update_path(filepath, "$game_config$", "alife\\actor_anims.ltx");
	CInifile* file = xr_new<CInifile>(filepath, true, true);

	actor->CanUseAnimationsMenu = false;

	if (actor && file)
	{
		u32 count = file->r_u8("animations", "count");

		for (int i = 0; i < count; i++)
		{
			string32 tmp = { 0 };
			_itoa(i, tmp, 10);
			string32 animation = { 0 };
			xr_strcat(animation, "anim_");
			xr_strcat(animation, tmp);

			if (file->section_exist(animation))
			{
				if (file->line_exist(animation, "anim_walk"))
					m_animation_can_walk[i] = file->r_bool(animation, "anim_walk");
				else
					m_animation_can_walk[i] = false;

				if (file->line_exist(animation, "attached_item"))
					m_animation_attach[i] = file->r_string(animation, "attached_item");

				if (file->line_exist(animation, "anim_use_slot"))
					m_animation_use_slot[i] = file->r_u16(animation, "anim_use_slot");

				if (m_animation_can_walk[i])
				{
					MotionID motion_fwd = K->ID_Cycle_Safe(file->r_string(animation, "anim_legs_fwd"));
					MotionID motion_back = K->ID_Cycle_Safe(file->r_string(animation, "anim_legs_back"));
					MotionID motion_ls = K->ID_Cycle_Safe(file->r_string(animation, "anim_legs_ls"));
					MotionID motion_rs = K->ID_Cycle_Safe(file->r_string(animation, "anim_legs_rs"));
					MotionID motion_idle = K->ID_Cycle_Safe(file->r_string(animation, "anim_legs_idle"));

					MotionID motion_twalk = K->ID_Cycle_Safe(file->r_string(animation, "anim_torso_walk"));
					MotionID motion_tidle = K->ID_Cycle_Safe(file->r_string(animation, "anim_torso_idle"));


					bool condition = motion_fwd.valid() && motion_back.valid() && motion_ls.valid() && motion_rs.valid() && motion_idle.valid();
					bool condition_t = motion_twalk.valid() && motion_tidle.valid();

					if (!condition || !condition_t)
					{
						string256 motions_info;
						xr_sprintf(motions_info, "~~~ Motions Validation: fwd: (%d), back: (%d), ls(%d), rs(%d), idle(%d), tidle(%d), twalk(%d)", motion_fwd.valid(), motion_back.valid(), motion_ls.valid(), motion_rs.valid(), motion_idle.valid(), motion_tidle.valid(), motion_twalk.valid());

						actor->CanUseAnimationsMenu = false;
						Msg("%s", motions_info);
					}

					walking_anims.m_animation_fwd[i] = motion_fwd;
					walking_anims.m_animation_back[i] = motion_back;
					walking_anims.m_animation_ls[i] = motion_ls;
					walking_anims.m_animation_rs[i] = motion_rs;
					walking_anims.m_animation_idle[i] = motion_idle;
					walking_anims.m_animation_torso_idle[i] = motion_tidle;
					walking_anims.m_animation_torso_walk[i] = motion_twalk;
 				}
				else
				{
					m_animation_loop[i] = file->r_bool(animation, "anim_loop");


					if (file->line_exist(animation, "anim_snd"))
					{
						shared_str snd = file->r_string(animation, "anim_snd");
						u32 snds = 0;
						if (file->line_exist(animation, "anim_snd_rnd"))
							snds = file->r_u32(animation, "anim_snd_rnd");
						else
							snds = 1;
						for (u32 snd_i = 1; snd_i <= snds; snd_i++)
						{
							string32 tmp = { 0 };
							_itoa(snd_i, tmp, 10);
							string128 path_snd = { 0 };
							xr_strcat(path_snd, snd.c_str());
							xr_strcat(path_snd, tmp);
							m_sound_Animation[i][snd_i].create(path_snd, st_Effect, 0);
						}
						m_rnd_snds[i] = snds;
					}
 
					LPCSTR anims_in = file->r_string(animation, "anim_in");
					LPCSTR anims_out = file->r_string(animation, "anim_out");
					LPCSTR anims_middle = file->r_string(animation, "anim_middle");
					u32 countIN = _GetItemCount(anims_in, ',');
					u32 countOUT = _GetItemCount(anims_out, ',');
					u32 countMID = _GetItemCount(anims_middle, ',');

					in_anims.count[i] = countIN;
					out_anims.count[i] = countOUT;
					middle_anims.count[i] = countMID;

					for (u32 id = 0; id != countIN; id++)
					{
						string64 anim = { 0 };
						_GetItem(anims_in, id, anim, ',');
						MotionID motionAnim = K->ID_Cycle_Safe(anim);
						in_anims.m_animation_in[i][id] = motionAnim;

						if (!motionAnim.valid())
						{
							//R_ASSERT(0, "Failed Load: Animation: %s (alife\\actor_anims.ltx)", anim);
							actor->CanUseAnimationsMenu = false;
							Msg("~~~ Failed Load: Animation: %s (alife\\actor_anims.ltx)", anim);
							return;
						}
					}

					for (u32 id = 0; id != countOUT; id++)
					{
						string64 anim = { 0 };
						_GetItem(anims_out, id, anim, ',');
						MotionID motionAnim = K->ID_Cycle_Safe(anim);
						out_anims.m_animation_out[i][id] = motionAnim;

						if (!motionAnim.valid())
						{
							actor->CanUseAnimationsMenu = false;
							Msg("~~~ Failed Load: Animation: %s (alife\\actor_anims.ltx)", anim);
							// return;
							//R_ASSERT(0, "Failed Load: Animation: %s (alife\\actor_anims.ltx)", anim);
						}
					}

					for (u32 id = 0; id != countMID; id++)
					{
						string64 anim = { 0 };
						_GetItem(anims_middle, id, anim, ',');
						MotionID motionAnim = K->ID_Cycle_Safe(anim);
						middle_anims.m_animation[i][id] = motionAnim;

						if (!motionAnim.valid())
						{
							actor->CanUseAnimationsMenu = false;
							Msg("~~~ Failed Load: Animation: %s (alife\\actor_anims.ltx)", anim);
							//R_ASSERT(0, "Failed Load: Animation: %s (alife\\actor_anims.ltx)", anim);
						}
					}

				}

			}

		}
	}

	actor->CanUseAnimationsMenu = true;
}

// NETWORK CODE 

void CActor::script_anim(MotionID Animation, PlayCallback Callback, LPVOID CallbackParam)
{
	/*
	IKinematicsAnimated* k = smart_cast<IKinematicsAnimated*>(Visual());
	k->LL_PlayCycle(
		k->LL_GetMotionDef(Animation)->bone_or_part,
		Animation,
		TRUE,
		k->LL_GetMotionDef(Animation)->Accrue(),
		k->LL_GetMotionDef(Animation)->Falloff(),
		k->LL_GetMotionDef(Animation)->Speed(),
		k->LL_GetMotionDef(Animation)->StopAtEnd(),
		Callback, CallbackParam, 0
	);
	*/

	CanChange = false;
	SendAnimationToServer(Animation);
}

/*
ID: [0] name: legs
ID: [1] name: torso
ID: [2] name: head
ID: [3] name: (null)
*/

void CActor::ReciveAnimationPacket(NET_Packet& packet)
{
	MotionID motion;
	packet.r(&motion, sizeof(motion));

	IKinematicsAnimated* ka = smart_cast<IKinematicsAnimated*>(Visual());

	if (motion.valid())
	{
		auto defination = ka->LL_GetMotionDef(motion);

		if (this->g_Alive() && ka->get_animation_valid(motion))
		{
			ka->LL_PlayCycle(
				defination->bone_or_part,
				motion,
				true,
				defination->Accrue(),
				defination->Falloff(),
				defination->Speed(),
				defination->StopAtEnd(),
				callbackAnim, this, 0
			);
			CanChange = false;
		}
		else
		{
			Msg("~~~ [ERROR] Try Set Animetion On Actor: %s, motion idx[%d] slot[%d], alive[%d]", this->Name(), motion.idx, motion.slot, this->g_Alive());
		}
	}

}

void CActor::ReciveActivateItem(NET_Packet& packet)
{
	u8 type;
	packet.r_u8(type);

	if (type == 0)
	{
		shared_str item;
		packet.r_stringZ(item);
		bool activate = packet.r_u8();

		Msg("Activate Item Name [%s] -> [%d]", item.c_str(), activate);

		CInventoryItem* inv_item = this->inventory().GetItemFromInventory(item.c_str());
		CAttachableItem* item_attach = smart_cast<CAttachableItem*>(inv_item);

		if (item_attach)
		{
			//if (this->can_attach(inv_item) && activate)
			{
				//	this->attach(inv_item);
			}

			if (activate)
			{
				this->attach_no_check(inv_item);
				item_attach->enable(true);
			}
			else
			{
				item_attach->enable(false);
				this->detach(inv_item);
			}
		}
	}
	else if (type == 1)
	{
		u8 slot;
		packet.r_u8(slot);
		bool activate = packet.r_u8();

		PIItem item = this->inventory().ItemFromSlot(slot);

		if (item)
		{
			if (activate && inventory().GetActiveSlot() != slot)
				inventory().Activate(slot);
		}

		if ((!activate || !item) && inventory().GetActiveSlot() != NO_ACTIVE_SLOT)
			inventory().Activate(NO_ACTIVE_SLOT);
	}
}

void CActor::ReciveSoundPlay(NET_Packet packet)
{
	u32 snd_id;
	packet.r_u32(snd_id);
	bool activate = packet.r_u8();

	Msg("Recive SND Packet [%d] / act[%d]", snd_id, activate);

	if (activate)
	{
		if (selected._p)
			if (!selected._feedback())
			{
				selected.stop();
			}

		ref_sound snd_sel = m_anims->m_script.m_sound_Animation[ANIM_CURRENT_PLAYED][snd_id];

		if (!snd_sel._feedback())
		{
			snd_sel.play_at_pos(this, Position(), false, 0);
			selected = snd_sel;
		}
	}
	else
	{
		if (selected._p)
			if (selected._feedback())
			{
				selected.stop();
			}
	}

}

/* NETWORK SEND CODE */

void CActor::SendAnimationToServer(MotionID motion)
{
	NET_Packet packet;
	u_EventGen(packet, GE_ACTOR_ANIMATION_MOTIONID, this->ID());
	packet.w(&motion, sizeof(motion));
	u_EventSend(packet, net_flags(true, true));
}

void CActor::SendActivateItem(shared_str item, bool activate)
{
	NET_Packet packet;
	u_EventGen(packet, GE_ACTOR_ITEM_ACTIVATE, this->ID());
	packet.w_u8(0);
	packet.w_stringZ(item);
	packet.w_u8(activate ? 1 : 0);
	u_EventSend(packet, net_flags(true, true));
}

void CActor::SendActivateItemSlot(u8 type, u8 itemSlot, bool activate)
{
	NET_Packet packet;
	u_EventGen(packet, GE_ACTOR_ITEM_ACTIVATE, this->ID());
	packet.w_u8(type);
	packet.w_u8(itemSlot);
	packet.w_u8(activate ? 1 : 0);
	u_EventSend(packet, net_flags(true, true));
}

void CActor::SendSoundPlay(u32 ID, bool Activate)
{
	NET_Packet packet;
	u_EventGen(packet, GE_ACTOR_SND_ACTIVATE, this->ID());
	packet.w_u32(ID);
	packet.w_u8(Activate);
	u_EventSend(packet, net_flags(true, true));
}

// SOUNDS CODE 
void CActor::soundPlay()
{
	if (MidPlay || !InPlay || m_anims->m_script.m_rnd_snds[ANIM_CURRENT_PLAYED] == 0)
		return;

	u32 rnd = Random.randI(1, m_anims->m_script.m_rnd_snds[ANIM_CURRENT_PLAYED]);

	sSndID = rnd;

	if (!start_sel)
	{
		ref_sound snd_sel = m_anims->m_script.m_sound_Animation[ANIM_CURRENT_PLAYED][sSndID];
		if (!snd_sel._feedback())
		{
			snd_sel.play_at_pos(this, Position(), false, 0);
			selected = snd_sel;
			SendSoundPlay(sSndID, 1);
		}
		start_sel = true;
	}

	if (!selected._feedback())
	{
		selected.stop();
		start_sel = false;
	}
}

void CActor::SelectScriptAnimation(u32 mstate_rl)
{
	if (AnimExtraExit)
	{
		StopExit();
		AnimExtraExit = false;
		return;
	}

	if (!CanChange)
		return;

	if (oldAnim != ANIM_SELECTED)
	{
		if (InPlay && MidPlay && OutPlay)
		{
			oldAnim = ANIM_SELECTED;
			InputAnim = 0;
			OutAnim = 0;
			MidAnim = 0;
			InitItems = false;
		}

		Msg("OLD Animation: %d, IsNew: %d", oldAnim, ANIM_SELECTED);

		ANIM_CURRENT_PLAYED = oldAnim;;
		ANIM_CAN_WALK = m_anims->m_script.m_animation_can_walk[ANIM_CURRENT_PLAYED];

		NET_Packet packet;
		Game().u_EventGen(packet, GE_ACTOR_ANIMATION_SCRIPT, this->ID());
		packet.w_u8(ANIM_CURRENT_PLAYED);
		packet.w_u8(ANIM_CAN_WALK);
		packet.w_u8(CanChange);
		Game().u_EventSend(packet);
	}

	if (!ANIM_CAN_WALK)
		SelectScriptAnimation_Standing();
	else
		SelectScriptAnimation_Walking(mstate_rl);
}

void CActor::SelectScriptAnimation_Standing()
{
	u32 countIN = m_anims->m_script.in_anims.count[ANIM_CURRENT_PLAYED];

	MidPlay = false;
	OutPlay = false;
	InPlay = false;

	MotionID script_BODY;

	if (countIN == 0)
		InPlay = true;
	else
		if (InputAnim >= countIN)
			InPlay = true;
		else
			InPlay = false;


	if (!InitItems)
	{
		InitItems = true;
		StartAnimItems();
	}

	if (!InPlay)
	{
		script_BODY = m_anims->m_script.in_anims.m_animation_in[ANIM_CURRENT_PLAYED][InputAnim];
		script_anim(script_BODY, callbackAnim, this);

		InputAnim += 1;
	}

	if (!InPlay)
		return;


	u32 countMid = m_anims->m_script.middle_anims.count[ANIM_CURRENT_PLAYED];

	if (MidAnim >= countMid)
	{
		bool valid = ANIM_CURRENT_PLAYED != ANIM_SELECTED;
		if (ANIM_SELECTED == 0)
			MidPlay = true;
		else
			if (m_anims->m_script.m_animation_loop[ANIM_CURRENT_PLAYED] && !valid)
				MidAnim = 0;
			else
				MidPlay = true;
	}
	else
	{
		if (countMid == 0)
			MidPlay = true;
		else
			MidPlay = false;
	}

	if (!MidPlay)
	{
		script_BODY = m_anims->m_script.middle_anims.m_animation[ANIM_CURRENT_PLAYED][MidAnim];
		script_anim(script_BODY, callbackAnim, this);

		MidAnim += 1;
		soundPlay();
	}

	if (!MidPlay)
		return;

	u32 countOUT = m_anims->m_script.out_anims.count[ANIM_CURRENT_PLAYED];

	if (countOUT == 0)
		OutPlay = true;

	if (OutAnim >= countOUT)
		OutPlay = true;
	else
		OutPlay = false;

	if (!OutPlay)
	{
		script_BODY = m_anims->m_script.out_anims.m_animation_out[ANIM_CURRENT_PLAYED][OutAnim];
		script_anim(script_BODY, callbackAnim, this);

		OutAnim += 1;
	}

	if (OutPlay)
		StopExit();
}

void CActor::StartAnimItems()
{
	NEED_EXIT = true;
	if (m_anims->m_script.m_animation_attach[ANIM_CURRENT_PLAYED].size() > 0)
	{
		shared_str attach = m_anims->m_script.m_animation_attach[ANIM_CURRENT_PLAYED];
		CInventoryItem* inv_item = this->inventory().GetItemFromInventory(attach.c_str());
		CAttachableItem* item = smart_cast<CAttachableItem*>(inv_item);
		if (item)
			SendActivateItem(attach, true);
	}
	else
	{
		u16 slot = m_anims->m_script.m_animation_use_slot[ANIM_CURRENT_PLAYED];

		if (slot != this->inventory().GetActiveSlot())
		{
			if (slot != 0 && inventory().ItemFromSlot(slot))
				SendActivateItemSlot(1, (u8)slot, true);
			else
				SendActivateItemSlot(1, NO_ACTIVE_SLOT, true);
		}

	}
}

void CActor::StopExit()
{
	m_bAnimTorsoPlayed = false;
	m_current_torso.invalidate();
	m_current_legs.invalidate();


	if (selected._feedback())
		selected.stop();


	if (InitItems)
	{
		//if (AnimExtraExit && 
		//	(m_anims->m_script.m_animation_use_slot[ANIM_CURRENT_PLAYED] == INV_SLOT_2 ||
		//	m_anims->m_script.m_animation_use_slot[ANIM_CURRENT_PLAYED] == INV_SLOT_3 ))
		//		return;

		if (m_anims->m_script.m_animation_use_slot[ANIM_CURRENT_PLAYED] > 0)
		{
			u16 slot = m_anims->m_script.m_animation_use_slot[ANIM_CURRENT_PLAYED];
			PIItem item = this->inventory().ItemFromSlot(slot);
			CWeapon* wpn = smart_cast<CWeapon*>(item);

			if (item && !wpn)
			{
				SendActivateItemSlot(1, slot, false);
			}
			else
			{
				if (inventory().GetActiveSlot() != NO_ACTIVE_SLOT)
					SendActivateItemSlot(1, NO_ACTIVE_SLOT, true);
			}
		}
		else if (m_anims->m_script.m_animation_attach[ANIM_CURRENT_PLAYED].size() > 0)
		{
			shared_str attach = m_anims->m_script.m_animation_attach[ANIM_CURRENT_PLAYED];
			CInventoryItem* inv_item = this->inventory().GetItemFromInventory(attach.c_str());
			CAttachableItem* item = smart_cast<CAttachableItem*>(inv_item);

			if (item)
				SendActivateItem(attach, false);
		}

		InitItems = false;
	}

	NEED_EXIT = false;
	ANIM_SELECTED = 0;
	ANIM_CURRENT_PLAYED = 0;
	ANIM_CAN_WALK = false;

	oldAnim = 0;

	InPlay = true;
	MidPlay = true;
	OutPlay = true;
	CanChange = true;



	NET_Packet packet;
	Game().u_EventGen(packet, GE_ACTOR_ANIMATION_SCRIPT, this->ID());
	packet.w_u8(ANIM_CURRENT_PLAYED);
	packet.w_u8(ANIM_CAN_WALK);
	packet.w_u8(CanChange);
	Game().u_EventSend(packet);
}

