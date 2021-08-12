////////////////////////////////////////////////////////////////////////////
//	Module 		: script_binder_object.cpp
//	Created 	: 29.03.2004
//  Modified 	: 29.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Script object binder
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "script_binder_object.h"
#include "script_game_object.h"

CScriptBinderObject::CScriptBinderObject	(CScriptGameObject *object)
{
	m_object		= object;
}

CScriptBinderObject::~CScriptBinderObject	()
{
//#ifdef DEBUG
	if (m_object)
		Msg			("Destroying binded object %s",m_object->Name());
//#endif
}

void CScriptBinderObject::reinit			()
{
	if (psDeviceFlags.test(rsCameraPos))
		Msg("reinit [%s]", m_object->Name());
}

void CScriptBinderObject::reload			(LPCSTR section)
{
	if (psDeviceFlags.test(rsCameraPos))
		Msg("reload [%s]", m_object->Name());
}

bool CScriptBinderObject::net_Spawn			(SpawnType DC)
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("net_Spawn [%s]", m_object->Name());
	return			(true);
}

void CScriptBinderObject::net_Destroy		()
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("net_Destroy [%s]", m_object->Name());
}

void CScriptBinderObject::net_Import		(NET_Packet *net_packet)
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("net_Import [%s]", m_object->Name());
}

void CScriptBinderObject::net_Export		(NET_Packet *net_packet)
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("net_Export [%s]", m_object->Name());
}

void CScriptBinderObject::shedule_Update	(u32 time_delta)
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("shedule_Update [%s]", m_object->Name());
}

void CScriptBinderObject::save				(NET_Packet *output_packet)
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("save [%s]", m_object->Name());
}

void CScriptBinderObject::load				(IReader	*input_packet)
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("load [%s]", m_object->Name());
}

bool CScriptBinderObject::net_SaveRelevant	()
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("net_SaveRelevant [%s]", m_object->Name());
	return		(false);
}

void CScriptBinderObject::net_Relcase		(CScriptGameObject *object)
{
	if (psDeviceFlags.test(rsCameraPos))
	Msg("net_Relcase [%s]", m_object->Name());
}
