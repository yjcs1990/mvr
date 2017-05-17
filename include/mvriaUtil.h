/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : mvriaUtil.h
 > Description  : Some Utility classes and functions
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/

#ifndef MVRIAUTIL_H
#define MVRIAUTIL_H

#define _GUN_SOURCE 1
#include <string>
// #define _XOPEN_SOURCE 500
#include <list>
#include <map>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <float.h>
#include <vector>

#if defined(_WIN32) || defined(WIN32)
#include <sys/timeb.h>
#include <sys/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <strings.h>
#endif // ifndef win32
#include <time.h>
#include "mvriaTypedefs.h"
#include "MvrLog.h"
#include "MvrFunctor.h"
#include "MvrArgumentParser.h"
#endif  // MVRIAUTIL_H