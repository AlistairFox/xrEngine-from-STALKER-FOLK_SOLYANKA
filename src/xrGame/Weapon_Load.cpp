#include "StdAfx.h"
#include "Weapon.h"

#include "..\xrEngine\LightAnimLibrary.h"
#include "ui/UIXmlInit.h"
#include "Actor.h"


#define WEAPON_REMOVE_TIME		60000


extern CUIXml* pWpnScopeXml = NULL;

void CWeapon::Load(LPCSTR section)
{
	inherited::Load(section);
	CShootingObject::Load(section);


	if (pSettings->line_exist(section, "flame_particles_2"))
		m_sFlameParticles2 = pSettings->r_string(section, "flame_particles_2");

	// load ammo classes
	m_ammoTypes.clear();
	LPCSTR				S = pSettings->r_string(section, "ammo_class");
	if (S && S[0])
	{
		string128		_ammoItem;
		int				count = _GetItemCount(S);
		for (int it = 0; it < count; ++it)
		{
			_GetItem(S, it, _ammoItem);
			m_ammoTypes.push_back(_ammoItem);
		}
	}

	iAmmoElapsed = pSettings->r_s32(section, "ammo_elapsed");
	iMagazineSize = pSettings->r_s32(section, "ammo_mag_size");

	////////////////////////////////////////////////////
	// дисперсия стрельбы

	//подбрасывание камеры во время отдачи
	u8 rm = READ_IF_EXISTS(pSettings, r_u8, section, "cam_return", 1);
	cam_recoil.ReturnMode = (rm == 1);

	rm = READ_IF_EXISTS(pSettings, r_u8, section, "cam_return_stop", 0);
	cam_recoil.StopReturn = (rm == 1);

	float temp_f = 0.0f;
	temp_f = pSettings->r_float(section, "cam_relax_speed");
	cam_recoil.RelaxSpeed = _abs(deg2rad(temp_f));
	VERIFY(!fis_zero(cam_recoil.RelaxSpeed));
	if (fis_zero(cam_recoil.RelaxSpeed))
	{
		cam_recoil.RelaxSpeed = EPS_L;
	}

	cam_recoil.RelaxSpeed_AI = cam_recoil.RelaxSpeed;
	if (pSettings->line_exist(section, "cam_relax_speed_ai"))
	{
		temp_f = pSettings->r_float(section, "cam_relax_speed_ai");
		cam_recoil.RelaxSpeed_AI = _abs(deg2rad(temp_f));
		VERIFY(!fis_zero(cam_recoil.RelaxSpeed_AI));
		if (fis_zero(cam_recoil.RelaxSpeed_AI))
		{
			cam_recoil.RelaxSpeed_AI = EPS_L;
		}
	}
	temp_f = pSettings->r_float(section, "cam_max_angle");
	cam_recoil.MaxAngleVert = _abs(deg2rad(temp_f));
	VERIFY(!fis_zero(cam_recoil.MaxAngleVert));
	if (fis_zero(cam_recoil.MaxAngleVert))
	{
		cam_recoil.MaxAngleVert = EPS;
	}

	temp_f = pSettings->r_float(section, "cam_max_angle_horz");
	cam_recoil.MaxAngleHorz = _abs(deg2rad(temp_f));
	VERIFY(!fis_zero(cam_recoil.MaxAngleHorz));
	if (fis_zero(cam_recoil.MaxAngleHorz))
	{
		cam_recoil.MaxAngleHorz = EPS;
	}

	temp_f = pSettings->r_float(section, "cam_step_angle_horz");
	cam_recoil.StepAngleHorz = deg2rad(temp_f);

	cam_recoil.DispersionFrac = _abs(READ_IF_EXISTS(pSettings, r_float, section, "cam_dispersion_frac", 0.7f));

	//подбрасывание камеры во время отдачи в режиме zoom ==> ironsight or scope
	//zoom_cam_recoil.Clone( cam_recoil ); ==== нельзя !!!!!!!!!!
	zoom_cam_recoil.RelaxSpeed = cam_recoil.RelaxSpeed;
	zoom_cam_recoil.RelaxSpeed_AI = cam_recoil.RelaxSpeed_AI;
	zoom_cam_recoil.DispersionFrac = cam_recoil.DispersionFrac;
	zoom_cam_recoil.MaxAngleVert = cam_recoil.MaxAngleVert;
	zoom_cam_recoil.MaxAngleHorz = cam_recoil.MaxAngleHorz;
	zoom_cam_recoil.StepAngleHorz = cam_recoil.StepAngleHorz;

	zoom_cam_recoil.ReturnMode = cam_recoil.ReturnMode;
	zoom_cam_recoil.StopReturn = cam_recoil.StopReturn;


	if (pSettings->line_exist(section, "zoom_cam_relax_speed"))
	{
		zoom_cam_recoil.RelaxSpeed = _abs(deg2rad(pSettings->r_float(section, "zoom_cam_relax_speed")));
		VERIFY(!fis_zero(zoom_cam_recoil.RelaxSpeed));
		if (fis_zero(zoom_cam_recoil.RelaxSpeed))
		{
			zoom_cam_recoil.RelaxSpeed = EPS_L;
		}
	}
	if (pSettings->line_exist(section, "zoom_cam_relax_speed_ai"))
	{
		zoom_cam_recoil.RelaxSpeed_AI = _abs(deg2rad(pSettings->r_float(section, "zoom_cam_relax_speed_ai")));
		VERIFY(!fis_zero(zoom_cam_recoil.RelaxSpeed_AI));
		if (fis_zero(zoom_cam_recoil.RelaxSpeed_AI))
		{
			zoom_cam_recoil.RelaxSpeed_AI = EPS_L;
		}
	}
	if (pSettings->line_exist(section, "zoom_cam_max_angle"))
	{
		zoom_cam_recoil.MaxAngleVert = _abs(deg2rad(pSettings->r_float(section, "zoom_cam_max_angle")));
		VERIFY(!fis_zero(zoom_cam_recoil.MaxAngleVert));
		if (fis_zero(zoom_cam_recoil.MaxAngleVert))
		{
			zoom_cam_recoil.MaxAngleVert = EPS;
		}
	}
	if (pSettings->line_exist(section, "zoom_cam_max_angle_horz"))
	{
		zoom_cam_recoil.MaxAngleHorz = _abs(deg2rad(pSettings->r_float(section, "zoom_cam_max_angle_horz")));
		VERIFY(!fis_zero(zoom_cam_recoil.MaxAngleHorz));
		if (fis_zero(zoom_cam_recoil.MaxAngleHorz))
		{
			zoom_cam_recoil.MaxAngleHorz = EPS;
		}
	}
	if (pSettings->line_exist(section, "zoom_cam_step_angle_horz")) {
		zoom_cam_recoil.StepAngleHorz = deg2rad(pSettings->r_float(section, "zoom_cam_step_angle_horz"));
	}
	if (pSettings->line_exist(section, "zoom_cam_dispersion_frac")) {
		zoom_cam_recoil.DispersionFrac = _abs(pSettings->r_float(section, "zoom_cam_dispersion_frac"));
	}

	m_pdm.m_fPDM_disp_base = pSettings->r_float(section, "PDM_disp_base");
	m_pdm.m_fPDM_disp_vel_factor = pSettings->r_float(section, "PDM_disp_vel_factor");
	m_pdm.m_fPDM_disp_accel_factor = pSettings->r_float(section, "PDM_disp_accel_factor");
	m_pdm.m_fPDM_disp_crouch = pSettings->r_float(section, "PDM_disp_crouch");
	m_pdm.m_fPDM_disp_crouch_no_acc = pSettings->r_float(section, "PDM_disp_crouch_no_acc");
	m_crosshair_inertion = READ_IF_EXISTS(pSettings, r_float, section, "crosshair_inertion", 5.91f);
	m_first_bullet_controller.load(section);
	fireDispersionConditionFactor = pSettings->r_float(section, "fire_dispersion_condition_factor");

	// Настройки стрейфа (боковая ходьба)
	const Fvector vZero = { 0.f, 0.f, 0.f };
	Fvector vDefStrafeValue;
	vDefStrafeValue.set(vZero);

	//--> Смещение в стрейфе
	m_strafe_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_hud_offset_pos", vDefStrafeValue);
	m_strafe_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_hud_offset_rot", vDefStrafeValue);

	//--> Поворот в стрейфе
	m_strafe_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_aim_hud_offset_pos", vDefStrafeValue);
	m_strafe_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_aim_hud_offset_rot", vDefStrafeValue);

	// Параметры стрейфа
	bool  bStrafeEnabled = READ_IF_EXISTS(pSettings, r_bool, section, "strafe_enabled", false);
	bool  bStrafeEnabled_aim = READ_IF_EXISTS(pSettings, r_bool, section, "strafe_aim_enabled", false);
	float fFullStrafeTime = READ_IF_EXISTS(pSettings, r_float, section, "strafe_transition_time", 0.01f);
	float fFullStrafeTime_aim = READ_IF_EXISTS(pSettings, r_float, section, "strafe_aim_transition_time", 0.01f);
	float fStrafeCamLFactor = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_limit_factor", 0.5f);
	float fStrafeCamLFactor_aim = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_limit_aim_factor", 1.0f);
	float fStrafeMinAngle = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_min_angle", 0.0f);
	float fStrafeMinAngle_aim = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_aim_min_angle", 7.0f);

	//--> (Data 1)
	m_strafe_offset[2][0].set((bStrafeEnabled ? 1.0f : 0.0f), fFullStrafeTime, NULL);         // normal
	m_strafe_offset[2][1].set((bStrafeEnabled_aim ? 1.0f : 0.0f), fFullStrafeTime_aim, NULL); // aim-GL

	//--> (Data 2)
	m_strafe_offset[3][0].set(fStrafeCamLFactor, fStrafeMinAngle, NULL); // normal
	m_strafe_offset[3][1].set(fStrafeCamLFactor_aim, fStrafeMinAngle_aim, NULL); // aim-GL


	// modified by Peacemaker [17.10.08]
	//	misfireProbability			  = pSettings->r_float(section,"misfire_probability"); 
	//	misfireConditionK			  = READ_IF_EXISTS(pSettings, r_float, section, "misfire_condition_k",	1.0f);
	misfireStartCondition = pSettings->r_float(section, "misfire_start_condition");
	misfireEndCondition = READ_IF_EXISTS(pSettings, r_float, section, "misfire_end_condition", 0.f);
	misfireStartProbability = READ_IF_EXISTS(pSettings, r_float, section, "misfire_start_prob", 0.f);
	misfireEndProbability = pSettings->r_float(section, "misfire_end_prob");
	conditionDecreasePerShot = pSettings->r_float(section, "condition_shot_dec");
	conditionDecreasePerQueueShot = READ_IF_EXISTS(pSettings, r_float, section, "condition_queue_shot_dec", conditionDecreasePerShot);
	conditionDecreasePerShotOnHit = READ_IF_EXISTS(pSettings, r_float, section, "condition_shot_dec_on_hit", 0.f);

	vLoadedFirePoint = pSettings->r_fvector3(section, "fire_point");

	if (pSettings->line_exist(section, "fire_point2"))
		vLoadedFirePoint2 = pSettings->r_fvector3(section, "fire_point2");
	else
		vLoadedFirePoint2 = vLoadedFirePoint;

	// hands
	eHandDependence = EHandDependence(pSettings->r_s32(section, "hand_dependence"));
	m_bIsSingleHanded = true;
	if (pSettings->line_exist(section, "single_handed"))
		m_bIsSingleHanded = !!pSettings->r_bool(section, "single_handed");
	// 
	m_fMinRadius = pSettings->r_float(section, "min_radius");
	m_fMaxRadius = pSettings->r_float(section, "max_radius");


	// информация о возможных апгрейдах и их визуализации в инвентаре
	m_eScopeStatus = (ALife::EWeaponAddonStatus)pSettings->r_s32(section, "scope_status");
	m_eSilencerStatus = (ALife::EWeaponAddonStatus)pSettings->r_s32(section, "silencer_status");
	m_eGrenadeLauncherStatus = (ALife::EWeaponAddonStatus)pSettings->r_s32(section, "grenade_launcher_status");
	m_eLaserDesignatorStatus = (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "laser_designator_status", 0);
	m_eTacticalTorchStatus = (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "tactical_torch_status", 0);

	m_zoom_params.m_bZoomEnabled = !!pSettings->r_bool(section, "zoom_enabled");
	m_zoom_params.m_fZoomRotateTime = pSettings->r_float(section, "zoom_rotate_time");

	bUseAltScope = !!bLoadAltScopesParams(section);

	if (!bUseAltScope)
		LoadOriginalScopesParams(section);

	LoadSilencerParams(section);
	LoadGrenadeLauncherParams(section);
	LoadLaserDesignatorParams(section);
	LoadTacticalTorchParams(section);

	UpdateAltScope();
	InitAddons();
	if (pSettings->line_exist(section, "weapon_remove_time"))
		m_dwWeaponRemoveTime = pSettings->r_u32(section, "weapon_remove_time");
	else
		m_dwWeaponRemoveTime = WEAPON_REMOVE_TIME;

	if (pSettings->line_exist(section, "auto_spawn_ammo"))
		m_bAutoSpawnAmmo = pSettings->r_bool(section, "auto_spawn_ammo");
	else
		m_bAutoSpawnAmmo = TRUE;

	m_zoom_params.m_bHideCrosshairInZoom = true;

	if (pSettings->line_exist(hud_sect, "zoom_hide_crosshair"))
		m_zoom_params.m_bHideCrosshairInZoom = !!pSettings->r_bool(hud_sect, "zoom_hide_crosshair");

	Fvector			def_dof;
	def_dof.set(-1, -1, -1);
	//	m_zoom_params.m_ZoomDof		= READ_IF_EXISTS(pSettings, r_fvector3, section, "zoom_dof", Fvector().set(-1,-1,-1));
	//	m_zoom_params.m_bZoomDofEnabled	= !def_dof.similar(m_zoom_params.m_ZoomDof);

	m_zoom_params.m_ReloadDof = READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_dof", Fvector4().set(-1, -1, -1, -1));

	m_zoom_params.m_ReloadEmptyDof = READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_empty_dof", Fvector4().set(-1, -1, -1, -1));



	m_bHasTracers = !!READ_IF_EXISTS(pSettings, r_bool, section, "tracers", true);
	m_u8TracerColorID = READ_IF_EXISTS(pSettings, r_u8, section, "tracers_color_ID", u8(-1));

	string256						temp;
	for (int i = egdNovice; i < egdCount; ++i)
	{
		strconcat(sizeof(temp), temp, "hit_probability_", get_token_name(difficulty_type_token, i));
		m_hit_probability[i] = READ_IF_EXISTS(pSettings, r_float, section, temp, 1.f);
	}

	if (pSettings->line_exist(section, "silencer_bone"))
		m_sWpn_silencer_bone = pSettings->r_string(section, "silencer_bone");
	else
		m_sWpn_silencer_bone = wpn_silencer_def_bone;

	if (pSettings->line_exist(section, "launcher_bone"))
		m_sWpn_launcher_bone = pSettings->r_string(section, "launcher_bone");
	else
		m_sWpn_launcher_bone = wpn_launcher_def_bone;

	//Кости самих аддонов
	m_sWpn_laser_bone = READ_IF_EXISTS(pSettings, r_string, section, "laser_attach_bone", wpn_laser_def_bone);
	m_sWpn_flashlight_bone = READ_IF_EXISTS(pSettings, r_string, section, "torch_attach_bone", wpn_torch_def_bone);

	//Кости для эффектов
	m_sWpn_laser_ray_bone = READ_IF_EXISTS(pSettings, r_string, section, "laser_ray_bones", "");
	m_sWpn_flashlight_cone_bone = READ_IF_EXISTS(pSettings, r_string, section, "torch_cone_bones", "");
	m_sHud_wpn_laser_ray_bone = READ_IF_EXISTS(pSettings, r_string, hud_sect, "laser_ray_bones", m_sWpn_laser_ray_bone);
	m_sHud_wpn_flashlight_cone_bone = READ_IF_EXISTS(pSettings, r_string, hud_sect, "torch_cone_bones", m_sWpn_flashlight_cone_bone);

	auto LoadBoneNames = [](pcstr section, pcstr line, RStringVec& list)
		{
			list.clear();
			if (pSettings->line_exist(section, line))
			{
				pcstr lineStr = pSettings->r_string(section, line);
				for (int j = 0, cnt = _GetItemCount(lineStr); j < cnt; ++j)
				{
					string128 bone_name;
					_GetItem(lineStr, j, bone_name);
					list.push_back(bone_name);
				}
				return true;
			}
			return false;
		};

	// Default shown bones
	LoadBoneNames(section, "def_show_bones", m_defShownBones);

	// Default hidden bones
	LoadBoneNames(section, "def_hide_bones", m_defHiddenBones);

	if (pSettings->line_exist(section, "bones"))
	{
		pcstr ScopeSect = pSettings->r_string(section, "scope_sect");
		for (int i = 0; i < _GetItemCount(ScopeSect); i++)
		{
			string128 scope;
			_GetItem(ScopeSect, i, scope);
			shared_str bone = pSettings->r_string(scope, "bones");
			m_all_scope_bones.push_back(bone);
		}
	}

	if (!laser_light_render && m_eLaserDesignatorStatus)
	{
		has_laser = true;

		laserdot_attach_bone = READ_IF_EXISTS(pSettings, r_string, section, "laserdot_attach_bone", m_sWpn_laser_bone);
		laserdot_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "laserdot_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_attach_offset_z", 0.0f) };
		laserdot_world_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "laserdot_world_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_world_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_world_attach_offset_z", 0.0f) };

		const bool b_r2 = psDeviceFlags.test(rsR2) || psDeviceFlags.test(rsR4);

		const char* m_light_section = pSettings->r_string(m_sLaserName, "laser_light_section");

		laser_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, m_light_section, "color_animator", ""));

		laser_light_render = ::Render->light_create();
		laser_light_render->set_type(IRender_Light::SPOT);
		laser_light_render->set_shadow(true);

		const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "color_r2" : "color", (Fcolor{ 1.0f, 0.0f, 0.0f, 1.0f }));
		laser_fBrightness = clr.intensity();
		laser_light_render->set_color(clr);
		const float range = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "range_r2" : "range", 100.f);
		laser_light_render->set_range(range);
		laser_light_render->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, m_light_section, "spot_angle", 1.f)));
		laser_light_render->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "spot_texture", nullptr));
	}

	if (!flashlight_render && m_eTacticalTorchStatus)
	{
		has_flashlight = true;

		flashlight_attach_bone = READ_IF_EXISTS(pSettings, r_string, section, "torch_light_bone", m_sWpn_flashlight_bone);
		flashlight_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_attach_offset_z", 0.0f) };
		flashlight_omni_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_attach_offset_z", 0.0f) };
		flashlight_world_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_world_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_world_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_world_attach_offset_z", 0.0f) };
		flashlight_omni_world_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_world_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_world_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_world_attach_offset_z", 0.0f) };

		const bool b_r2 = psDeviceFlags.test(rsR2) || psDeviceFlags.test(rsR4);

		const char* m_light_section = pSettings->r_string(m_sTacticalTorchName, "flashlight_section");

		flashlight_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, m_light_section, "color_animator", ""));

		flashlight_render = ::Render->light_create();
		flashlight_render->set_type(IRender_Light::SPOT);
		flashlight_render->set_shadow(true);

		const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "color_r2" : "color", (Fcolor{ 0.6f, 0.55f, 0.55f, 1.0f }));
		flashlight_fBrightness = clr.intensity();
		flashlight_render->set_color(clr);
		const float range = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "range_r2" : "range", 50.f);
		flashlight_render->set_range(range);
		flashlight_render->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, m_light_section, "spot_angle", 60.f)));
		flashlight_render->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "spot_texture", nullptr));

		flashlight_omni = ::Render->light_create();
		flashlight_omni->set_type((IRender_Light::LT)(READ_IF_EXISTS(pSettings, r_u8, m_light_section, "omni_type", 2))); //KRodin: вообще omni это обычно поинт, но поинт светит во все стороны от себя, поэтому тут спот используется по умолчанию.
		flashlight_omni->set_shadow(false);

		const Fcolor oclr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "omni_color_r2" : "omni_color", (Fcolor{ 1.0f , 1.0f , 1.0f , 0.0f }));
		flashlight_omni->set_color(oclr);
		const float orange = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "omni_range_r2" : "omni_range", 0.25f);
		flashlight_omni->set_range(orange);

		flashlight_glow = ::Render->glow_create();
		flashlight_glow->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "glow_texture", "glow\\glow_torch_r2"));
		flashlight_glow->set_color(clr);
		flashlight_glow->set_radius(READ_IF_EXISTS(pSettings, r_float, m_light_section, "glow_radius", 0.3f));
	}

	hud_recalc_koef = READ_IF_EXISTS(pSettings, r_float, hud_sect, "hud_recalc_koef", 1.35f); //На калаше при 1.35 вроде норм смотрится, другим стволам возможно придется подбирать другие значения.

	m_SuitableRepairKits.clear();
	m_ItemsForRepair.clear();

	LPCSTR repair_kits = READ_IF_EXISTS(pSettings, r_string, section, "suitable_repair_kits", "repair_kit");
	LPCSTR items_for_repair = READ_IF_EXISTS(pSettings, r_string, section, "items_for_repair", "");

	// Added by Axel, to enable optional condition use on any item
	m_flags.set(FUsingCondition, READ_IF_EXISTS(pSettings, r_bool, section, "use_condition", true));

	m_bShowWpnStats = READ_IF_EXISTS(pSettings, r_bool, section, "show_wpn_stats", true);
	m_bEnableBoreDof = READ_IF_EXISTS(pSettings, r_bool, section, "enable_bore_dof", true);
	m_bUseAimAnmDirDependency = READ_IF_EXISTS(pSettings, r_bool, section, "enable_aim_anm_dir_dependency", false);
	m_bUseScopeAimMoveAnims = READ_IF_EXISTS(pSettings, r_bool, section, "enable_scope_aim_move_anm", true);
	m_bUseAimSilShotAnim = READ_IF_EXISTS(pSettings, r_bool, section, "enable_aim_silencer_shoot_anm", false);
	m_bAltZoomEnabled = READ_IF_EXISTS(pSettings, r_bool, section, "enable_alternative_aim", false);

	if (repair_kits && repair_kits[0])
	{
		string128 repair_kits_sect;
		int count = _GetItemCount(repair_kits);
		for (int it = 0; it < count; ++it)
		{
			_GetItem(repair_kits, it, repair_kits_sect);
			m_SuitableRepairKits.push_back(repair_kits_sect);
		}
	}

	if (items_for_repair && items_for_repair[0])
	{
		string128 items_for_repair_sect;
		int count = _GetItemCount(items_for_repair);

		for (int it = 0; it < count; ++it)
		{
			_GetItem(items_for_repair, it, items_for_repair_sect);

			if ((it % 2 != 0 && it != 0) || it == 1)
				m_ItemsForRepair[it / 2].second = std::stoi(items_for_repair_sect);
			else
				m_ItemsForRepair.push_back(std::make_pair(items_for_repair_sect, 0));
		}
	}
}

void CWeapon::LoadFireParams(LPCSTR section)
{
	cam_recoil.Dispersion = deg2rad(pSettings->r_float(section, "cam_dispersion"));
	cam_recoil.DispersionInc = 0.0f;

	if (pSettings->line_exist(section, "cam_dispersion_inc")) {
		cam_recoil.DispersionInc = deg2rad(pSettings->r_float(section, "cam_dispersion_inc"));
	}

	zoom_cam_recoil.Dispersion = cam_recoil.Dispersion;
	zoom_cam_recoil.DispersionInc = cam_recoil.DispersionInc;

	if (pSettings->line_exist(section, "zoom_cam_dispersion")) {
		zoom_cam_recoil.Dispersion = deg2rad(pSettings->r_float(section, "zoom_cam_dispersion"));
	}
	if (pSettings->line_exist(section, "zoom_cam_dispersion_inc")) {
		zoom_cam_recoil.DispersionInc = deg2rad(pSettings->r_float(section, "zoom_cam_dispersion_inc"));
	}

	CShootingObject::LoadFireParams(section);
};

void CWeapon::LoadGrenadeLauncherParams(LPCSTR section)
{
	if (m_eGrenadeLauncherStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "grenade_launcher_attach_sect"))
		{
			m_sGrenadeLauncherAttachSection = pSettings->r_string(section, "grenade_launcher_attach_sect");
			m_sGrenadeLauncherName = pSettings->r_string(m_sGrenadeLauncherAttachSection, "grenade_launcher_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iGrenadeLauncherX = pSettings->r_s32(m_sGrenadeLauncherAttachSection, "grenade_launcher_x") * 2;
				m_iGrenadeLauncherY = pSettings->r_s32(m_sGrenadeLauncherAttachSection, "grenade_launcher_y") * 2;
			}
			else
			{
				m_iGrenadeLauncherX = pSettings->r_s32(m_sGrenadeLauncherAttachSection, "grenade_launcher_x");
				m_iGrenadeLauncherY = pSettings->r_s32(m_sGrenadeLauncherAttachSection, "grenade_launcher_y");
			}
		}
		else
		{
			m_sGrenadeLauncherName = pSettings->r_string(section, "grenade_launcher_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iGrenadeLauncherX = pSettings->r_s32(section, "grenade_launcher_x") * 2;
				m_iGrenadeLauncherY = pSettings->r_s32(section, "grenade_launcher_y") * 2;
			}
			else
			{
				m_iGrenadeLauncherX = pSettings->r_s32(section, "grenade_launcher_x");
				m_iGrenadeLauncherY = pSettings->r_s32(section, "grenade_launcher_y");
			}
		}
	}
	else if (m_eGrenadeLauncherStatus == ALife::eAddonPermanent)
	{
		m_sGrenadeLauncherName = READ_IF_EXISTS(pSettings, r_string, section, "grenade_launcher_name", "");
		m_sGrenadeLauncherAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "grenade_launcher_attach_sect", "");
	}
}

void CWeapon::LoadSilencerParams(LPCSTR section)
{
	if (m_eSilencerStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "silencer_attach_sect"))
		{
			m_sSilencerAttachSection = pSettings->r_string(section, "silencer_attach_sect");
			m_sSilencerName = pSettings->r_string(m_sSilencerAttachSection, "silencer_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iSilencerX = pSettings->r_s32(m_sSilencerAttachSection, "silencer_x") * 2;
				m_iSilencerY = pSettings->r_s32(m_sSilencerAttachSection, "silencer_y") * 2;
			}
			else
			{
				m_iSilencerX = pSettings->r_s32(m_sSilencerAttachSection, "silencer_x");
				m_iSilencerY = pSettings->r_s32(m_sSilencerAttachSection, "silencer_y");
			}
		}
		else
		{
			m_sSilencerName = pSettings->r_string(section, "silencer_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iSilencerX = pSettings->r_s32(section, "silencer_x") * 2;
				m_iSilencerY = pSettings->r_s32(section, "silencer_y") * 2;
			}
			else
			{
				m_iSilencerX = pSettings->r_s32(section, "silencer_x");
				m_iSilencerY = pSettings->r_s32(section, "silencer_y");
			}
		}
	}
	else if (m_eSilencerStatus == ALife::eAddonPermanent)
	{
		m_sSilencerName = READ_IF_EXISTS(pSettings, r_string, section, "silencer_name", "");
		m_sSilencerAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "silencer_attach_sect", "");
	}
}

void CWeapon::LoadLaserDesignatorParams(LPCSTR section)
{
	if (m_eLaserDesignatorStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "laser_designator_attach_sect"))
		{
			m_sLaserAttachSection = pSettings->r_string(section, "laser_designator_attach_sect");
			m_sLaserName = pSettings->r_string(m_sLaserAttachSection, "laser_designator_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iLaserX = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_x") * 2;
				m_iLaserY = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_y") * 2;
			}
			else
			{
				m_iLaserX = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_x");
				m_iLaserY = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_y");
			}
		}
		else
		{
			m_sLaserName = pSettings->r_string(section, "laser_designator_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iLaserX = pSettings->r_s32(section, "laser_designator_x") * 2;
				m_iLaserY = pSettings->r_s32(section, "laser_designator_y") * 2;
			}
			else
			{
				m_iLaserX = pSettings->r_s32(section, "laser_designator_x");
				m_iLaserY = pSettings->r_s32(section, "laser_designator_y");
			}
		}
	}
	else if (m_eLaserDesignatorStatus == ALife::eAddonPermanent)
	{
		m_sLaserName = READ_IF_EXISTS(pSettings, r_string, section, "laser_designator_name", "");
		m_sLaserAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "laser_designator_attach_sect", "");
	}
}

void CWeapon::LoadTacticalTorchParams(LPCSTR section)
{
	if (m_eTacticalTorchStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "tactical_torch_attach_sect"))
		{
			m_sTacticalTorchAttachSection = pSettings->r_string(section, "tactical_torch_attach_sect");
			m_sTacticalTorchName = pSettings->r_string(m_sTacticalTorchAttachSection, "tactical_torch_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iTacticalTorchX = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_x") * 2;
				m_iTacticalTorchY = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_y") * 2;
			}
			else
			{
				m_iTacticalTorchX = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_x");
				m_iTacticalTorchY = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_y");
			}
		}
		else
		{
			m_sTacticalTorchName = pSettings->r_string(section, "tactical_torch_name");

			if (GameConstants::GetUseHQ_Icons())
			{
				m_iTacticalTorchX = pSettings->r_s32(section, "tactical_torch_x") * 2;
				m_iTacticalTorchY = pSettings->r_s32(section, "tactical_torch_y") * 2;
			}
			else
			{
				m_iTacticalTorchX = pSettings->r_s32(section, "tactical_torch_x");
				m_iTacticalTorchY = pSettings->r_s32(section, "tactical_torch_y");
			}
		}
	}
	else if (m_eLaserDesignatorStatus == ALife::eAddonPermanent)
	{
		m_sTacticalTorchName = READ_IF_EXISTS(pSettings, r_string, section, "tactical_torch_name", "");
		m_sTacticalTorchAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "tactical_torch_attach_sect", "");
	}
}

bool CWeapon::bReloadSectionScope(LPCSTR section)
{
	if (!pSettings->line_exist(section, "scopes"))
		return false;

	if (pSettings->r_string(section, "scopes") == NULL)
		return false;

	if (xr_strcmp(pSettings->r_string(section, "scopes"), "none") == 0)
		return false;

	return true;
}

bool CWeapon::bLoadAltScopesParams(LPCSTR section)
{
	if (!pSettings->line_exist(section, "scopes"))
		return false;

	if (pSettings->r_string(section, "scopes") == NULL)
		return false;

	if (xr_strcmp(pSettings->r_string(section, "scopes"), "none") == 0)
		return false;

	if (m_eScopeStatus == ALife::eAddonAttachable)
	{
		LPCSTR str = pSettings->r_string(section, "scopes");
		for (int i = 0, count = _GetItemCount(str); i < count; ++i)
		{
			string128 scope_section;
			_GetItem(str, i, scope_section);
			m_scopes.push_back(scope_section);
		}
	}
	else if (m_eScopeStatus == ALife::eAddonPermanent)
	{
		LoadCurrentScopeParams(section);
	}

	return true;
}

void CWeapon::LoadOriginalScopesParams(LPCSTR section)
{
	if (m_eScopeStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "scopes_sect"))
		{
			LPCSTR str = pSettings->r_string(section, "scopes_sect");
			for (int i = 0, count = _GetItemCount(str); i < count; ++i)
			{
				string128						scope_section;
				_GetItem(str, i, scope_section);
				m_scopes.push_back(scope_section);
			}
		}
		else
		{
			m_scopes.push_back(section);
		}
	}
	else if (m_eScopeStatus == ALife::eAddonPermanent)
	{
		LoadCurrentScopeParams(section);
	}
}

void createWpnScopeXML()
{
	if (!pWpnScopeXml)
	{
		pWpnScopeXml = xr_new<CUIXml>();
		pWpnScopeXml->Load(CONFIG_PATH, UI_PATH, "scopes.xml");
	}
}

void CWeapon::LoadCurrentScopeParams(LPCSTR section)
{
	shared_str scope_tex_name = "none";
	bScopeIsHasTexture = false;
	if (pSettings->line_exist(section, "scope_texture"))
	{
		scope_tex_name = pSettings->r_string(section, "scope_texture");
		if (xr_strcmp(scope_tex_name, "none") != 0)
			bScopeIsHasTexture = true;
	}

	string256 attach_sect;
	strconcat(sizeof(attach_sect), attach_sect, m_eScopeStatus == ALife::eAddonPermanent ? "scope" : m_scopes[m_cur_scope].c_str(), "_attach_sect");

	if (attach_sect && pSettings->line_exist(m_section_id.c_str(), attach_sect))
		m_sScopeAttachSection = READ_IF_EXISTS(pSettings, r_string, m_section_id.c_str(), attach_sect, "");

	m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor");
	Load3DScopeParams(section);

	if (bIsSecondVPZoomPresent())
	{
		bScopeIsHasTexture = false;
		m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "3d_zoom_factor");
	}
	else
	{
		m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor");
	}

	if (bScopeIsHasTexture || bIsSecondVPZoomPresent())
	{
		if (bIsSecondVPZoomPresent())
			bNVsecondVPavaible = !!pSettings->line_exist(section, "scope_nightvision");
		else m_zoom_params.m_sUseZoomPostprocess = READ_IF_EXISTS(pSettings, r_string, section, "scope_nightvision", 0);

		m_zoom_params.m_bUseDynamicZoom = READ_IF_EXISTS(pSettings, r_bool, section, "scope_dynamic_zoom", FALSE);

		if (m_zoom_params.m_bUseDynamicZoom)
		{
			m_fZoomStepCount = READ_IF_EXISTS(pSettings, r_u8, section, "scope_zoom_steps", 3.0f);
			m_fZoomMinKoeff = READ_IF_EXISTS(pSettings, r_u8, section, "min_zoom_k", 0.3f);
		}

		m_zoom_params.m_sUseBinocularVision = READ_IF_EXISTS(pSettings, r_string, section, "scope_alive_detector", 0);
	}
	else
	{
		bNVsecondVPavaible = false;
		bNVsecondVPstatus = false;
	}

	m_fScopeInertionFactor = READ_IF_EXISTS(pSettings, r_float, section, "scope_inertion_factor", m_fControlInertionFactor);

	if (GameConstants::GetOGSE_WpnZoomSystem())
	{
		float delta, min_zoom_factor;
		GetZoomData(m_zoom_params.m_fScopeZoomFactor, delta, min_zoom_factor);

		m_fRTZoomFactor = min_zoom_factor; // set minimal zoom by default for ogse mode
	}
	else
	{
		m_fRTZoomFactor = m_zoom_params.m_fScopeZoomFactor;
	}

	if (m_UIScope)
	{
		xr_delete(m_UIScope);
	}

	if (bScopeIsHasTexture)
	{
		m_UIScope = xr_new<CUIWindow>();
		createWpnScopeXML();
		CUIXmlInit::InitWindow(*pWpnScopeXml, scope_tex_name.c_str(), 0, m_UIScope);
	}
}

void CWeapon::Load3DScopeParams(LPCSTR section)
{
	if (GameConstants::GetUse3dScope())
	{
		m_zoom_params.m_fSecondVPFovFactor = READ_IF_EXISTS(pSettings, r_float, section, "3d_fov", 0.0f);
		m_zoom_params.m_f3dZoomFactor = READ_IF_EXISTS(pSettings, r_float, section, "3d_zoom_factor", 100.0f);
	}
	else
	{
		m_zoom_params.m_fSecondVPFovFactor = 0.0f;
		m_zoom_params.m_f3dZoomFactor = 0.0f;
	}

}

//------------------------------------------------------ RELOAD ----------------------------------------------------------------
//------------------------------------------------------ RELOAD ----------------------------------------------------------------
//------------------------------------------------------ RELOAD ----------------------------------------------------------------
//------------------------------------------------------ RELOAD ----------------------------------------------------------------
//------------------------------------------------------ RELOAD ----------------------------------------------------------------
void CWeapon::reinit()
{
	CShootingObject::reinit();
	CHudItemObject::reinit();
}

void CWeapon::reload(LPCSTR section)
{
	CShootingObject::reload(section);
	CHudItemObject::reload(section);

	m_can_be_strapped = true;
	m_can_be_strapped_rifle = BaseSlot() == INV_SLOT_3;
	m_strapped_mode = false;
	m_strapped_mode_rifle = false;

	bUseAltScope = !!bReloadSectionScope(section);

	if (m_eScopeStatus == ALife::eAddonAttachable) {
		m_addon_holder_range_modifier = READ_IF_EXISTS(pSettings, r_float, GetScopeName(), "holder_range_modifier", m_holder_range_modifier);
		m_addon_holder_fov_modifier = READ_IF_EXISTS(pSettings, r_float, GetScopeName(), "holder_fov_modifier", m_holder_fov_modifier);
	}
	else
	{
		m_addon_holder_range_modifier = m_holder_range_modifier;
		m_addon_holder_fov_modifier = m_holder_fov_modifier;
	}


	{
		Fvector				pos, ypr;
		pos = pSettings->r_fvector3(section, "position");
		ypr = pSettings->r_fvector3(section, "orientation");
		ypr.mul(PI / 180.f);

		m_Offset.setHPB(ypr.x, ypr.y, ypr.z);
		m_Offset.translate_over(pos);
	}

	if (BaseSlot() == INV_SLOT_3) {
		// Strap bones:
		if (pSettings->line_exist(section, "strap_bone0"))
			m_strap_bone0 = pSettings->r_string(section, "strap_bone0");
		else {
			m_strap_bone0 = "bip01_spine2";
		}
		if (pSettings->line_exist(section, "strap_bone1"))
			m_strap_bone1 = pSettings->r_string(section, "strap_bone1");
		else {
			m_strap_bone1 = "bip01_spine1";
		}

		// Right shoulder strap coordinates:
		m_StrapOffset = m_Offset;
		Fvector				pos, ypr;
		if (pSettings->line_exist(section, "strap_position") &&
			pSettings->line_exist(section, "strap_orientation")) {
			pos = pSettings->r_fvector3(section, "strap_position");
			ypr = pSettings->r_fvector3(section, "strap_orientation");
		}
		else {
			pos = Fvector().set(-0.34f, -0.20f, 0.15f);
			ypr = Fvector().set(-0.0f, 0.0f, 84.0f);
		}
		ypr.mul(PI / 180.f);

		m_StrapOffset.setHPB(ypr.x, ypr.y, ypr.z);
		m_StrapOffset.translate_over(pos);

		// Left shoulder strap coordinates:
		m_StrapOffset_alt = m_Offset;
		Fvector pos_alt, ypr_alt;
		if (pSettings->line_exist(section, "strap_position_alt") &&
			pSettings->line_exist(section, "strap_orientation_alt")) {
			pos_alt = pSettings->r_fvector3(section, "strap_position_alt");
			ypr_alt = pSettings->r_fvector3(section, "strap_orientation_alt");
		}
		else {
			pos_alt = Fvector().set(-0.34f, 0.20f, 0.15f);
			ypr_alt = Fvector().set(0.0f, 0.0f, 94.0f);
		}
		ypr_alt.mul(PI / 180.f);
		m_StrapOffset_alt.setHPB(ypr_alt.x, ypr_alt.y, ypr_alt.z);
		m_StrapOffset_alt.translate_over(pos_alt);
	}
	else {
		m_can_be_strapped = false;
		m_can_be_strapped_rifle = false;
	}

	m_ef_main_weapon_type = READ_IF_EXISTS(pSettings, r_u32, section, "ef_main_weapon_type", u32(-1));
	m_ef_weapon_type = READ_IF_EXISTS(pSettings, r_u32, section, "ef_weapon_type", u32(-1));
}

// GETTER FOR SCOPES, SECTIONS HUD

const shared_str CWeapon::GetScopeName() const
{
	if (bUseAltScope)
	{
		return m_scopes[m_cur_scope];
	}
	else
	{
		return pSettings->r_string(m_scopes[m_cur_scope], "scope_name");
	}
}

void CWeapon::UpdateAltScope()
{
	if (m_eScopeStatus != ALife::eAddonAttachable || !bUseAltScope)
		return;

	shared_str sectionNeedLoad;

	sectionNeedLoad = IsScopeAttached() ? GetNameWithAttachment() : m_section_id;

	if (!pSettings->section_exist(sectionNeedLoad))
		return;

	shared_str vis = pSettings->r_string(sectionNeedLoad, "visual");

	if (vis != cNameVisual())
	{
		cNameVisual_set(vis);
	}

	shared_str new_hud = pSettings->r_string(sectionNeedLoad, "hud");
	if (new_hud != hud_sect)
	{
		hud_sect = new_hud;
	}
}

shared_str CWeapon::GetNameWithAttachment()
{
	string64 str;
	if (pSettings->line_exist(m_section_id.c_str(), "parent_section"))
	{
		shared_str parent = pSettings->r_string(m_section_id.c_str(), "parent_section");
		xr_sprintf(str, "%s_%s", parent.c_str(), GetScopeName().c_str());
	}
	else
	{
		xr_sprintf(str, "%s_%s", m_section_id.c_str(), GetScopeName().c_str());
	}
	return (shared_str)str;
}

int CWeapon::GetScopeX()
{
	if (bUseAltScope)
	{
		if (m_eScopeStatus != ALife::eAddonPermanent && IsScopeAttached())
		{
			if (GameConstants::GetUseHQ_Icons())
				return pSettings->r_s32(GetNameWithAttachment(), "scope_x") * 2;
			else
				return pSettings->r_s32(GetNameWithAttachment(), "scope_x");
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (GameConstants::GetUseHQ_Icons())
			return pSettings->r_s32(m_scopes[m_cur_scope], "scope_x") * 2;
		else
			return pSettings->r_s32(m_scopes[m_cur_scope], "scope_x");
	}
}

int CWeapon::GetScopeY()
{
	if (bUseAltScope)
	{
		if (m_eScopeStatus != ALife::eAddonPermanent && IsScopeAttached())
		{
			if (GameConstants::GetUseHQ_Icons())
				return pSettings->r_s32(GetNameWithAttachment(), "scope_y") * 2;
			else
				return pSettings->r_s32(GetNameWithAttachment(), "scope_y");
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (GameConstants::GetUseHQ_Icons())
			return pSettings->r_s32(m_scopes[m_cur_scope], "scope_y") * 2;
		else
			return pSettings->r_s32(m_scopes[m_cur_scope], "scope_y");
	}
}

// CLEAR PARRAMS


void CWeapon::SetDefaults()
{
	SetPending(FALSE);

	m_flags.set(FUsingCondition, TRUE);
	bMisfire = false;
	m_flagsAddOnState = 0;
	m_zoom_params.m_bIsZoomModeNow = false;
}

// ALIFE FUNCTIONS

u32	CWeapon::ef_main_weapon_type() const
{
	VERIFY(m_ef_main_weapon_type != u32(-1));
	return	(m_ef_main_weapon_type);
}

u32	CWeapon::ef_weapon_type() const
{
	VERIFY(m_ef_weapon_type != u32(-1));
	return	(m_ef_weapon_type);
}

// COST CALCULATION
u32 CWeapon::Cost() const
{
	u32 res = CInventoryItem::Cost();
	if (IsGrenadeLauncherAttached() && GetGrenadeLauncherName().size()) {
		res += pSettings->r_u32(GetGrenadeLauncherName(), "cost");
	}
	if (IsScopeAttached() && m_scopes.size()) {
		res += pSettings->r_u32(GetScopeName(), "cost");
	}
	if (IsSilencerAttached() && GetSilencerName().size()) {
		res += pSettings->r_u32(GetSilencerName(), "cost");
	}
	if (IsLaserAttached() && GetLaserName().size()) {
		res += pSettings->r_u32(GetLaserName(), "cost");
	}
	if (IsTacticalTorchAttached() && GetTacticalTorchName().size()) {
		res += pSettings->r_u32(GetTacticalTorchName(), "cost");
	}

	if (iAmmoElapsed)
	{
		float w = pSettings->r_float(m_ammoTypes[m_ammoType].c_str(), "cost");
		float bs = pSettings->r_float(m_ammoTypes[m_ammoType].c_str(), "box_size");

		res += iFloor(w * (iAmmoElapsed / bs));
	}
	return res;
}



// ANIM NAMES
const char* CWeapon::GetAnimAimName()
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (pActor)
	{
		const u32 state = pActor->get_state();

		if (state && state & mcAnyMove)
		{
			if (IsScopeAttached() && m_bUseScopeAimMoveAnims)
				return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_scope_moving"), (IsMisfire()) ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : "");
			else
			{
				if (m_bUseAimAnmDirDependency)
					return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving"), (state & mcFwd) ? "_forward" : ((state & mcBack) ? "_back" : ""), (state & mcLStrafe) ? "_left" : ((state & mcRStrafe) ? "_right" : ""), (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));
				else
					return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving"), (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));
			}
		}
	}
	return nullptr;
}

const char* CWeapon::GenerateAimAnimName(string64 base_anim)
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (pActor)
	{
		const u32 state = pActor->get_state();

		string64 buff;

		if (state & mcAnyMove)
		{
			if (!(state & mcCrouch))
			{
				if (state & mcAccel) //Ходьба медленная (SHIFT)
					return strconcat(sizeof(buff), buff, base_anim, "_slow");
				else
					return base_anim;
			}
			else if (state & mcAccel) //Ходьба в присяде (CTRL+SHIFT)
			{
				return strconcat(sizeof(buff), buff, base_anim, "_crouch_slow");
			}
			else
			{
				return strconcat(sizeof(buff), buff, base_anim, "_crouch");
			}
		}
	}

	return base_anim;
}


const float& CWeapon::hit_probability() const
{
	VERIFY((g_SingleGameDifficulty >= egdNovice) && (g_SingleGameDifficulty <= egdMaster));
	return					(m_hit_probability[egdNovice]);
}
