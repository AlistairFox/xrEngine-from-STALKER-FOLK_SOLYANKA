#include "StdAfx.h"
#include "guard_utils.hpp"
#include <Level.h>
#include <memory>

inline _NODISCARD bool __search_global_dll(const std::string& _Dll_name)
{
	handle_ptr snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!snap)
		return false;

	PROCESSENTRY32 pe{ sizeof(PROCESSENTRY32) };
	while (Process32Next(snap.get(), &pe))
	{
		MODULEENTRY32 me{ sizeof(MODULEENTRY32) };
		handle_ptr module_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pe.th32ProcessID);

		if (!module_snap)
			continue;

		while (Module32Next(module_snap.get(), &me))
			if (me.szModule == _Dll_name)
				return true;
	}
	return false;
}

inline _NODISCARD bool SearchGlobalDll(const std::string& dll_name)
{
	if (dll_name.empty())
		return false;
	return __search_global_dll(dll_name);
}

_NODISCARD bool IsCheatEngineRunning()
{
	return SearchGlobalDll(xor("lfs.dll"));
}

extern BaseThreadInitThunk* _Hk_base_thread_org{};
extern bool is_injected{};

void __fastcall BaseThreadInitHook(DWORD reserved, LPTHREAD_START_ROUTINE start_addr, PVOID arg)
{
	constexpr uintptr_t _Global_start_addr = 0x7FF000000000;

	const bool _Is_strange = reinterpret_cast<uintptr_t>(start_addr) < _Global_start_addr;
	bool _Is_not_internal = false;

	MEMORY_BASIC_INFORMATION _Mem_basic_info{};
	if (VirtualQuery(start_addr, &_Mem_basic_info, sizeof(_Mem_basic_info)))
		if (_Mem_basic_info.Type != MEM_IMAGE)
			_Is_not_internal = true;

	if (_Is_strange && _Is_not_internal)
	{
		if (g_pGameLevel && !Level().IsServer())
		{
			std::unique_ptr<NET_Packet> P = std::make_unique<NET_Packet>();

			auto game = Level().game;

			game->u_EventGen(*P, GE_INJECT_HOOK, 0);
			game->u_EventSend(*P);
		}
		else
			is_injected = true;
	}
	_Hk_base_thread_org(reserved, start_addr, arg);
}