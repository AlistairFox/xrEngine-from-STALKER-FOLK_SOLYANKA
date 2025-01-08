#pragma once
#include "eatable_item_object.h"
#include "ZoneCampfire.h"
#include "Actor.h"

class CCampFireSwitcher : public CEatableItemObject
{
	using inherited = CEatableItemObject;

public:

	CCampFireSwitcher();
	virtual ~CCampFireSwitcher();

	virtual void Load(LPCSTR section);
	virtual bool Useful() const;
	virtual BOOL net_Spawn(CSE_Abstract* DC);
	virtual bool UseBy(CEntityAlive* npc);
	virtual bool Empty() { return PortionsNum() == 0; };
	int PortionsNum() const { return m_iPortionsNum; };

	CZoneCampfire* GetNearestCampFire();

protected:
	int m_iPortionsNum;
};

