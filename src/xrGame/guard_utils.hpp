#pragma once
#include "hooks.hpp"
#include "handle.hpp"
#include "cryptor.hpp"

#include <TlHelp32.h>
#include <chrono>

using BaseThreadInitThunk = void(__fastcall)(DWORD, LPTHREAD_START_ROUTINE, PVOID);
extern BaseThreadInitThunk* _Hk_base_thread_org;

extern bool is_injected;
void __fastcall BaseThreadInitHook(DWORD reserved, LPTHREAD_START_ROUTINE start_addr, PVOID arg);

inline _NODISCARD bool __search_global_dll(const std::string& _Dll_name);
inline _NODISCARD bool SearchGlobalDll(const std::string& dll_name);
_NODISCARD bool IsCheatEngineRunning();

class Timer
{
public:
	Timer(const std::chrono::seconds time) : duration{ time }, initialized{} {}

	void init()
	{
		if (!initialized)
		{
			initialized = true;
			passed_time = std::chrono::system_clock::now();
		}
	}

	void uninit()
	{
		initialized = false;
	}

	void update()
	{
		passed_time = std::chrono::system_clock::now();
	}

	bool isTimeExceeded() const
	{
		return (std::chrono::system_clock::now() - passed_time) >= duration;
	}

private:
	std::chrono::system_clock::time_point passed_time;
	std::chrono::seconds duration;
	bool initialized;
};