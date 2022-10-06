#pragma once

#include "../Include/xrRender/KinematicsAnimated.h"

struct SAnimState
{
	MotionID	legs_fwd;
	MotionID    legs_fwd_safe;

	MotionID	legs_back;
	MotionID    legs_back_safe;

	MotionID	legs_ls;
	MotionID    legs_ls_safe;

	MotionID	legs_rs;
	MotionID    legs_rs_safe;

	void		Create								(IKinematicsAnimated* K, LPCSTR base0, LPCSTR base1);
};

struct STorsoWpn
{
	enum eMovingState{eIdle, eWalk, eRun, eSprint, eIdleSafe, eWalkSafe, eRunSafe, eSprintSafe, eTotal};
	MotionID	moving[eTotal];

	MotionID	zoom;
	MotionID	holster;
	MotionID	draw;
	MotionID	drop;
	MotionID	reload;
	MotionID	reload_1;
	MotionID	reload_2;
	MotionID	attack;
	MotionID	attack_zoom;
	MotionID	fire_idle;
	MotionID	fire_end;

	//анимации для атаки для всего тела (когда мы стоим на месте)
	MotionID	all_attack_0;
	MotionID	all_attack_1;
	MotionID	all_attack_2;

	//For Detector DRAWS AND HOLSTER
	MotionID	draw_all;
 	MotionID	draw_detector;

	MotionID	holster_all;
	MotionID	holster_detector;


	void		Create								(IKinematicsAnimated* K, LPCSTR base0, LPCSTR base1);

	void		CreateDetectorPistol(IKinematicsAnimated* K);
	void		CreateDetectorKnife(IKinematicsAnimated* K);
	void		CreateDetectorBolt(IKinematicsAnimated* K);
	void		CreateDetectorEMPTY(IKinematicsAnimated* K);

};

#define _total_anim_slots_ 32

struct SActorState
{

	MotionID		legs_idle;
	MotionID		legs_idle_safe;
	MotionID		jump_begin;
	MotionID		jump_idle;

	MotionID		landing[2];
	MotionID		legs_turn;
	MotionID		legs_turn_safe;
	MotionID		death;
	SAnimState		m_walk;
	SAnimState		m_run;
	STorsoWpn		m_torso[_total_anim_slots_];
	MotionID		m_torso_idle;
	MotionID		m_head_idle;
	MotionID		m_head_idle_safe;

	MotionID		m_damage[DAMAGE_FX_COUNT];
	void			Create							(IKinematicsAnimated* K, LPCSTR base);
	void			CreateClimb						(IKinematicsAnimated* K);
};

struct SActorSprintState 
{
	//leg anims
	MotionID		legs_fwd;
	MotionID		legs_fwd_safe;

	MotionID		legs_ls;
	MotionID		legs_ls_safe;
	
	MotionID		legs_rs;
	MotionID		legs_rs_safe;
	
	MotionID		legs_jump_fwd;
	MotionID		legs_jump_ls;
	MotionID		legs_jump_rs;

	void Create		(IKinematicsAnimated* K);
};


//vehicle anims
struct	SVehicleAnimCollection
{
	static const u16 MAX_IDLES = 3;
	u16				idles_num;
	MotionID		idles[MAX_IDLES];
	MotionID		steer_left;
	MotionID		steer_right;
					SVehicleAnimCollection	();
	void			Create				(IKinematicsAnimated* K,u16 num);
};

struct SActorVehicleAnims
{
	static const int TYPES_NUMBER=2;
	SVehicleAnimCollection m_vehicles_type_collections	[TYPES_NUMBER];
						SActorVehicleAnims				();
	void				Create							(IKinematicsAnimated* K);
};

#define MAX_ANIMS 256
#define MAX_ANIMS_CYCLE 32
#define MAX_SOUNDS 32

struct SScript_AnimInput
{
	MotionID m_animation_in[MAX_ANIMS][MAX_ANIMS_CYCLE];
	u32 count[MAX_ANIMS];
};

struct SScript_AnimOut
{
	MotionID m_animation_out[MAX_ANIMS][MAX_ANIMS_CYCLE];
	u32 count[MAX_ANIMS];
};

struct SScript_AnimMiddle
{
	MotionID m_animation[MAX_ANIMS][MAX_ANIMS_CYCLE];
	u32 count[MAX_ANIMS];
};

struct SActorStateAnimation
{
	SScript_AnimInput  in_anims;
	SScript_AnimOut    out_anims;
	SScript_AnimMiddle middle_anims;

	bool m_animation_loop[MAX_ANIMS];
	u32	 m_rnd_snds[MAX_ANIMS];
	ref_sound m_sound_Animation[MAX_ANIMS][MAX_SOUNDS];
 	u16 m_animation_use_slot[MAX_ANIMS];

	void CreateAnimationsScripted(IKinematicsAnimated* K);
};

struct SActorMotions
{
	MotionID			 m_dead_stop;
	SActorState			 m_normal;
	SActorState			 m_crouch;
	SActorState			 m_climb;
	SActorSprintState	 m_sprint;

	SActorStateAnimation m_script;

	STorsoWpn			 m_detector;		 //0
	STorsoWpn			 m_detector_knife;	 //1
	STorsoWpn			 m_detector_pistol;	 //2-3 (WNP PISTOL)
	STorsoWpn			 m_detector_bolt;    //6

	void				Create(IKinematicsAnimated* K);
 
};
