/*CCampfireSwitcher
*Author: AlistairFox
*/


#include "stdafx.h"
#include "CampFireSwitcher.h"

CCampFireSwitcher::CCampFireSwitcher()
{

}

CCampFireSwitcher::~CCampFireSwitcher()
{

}

void CCampFireSwitcher::Load(LPCSTR section)
{
	inherited::Load(section);
	m_iPortionsNum = pSettings->r_s32(section, "eat_portions_num");
}

bool CCampFireSwitcher::Useful() const
{
	if (!inherited::Useful()) return false;

	//ïðîâåðèòü íå âñå ëè åùå ñúåäåíî
	if (m_iPortionsNum == 0) return false;

	return true;
}

BOOL CCampFireSwitcher::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC)) return FALSE;

	return TRUE;
}

bool CCampFireSwitcher::UseBy(CEntityAlive* npc)
{

	CZoneCampfire* TargetCamp = GetNearestCampFire();

	if (!TargetCamp)
		return false;


	if (TargetCamp->IsEnabled())
		TargetCamp->GoDisabledState();
	else
		TargetCamp->GoEnabledState();


	if (m_iPortionsNum > 0)
		--m_iPortionsNum;
	else
		m_iPortionsNum = 0;

	return true;
}

CZoneCampfire* CCampFireSwitcher::GetNearestCampFire()
{
	CActor* pA = smart_cast<CActor*>(H_Parent());

	if (!pA)
		return nullptr;

	float CheckDistance = 100.f;
	u16 TargetObjectID = 0;


	for (u16 CampID : CZoneCampfire::vCampfires)
	{
		CObject* pObj = Level().Objects.net_Find(CampID);
		if (!pObj)
			continue;

		float NowDistance = pA->Position().distance_to(pObj->Position());

		if (NowDistance < CheckDistance)
		{
			TargetObjectID = CampID;
			CheckDistance = NowDistance;
		}
		else
			continue;
	}

	if (CheckDistance > 2.f)
		return nullptr;

	CZoneCampfire* TargetCamp = smart_cast<CZoneCampfire*>(Level().Objects.net_Find(TargetObjectID));

	return TargetCamp;
}
