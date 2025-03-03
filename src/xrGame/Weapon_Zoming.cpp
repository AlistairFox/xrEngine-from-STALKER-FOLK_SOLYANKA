#include "StdAfx.h"
#include "Weapon.h"
#include "GamePersistent.h"
#include "player_hud.h"

#include "Actor.h"
// #include "ActorNightVision.h"
#include "Inventory.h"
#include "ui/UIWindow.h"
#include "WeaponBinocularsVision.h"

// ZOOMS 
// Lex Addon (correct by Suhar_) 24.10.2018		(begin)
float LastZoomFactor = 0.0f;

bool CWeapon::render_item_ui_query()
{
	bool b_is_active_item = (m_pInventory->ActiveItem() == this);
	bool res = b_is_active_item && IsZoomed() && ZoomHideCrosshair() && ZoomTexture() && !IsRotatingToZoom();
	return res;
}

void CWeapon::render_item_ui()
{
	if (m_zoom_params.m_pVision)
		m_zoom_params.m_pVision->Draw();

	ZoomTexture()->Update();
	ZoomTexture()->Draw();
}

bool CWeapon::show_crosshair()
{
	return !IsPending() && (!IsZoomed() || !ZoomHideCrosshair());
}

bool CWeapon::show_indicators()
{
	return !(IsZoomed() && ZoomTexture());
}

extern u32 hud_adj_mode;

bool CWeapon::ZoomHideCrosshair()
{
	if (hud_adj_mode != 0)
		return false;

	return m_zoom_params.m_bHideCrosshairInZoom || ZoomTexture();
}


float CWeapon::CurrentZoomFactor()
{
	if (GameConstants::GetUse3dScope() && IsScopeAttached())
	{
		return GameConstants::GetOGSE_WpnZoomSystem() ? 1.0f : bIsSecondVPZoomPresent() ? m_zoom_params.m_f3dZoomFactor : m_zoom_params.m_fScopeZoomFactor; // no change to main fov zoom when use second vp
	}
	else
	{
		return (IsScopeAttached() && !m_bAltZoomActive) ? m_zoom_params.m_fScopeZoomFactor : m_zoom_params.m_fIronSightZoomFactor;
	}
};

void CWeapon::GetZoomData(const float scope_factor, float& delta, float& min_zoom_factor)
{
	float def_fov = GameConstants::GetOGSE_WpnZoomSystem() ? 1.f : bIsSecondVPZoomPresent() ? 75.0f : g_fov;
	float delta_factor_total = def_fov - scope_factor;
	VERIFY(delta_factor_total > 0);
	min_zoom_factor = def_fov - delta_factor_total * m_fZoomMinKoeff;
	delta = (delta_factor_total * (1 - m_fZoomMinKoeff)) / m_fZoomStepCount;
}

void CWeapon::OnZoomIn()
{
	//Alun: Force switch to first-person for zooming
	CActor* pA = smart_cast<CActor*>(H_Parent());
	if (pA && pA->active_cam() == eacLookAt)
	{
		pA->cam_Set(eacFirstEye);
		m_freelook_switch_back = true;
	}

	m_zoom_params.m_bIsZoomModeNow = true;
	if (bIsSecondVPZoomPresent() && m_zoom_params.m_bUseDynamicZoom && IsScopeAttached())
	{
		if (LastZoomFactor)
			m_fRTZoomFactor = LastZoomFactor;
		else
			m_fRTZoomFactor = CurrentZoomFactor();
		float delta, min_zoom_factor;
		GetZoomData(m_zoom_params.m_fScopeZoomFactor, delta, min_zoom_factor);
		clamp(m_fRTZoomFactor, m_zoom_params.m_fScopeZoomFactor, min_zoom_factor);

		SetZoomFactor(m_fRTZoomFactor);
	}
	else
		m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();

	if (GetHUDmode())
		GamePersistent().SetPickableEffectorDOF(true);

	if (m_zoom_params.m_sUseBinocularVision.size() && IsScopeAttached() && NULL == m_zoom_params.m_pVision)
		m_zoom_params.m_pVision = xr_new<CBinocularsVision>(m_zoom_params.m_sUseBinocularVision);

	if (pA && IsScopeAttached())
	{
		// if (GameConstants::GetPNV_W_SCOPE_DIS() && bIsSecondVPZoomPresent())
		// {
		// 		if (pA->GetNightVisionStatus())
		// 		{
		// 			OnZoomOut();
		// 		}
		// }
		// else 
		if (m_zoom_params.m_sUseZoomPostprocess.size())
		{
			if (NULL == m_zoom_params.m_pNight_vision)
			{
				m_zoom_params.m_pNight_vision = xr_new<CNightVisionEffector>(m_zoom_params.m_sUseZoomPostprocess);
			}
		}
	}
	g_player_hud->updateMovementLayerState();
}

void CWeapon::OnZoomOut()
{
	//Alun: Switch back to third-person if was forced
	if (m_freelook_switch_back)
	{
		CActor* pA = smart_cast<CActor*>(H_Parent());
		if (pA)
			pA->cam_Set(eacLookAt);
		m_freelook_switch_back = false;
	}

	if (!bIsSecondVPZoomPresent() || !GameConstants::GetUse3dScope())
		m_fRTZoomFactor = GetZoomFactor(); //  
	m_zoom_params.m_bIsZoomModeNow = false;
	SetZoomFactor(GameConstants::GetOGSE_WpnZoomSystem() ? 1.f : g_fov);
 
	if (GetHUDmode())
		GamePersistent().SetPickableEffectorDOF(false);

	ResetSubStateTime();

	// xr_delete(m_zoom_params.m_pVision);
	// if (m_zoom_params.m_pNight_vision)
	// {
	// 	m_zoom_params.m_pNight_vision->StopForScope(100000.0f, false);
	// 	xr_delete(m_zoom_params.m_pNight_vision);
	// }
	g_player_hud->updateMovementLayerState();
}

CUIWindow* CWeapon::ZoomTexture()
{
	if (UseScopeTexture() && !bIsSecondVPZoomPresent() && !m_bAltZoomActive)
		return m_UIScope;
	else
		return NULL;
}


// NEXT ZOOM 

void CWeapon::ZoomDynamicMod(bool bIncrement, bool bForceLimit)
{
	if (!IsScopeAttached())
		return;

	if (!m_zoom_params.m_bUseDynamicZoom)
		return;

	float delta, min_zoom_factor, max_zoom_factor;

	max_zoom_factor = (bIsSecondVPZoomPresent() ? GetSecondVPZoomFactor() * 100.0f : m_zoom_params.m_fScopeZoomFactor);

	GetZoomData(max_zoom_factor, delta, min_zoom_factor);

	if (bForceLimit)
	{
		m_fRTZoomFactor = (bIncrement ? max_zoom_factor : min_zoom_factor);
	}
	else
	{
		float f = (bIsSecondVPZoomPresent() ? m_fRTZoomFactor : GetZoomFactor());

		f -= delta * (bIncrement ? 1.f : -1.f);

		clamp(f, max_zoom_factor, min_zoom_factor);


		if (bIsSecondVPZoomPresent())
			m_fRTZoomFactor = f;
		else
			SetZoomFactor(f);

		// Lex Addon (correct by Suhar_) 24.10.2018		(begin)  
		LastZoomFactor = f;
		// Lex Addon (correct by Suhar_) 24.10.2018		(end)
	}

	if (m_sounds.FindSoundItem("sndChangeZoom", false) && LastZoomFactor != min_zoom_factor && LastZoomFactor != max_zoom_factor)
		PlaySound("sndChangeZoom", get_LastFP());
}

void CWeapon::ZoomInc()
{
	ZoomDynamicMod(true, false);
}

void CWeapon::ZoomDec()
{
	ZoomDynamicMod(false, false);
}



// SECONDARY VP

//  FOV       
float CWeapon::GetSecondVPFov() const
{
	if (m_zoom_params.m_bUseDynamicZoom && bIsSecondVPZoomPresent())
		return (m_fRTZoomFactor / 100.f) * 75.f;	//g_fov; 75.f

	return GetSecondVPZoomFactor() * 75.f;			//g_fov; 75.f
}



//      +SecondVP+
//      
void CWeapon::UpdateSecondVP(bool bInGrenade)
{
	bool b_is_active_item = (m_pInventory != NULL) && (m_pInventory->ActiveItem() == this);
	R_ASSERT(ParentIsActor() && b_is_active_item); //           

	CActor* pActor = smart_cast<CActor*>(H_Parent());

	bool bCond_1 = bInZoomRightNow();		// Мы должны целиться  

	bool bCond_2 = bIsSecondVPZoomPresent();						// В конфиге должен быть прописан фактор зума для линзы (scope_lense_factor
	// больше чем 0)
	bool bCond_3 = pActor->cam_Active() == pActor->cam_FirstEye();	// Мы должны быть от 1-го лица	

	if (GameConstants::GetUse3dScope)
	{
		// TODO add 3d scope
		//Device.m_SecondViewport.SetSVPActive(bCond_1 && bCond_2 && bCond_3 && !bInGrenade);
	}
}


bool CWeapon::bChangeNVSecondVPStatus()
{
	if (!bNVsecondVPavaible || !IsZoomed())
		return false;

	bNVsecondVPstatus = !bNVsecondVPstatus;

	return true;
}




