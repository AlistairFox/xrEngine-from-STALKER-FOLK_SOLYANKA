#include "stdafx.h"
#include "Actor.h"
#include "ActorAnimation.h"
#include "actor_anim_defs.h"
#include "Inventory.h"	

#include "HudItem.h"
#include "CustomDetector.h"
#include "WeaponPistol.h"
#include "Bolt.h"
#include "WeaponKnife.h"
 
void STorsoWpn::CreateDetectorEMPTY(IKinematicsAnimated* K)
{
	moving[eIdle] = K->ID_Cycle_Safe("norm_torso_0+detector_aim_1");
	moving[eWalk] = K->ID_Cycle_Safe("norm_torso_0+detector_aim_2");
	moving[eRun] = K->ID_Cycle_Safe("norm_torso_0+detector_aim_3");
	moving[eSprint] = K->ID_Cycle_Safe("norm_torso_0+detector_escape_0");

	moving[eIdleSafe] = K->ID_Cycle_Safe("norm_torso_0+detector_idle_1");
	moving[eWalkSafe] = K->ID_Cycle_Safe("norm_torso_0+detector_walk_1");
	moving[eRunSafe] = K->ID_Cycle_Safe("norm_torso_0+detector_run_1");
	moving[eSprintSafe] = K->ID_Cycle_Safe("norm_torso_0+detector_escape_0");


	zoom = K->ID_Cycle_Safe("norm_torso_0+detector_aim_0");
	holster = K->ID_Cycle_Safe("norm_torso_0+detector_holsterdevice_0");
	draw = K->ID_Cycle_Safe("norm_torso_0+detector_drawdevice_0");
	 
}

void STorsoWpn::CreateDetectorKnife(IKinematicsAnimated* K)
{
	moving[eIdle] = K->ID_Cycle_Safe("norm_torso_knife+detector_aim_1");
	moving[eWalk] = K->ID_Cycle_Safe("norm_torso_knife+detector_aim_2");
	moving[eRun] = K->ID_Cycle_Safe("norm_torso_knife+detector_aim_3");
	moving[eSprint] = K->ID_Cycle_Safe("norm_torso_knife+detector_escape_0");

	moving[eIdleSafe] = K->ID_Cycle_Safe("norm_torso_knife+detector_idle_1");
	moving[eWalkSafe] = K->ID_Cycle_Safe("norm_torso_knife+detector_walk_1");
	moving[eRunSafe] = K->ID_Cycle_Safe("norm_torso_knife+detector_run_1");
	moving[eSprintSafe] = K->ID_Cycle_Safe("norm_torso_knife+detector_escape_0");

	zoom = K->ID_Cycle_Safe("norm_torso_knife+detector_aim_0");

	holster = K->ID_Cycle_Safe("norm_torso_knife+detector_holster_0");
	holster_all = K->ID_Cycle_Safe("norm_torso_knife+detector_holsterall_0");
	holster_detector = K->ID_Cycle_Safe("norm_torso_knife+detector_holsterdevice_0");

	draw = K->ID_Cycle_Safe("norm_torso_knife+detector_draw_0");
	draw_all = K->ID_Cycle_Safe("norm_torso_knife+detector_drawall_0");
	draw_detector = K->ID_Cycle_Safe("norm_torso_knife+detector_drawdevice_0");

	reload = K->ID_Cycle_Safe("norm_torso_1_reload_0");

	attack_zoom = K->ID_Cycle_Safe("norm_torso_knife+detector_attack_0");
	fire_idle = K->ID_Cycle_Safe("norm_torso_knife+detector_attack_1");

	all_attack_0 = K->ID_Cycle_Safe("norm_torso_knife+detector_attack_0");
	all_attack_1 = K->ID_Cycle_Safe("norm_torso_knife+detector_attack_1");
}

void STorsoWpn::CreateDetectorPistol(IKinematicsAnimated* K)
{
	moving[eIdle] = K->ID_Cycle_Safe("norm_torso_pistol+detector_aim_1");
	moving[eWalk] = K->ID_Cycle_Safe("norm_torso_pistol+detector_aim_2");
	moving[eRun] = K->ID_Cycle_Safe("norm_torso_pistol+detector_aim_3");
	moving[eSprint] = K->ID_Cycle_Safe("norm_torso_pistol+detector_escape_0");

	moving[eIdleSafe] = K->ID_Cycle_Safe("norm_torso_pistol+detector_idle_1");
	moving[eWalkSafe] = K->ID_Cycle_Safe("norm_torso_pistol+detector_walk_1");
	moving[eRunSafe] = K->ID_Cycle_Safe("norm_torso_pistol+detector_run_1");
	moving[eSprintSafe] = K->ID_Cycle_Safe("norm_torso_pistol+detector_escape_0");

	zoom = K->ID_Cycle_Safe("norm_torso_pistol+detector_aim_0");

	holster = K->ID_Cycle_Safe("norm_torso_pistol+detector_holster_0");
	holster_all = K->ID_Cycle_Safe("norm_torso_pistol+detector_holsterall_0");
	holster_detector = K->ID_Cycle_Safe("norm_torso_pistol+detector_holsterdevice_0");

	draw = K->ID_Cycle_Safe("norm_torso_pistol+detector_draw_0");
	draw_all = K->ID_Cycle_Safe("norm_torso_pistol+detector_drawall_0");
	draw_detector = K->ID_Cycle_Safe("norm_torso_pistol+detector_drawdevice_0");

	reload = K->ID_Cycle_Safe("norm_torso_pistol+detector_reload_0");


	attack_zoom = K->ID_Cycle_Safe("norm_torso_pistol+detector_attack_0");
	fire_idle = K->ID_Cycle_Safe("norm_torso_pistol+detector_attack_1");
	attack = K->ID_Cycle_Safe("norm_torso_pistol+detector_attack_1");



	all_attack_0 = K->ID_Cycle_Safe("norm_torso_pistol+detector_attack_0");
	all_attack_1 = K->ID_Cycle_Safe("norm_torso_pistol+detector_attack_1");
	all_attack_2 = K->ID_Cycle_Safe("norm_torso_pistol+detector_attack_2");
}

void STorsoWpn::CreateDetectorBolt(IKinematicsAnimated* K)
{
	moving[eIdle]   = K->ID_Cycle_Safe("norm_torso_6+detector_aim_1");
	moving[eWalk]   = K->ID_Cycle_Safe("norm_torso_6+detector_aim_2");
	moving[eRun]    = K->ID_Cycle_Safe("norm_torso_6+detector_aim_3");
	moving[eSprint] = K->ID_Cycle_Safe("norm_torso_6+detector_escape_0");

	moving[eIdleSafe] = K->ID_Cycle_Safe("norm_torso_6+detector_idle_1");
	moving[eWalkSafe] = K->ID_Cycle_Safe("norm_torso_6+detector_walk_1");
	moving[eRunSafe]  = K->ID_Cycle_Safe("norm_torso_6+detector_run_1");
	moving[eSprintSafe] = K->ID_Cycle_Safe("norm_torso_6+detector_escape_0");

	zoom = K->ID_Cycle_Safe("norm_torso_6+detector_aim_0");

	holster = K->ID_Cycle_Safe("norm_torso_6+detector_holster_0");
	holster_all = K->ID_Cycle_Safe("norm_torso_6+detector_holsterall_0");
	holster_detector = K->ID_Cycle_Safe("norm_torso_6+detector_holsterdevice_0");

	draw = K->ID_Cycle_Safe("norm_torso_6+detector_draw_0");
	draw_all = K->ID_Cycle_Safe("norm_torso_6+detector_drawall_0");
	draw_detector = K->ID_Cycle_Safe("norm_torso_6+detector_drawdevice_0");

	reload = K->ID_Cycle_Safe("norm_torso_6+detector_reload_0");


	attack_zoom = K->ID_Cycle_Safe("norm_torso_6+detector_attack_0");
	fire_idle = K->ID_Cycle_Safe("norm_torso_6+detector_attack_1");
	fire_end = K->ID_Cycle_Safe("norm_torso_6+detector_attack_2");

	all_attack_0 = K->ID_Cycle_Safe("norm_torso_6+detector_attack_0");
	all_attack_1 = K->ID_Cycle_Safe("norm_torso_6+detector_attack_1");
	all_attack_2 = K->ID_Cycle_Safe("norm_torso_6+detector_attack_2");
}

void CActor::UpdateDetectorTorsoState(MotionID& M_torso, MotionID& M_head, MotionID& M_legs, int moving_idx, bool is_standing)
{
	bool K = inventory().GetActiveSlot() == KNIFE_SLOT;
	CWeaponCustomPistol* P = smart_cast<CWeaponCustomPistol*>(inventory().ActiveItem());
	CBolt* B = smart_cast<CBolt*>(inventory().ActiveItem());
	CCustomDetector* D = smart_cast<CCustomDetector*>(inventory().ItemFromSlot(DETECTOR_SLOT));
	CWeapon* W = smart_cast<CWeapon*>(inventory().ActiveItem());
	CHudItem* H = smart_cast<CHudItem*>(inventory().ActiveItem());

	u32 type = 0;

	if (D)
	{
		type = D->GetState();

		if (type < 3)
		{
			this->attach(D, false);
		}
		else
		{
			this->detach(D);
		}
	}

	if (!m_bAnimTorsoPlayed)
	{
		if (D && K && type < 3)
		{
			STorsoWpn* STD = &m_anims->m_detector_knife;

			if (type == 0)
			{
				bool not_state = false;
				switch (W->GetState())
				{
				case CWeapon::eShowing:
				{
					M_torso = STD->draw;
				}break;

				case CWeapon::eHiding:
				{
					M_torso = STD->holster;
				}break;

				case CWeapon::eFire:
					if (is_standing)
						M_torso = M_legs = M_head = STD->all_attack_0;
					else
						M_torso = STD->attack_zoom;
					break;

				case CWeapon::eFire2:
					if (is_standing)
						M_torso = M_legs = M_head = STD->all_attack_1;
					else
						M_torso = STD->fire_idle;
					break;


				case CWeapon::eReload:
				{
					M_torso = STD->reload;
				}break;

				default:
					not_state = true;
					break;
				}

				if (not_state)
				{
					if (!MpSafeMode())
						M_torso = STD->moving[moving_idx];
					else
						M_torso = STD->moving[moving_idx + 4];
				}
			}
			else if (type == 1)
			{
				if (W->GetState() == CWeapon::eShowing)
					M_torso = STD->draw_all;
				else
					M_torso = STD->draw_detector;
			}
			else if (type == 2)
			{
				if (W->GetState() == CWeapon::eHiding)
					M_torso = STD->holster_all;
				else
					M_torso = STD->holster_detector;
			}

		}
		else
		if (D && H && type < 3 && H->animation_slot() == 1)
		{
			STorsoWpn* STD = &m_anims->m_detector_pistol;
			u32 wpn_state = H->GetState();

			if (type == 1)
			{
				if (wpn_state == CWeapon::eShowing)
				{
					M_torso = STD->draw_all;
				}
				else
					M_torso = STD->draw_detector;
			}
			else
			if (type == 2)
			{
				if (wpn_state == CWeapon::eHiding)
				{
					M_torso = STD->holster_all;
				}
				else
					M_torso = STD->holster_detector;
			}
			else if (type == 0)
			{

				if (wpn_state == CWeapon::eHiding)
				{
					M_torso = STD->holster;
				}
				else if (wpn_state == CWeapon::eShowing)
				{
					M_torso = STD->draw;
				}
				else if (wpn_state == CWeapon::eIdle)
				{
					if (W->IsZoomed())
						M_torso = STD->attack_zoom;
					else
						M_torso = MpSafeMode() ? STD->moving[moving_idx + 4] : STD->moving[moving_idx];
				}
				else if (wpn_state == CWeapon::eFire)
				{
					M_torso = W->IsZoomed() ? STD->attack_zoom : STD->attack;
				}
				else if (wpn_state == CWeapon::eFire2)
				{
					M_torso = W->IsZoomed() ? STD->attack_zoom : STD->attack;
				}
				else if (wpn_state == CWeapon::eReload)
				{
					M_torso = STD->reload;
				}
			}
		}
		else
		if (D && B && type < 3)
		{
			STorsoWpn* STD = &m_anims->m_detector_bolt;

		}
		else
		if (D && type < 3)
		{
			STorsoWpn* STD = &m_anims->m_detector;

			if (type == CHUDState::eShowing)
			{
				M_torso = STD->draw;
			}
			else if (type == CHUDState::eHiding)
			{
				M_torso = STD->holster;
			}
			else if (type == CHUDState::eIdle)
			{
				if (!MpSafeMode())
					M_torso = STD->moving[moving_idx];
				else
					M_torso = STD->moving[moving_idx + 4];
			}
		}
	}

}