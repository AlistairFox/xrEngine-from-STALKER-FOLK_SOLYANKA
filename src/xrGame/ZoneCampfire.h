#pragma once
#include "MosquitoBald.h"

class CZoneCampfire :public CMosquitoBald
{
	typedef CMosquitoBald	inherited;
protected:
	CParticlesObject*		m_pEnablingParticles;
	CParticlesObject*		m_pDisabledParticles;
	ref_sound				m_disabled_sound;
	bool					m_turned_on;
	u32						m_turn_time;
	u32						m_update_save_time;
	u32						FireTimer = 0;
	bool					OffTimerStart = false;

		virtual	void		PlayIdleParticles			(bool bIdleLight=true);
		virtual	void		StopIdleParticles			(bool bIdleLight=true);
		virtual BOOL		AlwaysTheCrow				();
		virtual	void		UpdateWorkload				(u32 dt);

public:
							CZoneCampfire				();
	virtual					~CZoneCampfire				();
	virtual		void		Load						(LPCSTR section);
	virtual		void		GoEnabledState				();
	virtual		void		GoDisabledState				();
	virtual		BOOL		net_Spawn	(CSE_Abstract* DC);
				void		turn_on_script				();
				void		turn_off_script				();
				bool		is_on						();
	virtual		void		shedule_Update				(u32	dt	);
	virtual		void		OnStateSwitch(EZoneState new_state);
	virtual		bool		Enable();
	virtual		bool		Disable();
	static std::vector<u16> vCampfires;
	virtual		void	UpdateCL();
	virtual		void	OnEvent(NET_Packet& P, u16 type);

	u32			UpdateTimer = 0;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};


add_to_type_list(CZoneCampfire)
#undef script_type_list
#define script_type_list save_type_list(CZoneCampfire)