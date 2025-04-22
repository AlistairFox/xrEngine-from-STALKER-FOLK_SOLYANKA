#include "StdAfx.h"
#include "WeaponMagazined.h"


// LOADING 
//текущая дисперсия (в радианах) оружия с учетом используемого патрона и недисперсионных пуль
float CWeaponMagazined::GetFireDispersion(float cartridge_k, bool for_crosshair)
{
	float fire_disp = GetBaseDispersion(cartridge_k);
	if (for_crosshair || !m_iBaseDispersionedBulletsCount || !m_iShotNum || m_iShotNum > m_iBaseDispersionedBulletsCount)
	{
		fire_disp = inherited::GetFireDispersion(cartridge_k);
	}
	return fire_disp;
}

void CWeaponMagazined::SetQueueSize(int size)
{
	m_iQueueSize = size;
};

void CWeaponMagazined::save(NET_Packet& output_packet)
{
	inherited::save(output_packet);
	save_data(m_iQueueSize, output_packet);
	save_data(m_iShotNum, output_packet);
	save_data(m_iCurFireMode, output_packet);
}

void CWeaponMagazined::load(IReader& input_packet)
{
	inherited::load(input_packet);
	load_data(m_iQueueSize, input_packet); SetQueueSize(m_iQueueSize);
	load_data(m_iShotNum, input_packet);
	load_data(m_iCurFireMode, input_packet);
}


void CWeaponMagazined::Load(LPCSTR section)
{
	inherited::Load(section);

	// Проверяем наличие анимаций
	SetAnimFlag(ANM_SHOW_EMPTY, "anm_show_empty");
	SetAnimFlag(ANM_HIDE_EMPTY, "anm_hide_empty");
	SetAnimFlag(ANM_IDLE_EMPTY, "anm_idle_empty");
	SetAnimFlag(ANM_AIM_EMPTY, "anm_idle_aim_empty");
	SetAnimFlag(ANM_BORE_EMPTY, "anm_bore_empty");
	SetAnimFlag(ANM_SHOT_EMPTY, "anm_shot_l");
	SetAnimFlag(ANM_SPRINT_EMPTY, "anm_idle_sprint_empty");
	SetAnimFlag(ANM_MOVING_EMPTY, "anm_idle_moving_empty");
	SetAnimFlag(ANM_RELOAD_EMPTY, "anm_reload_empty");
	SetAnimFlag(ANM_MISFIRE, "anm_reload_misfire");
	SetAnimFlag(ANM_SHOT_AIM, "anm_shots_when_aim");

	// Sounds
	m_sounds.LoadSound(section, "snd_draw", "sndShow", false, m_eSoundShow);
	m_sounds.LoadSound(section, "snd_holster", "sndHide", false, m_eSoundHide);

	//Alundaio: LAYERED_SND_SHOOT
	m_sounds.LoadSound(section, "snd_shoot", "sndShot", false, m_eSoundShot);

	if (WeaponSoundExist(section, "snd_shoot_actor", true))
		m_sounds.LoadSound(section, "snd_shoot_actor", "sndShotActor", false, m_eSoundShot);
	//-Alundaio

	if (WeaponSoundExist(section, "snd_shoot_last", true))
		m_sounds.LoadSound(section, "snd_shoot_last", "sndShotLast", false, m_eSoundShot);

	if (WeaponSoundExist(section, "snd_silncer_shoot_last", true))
		m_sounds.LoadSound(section, "snd_silncer_shoot_last", "sndSilencerShotLast", false, m_eSoundShot);

	m_sSndShotCurrent = IsSilencerAttached() ? "sndSilencerShot" : "sndShot";

	m_sounds.LoadSound(section, "snd_empty", "sndEmptyClick", false, m_eSoundEmptyClick);
	m_sounds.LoadSound(section, "snd_reload", "sndReload", true, m_eSoundReload);
	m_sounds.LoadSound(section, "snd_reflect", "sndReflect", true, m_eSoundReflect);

	if (WeaponSoundExist(section, "snd_changefiremode", true))
		m_sounds.LoadSound(section, "snd_changefiremode", "sndFireModes", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_laser_on", true))
		m_sounds.LoadSound(section, "snd_laser_on", "sndLaserOn", false, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_laser_off", true))
		m_sounds.LoadSound(section, "snd_laser_off", "sndLaserOff", false, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_torch_on", true))
		m_sounds.LoadSound(section, "snd_torch_on", "sndFlashlightOn", false, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_torch_off", true))
		m_sounds.LoadSound(section, "snd_torch_off", "sndFlashlightOff", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_change_zoom", true))
		m_sounds.LoadSound(section, "snd_change_zoom", "sndChangeZoom", m_eSoundEmptyClick);

	// Звуки из класса пистолета
	if (WeaponSoundExist(section, "snd_close", true))
		m_sounds.LoadSound(section, "snd_close", "sndClose", false, m_eSoundClose);

	if (WeaponSoundExist(section, "snd_reload_empty", true))
		m_sounds.LoadSound(section, "snd_reload_empty", "sndReloadEmpty", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_misfire", true))
		m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_jammed", true))
		m_sounds.LoadSound(section, "snd_reload_jammed", "sndReloadJammed", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_pump_gun", true))
		m_sounds.LoadSound(section, "snd_pump_gun", "sndPumpGun", true, m_eSoundReload);

	//звуки и партиклы глушителя, еслит такой есть
	if (m_eSilencerStatus == ALife::eAddonAttachable || m_eSilencerStatus == ALife::eAddonPermanent)
	{
		if (pSettings->line_exist(section, "silencer_flame_particles"))
			m_sSilencerFlameParticles = pSettings->r_string(section, "silencer_flame_particles");
		if (pSettings->line_exist(section, "silencer_smoke_particles"))
			m_sSilencerSmokeParticles = pSettings->r_string(section, "silencer_smoke_particles");

		m_sounds.LoadSound(section, "snd_silncer_shot", "sndSilencerShot", false, m_eSoundShot);
	}

	m_iBaseDispersionedBulletsCount = READ_IF_EXISTS(pSettings, r_u8, section, "base_dispersioned_bullets_count", 0);
	m_fBaseDispersionedBulletsSpeed = READ_IF_EXISTS(pSettings, r_float, section, "base_dispersioned_bullets_speed", m_fStartBulletSpeed);

	if (pSettings->line_exist(section, "fire_modes"))
	{
		m_bHasDifferentFireModes = true;
		shared_str FireModesList = pSettings->r_string(section, "fire_modes");
		int ModesCount = _GetItemCount(FireModesList.c_str());
		m_aFireModes.clear();

		for (int i = 0; i < ModesCount; i++)
		{
			string16 sItem;
			_GetItem(FireModesList.c_str(), i, sItem);
			m_aFireModes.push_back((s8)atoi(sItem));
		}

		m_iCurFireMode = ModesCount - 1;
		m_iPrefferedFireMode = READ_IF_EXISTS(pSettings, r_s16, section, "preffered_fire_mode", -1);
	}
	else
	{
		m_bHasDifferentFireModes = false;
	}

	LoadSilencerKoeffs();

	m_bUseFiremodeChangeAnim = READ_IF_EXISTS(pSettings, r_bool, section, "use_firemode_change_anim", false);

	if (pSettings->line_exist(section, "bullet_bones"))
	{
		bHasBulletsToHide = true;
		LPCSTR str = pSettings->r_string(section, "bullet_bones");
		for (int i = 0, count = _GetItemCount(str); i < count; ++i)
		{
			string128 bullet_bone_name;
			_GetItem(str, i, bullet_bone_name);
			bullets_bones.push_back(bullet_bone_name);
			bullet_cnt++;
		}

	}
}
