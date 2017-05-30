/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLog.cpp
 > Description  : Classes for robot log
 > Author       : Yu Jie
 > Create Time  : 2017年04月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLog.h"
#include "MvrConfig.h"
#include "mvriaInternal.h"

#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#ifdef WIN32
#include <io.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <execinfo.h>
#endif  // WIN32

#if defined(_ALT_VER) || defined(MVRIA_MSVC_ATL_VER)
#include <atlbase.h>
#define HAVEATL 1
#endif  // _ALT_VER

MvrMutex MvrLog::ourMutex;
MvrLog::LogType MvrLog::ourType = StdOut;
MvrLog::LogLevel MvrLog::ourLevel = MvrLog::Normal;
FILE *MvrLog::ourFP = 0;
int MvrLog::ourCharsLogged = 0;
std::string MvrLog::ourFileName;
int MvrLog::ourColbertStream = -1;
bool MvrLog::ourLoggingTime = false;
bool MvrLog::ourAlsoPrint = false;
MVREXPORT void (*MvrLog::colbertPrint)(int i, const char *str);

MvrLog::LogType MvrLog::ourConfigLogType = MvrLog::StdOut;
MvrLog::LogLevel MvrLog::ourConfigLogLevel = MvrLog::Normal;
char MvrLog::ourConfigFileName[1024] = "log.txt";
bool MvrLog::ourConfigLogTime = false;
bool MvrLog::ourConfigAlsoPrint = false;
MvrGlobalRetFunctor<bool> MvrLog::ourConfigProcessFileCB(&MvrLog::processFile);

#ifdef MVRINTERFACE
char MvrLog::ourAramConfigLogLevel[1024] = "Normal";
double MvrLog::ourAramConfigLogSize = 10;
bool MvrLog::ourUseAramBehavior = false;
double MvrLog::ourAramLogSize = 0;
MvrGlobalRetFunctor<bool> MvrLog::ourAramConfigProcessFileCB(&MvrLog::aramProcessFile);
std::string MvrLog::ourAramPrefix = "";
#endif  // MVRINTERFACE

bool MvrLog::ourAramDaemonized = false;

MvrFunctor1<const char *> *MvrLog::ourFunctor;

MVREXPORT void MvrLog::logPlain(LogLevel level, const char *str)
{
  log(level, str);
}

/**
   This function is used like printf(). If the supplied level is less than
   or equal to the set level, it will be printed.
   @param level level of logging
   @param str printf() like formating string
*/
MVREXPORT void MvrLog::log(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return ;
  // printf("logging %s\n", str);

  char buf[10000];
  char *bufPtr;
  char *timeStr;
  int timeLen = 0;  // this is a value based on the standard ctime return

  time_t now;

  ourMutex.lock();

  // put our time in if we want it
  if (ourLoggingTime)
  {
    now = time(NULL);
    timeStr = ctime(&now);
    timeLen = 28;
    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
  }
}

