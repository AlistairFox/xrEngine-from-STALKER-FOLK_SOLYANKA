////////////////////////////////////////////////////////////////////////////
//	Module 		: level_changer.h
//	Created 	: 10.07.2003
//  Modified 	: 10.07.2003
//	Author		: Dmitriy Iassenev
//	Description : Level change object
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameObject.h"
#include "../xrEngine/feel_touch.h"
#include "game_graph_space.h"

class CZoneTeleport : public CGameObject, public Feel::Touch
{
private:
	Fvector TeleportToPosition ;
	u32		TeleportConectionTo = 0;


	typedef	CGameObject	inherited;
	 
public:
						CZoneTeleport();
	virtual				~CZoneTeleport();
	virtual BOOL		net_Spawn(CSE_Abstract* DC);
	virtual void		net_Destroy();
	virtual void		Center(Fvector& C) const;
	virtual float		Radius() const;
	virtual void		shedule_Update(u32 dt);
	virtual void		feel_touch_new(CObject* O);
	virtual BOOL		feel_touch_contact(CObject* O);
 
};
