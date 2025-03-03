#include "StdAfx.h"
#include "Weapon.h"

#include "player_hud.h"
#include "entity_alive.h"
#include "Entity.h"

#include "../Include/xrRender/Kinematics.h"
#include "Inventory.h"
#include "InventoryOwner.h"
#include "../xrphysics/mathutils.h"
#include "../xrEngine/LightAnimLibrary.h"

// DESTROY

int		g_iWeaponRemove = 1;

ALife::_TIME_ID	 CWeapon::TimePassedAfterIndependant()	const
{
	if (!H_Parent() && m_dwWeaponIndependencyTime != 0)
		return Level().timeServer() - m_dwWeaponIndependencyTime;
	else
		return 0;
}


bool CWeapon::NeedToDestroyObject()	const
{
	//	if (GameID() == eGameIDSingle) return false;
	//	if (Remote()) return false;
	//	if (H_Parent()) return false;
	//	if (g_iWeaponRemove == -1) return false;
	//	if (g_iWeaponRemove == 0) return true;
	//	if (TimePassedAfterIndependant() > m_dwWeaponRemoveTime)
	//		return true;

	return false;
}

// RENDER XFORM
bool CWeapon::need_renderable()
{
	// Se7kills TODO
	return /*Render->currentViewPort == MAIN_VIEWPORT && */ !(IsZoomed() && ZoomTexture() && !IsRotatingToZoom());
}

void CWeapon::renderable_Render()
{
	UpdateXForm();

	//нарисовать подсветку

	RenderLight();

	//если мы в режиме снайперки, то сам HUD рисовать не надо
	if (IsZoomed() && !IsRotatingToZoom() && ZoomTexture())
		RenderHud(FALSE);
	else
		RenderHud(TRUE);

	inherited::renderable_Render();
}



// UPDATE XFORM

// POSITION
void CWeapon::UpdatePosition(const Fmatrix& trans)
{
	Position().set(trans.c);
	if (m_strapped_mode || m_strapped_mode_rifle)
		XFORM().mul(trans, m_StrapOffset);
	else
		XFORM().mul(trans, m_Offset);

	VERIFY(!fis_zero(DET(renderable.xform)));
}

void CWeapon::UpdatePosition_alt(const Fmatrix& trans) {
	Position().set(trans.c);
	if (m_strapped_mode || m_strapped_mode_rifle)
		XFORM().mul(trans, m_StrapOffset_alt);
	else
		XFORM().mul(trans, m_Offset);

	VERIFY(!fis_zero(DET(renderable.xform)));
}

// UPDATE XFORM

void CWeapon::UpdateAddonsTransform(bool for_hud)
{
	Fmatrix base_model_trans = for_hud ? HudItemData()->m_item_transform : XFORM();
	IRenderVisual* model = for_hud ? HudItemData()->m_model->dcast_RenderVisual() : smart_cast<IRenderVisual*>(Visual());

	if (!for_hud)
	{
		Fmatrix scale, t;
		t = m_scopeAttachTransform;
		scale.scale(0.6666f, 0.6666f, 0.6666f);
		m_scopeAttachTransform.mul(t, scale); // rafa: hack for fucking gunslinger models
	}

	for (auto& mesh : m_weapon_attaches)
		mesh->UpdateAttachesPosition(model, base_model_trans, for_hud);

	if (!for_hud)
	{
		for (auto mesh : m_weapon_attaches)
			mesh->RenderAttach(false);
	}
}


void CWeapon::UpdateXForm()
{
	if (Device.dwFrame == dwXF_Frame)
		return;

	dwXF_Frame = Device.dwFrame;

	if (!GetHUDmode())
		UpdateAddonsTransform(false);

	if (!H_Parent())
		return;

	// Get access to entity and its visual
	CEntityAlive* E = smart_cast<CEntityAlive*>(H_Parent());

	if (!E) {
		if (!IsGameTypeSingle())
		{
			UpdatePosition(H_Parent()->XFORM());
			UpdatePosition_alt(H_Parent()->XFORM());
		}
		return;
	}

	const CInventoryOwner* parent = smart_cast<const CInventoryOwner*>(E);
	if (!parent || (parent && parent->use_simplified_visual()))
		return;

	if (!m_can_be_strapped_rifle) {
		if (parent->attached(this))
			return;
	}

	IKinematics* V = smart_cast<IKinematics*>	(E->Visual());
	VERIFY(V);

	// Get matrices
	int						boneL = -1, boneR = -1, boneR2 = -1;

	// this ugly case is possible in case of a CustomMonster, not a Stalker, nor an Actor
	if ((m_strap_bone0_id == -1 || m_strap_bone1_id == -1) && m_can_be_strapped_rifle)
	{
		m_strap_bone0_id = V->LL_BoneID(m_strap_bone0);
		m_strap_bone1_id = V->LL_BoneID(m_strap_bone1);
	}

	if (parent->inventory().GetActiveSlot() != CurrSlot() && m_can_be_strapped_rifle /* &&
		parent->inventory().InSlot(this)*/) {
		boneR = m_strap_bone0_id;
		boneR2 = m_strap_bone1_id;
		boneL = boneR;

		if (!m_strapped_mode_rifle)
			m_strapped_mode_rifle = true;
	}
	else
	{
		E->g_WeaponBones(boneL, boneR, boneR2);

		if (m_strapped_mode_rifle)
			m_strapped_mode_rifle = false;
	}

	if (boneR == -1)
		return;

#ifdef DEBUG
	static std::unordered_set<std::string> loggedVisuals;
	std::string visualStr = E->cNameVisual().c_str();
#endif

	if (m_strap_bone0_id == BI_NONE)
	{
#ifdef DEBUG
		if (loggedVisuals.find(visualStr) == loggedVisuals.end())
		{
			loggedVisuals.insert(visualStr);
			Msg("! Bone [%s] not found in entity [%s](%s) with visual [%s]!", m_strap_bone0, E->cNameSect().c_str(), E->Name(), E->cNameVisual().c_str());
		}
#endif

		return;
	}
	else if (m_strap_bone1_id == BI_NONE)
	{
#ifdef DEBUG
		if (loggedVisuals.find(visualStr) == loggedVisuals.end())
		{
			loggedVisuals.insert(visualStr);
			Msg("! Bone [%s] not found in entity [%s]([%s]) with visual [%s]!", m_strap_bone1, E->cNameSect().c_str(), E->Name(), E->cNameVisual().c_str());
		}
#endif

		return;
	}

	if ((HandDependence() == hd1Hand) || (GetState() == eReload) || (!E->g_Alive()))
		boneL = boneR2;

	if (getVisible())
	{
		V->CalculateBones_Invalidate();
		V->CalculateBones(true);
	}

	Fmatrix& mL = V->LL_GetTransform(u16(boneL));
	Fmatrix& mR = V->LL_GetTransform(u16(boneR));
	// Calculate
	Fmatrix					mRes;
	Fvector					R, D, N;
	D.sub(mL.c, mR.c);

	if (fis_zero(D.magnitude())) {
		mRes.set(E->XFORM());
		mRes.c.set(mR.c);
	}
	else {
		D.normalize();
		R.crossproduct(mR.j, D);

		N.crossproduct(D, R);
		N.normalize();

		mRes.set(R, N, D, mR.c);
		mRes.mulA_43(E->XFORM());
	}

	if (CurrSlot() == INV_SLOT_3)
		UpdatePosition(mRes);
	else if (CurrSlot() == INV_SLOT_2)
		UpdatePosition_alt(mRes);
}

void CWeapon::UpdateFireDependencies_internal()
{
	if (Device.dwFrame != dwFP_Frame)
	{
		dwFP_Frame = Device.dwFrame;

		UpdateXForm();

		if (GetHUDmode())
		{
			if (H_Parent() != Level().CurrentControlEntity())
			{
				Msg("Strange AFFECTED setup_firedeps.. [H_PARRENT is dont correct]");
			}
			HudItemData()->setup_firedeps(m_current_firedeps);
			VERIFY(_valid(m_current_firedeps.m_FireParticlesXForm));
		}
		else
		{
			// 3rd person or no parent
			Fmatrix& parent = XFORM();

			Fvector& fp = vLoadedFirePoint;
			Fvector& fp2 = vLoadedFirePoint2;
			Fvector& sp = vLoadedShellPoint;

			parent.transform_tiny(m_current_firedeps.vLastFP, fp);
			parent.transform_tiny(m_current_firedeps.vLastFP2, fp2);
			parent.transform_tiny(m_current_firedeps.vLastSP, sp);

			m_current_firedeps.vLastFD.set(0.f, 0.f, 1.f);
			parent.transform_dir(m_current_firedeps.vLastFD);

			m_current_firedeps.m_FireParticlesXForm.set(parent);
			VERIFY(_valid(m_current_firedeps.m_FireParticlesXForm));
		}
	}
}

void CWeapon::ForceUpdateFireParticles()
{
	if (!GetHUDmode())
	{//update particlesXFORM real bullet direction

		if (!H_Parent())
			return;

		Fvector					p, d;
		smart_cast<CEntity*>(H_Parent())->g_fireParams(this, p, d);

		Fmatrix						_pxf;
		_pxf.k = d;
		_pxf.i.crossproduct(Fvector().set(0.0f, 1.0f, 0.0f), _pxf.k);
		_pxf.j.crossproduct(_pxf.k, _pxf.i);
		_pxf.c = XFORM().c;

		m_current_firedeps.m_FireParticlesXForm.set(_pxf);
	}
}

// ADDONS VISABILITY

bool CWeapon::IsGrenadeLauncherAttached() const
{
	return (ALife::eAddonAttachable == m_eGrenadeLauncherStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher)) ||
		ALife::eAddonPermanent == m_eGrenadeLauncherStatus;
}

bool CWeapon::IsScopeAttached() const
{
	return (ALife::eAddonAttachable == m_eScopeStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonScope)) ||
		ALife::eAddonPermanent == m_eScopeStatus;

}

bool CWeapon::IsSilencerAttached() const
{
	return (ALife::eAddonAttachable == m_eSilencerStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer)) ||
		ALife::eAddonPermanent == m_eSilencerStatus;
}

bool CWeapon::IsLaserAttached() const
{
	return (ALife::eAddonAttachable == m_eLaserDesignatorStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator)) ||
		ALife::eAddonPermanent == m_eLaserDesignatorStatus;
}

bool CWeapon::IsTacticalTorchAttached() const
{
	return (ALife::eAddonAttachable == m_eTacticalTorchStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch)) ||
		ALife::eAddonPermanent == m_eTacticalTorchStatus;
}

bool CWeapon::GrenadeLauncherAttachable()
{
	return (ALife::eAddonAttachable == m_eGrenadeLauncherStatus);
}
bool CWeapon::ScopeAttachable()
{
	return (ALife::eAddonAttachable == m_eScopeStatus);
}
bool CWeapon::SilencerAttachable()
{
	return (ALife::eAddonAttachable == m_eSilencerStatus);
}
bool CWeapon::LaserAttachable()
{
	return (ALife::eAddonAttachable == m_eLaserDesignatorStatus);
}
bool CWeapon::TacticalTorchAttachable()
{
	return (ALife::eAddonAttachable == m_eTacticalTorchStatus);
}

void CWeapon::HUD_VisualBulletUpdate(bool force, int force_idx)
{
	if (!bHasBulletsToHide)
		return;

	if (!GetHUDmode()) return;

	bool hide = true;

	if (last_hide_bullet == bullet_cnt || force) hide = false;

	for (u8 b = 0; b < bullet_cnt; b++)
	{
		u16 bone_id = HudItemData()->m_model->LL_BoneID(bullets_bones[b]);

		if (bone_id != BI_NONE)
			HudItemData()->set_bone_visible(bullets_bones[b], !hide);

		if (b == last_hide_bullet) hide = false;
	}
}

void CWeapon::UpdateHUDAddonsVisibility()
{//actor only
	if (!GetHUDmode())										return;

	//.	return;

	u16 bone_id = HudItemData()->m_model->LL_BoneID(wpn_scope_def_bone);

	auto SetBoneVisible = [&](const shared_str& boneName, BOOL visibility)
		{
			HudItemData()->set_bone_visible(boneName, visibility, TRUE);
		};

	// Hide default bones
	for (const shared_str& bone : m_defHiddenBones)
	{
		SetBoneVisible(bone, FALSE);
	}

	// Show default bones
	for (const shared_str& bone : m_defShownBones)
	{
		SetBoneVisible(bone, TRUE);
	}

	for (int i = 0; i < m_all_scope_bones.size(); i++)
		SetBoneVisible(m_all_scope_bones[i], FALSE);

	if (m_cur_scope_bone != NULL)
		SetBoneVisible(m_cur_scope_bone, TRUE);

	if (bone_id != BI_NONE)
	{
		if (ScopeAttachable())
		{
			HudItemData()->set_bone_visible(wpn_scope_def_bone, IsScopeAttached() && !m_sScopeAttachSection.size());
		}

		if (m_eScopeStatus == ALife::eAddonDisabled || m_sScopeAttachSection.size())
		{
			HudItemData()->set_bone_visible(wpn_scope_def_bone, FALSE, TRUE);
		}
		else
			if (m_eScopeStatus == ALife::eAddonPermanent && !m_sScopeAttachSection.size())
				HudItemData()->set_bone_visible(wpn_scope_def_bone, TRUE, TRUE);
	}

	if (SilencerAttachable())
	{
		SetBoneVisible(m_sWpn_silencer_bone, IsSilencerAttached() && !m_sSilencerAttachSection.size());
	}
	if (m_eSilencerStatus == ALife::eAddonDisabled || m_sSilencerAttachSection.size())
	{
		SetBoneVisible(m_sWpn_silencer_bone, FALSE);
	}
	else
		if (m_eSilencerStatus == ALife::eAddonPermanent && !m_sSilencerAttachSection.size())
			SetBoneVisible(m_sWpn_silencer_bone, TRUE);

	if (GrenadeLauncherAttachable())
	{
		SetBoneVisible(m_sWpn_launcher_bone, IsGrenadeLauncherAttached() && !m_sGrenadeLauncherAttachSection.size());
	}
	if (m_eGrenadeLauncherStatus == ALife::eAddonDisabled || m_sGrenadeLauncherAttachSection.size())
	{
		SetBoneVisible(m_sWpn_launcher_bone, FALSE);
	}
	else if (m_eGrenadeLauncherStatus == ALife::eAddonPermanent && !m_sGrenadeLauncherAttachSection.size())
		SetBoneVisible(m_sWpn_launcher_bone, TRUE);

	if (LaserAttachable())
	{
		SetBoneVisible(m_sWpn_laser_bone, IsLaserAttached() && !m_sLaserAttachSection.size());
	}
	if (m_eLaserDesignatorStatus == ALife::eAddonDisabled || m_sLaserAttachSection.size())
	{
		SetBoneVisible(m_sWpn_laser_bone, FALSE);
	}
	else
		if (m_eLaserDesignatorStatus == ALife::eAddonPermanent && !m_sLaserAttachSection.size())
			SetBoneVisible(m_sWpn_laser_bone, TRUE);

	if (m_sHud_wpn_laser_ray_bone.size() && has_laser)
		SetBoneVisible(m_sHud_wpn_laser_ray_bone, IsLaserOn());

	if (TacticalTorchAttachable())
	{
		SetBoneVisible(m_sWpn_flashlight_bone, IsLaserAttached() && !m_sTacticalTorchAttachSection.size());
	}
	if (m_eTacticalTorchStatus == ALife::eAddonDisabled || m_sTacticalTorchAttachSection.size())
	{
		SetBoneVisible(m_sWpn_flashlight_bone, FALSE);
	}
	else
		if (m_eTacticalTorchStatus == ALife::eAddonPermanent && !m_sTacticalTorchAttachSection.size())
			SetBoneVisible(m_sWpn_laser_bone, TRUE);

	if (m_sHud_wpn_flashlight_cone_bone.size() && has_flashlight)
		SetBoneVisible(m_sHud_wpn_flashlight_cone_bone, IsFlashlightOn());
}

void CWeapon::UpdateAddonsVisibility()
{
	IKinematics* pWeaponVisual = smart_cast<IKinematics*>(Visual()); R_ASSERT(pWeaponVisual);

	u16 bone_id;
	UpdateHUDAddonsVisibility();

	pWeaponVisual->CalculateBones_Invalidate();

	bone_id = pWeaponVisual->LL_BoneID(wpn_scope_def_bone);

	auto SetBoneVisible = [&](const shared_str& boneName, BOOL visibility)
		{
			u16 bone_id = pWeaponVisual->LL_BoneID(boneName);
			if (bone_id != BI_NONE && visibility != pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, visibility, TRUE);
		};

	// Hide default bones
	for (const shared_str& bone : m_defHiddenBones)
	{
		SetBoneVisible(bone, FALSE);
	}

	// Show default bones
	for (const shared_str& bone : m_defShownBones)
	{
		SetBoneVisible(bone, TRUE);
	}

	for (int i = 0; i < m_all_scope_bones.size(); i++)
		SetBoneVisible(m_all_scope_bones[i], FALSE);

	if (m_cur_scope_bone != NULL)
		SetBoneVisible(m_cur_scope_bone, TRUE);

	if (ScopeAttachable())
	{
		if (IsScopeAttached() && !m_sScopeAttachSection.size())
		{
			if (bone_id != BI_NONE && !pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eScopeStatus == ALife::eAddonDisabled || m_sScopeAttachSection.size()) && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("scope", pWeaponVisual->LL_GetBoneVisible		(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_silencer_bone);

	if (SilencerAttachable())
	{
		if (IsSilencerAttached() && !m_sSilencerAttachSection.size())
		{
			if (!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id) && bone_id)
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eSilencerStatus == ALife::eAddonDisabled || m_sSilencerAttachSection.size()) && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("silencer", pWeaponVisual->LL_GetBoneVisible	(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_launcher_bone);
	if (GrenadeLauncherAttachable())
	{
		if (IsGrenadeLauncherAttached() && !m_sGrenadeLauncherAttachSection.size())
		{
			if (!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eGrenadeLauncherStatus == ALife::eAddonDisabled || m_sGrenadeLauncherAttachSection.size()) && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("gl", pWeaponVisual->LL_GetBoneVisible			(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_laser_bone);
	if (LaserAttachable())
	{
		if (IsLaserAttached() && !m_sLaserAttachSection.size())
		{
			if (!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eLaserDesignatorStatus == ALife::eAddonDisabled || m_sLaserAttachSection.size()) && bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("laser", pWeaponVisual->LL_GetBoneVisible	(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_flashlight_bone);
	if (TacticalTorchAttachable())
	{
		if (IsTacticalTorchAttached() && !m_sTacticalTorchAttachSection.size())
		{
			if (!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eTacticalTorchStatus == ALife::eAddonDisabled || m_sTacticalTorchAttachSection.size()) && bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("tactical torch", pWeaponVisual->LL_GetBoneVisible	(bone_id));
	}

	if (m_sWpn_laser_ray_bone.size() && has_laser)
	{
		bone_id = pWeaponVisual->LL_BoneID(m_sWpn_laser_ray_bone);

		if (bone_id != BI_NONE)
		{
			const bool laser_on = IsLaserOn();
			if (pWeaponVisual->LL_GetBoneVisible(bone_id) && !laser_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
			else if (!pWeaponVisual->LL_GetBoneVisible(bone_id) && laser_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
	}

	///////////////////////////////////////////////////////////////////

	if (m_sWpn_flashlight_cone_bone.size() && has_flashlight)
	{
		bone_id = pWeaponVisual->LL_BoneID(m_sWpn_flashlight_cone_bone);

		if (bone_id != BI_NONE)
		{
			const bool flashlight_on = IsFlashlightOn();
			if (pWeaponVisual->LL_GetBoneVisible(bone_id) && !flashlight_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
			else if (!pWeaponVisual->LL_GetBoneVisible(bone_id) && flashlight_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
	}

	///////////////////////////////////////////////////////////////////

	pWeaponVisual->CalculateBones_Invalidate();
	pWeaponVisual->CalculateBones(TRUE);
}

void CWeapon::InitAddons()
{
}

// XFORM LIGHTS FOR ATTACHES
void CWeapon::GetBoneOffsetPosDir(const shared_str& bone_name, Fvector& dest_pos, Fvector& dest_dir, const Fvector& offset)
{
	const u16 bone_id = HudItemData()->m_model->LL_BoneID(bone_name);
	//ASSERT_FMT(bone_id != BI_NONE, "!![%s] bone [%s] not found in weapon [%s]", __FUNCTION__, bone_name.c_str(), cNameSect().c_str());
	Fmatrix& fire_mat = HudItemData()->m_model->LL_GetTransform(bone_id);
	fire_mat.transform_tiny(dest_pos, offset);
	HudItemData()->m_item_transform.transform_tiny(dest_pos);
	dest_pos.add(Device.vCameraPosition);
	dest_dir.set(0.f, 0.f, 1.f);
	HudItemData()->m_item_transform.transform_dir(dest_dir);
}

void CWeapon::CorrectDirFromWorldToHud(Fvector& dir)
{
	const auto& CamDir = Device.vCameraDirection;
	const float Fov = Device.fFOV;
	extern ENGINE_API float psHUD_FOV;
	const float HudFov = psHUD_FOV < 1.f ? psHUD_FOV * Device.fFOV : psHUD_FOV;
	const float diff = hud_recalc_koef * Fov / HudFov;
	dir.sub(CamDir);
	dir.mul(diff);
	dir.add(CamDir);
	dir.normalize();
}

void CWeapon::UpdateLaser()
{
	if (laser_light_render)
	{
		auto io = smart_cast<CInventoryOwner*>(H_Parent());
		if (!laser_light_render->get_active() && IsLaserOn() && (!H_Parent() || (io && this == io->inventory().ActiveItem())))
		{
			laser_light_render->set_active(true);
			UpdateAddonsVisibility();
		}
		else if (laser_light_render->get_active() && (!IsLaserOn() || !(!H_Parent() || (io && this == io->inventory().ActiveItem()))))
		{
			laser_light_render->set_active(false);
			UpdateAddonsVisibility();
		}

		if (laser_light_render->get_active())
		{
			Fvector laser_pos = get_LastFP(), laser_dir = get_LastFD();

			if (GetHUDmode())
			{
				if (laserdot_attach_bone.size())
				{
					GetBoneOffsetPosDir(laserdot_attach_bone, laser_pos, laser_dir, laserdot_attach_offset);
					CorrectDirFromWorldToHud(laser_dir);
				}
			}
			else
			{
				XFORM().transform_tiny(laser_pos, laserdot_world_attach_offset);
			}

			Fmatrix laserXForm;
			laserXForm.identity();
			laserXForm.k.set(laser_dir);
			Fvector::generate_orthonormal_basis_normalized(laserXForm.k, laserXForm.j, laserXForm.i);

			laser_light_render->set_position(laser_pos);
			laser_light_render->set_rotation(laserXForm.k, laserXForm.i);

			// calc color animator
			if (laser_lanim)
			{
				int frame;
				const u32 clr = laser_lanim->CalculateBGR(Device.fTimeGlobal, frame);

				Fcolor fclr{ (float)color_get_B(clr), (float)color_get_G(clr), (float)color_get_R(clr), 1.f };
				fclr.mul_rgb(laser_fBrightness / 255.f);
				laser_light_render->set_color(fclr);
			}
		}
	}
}

void CWeapon::UpdateFlashlight()
{
	if (flashlight_render)
	{
		auto io = smart_cast<CInventoryOwner*>(H_Parent());
		if (!flashlight_render->get_active() && IsFlashlightOn() && (!H_Parent() || (io && this == io->inventory().ActiveItem())))
		{
			flashlight_render->set_active(true);
			flashlight_omni->set_active(true);
			flashlight_glow->set_active(true);
			UpdateAddonsVisibility();
		}
		else if (flashlight_render->get_active() && (!IsFlashlightOn() || !(!H_Parent() || (io && this == io->inventory().ActiveItem()))))
		{
			flashlight_render->set_active(false);
			flashlight_omni->set_active(false);
			flashlight_glow->set_active(false);
			UpdateAddonsVisibility();
		}

		if (flashlight_render->get_active())
		{
			Fvector flashlight_pos, flashlight_pos_omni, flashlight_dir, flashlight_dir_omni;

			if (GetHUDmode())
			{
				GetBoneOffsetPosDir(flashlight_attach_bone, flashlight_pos, flashlight_dir, flashlight_attach_offset);
				CorrectDirFromWorldToHud(flashlight_dir);

				GetBoneOffsetPosDir(flashlight_attach_bone, flashlight_pos_omni, flashlight_dir_omni, flashlight_omni_attach_offset);
				CorrectDirFromWorldToHud(flashlight_dir_omni);
			}
			else
			{
				flashlight_dir = get_LastFD();
				XFORM().transform_tiny(flashlight_pos, flashlight_world_attach_offset);

				flashlight_dir_omni = get_LastFD();
				XFORM().transform_tiny(flashlight_pos_omni, flashlight_omni_world_attach_offset);
			}

			Fmatrix flashlightXForm;
			flashlightXForm.identity();
			flashlightXForm.k.set(flashlight_dir);
			Fvector::generate_orthonormal_basis_normalized(flashlightXForm.k, flashlightXForm.j, flashlightXForm.i);
			flashlight_render->set_position(flashlight_pos);
			flashlight_render->set_rotation(flashlightXForm.k, flashlightXForm.i);

			flashlight_glow->set_position(flashlight_pos);
			flashlight_glow->set_direction(flashlightXForm.k);

			Fmatrix flashlightomniXForm;
			flashlightomniXForm.identity();
			flashlightomniXForm.k.set(flashlight_dir_omni);
			Fvector::generate_orthonormal_basis_normalized(flashlightomniXForm.k, flashlightomniXForm.j, flashlightomniXForm.i);
			flashlight_omni->set_position(flashlight_pos_omni);
			flashlight_omni->set_rotation(flashlightomniXForm.k, flashlightomniXForm.i);

			// calc color animator
			if (flashlight_lanim)
			{
				int frame;
				const u32 clr = flashlight_lanim->CalculateBGR(Device.fTimeGlobal, frame);

				Fcolor fclr{ (float)color_get_B(clr), (float)color_get_G(clr), (float)color_get_R(clr), 1.f };
				fclr.mul_rgb(flashlight_fBrightness / 255.f);
				flashlight_render->set_color(fclr);
				flashlight_omni->set_color(fclr);
				flashlight_glow->set_color(fclr);
			}
		}
	}
}
