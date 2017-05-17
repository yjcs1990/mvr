/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrMutex.h
 > Description  : Classes for robot log
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/
#ifndef MVRMUTEX_H
#define MVRMUTEX_H

#if !ifdefined(WIN32) || defined(MINGW)
#include <pthread.h>
#endif
#include <string>
#include <mvriaTypedefs.h>

#endif  // MVRMUTEX_H