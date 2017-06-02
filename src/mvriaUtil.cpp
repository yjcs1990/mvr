/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : mvriaUtil.cpp
 > Description  : Some Utility classes and functions
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月02日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>  // for timeGetTime() and mmsystem.h
#include <mmsystem.h> // for teimGetTime()
#else
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <utime.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#include "mvriaInternal.h"
#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

#ifndef MVRINTERFACE
#include "MvrSick.h"
#include "MvrUrg.h"
#include "MvrLMS1XX.h"
#include "MvrS3Series.h"
#include "MvrUrg_2_0.h"
#include "MvrSZSeries.h"
#include "MvrSonarMTX.h"
#include "MvrBatteryMTX.h"
#include "MvrLCDMTX.h"
#endif // MVRINTERFACE

#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#endif // WIN32

#ifdef WIN32
#include <io.h>
MVREXPORT const char *MvrUtil::COM1  = "COM1";
MVREXPORT const char *MvrUtil::COM2  = "COM2";
MVREXPORT const char *MvrUtil::COM3  = "COM3";
MVREXPORT const char *MvrUtil::COM4  = "COM4";
MVREXPORT const char *MvrUtil::COM5  = "COM5";
MVREXPORT const char *MvrUtil::COM6  = "COM6";
MVREXPORT const char *MvrUtil::COM7  = "COM7";
MVREXPORT const char *MvrUtil::COM8  = "COM8";
MVREXPORT const char *MvrUtil::COM9  = "COM9";
MVREXPORT const char *MvrUtil::COM10 = "\\\\.\\COM10";
MVREXPORT const char *MvrUtil::COM11 = "\\\\.\\COM11";
MVREXPORT const char *MvrUtil::COM12 = "\\\\.\\COM12";
MVREXPORT const char *MvrUtil::COM13 = "\\\\.\\COM13";
MVREXPORT const char *MvrUtil::COM14 = "\\\\.\\COM14";
MVREXPORT const char *MvrUtil::COM15 = "\\\\.\\COM15";
MVREXPORT const char *MvrUtil::COM16 = "\\\\.\\COM16";
#else // ifndef WIN32
MVREXPORT const char *MvrUtil::COM1  = "/dev/ttyS0";
MVREXPORT const char *MvrUtil::COM2  = "/dev/ttyS1";
MVREXPORT const char *MvrUtil::COM3  = "/dev/ttyS2";
MVREXPORT const char *MvrUtil::COM4  = "/dev/ttyS3";
MVREXPORT const char *MvrUtil::COM5  = "/dev/ttyS4";
MVREXPORT const char *MvrUtil::COM6  = "/dev/ttyS5";
MVREXPORT const char *MvrUtil::COM7  = "/dev/ttyS6";
MVREXPORT const char *MvrUtil::COM8  = "/dev/ttyS7";
MVREXPORT const char *MvrUtil::COM9  = "/dev/ttyS8";
MVREXPORT const char *MvrUtil::COM10 = "/dev/ttyS9";
MVREXPORT const char *MvrUtil::COM11 = "/dev/ttyS10";
MVREXPORT const char *MvrUtil::COM12 = "/dev/ttyS11";
MVREXPORT const char *MvrUtil::COM13 = "/dev/ttyS12";
MVREXPORT const char *MvrUtil::COM14 = "/dev/ttyS13";
MVREXPORT const char *MvrUtil::COM15 = "/dev/ttyS14";
MVREXPORT const char *MvrUtil::COM16 = "/dev/ttyS15";
#endif  // WIN32

MVREXPORT const char *MvrUtil::TRUESTRING = "true";
MVREXPORT const char *MvrUtil::FALSESTRING = "false";

// const double eps = std::numeric_limits<double>::epsilon();
const double MvrMat::ourEpsilon = 0.00000001; 

#ifdef WIN32
// max returned by rand()
const long MvrMath::ourRandMax = RAND_MAX;
#else
// max returned by lrand48()
const long MvrMath::ourRandMax = 2147483648;// 2^31, per lrand48 man page
#endif // WIN32

#ifdef WIN32
const char  MvrUtil::SEPARATOR_CHAR = '\\';
const char *MvrUtil::SEPARATOR_STRING = "\\";
const char  MvrUtil::OTHER_SEPARATOR_CHAR = '/';
#else
const char  MvrUtil::SEPARATOR_CHAR = '/';
const char *MvrUtil::SEPARATOR_STRING = "/";
const char  MvrUtil::OTHER_SEPARATOR_CHAR = '\\';
#endif

#ifdef WIN32
MvrMutex MvrUtil::ourLocaltimeMutex;
#endif

/**
  Sleep (do nothing, without continuing the program) for @arg ms miliseconds.
  Use this to add idle time to threads, or in situations such as waiting for
  hardware with a known response time.  To perform actions at specific intervals,
  however, use the ArTime timer utility instead, or the ArUtil::getTime() method
  to check time.
  @param ms the number of milliseconds to sleep for
*/
MVREXPORT void MvrUtil::sleep(unsigned int ms)
{
#ifdef WIN32
  Sleep(ms);
#else
  if (ms > 10)
    ms -= 10;
  usleep(ms * 1000);
#endif // WIN32
}

MVREXPORT unsigned int MvrUtil::getTime(void)
{
// the unix way
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
  struct timespec tp;
  if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    return tp.tv_nsec / 1000000 + (tp.tv_sec % 1000000) * 1000;
// the old unix way
#endif
#if !defined(WIN32)
  struct timeval tv;
  if (gettimeofday)
}