#include "stdafx.h"
#include "xrDebug.h"

#include <windows.h>
#include <dbghelp.h>
#include <winnt.h>

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "ntdll.lib")

bool xrDebug::GetNextStackFrameString(LPSTACKFRAME stackFrame, PCONTEXT threadCtx, std::string& frameStr)
{
	BOOL result = StackWalk(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), stackFrame, threadCtx, nullptr,
		SymFunctionTableAccess, SymGetModuleBase, nullptr);

	if (result == FALSE || stackFrame->AddrPC.Offset == 0)
	{
		return false;
	}

	frameStr.clear();
	string512 formatBuff;

	///
	/// Module name
	///
	HINSTANCE hModule = (HINSTANCE)SymGetModuleBase(GetCurrentProcess(), stackFrame->AddrPC.Offset);
	if (hModule && GetModuleFileName(hModule, formatBuff, _countof(formatBuff)))
	{
		frameStr.append(formatBuff);
	}

	///
	/// Address
	///
	xr_sprintf(formatBuff, _countof(formatBuff), " at %p", stackFrame->AddrPC.Offset);
	frameStr.append(formatBuff);

	///
	/// Function info
	///
	u8 arrSymBuffer[512];
	ZeroMemory(arrSymBuffer, sizeof(arrSymBuffer));
	PIMAGEHLP_SYMBOL functionInfo = reinterpret_cast<PIMAGEHLP_SYMBOL>(arrSymBuffer);
	functionInfo->SizeOfStruct = sizeof(*functionInfo);
	functionInfo->MaxNameLength = sizeof(arrSymBuffer) - sizeof(*functionInfo) + 1;
	DWORD_PTR dwFunctionOffset;

	result = SymGetSymFromAddr(GetCurrentProcess(), stackFrame->AddrPC.Offset, &dwFunctionOffset, functionInfo);

	if (result)
	{
		if (dwFunctionOffset)
		{
			xr_sprintf(formatBuff, _countof(formatBuff), " %s() + %Iu byte(s)", functionInfo->Name, dwFunctionOffset);
		}
		else
		{
			xr_sprintf(formatBuff, _countof(formatBuff), " %s()", functionInfo->Name);
		}
		frameStr.append(formatBuff);
	}

	///
	/// Source info
	///
	DWORD dwLineOffset;
	IMAGEHLP_LINE sourceInfo = {};
	sourceInfo.SizeOfStruct = sizeof(sourceInfo);

	result = SymGetLineFromAddr(GetCurrentProcess(), stackFrame->AddrPC.Offset, &dwLineOffset, &sourceInfo);

	if (result)
	{
		if (dwLineOffset)
		{
			xr_sprintf(formatBuff, _countof(formatBuff), " in %s line %u + %u byte(s)", sourceInfo.FileName,
				sourceInfo.LineNumber, dwLineOffset);
		}
		else
		{
			xr_sprintf(formatBuff, _countof(formatBuff), " in %s line %u", sourceInfo.FileName, sourceInfo.LineNumber);
		}
		frameStr.append(formatBuff);
	}

	return true;
}

bool xrDebug::InitializeSymbolEngine()
{
	if (!symEngineInitialized)
	{
		u32 dwOptions = SymGetOptions();
		SymSetOptions(dwOptions | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

		if (SymInitialize(GetCurrentProcess(), nullptr, TRUE))
		{
			symEngineInitialized = true;
		}
	}

	return symEngineInitialized;
}

void xrDebug::DeinitializeSymbolEngine(void)
{
	if (symEngineInitialized)
	{
		SymCleanup(GetCurrentProcess());

		symEngineInitialized = false;
	}
}

std::vector<std::string> xrDebug::BuildStackTrace(PCONTEXT threadCtx, int maxFramesCount)
{
	std::vector<std::string> traceResult;
	std::string frameStr;

	if (!InitializeSymbolEngine())
	{
		Msg("[xrDebug::BuildStackTrace]InitializeSymbolEngine failed with error: %d", GetLastError());
		return traceResult;
	}

	traceResult.reserve(maxFramesCount);

	STACKFRAME stackFrame{};
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrBStore.Mode = AddrModeFlat;

	// https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/ns-dbghelp-stackframe
	// https://github.com/reactos/reactos/blob/master/base/applications/drwtsn32/stacktrace.cpp
	stackFrame.AddrPC.Offset = threadCtx->Rip;
	stackFrame.AddrStack.Offset = threadCtx->Rsp;
	stackFrame.AddrFrame.Offset = threadCtx->Rbp;

	while (GetNextStackFrameString(&stackFrame, threadCtx, frameStr) && traceResult.size() <= maxFramesCount)
	{
		traceResult.push_back(frameStr);
	}

	DeinitializeSymbolEngine();

	return traceResult;
}

std::vector<std::string> xrDebug::BuildStackTrace(int maxFramesCount)
{
	CONTEXT currentThreadCtx = {};

	RtlCaptureContext(&currentThreadCtx); /// GetThreadContext can't be used on the current thread
	currentThreadCtx.ContextFlags = CONTEXT_FULL;

	return BuildStackTrace(&currentThreadCtx, maxFramesCount);
}

void xrDebug::LogStackTrace(const char* header)
{
	std::vector<std::string> stackTrace = BuildStackTrace(500);
	Msg("%s", header);
	for (const auto& frame : stackTrace)
	{
		Msg("%s", frame.c_str());
	}
}
