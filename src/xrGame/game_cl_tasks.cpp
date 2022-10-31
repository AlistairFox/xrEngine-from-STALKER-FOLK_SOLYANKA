#include "StdAfx.h"
#include "game_cl_freemp.h"
#include "GametaskManager.h"
#include "GameTask.h"
#include "Actor.h"

void game_cl_freemp::load_task(CGameTask* t)
{
	callback_load(t);
}

void game_cl_freemp::save_task(CGameTask* t)
{
	callback_save(t);
}

void game_cl_freemp::callback_load(CGameTask* t)
{
	CObjectJsonEx task_tab = json_ex.get_object(t->m_ID.c_str());
	Actor()->callback(GameObject::eTaskLoadState)  (t, t->m_ID.c_str(), &task_tab);

	t->load_json(task_tab);
}

void game_cl_freemp::callback_save(CGameTask* t)
{
	CObjectJsonEx task_tab;

	Actor()->callback(GameObject::eTaskSaveState)  (t, t->m_ID.c_str(), &task_tab);

	json_ex.set_object(t->m_ID.c_str(), task_tab);

	t->save_json(task_tab);
}