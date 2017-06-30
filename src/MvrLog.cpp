/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

#include "MvrExport.h"
#include "ariaOSDef.h"
#include "MvrLog.h"
#include "MvrConfig.h"
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include "ariaInternal.h"


#ifdef WIN32
#include <io.h>
#else
#include <fcntl.h>
#include <errno.h>
#include <execinfo.h>
#endif

#if defined(_ATL_VER) || defined(ARIA_MSVC_ATL_VER)
#include <atlbase.h>
#define HAVEATL 1
#endif // ifdef _ATL_VER

ArMutex MvrLog::ourMutex;
ArLog::LogType MvrLog::ourType=StdOut;
ArLog::LogLevel MvrLog::ourLevel=ArLog::Normal;
FILE * MvrLog::ourFP=0;
int MvrLog::ourCharsLogged = 0;
std::string MvrLog::ourFileName;
int MvrLog::ourColbertStream = -1;
bool MvrLog::ourLoggingTime = false;
bool MvrLog::ourAlsoPrint = false;
MVREXPORT void (* MvrLog::colbertPrint)(int i, const char *str);

ArLog::LogType MvrLog::ourConfigLogType = MvrLog::StdOut;
ArLog::LogLevel MvrLog::ourConfigLogLevel = MvrLog::Normal;
char MvrLog::ourConfigFileName[1024] = "log.txt";
bool MvrLog::ourConfigLogTime = false;
bool MvrLog::ourConfigAlsoPrint = false;
ArGlobalRetFunctor<bool> MvrLog::ourConfigProcessFileCB(&ArLog::processFile);

#ifndef ARINTERFACE
char MvrLog::ourAramConfigLogLevel[1024] = "Normal";
double MvrLog::ourAramConfigLogSize = 10;
bool MvrLog::ourUseAramBehavior = false;
double MvrLog::ourAramLogSize = 0;
ArGlobalRetFunctor<bool> MvrLog::ourAramConfigProcessFileCB(
	&ArLog::aramProcessFile);
std::string MvrLog::ourAramPrefix = "";
#endif
bool MvrLog::ourAramDaemonized = false;

ArFunctor1<const char *> *ArLog::ourFunctor;


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
    return;
  
  //printf("logging %s\n", str);

  char buf[10000];
  char *bufPtr;
  char *timeStr;
  int timeLen = 0; // this is a value based on the standard length of
                       // ctime return
  time_t now;


  ourMutex.lock();
  // put our time in if we want it
  if (ourLoggingTime)
  {
    now = time(NULL);
    timeStr = ctime(&now);
    timeLen = 20;
    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
    bufPtr = buf;
  va_list ptr;
  va_start(ptr, str);
  
  vsnprintf(bufPtr, sizeof(buf) - timeLen - 2, str, ptr);
  bufPtr[sizeof(buf) - timeLen - 1] = '\0';
  //vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourType == Colbert)
  {
    if (colbertPrint)		// check if we have a print routine
      (*colbertPrint)(ourColbertStream, buf);
  }
  else if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", buf)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    checkFileSize();
  }
  else if (ourType != None)
  {
    printf("%s\n", buf);
    fflush(stdout);
  }
  if (ourAlsoPrint)
    printf("%s\n", buf);

  invokeFunctor(buf);

#ifndef ARINTERFACE
  // check this down here instead of up in the if ourFP so that the log filled shows up after the printf
  if (ourUseAramBehavior && ourFP && ourAramLogSize > 0 && 
      ourCharsLogged > ourAramLogSize)
  {
    filledAramLog();
  }
#endif // ARINTERFACE

// Also send it to the VC++ debug output window...
#ifdef HAVEATL
  ATLTRACE2("%s\n", buf);
#endif
  
  va_end(ptr);
  ourMutex.unlock();
}

/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
MVREXPORT void MvrLog::logErrorFromOSPlain(LogLevel level, const char *str)
{
  logErrorFromOS(level, str);
}

/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
MVREXPORT void MvrLog::logErrorFromOS(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return;

#ifndef WIN32
  int err = errno;
#else
  DWORD err = GetLastError();
#endif 

  //printf("logging %s\n", str);

  char buf[10000];
  char *bufPtr;
  char *timeStr;
  int timeLen = 0; // this is a value based on the standard length of
                       // ctime return
  time_t now;


  ourMutex.lock();
  // put our time in if we want it
  if (ourLoggingTime)
  {
    now = time(NULL);
    timeStr = ctime(&now);
    timeLen = 20;
    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
    bufPtr = buf;
  va_list ptr;
  va_start(ptr, str);
  
  vsnprintf(bufPtr, sizeof(buf) - timeLen - 2, str, ptr);
  bufPtr[sizeof(buf) - timeLen - 1] = '\0';


  char bufWithError[10200];  

#ifndef WIN32
  const char *errorString = NULL;
  if (err < sys_nerr - 1)
    errorString = sys_errlist[err];
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';
#else
  LPVOID errorString = NULL;

  FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &errorString,
        0, NULL);
 
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, (char*)errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';

  LocalFree(errorString);
#endif

  //vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourType == Colbert)
  {
    if (colbertPrint)		// check if we have a print routine
      (*colbertPrint)(ourColbertStream, bufWithError);
  }
  else if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", bufWithError)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    checkFileSize();
  }
  else if (ourType != None)
  {
    printf("%s\n", bufWithError);
    fflush(stdout);
  }
  if (ourAlsoPrint)
    printf("%s\n", bufWithError);

  invokeFunctor(bufWithError);

#ifndef ARINTERFACE
  // check this down here instead of up in the if ourFP so that the log filled shows up after the printf
  if (ourUseAramBehavior && ourFP && ourAramLogSize > 0 && 
      ourCharsLogged > ourAramLogSize)
  {
    filledAramLog();
  }
#endif // ARINTERFACE

// Also send it to the VC++ debug output window...
#ifdef HAVEATL
  ATLTRACE2("%s\n", buf);
#endif
  
  va_end(ptr);
  ourMutex.unlock();
}


/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
MVREXPORT void MvrLog::logErrorFromOSPlainNoLock(LogLevel level, const char *str)
{
  logErrorFromOSNoLock(level, str);
}

/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
MVREXPORT void MvrLog::logErrorFromOSNoLock(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return;

#ifndef WIN32
  int err = errno;
#else
  DWORD err = GetLastError();
#endif 

  //printf("logging %s\n", str);

  char buf[10000];
  char *bufPtr;
  char *timeStr;
  int timeLen = 0; // this is a value based on the standard length of
                       // ctime return
  time_t now;


  // put our time in if we want it
  if (ourLoggingTime)
  {
    now = time(NULL);
    timeStr = ctime(&now);
    timeLen = 20;
    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
    bufPtr = buf;
  va_list ptr;
  va_start(ptr, str);
  
  vsnprintf(bufPtr, sizeof(buf) - timeLen - 2, str, ptr);
  bufPtr[sizeof(buf) - timeLen - 1] = '\0';


  char bufWithError[10200];  

#ifndef WIN32
  const char *errorString = NULL;
  if (err < sys_nerr - 1)
    errorString = sys_errlist[err];
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';
#else
  LPVOID errorString = NULL;

  FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &errorString,
        0, NULL);
 
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';

  LocalFree(errorString);
#endif

  //vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourType == Colbert)
  {
    if (colbertPrint)		// check if we have a print routine
      (*colbertPrint)(ourColbertStream, bufWithError);
  }
  else if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", bufWithError)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    checkFileSize();
  }
  else if (ourType != None)
  {
    printf("%s\n", bufWithError);
    fflush(stdout);
  }
  if (ourAlsoPrint)
    printf("%s\n", bufWithError);

  invokeFunctor(bufWithError);

#ifndef ARINTERFACE
  // check this down here instead of up in the if ourFP so that the log filled shows up after the printf
  if (ourUseAramBehavior && ourFP && ourAramLogSize > 0 && 
      ourCharsLogged > ourAramLogSize)
  {
    filledAramLog();
  }
#endif // ARINTERFACE

// Also send it to the VC++ debug output window...
#ifdef HAVEATL
  ATLTRACE2("%s\n", buf);
#endif
  
  va_end(ptr);
}

/**
   Initialize the logging utility by supplying the type of logging and the
   level of logging. If the type is File, the fileName needs to be supplied.
   @param type type of Logging
   @param level level of logging
   @param fileName the name of the file for File type of logging
   @param logTime if this is true then the time a message is given will be logged
   @param alsoPrint if this is true then in addition to whatever other logging (to a file for instance) the results will also be printed
   @param printThisCall if this is true the new settings will be printed otherwise they won't
*/
MVREXPORT bool MvrLog::init(LogType type, LogLevel level, const char *fileName,
			  bool logTime, bool alsoPrint, bool printThisCall)
{
  ourMutex.setLogName("MvrLog::ourMutex");

  ourMutex.lock();
  
  // if we weren't or won't be doing a file then close any old file
  if (ourType != File || type != File)
  {
    close();
  }
  
  if (type == StdOut)
    ourFP=stdout;
  else if (type == StdErr)
    ourFP=stderr;
  else if (type == File)
  {
    if (fileName != NULL)
    {
      if (strcmp(ourFileName.c_str(), fileName) == 0)
      {
	ArLog::logNoLock(MvrLog::Terse, "MvrLog::init: Continuing to log to the same file.");
      }
      else
      {
	close();
	if ((ourFP = MvrUtil::fopen(fileName, "w")) == NULL)
	{
	  MvrLog::logNoLock(MvrLog::Terse, "MvrLog::init: Could not open file %s for logging.", fileName);
	  ourMutex.unlock();
	  return(false);
	}
	ourFileName=fileName;
      }
    }
  }
  else if (type == Colbert)
  {
    colbertPrint = NULL;
    ourColbertStream = -1;	// use default stream
    if (fileName)
    {  // need to translate fileName to integer index
    }
  }
  else if (type == None)
  {

  }
  ourType=type;
  ourLevel=level;

  // environment variable overrides level
  {
    char* lev = getenv("ARLOG_LEVEL");
    if(lev)
    {
      switch(toupper(lev[0]))
      {
        case 'N':
          ourLevel = Normal;
          break;
        case 'T':
          ourLevel = Terse;
          break;
        case 'V':
          ourLevel = Verbose;
          break;
       }
    }
  }

  ourLoggingTime = logTime;
  ourAlsoPrint = alsoPrint;

  if (printThisCall)
  {
    printf("MvrLog::init: ");
    
    if (ourType == StdOut)
      printf(" StdOut\t");
    else if (ourType == StdErr)
      printf(" StdErr\t");
    else if (ourType == File)
      printf(" File(%s)\t", ourFileName.c_str());
    else if (ourType == Colbert)
      printf(" Colbert\t");
    else if (ourType == None)
      printf(" None\t");
    else
      printf(" BadType\t");
    
    if (ourLevel == Terse)
      printf(" Terse\t");
    else if (ourLevel == Normal)
      printf(" Normal\t");
    else if (ourLevel == Verbose)
      printf(" Verbose\t");
    else
      printf(" BadLevel\t");
    
    if (ourLoggingTime)
      printf(" Logging Time\t");
    else
      printf(" Not logging time\t");
    
    if (ourAlsoPrint)
      printf(" Also printing\n");
    else
      printf(" Not also printing\n");
  }
  ourMutex.unlock();
  return(true);
}

MVREXPORT void MvrLog::close()
{
  if (ourFP && (ourType == File))
  {
    fclose(ourFP);
    ourFP=0;
    ourFileName="";
  }
}

MVREXPORT void MvrLog::logNoLock(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return;

  char buf[2048];
  char *bufPtr;
  char *timeStr;
  int timeLen = 20; // this is a value based on the standard length of
                       // ctime return
  time_t now;

  
  // put our time in if we want it
  if (ourLoggingTime)
  {
    now = time(NULL);
    timeStr = ctime(&now);
    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
    bufPtr = buf;
  va_list ptr;
  va_start(ptr, str);
  //vsnprintf(bufPtr, sizeof(buf) - timeLen - 1, str, ptr);
  vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourType == Colbert)
  {
    if (colbertPrint)		// check if we have a print routine
      (*colbertPrint)(ourColbertStream, buf);
  }
  else if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", buf)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    checkFileSize();
  }
  else if (ourType != None)
    printf("%s\n", buf);
  if (ourAlsoPrint)
    printf("%s\n", buf);
  
  invokeFunctor(buf);

  va_end(ptr);
}

MVREXPORT void MvrLog::logBacktrace(LogLevel level)
{
#ifndef WIN32
  int size = 100;
  int numEntries;
  void *buffer[size];
  char **names;
  
  numEntries = backtrace(buffer, size);
  MvrLog::log(MvrLog::Normal, "Backtrace %d levels", numEntries);
  
  names = backtrace_symbols(buffer, numEntries);
  if (names == NULL)
    return;
  
  int i;
  for (i = 0; i < numEntries; i++)
    MvrLog::log(MvrLog::Normal, "%s", names[i]);
  
  free(names);
#endif
}

/// Log a file if it exists
MVREXPORT bool MvrLog::logFileContents(LogLevel level, const char *fileName)
{
  FILE *strFile;
  unsigned int i;
  char str[100000];
  
  str[0] = '\0';
  
  if ((strFile = MvrUtil::fopen(fileName, "r")) != NULL)
  {
    while (fgets(str, sizeof(str), strFile) != NULL)
    {
      bool endedLine = false;
      for (i = 0; i < sizeof(str) && !endedLine; i++)
      {
	if (str[i] == '\r' || str[i] == '\n' || str[i] == '\0')
	{
	  str[i] = '\0';
	  MvrLog::log(level, str);
	  endedLine = true;
	}
      }
    }
    fclose(strFile);
    return true;
  }
  else
  {
    return false;
  }
}

MVREXPORT void MvrLog::addToConfig(MvrConfig *config)
{
  std::string section = "LogConfig";
  config->addParam(
	  MvrConfigArg("LogType", (int *)&ourConfigLogType,
		      "The type of log we'll be using, 0 for StdOut, 1 for StdErr, 2 for File (and give it a file name), 3 for colbert (don't use that), and 4 for None", 
		      MvrLog::StdOut, MvrLog::None), 
	  section.c_str(), MvrPriority::TRIVIAL);
  config->addParam(
	  MvrConfigArg("LogLevel", (int *)&ourConfigLogLevel,
		      "The level of logging to do, 0 for Terse, 1 for Normal, and 2 for Verbose", 
		      MvrLog::Terse, MvrLog::Verbose), 
	  section.c_str(), MvrPriority::TRIVIAL);
  config->addParam(
	  MvrConfigArg("LogFileName", ourConfigFileName,
		      "File to log to", sizeof(ourConfigFileName)),
	  section.c_str(), MvrPriority::TRIVIAL);
  config->addParam(
	  MvrConfigArg("LogTime", &ourConfigLogTime,
		      "True to prefix log messages with time and date, false not to"),
	  section.c_str(), MvrPriority::TRIVIAL);
  config->addParam(
	  MvrConfigArg("LogAlsoPrint", &ourConfigAlsoPrint,
		      "True to also printf the message, false not to"),	  
	  section.c_str(), MvrPriority::TRIVIAL);
  ourConfigProcessFileCB.setName("MvrLog");
  config->addProcessFileCB(&ourConfigProcessFileCB, 200);
}

MVREXPORT bool MvrLog::processFile(void)
{
  if (ourConfigLogType != ourType || ourConfigLogLevel != ourLevel ||
      strcmp(ourConfigFileName, ourFileName.c_str()) != 0 || 
      ourConfigLogTime != ourLoggingTime || ourConfigAlsoPrint != ourAlsoPrint)
  {
    MvrLog::logNoLock(MvrLog::Normal, "Initializing log from config");
    return MvrLog::init(ourConfigLogType, ourConfigLogLevel, ourConfigFileName, 
		       ourConfigLogTime, ourConfigAlsoPrint, true);
  }
  return true;
}

#ifndef ARINTERFACE
MVREXPORT void MvrLog::aramInit(const char *prefix, MvrLog::LogLevel defaultLevel,
			      double defaultSize, bool daemonized)
{
  if (prefix == NULL || prefix[0] == '\0')
    ourAramPrefix = "";
  else
  {
    ourAramPrefix = prefix;
    if (prefix[strlen(prefix) - 1] != '/')
      ourAramPrefix += "/";
  }
  
  std::string section = "Log Config";
  Mvria::getConfig()->addParam(
	  MvrConfigArg("Level", ourAramConfigLogLevel,
		      "The level of logging type of log we'll be using", sizeof(ourAramConfigLogLevel)),
	  section.c_str(), MvrPriority::TRIVIAL, 
	  "Choices:Terse;;Normal;;Verbose");
  Mvria::getConfig()->addParam(
	  MvrConfigArg("LogFileSize", &ourAramConfigLogSize,
		      "The maximum size of the log files (6 files are rotated through), 0 means no maximum", 0, 20000),
	  section.c_str(), MvrPriority::TRIVIAL);

  ourUseAramBehavior = true;
  ourAramConfigProcessFileCB.setName("MvrLogAram");
  Mvria::getConfig()->addProcessFileCB(&ourAramConfigProcessFileCB, 210);

  if (defaultLevel == MvrLog::Terse)
    sprintf(ourAramConfigLogLevel, "Terse");
  else if (defaultLevel == MvrLog::Normal)
    sprintf(ourAramConfigLogLevel, "Normal");
  if (defaultLevel == MvrLog::Verbose)
    sprintf(ourAramConfigLogLevel, "Verbose");

  ourAramDaemonized = daemonized;

  char buf[2048];
  snprintf(buf, sizeof(buf), "%slog1.txt", ourAramPrefix.c_str());
  MvrLog::init(MvrLog::File, defaultLevel, buf, true, !daemonized, true);

  if (ourAramDaemonized)
  {

    if (dup2(fileno(ourFP), fileno(stderr)) < 0)
      MvrLog::logErrorFromOSNoLock(
	      MvrLog::Normal, "MvrLog: Error redirecting stderr to log file.");
    
    // this is is taken out since if you set this flag, the file gets
    //closed twice, then really weird stuff happens after the exec
    //ArUtil::setFileCloseOnExec(fileno(stderr), true);

    fprintf(stderr, "Stderr...\n");

    /* stdout is taken out since we don't necessarily need it and it
     * wound up at the end of the file, which got really strange

    if (dup2(fileno(ourFP), fileno(stdout)) < 0)

      MvrLog::logErrorFromOSNoLock(
	      MvrLog::Normal, "MvrLog: Error redirecting stdout to log file.");

    // this is is taken out since if you set this flag, the file gets
    //closed twice, then really weird stuff happens after the exec
    MvrUtil::setFileCloseOnExec(fileno(stdout), true);

    fprintf(stdout, "Stdout...\n");
    */
  }

  ourAramConfigLogSize  = defaultSize;  // even megabytes
  ourAramLogSize = MvrMath::roundInt(ourAramConfigLogSize * 1000000);  // even megabytes
}

MVREXPORT bool MvrLog::aramProcessFile(void)
{
  ourMutex.lock();
  ourAramLogSize = MvrMath::roundInt(ourAramConfigLogSize * 1000000);  // even megabytes
  if (strcasecmp(ourAramConfigLogLevel, "Terse") == 0)
    ourLevel = Terse;
  else if (strcasecmp(ourAramConfigLogLevel, "Verbose") == 0)
    ourLevel = Verbose;
  else if (strcasecmp(ourAramConfigLogLevel, "Normal") == 0)
    ourLevel = Normal;
  else 
  {
    MvrLog::logNoLock(MvrLog::Normal, 
	       "MvrLog: Bad log level '%s' defaulting to Normal", 
	       ourAramConfigLogLevel);
    ourLevel = Normal;
  }
  ourMutex.unlock();
  return true;
}

MVREXPORT void MvrLog::filledAramLog(void)
{
  MvrLog::logNoLock(MvrLog::Normal, "MvrLog: Log filled, starting new file");

  fclose(ourFP);

  char buf[2048];
  int i;
  for (i = 5; i > 0; i--)
  {
    snprintf(buf, sizeof(buf), "mv %slog%d.txt %slog%d.txt", 
	     ourAramPrefix.c_str(), i, ourAramPrefix.c_str(), i+1);
    system(buf);
  }

  snprintf(buf, sizeof(buf), "%slog1.txt", ourAramPrefix.c_str());
  if ((ourFP = MvrUtil::fopen(ourFileName.c_str(), "w")) == NULL)
  {
    ourType = StdOut;
    MvrLog::logNoLock(MvrLog::Normal, 
	       "MvrLog: Couldn't reopen file '%s', failing back to stdout",
	       ourFileName.c_str());
  }
  ourCharsLogged = 0;

  if (ourAramDaemonized)
  {

    if (dup2(fileno(ourFP), fileno(stderr)) < 0)
      MvrLog::logErrorFromOSNoLock(
	      MvrLog::Normal, "MvrLog: Error redirecting stderr to log file.");

    // this is is taken out since if you set this flag, the file gets
    //closed twice, then really weird stuff happens after the exec
    //ArUtil::setFileCloseOnExec(fileno(stderr), true);

    fprintf(stderr, "Stderr...\n");
    
    /* stdout is taken out since we don't necessarily need it and it
     * wound up at the end of the file, which got really strange

    if (dup2(fileno(ourFP), fileno(stdout)) < 0)
      MvrLog::logErrorFromOSNoLock(
	      MvrLog::Normal, "MvrLog: Error redirecting stdout to log file.");

    // this is is taken out since if you set this flag, the file gets
    //closed twice, then really weird stuff happens after the exec
    MvrUtil::setFileCloseOnExec(fileno(stdout), true);

    fprintf(stdout, "Stdout...\n");
    */
  }
}

#endif // ARINTERFACE

MVREXPORT void MvrLog::setFunctor(MvrFunctor1<const char *> *functor)
{
  ourFunctor = functor;
}

MVREXPORT void MvrLog::clearFunctor()
{
  ourFunctor = NULL;
}

MVREXPORT void MvrLog::invokeFunctor(const char *message)
{
  MvrFunctor1<const char *> *functor;
  functor = ourFunctor;
  if (functor != NULL)
    functor->invoke(message);
}

MVREXPORT void MvrLog::checkFileSize(void)
{
  if (ourAramDaemonized)
  {
    long size;
    size = MvrUtil::sizeFile(ourFileName);
    if (size > 0 && size > ourCharsLogged)
    {
      ourCharsLogged = size;
    }
  }
}

MVREXPORT void MvrLog::internalForceLockup(void)
{
  MvrLog::log(MvrLog::Terse, "MvrLog: forcing internal lockup");
  ourMutex.lock();
}

MVREXPORT void MvrLog::log_v(LogLevel level, const char *prefix, const char *str, va_list ptr)
{
  char buf[1024];
  strncpy(buf, prefix, sizeof(buf));
  const size_t prefixSize = strlen(prefix);
  vsnprintf(buf+prefixSize, sizeof(buf)-prefixSize, str, ptr);
  buf[sizeof(buf) - 1] = '\0';
  logNoLock(level, buf);
}


MVREXPORT void MvrLog::info(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Normal, "", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

MVREXPORT void MvrLog::warning(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Terse, "Warning: ", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

MVREXPORT void MvrLog::error(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Terse, "Error: ", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

MVREXPORT void MvrLog::debug(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Terse, "[debug] ", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

MVREXPORT void MvrLog::setLogLevel(LogLevel level) {
	ourMutex.lock();
	ourLevel = level;
	ourMutex.unlock();
}