#include "stdafx.h"
#pragma hdrstop

#include	"xrsharedmem.h"
#include	<malloc.h>

xrMemory	Memory;
BOOL		mem_initialized	= FALSE;
bool		shared_str_initialized	= false;

//fake fix of memory corruptions in multiplayer game :(
XRCORE_API	bool	g_allow_heap_min = true;

// Processor specific implementations
void	__stdcall	xrMemFill_x86(void* dest, int value, u32 count) { memset(dest, int(value), count); };
void	__stdcall	xrMemFill32_x86(void* dest, u32 value, u32 count)
{
	u32* ptr = (u32*)dest;
	u32* end = ptr + count;
	for (; ptr != end; ) *ptr++ = value;
};
void	__stdcall	xrMemCopy_x86(void* dest, const void* src, u32 count) { memcpy(dest, src, count); };
 

xrMemory::xrMemory()
{
	mem_copy	= xrMemCopy_x86;
	mem_fill	= xrMemFill_x86;
	mem_fill32	= xrMemFill32_x86;
}

void	xrMemory::_initialize	(BOOL bDebug)
{
	stat_calls				= 0;
	stat_counter			= 0;
	 
	mem_copy	= xrMemCopy_x86;
	mem_fill	= xrMemFill_x86;
	mem_fill32	= xrMemFill32_x86;
 
 	if (!strstr(Core.Params,"-pure_alloc"))
	{
		// initialize POOLs
		u32	element		= mem_pools_ebase;
		u32 sector		= mem_pools_ebase*4096;
		for (u32 pid=0; pid<mem_pools_count; pid++)
		{
			mem_pools[pid]._initialize(element,sector,0x1);
			element		+=	mem_pools_ebase;
		}
	}
 
	mem_initialized				= TRUE;

	g_pStringContainer			= xr_new<str_container>		();
	shared_str_initialized		= true;
	g_pSharedMemoryContainer	= xr_new<smem_container>	();
}

void	xrMemory::_destroy()
{
	xr_delete					(g_pSharedMemoryContainer);
	xr_delete					(g_pStringContainer);
	mem_initialized				= FALSE;
}

void	xrMemory::mem_compact	()
{
	RegFlushKey						( HKEY_CLASSES_ROOT );
	RegFlushKey						( HKEY_CURRENT_USER );
	if (g_allow_heap_min)
		_heapmin					( );

	HeapCompact					(GetProcessHeap(),0);
	if (g_pStringContainer)			g_pStringContainer->clean		();
	if (g_pSharedMemoryContainer)	g_pSharedMemoryContainer->clean	();
	if (strstr(Core.Params,"-swap_on_compact"))
		SetProcessWorkingSetSize	(GetCurrentProcess(),size_t(-1),size_t(-1));
}

// xr_strdup
char*			xr_strdup		(const char* string)
{	
	VERIFY	(string);
	u32		len			= u32(xr_strlen(string))+1	;
	char *	memory		= (char*)	Memory.mem_alloc( len );
	CopyMemory		(memory,string,len);
	return	memory;
}

XRCORE_API		BOOL			is_stack_ptr		( void* _ptr)
{
	int			local_value		= 0;
	void*		ptr_refsound	= _ptr;
	void*		ptr_local		= &local_value;
	ptrdiff_t	difference		= (ptrdiff_t)_abs(s64(ptrdiff_t(ptr_local) - ptrdiff_t(ptr_refsound)));
	return		(difference < (512*1024));
}
