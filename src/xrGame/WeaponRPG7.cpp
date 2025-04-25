#include "stdafx.h"
#include "weaponrpg7.h"
#include "xrserver_objects_alife_items.h"
#include "explosiverocket.h"
#include "entity.h"
#include "level.h"
#include "player_hud.h"
#include "hudmanager.h"
#include "Actor.h"

CWeaponRPG7::CWeaponRPG7()
{
}

CWeaponRPG7::~CWeaponRPG7()
{
}

void CWeaponRPG7::Load(LPCSTR section)
{
	inherited::Load(section);
	CRocketLauncher::Load(section);

	m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "max_zoom_factor");

	m_sRocketSection = pSettings->r_string(section, "rocket_class");
}

bool CWeaponRPG7::AllowBore()
{
	return inherited::AllowBore() && 0 != iAmmoElapsed;
}

void CWeaponRPG7::FireTrace(const Fvector& P, const Fvector& D)
{
	inherited::FireTrace(P, D);
	UpdateMissileVisibility();
}

void CWeaponRPG7::on_a_hud_attach()
{
	inherited::on_a_hud_attach();
	UpdateMissileVisibility();
}

void CWeaponRPG7::UpdateMissileVisibility()
{
	bool vis_hud, vis_weap;
	vis_hud = (!!iAmmoElapsed || GetState() == eReload);
	vis_weap = !!iAmmoElapsed;

	if (GetHUDmode())
	{
		HudItemData()->set_bone_visible("grenade", vis_hud, TRUE);
	}

	IKinematics* pWeaponVisual = smart_cast<IKinematics*>(Visual());
	VERIFY(pWeaponVisual);
	pWeaponVisual->LL_SetBoneVisible(pWeaponVisual->LL_BoneID("grenade"), vis_weap, TRUE);
}

void CWeaponRPG7::StartRocketRecive(Fvector& position, Fvector& direction)
{
	Msg("RPG 7 Start Rocket Recive");

	Fmatrix								launch_matrix;
	launch_matrix.identity();
	launch_matrix.k.set(direction);
	Fvector::generate_orthonormal_basis(launch_matrix.k, launch_matrix.j, launch_matrix.i);
	launch_matrix.c.set(position);

	direction.normalize();
	direction.mul(m_fLaunchSpeed);

	CRocketLauncher::LaunchRocket(launch_matrix, direction, zero_vel);
	CExplosiveRocket* pGrenade = smart_cast<CExplosiveRocket*>(getCurrentRocket());

	if (pGrenade && H_Parent())
		pGrenade->SetInitiator(H_Parent()->ID());

	if (OnServer())
	{
		NET_Packet						P;
		u_EventGen(P, GE_LAUNCH_ROCKET, ID());
		P.w_u16(u16(getCurrentRocket()->ID()));
		u_EventSend(P);
	}
}

void CWeaponRPG7::StartRocketSend()
{
	Msg("RPG 7 Start Rocket Send");

	Fvector position_wp, direction_wp, position;
	Fvector position_entity, direction_entity, direction;
	position_wp.set(get_LastFP());
	direction_wp.set(get_LastFD());
	position = position_wp;
	direction = direction_wp;

	CEntity* E = smart_cast<CEntity*>(H_Parent());
	if (E)
	{
		E->g_fireParams(this, position_entity, direction_entity);
		position  = position_entity;
		direction = direction_entity;

 		if (IsHudModeNow())
		{
			Fvector		p0;
			float dist = HUD().GetCurrentRayQuery().range;
			p0.mul(direction_entity, dist);
			p0.add(position_wp);
			position = position_wp;
			direction.sub(p0, position_wp);
			direction.normalize_safe();
		}
	}
	 
	NET_Packet packet;
	Game().u_EventGen(packet, GE_WPN_STARTGRENADE, ID());
	packet.w_vec3(position);
	packet.w_vec3(direction);
	Game().u_EventSend(packet);
}

BOOL CWeaponRPG7::net_Spawn(CSE_Abstract* DC)
{
	BOOL l_res = inherited::net_Spawn(DC);

	UpdateMissileVisibility();
	if (iAmmoElapsed != getRocketCount())
		CRocketLauncher::SpawnRocket(m_sRocketSection, this);

	return l_res;
}

void CWeaponRPG7::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch(S);
	UpdateMissileVisibility();
}

void CWeaponRPG7::UnloadMagazine(bool spawn_ammo)
{
	inherited::UnloadMagazine(spawn_ammo);
	UpdateMissileVisibility();
}

void CWeaponRPG7::ReloadMagazine()
{
	inherited::ReloadMagazine();
 
	if (iAmmoElapsed != getRocketCount())
	{
		CRocketLauncher::SpawnRocketSend(m_sRocketSection.c_str(), this);
		Msg("Send Spawn Rocket: %u | %u", iAmmoElapsed, getRocketCount());
	}

}

void CWeaponRPG7::SwitchState(u32 S)
{
	inherited::SwitchState(S);
}

void CWeaponRPG7::FireStart()
{
	inherited::FireStart();
}

#include "inventory.h"
#include "inventoryOwner.h"
void CWeaponRPG7::switch2_Fire()
{
	m_iShotNum = 0;
	m_bFireSingleShot = true;
	bWorking = false;

	if (GetState() == eFire)
	{
		iAmmoElapsed = getRocketCount();
		if (!getRocketCount())
		{
			Msg("No Has Any Rocket in vector");
			return;
		}
		 
		CActor* A = smart_cast<CActor*>(H_Parent());
		if (A->IsFocused())
		{
			StartRocketSend();
			return;
		}
		
		if (!A && OnServer())
			StartRocketSend();
	}
}

void CWeaponRPG7::PlayAnimReload()
{
	VERIFY(GetState() == eReload);
	PlayHUDMotion("anm_reload", FALSE, this, GetState());
}
 
void CWeaponRPG7::OnEvent(NET_Packet& P, u16 type)
{
	inherited::OnEvent(P, type);
	u16 id;
	switch (type) 
	{
		case GE_WPN_STARTGRENADE:
		{
			Fvector position, direction;
			P.r_vec3(position);
			P.r_vec3(direction);
			StartRocketRecive(position, direction);
		}break;

		case GE_WPN_SPAWNGRENADE:
		{
			if (OnServer())
			{
				shared_str section;
				P.r_stringZ(section);
				if (getCurrentRocket() <= 0)
					CRocketLauncher::SpawnRocket(section, this);
			}break;
		}break;


		case GE_OWNERSHIP_TAKE:
		{
			P.r_u16(id);
			CRocketLauncher::AttachRocket(id, this);
		} break;
	
		case GE_OWNERSHIP_REJECT:
		case GE_LAUNCH_ROCKET:
		{
			bool bLaunch = (type == GE_LAUNCH_ROCKET);
			P.r_u16(id);
			CRocketLauncher::DetachRocket(id, bLaunch);
			if (bLaunch)
				UpdateMissileVisibility();

		} break;
	}
}

void CWeaponRPG7::net_Import(NET_Packet& P)
{
	inherited::net_Import(P);
	UpdateMissileVisibility();
}
