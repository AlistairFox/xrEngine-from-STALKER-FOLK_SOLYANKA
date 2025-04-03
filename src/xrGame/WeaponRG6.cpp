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
	CActor* A = smart_cast<CActor*>(H_Parent());

	if (A && A->IsFocused())
	{
		FireStartSend();
	}
	else
	if (OnServer() && !A)
	{
		FireStartSend();
	}
}

void CWeaponRG6::FireStartSend()
{
	Msg("RG6 Fire Start Send [%u] ammos", getRocketCount());
	if (GetState() == eIdle)
	{
		inheritedSG::FireStart();

		Fvector position, direction;
		position.set(get_LastFP());
		direction.set(get_LastFD());

		CEntity* E = smart_cast<CEntity*>(H_Parent());
		if (E) 
  			E->g_fireParams(this, position, direction);
 
		CActor* A = smart_cast<CActor*>(H_Parent());

		if (IsZoomed() && A != nullptr && A->IsFocused())
		{
			H_Parent()->setEnabled(FALSE);
			setEnabled(FALSE);

			collide::rq_result RQ;
			BOOL HasPick = Level().ObjectSpace.RayPick(position, direction, 300.0f, collide::rqtStatic, RQ, this);

			setEnabled(TRUE);
			H_Parent()->setEnabled(TRUE);

			if (HasPick)
			{
				Fvector Transference;
				Transference.mul(direction, RQ.range);
				Fvector res[2];
				u8 canfire0 = TransferenceAndThrowVelToThrowDir(Transference, CRocketLauncher::m_fLaunchSpeed, EffectiveGravity(), res);
				if (canfire0 != 0)
				{
					direction = res[0];
				};
			}
		};

		if (getRocketCount() >= 1)
		{
			dropCurrentRocket();

			NET_Packet packet;
			Game().u_EventGen(packet, GE_WPN_STARTGRENADE, ID());
			packet.w_vec3(position);
			packet.w_vec3(direction);
			Game().u_EventSend(packet);
		}
		else
			Msg("[ERROR] Current Rocket: %u", getRocketCount());
	}
}
 
void CWeaponRG6::FireStartRecive(Fvector& position, Fvector& direction)
{
	Msg("RG6 Fire Start Recive [%u] ammos", getRocketCount());

	if (!getCurrentRocket())
	{	
		Msg("Current Rockets is no correct to SV");
		return;
	}

	Fmatrix launch_matrix;
	launch_matrix.identity();
	launch_matrix.k.set(direction);
	Fvector::generate_orthonormal_basis(launch_matrix.k, launch_matrix.j, launch_matrix.i);
	launch_matrix.c.set(position);

	direction.normalize();
	direction.mul(m_fLaunchSpeed);
	
	Msg("Setup Launch Rocket");
 	CRocketLauncher::LaunchRocket(launch_matrix, direction, zero_vel);

	CExplosiveRocket* pGrenade = smart_cast<CExplosiveRocket*>(getCurrentRocket());
	if (pGrenade != nullptr)
	{
		Msg("Setup Initiator Rocket");
		pGrenade->SetInitiator(H_Parent()->ID());
	}
	else
		Msg("Rocket[ getCurrentRocket() ] is empty !!!");

	if (OnServer())
	{
		NET_Packet P;
		u_EventGen(P, GE_LAUNCH_ROCKET, ID());
		P.w_u16(u16(getCurrentRocket()->ID()));
		u_EventSend(P);
	}
}
 
u8 CWeaponRG6::AddCartridge(u8 cnt)
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

void CWeaponRG6::OnEvent(NET_Packet& P, u16 type)
{
	inheritedSG::OnEvent(P, type);

	u16 id;
	switch (type)
	{
		case GE_WPN_STARTGRENADE:
		{ 
			Msg("GE_WPN_STARTGRENADE");
			Fvector position, direction;
			P.r_vec3(position);
			P.r_vec3(direction);
			FireStartRecive(direction, position);
		}break;

		case GE_WPN_SPAWNGRENADE:
		{
			Msg("GE_WPN_SPAWNGRENADE");
			if (OnServer())
			{
				shared_str section;
				P.r_stringZ(section);
				CRocketLauncher::SpawnRocket(section, this);
			}break;
		}break;

		case GE_OWNERSHIP_TAKE:
		{
			P.r_u16(id);
			
			// Msg("GE_ROCKET_TAKE : %u", id);
 			inheritedRL::AttachRocket(id, this);
		} break;
		
		case GE_OWNERSHIP_REJECT:
		case GE_LAUNCH_ROCKET:
		{
			bool bLaunch = (type == GE_LAUNCH_ROCKET);
			P.r_u16(id);

			// Msg("GE_LAUNCH_ROCKET : %u", id);
			inheritedRL::DetachRocket(id, bLaunch);
		} break;
	}
}
