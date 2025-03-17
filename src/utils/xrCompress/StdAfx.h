#pragma once

//. #define MOD_COMPRESS

#include "../../xrcore/xrCore.h"
#pragma warning(disable: 4995)
#pragma warning(disable: 4996)
#include "lzo\lzo1x.h"
#include <mmsystem.h>


#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <malloc.h>
 
#pragma comment	(lib,"xrCore.lib")
#pragma comment	(lib,"winmm")