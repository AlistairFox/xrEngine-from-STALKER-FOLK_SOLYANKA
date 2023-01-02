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

float level_events::shedule_Scale()
{
    return 1.f;
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
	if (freemp)
	for (auto obj : freemp->alife_objects)
	{
		CSE_ALifeHumanStalker* stalker = smart_cast<CSE_ALifeHumanStalker*>(obj.second);
		CSE_ALifeMonsterAbstract* monster = smart_cast<CSE_ALifeMonsterAbstract*>(obj.second);
		CSE_ALifeLevelChanger* changer = smart_cast<CSE_ALifeLevelChanger*>(obj.second);
		CSE_ALifeOnlineOfflineGroup* group = smart_cast<CSE_ALifeOnlineOfflineGroup*>(obj.second);
		CSE_ALifeSmartZone* zone = smart_cast<CSE_ALifeSmartZone*>(obj.second);

		if (stalker || monster || changer || group || zone)
			obj.second->update_CL();			  // Для вызова нужна функция в скрипте иле вылет	  (Поэтому чек на класс)
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
				ai().script_engine().print_stack();
			}
		}
		else
			Msg("--- Не найден скрипт (%s)", name_f.c_str());
	}

}
