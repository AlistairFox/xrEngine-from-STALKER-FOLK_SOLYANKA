#include "stdafx.h"
#include "Actor.h"
#include "Inventory.h"
#include "actor_anim_defs.h"
#include "../Include/xrRender/Kinematics.h"
#include "Weapon.h"

bool CActor::MpAnimationMode() const
{ 
	if (Level().CurrentControlEntity() == this)
	{
		if (!CanChange)
			return true;

		if (!InPlay || !MidPlay || !OutPlay)
			return true;

		return ANIM_SELECTED > 0;
	}
	else
		return !CanChange;
}

bool CActor::Setuped_callbacks()
{
	//luabind::functor<bool>	funct;
	//R_ASSERT(ai().script_engine().functor("mp_bind_actor.callback_setuped", funct));
	//bool ret = funct();

	//return ret;
	return true;
}

int SlotStop = -1; 

void SActorStateAnimation::CreateAnimationsScripted(IKinematicsAnimated* K)
{
	string_path filepath;
	FS.update_path(filepath, "$game_config$", "actor_anims.ltx");
	CInifile* file = xr_new<CInifile>(filepath, true, true);

	if (file && file->section_exist("animations"))
	{
		u32 count = file->r_u32("animations", "count");
 
		for (int i = 0; i < count; i++)
		{
			string32 tmp = { 0 };
			itoa(i, tmp, 10);
			string32 animation = { 0 };
			xr_strcat(animation, "anim_");
			xr_strcat(animation, tmp);

			if (file->section_exist(animation))
			{
				bool anim_loop = file->r_bool(animation, "anim_loop");
				if (file->line_exist(animation, "anim_snd"))
				{
					shared_str snd = file->r_string(animation, "anim_snd");
					u32 snds = 0;
					if (file->line_exist(animation, "anim_snd_rnd"))
						snds = file->r_u32(animation, "anim_snd_rnd");
					else
						snds = 1;

					for (int snd_i = 1; snd_i <= snds; snd_i++)
					{
						string32 tmp = { 0 };
						itoa(snd_i, tmp, 10);
						string128 path_snd = { 0 };
						xr_strcat(path_snd, snd.c_str());
						xr_strcat(path_snd, tmp);
						m_sound_Animation[i][snd_i].create(path_snd, st_Effect, 0);
					}
					m_rnd_snds[i] = snds;
				}

				m_animation_loop[i] = anim_loop;
 
				if (file->line_exist(animation, "anim_use_slot"))
					m_animation_use_slot[i] = file->r_u32(animation, "anim_use_slot");


				LPCSTR anims_in = file->r_string(animation, "anim_in");
				LPCSTR anims_out = file->r_string(animation, "anim_out");
				LPCSTR anims_middle = file->r_string(animation, "anim_middle");
				u32 countIN = _GetItemCount(anims_in, ',');
				u32 countOUT = _GetItemCount(anims_out, ',');
				u32 countMID = _GetItemCount(anims_middle, ',');

				in_anims.count[i] = countIN;
				out_anims.count[i] = countOUT;
				middle_anims.count[i] = countMID;

				for (int id = 0; id != countIN; id++)
				{
					string64 anim = { 0 };
					_GetItem(anims_in, id, anim, ',');
					MotionID motionAnim = K->ID_Cycle_Safe(anim);
					in_anims.m_animation_in[i][id] = motionAnim;
				}

				for (int id = 0; id != countOUT; id++)
				{
					string64 anim = { 0 };
					_GetItem(anims_out, id, anim, ',');
					MotionID motionAnim = K->ID_Cycle_Safe(anim);
					out_anims.m_animation_out[i][id] = motionAnim;
				}

				for (int id = 0; id != countMID; id++)
				{
					string64 anim = { 0 };
					_GetItem(anims_middle, id, anim, ',');
					MotionID motionAnim = K->ID_Cycle_Safe(anim);
					middle_anims.m_animation[i][id] = motionAnim;
				}
			}
		}
	}
}

void CActor::script_anim(MotionID Animation, PlayCallback Callback, LPVOID CallbackParam)
{
	IKinematicsAnimated* k = smart_cast<IKinematicsAnimated*>(Visual());
	
	CBlend* b = k->LL_PlayCycle(
		k->LL_GetMotionDef(Animation)->bone_or_part,
		Animation,
		TRUE,
		k->LL_GetMotionDef(Animation)->Accrue(),
		k->LL_GetMotionDef(Animation)->Falloff(),
		k->LL_GetMotionDef(Animation)->Speed(),
		//true,
		k->LL_GetMotionDef(Animation)->StopAtEnd(),
		Callback, CallbackParam, 0
	);

	if (b)
		Msg("BlendTime: %f", b->timeTotal);


	CanChange = false;
	SendAnimationToServer(Animation);

}

void callbackAnim(CBlend* blend)
{
	CActor* act = (CActor*)blend->CallbackParam;
	if (act)
		act->CanChange = true;
}

void CActor::ReciveAnimationPacket(NET_Packet& packet)
{
	MotionID motion;
	packet.r(&motion, sizeof(motion));
	IKinematicsAnimated* k = smart_cast<IKinematicsAnimated*>(Visual());

	if (motion.valid() && k)
	{
		CBlend* blend =  k->LL_PlayCycle(
			k->LL_GetMotionDef(motion)->bone_or_part,
			motion,
			TRUE,
			k->LL_GetMotionDef(motion)->Accrue(),
			k->LL_GetMotionDef(motion)->Falloff(),
			k->LL_GetMotionDef(motion)->Speed(),
			//true, // Всегда стопить иначе анимация зависнет без переключения (ток снимать костюм)
			k->LL_GetMotionDef(motion)->StopAtEnd(),
			callbackAnim, this, 0
		);

		if (blend)
			Msg("time_end: %f", blend->timeTotal);


		CanChange = false;
	}

}

void CActor::ReciveActivateItem(NET_Packet& packet)
{
	u16 slot;
	packet.r_u16(slot);
	bool activate = packet.r_u8();
 
	PIItem item = this->inventory().ItemFromSlot(slot);
 
	if (item)
	{
		item->enable(activate);
		
		if (activate)
			this->attach(item, false);
		else 
			this->detach(item);
	}
}

void CActor::ReciveSoundPlay(NET_Packet packet)
{
	u32 snd_id, sAnim;
	bool activate = packet.r_u8();

	if (activate)
	{
		packet.r_u32(sAnim);
		packet.r_u32(snd_id);

		if (selected._p)
			if (!selected._feedback())
			{
				selected.stop();
			}

		ref_sound snd_sel = m_anims->m_script.m_sound_Animation[sAnim][snd_id];

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

void CActor::SendAnimationToServer(MotionID motion)
{
	NET_Packet packet;
	u_EventGen(packet, GE_ACTOR_ANIMATIONS_EVENT, this->ID());
	packet.w_u8(1);
	packet.w(&motion, sizeof(motion));
	u_EventSend(packet, net_flags(true, true));
}

void CActor::SendActivateItem(u16 slot, bool activate)
{
	NET_Packet packet;
	u_EventGen(packet, GE_ACTOR_ANIMATIONS_EVENT, this->ID());
	packet.w_u8(2);
	packet.w_u16(slot);
	packet.w_u8(activate ? 1 : 0);
	u_EventSend(packet, net_flags(true, true));
}

void CActor::SendSoundPlay(u32 ID, bool Activate)
{
	NET_Packet packet;
	u_EventGen(packet, GE_ACTOR_ANIMATIONS_EVENT, this->ID());
	packet.w_u8(3);
	packet.w_u8(Activate);
	
	if (Activate)
	{
		packet.w_u32(oldAnim);
		packet.w_u32(ID);
	}


	u_EventSend(packet, net_flags(true, true));
}

void CActor::soundPlay()
{
	if (MidPlay || !InPlay || m_anims->m_script.m_rnd_snds[oldAnim] == 0)
		return;

	sSndID = Random.randI(1, m_anims->m_script.m_rnd_snds[oldAnim]);

	if (!started_music_hand_item)
	{
		ref_sound snd_sel = m_anims->m_script.m_sound_Animation[oldAnim][sSndID];
		if (!snd_sel._feedback())
		{
			snd_sel.play_at_pos(this, Position(), false, 0);
			selected = snd_sel;
			SendSoundPlay(sSndID, 1);
		}
		started_music_hand_item = true;
	}

	if (!selected._feedback())
	{
		selected.stop();
		started_music_hand_item = false;
	}
}

void CActor::SelectScriptAnimation()
{
 
	if (!CanChange)
		return;

	Msg("InAnim[%d]", IntAnim);
	Msg("MidAnim[%d]", MidAnim);
	Msg("OutAnim[%d]", OutAnim);
  
	if (oldAnim != ANIM_SELECTED)
	{
		if (InPlay && MidPlay && OutPlay)
		{
			oldAnim = ANIM_SELECTED;
			IntAnim = 0;
			OutAnim = 0;
			MidAnim = 0;
		}
	}

	u32 selectedAnimation = oldAnim;
	MotionID script_BODY;

	u32 countIN = m_anims->m_script.in_anims.count[selectedAnimation];

	MidPlay = false;
	OutPlay = false;
	InPlay = false;
	 
	if (IntAnim >= countIN || countIN == 0)
		InPlay = true;
	else
		InPlay = false;

	if (!InPlay)
	{
		script_BODY = m_anims->m_script.in_anims.m_animation_in[selectedAnimation][IntAnim];
		script_anim(script_BODY, callbackAnim, this);
		IntAnim += 1;

		bool weapon = false;

		if (m_anims->m_script.m_animation_use_slot[selectedAnimation] > 0)
		{
			u16 slot = m_anims->m_script.m_animation_use_slot[selectedAnimation];
			PIItem item =  this->inventory().ItemFromSlot(slot);
			CWeapon* wpn = smart_cast<CWeapon*>(item);

			if (item && !wpn)
			{
				item->enable(true);
				this->attach(item, false);
				SendActivateItem(slot, true);
			}
			else
			{
				if (wpn)
				{
					weapon = true;
					NET_Packet packet;
					u_EventGen(packet, GE_ACTOR_ANIMATIONS_EVENT, this->ID());
					packet.w_u8(4);
					packet.w_u16(slot);
					u_EventSend(packet, net_flags(true, true));
				}
			}
		}

		if (!weapon)
		{
			NET_Packet packet;
			u_EventGen(packet, GE_ACTOR_ANIMATIONS_EVENT, this->ID());
			packet.w_u8(4);
			packet.w_u16(0);
			u_EventSend(packet, net_flags(true, true));
		}

		return;
	}

	u32 countMid = m_anims->m_script.middle_anims.count[selectedAnimation];

	if (MidAnim >= countMid && 
		m_anims->m_script.m_animation_loop[selectedAnimation] &&
		selectedAnimation == ANIM_SELECTED)
		MidAnim = 0;	 
 
	if (countMid == 0)
		MidPlay = true;

	if (ANIM_SELECTED == 0)
		MidPlay = true;

	if (!MidPlay)
	{
		script_BODY = m_anims->m_script.middle_anims.m_animation[selectedAnimation][MidAnim];
		script_anim(script_BODY, callbackAnim, this);
		MidAnim += 1;

		soundPlay();
	}

	if (!MidPlay)
		return;

	u32 countOUT = m_anims->m_script.out_anims.count[selectedAnimation];

	if (OutAnim >= countOUT || countOUT == 0)
		OutPlay = true;
	else
		OutPlay = false;

	if (!OutPlay)
	{
		script_BODY = m_anims->m_script.out_anims.m_animation_out[selectedAnimation][OutAnim];
		script_anim(script_BODY, callbackAnim, this);
		OutAnim += 1;
	}

	if (OutPlay)
		EndAnimation(selectedAnimation);
}

void CActor::EndAnimation(int selectedAnimation)
{
	if (selected._p)
	{
		SendSoundPlay(0, false);
		selected.stop();
	}

	if (m_anims->m_script.m_animation_use_slot[selectedAnimation] > 0)
	{
		u16 slot = m_anims->m_script.m_animation_use_slot[selectedAnimation];
		PIItem item = this->inventory().ItemFromSlot(slot);
		CWeapon* wpn = smart_cast<CWeapon*>(item);
		if (item && !wpn)
		{
			item->enable(false);
			this->detach(item);
			SendActivateItem(slot, false);
		}
	}
}

void CActor::StopAllSNDs()
{
	IntAnim = 0;
	OutAnim = 0;
	MidAnim = 0;

	InPlay = true;
	MidAnim = true;
	OutPlay = true;

	CanChange = true;

	EndAnimation(oldAnim);

	oldAnim = ANIM_SELECTED;

}


 
