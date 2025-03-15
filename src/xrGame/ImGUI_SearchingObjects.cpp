#include "stdafx.h"
#include "Level.h"
#include "Actor.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"

#include "../xrEngine/XR_IOConsole.h"
#include "string_table.h"

#include "ai_space.h"

#include "ImUtils.h"
#include "ImGUI_Loader.h"

void RenderSearchManagerWindow()
{
	if (!g_pGameLevel || g_pClsidManager == nullptr || imgui_search_manager.is_initialized == false)
		return;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, kGeneralAlphaLevelForImGuiWindows));
	if (ImGui::Begin("Search Manager"))
	{
		constexpr size_t kItemSize = sizeof(imgui_search_manager.combo_items) / sizeof(imgui_search_manager.combo_items[0]);
		ImGui::Combo("Category", &imgui_search_manager.selected_type, imgui_search_manager.combo_items, kItemSize);

		ImGui::SeparatorText("Stats");
		ImGui::Text("Current category: %s (%d)", imgui_search_manager.convertTypeToString(imgui_search_manager.selected_type), imgui_search_manager.selected_type);
		ImGui::Text("Level: %s", Level().name().c_str());

		ImGui::Text("All: %d", imgui_search_manager.counts[(eSelectedType::kSelectedType_All)]);
		ImGui::Text("%s: %d", imgui_search_manager.pTranslatedLabel_SmartCover, imgui_search_manager.counts[(eSelectedType::kSelectedType_SmartCover)]);
		ImGui::Text("%s: %d", imgui_search_manager.pTranslatedLabel_SmartTerrain, imgui_search_manager.counts[(eSelectedType::kSelectedType_SmartTerrain)]);
		ImGui::Text("%s: %d", imgui_search_manager.pTranslatedLabel_Stalker, imgui_search_manager.counts[(eSelectedType::kSelectedType_Stalker)]);
		ImGui::Text("%s: %d", imgui_search_manager.pTranslatedLabel_Car, imgui_search_manager.counts[(eSelectedType::kSelectedType_Car)]);
		ImGui::Text("%s: %d", imgui_search_manager.pTranslatedLabel_LevelChanger, imgui_search_manager.counts[(eSelectedType::kSelectedType_LevelChanger)]);
		ImGui::Text("%s: %d", imgui_search_manager.pTranslatedLabel_Artefact, imgui_search_manager.counts[(eSelectedType::kSelectedType_Artefact)]);

		char colh_monsters[24]{};
		sprintf_s(colh_monsters, sizeof(colh_monsters), "Monsters: %d", imgui_search_manager.counts[eSelectedType::kSelectedType_Monster_All]);

		if (ImGui::CollapsingHeader(colh_monsters))
		{
			for (const auto& id : g_pClsidManager->get_monsters())
			{
				char monster_name[32]{};
				sprintf_s(monster_name, sizeof(monster_name), "%s: %d", g_pClsidManager->translateCLSID(id), imgui_search_manager.counts[imgui_search_manager.convertCLSIDToType(id)]);
				ImGui::Text(monster_name);
			}
		}

		char colh_weapons[24]{};
		sprintf_s(colh_weapons, sizeof(colh_weapons), "Weapons: %d", imgui_search_manager.counts[eSelectedType::kSelectedType_Weapon_All]);

		if (ImGui::CollapsingHeader(colh_weapons))
		{
			for (const auto& id : g_pClsidManager->get_weapons())
			{
				char weapon_name[32]{};
				sprintf_s(weapon_name, sizeof(weapon_name), "%s: %d", g_pClsidManager->translateCLSID(id), imgui_search_manager.counts[imgui_search_manager.convertCLSIDToType(id)]);
				ImGui::Text(weapon_name);
			}
		}

		ImGui::SeparatorText("Settings");
		ImGui::Checkbox("Alive", &imgui_search_manager.show_alive_creatures);
		if (ImGui::BeginItemTooltip())
		{
			ImGui::Text("Shows alive or not alive creature(if it is not creature this flag doesn't affect)");
			ImGui::EndTooltip();
		}

		ImGui::SeparatorText("Simulation");

		if (ImGui::BeginTabBar("##TB_InGameSearchManager"))
		{
			if (ImGui::BeginTabItem("Online##TB_Online_InGameSearchManager"))
			{
				memset(imgui_search_manager.counts, 0, sizeof(imgui_search_manager.counts));

				ImGui::InputText("##IT_InGameSeachManager", imgui_search_manager.search_string, sizeof(imgui_search_manager.search_string));

				char category_name_separator[64]{};
				const char* pTranslatedCategoryName = imgui_search_manager.convertTypeToString(imgui_search_manager.selected_type);
				size_t translate_str_len = strlen(pTranslatedCategoryName);
				memcpy_s(category_name_separator, sizeof(category_name_separator), pTranslatedCategoryName, translate_str_len);
				ImGui::SeparatorText(category_name_separator);

 				for (auto i = 0; i < Level().Objects.o_count(); ++i)
				{
					auto* pObject = Level().Objects.o_get_by_iterator(i);

					if (!pObject)
						continue;

					if (0 == strstr(pObject->cName().c_str(), imgui_search_manager.search_string))
						continue;

					if (pObject->H_Parent() == nullptr)
					{
						if (imgui_search_manager.valid(pObject->CLS_ID))
						{
							CGameObject* pCasted = smart_cast<CGameObject*>(pObject);
							xr_string name = pObject->cName().c_str();

							if (pCasted)
							{
								name += " ";
								name += "[";
								name += toUtf8(pCasted->Name()).c_str();
								name += "]";
							}

							name += "##InGame_SM_";
							name += std::to_string(i);

							if (ImGui::Button(name.c_str()))
							{
								CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());

								if (pActor)
								{
									Actor()->MoveActor(pObject->Position(), Fvector().set(0, 0, 0));
								}
							}

							if (ImGui::BeginItemTooltip())
							{
								ImGui::Text("system name: [%s]", pObject->cName().c_str());
								ImGui::Text("section name: [%s]", pObject->cNameSect().c_str());
								ImGui::Text("translated name: [%s]", toUtf8(pCasted->Name()).c_str());
								ImGui::Text("position: %f %f %f", pObject->Position().x, pObject->Position().y, pObject->Position().z);

								ImGui::EndTooltip();
							}
						}
					}
				}


				ImGui::EndTabItem();
			}

			// if (ImGui::BeginTabItem("Offline##TB_Offline_InGameSearchManager"))
			// {
			// 	memset(imgui_search_manager.counts, 0, sizeof(imgui_search_manager.counts));
			// 
			// 	ImGui::InputText("##IT_InGameSearchManager", imgui_search_manager.search_string, sizeof(imgui_search_manager.search_string));
			// 
			// 	char category_name_separator[64]{};
			// 	const char* pTranslatedCategoryName = imgui_search_manager.convertTypeToString(imgui_search_manager.selected_type);
			// 	size_t translate_str_len = strlen(pTranslatedCategoryName);
			// 	memcpy_s(category_name_separator, sizeof(category_name_separator), pTranslatedCategoryName, translate_str_len);
			// 	ImGui::SeparatorText(category_name_separator);
			// 
			// 	// todo: think about filtering for offline objects, because they can be REAL huge up to 32k...
			// 	// filtering is slow because it is linear, possible variants for optimization: unordered_map for names and name_replace
			// 	// possible suggestions: filter when button is pressed (but you need to remember the result and render only cache version (the result of filtering), not whole vector), create additional cache structures like filter by location and etc
			// 	for (auto& O : ai().alife().objects().objects())
			// 	{
			// 		CSE_ALifeDynamicObject* pServerObject = O.second;
			// 		if (!pServerObject)
			// 			continue;
			// 		if (0 == strstr(pServerObject->name_replace(), imgui_search_manager.search_string))
			// 			continue;
			// 
			// 		if (pServerObject->ID_Parent == 0xffff)
			// 		{
			// 			if (imgui_search_manager.valid(pServerObject->m_tClassID))
			// 			{
			// 				char button_name[128];
			// 				sprintf_s(button_name, "%s [%s]", pServerObject->name_replace() ? pServerObject->name_replace() : "", toUtf8(pServerObject->s_name.c_str()).c_str());
			// 
			// 				if (ImGui::Button(button_name))
			// 				{
			// 					CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());
			// 
			// 					if (pActor)
			// 					{
			// 						pActor->MoveActor(pServerObject->Position(), Fvector().set(0, 0, 0));
			// 					}
			// 				}
			// 			}
			// 		}
			// 	}
			// 
			// 
			// 	ImGui::EndTabItem();
			// }

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
	ImGui::PopStyleColor(1);
}
