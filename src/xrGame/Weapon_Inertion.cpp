#include "StdAfx.h"
#include "Weapon.h"
#include "Actor.h"
#include "player_hud.h"
#include "../xrEngine/GameMtlLib.h"
// #include "GameMtlLib.h"

BOOL	b_hud_collision = FALSE;

// CURRENT HUD OFFSET IDX !!!! FOR AIMING
u8 CWeapon::GetCurrentHudOffsetIdx()
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor)
		return 0;

	if (pActor->MpSafeMode())
		return 4;

	bool b_aiming = ((IsZoomed() && m_zoom_params.m_fZoomRotationFactor <= 1.f) || (!IsZoomed() && m_zoom_params.m_fZoomRotationFactor > 0.f));

	if (!b_aiming)
		return 0;
	if (!m_bAltZoomActive)
		return 1;
	else
		return 3;
}

static BOOL pick_trace_callback(collide::rq_result& result, LPVOID params)
{
	collide::rq_result* RQ = (collide::rq_result*)params;
	if (!result.O)
	{
		// получить треугольник и узнать его материал
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
		if (T->material < GMLib.CountMaterial())
		{
			if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable) || GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flActorObstacle))
				return TRUE;
		}
	}
	*RQ = result;
	return FALSE;
}

static float GetRayQueryDist()
{
	collide::rq_result RQ;
	g_pGameLevel->ObjectSpace.RayPick(Device.vCameraPosition, Device.vCameraDirection, 3.0f, collide::rqtStatic, RQ, Actor());
	if (!RQ.O)
	{
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + RQ.element;
		if (T->material < GMLib.CountMaterial())
		{
			collide::rq_result  RQ2;
			collide::rq_results RQR;
			RQ2.range = 3.0f;
			collide::ray_defs RD(Device.vCameraPosition, Device.vCameraDirection, RQ2.range, CDB::OPT_CULL, collide::rqtStatic);
			if (Level().ObjectSpace.RayQuery(RQR, RD, pick_trace_callback, &RQ2, NULL, Level().CurrentEntity()))
			{
				clamp(RQ2.range, RQ.range, RQ2.range);
				return RQ2.range;
			}
		}
	}
	return RQ.range;
}

float CWeapon::GetControlInertionFactor() const
{
	float fInertionFactor = inherited::GetControlInertionFactor();
	if (IsScopeAttached() && IsZoomed())
		return m_fScopeInertionFactor;

	return fInertionFactor;
}

void _inertion(float& _val_cur, const float& _val_trgt, const float& _friction)
{
	float friction_i = 1.f - _friction;
	_val_cur = _val_cur * _friction + _val_trgt * friction_i;
}

float _lerp(const float& _val_a, const float& _val_b, const float& _factor)
{
	return (_val_a * (1.0 - _factor)) + (_val_b * _factor);
}

float m_fSafeModeRotateTime = 0.3f;
void CWeapon::UpdateHudAdditional(Fmatrix& trans)
{
	if (!ParentIsActor())
		return;

	CActor* pActor = smart_cast<CActor*>(H_Parent());

	attachable_hud_item* hi = HudItemData();
	R_ASSERT(hi);

	u8 idx = GetCurrentHudOffsetIdx();

	//============= Поворот ствола во время аима =============//

	// if ((IsZoomed() && m_zoom_params.m_fZoomRotationFactor <= 1.f) || (!IsZoomed() && m_zoom_params.m_fZoomRotationFactor > 0.f))
	
	{
		// Оригенальный curr_rot
		Fvector						curr_offs, curr_rot;
		curr_offs = hi->m_measures.m_hands_offset[0][idx];//pos,aim
		curr_rot = hi->m_measures.m_hands_offset[1][idx];//rot,aim
		curr_offs.mul(m_zoom_params.m_fZoomRotationFactor);
		curr_rot.mul(m_zoom_params.m_fZoomRotationFactor);
		float factor;

		// SAFE MODE ROTATION
		if (idx == 4 || last_idx == 4)
			factor = Device.fTimeDelta / m_fSafeModeRotateTime; //
		else
			factor = Device.fTimeDelta / m_zoom_params.m_fZoomRotateTime;


		if (curr_offs.similar(m_hud_offset[0], EPS))
		{
			m_hud_offset[0].set(curr_offs);
		}
		else
		{
			Fvector diff;
			diff.set(curr_offs);
			diff.sub(m_hud_offset[0]);
			diff.mul(factor * 2.5f);
			m_hud_offset[0].add(diff);
		}

		if (curr_rot.similar(m_hud_offset[1], EPS))
		{
			m_hud_offset[1].set(curr_rot);
		}
		else
		{
			Fvector diff;
			diff.set(curr_rot);
			diff.sub(m_hud_offset[1]);
			diff.mul(factor * 2.5f);
			m_hud_offset[1].add(diff);
		}

		//if (idx == 4 || last_idx == 4)
		//	Msg("Safemode ROT: {%f, %f, %f} POS {%f, %f, %f}",
		//		VPUSH(m_hud_offset[1]), VPUSH(m_hud_offset[0]));

		// Remove pending state before weapon has fully moved to the new position to remove some delay
		if (curr_offs.similar(m_hud_offset[0], .02f) && curr_rot.similar(m_hud_offset[1], .02f))
		{
			if ((idx == 4 || last_idx == 4) && IsPending())
				SetPending(FALSE);
			last_idx = idx;
		}

		// Оригенальный код разворота
		Fmatrix						hud_rotation;
		hud_rotation.identity();
		hud_rotation.rotateX( /* curr_rot.x */ m_hud_offset[1].x);

		Fmatrix						hud_rotation_y;
		hud_rotation_y.identity();
		hud_rotation_y.rotateY(/* curr_rot.y*/ m_hud_offset[1].y);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation_y.identity();
		hud_rotation_y.rotateZ( /*curr_rot.z*/ m_hud_offset[1].z);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation.translate_over( /*curr_offs*/ m_hud_offset[0]);
		trans.mulB_43(hud_rotation);

		if (pActor->IsZoomAimingMode() || idx == 4)
			m_zoom_params.m_fZoomRotationFactor += factor; //Device.fTimeDelta / m_zoom_params.m_fZoomRotateTime;
		else
			m_zoom_params.m_fZoomRotationFactor -= factor; //-= Device.fTimeDelta / m_zoom_params.m_fZoomRotateTime;

		clamp(m_zoom_params.m_fZoomRotationFactor, 0.f, 1.f);
	}


	//============= Коллизия оружия =============//
	if (idx != 4) // SafeMode
	{
		float dist = GetRayQueryDist();

		Fvector curr_offs, curr_rot;
		curr_offs = hi->m_measures.m_collision_offset[0];//pos,aim
		curr_rot = hi->m_measures.m_collision_offset[1];//rot,aim
		curr_offs.mul(m_fCollisionFactor);
		curr_rot.mul(m_fCollisionFactor);

		float m_fColPosition;
		float m_fColRotation;

		if (dist <= 0.8 && !IsZoomed())
		{
			m_fColPosition = curr_offs.y + ((1 - dist - 0.2) * 5.0f);
			m_fColRotation = curr_rot.x + ((1 - dist - 0.2) * 5.0f);
		}
		else
		{
			m_fColPosition = curr_offs.y;
			m_fColRotation = curr_rot.x;
		}

		if (m_fCollisionFactor < m_fColPosition)
		{
			m_fCollisionFactor += Device.fTimeDelta / 0.3;
			if (m_fCollisionFactor > m_fColPosition)
				m_fCollisionFactor = m_fColPosition;
		}
		else if (m_fCollisionFactor > m_fColPosition)
		{
			m_fCollisionFactor -= Device.fTimeDelta / 0.3;
			if (m_fCollisionFactor < m_fColPosition)
				m_fCollisionFactor = m_fColPosition;
		}

		Fmatrix hud_rotation;
		hud_rotation.identity();
		hud_rotation.rotateX(curr_rot.x);

		Fmatrix hud_rotation_y;
		hud_rotation_y.identity();
		hud_rotation_y.rotateY(curr_rot.y);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation_y.identity();
		hud_rotation_y.rotateZ(curr_rot.z);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation.translate_over(curr_offs);
		trans.mulB_43(hud_rotation);

		clamp(m_fCollisionFactor, 0.f, 1.f);
	}
	else
	{
		m_fCollisionFactor = 0.0;
	}


	//============= Подготавливаем общие переменные =============//

	clamp(idx, u8(0), u8(1));
	bool bForAim = (idx == 1);

	float fInertiaPower = GetInertionPowerFactor();

	float fYMag = pActor->fFPCamYawMagnitude;
	float fPMag = pActor->fFPCamPitchMagnitude;

	static float fAvgTimeDelta = Device.fTimeDelta;
	_inertion(fAvgTimeDelta, Device.fTimeDelta, 0.8f);

	//======== Проверяем доступность инерции и стрейфа ========//
	if (!g_player_hud->inertion_allowed())
		return;

	//============= Боковой стрейф с оружием =============//
	float fStrafeMaxTime = m_strafe_offset[2][idx].y; // Макс. время в секундах, за которое мы наклонимся из центрального положения
	if (fStrafeMaxTime <= EPS)
		fStrafeMaxTime = 0.01f;

	float fStepPerUpd = fAvgTimeDelta / fStrafeMaxTime; // Величина изменение фактора поворота

	// Добавляем боковой наклон от движения камеры
	float fCamReturnSpeedMod = 1.5f; // Восколько ускоряем нормализацию наклона, полученного от движения камеры (только от бедра)
	// Высчитываем минимальную скорость поворота камеры для начала инерции
	float fStrafeMinAngle = _lerp(
		m_strafe_offset[3][0].y,
		m_strafe_offset[3][1].y,
		m_zoom_params.m_fZoomRotationFactor);

	// Высчитываем мксимальный наклон от поворота камеры
	float fCamLimitBlend = _lerp(
		m_strafe_offset[3][0].x,
		m_strafe_offset[3][1].x,
		m_zoom_params.m_fZoomRotationFactor);

	// Считаем стрейф от поворота камеры
	if (abs(fYMag) > (m_fLR_CameraFactor == 0.0f ? fStrafeMinAngle : 0.0f))
	{ //--> Камера крутится по оси Y
		m_fLR_CameraFactor -= (fYMag * 0.025f);

		clamp(m_fLR_CameraFactor, -fCamLimitBlend, fCamLimitBlend);
	}
	else
	{ //--> Камера не поворачивается - убираем наклон
		if (m_fLR_CameraFactor < 0.0f)
		{
			m_fLR_CameraFactor += fStepPerUpd * (bForAim ? 1.0f : fCamReturnSpeedMod);
			clamp(m_fLR_CameraFactor, -fCamLimitBlend, 0.0f);
		}
		else
		{
			m_fLR_CameraFactor -= fStepPerUpd * (bForAim ? 1.0f : fCamReturnSpeedMod);
			clamp(m_fLR_CameraFactor, 0.0f, fCamLimitBlend);
		}
	}
	// Добавляем боковой наклон от ходьбы вбок
	float fChangeDirSpeedMod = 3; // Восколько быстро меняем направление направление наклона, если оно в другую сторону от текущего

	u32 iMovingState = pActor->MovingState();
	if ((iMovingState & mcLStrafe) != 0)
	{ // Движемся влево
		float fVal = (m_fLR_MovingFactor > 0.f ? fStepPerUpd * fChangeDirSpeedMod : fStepPerUpd);
		m_fLR_MovingFactor -= fVal;
	}
	else if ((iMovingState & mcRStrafe) != 0)
	{ // Движемся вправо
		float fVal = (m_fLR_MovingFactor < 0.f ? fStepPerUpd * fChangeDirSpeedMod : fStepPerUpd);
		m_fLR_MovingFactor += fVal;
	}
	else
	{ // Двигаемся в любом другом направлении - плавно убираем наклон
		if (m_fLR_MovingFactor < 0.0f)
		{
			m_fLR_MovingFactor += fStepPerUpd;
			clamp(m_fLR_MovingFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fLR_MovingFactor -= fStepPerUpd;
			clamp(m_fLR_MovingFactor, 0.0f, 1.0f);
		}
	}

	clamp(m_fLR_MovingFactor, -1.0f, 1.0f); // Фактор боковой ходьбы не должен превышать эти лимиты

	// Вычисляем и нормализируем итоговый фактор наклона
	float fLR_Factor = m_fLR_MovingFactor + (m_fLR_CameraFactor * fInertiaPower);
	clamp(fLR_Factor, -1.0f, 1.0f); // Фактор боковой ходьбы не должен превышать эти лимиты

	// Производим наклон ствола для нормального режима и аима
	for (int _idx = 0; _idx <= 1; _idx++)//<-- Для плавного перехода
	{
		bool bEnabled = (m_strafe_offset[2][_idx].x != 0.0f);
		if (!bEnabled)
			continue;

		Fvector curr_offs, curr_rot;

		// Смещение позиции худа в стрейфе
		curr_offs = m_strafe_offset[0][_idx]; //pos
		curr_offs.mul(fLR_Factor);                   // Умножаем на фактор стрейфа

		// Поворот худа в стрейфе
		curr_rot = m_strafe_offset[1][_idx]; //rot
		curr_rot.mul(-PI / 180.f);                          // Преобразуем углы в радианы
		curr_rot.mul(fLR_Factor);                   // Умножаем на фактор стрейфа

		// Мягкий переход между бедром \ прицелом
		if (_idx == 0)
		{ // От бедра
			curr_offs.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
			curr_rot.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
		}
		else
		{ // Во время аима
			curr_offs.mul(m_zoom_params.m_fZoomRotationFactor);
			curr_rot.mul(m_zoom_params.m_fZoomRotationFactor);
		}

		Fmatrix hud_rotation;
		Fmatrix hud_rotation_y;

		hud_rotation.identity();
		hud_rotation.rotateX(curr_rot.x);

		hud_rotation_y.identity();
		hud_rotation_y.rotateY(curr_rot.y);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation_y.identity();
		hud_rotation_y.rotateZ(curr_rot.z);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation.translate_over(curr_offs);
		trans.mulB_43(hud_rotation);
	}

	//============= Инерция оружия =============//
   // Параметры инерции
	float fInertiaSpeedMod = _lerp(
		hi->m_measures.m_inertion_params.m_tendto_speed,
		hi->m_measures.m_inertion_params.m_tendto_speed_aim,
		m_zoom_params.m_fZoomRotationFactor);

	float fInertiaReturnSpeedMod = _lerp(
		hi->m_measures.m_inertion_params.m_tendto_ret_speed,
		hi->m_measures.m_inertion_params.m_tendto_ret_speed_aim,
		m_zoom_params.m_fZoomRotationFactor);

	float fInertiaMinAngle = _lerp(
		hi->m_measures.m_inertion_params.m_min_angle,
		hi->m_measures.m_inertion_params.m_min_angle_aim,
		m_zoom_params.m_fZoomRotationFactor);

	Fvector4 vIOffsets; // x = L, y = R, z = U, w = D
	vIOffsets.x = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.x,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.x,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
	vIOffsets.y = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.y,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.y,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
	vIOffsets.z = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.z,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.z,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
	vIOffsets.w = _lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.w,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.w,
		m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;

	// Высчитываем инерцию из поворотов камеры
	bool bIsInertionPresent = m_fLR_InertiaFactor != 0.0f || m_fUD_InertiaFactor != 0.0f;
	if (abs(fYMag) > fInertiaMinAngle || bIsInertionPresent)
	{
		float fSpeed = fInertiaSpeedMod;
		if (fYMag > 0.0f && m_fLR_InertiaFactor > 0.0f ||
			fYMag < 0.0f && m_fLR_InertiaFactor < 0.0f)
		{
			fSpeed *= 2.f; //--> Ускоряем инерцию при движении в противоположную сторону
		}

		m_fLR_InertiaFactor -= (fYMag * fAvgTimeDelta * fSpeed); // Горизонталь (м.б. > |1.0|)
	}

	if (abs(fPMag) > fInertiaMinAngle || bIsInertionPresent)
	{
		float fSpeed = fInertiaSpeedMod;
		if (fPMag > 0.0f && m_fUD_InertiaFactor > 0.0f ||
			fPMag < 0.0f && m_fUD_InertiaFactor < 0.0f)
		{
			fSpeed *= 2.f; //--> Ускоряем инерцию при движении в противоположную сторону
		}

		m_fUD_InertiaFactor -= (fPMag * fAvgTimeDelta * fSpeed); // Вертикаль (м.б. > |1.0|)
	}

	clamp(m_fLR_InertiaFactor, -1.0f, 1.0f);
	clamp(m_fUD_InertiaFactor, -1.0f, 1.0f);

	// Плавное затухание инерции (основное, но без линейной никогда не опустит инерцию до полного 0.0f)
	m_fLR_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);
	m_fUD_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);

	// Минимальное линейное затухание инерции при покое (горизонталь)
	if (fYMag == 0.0f)
	{
		float fRetSpeedMod = (fYMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
		if (m_fLR_InertiaFactor < 0.0f)
		{
			m_fLR_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_InertiaFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fLR_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_InertiaFactor, 0.0f, 1.0f);
		}
	}

	// Минимальное линейное затухание инерции при покое (вертикаль)
	if (fPMag == 0.0f)
	{
		float fRetSpeedMod = (fPMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
		if (m_fUD_InertiaFactor < 0.0f)
		{
			m_fUD_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_InertiaFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fUD_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_InertiaFactor, 0.0f, 1.0f);
		}
	}

	// Применяем инерцию к худу
	float fLR_lim = (m_fLR_InertiaFactor < 0.0f ? vIOffsets.x : vIOffsets.y);
	float fUD_lim = (m_fUD_InertiaFactor < 0.0f ? vIOffsets.z : vIOffsets.w);

	Fvector curr_offs;
	curr_offs = { fLR_lim * -1.f * m_fLR_InertiaFactor, fUD_lim * m_fUD_InertiaFactor, 0.0f };

	Fmatrix hud_rotation;
	hud_rotation.identity();
	hud_rotation.translate_over(curr_offs);
	trans.mulB_43(hud_rotation);
}