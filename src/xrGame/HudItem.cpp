#include "stdafx.h"
#include "HudItem.h"
#include "physic_item.h"
#include "actor.h"
#include "actoreffector.h"
#include "Missile.h"
#include "xrmessages.h"
#include "level.h"
#include "inventory.h"
#include "../xrEngine/CameraBase.h"
#include "player_hud.h"
#include "../xrEngine/SkeletonMotions.h"
#include "ui_base.h"
#include "HUDManager.h"
#include "Weapon.h"

#include "script_callback_ex.h"
#include "script_game_object.h"

ENGINE_API extern float psHUD_FOV_def;

CHudItem::CHudItem()
{
	RenderHud(TRUE);
	EnableHudInertion(TRUE);
	AllowHudInertion(TRUE);
	m_bStopAtEndAnimIsRunning = false;
	m_bSprintType = false;
	m_current_motion_def = NULL;
	m_started_rnd_anim_idx = u8(-1);
}

DLL_Pure* CHudItem::_construct()
{
	m_object = smart_cast<CPhysicItem*>(this);
	VERIFY(m_object);

	m_item = smart_cast<CInventoryItem*>(this);
	VERIFY(m_item);

	return				(m_object);
}

CHudItem::~CHudItem()
{
}

void CHudItem::Load(LPCSTR section)
{
	hud_sect = pSettings->r_string(section, "hud");
	item_sect = section;
	m_animation_slot = pSettings->r_u32(section, "animation_slot");

	m_sounds.LoadSound(section, "snd_bore", "sndBore", true);

	// HUD FOV
	m_nearwall_enabled = READ_IF_EXISTS(pSettings, r_bool, section, "nearwall_on", true);
	m_nearwall_target_hud_fov = READ_IF_EXISTS(pSettings, r_float, section, "nearwall_target_hud_fov", 0.27f);
	m_nearwall_dist_min = READ_IF_EXISTS(pSettings, r_float, section, "nearwall_dist_min", 0.5f);
	m_nearwall_dist_max = READ_IF_EXISTS(pSettings, r_float, section, "nearwall_dist_max", 1.f);
	m_nearwall_speed_mod = READ_IF_EXISTS(pSettings, r_float, section, "nearwall_speed_mod", 10.f);

	m_base_fov = READ_IF_EXISTS(pSettings, r_float, section, "hud_fov", psHUD_FOV_def);
	m_nearwall_last_hud_fov = m_base_fov;
}


void CHudItem::PlaySound(LPCSTR alias, const Fvector& position)
{
	m_sounds.PlaySound(alias, position, object().H_Root(), !!GetHUDmode());
}

void CHudItem::renderable_Render()
{
	UpdateXForm();
	BOOL _hud_render = ::Render->get_HUD() && GetHUDmode();

	if (_hud_render && !IsHidden())
	{
	}
	else
	{
		if (!object().H_Parent() || (!_hud_render && !IsHidden()))
		{
			on_renderable_Render();
			debug_draw_firedeps();
		}
		else
			if (object().H_Parent())
			{
				CInventoryOwner* owner = smart_cast<CInventoryOwner*>(object().H_Parent());
				VERIFY(owner);
				CInventoryItem* self = smart_cast<CInventoryItem*>(this);
				if (owner->attached(self) || item().BaseSlot() == INV_SLOT_3)
					on_renderable_Render();
			}
	}
}

void CHudItem::SwitchState(u32 S)
{

#ifndef USE_CLIENT_SIDE_WEAPONS
	if (OnClient())
		return;
#else 
	if (Level().CurrentControlEntity() == object().H_Parent())
		OnStateSwitch(S);
#endif 

	if (OnServer())
		SetNextState(S);

#ifndef USE_CLIENT_SIDE_WEAPONS
	if (object().Local() && !object().getDestroy())
#else 
	if (OnServer() || Level().CurrentControlEntity() == object().H_Parent())
	if (!object().getDestroy())
#endif 
 	{
		// !!! Just single entry for given state !!!
		NET_Packet				P;
		object().u_EventGen(P, GE_WPN_STATE_CHANGE, object().ID());
		P.w_u8(u8(S));
		object().u_EventSend(P);
	}
}

void CHudItem::OnEvent(NET_Packet& P, u16 type)
{
	switch (type)
	{
	case GE_WPN_STATE_CHANGE:
	{
		u8				S;
		P.r_u8(S);
		OnStateSwitch(u32(S));
	}
	break;
	}
}

void CHudItem::OnStateSwitch(u32 S)
{
	SetState(S);

#ifndef USE_CLIENT_SIDE_WEAPONS
	if (object().Remote())
		SetNextState(S);
#else 
	SetNextState(S);
#endif 

	if (S == eHidden)
		m_bSprintType = false;
	if (S != eIdle)
		m_bSprintType = false;

	switch (S)
	{
	case eBore:
	{
		SetPending(FALSE);

		PlayAnimBore();
		if (HudItemData())
		{
			Fvector P = HudItemData()->m_item_transform.c;
			m_sounds.PlaySound("sndBore", P, object().H_Root(), !!GetHUDmode(), false, m_started_rnd_anim_idx);
		}

	} break;
	case eSprintStart:
		PlayAnimSprintStart();
		break;
	case eSprintEnd:
		PlayAnimSprintEnd();
		break;
	}
	g_player_hud->updateMovementLayerState();
}

void CHudItem::OnAnimationEnd(u32 state)
{
	//	CActor* actor = smart_cast<CActor*>(object().H_Parent());
	//	if (actor)
	//	{
	//		actor->callback(GameObject::eActorHudAnimationEnd)(smart_cast<CGameObject*>(this)->lua_game_object(),
	//			this->hud_sect.c_str(), this->m_current_motion.c_str(), state,
	//			this->animation_slot());
	//	}

	switch (state)
	{
	case eBore:
	{
		SwitchState(eIdle);
	} break;
	case eSprintStart:
	{
		m_bSprintType = true;
		SwitchState(eIdle);
	} break;
	case eSprintEnd:
	{
		m_bSprintType = false;
		SwitchState(eIdle);
	} break;
	}
}

bool CHudItem::ActivateItem()
{
	OnActiveItem();
	return			true;
}

void CHudItem::DeactivateItem()
{
	OnHiddenItem();
}
void CHudItem::OnMoveToRuck(const SInvItemPlace& prev)
{
	SwitchState(eHidden);
}

void CHudItem::SendDeactivateItem()
{
	SendHiddenItem();
}
void CHudItem::SendHiddenItem()
{
	if (!object().getDestroy())
	{
		NET_Packet				P;
		object().u_EventGen(P, GE_WPN_STATE_CHANGE, object().ID());
		P.w_u8(u8(eHiding));
		object().u_EventSend(P, net_flags(TRUE, TRUE, FALSE, TRUE));
	}
}


void CHudItem::UpdateHudAdditional(Fmatrix& hud_trans)
{
}

void CHudItem::UpdateCL()
{
	if (m_current_motion_def)
	{
		if (m_bStopAtEndAnimIsRunning)
		{
			const xr_vector<motion_marks>& marks = m_current_motion_def->marks;
			if (!marks.empty())
			{
				float motion_prev_time = ((float)m_dwMotionCurrTm - (float)m_dwMotionStartTm) / 1000.0f;
				float motion_curr_time = ((float)Device.dwTimeGlobal - (float)m_dwMotionStartTm) / 1000.0f;

				xr_vector<motion_marks>::const_iterator it = marks.begin();
				xr_vector<motion_marks>::const_iterator it_e = marks.end();
				for (; it != it_e; ++it)
				{
					const motion_marks& M = (*it);
					if (M.is_empty())
						continue;

					const motion_marks::interval* Iprev = M.pick_mark(motion_prev_time);
					const motion_marks::interval* Icurr = M.pick_mark(motion_curr_time);
					if (Iprev == NULL && Icurr != NULL /* || M.is_mark_between(motion_prev_time, motion_curr_time)*/)
					{
						OnMotionMark(m_startedMotionState, M);
					}
				}

			}

			m_dwMotionCurrTm = Device.dwTimeGlobal;
			if (m_dwMotionCurrTm > m_dwMotionEndTm)
			{
				m_current_motion_def = NULL;
				m_dwMotionStartTm = 0;
				m_dwMotionEndTm = 0;
				m_dwMotionCurrTm = 0;
				m_bStopAtEndAnimIsRunning = false;
				OnAnimationEnd(m_startedMotionState);
			}
		}
	}
}

void CHudItem::OnH_A_Chield()
{
}

void CHudItem::OnH_B_Chield()
{
	StopCurrentAnimWithoutCallback();

	if (pSettings->line_exist(item_sect, "hud_fov"))
		m_nearwall_last_hud_fov = m_base_fov;
	else
		m_nearwall_last_hud_fov = psHUD_FOV_def;
}

void CHudItem::OnH_B_Independent(bool just_before_destroy)
{
	m_sounds.StopAllSounds();
	UpdateXForm();

	if (pSettings->line_exist(item_sect, "hud_fov"))
		m_nearwall_last_hud_fov = m_base_fov;
	else
		m_nearwall_last_hud_fov = psHUD_FOV_def;

	// next code was commented 
	/*
	if(HudItemData() && !just_before_destroy)
	{
		object().XFORM().set( HudItemData()->m_item_transform );
	}

	if (HudItemData())
	{
		g_player_hud->detach_item(this);
		Msg("---Detaching hud item [%s][%d]", this->HudSection().c_str(), this->object().ID());
	}*/
	//SetHudItemData			(NULL);
}

void CHudItem::OnH_A_Independent()
{
	if (HudItemData())
		g_player_hud->detach_item(this);
	StopCurrentAnimWithoutCallback();
}

void CHudItem::on_b_hud_detach()
{
	m_sounds.StopAllSounds();
}

void CHudItem::on_a_hud_attach()
{
	if (m_current_motion_def && m_current_motion.size())
	{
		PlayHUDMotion_noCB(m_current_motion, false);
#ifdef DEBUG
		//		Msg("continue playing [%s][%d]",m_current_motion.c_str(), Device.dwFrame);
#endif // #ifdef DEBUG
	}
}

u32 CHudItem::PlayHUDMotion(const shared_str& M, BOOL bMixIn, CHudItem* W, u32 state, float speed)
{
	u32 anim_time = PlayHUDMotion_noCB(M, bMixIn, speed);
	if (anim_time > 0)
	{
		m_bStopAtEndAnimIsRunning = true;
		m_dwMotionStartTm = Device.dwTimeGlobal;
		m_dwMotionCurrTm = m_dwMotionStartTm;
		m_dwMotionEndTm = m_dwMotionStartTm + anim_time;
		m_startedMotionState = state;
	}
	else
		m_bStopAtEndAnimIsRunning = false;

	return anim_time;
}

u32 CHudItem::PlayHUDMotionNew(const shared_str& M, const bool bMixIn, const u32 state, const bool randomAnim, float speed)
{
	//Msg("~~[%s] Playing motion [%s] for [%s]", __FUNCTION__, M.c_str(), HudSection().c_str());
	u32 anim_time = PlayHUDMotion_noCB(M, bMixIn, speed);
	if (anim_time > 0)
	{
		m_bStopAtEndAnimIsRunning = true;
		m_dwMotionStartTm = Device.dwTimeGlobal;
		m_dwMotionCurrTm = m_dwMotionStartTm;
		m_dwMotionEndTm = m_dwMotionStartTm + anim_time;
		m_startedMotionState = state;
	}
	else
		m_bStopAtEndAnimIsRunning = false;

	return anim_time;
}

//AVO: check if animation exists
bool CHudItem::isHUDAnimationExist(LPCSTR anim_name)
{
	if (HudItemData()) // First person
	{
		string256 anim_name_r;
		bool is_16x9 = UI().is_widescreen();
		u16 attach_place_idx = pSettings->r_u16(HudItemData()->m_sect_name, "attach_place_idx");
		xr_sprintf(anim_name_r, "%s%s", anim_name, (attach_place_idx == 1 && is_16x9) ? "_16x9" : "");
		player_hud_motion* anm = HudItemData()->m_hand_motions.find_motion(anim_name_r);
		if (anm)
			return true;
	}
	else // Third person
		if (g_player_hud->motion_length(anim_name, HudSection(), m_current_motion_def) > 100)
			return true;
#ifdef DEBUG
	Msg("~ [WARNING] [%s]: Animation [%s] does not exist in [%s]", __FUNCTION__, anim_name, HudSection().c_str());
#endif
	return false;
}

u32 CHudItem::PlayHUDMotionIfExists(std::initializer_list<const char*> Ms, const bool bMixIn, const u32 state, const bool randomAnim, float speed)
{
	for (const auto* M : Ms)
		if (isHUDAnimationExist(M))
			return PlayHUDMotionNew(M, bMixIn, state, randomAnim, speed);

	std::string dbg_anim_name;
	for (const auto* M : Ms)
	{
		dbg_anim_name += M;
		dbg_anim_name += ", ";
	}

#ifdef DEBUG
	Msg("~ [WARNING] [%s]: Motions [%s] not found for [%s]", __FUNCTION__, dbg_anim_name.c_str(), HudSection().c_str());
#endif

	return 0;
}

extern int bDebugHud;
u32 CHudItem::PlayHUDMotion_noCB(const shared_str& motion_name, const bool bMixIn, const bool randomAnim, float speed)
{
	m_current_motion = motion_name;

	if (bDebugHud && item().m_pInventory)
	{
		Msg("-[%s] as[%d] [%d] anim_play [%s][%d]",
			HudItemData() ? "HUD" : "Simulating",
			item().m_pInventory->GetActiveSlot(),
			item().object_id(),
			motion_name.c_str(),
			Device.dwFrame);
	}
	if (HudItemData())
	{
		return HudItemData()->anim_play(motion_name, bMixIn, m_current_motion_def, m_started_rnd_anim_idx, speed);
	}
	else
	{
		m_started_rnd_anim_idx = 0;
		return g_player_hud->motion_length(motion_name, HudSection(), m_current_motion_def);
	}
}

void CHudItem::StopCurrentAnimWithoutCallback()
{
	m_dwMotionStartTm = 0;
	m_dwMotionEndTm = 0;
	m_dwMotionCurrTm = 0;
	m_bStopAtEndAnimIsRunning = false;
	m_current_motion_def = NULL;
}


// PLAY HUD ANIM	*----------------------------------------------------------------------------------*

void CHudItem::PlayAnimBore()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_bore_jammed", "anm_bore" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_bore_empty", "anm_bore" }, true, GetState());
	else
		PlayHUDMotion("anm_bore", TRUE, this, GetState());
}


void CHudItem::PlayAnimIdle()
{
	if (TryPlayAnimIdle()) return;

	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_jammed", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_empty", "anm_idle" }, true, GetState());
	else
		PlayHUDMotion("anm_idle", TRUE, NULL, GetState());
}

bool CHudItem::TryPlayAnimIdle()
{
	if (MovingAnimAllowedNow())
	{
		CActor* pActor = smart_cast<CActor*>(object().H_Parent());
		if (pActor)
		{
			const u32 State = pActor->get_state();
			if (State & mcSprint)
			{
				if (!m_bSprintType)
				{
					SwitchState(eSprintStart);
					return true;
				}

				PlayAnimIdleSprint();
				return true;
			}
			else if (m_bSprintType)
			{
				if ((State & mcClimb))
					return false;

				SwitchState(eSprintEnd);
				return true;
			}
			else if (State & mcAnyMove)
			{
				if (!(State & mcCrouch))
				{
					if (State & mcAccel) //������ ��������� (SHIFT)
						PlayAnimIdleMovingSlow();
					else
						PlayAnimIdleMoving();
					return true;
				}
				else if (State & mcAccel) //������ � ������� (CTRL+SHIFT)
				{
					PlayAnimIdleMovingCrouchSlow();
					return true;
				}
				else
				{
					PlayAnimIdleMovingCrouch();
					return true;
				}
			}
		}
	}
	return false;
}

bool CHudItem::NeedBlendAnm()
{
	u32 state = GetState();
	return (state != eIdle && state != eHidden);
}

void CHudItem::PlayAnimIdleMoving()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_jammed", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving", "anm_idle" }, true, GetState());
}

void CHudItem::PlayAnimIdleMovingSlow()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_slow_jammed", "anm_idle_moving_slow", "anm_idle_moving", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_moving_slow_empty", "anm_idle_moving_slow", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving_slow", "anm_idle_moving", "anm_idle" }, true, GetState());
}

void CHudItem::PlayAnimIdleMovingCrouch()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_jammed", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_empty", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
}

void CHudItem::PlayAnimIdleMovingCrouchSlow()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_jammed", "anm_idle_moving_crouch_slow", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_empty", "anm_idle_moving_crouch_slow", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
}

void CHudItem::PlayAnimIdleSprint()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_sprint_jammed", "anm_idle_sprint", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_sprint", "anm_idle" }, true, GetState());
}

void CHudItem::PlayAnimSprintStart()
{
	CWeapon* wpn = smart_cast<CWeapon*>(this);

	string_path guns_sprint_start_anm{};
	strconcat(sizeof(guns_sprint_start_anm), guns_sprint_start_anm, "anm_idle_sprint_start", (wpn && wpn->IsGrenadeLauncherAttached()) ? (wpn && wpn->IsGrenadeMode() ? "_g" : "_w_gl") : "", (IsMisfireNow() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_sprint_start_anm))
		PlayHUDMotionNew(guns_sprint_start_anm, true, GetState());
	else if (strstr(guns_sprint_start_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_start_anm);
		new_guns_aim_anm[strlen(guns_sprint_start_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = true;
			SwitchState(eIdle);
		}
	}
	else if (strstr(guns_sprint_start_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_start_anm);
		new_guns_aim_anm[strlen(guns_sprint_start_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = true;
			SwitchState(eIdle);
		}
	}
	else
	{
		m_bSprintType = true;
		SwitchState(eIdle);
	}
}

void CHudItem::PlayAnimSprintEnd()
{
	CWeapon* wpn = smart_cast<CWeapon*>(this);

	string_path guns_sprint_end_anm{};
	strconcat(sizeof(guns_sprint_end_anm), guns_sprint_end_anm, "anm_idle_sprint_end", (wpn && wpn->IsGrenadeLauncherAttached()) ? (wpn && wpn->IsGrenadeMode() ? "_g" : "_w_gl") : "", (IsMisfireNow() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_sprint_end_anm))
		PlayHUDMotionNew(guns_sprint_end_anm, true, GetState());
	else if (strstr(guns_sprint_end_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_end_anm);
		new_guns_aim_anm[strlen(guns_sprint_end_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = false;
			SwitchState(eIdle);
		}
	}
	else if (strstr(guns_sprint_end_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_end_anm);
		new_guns_aim_anm[strlen(guns_sprint_end_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = false;
			SwitchState(eIdle);
		}
	}
	else
	{
		m_bSprintType = false;
		SwitchState(eIdle);
	}
}

void CHudItem::OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd)
{
	if (GetState() == eIdle && !m_bStopAtEndAnimIsRunning)
	{
		PlayAnimIdle();
		ResetSubStateTime();
	}
}

attachable_hud_item* CHudItem::HudItemData()
{
	if (!g_player_hud)
		return				nullptr;

	attachable_hud_item* hi = NULL;
	hi = g_player_hud->attached_item(0);
	if (hi && hi->m_parent_hud_item == this)
		return hi;

	hi = g_player_hud->attached_item(1);
	if (hi && hi->m_parent_hud_item == this)
		return hi;

	return nullptr;
}


BOOL CHudItem::GetHUDmode()
{
	if (object().H_Parent() && ParentIsActor())
	{
		CActor* A = smart_cast<CActor*>(object().H_Parent());
		return (A && A->HUDview() && HudItemData());
	}
	else
		return FALSE;
}

BOOL CHudItem::ParentIsActor()
{
	CObject* O = object().H_Parent();
	if (!O)
		return false;

	CEntityAlive* EA = smart_cast<CEntityAlive*>(O);
	if (!EA)
		return false;

	if (Level().CurrentControlEntity() == object().H_Parent())
		return !!EA->cast_actor();
	else
		return false;
}

void CHudItem::ReplaceHudSection(LPCSTR hud_section)
{
	if (hud_section != hud_sect)
		hud_sect = hud_section;
}

float CHudItem::GetHudFov()
{
	m_nearwall_enabled = false;
	if (m_nearwall_enabled && ParentIsActor() && Level().CurrentViewEntity() == object().H_Parent())
	{
		collide::rq_result& RQ = HUD().GetCurrentRayQuery();
		float dist = RQ.range;

		clamp(dist, m_nearwall_dist_min, m_nearwall_dist_max);
		float fDistanceMod = ((dist - m_nearwall_dist_min) / (m_nearwall_dist_max - m_nearwall_dist_min)); // 0.f ... 1.f

		float fBaseFov{ psHUD_FOV_def };

		if (pSettings->line_exist(item_sect, "hud_fov"))
			fBaseFov = m_base_fov;

		clamp(fBaseFov, 0.0f, FLT_MAX);

		float src = m_nearwall_speed_mod * Device.fTimeDelta;
		clamp(src, 0.f, 1.f);

		float fTrgFov = m_nearwall_target_hud_fov + fDistanceMod * (fBaseFov - m_nearwall_target_hud_fov);
		m_nearwall_last_hud_fov = m_nearwall_last_hud_fov * (1 - src) + fTrgFov * src;
	}

	return m_nearwall_last_hud_fov;
}
 