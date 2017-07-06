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

#define _GNU_SOURCE 1 // for isnormal() and other newer (non-ansi) C functions

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
#include <mmsystem.h> // for timeGetTime()
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
#endif


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

#ifdef WIN32
#include <io.h>
MVREXPORT const char *MvrUtil::COM1 = "COM1";
MVREXPORT const char *MvrUtil::COM2 = "COM2";
MVREXPORT const char *MvrUtil::COM3 = "COM3";
MVREXPORT const char *MvrUtil::COM4 = "COM4";
MVREXPORT const char *MvrUtil::COM5 = "COM5";
MVREXPORT const char *MvrUtil::COM6 = "COM6";
MVREXPORT const char *MvrUtil::COM7 = "COM7";
MVREXPORT const char *MvrUtil::COM8 = "COM8";
MVREXPORT const char *MvrUtil::COM9 = "COM9";
MVREXPORT const char *MvrUtil::COM10 = "\\\\.\\COM10";
MVREXPORT const char *MvrUtil::COM11 = "\\\\.\\COM11";
MVREXPORT const char *MvrUtil::COM12 = "\\\\.\\COM12";
MVREXPORT const char *MvrUtil::COM13 = "\\\\.\\COM13";
MVREXPORT const char *MvrUtil::COM14 = "\\\\.\\COM14";
MVREXPORT const char *MvrUtil::COM15 = "\\\\.\\COM15";
MVREXPORT const char *MvrUtil::COM16 = "\\\\.\\COM16";
// See http://support.microsoft.com/kb/115831 for explanation of port names for
// COM10 and up.
#else // ifndef WIN32
MVREXPORT const char *MvrUtil::COM1 = "/dev/ttyS0";
MVREXPORT const char *MvrUtil::COM2 = "/dev/ttyS1";
MVREXPORT const char *MvrUtil::COM3 = "/dev/ttyS2";
MVREXPORT const char *MvrUtil::COM4 = "/dev/ttyS3";
MVREXPORT const char *MvrUtil::COM5 = "/dev/ttyS4";
MVREXPORT const char *MvrUtil::COM6 = "/dev/ttyS5";
MVREXPORT const char *MvrUtil::COM7 = "/dev/ttyS6";
MVREXPORT const char *MvrUtil::COM8 = "/dev/ttyS7";
MVREXPORT const char *MvrUtil::COM9 = "/dev/ttyS8";
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
const double MvrMath::ourEpsilon = 0.00000001; 

#ifdef WIN32
// max returned by rand()
const long MvrMath::ourRandMax = RAND_MAX;
#else
// max returned by lrand48()
const long MvrMath::ourRandMax = 2147483648;// 2^31, per lrand48 man page
#endif

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
however, use the MvrTime timer utility instead, or the MvrUtil::getTime() method
to check time.
  @note in Linux, it actually calls the system usleep() function with 10 ms less
than the desired sleep time, since usleep() can sleep for about 10 ms. more than
requested (for small sleep times especially, it sleeps for the next highest
multiple of 10.)
   @param ms the number of milliseconds to sleep for
*/
MVREXPORT void MvrUtil::sleep(unsigned int ms)
{
#ifdef WIN32
  Sleep(ms);
#else // ifndef win32
  if (ms > 10)
    ms -= 10;
  usleep(ms * 1000);
#endif // linux

}

/**
   Get the time in milliseconds, counting from some arbitrary point.
   This time is only valid within this run of the program, it does not represent
"time of day". In other words, values returned by getTime() may be compared to
each other within the same run time of a program.
   If ARIA was compiled on Linux with POSIX monotonic clock functions
available, and the current running Linux kernel supports the monotonic clock,
then the clock_gettime() function is used with the CLOCK_MONOTONIC option 
(this is an accurate clock that is robust against e.g. changes to the
system time, etc.). Otherwise, gettimeofday() is used instead. Programs using ARIA
on Linux must therefore link to the librt library (with -lrt) as well as libMvr.  On Windows, the
timeGetTime() function is used from the winmm library -- this means programs
using ARIA on Windows must be linked to the winmm.lib library as well as ARIA.
   @return millisecond time
*/
MVREXPORT unsigned int MvrUtil::getTime(void)
{
// the good unix way
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
  struct timespec tp;
  if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    return tp.tv_nsec / 1000000 + (tp.tv_sec % 1000000)*1000;
// the old unix way as a fallback
#endif // if it isn't the good way
#if !defined(WIN32)
  struct timeval tv;
  if (gettimeofday(&tv,NULL) == 0)
    return tv.tv_usec/1000 + (tv.tv_sec % 1000000)*1000;
  else
    return 0;
#elif defined(WIN32)
  return timeGetTime();
#endif
}

/*
   Takes a string and splits it into a list of words. It appends the words
   to the outList. If there is nothing found, it will not touch the outList.
   @param inString the input string to split
   @param outList the list in which to store the words that are found
*/
/*
MVREXPORT void MvrUtil::splitString(std::string inString,
				  std::list<std::string> &outList)
{
  const char *start, *end;

  // Strip off leading white space
  for (end=inString.c_str(); *end && isspace(*end); ++end)
    ;
  while (*end)
  {
    // Mark start of the word then find end
    for (start=end; *end && !isspace(*end); ++end)
      ;
    // Store the word
    if (*start && ((*end && isspace(*end)) || !*end))
      outList.push_back(std::string(start, (int)(end-start)));
    for (; *end && isspace(*end); ++end)
      ;
  }
}
*/



#ifdef WIN32

/**
   @return size in bytes. -1 on error.
   @param fileName name of the file to size
*/
MVREXPORT long MvrUtil::sizeFile(std::string fileName)
{
  struct _stat buf;

  if (_stat(fileName.c_str(), &buf) < 0)
    return(-1);

  if (!(buf.st_mode | _S_IFREG))
    return(-1);

  return(buf.st_size);
}

/**
   @return size in bytes. -1 on error.
   @param fileName name of the file to size
*/
MVREXPORT long MvrUtil::sizeFile(const char * fileName)
{
  struct _stat buf;

  if (_stat(fileName, &buf) < 0)
    return(-1);

  if (!(buf.st_mode | _S_IFREG))
    return(-1);

  return(buf.st_size);
}

#else // !WIN32

MVREXPORT long MvrUtil::sizeFile(std::string fileName)
{
  struct stat buf;

  if (stat(fileName.c_str(), &buf) < 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrUtil::sizeFile: stat failed");
    return(-1);
  }

  if (!S_ISREG(buf.st_mode))
    return(-1);

  return(buf.st_size);
}


/**
   @return size in bytes. -1 on error.
   @param fileName name of the file to size
*/
MVREXPORT long MvrUtil::sizeFile(const char * fileName)
{
  struct stat buf;

  if (stat(fileName, &buf) < 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrUtil::sizeFile: stat failed");
    return(-1);
  }

  if (!S_ISREG(buf.st_mode))
    return(-1);

  return(buf.st_size);
}

#endif // else !WIN32

/**
   @return true if file is found
   @param fileName name of the file to size
*/
MVREXPORT bool MvrUtil::findFile(const char *fileName)
{
  FILE *fp;

  if ((fp=MvrUtil::fopen(fileName, "r")))
  {
    fclose(fp);
    return(true);
  }
  else
    return(false);
}




/*
   Works for \ and /. Returns true if something was actualy done. Sets
   fileOut to be what ever the answer is.
   @return true if the path contains a file
   @param fileIn input path/fileName
   @param fileOut output fileName
*/
/*MVREXPORT bool MvrUtil::stripDir(std::string fileIn, std::string &fileOut)
{
  const char *ptr;

  for (ptr=fileIn.c_str(); *ptr; ++ptr)
    ;
  for (--ptr; (ptr > fileIn.c_str()) && (*ptr != '/') && (*ptr != '\\'); --ptr)
    ;
  if ((*ptr == '/') || (*ptr == '\\'))
  {
    fileOut=ptr+1;
    return(true);
  }
  else
  {
    fileOut=fileIn;
    return(false);
  }
}
*/
/*
   Works for \ and /. Returns true if something was actualy done. Sets
   fileOut to be what ever the answer is.
   @return true if the file contains a path
   @param fileIn input path/fileName
   @param fileOut output path
*/
/*
MVREXPORT bool MvrUtil::stripFile(std::string fileIn, std::string &fileOut)
{
  const char *start, *end;

  for (start=end=fileIn.c_str(); *end; ++end)
  {
    if ((*end == '/') || (*end == '\\'))
    {
      start=end;
      for (; *end && ((*end == '/') || (*end == '\\')); ++end)
        ;
    }
  }

  if (start < end)
  {
    fileOut.assign(fileIn, 0, start-fileIn.c_str());
    return(true);
  }

  fileOut=fileIn;
  return(false);
}
*/
MVREXPORT bool MvrUtil::stripQuotes(char *dest, const char *src, size_t destLen)
{
  size_t srcLen = strlen(src);
  if (destLen < srcLen + 1)
  {
    MvrLog::log(MvrLog::Normal, "MvrUtil::stripQuotes: destLen isn't long enough to fit copy its %d should be %d", destLen, srcLen + 1);
    return false;
  }
  // if there are no quotes to strip just copy and return
  if (srcLen < 2 || 
      (src[0] != '"' || src[srcLen - 1] != '"'))
  {
    strcpy(dest, src);
    return true;
  }
  // we have quotes so chop of the first and last char
  strncpy(dest, &src[1], srcLen - 1);
  dest[srcLen - 2] = '\0';
  return true;
}


/**
 * This method behaves similarly to the char[] version, except that it modifies
 * the given std::string.  If the first and last characters of the string are 
 * quotation marks ("), then they are removed from the string.  Multiple nested
 * quotation marks are not handled.
 * @param strToStrip a pointer to the std::string to be read/modified; must be 
 * non-NULL
 * @return bool true if the string was successfully processed; false otherwise 
**/  
MVREXPORT bool MvrUtil::stripQuotes(std::string *strToStrip)
{
  if (strToStrip == NULL) {
    MvrLog::log(MvrLog::Normal,
               "MvrUtil::stripQuotes() NULL string");
    return false;
  }
  
  // If there are no matching quotes to strip, just return
  if ((strToStrip->size() < 2) || 
      (strToStrip->at(0) != '"') || 
      (strToStrip->at(strToStrip->size() - 1) != '"'))
  {
    return true;
  }
  
  // Matching quotes found so chop of the first and last char
  strToStrip->erase(strToStrip->begin());
  strToStrip->resize(strToStrip->size() - 1);

  return true;
               
} // end method stripQuotes

/**
 * This method strips out bad characters
**/  
MVREXPORT bool MvrUtil::fixBadCharacters(
	std::string *strToStrip, bool removeSpaces, bool fixOtherWhiteSpace)
{
  if (strToStrip == NULL) {
    MvrLog::log(MvrLog::Normal,
               "MvrUtil::fixBadCharacters() NULL string");
    return false;
  }
  
  // The .length() isn't guaranteed to be O(1) (it could be O(n)) but
  // it's not usually O(1).... and hopefully this isn't called that
  // often (the original place was the identifer, which is once at
  // startup)... most of the other solutions I've seen are way more
  // complicated
  for (size_t i = 0; i < (*strToStrip).length(); i++)
  {
    if (!removeSpaces && (*strToStrip)[i] == ' ')
      continue;
    else if (!removeSpaces && fixOtherWhiteSpace && isspace((*strToStrip)[i]))
      (*strToStrip)[i] = ' ';
    else if ((*strToStrip)[i] == '(' || (*strToStrip)[i] == '{')
      (*strToStrip)[i] = '[';
    else if ((*strToStrip)[i] == ')' || (*strToStrip)[i] == '}')
      (*strToStrip)[i] = ']';
    else if (isalpha((*strToStrip)[i]) || isdigit((*strToStrip)[i]) || 
	     (*strToStrip)[i] == '.' || (*strToStrip)[i] == '_' || 
	     (*strToStrip)[i] == '-' || (*strToStrip)[i] == '+' || 
	     (*strToStrip)[i] == '[' || (*strToStrip)[i] == ']')
      continue;
    else
      (*strToStrip)[i] = '-';
  }

  return true;
               
} // end method stripQuotes

/** Append a directory separator character to the given path string, depending on the
 * platform.  On Windows, a backslash ('\\') is added. On other platforms, a 
 * forward slash ('/') is appended. If there is no more allocated space in the
 * path string (as given by the @arg pathLength parameter), no character will be appended.
   @param path the path string to append a slash to
   @param pathLength maximum length allocated for path string
*/
MVREXPORT void MvrUtil::appendSlash(char *path, size_t pathLength)
{
  // first check boundary
  size_t len;
  len = strlen(path);
  if (len > pathLength - 2)
    return;

  if (len == 0 || (path[len - 1] != '\\' && path[len - 1] != '/'))
  {
#ifdef WIN32
    path[len] = '\\';
#else
    path[len] = '/';
#endif
    path[len + 1] = '\0';
  }
}

/** Append the appropriate directory separator for this platform (a forward
 * slash "/" on Linux, or a backslash "\" on Windows) to @arg path.
 */
MVREXPORT void MvrUtil::appendSlash(std::string &path)
{
  // first check boundary
  size_t len = path.length();
  if ((len == 0) || 
      (path[len - 1] != SEPARATOR_CHAR && path[len - 1] != OTHER_SEPARATOR_CHAR)) {
    path += SEPARATOR_STRING;
  }

} // end method appendSlash

/**
   Replace in @a path all incorrect directory separators for this platform with
the correct directory separator character (forward slash '/' on Linux, backslash
'\' on Windows).
   @param path the path in which to fix the orientation of the slashes
   @param pathLength the maximum length of path
*/
MVREXPORT void MvrUtil::fixSlashes(char *path, size_t pathLength)
{
#ifdef WIN32
  fixSlashesBackward(path, pathLength);
#else
  fixSlashesForward(path, pathLength);
#endif
}

/** Replace any forward slash charactars ('/') in @a path with backslashes ('\').
   @param path the path in which to fix the orientation of the slashes
   @param pathLength size of @a path
*/
MVREXPORT void MvrUtil::fixSlashesBackward(char *path, size_t pathLength)
{
  for (size_t i=0; path[i] != '\0' && i < pathLength; i++)
  {
    if (path[i] == '/')
      path[i]='\\';
  }
}

/** Replace any forward slash charactars ('/') in @a path with backslashes ('\').
   @param path the path in which to fix the orientation of the slashes
   @param pathLength size of @a path
*/
MVREXPORT void MvrUtil::fixSlashesForward(char *path, size_t pathLength)
{

  for (size_t i=0; path[i] != '\0' && i < pathLength; i++)
  {
    if (path[i] == '\\')
      path[i]='/';
  }
}

/**
   Replace in @a path all incorrect directory separators for this platform with
the correct directory separator character (forward slash '/' on Linux, backslash
'\' on Windows).
   @param path the path in which to fix the orientation of the slashes
*/
MVREXPORT void MvrUtil::fixSlashes(std::string &path) 
{
  for (size_t i = 0; i < path.length(); i++)
  {
    if (path[i] == OTHER_SEPARATOR_CHAR)
      path[i]= SEPARATOR_CHAR;
  }
}
  
/** What is the appropriate directory path separator character for this
 * platform? */
MVREXPORT char MvrUtil::getSlash()
{
  return SEPARATOR_CHAR;
}

/**
   This function will take the @a baseDir and add @a insideDir after
   it, separated by appropriate directory path separators for this platform,
   with a final directory separator retained or added after @a inside dir.
   For example on Linux, this results in 'baseDir/insideDir/', whereas
   backslashes are used on Windows instead. The resulting string is 
   placed in the @a dest buffer (the path string will be truncated if
   there is not enough space in @a dest according to @a destLength)

   @param dest the place to put the result
   @param destLength the length available in @a dest
   @param baseDir the directory to start with
   @param insideDir the directory to place after the baseDir 
**/
MVREXPORT void MvrUtil::addDirectories(char *dest, size_t destLength, 
				     const char *baseDir,
				     const char *insideDir)
{
  // start it off
  strncpy(dest, baseDir, destLength - 1);
  // make sure we have a null term
  dest[destLength - 1] = '\0';
  // toss on that slash
  appendSlash(dest, destLength);
  // put on the inside dir
  strncat(dest, insideDir, destLength - strlen(dest) - 1);
  // now toss on that slash
  appendSlash(dest, destLength);
  // and now fix up all the slashes
  fixSlashes(dest, destLength);
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if @a str  is  found, respectively, to be less than, to
    match, or be greater than @a str2. 
    (This is a version of the standard C strcmp() function
    for use with std::string strings instead.)
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
MVREXPORT int MvrUtil::strcmp(const std::string &str, const std::string &str2)
{
  return ::strcmp(str.c_str(), str2.c_str());
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if  str  is  found, respectively, to be less than, to
    match, or be greater than str2.
    (This is a version of the standard C strcmp() function
    for use with std::string strings instead.)
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
MVREXPORT int MvrUtil::strcmp(const std::string &str, const char *str2)
{
  if (str2 != NULL) {
    return ::strcmp(str.c_str(), str2);
  }
  else {
    return 1;
  }
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if  str  is  found, respectively, to be less than, to
    match, or be greater than str2.
    (This is a version of the standard C strcmp() function
    for use with std::string strings instead.)
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
MVREXPORT int MvrUtil::strcmp(const char *str, const std::string &str2)
{
  if (str != NULL) {
    return ::strcmp(str, str2.c_str());
  }
  else {
    return -1;
  }
}

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if  str  is  found, respectively, to be less than, to
    match, or be greater than str2.
    @param str the string to compare
    @param str2 the second string to compare
    @return an integer less than, equal to, or greater than zero if str is 
    found, respectively, to be less than, to match, or be greater than str2.
*/
MVREXPORT int MvrUtil::strcmp(const char *str, const char *str2)
{
  if ((str != NULL) && (str2 != NULL)) {
    return ::strcmp(str, str2);
  }
  else if ((str == NULL) && (str2 == NULL)) {
    return 0;
  }
  else if (str == NULL) {
    return -1;
  }
  else { // str2 == NULL
    return 1;
  }
}


/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
MVREXPORT int MvrUtil::strcasecmp(const std::string &str, 
                                const std::string &str2)
{
  return ::strcasecmp(str.c_str(), str2.c_str());
}

/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
MVREXPORT int MvrUtil::strcasecmp(const std::string &str, const char *str2)
{
  if (str2 != NULL) {
    return ::strcasecmp(str.c_str(), str2);
  }
  else {
    return 1;
  }
}

/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
MVREXPORT int MvrUtil::strcasecmp(const char *str, const std::string &str2)
{
  if (str != NULL) {
    return ::strcasecmp(str, str2.c_str());
  }
  else {
    return -1;
  }
}

/** 
    This compares two strings ignoring case, it returns an integer
    less than, equal to, or greater than zero if str is found,
    respectively, to be less than, to match, or be greater than str2.
    @param str the string to compare @param str2 the second string to
    compare @return an integer less than, equal to, or greater than
    zero if str is found, respectively, to be less than, to match, or
    be greater than str2.  */
MVREXPORT int MvrUtil::strcasecmp(const char *str, const char *str2)
{
  if ((str != NULL) && (str2 != NULL)) {
    return ::strcasecmp(str, str2);
  }
  else if ((str == NULL) && (str2 == NULL)) {
    return 0;
  }
  else if (str == NULL) {
    return -1;
  }
  else { // str2 == NULL
    return 1;
  }
}


MVREXPORT bool MvrUtil::strSuffixCmp(const char *str, const char *suffix)
{
  if (str != NULL && str[0] != '\0' && 
      suffix != NULL && suffix[0] != '\0' &&
      strlen(str) > strlen(suffix) + 1 &&
      strncmp(&str[strlen(str) - strlen(suffix)], 
		  suffix, strlen(suffix)) == 0)
    return true;
  else
    return false;
 
}

MVREXPORT bool MvrUtil::strSuffixCaseCmp(const char *str, const char *suffix)
{
  if (str != NULL && str[0] != '\0' && 
      suffix != NULL && suffix[0] != '\0' &&
      strlen(str) > strlen(suffix) + 1 &&
      strncasecmp(&str[strlen(str) - strlen(suffix)], 
		  suffix, strlen(suffix)) == 0)
    return true;
  else
    return false;
}


MVREXPORT int MvrUtil::strcasequotecmp(const std::string &inStr1, 
                                     const std::string &inStr2)
{
std::string str1 = inStr1;
std::string str2 = inStr2;

	int x = 0;
	while (x < str1.length()) {
		if (isalpha(str1[x]) && isupper(str1[x]))
			str1[x] = tolower(str1[x]);
	x++;
	}

	x = 0;
	while (x < str2.length()) {
		if (isalpha(str2[x]) && isupper(str2[x]))
			str2[x] = tolower(str2[x]);
	x++;
	}
  
  int len1 = str1.length();
  size_t pos1 = 0;
  if ((len1 >= 2) && (str1[0] == '\"') && (str1[len1 - 1] == '\"')) {
    pos1 = 1;
  }
  int len2 = str2.length();
  size_t pos2 = 0;
  if ((len2 >= 2) && (str2[0] == '\"') && (str2[len2 - 1] == '\"')) {
    pos2 = 1;
  }

  /* Unfortunately gcc2 does't support the 5 argument version of std::string::compare()... 
   * (Furthermore, note that it's 3-argument compare has the arguments in the wrong order.)
   */
#if defined(__GNUC__) && (__GNUC__ <= 2) && (__GNUC_MINOR__ <= 96)
#warning Using GCC 2.96 or less so must use nonstandard std::string::compare method.
  int cmp = str1.compare(str2.substr(pos2, len2 - 2 * pos2), pos1, len1 - 2 * pos1);
#else
  int cmp = str1.compare(pos1, 
	 	  	 len1 - 2 * pos1,
                         str2,
                         pos2, 
			 len2 - 2 * pos2);
#endif

  return cmp;

} // end method strcasequotecmp

/**
   This copies src into dest but puts a \ before any spaces in src,
   escaping them... its mostly for use with MvrArgumentBuilder... 
   make sure you have at least maxLen spaces in the arrays that you're passing 
   as dest... this allocates no memory
**/
MVREXPORT void MvrUtil::escapeSpaces(char *dest, const char *src, size_t maxLen)
{
  size_t i, adj, len;

  len = strlen(src);
  // walk it, when we find one toss in the slash and incr adj so the
  // next characters go in the right space
  for (i = 0, adj = 0; i < len && i + adj < maxLen; i++)
  {
    if (src[i] == ' ')
    {
      dest[i+adj] = '\\';
      adj++;
    }
    dest[i+adj] = src[i];
  }
  // make sure its null terminated
  dest[i+adj] = '\0';
}

/**
   This copies src into dest but makes it lower case make sure you
   have at least maxLen arrays that you're passing as dest... this
   allocates no memory
**/
MVREXPORT void MvrUtil::lower(char *dest, const char *src, size_t maxLen)
{
  size_t i;
  size_t len;
  
  len = strlen(src);
  for (i = 0; i < len && i < maxLen; i++)
    dest[i] = tolower(src[i]);
  dest[i] = '\0';

}


MVREXPORT bool MvrUtil::isOnlyAlphaNumeric(const char *str)
{
  unsigned int ui;
  unsigned int len;
  if (str == NULL)
    return true;
  for (ui = 0, len = strlen(str); ui < len; ui++)
  {
    if (!isalpha(str[ui]) && !isdigit(str[ui]) && str[ui] != '\0' && 
	str[ui] != '+' && str[ui] != '-')
      return false;
  }
  return true;
}

MVREXPORT bool MvrUtil::isOnlyNumeric(const char *str)
{
  if (str == NULL)
    return true;
  for (unsigned i = 0, len = strlen(str); i < len; i++)
  {
    if (!isdigit(str[i]) && str[i] != '\0' && str[i] != '+' && str[i] != '-')
      return false;
  }
  return true;
}

MVREXPORT bool MvrUtil::isStrEmpty(const char *str)
{
	if (str == NULL) {
		return true;
	}
	if (str[0] == '\0') {
		return true;
	}
	return false;

} // end method isStrEmpty

  
MVREXPORT bool MvrUtil::isStrInList(const char *str,
                                  const std::list<std::string> &list,
                                  bool isIgnoreCase)
{
  if (str == NULL) {
    return false;
  }
  for (std::list<std::string>::const_iterator aIter = list.begin();
       aIter != list.end();
       aIter++) {
    if (!isIgnoreCase) {
      if (strcmp((*aIter).c_str(), str) == 0) {
        return true;
      }
    }
    else { // ignore case
      if (strcasecmp((*aIter).c_str(), str) == 0) {
        return true;
      }
    } // end else ignore case
  } // end for each string

  return false;

} // end method isStrInList


MVREXPORT const char *MvrUtil::convertBool(int val)
{
  if (val)
    return TRUESTRING;
  else
    return FALSESTRING;
}

MVREXPORT double MvrUtil::atof(const char *nptr)
{
  if (strcasecmp(nptr, "inf") == 0)
    return HUGE_VAL;
  else if (strcasecmp(nptr, "-inf") == 0)
    return -HUGE_VAL;
  else
	return ::atof(nptr);
}


MVREXPORT void MvrUtil::functorPrintf(MvrFunctor1<const char *> *functor,
				    const char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);
  //vsprintf(buf, str, ptr);
  vsnprintf(buf, sizeof(buf) - 1, str, ptr);
  buf[sizeof(buf) - 1] = '\0';
  functor->invoke(buf);
  va_end(ptr);
}

// preserving this old version that takes char* as format str instead of const char* 
// to maximize compatibility
MVREXPORT void MvrUtil::functorPrintf(MvrFunctor1<const char *> *functor,
				    char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);
  //vsprintf(buf, str, ptr);
  vsnprintf(buf, sizeof(buf) - 1, (const char*)str, ptr);
  buf[sizeof(buf) - 1] = '\0';
  functor->invoke(buf);
  va_end(ptr);
}


MVREXPORT void MvrUtil::writeToFile(const char *str, FILE *file)
{
  fputs(str, file);
}


/** 
   This function reads a string from a file.
   The file can contain spaces or tabs, but a '\\r'
   or '\\n' will be treated as the end of the string, and the string
   cannot have more characters than the value given by strLen.  This is mostly for internal use
   with Linux to determine the Mvr directory from a file in /etc, but
   will work with Linux or Windows. 

   @param fileName name of the file in which to look
   @param str the string to copy the file contents into
   @param strLen the maximum allocated length of str
**/
MVREXPORT bool MvrUtil::getStringFromFile(const char *fileName, 
					char *str, size_t strLen)
{
  FILE *strFile;
  unsigned int i;
  
  str[0] = '\0';
  
  if ((strFile = MvrUtil::fopen(fileName, "r")) != NULL)
  {
    fgets(str, strLen, strFile);
    for (i = 0; i < strLen; i++)
    {
      if (str[i] == '\r' || str[i] == '\n' || str[i] == '\0')
      {
	str[i] = '\0';
	fclose(strFile);
	break;
      }
    }
  }
  else
  {
    str[0] = '\0';
    return false;
  }
  return true;
}

/**
 * Look up the given value under the given key, within the given registry root
 * key.

   @param root the root key to use, one of the REGKEY enum values

   @param key the name of the key to find

   @param value the value name in which to find the string

   @param str where to put the string found, or if it could not be
   found, an empty (length() == 0) string

   @param len the length of the allocated memory in str

   @return true if the string was found, false if it was not found or if there was a problem such as the string not being long enough 
 **/

MVREXPORT bool MvrUtil::getStringFromRegistry(REGKEY root,
						   const char *key,
						   const char *value,
						   char *str,
						   int len)
{
#ifndef WIN32
  return false;
#else // WIN32

  HKEY hkey;
  int err;
  unsigned long numKeys;
  unsigned long longestKey;
  unsigned long numValues;
  unsigned long longestValue;
  unsigned long longestDataLength;
  char *valueName;
  unsigned long valueLength;
  unsigned long type;
  char *data;
  unsigned long dataLength;
  HKEY rootKey;


  switch (root)
  {
  case REGKEY_CLASSES_ROOT:
    rootKey = HKEY_CLASSES_ROOT;
    break;
  case REGKEY_CURRENT_CONFIG:
    rootKey = HKEY_CURRENT_CONFIG;
    break;
  case REGKEY_CURRENT_USER:
    rootKey = HKEY_CURRENT_USER;
    break;
  case REGKEY_LOCAL_MACHINE:
    rootKey = HKEY_LOCAL_MACHINE;
    break;
  case REGKEY_USERS:
    rootKey=HKEY_USERS;
    break;
  default:
    MvrLog::log(MvrLog::Terse, 
	       "MvrUtil::getStringFromRegistry: Bad root key given.");
    return false;
  }


  if ((err = RegOpenKeyEx(rootKey, key, 0, KEY_READ, &hkey)) == ERROR_SUCCESS)
  {
    //printf("Got a key\n");
    if (RegQueryInfoKey(hkey, NULL, NULL, NULL, &numKeys, &longestKey, NULL, 
			&numValues, &longestValue, &longestDataLength, NULL, NULL) == ERROR_SUCCESS)
    {
	/*
      printf("Have %d keys longest is %d, have %d values longest name is %d, longest data is %d\n",
	     numKeys, longestKey, numValues, longestValue, longestDataLength);
	*/	 
      data = new char[longestDataLength+2];
      valueName = new char[longestValue+2];
      for (unsigned long i = 0; i < numValues; ++i)
      {
	dataLength = longestDataLength+1;
	valueLength = longestValue+1;
	if ((err = RegEnumValue(hkey, i, valueName, &valueLength, NULL, 
				&type, (unsigned char *)data, &dataLength)) == ERROR_SUCCESS)
	{
		//printf("Enumed value %d, name is %s, value is %s\n", i, valueName, data);
	  if (strcmp(value, valueName) == 0)
	  {
	    if (len < dataLength)
	    {
	      MvrLog::log(MvrLog::Terse,"MvrUtil::getStringFromRegistry: str passed in not long enough for data.");
	      delete data;
	      delete valueName;
	      return false;
	    }
	    strncpy(str, data, len);
	    delete data;
	    delete valueName;
	    return true;
	  }
	}
	/*
	else
		printf("Couldn't enum value %d cause %d\n",i,  err);
		*/
	    }
      delete data;
      delete valueName;
    }
	/*
    else
      printf("QueryInfoKey failed\n");
	  */
  }
  /*
  else
    printf("No key %d\n", err);
  */
  return false;
#endif
}
  
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
bool MvrTime::ourMonotonicClock = true;
#endif 

MVREXPORT void MvrTime::setToNow(void)
{
// if we have the best way of finding time use that
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
  if (ourMonotonicClock)
  {
    struct timespec timeNow;
    if (clock_gettime(CLOCK_MONOTONIC, &timeNow) == 0)
    {
      // start a million seconds into the future so we have some room
      // to go backwards
      mySec = timeNow.tv_sec + 1000000;
      myMSec = timeNow.tv_nsec / 1000000;
      return;
    }
    else
    {
      ourMonotonicClock = false;
      MvrLog::logNoLock(MvrLog::Terse, "MvrTime::setToNow: invalid return from clock_gettime.");
    }
  }
#endif
// if our good way didn't work use the old ways
#ifndef WIN32
  struct timeval timeNow;
  
  if (gettimeofday(&timeNow, NULL) == 0)
  {
    // start a million seconds into the future so we have some room
    // to go backwards
    mySec = timeNow.tv_sec + 1000000;
    myMSec = timeNow.tv_usec / 1000;
  }
  else
    MvrLog::logNoLock(MvrLog::Terse, "MvrTime::setToNow: invalid return from gettimeofday.");
// thats probably not available in windows, so this is the one we've been using
#else
      /* this should be the better way, but it doesn't really work...
	 this would be seconds from 1970, but it is based on the
	 hardware timer or something and so winds up not being updated
	 all the time and winds up being some number of ms < 20 ms off
      struct _timeb startTime;
      _ftime(&startTime);
      mySec = startTime.time;
      myMSec = startTime.millitm;*/
      // so we're going with just their normal function, msec since boot
  long timeNow;
  timeNow = timeGetTime();
  // start a million seconds into the future so we have some room
  // to go backwards 
  mySec = timeNow / 1000 + 1000000;
  myMSec = timeNow % 1000;
// but if the good way isn't available use the old way...
#endif
      
}

MVREXPORT MvrRunningAverage::MvrRunningAverage(size_t numToAverage)
{
  myNumToAverage = numToAverage;
  myTotal = 0;
  myNum = 0;
  myUseRootMeanSquare = false;
}

MVREXPORT MvrRunningAverage::~MvrRunningAverage()
{

}

MVREXPORT double MvrRunningAverage::getAverage(void) const
{
  if (myNum == 0)
    return 0.0;

  if (myUseRootMeanSquare)
    return sqrt(myTotal / myNum);
  else
    return myTotal / myNum;
}

MVREXPORT void MvrRunningAverage::add(double val)
{
  if (myUseRootMeanSquare)
    myTotal += (val * val);
  else
    myTotal += val;
  myNum++;
  myVals.push_front(val);
  if (myVals.size() > myNumToAverage || myNum > myNumToAverage)
  {
    if (myUseRootMeanSquare)
      myTotal -= (myVals.back() * myVals.back());
    else
      myTotal -= myVals.back();
    myNum--;
    myVals.pop_back();
  }
}

MVREXPORT void MvrRunningAverage::clear(void)
{
  while (myVals.size() > 0)
    myVals.pop_back();
  myNum = 0;
  myTotal = 0;
}

MVREXPORT size_t MvrRunningAverage::getNumToAverage(void) const
{
  return myNumToAverage;
}

MVREXPORT void MvrRunningAverage::setNumToAverage(size_t numToAverage)
{
  myNumToAverage = numToAverage;
  while (myVals.size() > myNumToAverage)
  {
    if (myUseRootMeanSquare)
      myTotal -= (myVals.back() * myVals.back());
    else
      myTotal -= myVals.back();
    myNum--;
    myVals.pop_back();
  }
}

MVREXPORT size_t MvrRunningAverage::getCurrentNumAveraged(void)
{
  return myNum;
}

MVREXPORT void MvrRunningAverage::setUseRootMeanSquare(bool useRootMeanSquare)
{
  if (myUseRootMeanSquare != useRootMeanSquare)
  {
    myTotal = 0;
    std::list<double>::iterator it;
    for (it = myVals.begin(); it != myVals.end(); it++)
    {
      if (useRootMeanSquare)
	myTotal += ((*it) * (*it));
      else
	myTotal += (*it);
    }
  }

  myUseRootMeanSquare = useRootMeanSquare;
}

MVREXPORT bool MvrRunningAverage::getUseRootMeanSquare(void)
{
  return myUseRootMeanSquare;
}

MVREXPORT MvrRootMeanSquareCalculator::MvrRootMeanSquareCalculator()
{
  clear();
  myName = "MvrRootMeanSquareCalculator";
}

MVREXPORT MvrRootMeanSquareCalculator::~MvrRootMeanSquareCalculator()
{

}

MVREXPORT double MvrRootMeanSquareCalculator::getRootMeanSquare (void) const
{
  if (myNum == 0)
    return 0;
  else
    return sqrt((double) myTotal / (double)myNum);
}

MVREXPORT void MvrRootMeanSquareCalculator::add(int val)
{
  myTotal += val * val;
  myNum++;
  if (myTotal < 0)
  {
    MvrLog::log(MvrLog::Normal, "%s: total wrapped, resetting", myName.c_str());
    clear();
    // this isn't a clean fix, but won't let it infinitely loop on a bad value
    //add(val);
  }
}

MVREXPORT void MvrRootMeanSquareCalculator::clear(void)
{
  myTotal = 0;
  myNum = 0;
}

MVREXPORT size_t MvrRootMeanSquareCalculator::getCurrentNumAveraged(void)
{
  return myNum;
}

MVREXPORT void MvrRootMeanSquareCalculator::setName(const char *name)
{
  if (name != NULL)
    myName = name;
  else
    myName = "MvrRootMeanSquareCalculator";
}

MVREXPORT const char *MvrRootMeanSquareCalculator::getName(void)
{
  return myName.c_str();
}

#ifndef WIN32

MVREXPORT MvrDaemonizer::MvrDaemonizer(int *argc, char **argv, 
				    bool closeStdErrAndStdOut) :
  myParser(argc, argv),
  myLogOptionsCB(this, &MvrDaemonizer::logOptions)
{
  myIsDaemonized = false;
  myCloseStdErrAndStdOut = closeStdErrAndStdOut;
  Mvria::addLogOptionsCB(&myLogOptionsCB);
}

MVREXPORT MvrDaemonizer::~MvrDaemonizer()
{

}

MVREXPORT bool MvrDaemonizer::daemonize(void)
{
  if (myParser.checkArgument("-daemonize") ||
      myParser.checkArgument("-d"))
  {
    return forceDaemonize();
  }
  else
    return true;

}

/**
   This returns true if daemonizing worked, returns false if it
   didn't... the parent process exits here if forking worked.
 **/
MVREXPORT bool MvrDaemonizer::forceDaemonize(void)
{
    switch (fork())
    {
    case 0: // child process just return
      myIsDaemonized = true;
      if (myCloseStdErrAndStdOut)
      {
	fclose(stdout);
	fclose(stderr);
      }
      return true;
    case -1: // error.... fail
      printf("Can't fork");
      MvrLog::log(MvrLog::Terse, "MvrDaemonizer: Can't fork");
      return false;
    default: // parent process
      printf("Daemon started\n");
      exit(0);
    }
}

MVREXPORT void MvrDaemonizer::logOptions(void) const
{
  MvrLog::log(MvrLog::Terse, "Options for Daemonizing:");
  MvrLog::log(MvrLog::Terse, "-daemonize");
  MvrLog::log(MvrLog::Terse, "-d");
  MvrLog::log(MvrLog::Terse, "");
}

#endif // WIN32


std::map<MvrPriority::Priority, std::string> MvrPriority::ourPriorityNames;
std::map<std::string, MvrPriority::Priority, MvrStrCaseCmpOp> MvrPriority::ourNameToPriorityMap;

std::string MvrPriority::ourUnknownPriorityName;
bool MvrPriority::ourStringsInited = false;

MVREXPORT const char *MvrPriority::getPriorityName(Priority priority) 
{

  if (!ourStringsInited)
  {
    ourPriorityNames[IMPORTANT]     = "Basic";
    ourPriorityNames[NORMAL]        = "Intermediate";
    ourPriorityNames[TRIVIAL]       = "Advanced";
    ourPriorityNames[DETAILED]      = "Advanced";

    ourPriorityNames[EXPERT]        = "Expert";
    ourPriorityNames[FACTORY]       = "Factory";
    ourPriorityNames[CALIBRATION]   = "Calibration";
    
    for (std::map<MvrPriority::Priority, std::string>::iterator iter = ourPriorityNames.begin();
         iter != ourPriorityNames.end();
         iter++) {
      ourNameToPriorityMap[iter->second] = iter->first;
    }

    ourUnknownPriorityName  = "Unknown";
    ourStringsInited        = true;
  }

  std::map<MvrPriority::Priority, std::string>::iterator iter = 
                                                  ourPriorityNames.find(priority);
  if (iter != ourPriorityNames.end()) {
    return iter->second.c_str();
  }
  else {
    return ourUnknownPriorityName.c_str();
  }
}

MVREXPORT MvrPriority::Priority MvrPriority::getPriorityFromName(const char *text, 
                                                              bool *ok)
{
  // This is merely called to initialize the map
  if (!ourStringsInited) {
     getPriorityName(IMPORTANT);
  }
   
  // Assume failure (until successful) 
  if (ok != NULL) {
    *ok = false;
  }

  if (MvrUtil::isStrEmpty(text)) {
    MvrLog::log(MvrLog::Normal,
               "MvrPriority::getPriorityFromName() error finding priority for empty text");
    return LAST_PRIORITY;
  }

  std::map<std::string, MvrPriority::Priority, MvrStrCaseCmpOp>::iterator iter = 
                                                  ourNameToPriorityMap.find(text);
  if (iter != ourNameToPriorityMap.end()) {
    if (ok != NULL) {
      *ok = true;
    }
    return iter->second;
  }
  
  MvrLog::log(MvrLog::Normal,
             "MvrPriority::getPriorityFromName() error finding priority for %s",
             text);

  return LAST_PRIORITY;
  
} // end method getPriorityFromName


MVREXPORT void MvrUtil::putCurrentYearInString(char* s, size_t len)
{
  struct tm t;
  MvrUtil::localtime(&t);
  snprintf(s, len, "%4d", 1900 + t.tm_year);
  s[len-1] = '\0';
}

MVREXPORT void MvrUtil::putCurrentMonthInString(char* s, size_t len)
{

  struct tm t;
  MvrUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_mon + 1);
  s[len-1] = '\0';
}
MVREXPORT void MvrUtil::putCurrentDayInString(char* s, size_t len)
{
  struct tm t;
  MvrUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_mday);
  s[len-1] = '\0';
}
MVREXPORT void MvrUtil::putCurrentHourInString(char* s, size_t len)
{
  struct tm t;
  MvrUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_hour);
  s[len-1] = '\0';
}
MVREXPORT void MvrUtil::putCurrentMinuteInString(char* s, size_t len)
{
  struct tm t; 
  MvrUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_min);
  s[len-1] = '\0';
}
MVREXPORT void MvrUtil::putCurrentSecondInString(char* s, size_t len)
{
  struct tm t;
  MvrUtil::localtime(&t);
  snprintf(s, len, "%02d", t.tm_sec);
  s[len-1] = '\0';
}



MVREXPORT time_t MvrUtil::parseTime(const char *str, bool *ok, bool toToday)
{

  struct tm tmOut;
  if (toToday)
  {
    struct tm now;
    if (!localtime(&now))
    {
      *ok = false;
      return 0;
    }
    memcpy(&tmOut, &now, sizeof(now));
  }
  else
  {
    memset(&tmOut, 0, sizeof(tmOut));
    // The day-of-the-month starts at 1 (not 0)...
    tmOut.tm_mday = 1;
    // Setting the year to 70 because if it is left at 0 or 1, then
    // the call to mktime() returns an apparently bogus value.  Think
    // that 70 makes sense since times are generally measured from
    // 1/1/1970 (but still, it's all a little strange).
    tmOut.tm_year = 70;
    tmOut.tm_isdst = -1; // Negative value means unknown
  }

  bool isValid = true;
  int hrs = -1;
  int min = -1;
  int sec = 0;

  MvrArgumentBuilder separator(512, ':');
  separator.add(str);
  
  // if there's the wrong number of args, or any of the args aren't
  // integers then it's invalid and we won't parse it
  if ((separator.getArgc() != 2 && separator.getArgc() != 3) || 
      !separator.isArgInt(0) || !separator.isArgInt(1) ||
      (separator.getArgc() == 3 && !separator.isArgInt(2)))
  {
    //printf("Invalid... %d\n", separator.getArgc());
    //separator.log();
    isValid = false;
  }
  else
  {
    hrs = separator.getArgInt(0);
    min = separator.getArgInt(1);
    if (separator.getArgc() == 3)
      sec = separator.getArgInt(2);
    //printf("Was %02d:%02d:%02d", hrs, min, sec);
  }

  /*
  char *tempBuf = new char[strlen(str) + 1];
  strncpy(tempBuf, str, sizeof(tempBuf));
  
  // Attempted to use strptime, but it doesn't seem to be universally
  // available.
  char *pch = strtok(tempBuf, ":");
  if (pch != NULL) {
    hrs = atoi(pch);
  }
  
  pch = strtok(NULL, ":");
  if (pch != NULL) {
    min = atoi(pch);
  }
  */

  // make sure the actual numbers are valid
  if (!((hrs  >= 0) && (hrs < 24) && (min >= 0) && (min < 60) && 
	(sec >= 0) && (sec < 60)))
    isValid = false;
  
  if (isValid) 
  {
    tmOut.tm_hour = hrs;
    tmOut.tm_min  = min;
    tmOut.tm_sec  = sec;
  }
  
  time_t newTime = mktime(&tmOut);
  
  if (ok != NULL) 
  {
    *ok = (isValid && (newTime != -1));
  }
  
  //delete [] tempBuf;
  
  return newTime;

} // end method parseTime



MVREXPORT bool MvrUtil::localtime(const time_t *timep, struct tm *result) 
{
#ifdef WIN32
  ourLocaltimeMutex.lock();
  struct tm *r = ::localtime(timep);
  if(r == NULL) { 
    ourLocaltimeMutex.unlock();
    return false;
  }
  *result = *r; // copy the 'struct tm' object before unlocking.
  ourLocaltimeMutex.unlock();
  return true;
#else
  return (::localtime_r(timep, result) != NULL);
#endif
}

/** Call MvrUtil::localtime() with the current time obtained by calling
* time(NULL).
*  @return false on error (e.g. invalid input), otherwise true.
*/
MVREXPORT bool MvrUtil::localtime(struct tm *result) 
{ 
  time_t now = time(NULL);
  return MvrUtil::localtime(&now, result); 
}


#ifndef WIN32
/**
   @param baseDir the base directory to work from
   @param fileName the fileName to squash the case from
   
   @param result where to put the result
   @param resultLen length of the result

   @return true if it could find the file, the result is in result,
   false if it couldn't find the file
**/
MVREXPORT bool MvrUtil::matchCase(const char *baseDir, 
					   const char *fileName,
					   char *result,
					   size_t resultLen)
{

  /***
  MvrLog::log(MvrLog::Normal, 
             "MvrUtil::matchCase() baseDir = \"%s\" fileName = \"%s\"",
             baseDir,
             fileName);
  ***/

  DIR *dir;
  struct dirent *ent;

  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif

  result[0] = '\0';

  std::list<std::string> split = splitFileName(fileName);
  std::list<std::string>::iterator it = split.begin();
  std::string finding = (*it);

  /*
  for (it = split.begin(); it != split.end(); it++)
  {
    printf("@@@@@@@@ %s\n", (*it).c_str());
  }
  */
  
  // how this works is we start at the base dir then read through
  // until we find what the next name we need, if entry is a directory
  // and we're not at the end of our string list then we change into
  // that dir and the while loop keeps going, if the entry isn't a
  // directory and matchs and its the last in our string list we've
  // found what we want
  if ((dir = opendir(baseDir)) == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrUtil: No such directory '%s' for base", 
	       baseDir);
    return false;
  }

  if (finding == ".")
  {
    it++;
    if (it != split.end())
    {
      finding = (*it);
    }
    else
    {
      MvrLog::log(MvrLog::Normal, 
		             "MvrUtil: No file or directory given (base = %s file = %s)", 
		             baseDir,
                 fileName);
      closedir(dir);

      // KMC NEED TO DETERMINE WHICH IS CORRECT.
      // The following change appears to be necessary for maps, but is still
      // undergoing testing....
      //   Just return the given ".". (This is necessary to find maps in the local 
      //   directory under some circumstances.)
      //   snprintf(result, resultLen, finding.c_str());
      //   return true;  

      return false;

      
    }
  }

  while ((ent = readdir(dir)) != NULL)
  {
    // ignore some of these
    if (ent->d_name[0] == '.')
    {
      //printf("Ignoring %s\n", ent->d_name[0]);
      continue;
    }
    //printf("NAME %s finding %s\n", ent->d_name, finding.c_str());
    
    // we've found what we were looking for
    if (MvrUtil::strcasecmp(ent->d_name, finding) == 0)
    {
      size_t lenOfResult;
      lenOfResult = strlen(result);

      // make sure we can put the filename in
      if (strlen(ent->d_name) > resultLen - lenOfResult - 2)
      {
	MvrLog::log(MvrLog::Normal, 
		   "MvrUtil::matchCase: result not long enough");
	closedir(dir);
	return false;
      }
      //printf("Before %s", result);
      if (lenOfResult != 0)
      {
	result[lenOfResult] = separator;
	result[lenOfResult+1] = '\0';
      }
      // put the filename in
      strcpy(&result[strlen(result)], ent->d_name);
      //printf("after %s\n", result);
      // see if we're at the end
      it++;
      if (it != split.end())
      {
	//printf("Um.........\n");
	finding = (*it);
	std::string wholeDir;
	wholeDir = baseDir;
	wholeDir += result;
	closedir(dir);
	//printf("'%s' '%s' '%s'\n", baseDir, result, wholeDir.c_str());
	if ((dir = opendir(wholeDir.c_str())) == NULL)
	{
	  MvrLog::log(MvrLog::Normal, 
		     "MvrUtil::matchCase: Error going into %s", 
		     result);
	  return false;
	}
      }
      else
      {
	//printf("\n########## Got it %s\n", result);
	closedir(dir);
	return true;
      }
    }
  }
  MvrLog::log(MvrLog::Normal, 
	     "MvrUtil::matchCase: %s doesn't exist in %s", fileName, 
	     baseDir);
  //printf("!!!!!!!! %s", finding.c_str());
  closedir(dir);
  return false;
} 

#endif // !WIN32


MVREXPORT bool MvrUtil::getDirectory(const char *fileName, 
					     char *result, size_t resultLen)
{
  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif
  
  if (fileName == NULL || fileName[0] == '\0' || resultLen == 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrUtil: getDirectory, bad setup");
    return false;
  }
  
  // just play in the result buffer
  strncpy(result, fileName, resultLen - 1);
  // make sure its nulled
  result[resultLen - 1] = '\0';
  char *toPos;
  MvrUtil::fixSlashes(result, resultLen);
  // see where the last directory is
  toPos = strrchr(result, separator);
  // if there's no divider it must just be a file name
  if (toPos == NULL)
  {
    result[0] = '\0';
    return true;
  }
  // otherwise just toss a null into the last separator and we're done
  else
  {    
    *toPos = '\0';
    return true;
  }
}

MVREXPORT bool MvrUtil::getFileName(const char *fileName, 
					 char *result, size_t resultLen)
{
  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif
  
  if (fileName == NULL || fileName[0] == '\0' || resultLen == 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrUtil: getFileName, bad setup");
    return false;
  }

  char *str;
  size_t fileNameLen = strlen(fileName);
  str = new char[fileNameLen + 1];
  //printf("0 %s\n", fileName);
  // just play in the result buffer
  strncpy(str, fileName, fileNameLen);
  // make sure its nulled
  str[fileNameLen] = '\0';
  //printf("1 %s\n", str);

  char *toPos;
  MvrUtil::fixSlashes(str, fileNameLen + 1);
  //printf("2 %s\n", str);
  // see where the last directory is
  toPos = strrchr(str, separator);
  // if there's no divider it must just be a file name
  if (toPos == NULL)
  {
    // copy the filename in and make sure it has a null
    strncpy(result, str, resultLen - 1);
    result[resultLen - 1] = '\0';
    //printf("3 %s\n", result);
    delete[] str;
    return true;
  }
  // otherwise take the section from that separator to the end
  else
  {
    strncpy(result, &str[toPos - str + 1], resultLen - 2);
    result[resultLen - 1] = '\0';
    //printf("4 %s\n", result);
    delete[] str;
    return true;
  }
}

#ifndef WIN32
/**
   This function assumes the slashes are all heading the right way already.
**/
std::list<std::string> MvrUtil::splitFileName(const char *fileName)
{
  std::list<std::string> split;
  if (fileName == NULL)
    return split;

  char separator;  
#ifndef WIN32
  separator = '/';
#else
  separator = '\\';
#endif

  size_t len;
  size_t i;
  size_t last;
  bool justSepped;
  char entry[2048];
  for (i = 0, justSepped = false, last = 0, len = strlen(fileName); 
       ; 
       i++)
  {
    
    if ((fileName[i] == separator && !justSepped) 
	|| fileName[i] == '\0' || i >= len)
    {
      if (i - last > 2047)
      {
	MvrLog::log(MvrLog::Normal, "MvrUtil::splitFileName: some directory or file too long");
      }
      if (!justSepped)
      {
	strncpy(entry, &fileName[last], i - last);
	entry[i-last] = '\0';
	split.push_back(entry);

	justSepped = true;
      }
      if (fileName[i] == '\0' || i >= len)
	return split;
    }
    else if (fileName[i] == separator && justSepped)
    {
      justSepped = true;
      last = i;
    }
    else if (fileName[i] != separator && justSepped)
    {
      justSepped = false;
      last = i;
    }
  }
  MvrLog::log(MvrLog::Normal, "MvrUtil::splitFileName: file str ('%s') happened weird", fileName);
  return split;
}



#endif // !WIN32


MVREXPORT bool MvrUtil::changeFileTimestamp(const char *fileName, 
                                          time_t timestamp) 
{
  if (MvrUtil::isStrEmpty(fileName)) {
    MvrLog::log(MvrLog::Normal,
               "Cannot change date on file with empty name");
    return false;
  }
#ifdef WIN32

  FILETIME fileTime;

  HANDLE hFile = CreateFile(fileName,
                            GENERIC_READ | GENERIC_WRITE,
                             0,NULL,
                             OPEN_EXISTING,
                             0,NULL);

  if (hFile == NULL) {
    return false;
  }


  // The following is extracted from the MSDN article "Converting a time_t Value
  // to a File Time".
  LONGLONG temp = Int32x32To64(timestamp, 10000000) + 116444736000000000;
  fileTime.dwLowDateTime = (DWORD) temp;
  fileTime.dwHighDateTime = temp >> 32;

  SetFileTime(hFile, 
              &fileTime, 
              (LPFILETIME) NULL,  // don't change last access time (?)
              &fileTime);

  CloseHandle(hFile);

#else // unix
        
  char timeBuf[500];
  strftime(timeBuf, sizeof(timeBuf), "%c", ::localtime(&timestamp));
  MvrLog::log(MvrLog::Normal,
             "Changing file %s modified time to %s",
             fileName,
             timeBuf);


  // time_t newTime = mktime(&timestamp);
  struct utimbuf fileTime;
  fileTime.actime  = timestamp;
  fileTime.modtime = timestamp;
  utime(fileName, &fileTime);

#endif // else unix

  return true;

} // end method changeFileTimestamp



MVREXPORT void MvrUtil::setFileCloseOnExec(int fd, bool closeOnExec)
{
#ifndef WIN32
  if (fd <= 0)
    return;

  int flags;

  if ((flags = fcntl(fd, F_GETFD)) < 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrUtil::setFileCloseOnExec: Cannot use F_GETFD in fnctl on fd %d", fd);
    return;
  }

  if (closeOnExec)
    flags |= FD_CLOEXEC;
  else
    flags &= ~FD_CLOEXEC;

  if (fcntl(fd, F_SETFD, flags) < 0)
  {
    MvrLog::log(MvrLog::Normal, "MvrUtil::setFileCloseOnExec: Cannot use F_GETFD in fnctl on fd %d", fd);
    return;
  }
#endif
}

MVREXPORT void MvrUtil::setFileCloseOnExec(FILE *file, bool closeOnExec)
{
  if (file != NULL)
    setFileCloseOnExec(fileno(file));
}

MVREXPORT FILE *MvrUtil::fopen(const char *path, const char *mode, 
			     bool closeOnExec)
{
  FILE *file;
  file = ::fopen(path, mode);
  setFileCloseOnExec(file, closeOnExec);
  return file;
}

MVREXPORT int MvrUtil::open(const char *pathname, int flags, 
			  bool closeOnExec)
{
  int fd;
  fd = ::open(pathname, flags);
  setFileCloseOnExec(fd, closeOnExec);
  return fd;
}

MVREXPORT int MvrUtil::open(const char *pathname, int flags, mode_t mode, 
			  bool closeOnExec)
{
  int fd;
  fd = ::open(pathname, flags, mode);
  setFileCloseOnExec(fd, closeOnExec);
  return fd;
}

MVREXPORT int MvrUtil::close(int fd)
{
	return ::close(fd);
}

MVREXPORT int MvrUtil::creat(const char *pathname, mode_t mode, 
			   bool closeOnExec)
{
  int fd;
  fd = ::creat(pathname, mode);
  setFileCloseOnExec(fd, closeOnExec);
  return fd;
}

MVREXPORT FILE *MvrUtil::popen(const char *command, const char *type, 
			     bool closeOnExec)
{
  FILE *file;
#ifndef WIN32
  file = ::popen(command, type);
#else
  file = _popen(command, type);
#endif
  setFileCloseOnExec(file, closeOnExec);
  return file;
}


MVREXPORT bool MvrUtil::floatIsNormal(double f)
{
#ifdef WIN32
	  return (!::_isnan(f) && ::_finite(f));
#else
	  return isnormal(f);
#endif
}

MVREXPORT int MvrUtil::atoi(const char *str, bool *ok, bool forceHex) 
{
  bool isSuccess = false;
  int ret = 0;

  // if the argument isn't bogus
  if (str != NULL) {
  
    int base = 10;
    if (forceHex)
      base = 16;
    // see if it has the hex prefix and strip it
    if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
      str = &str[2];
      base = 16;
    }
    char *endPtr = NULL;
    ret = strtol(str, &endPtr, base);
 
    if (endPtr[0] == '\0' && endPtr != str) {
      isSuccess = true;
    }
  } // end if valid arg

  if (ok != NULL) {
    *ok = isSuccess;
  }
  
  if (isSuccess) 
    return ret;
  else 
    return 0;

} // end method atoi


MVREXPORT long MvrMath::randomInRange(long m, long n)
{
    // simple method
    return m + random() / (ourRandMax / (n - m + 1) + 1);
    // alternate method is to use drand48, multiply and round (does Windows have
    // drand48?), or keep trying numbers until we get one in range.
}

MVREXPORT double MvrMath::epsilon() { return ourEpsilon; }
MVREXPORT long MvrMath::getRandMax() { return ourRandMax; }

#ifndef MVRINTERFACE

MvrGlobalRetFunctor2<MvrLaser *, int, const char *> 
MvrLaserCreatorHelper::ourLMS2xxCB(&MvrLaserCreatorHelper::createLMS2xx);

MvrGlobalRetFunctor2<MvrLaser *, int, const char *> 
MvrLaserCreatorHelper::ourUrgCB(&MvrLaserCreatorHelper::createUrg);

MvrGlobalRetFunctor2<MvrLaser *, int, const char *> 
MvrLaserCreatorHelper::ourLMS1XXCB(&MvrLaserCreatorHelper::createLMS1XX);

MvrGlobalRetFunctor2<MvrLaser *, int, const char *> 
MvrLaserCreatorHelper::ourS3SeriesCB(&MvrLaserCreatorHelper::createS3Series);

MvrGlobalRetFunctor2<MvrLaser *, int, const char *>
MvrLaserCreatorHelper::ourUrg_2_0CB(&MvrLaserCreatorHelper::createUrg_2_0);

MvrGlobalRetFunctor2<MvrLaser *, int, const char *>
MvrLaserCreatorHelper::ourLMS5XXCB(&MvrLaserCreatorHelper::createLMS5XX);

MvrGlobalRetFunctor2<MvrLaser *, int, const char *>
MvrLaserCreatorHelper::ourTiM3XXCB(&MvrLaserCreatorHelper::createTiM3XX);

MvrGlobalRetFunctor2<MvrLaser *, int, const char *>
MvrLaserCreatorHelper::ourSZSeriesCB(&MvrLaserCreatorHelper::createSZSeries);

MvrLaser *createAnyLMS1xx(int laserNumber, const char *logPrefix, const char *name, MvrLMS1XX::LaserModel model)
{
	return new MvrLMS1XX(laserNumber, name, model);
}

MvrGlobalRetFunctor4<MvrLaser*, int, const char*, const char *, MvrLMS1XX::LaserModel>
TiM551CB(&createAnyLMS1xx, -1, "", "tim551", MvrLMS1XX::TiM551);

MvrGlobalRetFunctor4<MvrLaser*, int, const char*, const char *, MvrLMS1XX::LaserModel>
TiM561CB(&createAnyLMS1xx, -1, "", "tim561", MvrLMS1XX::TiM561); 

MvrGlobalRetFunctor4<MvrLaser*, int, const char*, const char *, MvrLMS1XX::LaserModel>
TiM571CB(&createAnyLMS1xx, -1, "", "tim571", MvrLMS1XX::TiM571);

MvrRetFunctor2<MvrLaser *, int, const char *> *
MvrLaserCreatorHelper::getCreateTiM551CB(void) {
  return &TiM551CB;
}

MvrRetFunctor2<MvrLaser *, int, const char *> *
MvrLaserCreatorHelper::getCreateTiM561CB(void) {
  return &TiM561CB;
}

MvrRetFunctor2<MvrLaser *, int, const char *> *
MvrLaserCreatorHelper::getCreateTiM571CB(void) {
  return &TiM571CB;
}


MvrGlobalRetFunctor2<MvrBatteryMTX *, int, const char *>
MvrBatteryMTXCreatorHelper::ourBatteryMTXCB(&MvrBatteryMTXCreatorHelper::createBatteryMTX);

MvrGlobalRetFunctor2<MvrLCDMTX *, int, const char *>
MvrLCDMTXCreatorHelper::ourLCDMTXCB(&MvrLCDMTXCreatorHelper::createLCDMTX);

MvrGlobalRetFunctor2<MvrSonarMTX *, int, const char *>
MvrSonarMTXCreatorHelper::ourSonarMTXCB(&MvrSonarMTXCreatorHelper::createSonarMTX);


MvrLaser *MvrLaserCreatorHelper::createLMS2xx(int laserNumber, 
					    const char *logPrefix)
{
  return new MvrLMS2xx(laserNumber);
}

MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateLMS2xxCB(void)
{
  return &ourLMS2xxCB;
}

MvrLaser *MvrLaserCreatorHelper::createUrg(int laserNumber, const char *logPrefix)
{
  return new MvrUrg(laserNumber);
}


MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateUrgCB(void)
{
  return &ourUrgCB;
}

MvrLaser *MvrLaserCreatorHelper::createLMS1XX(int laserNumber, const char *logPrefix)
{
	return new MvrLMS1XX(laserNumber, "lms1xx", MvrLMS1XX::LMS1XX);
}

MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateLMS1XXCB(void)
{
  return &ourLMS1XXCB;
}

MvrLaser *MvrLaserCreatorHelper::createS3Series(int laserNumber, 
					    const char *logPrefix)
{
  return new MvrS3Series(laserNumber);
}

MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateS3SeriesCB(void)
{
  return &ourS3SeriesCB;
}


MvrLaser *MvrLaserCreatorHelper::createUrg_2_0(int laserNumber, 
					     const char *logPrefix)
{
  return new MvrUrg_2_0(laserNumber);
}


MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateUrg_2_0CB(void)
{
  return &ourUrg_2_0CB;
}

MvrLaser *MvrLaserCreatorHelper::createLMS5XX(int laserNumber,
		const char *logPrefix)
{

	// PS 8/22/11 - added "lms5xx" and flag specifying laser is an lms5xx
	return new MvrLMS1XX(laserNumber, "lms5XX", MvrLMS1XX::LMS5XX);
}

MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateLMS5XXCB(void)
{
  return &ourLMS5XXCB;
}

MvrLaser *MvrLaserCreatorHelper::createTiM3XX(int laserNumber,
		const char *logPrefix)
{

	// PS 8/22/11 - added "lms5xx" and flag specifying laser is an lms5xx
	return new MvrLMS1XX(laserNumber, "tim3XX", MvrLMS1XX::TiM3XX);
}

MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateTiM3XXCB(void)
{
  return &ourTiM3XXCB;
}

MvrLaser *MvrLaserCreatorHelper::createSZSeries(int laserNumber,
					    const char *logPrefix)
{
  return new MvrSZSeries(laserNumber);
}

MvrRetFunctor2<MvrLaser *, int, const char *> *MvrLaserCreatorHelper::getCreateSZSeriesCB(void)
{
  return &ourSZSeriesCB;
}

MvrBatteryMTX *MvrBatteryMTXCreatorHelper::createBatteryMTX(int batteryNumber,
					    const char *logPrefix)
{
  return new MvrBatteryMTX(batteryNumber);
}

MvrRetFunctor2<MvrBatteryMTX *, int, const char *> *MvrBatteryMTXCreatorHelper::getCreateBatteryMTXCB(void)
{
  return &ourBatteryMTXCB;
}

MvrLCDMTX *MvrLCDMTXCreatorHelper::createLCDMTX(int lcdNumber,
					    const char *logPrefix)
{
  return new MvrLCDMTX(lcdNumber);
}

MvrRetFunctor2<MvrLCDMTX *, int, const char *> *MvrLCDMTXCreatorHelper::getCreateLCDMTXCB(void)
{
  return &ourLCDMTXCB;
}

MvrSonarMTX *MvrSonarMTXCreatorHelper::createSonarMTX(int sonarNumber,
					    const char *logPrefix)
{
  return new MvrSonarMTX(sonarNumber);
}

MvrRetFunctor2<MvrSonarMTX *, int, const char *> *MvrSonarMTXCreatorHelper::getCreateSonarMTXCB(void)
{
  return &ourSonarMTXCB;
}

#endif // MVRINTERFACE

MvrGlobalRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> 
MvrDeviceConnectionCreatorHelper::ourSerialCB(
	&MvrDeviceConnectionCreatorHelper::createSerialConnection);
MvrGlobalRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> 
MvrDeviceConnectionCreatorHelper::ourTcpCB(
	&MvrDeviceConnectionCreatorHelper::createTcpConnection);
MvrGlobalRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *>
MvrDeviceConnectionCreatorHelper::ourSerial422CB(
	&MvrDeviceConnectionCreatorHelper::createSerial422Connection);
MvrLog::LogLevel MvrDeviceConnectionCreatorHelper::ourSuccessLogLevel = MvrLog::Verbose;

MvrDeviceConnection *MvrDeviceConnectionCreatorHelper::createSerialConnection(
	const char *port, const char *defaultInfo, const char *logPrefix)
{
	MvrDeviceConnection *devConn;

	devConn = internalCreateSerialConnection(port, defaultInfo, logPrefix, false);

	return devConn;
}

MvrDeviceConnection *MvrDeviceConnectionCreatorHelper::createSerial422Connection(
	const char *port, const char *defaultInfo, const char *logPrefix)
{
	MvrDeviceConnection *devConn;

	devConn = internalCreateSerialConnection(port, defaultInfo, logPrefix, true);

	return devConn;
}


MvrDeviceConnection *MvrDeviceConnectionCreatorHelper::internalCreateSerialConnection(
	const char *port, const char *defaultInfo, const char *logPrefix, bool is422)
{
  MvrSerialConnection *serConn = new MvrSerialConnection(is422);
  
  std::string serPort;
  if (strcasecmp(port, "COM1") == 0)
    serPort = MvrUtil::COM1;
  else if (strcasecmp(port, "COM2") == 0)
    serPort = MvrUtil::COM2;
  else if (strcasecmp(port, "COM3") == 0)
    serPort = MvrUtil::COM3;
  else if (strcasecmp(port, "COM4") == 0)
    serPort = MvrUtil::COM4;
  else if (strcasecmp(port, "COM5") == 0)
    serPort = MvrUtil::COM5;
  else if (strcasecmp(port, "COM6") == 0)
    serPort = MvrUtil::COM6;
  else if (strcasecmp(port, "COM7") == 0)
    serPort = MvrUtil::COM7;
  else if (strcasecmp(port, "COM8") == 0)
    serPort = MvrUtil::COM8;
  else if (strcasecmp(port, "COM9") == 0)
    serPort = MvrUtil::COM9;
  else if (strcasecmp(port, "COM10") == 0)
    serPort = MvrUtil::COM10;
  else if (strcasecmp(port, "COM11") == 0)
    serPort = MvrUtil::COM11;
  else if (strcasecmp(port, "COM12") == 0)
    serPort = MvrUtil::COM12;
  else if (strcasecmp(port, "COM13") == 0)
    serPort = MvrUtil::COM13;
  else if (strcasecmp(port, "COM14") == 0)
    serPort = MvrUtil::COM14;
  else if (strcasecmp(port, "COM15") == 0)
    serPort = MvrUtil::COM15;
  else if (strcasecmp(port, "COM16") == 0)
    serPort = MvrUtil::COM16;
  else if (port != NULL)
    serPort = port;
  
  MvrLog::log(ourSuccessLogLevel, "%sSet serial port to open %s", 
	     logPrefix, serPort.c_str());
  serConn->setPort(serPort.c_str());
  return serConn;
  /*  
      This code is commented out because it created problems with demo
      (or any other program that used MvrLaserConnector::connectLasers
      with addAllLasersToRobot as true)

  int ret;
  
  if ((ret = serConn->open(serPort.c_str())) == 0)
  {
    MvrLog::log(ourSuccessLogLevel, "%sOpened serial port %s", 
	       logPrefix, serPort.c_str());
    return serConn;
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "%sCould not open serial port %s (from %s), because %s", 
	       logPrefix, serPort.c_str(), port,
	       serConn->getOpenMessage(ret));
    delete serConn;
    return NULL;
  }
  */
}


MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *
MvrDeviceConnectionCreatorHelper::getCreateSerialCB(void)
{
  return &ourSerialCB;
}

MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *
MvrDeviceConnectionCreatorHelper::getCreateSerial422CB(void)
{
  return &ourSerial422CB;
}

MvrDeviceConnection *MvrDeviceConnectionCreatorHelper::createTcpConnection(
	const char *port, const char *defaultInfo, const char *logPrefix)
{
  MvrTcpConnection *tcpConn = new MvrTcpConnection;
  //int ret;

  tcpConn->setPort(port, atoi(defaultInfo));
  MvrLog::log(ourSuccessLogLevel, 
	     "%sSet tcp connection to open %s (and port %d)", 
	     logPrefix, port, atoi(defaultInfo));
  return tcpConn;

  /*
      This code is commented out because it created problems with demo
      (or any other program that used MvrLaserConnector::connectLasers
      with addAllLasersToRobot as true)
  
  if ((ret = tcpConn->open(port, atoi(defaultInfo))) == 0)
  {
    MvrLog::log(ourSuccessLogLevel, 
	       "%sOpened tcp connection from %s (and port %d)", 
	       logPrefix, port, atoi(defaultInfo));
    return tcpConn;
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "%sCould not open a tcp connection to host '%s' with default port %d (from '%s'), because %s", 
	       logPrefix, port, atoi(defaultInfo), defaultInfo,
	       tcpConn->getOpenMessage(ret));
    delete tcpConn;
    return NULL;
  }
  */
}

MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *
MvrDeviceConnectionCreatorHelper::getCreateTcpCB(void)
{
  return &ourTcpCB;
}

void MvrDeviceConnectionCreatorHelper::setSuccessLogLevel(
	MvrLog::LogLevel successLogLevel)
{
  ourSuccessLogLevel = successLogLevel;
}

MvrLog::LogLevel MvrDeviceConnectionCreatorHelper::setSuccessLogLevel(void)
{
  return ourSuccessLogLevel;
}

MVREXPORT std::list<MvrPose> MvrPoseUtil::findCornersFromRobotBounds(
	double radius, double widthLeft, double widthRight, 
	double lengthFront, double lengthRear, bool fastButUnsafe)
{

  std::list<MvrPose> ret;

  if (fastButUnsafe)
  {
    MvrPose frontLeft;   
    if (lengthFront >= radius && widthLeft >= radius)
      frontLeft.setPose(lengthFront,
			widthLeft);
    else if (lengthFront >= radius)
      frontLeft.setPose(lengthFront,
			0);
    else
      frontLeft.setPose(lengthFront,
			sqrt(radius * radius - lengthFront * lengthFront));
    
    MvrPose leftFront;
    if (widthLeft >= radius && lengthFront >= radius)
      leftFront.setPose(lengthFront, 
			widthLeft);
    else if (widthLeft >= radius)
      leftFront.setPose(0, 
			widthLeft);
    else
      leftFront.setPose(sqrt(radius * radius - widthLeft * widthLeft),
			widthLeft);

    MvrPose leftRear;
    if (widthLeft >= radius && lengthRear >= radius)
      leftRear.setPose(-lengthRear, 
		       widthLeft);
    else if (widthLeft >= radius)
      leftRear.setPose(0, 
		       widthLeft);
    else
      leftRear.setPose(-sqrt(radius * radius - widthLeft * widthLeft),
		       widthLeft);

    MvrPose rearLeft;
    if (lengthRear >= radius && widthLeft >= radius)
      rearLeft.setPose(-lengthRear, 
		       widthLeft);
    else if (lengthRear >= radius)
      rearLeft.setPose(-lengthRear, 
		       0);
    else
      rearLeft.setPose(-lengthRear,
		       sqrt(radius * radius - lengthRear * lengthRear ));


    MvrPose rearRight;
    if (lengthRear >= radius && widthRight >= radius)
      rearRight.setPose(-lengthRear, 
			-widthRight);
    else if (lengthRear >= radius)
      rearRight.setPose(-lengthRear, 
		       0);
    else
      rearRight.setPose(-lengthRear,
			-sqrt(radius * radius - lengthRear * lengthRear));


    MvrPose rightRear;
    if (widthRight >= radius && lengthRear >= radius)      
      rightRear.setPose(-lengthRear, 
			-widthRight);
    else if (widthRight >= radius)
      rightRear.setPose(0, 
			-widthRight);
    else
      rightRear.setPose(-sqrt(radius * radius - widthRight * widthRight),
			-widthRight);

    MvrPose rightFront;
    if (widthRight >= radius && lengthFront >= radius)
      rightFront.setPose(lengthFront, 
			 -widthRight);
    else if (widthRight >= radius)
      rightFront.setPose(0, 
			-widthRight);
    else
      rightFront.setPose(sqrt(radius * radius - widthRight * widthRight),
			 -widthRight);

    MvrPose frontRight;
    if (lengthFront >= radius && widthRight >= radius)
      frontRight.setPose(lengthFront,
			 -widthRight);
    else if (lengthFront >= radius)
      frontRight.setPose(lengthFront,
			0);
    else
      frontRight.setPose(lengthFront,
			 -sqrt(radius * radius - lengthFront * lengthFront));

    if (frontRight.squaredFindDistanceTo(frontLeft) > 1)
      ret.push_back(frontLeft);

    if (frontLeft.squaredFindDistanceTo(leftFront) > 1)
      ret.push_back(leftFront);
    if (leftFront.squaredFindDistanceTo(leftRear) > 1)
      ret.push_back(leftRear);

    if (leftRear.squaredFindDistanceTo(rearLeft) > 1) 
      ret.push_back(rearLeft);
    if (rearLeft.squaredFindDistanceTo(rearRight) > 1) 
      ret.push_back(rearRight);

    if (rearRight.squaredFindDistanceTo(rightRear) > 1)
      ret.push_back(rightRear);
    if (rightRear.squaredFindDistanceTo(rightFront) > 1)
      ret.push_back(rightFront);

    if (rightFront.squaredFindDistanceTo(frontRight) > 1)
      ret.push_back(frontRight);
    return ret;
  }

  return ret;

  /// MPL this code worked, but didn't give us good corners when the width/length got nuts
#if 0
  if (fastButUnsafe)
  {

    MvrPose frontLeft;
    if (lengthFront >= radius)
      frontLeft.setPose(lengthFront,
			0);
    else
      frontLeft.setPose(lengthFront,
			sqrt(radius * radius - lengthFront * lengthFront));
    
    MvrPose leftFront;
    if (widthLeft >= radius)
      leftFront.setPose(0, 
			widthLeft);
    else
      leftFront.setPose(sqrt(radius * radius - widthLeft * widthLeft),
			widthLeft);

    MvrPose leftRear(-leftFront.getX(), 
		    leftFront.getY());
    /*
    MvrPose leftRear;
    if (widthLefth >= radius)
      leftRear.setPose(0, widthLeft);
    else
      leftRear.setPose(-sqrt(radius * radius - widthLeft * widthLeft),
		       widthLeft);
    */

    MvrPose rearLeft;
    if (lengthRear >= radius)
      rearLeft.setPose(-lengthRear, 
		       0);
    else
      rearLeft.setPose(-lengthRear,
		       sqrt(radius * radius - lengthRear * lengthRear ));

    MvrPose rearRight(rearLeft.getX(), 
		     -rearLeft.getY());
    /*
    MvrPose rearRight;
    if (lengthRear >= radius)
      rightRear.setPose(lengthRear, 
		       0);
    else
      rightRear.setPose(lengthRear,
			-sqrt(radius * radius - lengthRear * lengthRear));
    */

    MvrPose rightRear;
    if (widthRight >= radius)
      rightRear.setPose(0, 
			-widthRight);
    else
      rightRear.setPose(-sqrt(radius * radius - widthRight * widthRight),
			-widthRight);

    MvrPose rightFront(-rightRear.getX(),
		      rightRear.getY());
    /*
    MvrPose rightFront;
    if (widthRight >= radius)
      rightFront.setPose(0, 
			-widthRight);
    else
      rightFront.setPose(sqrt(radius * radius - widthRight * widthRight),
			 -widthRight);
    */

    MvrPose frontRight(frontLeft.getX(),
		      -frontLeft.getY());
    /*
    MvrPose frontRight;
    if (lengthFront >= radius)
      rightFront.setPose(lengthFront,
			0);
    else
      rightFront.setPose(lengthFront,
			 -sqrt(radius * radius - lengthFront * lengthFront));
    */

    std::list<MvrPose> ret;
    ret.push_back(frontLeft);

    ret.push_back(leftFront);
    ret.push_back(leftRear);

    ret.push_back(rearLeft);
    ret.push_back(rearRight);

    ret.push_back(rightRear);
    ret.push_back(rightFront);

    ret.push_back(frontRight);
    return ret;
  }
#endif	    
}


MVREXPORT std::list<MvrPose> MvrPoseUtil::breakUpDistanceEvenly(
	MvrPose start, MvrPose end, int resolution)
{
  std::list<MvrPose> ret;

  ret.push_back(start);

  double dist = start.findDistanceTo(end);
  double angle = start.findAngleTo(end);
  double cos = MvrMath::cos(angle);
  double sin = MvrMath::sin(angle);

  if (dist > resolution)
  {
    // we're using integer truncation here
    int steps = dist / resolution + 1;
    double increment = dist / steps;

    double atX = start.getX();
    double atY = start.getY();
    
    // now walk the length of the line and see if we should put the points in
    for (int ii = 1; ii <= steps; ii++)
    {
      atX += increment * cos;
      atY += increment * sin;
      ret.push_back(MvrPose(atX, atY));
    }
  }

  ret.push_back(end);
  return ret;
}

MVREXPORT MvrTimeChecker::MvrTimeChecker(const char *name, int defaultMSecs)
{
  if (name != NULL)
    myName = name;
  else
    myName = "Unknown";
  myMSecs = defaultMSecs;
}

MVREXPORT MvrTimeChecker::~MvrTimeChecker()
{

}

MVREXPORT void MvrTimeChecker::start(void)
{
  myStarted.setToNow();
  myLastCheck.setToNow();
}

MVREXPORT void MvrTimeChecker::check(const char *subName)
{
  long long took = myLastCheck.mSecSinceLL();

  if (took > (long long) myMSecs && subName != NULL)
    MvrLog::log(MvrLog::Normal, "%s::%s took too long (%lld msecs) in thread %s",
	       myName.c_str(), subName, took, 
	       MvrThread::self()->getThreadName());

  myLastCheck.setToNow();
}


MVREXPORT void MvrTimeChecker::finish(void)
{
  long long took = myStarted.mSecSinceLL();

  if (took > (long long) myMSecs)
    MvrLog::log(MvrLog::Normal, "%s took too long (%lld msecs) in thread %s",
	       myName.c_str(), took, MvrThread::self()->getThreadName());
}
