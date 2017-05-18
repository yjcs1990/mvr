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
// #include "MvrInternal.h
#include "mvriaOSDef.h"

class MvrLaser;
class MvrBatteryMTX;
class MvrLCDMTX;
class MvrSonarMTX:
class MvrDevieConnection

#ifndef M_PI
#define M_PI 3.1415927
#endif  // M_PI

/// Contains various utility functions, including cross-platform wrappers around common system functions.
/** @ingroup UtilityClasses
    @ingroup ImportantClasses
*/
class MvrUtil
{
public:
  /// Values for the bits from 0 to 16
  enum BITS { 
    BIT0 = 0x1,       ///< value of BIT0
    BIT1 = 0x2,       ///< value of BIT1
    BIT2 = 0x4,       ///< value of BIT2
    BIT3 = 0x8,       ///< value of BIT3
    BIT4 = 0x10,      ///< value of BIT4
    BIT5 = 0x20,      ///< value of BIT5
    BIT6 = 0x40,      ///< value of BIT6
    BIT7 = 0x80,      ///< value of BIT7
    BIT8 = 0x100,     ///< value of BIT8
    BIT9 = 0x200,     ///< value of BIT9
    BIT10 = 0x400,    ///< value of BIT10
    BIT11 = 0x800,    ///< value of BIT11
    BIT12 = 0x1000,   ///< value of BIT12
    BIT13 = 0x2000,   ///< value of BIT13
    BIT14 = 0x4000,   ///< value of BIT14
    BIT15 = 0x8000,   ///< value of BIT15
  };
#ifdef WIN32
  typedef int mode_t
#endif  // WIN32

  /// Sleep for the given number of milliseconds
  /// @ingroup easy
  MVREXPORT static void sleep(unsigned int ms);

  /// Get the time in milliseconds
  MVREXPORT static unsigned int getTime(void);

  /// Delete all members of a set. Does NOT empty the set.
  /** 
      Assumes that T is an iterator that supports the operator*, operator!=
      and operator++. The return is assumed to be a pointer to a class that
      needs to be deleted.
  */
  template<class T>
  static void deleteSet(T begin, T end)
  {
    for (; begin != end; ++begin)
    {
	    delete (*begin);
    }
  }

  /// Delete all members of a set. Does NOT empty the set.
  /**
     Assumes that T is an iterator that supports the operator**, operator!=
     and operator++. The return is assumed to be a pair. The second value of
     the pair is assumed to be a pointer to a class that needs to be deleted.
  */
  template<class T>
  static void deleteSetPairs(T begin, T end)
  {
    for (; begin != end; ++begin)
    {
	    delete (*begin).second;
    }
  }
protected:
};
#endif  // MVRIAUTIL_H