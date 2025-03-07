#include "StdAfx.h"
#include "Weapon.h"
#include "object_saver.h"
#include "object_loader.h"
#include "Inventory.h"
#include "Level.h"


void CWeapon::signal_HideComplete()
{
	if (H_Parent())
		setVisible(FALSE);
	SetPending(FALSE);

	m_fLR_MovingFactor = 0.f;
	m_fLR_CameraFactor = 0.f;
	m_fLR_InertiaFactor = 0.f;
	m_fUD_InertiaFactor = 0.f;
}


void CWeapon::OnActiveItem()
{
	//. from Activate
	UpdateAddonsVisibility();
	m_BriefInfo_CalcFrame = 0;
	SwitchState(eShowing);
	inherited::OnActiveItem();
}

void CWeapon::OnHiddenItem()
{
	m_BriefInfo_CalcFrame = 0;

#ifdef USE_CLIENT_SIDE_WEAPONS
	SwitchState(eHiding);
#else 
	SwitchState(eHidden);
#endif

	OnZoomOut();
	inherited::OnHiddenItem();

	m_set_next_ammoType_on_reload = undefined_ammo_type;
}

void CWeapon::SendHiddenItem()
{
	if (!CHudItem::object().getDestroy() && m_pInventory)
	{
		// !!! Just single entry for given state !!!
		NET_Packet		P;
		CHudItem::object().u_EventGen(P, GE_WPN_STATE_CHANGE, CHudItem::object().ID());
		P.w_u8(u8(eHiding));
		P.w_u8(u8(m_sub_state));
		P.w_u8(m_ammoType);
		P.w_u8(u8(iAmmoElapsed & 0xff));
		P.w_u8(m_set_next_ammoType_on_reload);
		CHudItem::object().u_EventSend(P, net_flags(TRUE, TRUE, FALSE, TRUE));
		SetPending(TRUE);
	}
}

// NETWORK

BOOL CWeapon::net_Spawn(CSE_Abstract* DC)
{
	m_fRTZoomFactor = m_zoom_params.m_fScopeZoomFactor;
	BOOL bResult = inherited::net_Spawn(DC);
	CSE_Abstract* e = (CSE_Abstract*)(DC);
	CSE_ALifeItemWeapon* E = smart_cast<CSE_ALifeItemWeapon*>(e);

	//iAmmoCurrent					= E->a_current;
	iAmmoElapsed = E->a_elapsed;
	m_flagsAddOnState = E->m_addon_flags.get();
	m_ammoType = E->ammo_type;
	if (E->m_cur_scope < m_scopes.size() && m_scopes.size()>1)
		m_cur_scope = E->m_cur_scope;
	SetState(E->wpn_state);
	SetNextState(E->wpn_state);

	m_DefaultCartridge.Load(m_ammoTypes[m_ammoType].c_str(), m_ammoType);
	if (iAmmoElapsed)
	{
		m_fCurrentCartirdgeDisp = m_DefaultCartridge.param_s.kDisp;
		for (int i = 0; i < iAmmoElapsed; ++i)
			m_magazine.push_back(m_DefaultCartridge);
	}

	UpdateAltScope();
	UpdateAddonsVisibility();
	InitAddons();

	m_dwWeaponIndependencyTime = 0;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());
	m_bAmmoWasSpawned = false;


	return bResult;
}

void CWeapon::net_Destroy()
{
	inherited::net_Destroy();

	//удалить объекты партиклов
	StopFlameParticles();
	StopFlameParticles2();
	StopLight();
	Light_Destroy();

	while (m_magazine.size()) m_magazine.pop_back();
}


void CWeapon::net_Export(NET_Packet& P)
{
	inherited::net_Export(P);
 	P.w_float_q8(GetCondition(), 0.0f, 1.0f);
  	P.w_u8(IsUpdating());
	P.w_u16(u16(iAmmoElapsed));
	P.w_u8(m_flagsAddOnState);
	P.w_u8(m_ammoType);
	P.w_u8((u8)GetState());
	P.w_u8((u8)IsZoomed());
	P.w_u8((u8)m_cur_scope);
}

void CWeapon::net_Import(NET_Packet& P)
{
	inherited::net_Import(P);

	float _cond;
	P.r_float_q8(_cond, 0.0f, 1.0f);
	SetCondition(_cond);

	u8 flags = 0;
	P.r_u8(flags);

	u16 ammo_elapsed = 0;
	P.r_u16(ammo_elapsed);

	u8						NewAddonState;
	P.r_u8(NewAddonState);

	m_flagsAddOnState = NewAddonState;
	UpdateAddonsVisibility();

	u8 ammoType, wstate;
	P.r_u8(ammoType);
	P.r_u8(wstate);

	u8 Zoom;
	P.r_u8((u8)Zoom);

	u8 scope;
	P.r_u8(scope);

	m_cur_scope = scope;

#ifdef USE_CLIENT_SIDE_WEAPONS
	CGameObject* object = (CGameObject*)H_Parent();
	if (object && !object->cast_actor())
#endif
	{


		if (H_Parent() && H_Parent()->Remote())
		{
			if (Zoom)
				OnZoomIn();
			else
				OnZoomOut();
		};

		switch (wstate)
		{
		case eFire:
		case eFire2:
		case eSwitch:
		case eReload:
		{
		}break;
		default:
		{
			if (ammoType >= m_ammoTypes.size())
				Msg("!! Weapon [%d], State - [%d]", ID(), wstate);
			else
			{
				m_ammoType = ammoType;
				SetAmmoElapsed((ammo_elapsed));
			}
		}break;
		}

		VERIFY((u32)iAmmoElapsed == m_magazine.size());
	}
}

void CWeapon::save(NET_Packet& output_packet)
{
	inherited::save(output_packet);
	save_data(iAmmoElapsed, output_packet);
	save_data(m_cur_scope, output_packet);
	save_data(m_flagsAddOnState, output_packet);
	save_data(m_ammoType, output_packet);
	save_data(m_zoom_params.m_bIsZoomModeNow, output_packet);
	save_data(m_bRememberActorNVisnStatus, output_packet);
	save_data(bNVsecondVPstatus, output_packet);
}

void CWeapon::load(IReader& input_packet)
{
	inherited::load(input_packet);
	load_data(iAmmoElapsed, input_packet);
	load_data(m_cur_scope, input_packet);
	load_data(m_flagsAddOnState, input_packet);
	UpdateAddonsVisibility();
	load_data(m_ammoType, input_packet);
	load_data(m_zoom_params.m_bIsZoomModeNow, input_packet);

	if (m_zoom_params.m_bIsZoomModeNow)
		OnZoomIn();
	else
		OnZoomOut();

	load_data(m_bRememberActorNVisnStatus, input_packet);
	load_data(bNVsecondVPstatus, input_packet);
}

// EVENTS
void CWeapon::WriteUpdateActiveSlot(NET_Packet& P)
{
	P.w_u8(u8(GetState()));
	P.w_u8(m_sub_state);
	P.w_u8(m_ammoType);
	P.w_u8(u8(iAmmoElapsed & 0xff));
	P.w_u8(m_set_next_ammoType_on_reload);
}

void CWeapon::OnUpdateActiveSlot(NET_Packet& P)
{
	u8	state;
	P.r_u8(state);
	P.r_u8(m_sub_state);

	u8 NewAmmoType = P.r_u8();
	u8 AmmoElapsed = P.r_u8();
	u8 NextAmmo = P.r_u8();

	if (NextAmmo == undefined_ammo_type)
		m_set_next_ammoType_on_reload = undefined_ammo_type;
	else
		m_set_next_ammoType_on_reload = NextAmmo;

	SetAmmoElapsed(int(AmmoElapsed));
	SetState(state);

	// if (state != GetState())
	// {
	// 	OnStateSwitch(u32(state));
	// 	if (OnServer())
	// 		SetNextState(state);
	// }
}


void CWeapon::OnEvent(NET_Packet& P, u16 type)
{
	switch (type)
	{

	case GE_ADDON_CHANGE:
	{
		P.r_u8(m_flagsAddOnState);
		InitAddons();
		UpdateAddonsVisibility();
	}break;

	case GE_WPN_STATE_CHANGE:
	{
#ifdef USE_CLIENT_SIDE_WEAPONS
		if (H_Parent() != Level().CurrentControlEntity())
#endif
		{
			u8				state;
			P.r_u8(state);
			P.r_u8(m_sub_state);
			u8 NewAmmoType = P.r_u8();
			u8 AmmoElapsed = P.r_u8();
			u8 NextAmmo = P.r_u8();

			if (NextAmmo == undefined_ammo_type)
				m_set_next_ammoType_on_reload = undefined_ammo_type;
			else
				m_set_next_ammoType_on_reload = NextAmmo;

			SetAmmoElapsed(int(AmmoElapsed));
			OnStateSwitch(u32(state));

			if (OnServer())
				SetNextState(state);
		}
	}
	break;
	default:
	{
		inherited::OnEvent(P, type);
	}break;
	}
};


// STATE SWITCH
#include "../xrEngine/x_ray.h"
#include "UIGameCustom.h"
#include "ui\UIActorMenu.h"
#include "Actor.h"


void CWeapon::SwitchState(u32 S)
{
#ifdef USE_CLIENT_SIDE_WEAPONS
	if (Level().CurrentControlEntity() == H_Parent())
		OnStateSwitch(S);
#else 
	if (OnClient())
		return;
#endif
	if (OnServer())
		SetNextState(S);

	if (OnServer() || Level().CurrentControlEntity() == H_Parent())
		if (!CHudItem::object().getDestroy() && m_pInventory)
		{
			// !!! Just single entry for given state !!!
			NET_Packet		P;
			CHudItem::object().u_EventGen(P, GE_WPN_STATE_CHANGE, CHudItem::object().ID());
			P.w_u8(u8(S));
			P.w_u8(u8(m_sub_state));
			P.w_u8(m_ammoType);
			P.w_u8(u8(iAmmoElapsed & 0xff));
			P.w_u8(m_set_next_ammoType_on_reload);
			CHudItem::object().u_EventSend(P, net_flags(TRUE, TRUE, FALSE, TRUE));
		}

}


void CWeapon::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch(S);
	m_BriefInfo_CalcFrame = 0;

	/*
	if (H_Parent() == Level().CurrentEntity())
	{
		CActor* current_actor = smart_cast<CActor*>(H_Parent());

		if (&CurrentGameUI()->ActorMenu() && CurrentGameUI()->ActorMenu().GetMenuMode() == mmUndefined)
		{
			if ((GetState() == eReload || GetState() == eUnMisfire || (GetState() == eBore && (GameConstants::GetSSFX_EnableBoreDoF() && m_bEnableBoreDof))) && current_actor)
			{
				ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
				ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
			}
			else
			{
				ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
				ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
			}
		}
	}
	*/
}



// ACTION EVENTS

BOOL	b_toggle_weapon_aim = FALSE;

bool CWeapon::Action(u16 cmd, u32 flags)
{
	if (inherited::Action(cmd, flags)) return true;


	switch (cmd)
	{
	//case kWPN_NV_CHANGE:
	//{
	//	return bChangeNVSecondVPStatus();
	//}
	//	break;

	case kWPN_FIRE:
	{
		//если оружие чем-то занято, то ничего не делать
		{
			if (IsPending())
				return				false;

			if (flags & CMD_START)
				FireStart();
			else
				FireEnd();
		};
		return true;
	}
 
	case kWPN_NEXT:
	{
		return SwitchAmmoType(flags);
	}

	case kWPN_ZOOM:
	{
		if (IsZoomEnabled())
		{
			if (b_toggle_weapon_aim)
			{
				if (flags & CMD_START)
				{
					if (!IsZoomed())
					{
						if (!IsPending())
						{
							if (GetState() != eIdle)
								SwitchState(eIdle);
							OnZoomIn();
						}
					}
					else
						OnZoomOut();
				}
			}
			else
			{
				if (flags & CMD_START)
				{
					if (!IsZoomed() && !IsPending())
					{
						if (GetState() != eIdle)
							SwitchState(eIdle);
						OnZoomIn();
					}
				}
				else
					if (IsZoomed())
						OnZoomOut();
			}
			return true;
		}
		else
			return false;
	}

	case kWPN_ZOOM_INC:
	case kWPN_ZOOM_DEC:
		if (IsZoomEnabled() && IsZoomed())
		{
			if (cmd == kWPN_ZOOM_INC)  ZoomInc();
			else					ZoomDec();
			return true;
		}
		else
			return false;
	}
	return false;
}

void CWeapon::OnAnimationEnd(u32 state)
{
	inherited::OnAnimationEnd(state);
}

