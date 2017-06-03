/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : mvriaUtil.cpp
 > Description  : Some Utility classes and functions
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月02日
***************************************************************************************************/
#define _GNU_SOURCE 1
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
  however, use the MvrTime timer utility instead, or the MvrUtil::getTime() method
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
  if (gettimeofday(&tv, NULL) == 0)
    return tv.tv_usec/1000 + (tv.tv_sec % 1000000) * 1000;
  else
    return 0;
#elif defined(WIN32)
  return timeGetTime();
#endif
}

#ifdef WIN32

/*
 * @param fileName name of the file to size
 * @return size in byte, -1 on error
 */
MVREXPORT long MvrUtil::sizeFile(std::string fileName)
{
  struct _stat buf;
  if (_stat(fileName.c_str(), &buf) < 0)
    return -1;
  if (!(buf.st_mode | _S_IFREG))
    return -1;
  return (buf.st_size);  
}

/*
 * @param fileName of the file to size
 * @return size in byte, -1 on error
 */
MVREXPORT long MvrUtil::sizeFile(const char *fileName)
{
  struct _stat buf;
  if (_stat(fileName, &buf) < 0)
    return -1;
  if (!(buf.st_mode | _S_IFREG))
    return -1;
  return (buf.st_size);  
}

#else
/*
 * @param fileName of the file to size
 * @return size in byte, -1 on error
 */
MVREXPORT long MvrUtil::sizeFile(std::string fileName)
{
  struct stat buf;

  if (stat(fileName.c_str(), &buf) < 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrUtil::sizeFile: stat failed");
    return -1;
  }

  if (!S_ISREG(buf.st_mode))
    return -1;
  return buf.st_size;
}
/*
 * @param fileName of the file to size
 * @return size in byte, -1 on error
 */
MVREXPORT long MvrUtil::sizeFile(const char * fileName)
{
  struct stat buf;

  if (stat(fileName, &buf) < 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrUtil::sizeFile: stat failed");
    return -1;
  }

  if (!S_ISREG(buf.st_mode))
    return -1;
  return buf.st_size;
}
#endif // WIN32

/*
 * @param fileName name of the file to size
 * @return  true if file is found
 */
MVREXPORT bool MvrUtil::findFile(const char * fileName)
{
  FILE *fp;
  if ((fp=MvrUtil::fopen(fileName, "r")))
  {
    fclose(fp);
    return true;
  }
  else
    return false;
}

MVREXPORT bool MvrUtil::stripQuotes(char *dest, const char *src, size_t destLen)
{
  size_t srcLen = strlen(src);
  if (destLen < srcLen+1)
  {
    MvrLog::log(MvrLog::Normal, "MvrUtil::stripQuotes: destLen isn't long enough to fit copy its should be %d", destLen, srcLen + 1);
    return false;
  }
  // if there are no quotes to strip jsut copy and return 
  if (srcLen < 2 || (src[0] != '"' || src[srcLen-1] != '"'))
  {
    strcpy(dest, src);
    return true;
  }
  // we have quotes so chop of the first and last char
  strncpy(dest, &src[1], srcLen-1);
  dest[srcLen-2] = '\0';
  return true;
}

/**
 * This method behaves similarly to the char[] version, except that it modifies
 * the given std::string. 
 * @param strToStrip a pointer to the std::string to be read/modified; must be 
 * non-NULL
 * @return bool true if the string was successfully processed; false otherwise 
**/  
MVREXPORT bool MvrUtil::stripQuotes(std::string *strToStrip)
{
  if (strToStrip == NULL){
    MvrLog::log(MvrLog::Normal, "MvrUtil::stripQuotes() NULL string");
    return false;
  }
  // if there are no matching quotes to strip, just return
  if ((strToStrip->size() < 2 )|| 
      (strToStrip->at(0) != '"') ||
      (strToStrip->at(strToStrip->size()-1) != '"'))
  {
    return true;
  }

  // Matching quotes found so chop of the first and last char
  strToStrip->erase(strToStrip->begin());
  strToStrip->erase(strToStrip->size() - 1);

  return true; 
}

/*
 * This method strips out bad characters
 */

MVREXPORT bool MvrUtil::fixBadCharacters(std::string *strToFix, bool removeSpaces, bool fixOtherWhiteSpace)
{
  if (strToFix == NULL){
    MvrLog::log(MvrLog::Normal, "MvrUtil::fixBadCharacters() NULL string");
    return false;
  }

  for (size_t i=0; i<(*strToFix).length(); i++)
  {
    if (!removeSpaces && (*strToFix)[i] == ' ')
      continue;
    else if (!removeSpaces && fixOtherWhiteSpace && isspace((*strToFix)[i]))
      (*strToFix)[i] = ' ';
    else if ((*strToFix)[i] == '(' || (*strToFix)[i] == '{' )
      (*strToFix)[i] = '[';
    else if ((*strToFix)[i] == ')' || (*strToFix)[i] == '}' )
      (*strToFix)[i] = ']';
    else if (isalpha((*strToFix)[i]) || isdigit((*strToFix)[i]) ||
             (*strToFix)[i] == '.' || (*strToFix)[i] == '_' ||
             (*strToFix)[i] == '-' || (*strToFix)[i] == '+' ||
             (*strToFix)[i] == '[' || (*strToFix)[i] == ']' )
      continue;
    else
      (*strToFix)[i] = '-';
  }
  return true;
}

/** Append a directory separator character to the given path string, depending on the
 * platform. 
 * @param path the path string to append a slash to
 * @param pathLength maximum length allocated for path string
 */
MVREXPORT void MvrUtil::appendSlash(char *path, size_t pathLength)
{
  // first check boundary
  size_t len;
  len = strlen(path);
  if (len > pathLength-2)
    return;

  if (len == 0 || (path[len-1] != '\\' && path[len-1] != '/'))
  {
#ifdef WIN32
    path[len] = '\\';
#else
    path[len] = '/'
#endif
    path[len+1] = '\0';
  }
}

/// Append the appropriate directory separator for this platform
MVREXPORT void MvrUtil::appendSlash(std::string &path)
{
  size_t len = path.length();
  if ((len == 0) || (path[len-1] != SEPARATOR_CHAR && path[len-1] != OTHER_SEPARATOR_CHAR)){
    path += SEPARATOR_STRING;
  }
}

/**
   Replace in @a path all incorrect directory separators for this platform with
   the correct directory separator character.
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
  for (size_t i=0; path[i] != '\0' && i<pathLength; i++){
    if (path[i] == '/')
      path[i] = '\\';
  }
}

MVREXPORT void MvrUtil::fixSlashesForward(char *path, size_t pathLength)
{
  for (size_t i=0; path[i] != '\0' && i<pathLength; i++){
    if (path[i] == '\\')
      path[i] = '/';
  }
}

/**
   Replace in @a path all incorrect directory separators for this platform with
   the correct directory separator character 
   @param path the path in which to fix the orientation of the slashes
*/
MVREXPORT void MvrUtil::fixSlashes(std::string &path)
{
  for (size_t i=0; i<path.length(); i++)
  {
    if (path[i] == OTHER_SEPARATOR_CHAR)
      path[i] = SEPARATOR_CHAR;
  }
}

MVREXPORT char MvrUtil::getSlash(void)
{
  return SEPARATOR_CHAR;
}

/*
   This function will take the @a baseDir and add @a insideDir after
   it, separated by appropriate directory path separators for this platform,
   with a final directory separator retained or added after @a inside dir.
   
   @param dest the place to put the result
   @param destLength the length available in @a dest
   @param baseDir the directory to start with
   @param insideDir the directory to place after the baseDir 
 */
MVREXPORT void MvrUtil::addDirectories(char *dest, size_t destLength,
                                       const char *baseDir,
                                       const char *insideDir)
{
  // start it off
  strncpy(dest, baseDir, destLength-1);
  // make sure we have null term
  dest[destLength-1] = '\0';
  appendSlash(dest, destLength);

  strncat(dest, insideDir, destLength-strlen(dest) -1);

  appendSlash(dest, destLength);
  fixSlashes(dest, destLength);
}                                       

/** 
    This compares two strings, it returns an integer less than, equal to, 
    or greater than zero  if @a str  is  found, respectively, to be less than, to
    match, or be greater than @a str2. 

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

MVREXPORT int MvrUtil::strcmp(const char *str, const std::string &str2)
{
  if (str != NULL) {
    return ::strcmp(str, str2.c_str());
  }
  else {
    return -1;
  }
}

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

MVREXPORT int MvrUtil::strcasecmp(const std::string &str, const std::string &str2)
{
  return ::strcasecmp(str.c_str(), str2.c_str());
}

MVREXPORT int MvrUtil::strcasecmp(const std::string &str, const char *str2)
{
  if (str2 != NULL) {
    return ::strcasecmp(str.c_str(), str2);
  }
  else {
    return 1;
  }
}

MVREXPORT int MvrUtil::strcasecmp(const char *str, const std::string &str2)
{
  if (str != NULL) {
    return ::strcasecmp(str, str2.c_str());
  }
  else {
    return -1;
  }
}

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

MVREXPORT int MvrUtil::strcasequotecmp(const std::string &inStr1, const std::string &inStr2)
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

#if defined(__GNUC__) && (__GNUC__ <= 2) && (__GNUC_MINOR__ <= 96)
#warning Using GCC 2.96 or less so must use nonstandard std::string::compare method.
  int cmp = str1.compare(str2.substr(pos2, len2 - 2 * pos2), pos1, len1 - 2 * pos1);
#else
  int cmp = str1.compare(pos1, len1 - 2 * pos1, str2, pos2, len2 - 2 * pos2);
#endif
  return cmp;
} // end method strcasequotecmp

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
  for (i=0; i<len && i<maxLen; i++)
  {
    dest[i] = tolower(src[i]);
  }
  dest[i] = '\0';
}

MVREXPORT bool MvrUtil::isOnlyAlphaNumeric(const char *str)
{
  unsigned int ui;
  unsigned int len;
  if (str == NULL)
    return true;
  for (ui=0, len=strlen(str); ui<len; ui++)
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
  for (unsigned i=0, len=strlen(str); i<len; i++)
  {
    if (!isdigit(str[i]) && str[i] != '\0' && str[i] !='+' && str[i] != '-')
      return false;
  }
  return true;
}

MVREXPORT bool MvrUtil::isStrEmpty(const char *str)
{
  if (str == NULL){
    return true;
  }
  if (str[0] == '\0')
    return true;
  return false;
}

MVREXPORT bool MvrUtil::isStrInList(const char *str,
                                    const std::list<std::string> &list,
                                    bool isIgnoreCase)
{
  if (str == NULL)
    return false;
  for (std::list<std::string>::const_iterator aIter = list.begin();
       aIter != list.end();
       aIter++)
  {
    if (!isIgnoreCase){
      if (strcmp((*aIter).c_str(), str) == 0){
        return true;
      }
    }
    else{
      if (strcasecmp((*aIter).c_str(), str) == 0){
        return true;
      }
    }
  }
  return false;
}                                    

MVREXPORt const char *MvrUtil::convertBool(int val)
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

MVREXPORT void MvrUtil::functorPrintf(MvrFunctor1<const char *> *functor, const char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);
  vsnprintf(buf, sizeof(buf)-1, str, ptr);
  buf[sizeof(buf)-1] = '\0';
  functor->invoke(buf);
  va_end(ptr);
}

// preserving this old version that takes char* as format str instead of const char*
// to maximize compatibility
MVREXPORT void MvrUtil::functorPrintf(MvrFunctor1<const char *> *functor, char *str, ...)
{
  char buf[10000];
  va_list ptr;
  va_start(ptr, str);

  vsnprintf(buf, sizeof(buf)-1, (const char*)str, ptr);
  buf[sizeof(buf)-1] = '\0';
  functor->invoke(buf);
  va_end(ptr);
}

MVREXPORT void MvrUtil::writeToFIle(const char *str, FILE *file)
{
  fputs(str, file);
}

/* 
   This function reads a string from a file.
   The file can contain spaces or tabs.
   @param fileName name of the file in which to look
   @param str the string to copy the file contents into
   @param strLen the maximum allocated length of str
 */
MVREXPORT bool MvrUtil::getStringFromFIle(const char *fileName, char *str, size_t strLen)
{
  FILE *strFile;
  unsigned int i;

  str[0] = '\0';

  if ((strFile = MvrUtil::fopen(fileName, "r")) != NULL)
  {
    fgets(str, strLen, strFile);
    for (i=0; i<strLen; i++)
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
#else  // WIN32
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
	       "MvrLog::getStringFromRegistry: Bad root key given.");
    return false;
  }


  if ((err = RegOpenKeyEx(rootKey, key, 0, KEY_READ, &hkey)) == ERROR_SUCCESS)
  {
    //printf("Got a key\n");
    if (RegQueryInfoKey(hkey, NULL, NULL, NULL, &numKeys, &longestKey, NULL, 
			&numValues, &longestValue, &longestDataLength, NULL, NULL) == ERROR_SUCCESS)
    {
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
	    }
      delete data;
      delete valueName;
    }
  }
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
      // start a million seconds into the future so we have some
      // room to go backwards
      mySec  = timeNow.tv_sec + 1000000;
      myMSec = timeNow.tv_nsec / 1000000;
      return;
    }
    else
    {
      ourMonotonicClock = false;
      MvrLog::logNoLock(MvrLog::Terse, "MvrTime::setNow: invalid return from clock_gettime");
    }
  }
#endif
// if our good way didn't work use the old way
#ifndef WIN32
  struct timeval timeNow;
  
  if (gettimeofday(&timeNow, NULL) == 0)
  {
    // start a million seconds into the future so we have some
    // room to go backwards
    mySec  = timeNow.tv_sec + 1000000;
    myMSec = timeNow.tv_usec / 1000;
  }
  else
    MvrLog::logNoLock(MvrLog::Terse, "MvrTime::setToNow: invalid return from gettimeofday.");
#else
  long timeNow;
  timeNow = timeGetTime();
  // start a million seconds into the future so we have some
  // room to go backwards
  mySec  = timeNow / 1000 + 1000000;
  myMSec = timeNow % 1000;
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
    myTotal += vale;
  myNum++;
  myVals.push_front(val);
  if (myVals.size() > myNumToAverage || myNum > myNumToAverage)
  {
    if (myUseRootMeanSquare)
      myTotal -= (myVals.back() * myVals.back());
    else
      myTotal -= myVals.back();
    myNum --;
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
    for (it=myVal)
  }
}
