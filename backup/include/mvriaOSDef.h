/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : mvriaOSDef.h
 > Description  : Some definitions about the OS
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/

#ifndef MVRIAOSDEF_H
#define MVRIAOSDEF_H

#if defined(_WIN32) && !defined(WIN32)
#define WIN32 _WIN32
#endif  // _WIN32

#if defined(WIN32) && !defined(MINGW)

////
//// Windows - Massage the windows compiler into working
////

// Turn off warning of usage of 'this' in
// constructor chaining
#pragma warning(disable:4355)

// Turn off warning about truncated identifiers which happens
// in debug builds of code using STL templatized stuff.
#pragma warning(disable:4786)

// Turn off warning about 'benign macro redef'.
#pragma warning(disable:4142)

// Turn off warning about loosing from the conversion to double.
#pragma warning(disable:4244)

// Turn off warning about forcing value to bool 'true' or 'false'.
#pragma warning(disable:4800)

// Turn off warning about using some standard C libraries that have been deprecated
// by MSVC. (e.g. they want you to use snprintf_s instead of snprintf, etc.)
#pragma warning(disable:4996)

// Warning about "new behavior" in VC2008 that array elements are not automatically initialized 
// (which is normal C++ behavior anyway and MVRIA doesn't assume it)
#pragma warning(disable:4351)


#include "mvriaTypedefs.h"

// Compatibility functions to help windows out.
inline int strcasecmp(const char *s1, const char *s2)  {return _stricmp(s1, s2);}
inline int strncasecmp(const char *s1, const char *s2, size_t n) {return _strnicmp(s1, s2, n);}

#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif  // snprintf


#endif  //MVRIAOSDEF_H
