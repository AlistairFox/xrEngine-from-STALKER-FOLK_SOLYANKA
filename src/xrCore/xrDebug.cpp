#include "stdafx.h"

#define IXR_WINDOWS
 
#include "xrDebug.h"
#include "os_clipboard.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>

#if defined(IXR_WINDOWS) && !defined(IXR_ARM64)
#	include <direct.h>
#	include <dxerr.h>
#endif

#pragma warning(pop)

extern bool shared_str_initialized;

#define DEBUG_INVOKE	__debugbreak();
static BOOL bException = FALSE;
#define USE_OWN_ERROR_MESSAGE_WINDOW


#ifdef IXR_WINDOWS
#include <new.h>							// for _set_new_mode
#include <signal.h>							// for signals
#endif
#include <DbgHelp.h>

 

#	define USE_OWN_MINI_DUMP
 

XRCORE_API	xrDebug		Debug;

static bool	error_after_dialog = false;

void xrDebug::gather_info(const char* expression, const char* description, const char* argument0, const char* argument1, const char* file, int line, const char* function, LPSTR assertion_info, u32 const assertion_info_size)
{
	LPSTR				buffer_base = assertion_info;
	LPSTR				buffer = assertion_info;
	int assertion_size = (int)assertion_info_size;
	LPCSTR				endline = "\n";
	LPCSTR				prefix = "[error]";
	bool				extended_description = (description && !argument0 && strchr(description, '\n'));
	for (int i = 0; i < 2; ++i) {
		if (!i)
			buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sFATAL ERROR%s%s", endline, endline, endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sExpression    : %s%s", prefix, expression, endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sFunction      : %s%s", prefix, function, endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sFile          : %s%s", prefix, file, endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sLine          : %d%s", prefix, line, endline);

		if (extended_description) {
			buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s%s%s", endline, description, endline);
			if (argument0) {
				if (argument1) {
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s%s", argument0, endline);
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s%s", argument1, endline);
				}
				else
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s%s", argument0, endline);
			}
		}
		else {
			buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sDescription   : %s%s", prefix, description, endline);
			if (argument0) {
				if (argument1) {
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sArgument 0    : %s%s", prefix, argument0, endline);
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sArgument 1    : %s%s", prefix, argument1, endline);
				}
				else
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sArguments     : %s%s", prefix, argument0, endline);
			}
		}

		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s", endline);
		if (!i) {
			if (shared_str_initialized) {
				Msg("%s", assertion_info);
				FlushLog();
			}
			buffer = assertion_info;
			endline = "\r\n";
			prefix = "";
		}
	}

	if (!IsDebuggerPresent() && !strstr(GetCommandLineA(), "-no_call_stack_assert")) {
		if (shared_str_initialized)
			Msg("stack trace:\n");

#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "stack trace:%s%s", endline, endline);
#endif // USE_OWN_ERROR_MESSAGE_WINDOW

		if (shared_str_initialized)
			FlushLog();

		os_clipboard::copy_to_clipboard(assertion_info);
	}
}

void xrDebug::do_exit(const std::string& message)
{
	FlushLog();
// 	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message.c_str(), nullptr);
#ifdef IXR_WINDOWS
	TerminateProcess(GetCurrentProcess(), 1);
#else
	kill(getpid(), SIGKILL);
#endif
}

void xrDebug::backend(const char* expression, const char* description, const char* argument0, const char* argument1, const char* file, int line, const char* function, bool& ignore_always)
{
	static xrCriticalSection CS
#ifdef PROFILE_CRITICAL_SECTIONS
	(MUTEX_PROFILE_ID(xrDebug::backend))
#endif // PROFILE_CRITICAL_SECTIONS
		;

	CS.Enter();

	error_after_dialog = true;

	string4096			assertion_info;

	gather_info(expression, description, argument0, argument1, file, line, function, assertion_info, sizeof(assertion_info));

#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
	LPCSTR				endline = "\r\n";
	LPSTR				buffer = assertion_info + xr_strlen(assertion_info);
	buffer += xr_sprintf(buffer, sizeof(assertion_info) - u32(buffer - &assertion_info[0]), "%sPress CANCEL to abort execution%s", endline, endline);
	buffer += xr_sprintf(buffer, sizeof(assertion_info) - u32(buffer - &assertion_info[0]), "Press TRY AGAIN to continue execution%s", endline);
	buffer += xr_sprintf(buffer, sizeof(assertion_info) - u32(buffer - &assertion_info[0]), "Press CONTINUE to continue execution and ignore all the errors of this type%s%s", endline, endline);
#endif // USE_OWN_ERROR_MESSAGE_WINDOW
     
	//if (IsDebuggerPresent())
	//{
	//	DebugBreak();
	//}
 
	Msg("BackendError: \\n %s", buffer);

	CS.Leave();
}

LPCSTR xrDebug::error2string(long code)
{
	static char desc_storage[1024] = {};
#ifdef IXR_WINDOWS
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), desc_storage, 0, nullptr);
#endif
	return desc_storage;
}
 
void xrDebug::error(long hr, const char* expr, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(error2string(hr), expr, 0, 0, file, line, function, ignore_always);
}

void xrDebug::error(long hr, const char* expr, const char* e2, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(error2string(hr), expr, e2, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* file, int line, const char* function, bool& ignore_always)
{
	backend("assertion failed", e1, 0, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const std::string& e2, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(e1, e2.c_str(), 0, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* e2, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(e1, e2, 0, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* e2, const char* e3, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(e1, e2, e3, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* e2, const char* e3, const char* e4, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(e1, e2, e3, e4, file, line, function, ignore_always);
}
 
void __cdecl xrDebug::fatal(const char* file, int line, const char* function, const char* F, ...)
{
	string1024	buffer;

	va_list		p;
	va_start(p, F);
	vsprintf(buffer, F, p);
	va_end(p);

	bool		ignore_always = true;

	backend("fatal error", "<no expression>", buffer, 0, file, line, function, ignore_always);
}

typedef void (*full_memory_stats_callback_type) ();
XRCORE_API full_memory_stats_callback_type g_full_memory_stats_callback = 0;

int out_of_memory_handler(size_t size)
{
	if (g_full_memory_stats_callback)
		g_full_memory_stats_callback();
	else {
		Memory.mem_compact();

		u32					process_heap = mem_usage_impl(nullptr, nullptr, nullptr);
		int					eco_strings = (int)g_pStringContainer->stat_economy();
		int					eco_smem = (int)g_pSharedMemoryContainer->stat_economy();
		Msg("* [x-ray]: process heap[%d K]", process_heap / 1024);
		Msg("* [x-ray]: economy: strings[%d K], smem[%d K]", eco_strings / 1024, eco_smem);
	}

	Debug.fatal(DEBUG_INFO, "Out of memory. Memory request: %d K", size / 1024);
	return					1;
}

XRCORE_API string_path g_bug_report_file;

#if defined(IXR_WINDOWS)
typedef long WINAPI UnhandledExceptionFilterType(struct _EXCEPTION_POINTERS* pExceptionInfo);
typedef long (*PFNCHFILTFN) (EXCEPTION_POINTERS* pExPtrs);
extern "C" BOOL  SetCrashHandlerFilter(PFNCHFILTFN pFn);

static UnhandledExceptionFilterType* previous_filter = 0;

#ifdef USE_OWN_MINI_DUMP
typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

// TODO: windows specific stuff, Linux would require debugging tools and APIs like `libunwind`, `libbfd`, and `gdb`...
void save_mini_dump(_EXCEPTION_POINTERS* pExceptionInfo)
{
	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = nullptr;
	string_path		szDbgHelpPath;

	if (GetModuleFileNameA(nullptr, szDbgHelpPath, _MAX_PATH))
	{
		char* pSlash = strchr(szDbgHelpPath, '\\');
		if (pSlash)
		{
			xr_strcpy(pSlash + 1, sizeof(szDbgHelpPath) - (pSlash - szDbgHelpPath), "DBGHELP.DLL");
			hDll = ::LoadLibraryA(szDbgHelpPath);
		}
	}

	if (hDll == nullptr)
	{
		// load any version we can
		hDll = ::LoadLibraryA("DBGHELP.DLL");
	}

	const char* szResult = nullptr;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
		if (pDump)
		{
			string_path	szDumpPath;
			string_path	szScratch;
			string64	t_stemp;

			timestamp(t_stemp);
			xr_strcpy(szDumpPath, Core.ApplicationName);
			xr_strcat(szDumpPath, "_");
			xr_strcat(szDumpPath, Core.UserName);
			xr_strcat(szDumpPath, "_");
			xr_strcat(szDumpPath, t_stemp);
			xr_strcat(szDumpPath, ".mdmp");

			__try {
				if (FS.path_exist("$logs$"))
					FS.update_path(szDumpPath, "$logs$", szDumpPath);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				string_path	temp;
				xr_strcpy(temp, szDumpPath);
				xr_strcpy(szDumpPath, "logs/");
				xr_strcat(szDumpPath, temp);
			}

			// create the file
			HANDLE hFile = ::CreateFileA(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (INVALID_HANDLE_VALUE == hFile)
			{
				// try to place into current directory
				MoveMemory(szDumpPath, szDumpPath + 5, strlen(szDumpPath));
				hFile = ::CreateFileA(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			}
			if (hFile != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = false;

				// write the dump
				MINIDUMP_TYPE	dump_flags = MINIDUMP_TYPE(MiniDumpNormal | MiniDumpFilterMemory | MiniDumpScanMemory);

				BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dump_flags, &ExInfo, nullptr, nullptr);
				if (bOK)
				{
					xr_sprintf(szScratch, "Saved dump file to '%s'", szDumpPath);
					szResult = szScratch;
					//					retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					xr_sprintf(szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError());
					szResult = szScratch;
				}
				::CloseHandle(hFile);
			}
			else
			{
				xr_sprintf(szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError());
				szResult = szScratch;
			}
		}
		else
		{
			szResult = "DBGHELP.DLL too old";
		}
	}
	else
	{
		szResult = "DBGHELP.DLL not found";
	}
}
#endif // USE_OWN_MINI_DUMP

void format_message(LPSTR buffer, const u32& buffer_size)
{
	LPVOID		message;
	DWORD		error_code = GetLastError();

	if (!error_code) {
		*buffer = 0;
		return;
	}

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&message,
		0,
		nullptr
	);

	xr_sprintf(buffer, buffer_size, "[error][%8d]    : %s", error_code, message);
	LocalFree(message);
}

#ifndef _EDITOR
#include <errorrep.h>
#endif

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Faultrep.lib")

#include "StackTrace.h"

static bool EnabledStackTrace = true;

LONG WINAPI UnhandledFilter(_EXCEPTION_POINTERS* pExceptionInfo)
{
	/*
	string256				error_message;
	format_message(error_message, sizeof(error_message));
	
	if (EnabledStackTrace)
	{
		Msg("\n----------------------------------------------");
		Msg("stack trace DBG_HELP:\n");

		Debug.Callstack();

		CONTEXT save = *pExceptionInfo->ContextRecord;

		using namespace StackTrace;
		std::vector<std::string> stackTrace = BuildStackTrace(pExceptionInfo->ContextRecord, 1024);
		*pExceptionInfo->ContextRecord = save;
	
		string4096			buffer;
		Msg("\n----------------------------------------------");
		Msg("stack trace IXray: stack_size(%u) \n", stackTrace.size() );


		for (size_t i = 0; i < stackTrace.size(); i++)
		{
			Log(stackTrace[i].c_str());
			xr_sprintf(buffer, sizeof(buffer), "%s\r\n", stackTrace[i].c_str());
		}

		Msg("----------------------------------------------\n\n");

		if (*error_message)
		{
			if (shared_str_initialized)
				Msg("\n%s", error_message);

			xr_strcat(error_message, sizeof(error_message), "\r\n");
			os_clipboard::update_clipboard(buffer);
		}
	}
	*/

	if (Debug.debug_calltack_lua)
		Debug.debug_calltack_lua();

	if (shared_str_initialized)
		FlushLog();

 	save_mini_dump(pExceptionInfo);
 
	if (!error_after_dialog) {
		if (Debug.get_on_dialog())
			Debug.get_on_dialog()	(true);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

//////////////////////////////////////////////////////////////////////
void _terminate()
{
	if (strstr(GetCommandLineA(), "-silent_error_mode"))
		exit(-1);

	string4096				assertion_info;

	Debug.gather_info(
		//gather_info				(
		"<no expression>",
		"Unexpected application termination",
		0,
		0,
#ifdef ANONYMOUS_BUILD
		"",
		0,
#else
		__FILE__,
		__LINE__,
#endif
#ifndef _EDITOR
		__FUNCTION__,
#else // _EDITOR
		"",
#endif // _EDITOR
		assertion_info
	);

	LPCSTR endline = "\r\n";
	LPSTR buffer = assertion_info + xr_strlen(assertion_info);
	buffer += xr_sprintf(buffer, xr_strlen(assertion_info), "Press OK to abort execution%s", endline);

	Msg("Terminate: \\n %s", buffer);
	exit(-1);
}

#ifdef IXR_WINDOWS
IC void handler_base(const char* reason_string)
{
	bool skip;
	Debug.backend("Error handler is invoked!", reason_string, nullptr, nullptr, DEBUG_INFO, skip);
}

void invalid_parameter_handler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved)
{
	string4096	expression_,
		function_,
		file_;

	size_t converted_chars = 0;

	if (expression)
		wcstombs_s(&converted_chars, expression_, sizeof(expression_), expression, (wcslen(expression) + 1) * 2 * sizeof(char));
	else
		xr_strcpy(expression_, "");

	if (function)
		wcstombs_s(&converted_chars, function_, sizeof(function_), function, (wcslen(function) + 1) * 2 * sizeof(char));
	else
		xr_strcpy(function_, __FUNCTION__);

	if (file)
		wcstombs_s(&converted_chars, file_, sizeof(file_), file, (wcslen(file) + 1) * 2 * sizeof(char));
	else
	{
		line = __LINE__;
		xr_strcpy(file_, __FILE__);
	}

	bool skip;
	Debug.backend("Error handler is invoked!", expression_, nullptr, nullptr, file_, line, function_, skip);
}
#endif

void __cdecl debug_on_thread_spawn(void)
{
#ifdef IXR_WINDOWS
	SetUnhandledExceptionFilter(UnhandledFilter);

	auto abort_handler = [](int signal) { handler_base("Application is aborting"); };
	auto floating_point_handler = [](int signal) { handler_base("Floating point error"); };
	auto pure_call_handler = []() { handler_base("Pure virtual function call"); };
	auto illegal_instruction_handler = [](int signal) { handler_base("Illegal instruction"); };

	signal(SIGABRT, abort_handler);
	signal(SIGFPE, floating_point_handler);
	signal(SIGILL, illegal_instruction_handler);

	_set_invalid_parameter_handler(&invalid_parameter_handler);

	_set_new_mode(1);
	_set_new_handler(&out_of_memory_handler);

	_set_purecall_handler(pure_call_handler);
#endif
}

void xrDebug::_initialize(const bool& dedicated)
{
	static bool is_dedicated = dedicated;

	*g_bug_report_file = 0;
#ifdef IXR_WINDOWS
	previous_filter = ::SetUnhandledExceptionFilter(UnhandledFilter);	// exception handler to all "unhandled" exceptions
#endif
}

 
typedef USHORT(WINAPI* CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);

bool isInitialized = false;

void xrDebug::Callstack()
{
	//if (!isCallstackEnable)
	//	return;

	if (!isInitialized)
	{
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		isInitialized = SymInitialize(GetCurrentProcess(), 0, TRUE);
	}

	if (!isInitialized)
		return;

	CaptureStackBackTraceType func_callstack = (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));

	if (func_callstack != nullptr)
	{
		const int max_frames = 64;
		void* callstack[max_frames];

		USHORT frames = func_callstack(0, max_frames, callstack, NULL);
		SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);

		if (symbol != 0)
		{
			symbol->MaxNameLen = 255;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);


			for (USHORT i = 0; i < frames; ++i)
			{
				SymFromAddr(GetCurrentProcess(), (DWORD64)(callstack[i]), 0, symbol);

				IMAGEHLP_LINE64 data;

				DWORD dwDisplacement;
				SymGetLineFromAddr(GetCurrentProcess(), (DWORD64)(callstack[i]), &dwDisplacement, &data);

				DWORD_PTR displacement = 0;
				const DWORD max_name_len = 1024;
				char nameBuffer[max_name_len];
				UnDecorateSymbolName(symbol->Name, nameBuffer, max_name_len, UNDNAME_COMPLETE);

				Msg("callstack[%d]: SYMVOL:%s, LINE: %d, File: %s", i, nameBuffer, data.LineNumber, data.FileName != nullptr ? data.FileName : "no file");
			}

			free(symbol);
		}
	}


}