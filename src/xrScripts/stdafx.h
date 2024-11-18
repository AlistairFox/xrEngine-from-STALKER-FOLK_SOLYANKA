#include <functional>
 
#include <algorithm>
#include <limits>
#include <vector>
#include <stack>
#include <list>
#include <set>
#include <map>

#include <string>

#ifndef _EDITOR
#	define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#	include <hash_map>
#	include <hash_set>
#endif


#include "xrScripts.h"
extern "C"
{
#include <lua\lua.h>
};