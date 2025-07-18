/*
** State and stack handling.
** Copyright (C) 2005-2021 Mike Pall. See Copyright Notice in luajit.h
**
** Portions taken verbatim or adapted from the Lua interpreter.
** Copyright (C) 1994-2008 Lua.org, PUC-Rio. See Copyright Notice in lua.h
*/

#define lj_state_c
#define LUA_CORE

#include "lj_obj.h"
#include "lj_gc.h"
#include "lj_err.h"
#include "lj_buf.h"
#include "lj_str.h"
#include "lj_tab.h"
#include "lj_func.h"
#include "lj_meta.h"
#include "lj_state.h"
#include "lj_frame.h"
#if LJ_HASFFI
#include "lj_ctype.h"
#endif
#include "lj_trace.h"
#include "lj_dispatch.h"
#include "lj_vm.h"
#include "lj_prng.h"
#include "lj_lex.h"
#include "lj_alloc.h"
#include "luajit.h"

/* -- Stack handling ------------------------------------------------------ */

/* Stack sizes. */
#define LJ_STACK_MIN	LUA_MINSTACK	/* Min. stack size. */
#define LJ_STACK_MAX	LUAI_MAXSTACK	/* Max. stack size. */
#define LJ_STACK_START	(2*LJ_STACK_MIN)	/* Starting stack size. */
#define LJ_STACK_MAXEX	(LJ_STACK_MAX + 1 + LJ_STACK_EXTRA)

/* Explanation of LJ_STACK_EXTRA:
**
** Calls to metamethods store their arguments beyond the current top
** without checking for the stack limit. This avoids stack resizes which
** would invalidate passed TValue pointers. The stack check is performed
** later by the function header. This can safely resize the stack or raise
** an error. Thus we need some extra slots beyond the current stack limit.
**
** Most metamethods need 4 slots above top (cont, mobj, arg1, arg2) plus
** one extra slot if mobj is not a function. Only lj_meta_tset needs 5
** slots above top, but then mobj is always a function. So we can get by
** with 5 extra slots.
** LJ_FR2: We need 2 more slots for the frame PC and the continuation PC.
*/

/* Resize stack slots and adjust pointers in state. */
static void resizestack(lua_State *L, MSize n)
{
  TValue *st, *oldst = tvref(L->stack);
  ptrdiff_t delta;
  MSize oldsize = L->stacksize;
  MSize realsize = n + 1 + LJ_STACK_EXTRA;
  GCobj *up;
  lj_assertL((MSize)(tvref(L->maxstack)-oldst) == L->stacksize-LJ_STACK_EXTRA-1,
	     "inconsistent stack size");
  st = (TValue *)lj_mem_realloc(L, tvref(L->stack),
				(MSize)(oldsize*sizeof(TValue)),
				(MSize)(realsize*sizeof(TValue)));
  setmref(L->stack, st);
  delta = (char *)st - (char *)oldst;
  setmref(L->maxstack, st + n);
  while (oldsize < realsize)  /* Clear new slots. */
    setnilV(st + oldsize++);
  L->stacksize = realsize;
  if ((size_t)(mref(G(L)->jit_base, char) - (char *)oldst) < oldsize)
    setmref(G(L)->jit_base, mref(G(L)->jit_base, char) + delta);
  L->base = (TValue *)((char *)L->base + delta);
  L->top = (TValue *)((char *)L->top + delta);
  for (up = gcref(L->openupval); up != NULL; up = gcnext(up))
    setmref(gco2uv(up)->v, (TValue *)((char *)uvval(gco2uv(up)) + delta));
}

/* Relimit stack after error, in case the limit was overdrawn. */
void lj_state_relimitstack(lua_State *L)
{
  if (L->stacksize > LJ_STACK_MAXEX && L->top-tvref(L->stack) < LJ_STACK_MAX-1)
    resizestack(L, LJ_STACK_MAX);
}

/* Try to shrink the stack (called from GC). */
void lj_state_shrinkstack(lua_State *L, MSize used)
{
  if (L->stacksize > LJ_STACK_MAXEX)
    return;  /* Avoid stack shrinking while handling stack overflow. */
  if (4*used < L->stacksize &&
      2*(LJ_STACK_START+LJ_STACK_EXTRA) < L->stacksize &&
      /* Don't shrink stack of live trace. */
      (tvref(G(L)->jit_base) == NULL || obj2gco(L) != gcref(G(L)->cur_L)))
    resizestack(L, L->stacksize >> 1);
}

/* Try to grow stack. */
void LJ_FASTCALL lj_state_growstack(lua_State *L, MSize need)
{
  MSize n;
  if (L->stacksize > LJ_STACK_MAXEX)  /* Overflow while handling overflow? */
    lj_err_throw(L, LUA_ERRERR);
  n = L->stacksize + need;
  if (n > LJ_STACK_MAX) {
    n += 2*LUA_MINSTACK;
  } else if (n < 2*L->stacksize) {
    n = 2*L->stacksize;
    if (n >= LJ_STACK_MAX)
      n = LJ_STACK_MAX;
  }
  resizestack(L, n);
  if (L->stacksize > LJ_STACK_MAXEX)
    lj_err_msg(L, LJ_ERR_STKOV);
}

void LJ_FASTCALL lj_state_growstack1(lua_State *L)
{
  lj_state_growstack(L, 1);
}

/* Allocate basic stack for new state. */
static void stack_init(lua_State *L1, lua_State *L)
{
  TValue *stend, *st = lj_mem_newvec(L, LJ_STACK_START+LJ_STACK_EXTRA, TValue);
  setmref(L1->stack, st);
  L1->stacksize = LJ_STACK_START + LJ_STACK_EXTRA;
  stend = st + L1->stacksize;
  setmref(L1->maxstack, stend - LJ_STACK_EXTRA - 1);
  setthreadV(L1, st++, L1);  /* Needed for curr_funcisL() on empty stack. */
  if (LJ_FR2) setnilV(st++);
  L1->base = L1->top = st;
  while (st < stend)  /* Clear new slots. */
    setnilV(st++);
}

/* -- State handling ------------------------------------------------------ */

/* Open parts that may cause memory-allocation errors. */
static TValue *cpluaopen(lua_State *L, lua_CFunction dummy, void *ud)
{
  global_State *g = G(L);
  UNUSED(dummy);
  UNUSED(ud);
  stack_init(L, L);
  /* NOBARRIER: State initialization, all objects are white. */
  setgcref(L->env, obj2gco(lj_tab_new(L, 0, LJ_MIN_GLOBAL)));
  settabV(L, registry(L), lj_tab_new(L, 0, LJ_MIN_REGISTRY));
  lj_str_resize(L, LJ_MIN_STRTAB-1);
  lj_meta_init(L);
  lj_lex_init(L);
  fixstring(lj_err_str(L, LJ_ERR_ERRMEM));  /* Preallocate memory error msg. */
  g->gc.threshold = 4*g->gc.total;
  lj_trace_initstate(g);
  return NULL;
}

static void close_state(lua_State *L)
{
  global_State *g = G(L);
  lj_func_closeuv(L, tvref(L->stack));
  lj_gc_freeall(g);
  lj_assertG(gcref(g->gc.root) == obj2gco(L),
	     "main thread is not first GC object");
  lj_assertG(g->strnum == 0, "leaked %d strings", g->strnum);
  lj_trace_freestate(g);
#if LJ_HASFFI
  lj_ctype_freestate(g);
#endif
  lj_mem_freevec(g, g->strhash, g->strmask+1, GCRef);
  lj_buf_free(g, &g->tmpbuf);
  lj_mem_freevec(g, tvref(L->stack), L->stacksize, TValue);
#if LJ_64
  if (mref(g->gc.lightudseg, uint32_t)) {
    MSize segnum = g->gc.lightudnum ? (2 << lj_fls(g->gc.lightudnum)) : 2;
    lj_mem_freevec(g, mref(g->gc.lightudseg, uint32_t), segnum, uint32_t);
  }
#endif
  lj_assertG(g->gc.total == sizeof(GG_State),
	     "memory leak of %lld bytes",
	     (long long)(g->gc.total - sizeof(GG_State)));
#ifndef LUAJIT_USE_SYSMALLOC
  if (g->allocf == lj_alloc_f)
    lj_alloc_destroy(g->allocd);
  else
#endif
    g->allocf(g->allocd, G2GG(g), sizeof(GG_State), 0);
}

#if LJ_64 && !LJ_GC64 && !(defined(LUAJIT_USE_VALGRIND) && defined(LUAJIT_USE_SYSMALLOC))
lua_State *lj_state_newstate(lua_Alloc allocf, void *allocd)
#else
LUA_API lua_State *lua_newstate(lua_Alloc allocf, void *allocd)
#endif
{
  PRNGState prng;
  GG_State *GG;
  lua_State *L;
  global_State *g;
  /* We need the PRNG for the memory allocator, so initialize this first. */
  if (!lj_prng_seed_secure(&prng)) {
    lj_assertX(0, "secure PRNG seeding failed");
    /* Can only return NULL here, so this errors with "not enough memory". */
    return NULL;
  }
#ifndef LUAJIT_USE_SYSMALLOC
  if (allocf == LJ_ALLOCF_INTERNAL) {
    allocd = lj_alloc_create(&prng);
    if (!allocd) return NULL;
    allocf = lj_alloc_f;
  }
#endif
  GG = (GG_State *)allocf(allocd, NULL, 0, sizeof(GG_State));
  if (GG == NULL || !checkptrGC(GG)) return NULL;
  memset(GG, 0, sizeof(GG_State));
  L = &GG->L;
  g = &GG->g;
  L->gct = ~LJ_TTHREAD;
  L->marked = LJ_GC_WHITE0 | LJ_GC_FIXED | LJ_GC_SFIXED;  /* Prevent free. */
  L->dummy_ffid = FF_C;
  setmref(L->glref, g);
  g->gc.currentwhite = LJ_GC_WHITE0 | LJ_GC_FIXED;
  g->strempty.marked = LJ_GC_WHITE0;
  g->strempty.gct = ~LJ_TSTR;
  g->allocf = allocf;
  g->allocd = allocd;
  g->prng = prng;
#ifndef LUAJIT_USE_SYSMALLOC
  if (allocf == lj_alloc_f) {
    lj_alloc_setprng(allocd, &g->prng);
  }
#endif
  setgcref(g->mainthref, obj2gco(L));
  setgcref(g->uvhead.prev, obj2gco(&g->uvhead));
  setgcref(g->uvhead.next, obj2gco(&g->uvhead));
  g->strmask = ~(MSize)0;
  setnilV(registry(L));
  setnilV(&g->nilnode.val);
  setnilV(&g->nilnode.key);
#if !LJ_GC64
  setmref(g->nilnode.freetop, &g->nilnode);
#endif
  lj_buf_init(NULL, &g->tmpbuf);
  g->gc.state = GCSpause;
  setgcref(g->gc.root, obj2gco(L));
  setmref(g->gc.sweep, &g->gc.root);
  g->gc.total = sizeof(GG_State);
  g->gc.pause = LUAI_GCPAUSE;
  g->gc.stepmul = LUAI_GCMUL;
  lj_dispatch_init((GG_State *)L);
  L->status = LUA_ERRERR+1;  /* Avoid touching the stack upon memory error. */
  if (lj_vm_cpcall(L, NULL, NULL, cpluaopen) != 0) {
    /* Memory allocation error: free partial state. */
    close_state(L);
    return NULL;
  }
  L->status = LUA_OK;
  return L;
}

static TValue *cpfinalize(lua_State *L, lua_CFunction dummy, void *ud)
{
  UNUSED(dummy);
  UNUSED(ud);
  lj_gc_finalize_cdata(L);
  lj_gc_finalize_udata(L);
  /* Frame pop omitted. */
  return NULL;
}

LUA_API void lua_close(lua_State *L)
{
  global_State *g = G(L);
  int i;
  L = mainthread(g);  /* Only the main thread can be closed. */
#if LJ_HASPROFILE
  luaJIT_profile_stop(L);
#endif
  setgcrefnull(g->cur_L);
  lj_func_closeuv(L, tvref(L->stack));
  lj_gc_separateudata(g, 1);  /* Separate udata which have GC metamethods. */
#if LJ_HASJIT
  G2J(g)->flags &= ~JIT_F_ON;
  G2J(g)->state = LJ_TRACE_IDLE;
  lj_dispatch_update(g);
#endif
  for (i = 0;;) {
    hook_enter(g);
    L->status = LUA_OK;
    L->base = L->top = tvref(L->stack) + 1 + LJ_FR2;
    L->cframe = NULL;
    if (lj_vm_cpcall(L, NULL, NULL, cpfinalize) == LUA_OK) {
      if (++i >= 10) break;
      lj_gc_separateudata(g, 1);  /* Separate udata again. */
      if (gcref(g->gc.mmudata) == NULL)  /* Until nothing is left to do. */
	break;
    }
  }
  close_state(L);
}

lua_State *lj_state_new(lua_State *L)
{
  lua_State *L1 = lj_mem_newobj(L, lua_State);
  L1->gct = ~LJ_TTHREAD;
  L1->dummy_ffid = FF_C;
  L1->status = LUA_OK;
  L1->stacksize = 0;
  setmref(L1->stack, NULL);
  L1->cframe = NULL;
  /* NOBARRIER: The lua_State is new (marked white). */
  setgcrefnull(L1->openupval);
  setmrefr(L1->glref, L->glref);
  setgcrefr(L1->env, L->env);
  stack_init(L1, L);  /* init stack */
  lj_assertL(iswhite(obj2gco(L1)), "new thread object is not white");
  return L1;
}

void LJ_FASTCALL lj_state_free(global_State *g, lua_State *L)
{
  lj_assertG(L != mainthread(g), "free of main thread");
  if (obj2gco(L) == gcref(g->cur_L))
    setgcrefnull(g->cur_L);
  lj_func_closeuv(L, tvref(L->stack));
  lj_assertG(gcref(L->openupval) == NULL, "stale open upvalues");
  lj_mem_freevec(g, tvref(L->stack), L->stacksize, TValue);
  lj_mem_freet(g, L);
}

