#include "StdAfx.h"
#include "UIActorMenu.h"
#include "inventory_item.h"
#include "Level.h"
#include "Actor.h"

void CUIActorMenu::InitCraft()
{
	recipes.clear();

	string_path file_name;
	FS.update_path(file_name, "$game_config$", "craft.ltx");

	CInifile* file = xr_new<CInifile>(file_name, true); //read only

	if (!file)
		return;

	if (file->section_exist("recipes"))
	{
		CInifile::Sect& sect = file->r_section("recipes");
 		for (const auto& read_sect : sect.Data)
		{
			if (file->line_exist(read_sect.first, "ingredients") && 
				file->line_exist(read_sect.first, "ingredients_2"))
			{
				LPCSTR S = file->r_string(read_sect.first, "ingredients");
				LPCSTR S2 = file->r_string(read_sect.first, "ingredients_2");
				if ((S && S[0]) && (S2 && S2[0]))
				{
					RecipeSection recipe_data;

					recipe_data.out_section._set(read_sect.first);
					recipe_data.craft_chance = READ_IF_EXISTS(file, r_float, read_sect.first, "chance", 1.0f);

					string128		str_temp;
					int				count = _GetItemCount(S);
					for (int it = 0; it < count; ++it)
					{
						_GetItem(S, it, str_temp);
						recipe_data.ing_1_sections.push_back(str_temp);
					}
					string128		str_temp2;
					int				count2 = _GetItemCount(S2);
					for (int i = 0; i < count2; ++i)
					{
						_GetItem(S2, i, str_temp2);
						recipe_data.ing_2_sections.push_back(str_temp2);
					}

					recipes.push_back(recipe_data);
				}
			}			
		}
	}
  
	xr_delete(file);
}

bool CUIActorMenu::VerifyCraftRecipe(shared_str sect1, shared_str sect2, RecipeSection& recipe)
{
	for (const auto& data : recipes)
	{
        bool found_in_ing1 = false;
        bool found_in_ing2 = false;

        for (const auto& ing : data.ing_1_sections) 
		{
            if (xr_strcmp(sect1.c_str(), ing.c_str()) == 0 || xr_strcmp(sect2.c_str(), ing.c_str()) == 0) 
			{
                found_in_ing1 = true;
                break;
            }
        }
        for (const auto& ing : data.ing_2_sections) 
		{
            if (xr_strcmp(sect1.c_str(), ing.c_str()) == 0 || xr_strcmp(sect2.c_str(), ing.c_str()) == 0)
			{
                found_in_ing2 = true;
                break;
            }
        }

        if (found_in_ing1 && found_in_ing2) {
			recipe = data;
            return true;
        }
	}

	return false;
}

void CUIActorMenu::CraftDestroyItem(PIItem itm)
{
	if (!itm)
		return;

	NET_Packet							packet;
	packet.w_begin						(M_EVENT);
	packet.w_u32						(Level().timeServer());
	packet.w_u16						(GE_DESTROY);
	packet.w_u16						(itm->object_id());
	Level().Send						(packet,net_flags(TRUE,TRUE));
}

void CUIActorMenu::TryCraftItem(PIItem first, PIItem second, RecipeSection section_to_craft)
{
	if (!section_to_craft.out_section.size())
		return;

	float probability = Random.randF();

	CraftDestroyItem(first);
	CraftDestroyItem(second);

	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());

	if (!actor)
		return;

	NET_Packet								P;
	actor->u_EventGen					(P, GE_CRAFT_ITEM, Game().local_player->GameID);

	if (probability <= section_to_craft.craft_chance)
	{
		P.w_u8(1);
		P.w_stringZ								(section_to_craft.out_section);
	}
	else
	{
		P.w_u8(0);
		P.w_stringZ(first->m_section_id);
		P.w_stringZ(second->m_section_id);
	}

	actor->u_EventSend				(P);
}