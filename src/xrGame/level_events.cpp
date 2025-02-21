#include "stdafx.h"
#include "level_events.h"

#include "Level.h"
#include "game_cl_freemp.h"

#include "../xrServerEntities/xrServer_Objects_ALife_Monsters.h"

level_events::level_events()
{
    shedule.t_min = 100;
    shedule.t_max = 200;

    shedule_register();
}

level_events::~level_events()
{
    shedule_unregister(); 
	Level().event_functors.clear_and_free();
}

extern float Shedule_Events;

float level_events::shedule_Scale()
{
    return Shedule_Events;
}


bool level_events::shedule_Needed()
{
    return true;
}

shared_str level_events::shedule_clsid()
{
    return "level_events";
}

void level_events::shedule_Update(u32 dt)
{
    ISheduled::shedule_Update(dt);
 
	game_cl_freemp* freemp = smart_cast<game_cl_freemp*>(Level().game);
	 
	//Апдейт Засинхреных AlifeObjects
	if (freemp && m_last_update_data < Device.dwTimeGlobal)
	{
		m_last_update_data = Device.dwTimeGlobal + 500;

		// Msg("[level_events] freemp is used mode");
  		for (auto obj : freemp->alife_objects)
		{
			//CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(obj.second);
			//CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(obj.second);
			//CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(obj.second);
			CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(obj.second);
			CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(obj.second);

			// if (obj.second != nullptr)
			// 	Msg("[level_events] update object: %s", obj.second->name_replace());
			// else
			// 	Msg("[level_events] object %d, is dead: nullptr", obj.first);
		
			if (group || zone) // stalker || monster || changer || 
				obj.second->update_CL();			  // Для вызова нужна функция в скрипте иле вылет	  (Поэтому чек на класс)
		}
	}
 
	for (auto name_f : Level().event_functors)
	{
		luabind::functor<void> update;
		bool has_functor = ai().script_engine().functor(name_f.c_str(), update);
		if (has_functor)
		{
			try
			{			
				update(Device.dwTimeGlobal);
			}
			catch (...)
			{
				Msg("--- Crushed Update Event [%s]", name_f.c_str());
 			}
		}
		else
			Msg("--- Не найден скрипт (%s)", name_f.c_str());
	}

}
