//////////////////////////////////////////////////////////////////////
// RocketLauncher.cpp:	��������� ��� ��������� �������� 
//						���������� ��������� � ��������
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RocketLauncher.h"
#include "CustomRocket.h"
#include "xrserver_objects_alife_items.h"
#include "level.h"
#include "ai_object_location.h"
#include "../xrEngine/IGame_Persistent.h"

CRocketLauncher::CRocketLauncher()
{
//	m_pRocket =  NULL;
}
CRocketLauncher::~CRocketLauncher()
{
}
void  CRocketLauncher::Load	(LPCSTR section)
{
	m_fLaunchSpeed = pSettings->r_float(section, "launch_speed");
}

void CRocketLauncher::SpawnRocket(const shared_str& rocket_section, CGameObject* parent_rocket_launcher)
{
	if (OnClient())	
		return;

	CSE_Abstract*		D	= F_entity_Create(rocket_section.c_str());
	R_ASSERT			(D);
	CSE_Temporary		*l_tpTemporary = smart_cast<CSE_Temporary*>(D);
	R_ASSERT			(l_tpTemporary);
	l_tpTemporary->m_tNodeID= (g_dedicated_server)?u32(-1) : parent_rocket_launcher->ai_location().level_vertex_id();
	D->s_name			= rocket_section;
	D->set_name_replace	("");
	
 	D->s_RP				=	0xff;
	D->ID				=	0xffff;
	D->ID_Parent		=	parent_rocket_launcher->ID();
	D->ID_Phantom		=	0xffff;
	D->s_flags.assign	(M_SPAWN_OBJECT_LOCAL);
	D->RespawnTime		=	0;
	
	NET_Packet			P;
	D->Spawn_Write		(P,TRUE);
	Level().Send		(P,net_flags(TRUE));
	F_entity_Destroy	(D);
}

void CRocketLauncher::SpawnRocketSend(const shared_str& rocket_section, CGameObject* parent_rocket_launcher)
{
	NET_Packet packet;
	Game().u_EventGen(packet, GE_WPN_SPAWNGRENADE, parent_rocket_launcher->ID());
	packet.w_stringZ(rocket_section);
	Game().u_EventSend(packet);
}

void CRocketLauncher::AttachRocket(u16 rocket_id, CGameObject* parent_rocket_launcher)
{
	CCustomRocket * pRocket = smart_cast<CCustomRocket*>(Level().Objects.net_Find(rocket_id));
	pRocket->m_pOwner = smart_cast<CGameObject*>(parent_rocket_launcher->H_Root());
	VERIFY(pRocket->m_pOwner);
	pRocket->H_SetParent(parent_rocket_launcher);
	m_rockets.push_back(pRocket);
}

void CRocketLauncher::DetachRocket(u16 rocket_id, bool bLaunch)
{
	CCustomRocket *pRocket = smart_cast<CCustomRocket*>(Level().Objects.net_Find(rocket_id));
	if (!pRocket && OnClient())
		return;
	ROCKETIT It = std::find(m_rockets.begin(), m_rockets.end(), pRocket);
	ROCKETIT It_l = std::find(m_launched_rockets.begin(), m_launched_rockets.end(), pRocket);
 	if (OnServer())
	{
		VERIFY( (It != m_rockets.end()) || (It_l != m_launched_rockets.end()) );
	};
	 
	if( It != m_rockets.end() )
	{
		(*It)->m_bLaunched	= bLaunch;
		(*It)->H_SetParent	(NULL);
		m_rockets.erase		(It);
	}

	if( It_l != m_launched_rockets.end() )
	{
		(*It)->m_bLaunched			= bLaunch;
		(*It_l)->H_SetParent		(NULL);
		m_launched_rockets.erase	(It_l);
	}

}

void CRocketLauncher::LaunchRocket(const Fmatrix& xform,  
								   const Fvector& vel, 
								   const Fvector& angular_vel)
{
  	getCurrentRocket()->SetLaunchParams(xform, vel, angular_vel);
	m_launched_rockets.push_back( getCurrentRocket() );
}

CCustomRocket*	CRocketLauncher::getCurrentRocket()
{
	if( m_rockets.size() )
		return m_rockets.back();
	else
		return (CCustomRocket*)0;
}

void CRocketLauncher::dropCurrentRocket()
{
	m_rockets.pop_back();
}

u32 CRocketLauncher::getRocketCount()
{
	return m_rockets.size();
}
