#include "StdAfx.h"
#include "Weapon.h"
#include "Level.h"

// SHELL

void CWeapon::create_physic_shell()
{
	CPhysicsShellHolder::create_physic_shell();
}

void CWeapon::activate_physic_shell()
{
	UpdateXForm();
	CPhysicsShellHolder::activate_physic_shell();
}

void CWeapon::setup_physic_shell()
{
	CPhysicsShellHolder::setup_physic_shell();
}



void CWeapon::OnH_B_Independent(bool just_before_destroy)
{
	RemoveShotEffector();

	inherited::OnH_B_Independent(just_before_destroy);

	FireEnd();
	SetPending(FALSE);
	SwitchState(eHidden);

	m_strapped_mode = false;
	m_strapped_mode_rifle = false;
	m_zoom_params.m_bIsZoomModeNow = false;
	UpdateXForm();
}

void CWeapon::OnH_A_Independent()
{
	m_dwWeaponIndependencyTime = Level().timeServer();
	m_fLR_MovingFactor = 0.f;
	m_fLR_CameraFactor = 0.f;
	m_fLR_InertiaFactor = 0.f;
	m_fUD_InertiaFactor = 0.f;

	inherited::OnH_A_Independent();
	Light_Destroy();
	UpdateAddonsVisibility();
};

void CWeapon::OnH_A_Chield()
{
	inherited::OnH_A_Chield();
	UpdateAddonsVisibility();
};

void CWeapon::OnH_B_Chield()
{
	m_dwWeaponIndependencyTime = 0;
	inherited::OnH_B_Chield();

	OnZoomOut();
	m_set_next_ammoType_on_reload = undefined_ammo_type;
}
