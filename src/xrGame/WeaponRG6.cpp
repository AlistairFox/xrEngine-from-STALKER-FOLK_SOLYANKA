#include "stdafx.h"
#include "WeaponRG6.h"
#include "entity.h"
#include "explosiveRocket.h"
#include "level.h"
#include "../xrphysics/MathUtils.h"
#include "actor.h"

#ifdef DEBUG
#	include "phdebug.h"
#endif


CWeaponRG6::~CWeaponRG6()
{
}

BOOL	CWeaponRG6::net_Spawn(CSE_Abstract* DC)
{
	BOOL l_res = inheritedSG::net_Spawn(DC);
	if (!l_res) return l_res;

	if (iAmmoElapsed && !getCurrentRocket())
	{
		shared_str grenade_name = m_ammoTypes[0];
		shared_str fake_grenade_name = pSettings->r_string(grenade_name, "fake_grenade_name");

		if (fake_grenade_name.size())
		{
			int k = iAmmoElapsed;
			while (k)
			{
				k--;
				inheritedRL::SpawnRocket(*fake_grenade_name, this);
			}
		}
	}

	return l_res;
};

void CWeaponRG6::Load(LPCSTR section)
{
	inheritedRL::Load(section);
	inheritedSG::Load(section);
}
#include "inventory.h"
#include "inventoryOwner.h"
void CWeaponRG6::FireStart()
{
	Msg("Rockets : %u", getRocketCount());

	if (GetState() == eIdle && getRocketCount())
	{
		inheritedSG::FireStart();

		Fvector Position, Direction;
		Position.set(get_LastFP());
		Direction.set(get_LastFD());

		CEntity* E = smart_cast<CEntity*>(H_Parent());
		if (E)
		{
			CInventoryOwner* io = smart_cast<CInventoryOwner*>(H_Parent());
			if (NULL == io->inventory().ActiveItem())
			{
				Log("current_state", GetState());
				Log("next_state", GetNextState());
				Log("item_sect", cNameSect().c_str());
				Log("H_Parent", H_Parent()->cNameSect().c_str());
			}
			E->g_fireParams(this, Position, Direction);
		}

		Fmatrix launch_matrix;
		launch_matrix.identity();
		launch_matrix.k.set(Direction);
		Fvector::generate_orthonormal_basis(launch_matrix.k, launch_matrix.j, launch_matrix.i);
		launch_matrix.c.set(Position);

		if (Level().CurrentControlEntity() == H_Parent() && IsZoomed())
		{
			H_Parent()->setEnabled(FALSE);
			setEnabled(FALSE);

			collide::rq_result RQ;
			BOOL HasPick = Level().ObjectSpace.RayPick(Position, Direction, 300.0f, collide::rqtStatic, RQ, this);

			setEnabled(TRUE);
			H_Parent()->setEnabled(TRUE);

			if (HasPick)
			{
				Fvector Transference;
				Transference.mul(Direction, RQ.range);
				Fvector res[2];
				u8 canfire0 = TransferenceAndThrowVelToThrowDir(Transference, CRocketLauncher::m_fLaunchSpeed, EffectiveGravity(), res);
				if (canfire0 != 0)
				{
					Direction = res[0];
				};
			}
		};

		Direction.normalize();
		Direction.mul(m_fLaunchSpeed);

		u32 RocketID = getCurrentRocket()->ID();

		NET_Packet P;
		Game().u_EventGen(P, GE_WPN_STARTGRENADE, ID());
		P.w_u32(RocketID);
		P.w_matrix(launch_matrix);
		P.w_vec3(Direction);
		Game().u_EventSend(P);

		/*
		CRocketLauncher::LaunchRocket(launch_matrix, Direction, zero_vel);

		CExplosiveRocket* pGrenade = smart_cast<CExplosiveRocket*>(getCurrentRocket());
		VERIFY(pGrenade);
		pGrenade->SetInitiator(H_Parent()->ID());

		if (OnServer())
		{
			NET_Packet P;
			u_EventGen(P, GE_LAUNCH_ROCKET, ID());
			P.w_u16(u16(getCurrentRocket()->ID()));
			u_EventSend(P);
		}

		dropCurrentRocket();
		*/
	}
}

 
u8 CWeaponRG6::AddCartridge(u8 cnt)
{
	CActor* a = smart_cast<CActor*> (H_Parent());
	bool isCurentCotrl = a == Level().CurrentControlEntity();

	if (isCurentCotrl || !a && OnServer())
	{
		u8 t = inheritedSG::AddCartridge(cnt);
		u8 k = cnt - t;
		shared_str fake_grenade_name = pSettings->r_string(m_ammoTypes[m_ammoType].c_str(), "fake_grenade_name");
		while (k)
		{
			--k;
			inheritedRL::SpawnRocketSend(*fake_grenade_name, this);
		}

		return k;
	}
	else
		return 0;
}

void CWeaponRG6::OnEvent(NET_Packet& P, u16 type)
{
	inheritedSG::OnEvent(P, type);

	u16 id;
	switch (type)
	{
	case GE_WPN_SPAWNGRENADE:
	{
		if (OnServer())
		{
			shared_str section;
			P.r_stringZ(section);
			CRocketLauncher::SpawnRocket(section, this);
		}break;
	}break;

	case GE_WPN_STARTGRENADE:
	{
		Fmatrix launch_matrix;
		Fvector Direction;
		u32 RocketID;

		P.r_u32(RocketID);
		P.r_matrix(launch_matrix);
		P.r_vec3(Direction);

		CExplosiveRocket* rocket = smart_cast<CExplosiveRocket*> (Level().Objects.net_Find(RocketID));

		if (rocket)
		{
			Msg("Launch Rocket : %u == current: %u", RocketID, getCurrentRocket()->ID());

			CRocketLauncher::LaunchRocket(launch_matrix, Direction, zero_vel);

			CExplosiveRocket* pGrenade = smart_cast<CExplosiveRocket*>(getCurrentRocket());
			VERIFY(pGrenade);
			pGrenade->SetInitiator(H_Parent()->ID());

			if (OnServer())
			{
				NET_Packet P;
				u_EventGen(P, GE_LAUNCH_ROCKET, ID());
				P.w_u16(getCurrentRocket()->ID());
				u_EventSend(P);
			}

			dropCurrentRocket();
		}

	}break;

	case GE_OWNERSHIP_TAKE:
	{
		P.r_u16(id);
		inheritedRL::AttachRocket(id, this);
	} break;

	case GE_OWNERSHIP_REJECT:
	case GE_LAUNCH_ROCKET:
	{
		bool bLaunch = (type == GE_LAUNCH_ROCKET);
		P.r_u16(id);
		inheritedRL::DetachRocket(id, bLaunch);
	} break;
	}
}
