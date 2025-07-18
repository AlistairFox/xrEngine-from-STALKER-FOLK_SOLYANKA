#pragma once
#include "../BaseMonster/base_monster.h"
#include "../telekinesis.h"
#include "../anim_triple.h"
#include "../scanning_ability.h"
#include "../../../../xrServerEntities/script_export_space.h"

class CCharacterPhysicsSupport;
class CBurerFastGravi;

class CBurer :	public CBaseMonster,
				public CTelekinesis 
{

	typedef		CBaseMonster				inherited;

private:
	xr_vector<CObject*>	m_nearest;

public:
	static		bool	can_scan;

				u32		last_hit_frame;
				u32		time_last_scan;
	

	typedef		CTelekinesis				TTelekinesis;

	struct	GraviObject {
		bool		active;
		Fvector		cur_pos;
		Fvector		target_pos;
		Fvector		from_pos;

		u32			time_last_update;

		const CEntityAlive *enemy;
		
		GraviObject() {
			active = false;
			enemy = 0;
		}
		
		
		void		activate(const CEntityAlive *e, const Fvector &cp, const Fvector &tp) {
			active				= true;
			from_pos			= cp;
			cur_pos				= cp;
			target_pos			= tp;
			time_last_update	= Device.dwTimeGlobal;
			enemy				= e;
		}

		void		deactivate() {
			active = false;
		}

	} m_gravi_object;

	LPCSTR	 particle_gravi_wave;
	LPCSTR   particle_gravi_prepare;
	LPCSTR	 particle_tele_object;

	//////////////////////////////////////////////////////////////////////////
	// Sounds
	ref_sound	sound_gravi_wave;
	ref_sound	sound_scan;
	
	ref_sound	sound_tele_hold;
	ref_sound	sound_tele_throw;

	/*enum EBurerSounds {
		eAdditionalSounds			= MonsterSound::eMonsterSoundCustom,

		eMonsterSoundGraviAttack	= eAdditionalSounds | 0,
		eMonsterSoundTeleAttack		= eAdditionalSounds | 1,
	};	*/
	//////////////////////////////////////////////////////////////////////////

	struct gravi_params
	{
		float	speed;
		u32		cooldown;
		float	min_dist;
		float	max_dist;
		float	step;
		TTime	time_to_hold;
		float	radius;
		float	impulse_to_objects;
		float	impulse_to_enemy;
		float	hit_power;

	}		m_gravi;

	u32		m_tele_max_handled_objects;
	u32		m_tele_time_to_hold;
	u32		m_tele_max_time;
	float	m_tele_object_min_mass;
	float	m_tele_object_max_mass;
	float	m_tele_find_radius;
	float	m_tele_min_distance;
	float	m_tele_max_distance;
	float	m_tele_raise_speed;
	float	m_tele_fly_velocity;
	float	m_tele_object_height;

	float	m_weight_to_stamina_hit;
	float	m_weapon_drop_stamina_k;
	float	m_runaway_distance;
	float	m_normal_distance;
	TTime	m_max_runaway_time;
	
	float	m_weapon_drop_velocity;

	TTime	m_shield_cooldown;
	TTime	m_shield_time;
	bool	m_shield_active;
	LPCSTR	m_shield_keep_particle;
	TTime	m_shield_keep_particle_period;
	LPCSTR	particle_fire_shield;

	CBurerFastGravi	*m_fast_gravi;

public:
					CBurer				();
	virtual			~CBurer				();	


	virtual void	reinit				();
	virtual void	reload				(LPCSTR section);

	virtual void	Load				(LPCSTR section);
	virtual void	PostLoad			(LPCSTR section);

	virtual void	net_Destroy			();
	virtual void	net_Relcase			(CObject *O);
	virtual	void	shedule_Update		(u32 dt);
	virtual void	UpdateCL			();
	virtual	void	Hit					(SHit* pHDS);
	virtual void	Die					(CObject* who);
			void	ProcessTurn			();
	virtual void	CheckSpecParams		(u32 spec_params);

	virtual void	OnEvent				(NET_Packet& P, u16 type);
	void			StartGraviMP		();
	void			shieldParticlesMP	();

			void	UpdateGraviObject	();

			void	UpdateGraviObjectCL	();

			void	StartGraviPrepare	();
			void	StopGraviPrepare	();

			void	StartTeleObjectParticle(CGameObject *pO);
			void	StopTeleObjectParticle(CGameObject *pO);

			void	ActivateShield		();
			void	DeactivateShield	();

			bool	need_shotmark () const { return !m_shield_active; }

	virtual bool	ability_distant_feel() {return true;}
	virtual	char*	get_monster_class_name () { return "burer"; }

#ifdef DEBUG
	virtual CBaseMonster::SDebugInfo show_debug_info();
#endif

			void			set_force_gravi_attack (bool force_gravi) { m_force_gravi_attack = force_gravi; }
			bool			get_force_gravi_attack () const { return m_force_gravi_attack; }


private:
			bool			m_force_gravi_attack;

	void xr_stdcall			StaminaHit			();

	DECLARE_SCRIPT_REGISTER_FUNCTION

			void	face_enemy					();
};

bool   actor_is_reloading_weapon ();

add_to_type_list(CBurer)
#undef script_type_list
#define script_type_list save_type_list(CBurer)
