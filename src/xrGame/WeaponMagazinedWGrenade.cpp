#include "stdafx.h"
#include "weaponmagazinedwgrenade.h"
#include "entity.h"
#include "ParticlesObject.h"
#include "GrenadeLauncher.h"
#include "xrserver_objects_alife_items.h"
#include "ExplosiveRocket.h"
#include "Actor.h"
#include "xr_level_controller.h"
#include "level.h"
#include "object_broker.h"
#include "game_base_space.h"
#include "../xrphysics/MathUtils.h"
#include "player_hud.h"

#include "script_callback_ex.h"
#include "script_game_object.h"

#ifdef DEBUG
#	include "phdebug.h"
#endif

CWeaponMagazinedWGrenade::CWeaponMagazinedWGrenade(ESoundTypes eSoundType) : CWeaponMagazined(eSoundType)
{
	m_ammoType2 = 0;
	m_bGrenadeMode = false;
}

CWeaponMagazinedWGrenade::~CWeaponMagazinedWGrenade()
{
}

void CWeaponMagazinedWGrenade::net_Destroy()
{
	inherited::net_Destroy();
}

// LOADING

void CWeaponMagazinedWGrenade::Load(LPCSTR section)
{
	inherited::Load(section);
	CRocketLauncher::Load(section);

	SetAnimFlag(ANM_RELOAD_EMPTY_GL, "anm_reload_empty_w_gl");
	SetAnimFlag(ANM_SHOT_AIM_GL, "anm_shots_w_gl_when_aim");
	SetAnimFlag(ANM_MISFIRE_GL, "anm_reload_misfire_w_gl");

	//// Sounds
	m_sounds.LoadSound(section, "snd_shoot_grenade", "sndShotG", false, m_eSoundShot);
	m_sounds.LoadSound(section, "snd_reload_grenade", "sndReloadG", true, m_eSoundReload);
	m_sounds.LoadSound(section, "snd_switch", "sndSwitch", true, m_eSoundReload);


	m_sFlameParticles2 = pSettings->r_string(section, "grenade_flame_particles");


	if (m_eGrenadeLauncherStatus == ALife::eAddonPermanent)
	{
		CRocketLauncher::m_fLaunchSpeed = pSettings->r_float(section, "grenade_vel");
	}

	// load ammo classes SECOND (grenade_class)
	m_ammoTypes2.clear();
	LPCSTR				S = pSettings->r_string(section, "grenade_class");
	if (S && S[0])
	{
		string128		_ammoItem;
		int				count = _GetItemCount(S);
		for (int it = 0; it < count; ++it)
		{
			_GetItem(S, it, _ammoItem);
			m_ammoTypes2.push_back(_ammoItem);
		}
	}

	iMagazineSize2 = iMagazineSize;
	iAmmoElapsedMain = 0;
}

void CWeaponMagazinedWGrenade::save(NET_Packet& output_packet)
{
	inherited::save(output_packet);
	save_data(m_bGrenadeMode, output_packet);
	save_data(m_magazine2.size(), output_packet);

}


void CWeaponMagazinedWGrenade::load(IReader& input_packet)
{
	inherited::load(input_packet);
	bool b;
	load_data(b, input_packet);
	if (b != m_bGrenadeMode)
		SwitchMode();

	u32 sz;
	load_data(sz, input_packet);

	CCartridge					l_cartridge;
	l_cartridge.Load(m_ammoTypes2[m_ammoType2].c_str(), m_ammoType2);

	while (sz > m_magazine2.size())
		m_magazine2.push_back(l_cartridge);
}


/// OTHER


void CWeaponMagazinedWGrenade::UpdateSecondVP(bool bInGrenade)
{
	inherited::UpdateSecondVP(m_bGrenadeMode);
}

BOOL CWeaponMagazinedWGrenade::net_Spawn(CSE_Abstract* DC)
{
	CSE_ALifeItemWeapon* const weapon = smart_cast<CSE_ALifeItemWeapon*>(DC);
	R_ASSERT(weapon);
	//	if (IsGameTypeSingle())
	{
		inherited::net_Spawn_install_upgrades(weapon->m_upgrades);
	}

	BOOL l_res = inherited::net_Spawn(DC);

	UpdateGrenadeVisibility(!!iAmmoElapsed);
	SetPending(FALSE);

	iAmmoElapsed2 = weapon->a_elapsed_grenades.grenades_count;
	m_ammoType2 = weapon->a_elapsed_grenades.grenades_type;

	m_DefaultCartridge2.Load(m_ammoTypes2[m_ammoType2].c_str(), m_ammoType2);

	if (!IsGameTypeSingle())
	{
		if (!m_bGrenadeMode && IsGrenadeLauncherAttached() && !getRocketCount() && iAmmoElapsed2)
		{
			m_magazine2.push_back(m_DefaultCartridge2);

			shared_str grenade_name = m_DefaultCartridge2.m_ammoSect;
			shared_str fake_grenade_name = pSettings->r_string(grenade_name, "fake_grenade_name");

			CRocketLauncher::SpawnRocket(*fake_grenade_name, this);
		}
	}
	else
	{
		xr_vector<CCartridge>* pM = NULL;
		bool b_if_grenade_mode = (m_bGrenadeMode && iAmmoElapsed && !getRocketCount());
		if (b_if_grenade_mode)
			pM = &m_magazine;

		bool b_if_simple_mode = (!m_bGrenadeMode && m_magazine2.size() && !getRocketCount());
		if (b_if_simple_mode)
			pM = &m_magazine2;

		if (b_if_grenade_mode || b_if_simple_mode)
		{
			shared_str fake_grenade_name = pSettings->r_string(pM->back().m_ammoSect, "fake_grenade_name");

			CRocketLauncher::SpawnRocket(*fake_grenade_name, this);
		}
	}
	return l_res;
}



void CWeaponMagazinedWGrenade::OnMotionMark(u32 state, const motion_marks& M)
{
	inherited::OnMotionMark(state, M);
}

void  CWeaponMagazinedWGrenade::LaunchGrenade()
{
	if (!getRocketCount())
	{
		Msg("Rockets : %d", getRocketCount());
		return;
	}

	R_ASSERT(m_bGrenadeMode);
	{
		Fvector						p1, d;
		p1.set(get_LastFP2());
		d.set(get_LastFD());
		CEntity* E = smart_cast<CEntity*>(H_Parent());
		if (E)
		{
			CInventoryOwner* io = smart_cast<CInventoryOwner*>(H_Parent());
			E->g_fireParams(this, p1, d);
		}
		if (IsGameTypeSingle())
			p1.set(get_LastFP2());

		Fmatrix							launch_matrix;
		launch_matrix.identity();
		launch_matrix.k.set(d);
		Fvector::generate_orthonormal_basis(launch_matrix.k,
			launch_matrix.j,
			launch_matrix.i);

		launch_matrix.c.set(p1);

		if (IsZoomed() && smart_cast<CActor*>(H_Parent()))
		{
			H_Parent()->setEnabled(FALSE);
			setEnabled(FALSE);

			collide::rq_result			RQ;
			BOOL HasPick = Level().ObjectSpace.RayPick(p1, d, 300.0f, collide::rqtStatic, RQ, this);

			setEnabled(TRUE);
			H_Parent()->setEnabled(TRUE);

			if (HasPick)
			{
				Fvector					Transference;
				Transference.mul(d, RQ.range);
				Fvector					res[2];

				u8 canfire0 = TransferenceAndThrowVelToThrowDir(Transference,
					CRocketLauncher::m_fLaunchSpeed,
					EffectiveGravity(),
					res);
				if (canfire0 != 0)
				{
					d = res[0];
				};
			}
		};

		d.normalize();
		d.mul(CRocketLauncher::m_fLaunchSpeed);
		VERIFY2(_valid(launch_matrix), "CWeaponMagazinedWGrenade::SwitchState. Invalid launch_matrix!");
		CRocketLauncher::LaunchRocket(launch_matrix, d, zero_vel);

		CExplosiveRocket* pGrenade = smart_cast<CExplosiveRocket*>(getCurrentRocket());
		VERIFY(pGrenade);
		pGrenade->SetInitiator(H_Parent()->ID());


		if (Local() && OnServer())
		{
			Msg("Magazine Size: %d", m_magazine.size());

			if (m_magazine.size() > 0)	// se7kills Оружие багует (изза синхры через клиент делается вызов к 0му буферу)
			{
				VERIFY(m_magazine.size());
				m_magazine.pop_back();
				VERIFY((u32)iAmmoElapsed == m_magazine.size());
			}
			--iAmmoElapsed;

			NET_Packet					P;
			u_EventGen(P, GE_LAUNCH_ROCKET, ID());
			P.w_u16(getCurrentRocket()->ID());
			u_EventSend(P);
		};
	}
}

void CWeaponMagazinedWGrenade::LaunchGrenadeRecived(Fvector direction, Fvector position)
{
	if (!getRocketCount())
	{
		Msg("Rockets : %d", getRocketCount());
		return;
	}
	{
		Fmatrix							launch_matrix;
		launch_matrix.identity();
		launch_matrix.k.set(direction);
		Fvector::generate_orthonormal_basis(launch_matrix.k,
			launch_matrix.j,
			launch_matrix.i);
		launch_matrix.c.set(position);

		direction.normalize();
		direction.mul(CRocketLauncher::m_fLaunchSpeed);

		CRocketLauncher::LaunchRocket(launch_matrix, direction, zero_vel);

		CExplosiveRocket* pGrenade = smart_cast<CExplosiveRocket*>(getCurrentRocket());
		pGrenade->SetInitiator(H_Parent()->ID());	// Скорей всего чтобы не взорвалась в игроке иле нпс


		if (Local() && OnServer())
		{
			// if (m_magazine.size() > 0)	// se7kills Оружие багует (изза синхры через клиент делается вызов к 0му буферу)
			// {
			// 		VERIFY(m_magazine.size());
			// 		m_magazine.pop_back();
			// 		VERIFY((u32)iAmmoElapsed == m_magazine.size());
			// 		--iAmmoElapsed;
			// }

			NET_Packet					P;
			u_EventGen(P, GE_LAUNCH_ROCKET, ID());
			P.w_u16(getCurrentRocket()->ID());
			u_EventSend(P);
		};
	}
}

void CWeaponMagazinedWGrenade::LaunchGrenadeSend()
{
	Fvector						position, direction;
	position.set(get_LastFP2());
	direction.set(get_LastFD());

	CEntity* E = smart_cast<CEntity*>(H_Parent());
	if (E)
	{
		CInventoryOwner* io = smart_cast<CInventoryOwner*>(H_Parent());
		E->g_fireParams(this, position, direction);
	}

	// FOR SINGLE USE THIS
	//position.set(get_LastFP2());

	if (IsZoomed() && smart_cast<CActor*>(H_Parent()))
	{
		H_Parent()->setEnabled(FALSE);
		setEnabled(FALSE);

		collide::rq_result			RQ;
		BOOL HasPick = Level().ObjectSpace.RayPick(position, direction, 300.0f, collide::rqtStatic, RQ, this);

		setEnabled(TRUE);
		H_Parent()->setEnabled(TRUE);

		if (HasPick)
		{
			Fvector					Transference;
			Transference.mul(direction, RQ.range);
			Fvector					res[2];

			u8 canfire0 = TransferenceAndThrowVelToThrowDir(Transference, CRocketLauncher::m_fLaunchSpeed, EffectiveGravity(), res);
			if (canfire0 != 0)
			{
				direction = res[0];
			};
		}
	};


	if (m_magazine.size() > 0)
	{
		NET_Packet packet;
		Game().u_EventGen(packet, GE_WPN_STARTGRENADE, ID());
		packet.w_vec3(position);
		packet.w_vec3(direction);
		Game().u_EventSend(packet);

		m_magazine.pop_back();
		--iAmmoElapsed;
	}
}

void CWeaponMagazinedWGrenade::FireEnd()
{
	if (m_bGrenadeMode)
	{
		CWeapon::FireEnd();
	}
	else
		inherited::FireEnd();
}

void CWeaponMagazinedWGrenade::OnMagazineEmpty()
{
	if (GetState() == eIdle)
	{
		OnEmptyClick();
	}
}

void CWeaponMagazinedWGrenade::ReloadMagazine()
{
	inherited::ReloadMagazine();

	//перезарядка подствольного гранатомета
	if (iAmmoElapsed && !getRocketCount() && m_bGrenadeMode)
	{
		shared_str fake_grenade_name = pSettings->r_string(m_ammoTypes[m_ammoType].c_str(), "fake_grenade_name");

		CRocketLauncher::SpawnRocketSend(*fake_grenade_name, this);
	}
}

void CWeaponMagazinedWGrenade::OnH_B_Independent(bool just_before_destroy)
{
	inherited::OnH_B_Independent(just_before_destroy);

	SetPending(FALSE);
	if (m_bGrenadeMode) {
		SetState(eIdle);
		SetPending(FALSE);
	}
}

float	CWeaponMagazinedWGrenade::CurrentZoomFactor()
{
	if (IsGrenadeLauncherAttached() && m_bGrenadeMode) return m_zoom_params.m_fIronSightZoomFactor;
	return inherited::CurrentZoomFactor();
}

void CWeaponMagazinedWGrenade::UpdateSounds()
{
	inherited::UpdateSounds();
	Fvector P = get_LastFP();
	m_sounds.SetPosition("sndShotG", P);
	m_sounds.SetPosition("sndReloadG", P);
	m_sounds.SetPosition("sndSwitch", P);
}

void CWeaponMagazinedWGrenade::UpdateGrenadeVisibility(bool visibility)
{
	if (!GetHUDmode())							return;
	HudItemData()->set_bone_visible("grenade", visibility, TRUE);
}


void CWeaponMagazinedWGrenade::net_Export(NET_Packet& P)
{
	P.w_u8(m_bGrenadeMode ? 1 : 0);

	inherited::net_Export(P);
}

void CWeaponMagazinedWGrenade::net_Import(NET_Packet& P)
{
	bool NewMode = FALSE;
	NewMode = !!P.r_u8();
	if (NewMode != m_bGrenadeMode)
		SwitchMode();

	inherited::net_Import(P);
}

bool CWeaponMagazinedWGrenade::IsNecessaryItem(const shared_str& item_sect)
{
	return (std::find(m_ammoTypes.begin(), m_ammoTypes.end(), item_sect) != m_ammoTypes.end() ||
		std::find(m_ammoTypes2.begin(), m_ammoTypes2.end(), item_sect) != m_ammoTypes2.end()
		);
}

u8 CWeaponMagazinedWGrenade::GetCurrentHudOffsetIdx()
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor)
		return 0;

	if (pActor->MpSafeMode())
		return 4;

	bool b_aiming = ((IsZoomed() && m_zoom_params.m_fZoomRotationFactor <= 1.f) || (!IsZoomed() && m_zoom_params.m_fZoomRotationFactor > 0.f));
	if (!b_aiming)
		return 0;
	if (m_bGrenadeMode)
		return 2;
	else if (m_bAltZoomActive)
		return 3;
	else
		return 1;

}


int CWeaponMagazinedWGrenade::GetAmmoCount2(u8 ammo2_type) const
{
	VERIFY(m_pInventory);
	R_ASSERT(ammo2_type < m_ammoTypes2.size());

	return GetAmmoCount_forType(m_ammoTypes2[ammo2_type]);
}


void CWeaponMagazinedWGrenade::CheckMagazine()
{
	if (m_bGrenadeMode || !ParentIsActor())
	{
		m_bNeedBulletInGun = false;
		return;
	}

	if ((isHUDAnimationExist("anm_reload_w_gl_empty") || isHUDAnimationExist("anm_reload_empty_w_gl") || isHUDAnimationExist("anm_reload_empty")) && iAmmoElapsed >= 1 && m_bNeedBulletInGun == false)
	{
		m_bNeedBulletInGun = true;
	}
	else if ((isHUDAnimationExist("anm_reload_w_gl_empty") || isHUDAnimationExist("anm_reload_empty_w_gl") || isHUDAnimationExist("anm_reload_empty")) && iAmmoElapsed == 0 && m_bNeedBulletInGun == true)
	{
		m_bNeedBulletInGun = false;
	}
}


