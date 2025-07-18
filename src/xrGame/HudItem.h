#pragma once

class CSE_Abstract;
class CPhysicItem;
class NET_Packet;
class CInventoryItem;
class CMotionDef;

#include "actor_defs.h"
#include "inventory_space.h"
#include "hudsound.h"

struct attachable_hud_item;
class motion_marks;

class CHUDState
{
public:
	enum EHudStates {
		eIdle = 0,
		eShowing,
		eHiding,
		eHidden,
		eBore,
		eSprintStart,
		eSprintEnd,
		eLastBaseState = eSprintEnd,
	};

private:
	u32						m_hud_item_state;
	u32						m_nextState;
	u32						m_dw_curr_state_time;
protected:
	u32						m_dw_curr_substate_time;
public:
	CHUDState() { SetState(eHidden); }
	IC		u32				GetNextState() const { return		m_nextState; }
	IC		u32				GetState() const { return		m_hud_item_state; }

	IC		void			SetState(u32 v) { m_hud_item_state = v; m_dw_curr_state_time = Device.dwTimeGlobal; ResetSubStateTime(); }
	IC		void			SetNextState(u32 v) { m_nextState = v; }
	IC		u32				CurrStateTime() const { return Device.dwTimeGlobal - m_dw_curr_state_time; }
	IC		void			ResetSubStateTime() { m_dw_curr_substate_time = Device.dwTimeGlobal; }
	virtual void			SwitchState(u32 S) = 0;
	virtual void			OnStateSwitch(u32 S) = 0;
};

class CHudItem :public CHUDState
{
protected:
	CHudItem();
	virtual					~CHudItem();
	virtual DLL_Pure* _construct();

	Flags16					m_huditem_flags;
	enum {
		fl_pending = (1 << 0),
		fl_renderhud = (1 << 1),
		fl_inertion_enable = (1 << 2),
		fl_inertion_allow = (1 << 3),
	};

	struct {
		const CMotionDef* m_current_motion_def;
		shared_str				m_current_motion;
		u32						m_dwMotionCurrTm;
		u32						m_dwMotionStartTm;
		u32						m_dwMotionEndTm;
		u32						m_startedMotionState;
		u8						m_started_rnd_anim_idx;
		bool					m_bStopAtEndAnimIsRunning;
	};
public:
	virtual void				Load(LPCSTR section);
	virtual	BOOL				net_Spawn(CSE_Abstract* DC) { return TRUE; };
	virtual void				net_Destroy() {};
	virtual void				OnEvent(NET_Packet& P, u16 type);

	virtual void				OnH_A_Chield();
	virtual void				OnH_B_Chield();
	virtual void				OnH_B_Independent(bool just_before_destroy);
	virtual void				OnH_A_Independent();

	virtual void				PlaySound(LPCSTR alias, const Fvector& position);
	void				StopAllSounds() { m_sounds.StopAllSounds(); }

	virtual bool				Action(u16 cmd, u32 flags) { return false; }
	void				OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd);

	virtual	u8					GetCurrentHudOffsetIdx() { return 0; }

	BOOL						GetHUDmode();
	IC BOOL						IsPending()		const { return !!m_huditem_flags.test(fl_pending); }

	virtual bool				ActivateItem();
	virtual void				DeactivateItem();
	virtual void				SendDeactivateItem();
	virtual void				OnActiveItem() {};
	virtual void				OnHiddenItem() {};
	virtual void				SendHiddenItem();			//same as OnHiddenItem but for client... (sends message to a server)...
	virtual void				OnMoveToRuck(const SInvItemPlace& prev);

	bool						IsHidden()	const { return GetState() == eHidden; }						// Does weapon is in hidden state
	bool						IsHiding()	const { return GetState() == eHiding; }
	bool						IsShowing()	const { return GetState() == eShowing; }

	virtual void				SwitchState(u32 S);
	virtual void				OnStateSwitch(u32 S);

	virtual void				OnAnimationEnd(u32 state);
	virtual void				OnMotionMark(u32 state, const motion_marks&) {};

	virtual void				PlayAnimIdle();
	virtual void				PlayAnimBore();
	bool						TryPlayAnimIdle();
	virtual bool				MovingAnimAllowedNow() { return true; }
	virtual bool				IsMisfireNow() { return false; }
	virtual bool				IsMagazineEmpty() { return false; }
	virtual bool				IsGrenadeMode() const { return false; }
	virtual bool				NeedBlendAnm();

	virtual void				PlayAnimIdleMoving();
	virtual void				PlayAnimIdleMovingSlow();
	virtual void				PlayAnimIdleMovingCrouch();
	virtual void				PlayAnimIdleMovingCrouchSlow();
	virtual void				PlayAnimIdleSprint();
	virtual void				PlayAnimSprintStart();
	virtual void				PlayAnimSprintEnd();

	virtual void				UpdateCL();
	virtual void				renderable_Render();


	virtual void				UpdateHudAdditional(Fmatrix&);


	virtual	void				UpdateXForm() = 0;

	u32							PlayHUDMotion(const shared_str& M, BOOL bMixIn, CHudItem* W, u32 state, float speed = 1.f);
	u32							PlayHUDMotionNew(const shared_str& M, const bool bMixIn, const u32 state, const bool randomAnim = true, float speed = 1.f);
	u32							PlayHUDMotionIfExists(std::initializer_list<const char*>, const bool bMixIn, const u32 state, const bool randomAnim = true, float speed = 1.f);
	u32							PlayHUDMotion_noCB(const shared_str& M, const bool bMixIn, const bool randomAnim = true, float speed = 1.f);
	bool						isHUDAnimationExist(LPCSTR anim_name);
	void						StopCurrentAnimWithoutCallback();
	virtual void				UpdateAddonsTransform(bool for_hud) {};

	IC void						RenderHud(BOOL B) { m_huditem_flags.set(fl_renderhud, B); }
	IC BOOL						RenderHud() { return m_huditem_flags.test(fl_renderhud); }
	attachable_hud_item* HudItemData();
	virtual void				on_a_hud_attach();
	virtual void				on_b_hud_detach();
	IC BOOL						HudInertionEnabled()	const { return m_huditem_flags.test(fl_inertion_enable); }
	IC BOOL						HudInertionAllowed()	const { return m_huditem_flags.test(fl_inertion_allow); }
	virtual float				GetInertionFactor() { return 1.f; }; //--#SM+#--
	virtual float				GetInertionPowerFactor() { return 1.f; }; //--#SM+#--
	virtual void				render_hud_mode() {};
	virtual bool				need_renderable() { return true; };
	virtual void				render_item_3d_ui() {}
	virtual bool				render_item_3d_ui_query() { return false; }

	virtual bool				CheckCompatibility(CHudItem*) { return true; }
	virtual BOOL				ParentIsActor();
	void				ReplaceHudSection(LPCSTR hud_section);
protected:

	IC		void				SetPending(BOOL H) { m_huditem_flags.set(fl_pending, H); }
	shared_str					hud_sect;
	shared_str					item_sect;

	//����� ������� ��������� XFORM � FirePos
	u32							dwFP_Frame;
	u32							dwXF_Frame;

	IC void						EnableHudInertion(BOOL B) { m_huditem_flags.set(fl_inertion_enable, B); }
	IC void						AllowHudInertion(BOOL B) { m_huditem_flags.set(fl_inertion_allow, B); }

	u32							m_animation_slot;

	HUD_SOUND_COLLECTION_LAYERED m_sounds;

private:
	CPhysicItem* m_object;
	CInventoryItem* m_item;

public:
	const shared_str& HudSection() const { return hud_sect; }
	IC CPhysicItem& object() const { VERIFY(m_object); return(*m_object); }
	IC CInventoryItem& item() const { VERIFY(m_item); return(*m_item); }
	IC		u32					animation_slot() { return m_animation_slot; }

	virtual void				on_renderable_Render() = 0;
	virtual void				debug_draw_firedeps() {};

	virtual CHudItem* cast_hud_item() { return this; }

	float						GetHudFov();

	bool  m_nearwall_enabled;
	bool  m_bSprintType;
	float m_hud_fov_add_mod;
	float m_nearwall_last_hud_fov;
	float m_nearwall_dist_max = 0.f;
	float m_nearwall_dist_min = 0.f;
	float m_nearwall_target_hud_fov = 0.f;
	float m_nearwall_speed_mod = 0.f;
	float m_base_fov = 0.f;
};

