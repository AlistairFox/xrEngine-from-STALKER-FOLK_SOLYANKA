#pragma once
#include "weaponmagazined.h"
#include "rocketlauncher.h"


class CWeaponFakeGrenade;


class CWeaponMagazinedWGrenade : public CWeaponMagazined,
	public CRocketLauncher
{
	typedef CWeaponMagazined inherited;
public:
	CWeaponMagazinedWGrenade(ESoundTypes eSoundType = SOUND_TYPE_WEAPON_SUBMACHINEGUN);
	virtual			~CWeaponMagazinedWGrenade();

	virtual void	Load(LPCSTR section);

	virtual BOOL	net_Spawn(CSE_Abstract* DC);
	virtual void	net_Destroy();
	virtual void	net_Export(NET_Packet& P);
	virtual void	net_Import(NET_Packet& P);

	virtual void	OnH_B_Independent(bool just_before_destroy);

	virtual void	save(NET_Packet& output_packet);
	virtual void	load(IReader& input_packet);

	// Mortan: ����� ��������� �����
	virtual	bool    bMarkCanShow() { return IsZoomed() && !m_bGrenadeMode; }
	virtual void	UpdateSecondVP(bool bInGrenade = false);
	virtual	bool	bInZoomRightNow() const { return (m_zoom_params.m_fZoomRotationFactor > 0.05) && !m_bGrenadeMode; }
	virtual void    CheckMagazine();
	//=========================================

	virtual bool	Attach(PIItem pIItem, bool b_send_event);
	virtual bool	Detach(const char* item_section_name, bool b_spawn_item);
	virtual bool	CanAttach(PIItem pIItem);
	virtual bool	CanDetach(const char* item_section_name);
	virtual void	InitAddons();
	virtual bool	UseScopeTexture();
	virtual	float	CurrentZoomFactor();
	virtual	u8		GetCurrentHudOffsetIdx();
	virtual void	FireEnd();

	//virtual void	LaunchGrenade();

	virtual void	LaunchGrenadeRecived(Fvector DIR, Fvector POS);
	virtual void	LaunchGrenadeSend();

	virtual void	OnStateSwitch(u32 S);

	virtual void	switch2_Reload();
	virtual void    switch2_Unmis();
	virtual void	state_Fire(float dt);
	virtual void	OnShot();
	virtual void	OnEvent(NET_Packet& P, u16 type);
	virtual void	ReloadMagazine();
	virtual void	OnMotionMark(u32 state, const motion_marks& M);

	virtual bool	Action(u16 cmd, u32 flags);

	virtual void	UpdateSounds();

	//������������ � ����� �������������
	virtual bool	SwitchMode();
	void			PerformSwitchGL();
	void			OnAnimationEnd(u32 state);
	virtual void	OnMagazineEmpty();
	virtual bool	GetBriefInfo(II_BriefInfo& info);

	virtual bool	IsNecessaryItem(const shared_str& item_sect);

	bool			IsGrenadeMode() const override { return m_bGrenadeMode; }

	//����������� ������� ��� ������������ �������� HUD
	virtual void	PlayAnimShow();
	virtual void	PlayAnimHide();
	virtual void	PlayAnimReload();
	virtual void	PlayAnimIdle();
	virtual void	PlayAnimShoot();
	virtual void	PlayAnimModeSwitch();
	virtual void	PlayAnimBore();
	virtual void	PlayAnimFireMode();
	virtual void	PlayAnimLaserSwitch();
	virtual void	PlayAnimFlashlightSwitch();

private:
	virtual	void	net_Spawn_install_upgrades(Upgrades_type saved_upgrades);
	virtual bool	install_upgrade_impl(LPCSTR section, bool test);
	virtual	bool	install_upgrade_ammo_class(LPCSTR section, bool test);

	int		GetAmmoCount2(u8 ammo2_type) const;

public:
	//�������������� ��������� �������� 
	//��� �������������
//-	CWeaponAmmo*			m_pAmmo2;
	xr_vector<shared_str>	m_ammoTypes2;
	u8						m_ammoType2;

	int						iMagazineSize2;
	xr_vector<CCartridge>	m_magazine2;

	bool					m_bGrenadeMode;

	CCartridge				m_DefaultCartridge2;
	u8						iAmmoElapsed2;

	int						iAmmoElapsedMain;

	bool					IsMainMagazineEmpty() { return iAmmoElapsedMain <= 0; }

	virtual void UpdateGrenadeVisibility(bool visibility);
};