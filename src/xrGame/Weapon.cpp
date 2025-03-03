#include "stdafx.h"
#include "Weapon.h"

#include "actor.h"	


#define ROTATION_TIME			0.25f


CWeapon::CWeapon()
{
	SetState(eHidden);
	SetNextState(eHidden);
	m_sub_state = eSubstateReloadBegin;
	m_bTriStateReload = false;
	SetDefaults();

	m_Offset.identity();
	m_StrapOffset.identity();
	m_StrapOffset_alt.identity();

	m_iAmmoCurrentTotal = 0;
	m_BriefInfo_CalcFrame = 0;

	iAmmoElapsed = -1;
	iMagazineSize = -1;
	m_ammoType = 0;

	eHandDependence = hdNone;

	m_zoom_params.m_fCurrentZoomFactor = GameConstants::GetOGSE_WpnZoomSystem() ? 1.f : g_fov;
	m_zoom_params.m_fZoomRotationFactor = 0.f;
	m_zoom_params.m_pVision = NULL;
	m_zoom_params.m_pNight_vision = NULL;
	m_zoom_params.m_fSecondVPFovFactor = 0.0f;
	m_zoom_params.m_f3dZoomFactor = 0.0f;

	m_pCurrentAmmo = NULL;

	m_pFlameParticles2 = NULL;
	m_sFlameParticles2 = NULL;


	m_fCurrentCartirdgeDisp = 1.f;

	m_strap_bone0 = 0;
	m_strap_bone1 = 0;
	m_strap_bone0_id = -1;
	m_strap_bone1_id = -1;
	m_StrapOffset.identity();
	m_StrapOffset_alt.identity();
	m_strapped_mode = false;
	m_strapped_mode_rifle = false;
	m_can_be_strapped_rifle = false;
	m_can_be_strapped = false;
	m_ef_main_weapon_type = u32(-1);
	m_ef_weapon_type = u32(-1);
	m_UIScope = NULL;
	m_set_next_ammoType_on_reload = undefined_ammo_type;
	m_crosshair_inertion = 0.f;
	m_activation_speed_is_overriden = false;
	m_cur_scope = NULL;
	m_bRememberActorNVisnStatus = false;
	m_freelook_switch_back = false;
	m_fLR_MovingFactor = 0.f;
	m_fLR_CameraFactor = 0.f;
	m_fLR_InertiaFactor = 0.f;
	m_fUD_InertiaFactor = 0.f;

	//Mortan: new params
	bUseAltScope = false;
	bScopeIsHasTexture = false;
	bNVsecondVPavaible = false;
	bNVsecondVPstatus = false;
	m_fZoomStepCount = 3.0f;
	m_fZoomMinKoeff = 0.3f;

	bHasBulletsToHide = false;
	bullet_cnt = 0;

	m_bUseAimAnmDirDependency = false;
	m_bUseScopeAimMoveAnims = true;
	m_bAltZoomEnabled = false;
	m_bAltZoomActive = false;


	m_hud_offset[0] = Fvector().set(0, 0, 0);
	m_hud_offset[1] = Fvector().set(0, 0, 0);
}

CWeapon::~CWeapon()
{
	xr_delete(m_UIScope);
	delete_data(m_scopes);

	laser_light_render.destroy();
	flashlight_render.destroy();
	flashlight_omni.destroy();
	flashlight_glow.destroy();
}

void CWeapon::Hit(SHit* pHDS)
{
	inherited::Hit(pHDS);
}

void CWeapon::shedule_Update(u32 dT)
{
	// Queue shrink
//	u32	dwTimeCL		= Level().timeServer()-NET_Latency;
//	while ((NET.size()>2) && (NET[1].dwTimeStamp<dwTimeCL)) NET.pop_front();	

	// Inherited
	inherited::shedule_Update(dT);
}

void CWeapon::UpdateCL()
{
	inherited::UpdateCL();
	UpdateHUDAddonsVisibility();
	//подсветка от выстрела
	UpdateLight();
	UpdateLaser();
	UpdateFlashlight();

	//нарисовать партиклы
	UpdateFlameParticles();
	UpdateFlameParticles2();

	if (!IsGameTypeSingle())
		make_Interpolation();

	if ((GetNextState() == GetState()) && IsGameTypeSingle() && H_Parent() == Level().CurrentEntity())
		UpdateBoreState();

	UpdateNightVision();
}

// SPEED
bool CWeapon::ActivationSpeedOverriden(Fvector& dest, bool clear_override)
{
	if (m_activation_speed_is_overriden)
	{
		if (clear_override)
		{
			m_activation_speed_is_overriden = false;
		}

		dest = m_overriden_activation_speed;
		return							true;
	}

	return								false;
}

void CWeapon::SetActivationSpeedOverride(Fvector const& speed)
{
	m_overriden_activation_speed = speed;
	m_activation_speed_is_overriden = true;
}

void CWeapon::modify_holder_params(float& range, float& fov) const
{
	if (!IsScopeAttached()) {
		inherited::modify_holder_params(range, fov);
		return;
	}
	range *= m_addon_holder_range_modifier;
	fov *= m_addon_holder_fov_modifier;
}


float CWeapon::GetConditionToShow() const
{
	return	(GetCondition());
}

BOOL CWeapon::ParentMayHaveAimBullet()
{
	CObject* O = H_Parent();
	CEntityAlive* EA = smart_cast<CEntityAlive*>(O);
	return EA->cast_actor() != 0;
}

BOOL CWeapon::ParentIsActor()
{
	CObject* O = H_Parent();
	if (!O)
		return FALSE;

	CEntityAlive* EA = smart_cast<CEntityAlive*>(O);
	if (!EA)
		return FALSE;

	if (Level().CurrentControlEntity() == H_Parent())
		return EA->cast_actor() != 0;
	else
		return false;
}

#ifdef DEBUG
#include "debug_renderer.h"
void CWeapon::debug_draw_firedeps()
{

	if (hud_adj_mode == 5 || hud_adj_mode == 6 || hud_adj_mode == 7)
	{
		CDebugRenderer& render = Level().debug_renderer();

		if (hud_adj_mode == 5)
			render.draw_aabb(get_LastFP(), 0.005f, 0.005f, 0.005f, color_xrgb(255, 0, 0));

		if (hud_adj_mode == 6)
			render.draw_aabb(get_LastFP2(), 0.005f, 0.005f, 0.005f, color_xrgb(0, 0, 255));

		if (hud_adj_mode == 7)
			render.draw_aabb(get_LastSP(), 0.005f, 0.005f, 0.005f, color_xrgb(0, 255, 0));
	}

}
#endif

void CWeapon::OnBulletHit()
{
	if (!fis_zero(conditionDecreasePerShotOnHit))
		ChangeCondition(-conditionDecreasePerShotOnHit);
}



