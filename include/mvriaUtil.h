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
  /// Returns the minimum of the two values
  /// @ingroup easy
  static int findMin(int first, int second) 
  { if (first < second) return first; else return second; }
  /// Returns the maximum of the two values
  /// @ingroup easy
  static int findMax(int first, int second) 
  { if (first > second) return first; else return second; }

  /// Returns the minimum of the two values
  /// @ingroup easy
  static unsigned int findMinU(unsigned int first, unsigned int second) 
  { if (first < second) return first; else return second; }
  /// Returns the maximum of the two values
  /// @ingroup easy
  static unsigned int findMaxU(unsigned int first, unsigned int second) 
  { if (first > second) return first; else return second; }

  /// Returns the minimum of the two values
  /// @ingroup easy
  static double findMin(double first, double second) 
  { if (first < second) return first; else return second; }
  /// Returns the maximum of the two values
  /// @ingroup easy
  static double findMax(double first, double second) 
  { if (first > second) return first; else return second; }

  /// OS-independent way of finding the size of a file.
  MVREXPORT static long sizeFile(const char *fileName);

  /// OS-independent way of finding the size of a file.
  MVREXPORT static long sizeFile(std::string fileName);

  /// OS-independent way of checking to see if a file exists and is readable.
  MVREXPORT static bool findFile(const char *fileName);

  /// Appends a slash to a path if there is not one there already
  MVREXPORT static void appendSlash(char *path, size_t pathLength);
  
  /// Appends a slash to the given string path if necessary.
  MVREXPORT static void appendSlash(std::string &path);

  /// Fix the slash orientation in file path string for windows or linux
  MVREXPORT static void fixSlashes(char *path, size_t pathLength);
  
  /// Fixes the slash orientation in the given file path string for the current platform
  MVREXPORT static void fixSlashes(std::string &path); 

  /// Fix the slash orientation in file path string to be all forward
  MVREXPORT static void fixSlashesForward(char *path, size_t pathLength);

  /// Fix the slash orientation in file path string to be all backward
  MVREXPORT static void fixSlashesBackward(char *path, size_t pathLength);

  /// Returns the slash (i.e. separator) character for the current platform
  MVREXPORT static char getSlash();

  /// Adds two directories, taking care of all slash issues
  MVREXPORT static void addDirectories(char *dest, size_t destLength,
				    const char *baseDir, const char *insideDir);

  /// Finds out if two strings are equal
  MVREXPORT static int strcmp(const std::string &str, const std::string &str2);

  /// Finds out if two strings are equal
  MVREXPORT static int strcmp(const std::string &str, const char *str2);

  /// Finds out if two strings are equal
  MVREXPORT static int strcmp(const char *str, const std::string &str2);

  /// Finds out if two strings are equal
  MVREXPORT static int strcmp(const char *str, const char *str2);

  /// Finds out if two strings are equal (ignoring case)
  MVREXPORT static int strcasecmp(const std::string &str, const std::string &str2);

  /// Finds out if two strings are equal (ignoring case)
  MVREXPORT static int strcasecmp(const std::string &str, const char *str2);

  /// Finds out if two strings are equal (ignoring case)
  MVREXPORT static int strcasecmp(const char *str, const std::string &str2);

  /// Finds out if two strings are equal (ignoring case)
  MVREXPORT static int strcasecmp(const char *str, const char *str2);

  /// Finds out if a string has a suffix 
  MVREXPORT static bool strSuffixCmp(const char *str, const char *suffix);

  /// Finds out if a string has a suffix 
  MVREXPORT static bool strSuffixCaseCmp(const char *str, const char *suffix);

    /// Compares two strings (ignoring case and surrounding quotes)
  /**
   * This helper method is primarily used to ignore surrounding quotes 
   * when comparing MvrAgumentBuilder args.
   * @return int set to 0 if the two strings are equivalent, a negative 
   * number if str1 is "less than" str2, and a postive number if it is
   * "greater than".
  **/
  MVREXPORT static int strcasequotecmp(const std::string &str1, const std::string &str2);

  /// Puts a \ before spaces in src, puts it into dest
  MVREXPORT static void escapeSpaces(char *dest, const char *src, size_t maxLen);

  /// Strips out the quotes in the src buffer into the dest buffer
  MVREXPORT static bool stripQuotes(char *dest, const char *src,size_t destLen);
  
  /// Strips the quotes from the given string.
  MVREXPORT static bool stripQuotes(std::string *strToStrip);

  /// Fixes the bad characters in the given string.
  MVREXPORT static bool fixBadCharacters(std::string *strToFix, 
					  bool removeSpaces, bool fixOtherWhiteSpace = true);

  /// Lowers a string from src into dest, make sure there's enough space
  MVREXPORT static void lower(char *dest, const char *src, 
			      size_t maxLen);
  /// Returns true if this string is only alphanumeric (i.e. it contains only leters and numbers), false if it contains any non alphanumeric characters (punctuation, whitespace, control characters, etc.)
  MVREXPORT static bool isOnlyAlphaNumeric(const char *str);

  /// Returns true if this string is only numeric (i.e. it contains only numeric
  //digits), or it's null, or false if it contains any non nonnumeric characters (alphabetic, punctuation, whitespace, control characters, etc.)
  MVREXPORT static bool isOnlyNumeric(const char *str);

  /// Returns true if the given string is null or of zero length, false otherwise
  MVREXPORT static bool isStrEmpty(const char *str);

  /// Determines whether the given text is contained in the given list of strings.
  MVREXPORT static bool isStrInList(const char *str, const std::list<std::string> &list,
                   bool isIgnoreCase = false);

  /// Returns the floating point number from the string representation of that number in @a nptr, or HUGE_VAL for "inf" or -HUGE_VAL for "-inf".
  MVREXPORT static double atof(const char *nptr);

  /// Converts an integer value into a string for true or false
  MVREXPORT static const char *convertBool(int val);
  
#ifndef SWIG
  /** Invoke a functor with a string generated via sprintf format conversion
      @param functor The functor to invoke with the formatted string
      @param formatstr The format string into which additional argument values are inserted using vsnprintf() 
      @param ... Additional arguments are values to interpolate into @a formatstr to generate the final string passed as the argument in the functor invocation.
      @swigomit
  */
  MVREXPORT static void functorPrintf(MvrFunctor1<const char *> *functor,
				    const char *formatstr, ...);
  /// @deprecated format string should be a const char*
  MVREXPORT static void functorPrintf(MvrFunctor1<const char *> *functor,
				    char *formatstr, ...);
#endif

  /// Function for doing a fprintf to a file (here to make a functor for)
  MVREXPORT static void writeToFile(const char *str, FILE *file);

  /// Gets a string contained in an arbitrary file
  MVREXPORT static bool getStringFromFile(const char *fileName, 
					  char *str, size_t strLen);
  /** 
  These are for passing into getStringFromRegistry
  **/
  enum REGKEY {
    REGKEY_CLASSES_ROOT,     ///< use HKEY_CLASSES_ROOT
    REGKEY_CURRENT_CONFIG,   ///< use HKEY_CURRENT_CONFIG
    REGKEY_CURRENT_USER,     ///< use HKEY_CURRENT_USER
    REGKEY_LOCAL_MACHINE,    ///< use HKEY_LOCAL_MACHINE
    REGKEY_USERS             ///< use HKEY_USERS
  };

  /// Returns a string from the Windows registry
  MVREXPORT static bool getStringFromRegistry(REGKEY root, const char *key,
            const char *value, char *str, int len);

  /// Returns a string from the Windows registry, searching each of the following registry root paths in order: REGKEY_CURRENT_USER, REGKEY_LOCAL_MACHINE
  static bool findFirstStringInRegistry(const char* key, const char* value, char* str, int len)
  {
	  if(!getStringFromRegistry(REGKEY_CURRENT_USER, key, value, str, len))
	 	  return getStringFromRegistry(REGKEY_LOCAL_MACHINE, key, value, str, len);
    return true;
  }

  MVREXPORT static const char *COM1;  ///< First serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM2;  ///< Second serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM3;  ///< Third serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM4;  ///< Fourth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM5;  ///< Fifth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM6;  ///< Sixth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM7;  ///< Seventh serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM8;  ///< Eighth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM9;  ///< Ninth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM10; ///< Tenth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM11; ///< Eleventh serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM12; ///< Twelth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM13; ///< Thirteenth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM14; ///< Fourteenth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM15; ///< Fifteenth serial port device name (value depends on compilation platform)
  MVREXPORT static const char *COM16; ///< Sixteenth serial port device name (value depends on compilation platform)

  MVREXPORT static const char *TRUESTRING;  ///< "true"
  MVREXPORT static const char *FALSESTRING; ///< "false"

  /** Put the current year (GMT) in s (e.g. "2005"). 
   *  @param s String buffer (allocated) to write year into
   *  @param len Size of @a s
   */
  MVREXPORT static void putCurrentYearInString(char* s, size_t len);
  /** Put the current month (GMT) in s (e.g. "09" if September). 
   *  @param s String buffer (allocated) to write month into
   *  @param len Size of @a s
   */
  MVREXPORT static void putCurrentMonthInString(char* s, size_t len);
  /** Put the current day (GMT) of the month in s (e.g. "20"). 
   *  @param s String buffer (allocated) to write day into
   *  @param len Size of @a s
   */
  MVREXPORT static void putCurrentDayInString(char* s, size_t len);
  /** Put the current hour (GMT) in s (e.g. "13" for 1 o'clock PM). 
   *  @param s String buffer (allocated) to write hour into
   *  @param len Size of @a s
   */
  MVREXPORT static void putCurrentHourInString(char* s, size_t len);
  /** Put the current minute (GMT) in s (e.g. "05"). 
   *  @param s String buffer (allocated) to write minutes into
   *  @param len Size of @a s
   */
  MVREXPORT static void putCurrentMinuteInString(char* s, size_t len);
  /** Put the current second (GMT) in s (e.g. "59"). 
   *  @param s String buffer (allocated) to write seconds into
   *  @param len Size of @a s
   */
  MVREXPORT static void putCurrentSecondInString(char* s, size_t len);

  /// Parses the given time string (h:mm) and returns the corresponding time.
  /**
   * @param str the char * string to be parsed; in the 24-hour format h:mm
   * @param ok an output bool * set to true if the time is successfully parsed;
   * false, otherwise
   * @param toToday true to find the time on the current day, false to find the time on 1/1/70
   * @return time_t if toToday is true then its the parsed time on the current day, if toToday is false then its the parsed time on 1/1/70
   * 1/1/70
  **/
  MVREXPORT static time_t parseTime(const char *str, bool *ok = NULL, bool toToday = true);


  /** Interface to native platform localtime() function.
   *  On Linux, this is equivalent to a call to localtime_r(@a timep, @a result) (which is threadsafe, including the returned pointer, since it uses a different time struct for each thread)
   *  On Windows, this is equivalent to a call to localtime(@a timep, @a result). In addition, a static mutex is used to make it threadsafe.
   *
   *  @param timep Pointer to current time (Unix time_t; seconds since epoch) 
   *  @param result The result of calling platform localtime function is copied into this struct, so it must have been allocated.
   *  @return false on error (e.g. invalid input), otherwise true.
   */
  MVREXPORT static bool localtime(const time_t *timep, struct tm *result);

   
  /** Call MvrUtil::localtime(const time_t*, struct tm *) with the current time obtained by calling
   * time(NULL).
   *  @return false on error (e.g. invalid input), otherwise true.
   */
  MVREXPORT static bool localtime(struct tm *result);

// these aren't needed in windows since it ignores case anyhow
#ifndef WIN32
  /// this matches the case out of what file we want
  MVREXPORT static bool matchCase(const char *baseDir, const char *fileName, 
			      char * result, size_t resultLen);
#endif 
  /// Pulls the directory out of a file name
  MVREXPORT static bool getDirectory(const char *fileName, 
				    char * result, size_t resultLen);
  /// Pulls the filename out of the file name
  MVREXPORT static bool getFileName(const char *fileName, 
				    char * result, size_t resultLen);
  
  /// Sets the timestamp on the specified file
  MVREXPORT static bool changeFileTimestamp(const char *fileName, time_t timestamp);

  /// Opens a file, defaulting it so that the file will close on exec
  MVREXPORT static FILE *fopen(const char *path, const char *mode, 
			      bool closeOnExec = true);
  /// Opens a file, defaulting it so that the file will close on exec
  MVREXPORT static int open(const char *pathname, int flags, 
			      bool closeOnExec = true);
  /// Opens a file, defaulting it so that the file will close on exec
  MVREXPORT static int open(const char *pathname, int flags, mode_t mode, 
			      bool closeOnExec = true);
  MVREXPORT static int close(int fd);
  /// Opens a file, defaulting it so that the file will close on exec
  MVREXPORT static int creat(const char *pathname, mode_t mode,
			      bool closeOnExec = true);
  /// Opens a pipe, defaulting it so that the file will close on exec
  MVREXPORT static FILE *popen(const char *command, const char *type, 
			      bool closeOnExec = true);
  /// Sets if the file descriptor will be closed on exec or not
  MVREXPORT static void setFileCloseOnExec(int fd, bool closeOnExec = true);
  /// Sets if the file descriptor will be closed on exec or not
  MVREXPORT static void setFileCloseOnExec(FILE *file, bool closeOnExec = true);

  /** Return true if the value of @a f is not NaN and is not infinite (+/- INF) */
  MVREXPORT static bool floatIsNormal(double f);

  /** Convert seconds to milliseconds */
  static double secToMSec(const double sec) { return sec * 1000.0; }
  
  /** Convert milliseconds to seconds */
  static double mSecToSec(const double msec) { return msec / 1000.0; }

  /** Convert meters to US feet */
  static double metersToFeet(const double m) { return m * 3.2808399; }

  /** Convert US feet  to meters */
  static double feetToMeters(const double f) { return f / 3.2808399; }

  MVREXPORT static int atoi(const char *str, bool *ok = NULL, 
			      bool forceHex = false);
protected:
//#ifndef WIN32
  /// this splits up a file name (it isn't exported since it'd crash with dlls)
  static std::list<std::string> splitFileName(const char *fileName);
//#endif

private:
  /// The character used as a file separator on the current platform (i.e. Linux or Windows)
  static const char SEPARATOR_CHAR;
  /// The character used as a file separator on the current platform, in a string format
  static const char *SEPARATOR_STRING;
  /// The character used as a file separator on the other platforms (i.e. slash in opposite direction)
  static const char OTHER_SEPARATOR_CHAR;

#ifdef WIN32
  // Used on Windows to make MvrUtil::localtime() function threadsafe
  static MvrMutex ourLocaltimeMutex;
#endif
};

/** Common math operations
    @ingroup UtilityClasses
    @ingroup easy
*/
class MvrMath
{
private:
  /* see MvrMath::epsilon() */
  static const double ourEpsilon; 

  // see getRandMax())
  static const long ourRandMax;

public: 
  /** @return a very small number which can be used for comparisons of floating 
   * point values, etc. 
   */
  MVREXPORT static double epsilon();

  /// This adds two angles together and fixes the result to [-180, 180] 
  /**
     @param ang1 first angle
     @param ang2 second angle, added to first
     @return sum of the angles, in range [-180,180]
     @see subAngle
     @see fixAngle 
     @ingroup easy
  */
  static double addAngle(double ang1, double ang2) 
  { return fixAngle(ang1 + ang2); }

  /// This subtracts one angle from another and fixes the result to [-180,180]
  /**
     @param ang1 first angle
     @param ang2 second angle, subtracted from first angle
     @return resulting angle, in range [-180,180]
     @see addAngle
     @see fixAngle
     @ingroup easy
  */
  static double subAngle(double ang1, double ang2) 
  { return fixAngle(ang1 - ang2); }

  /// Takes an angle and returns the angle in range (-180,180]
  /**
     @param angle the angle to fix
     @return the angle in range (-180,180]
     @see addAngle
     @see subAngle
     @ingroup easy
  */
  static double fixAngle(double angle) 
  {
    if (angle >= 360)
      angle = angle - 360.0 * (double)((int)angle / 360);
    if (angle < -360)
      angle = angle + 360.0 * (double)((int)angle / -360);
    if (angle <= -180)
      angle = + 180.0 + (angle + 180.0);
    if (angle > 180)
      angle = - 180.0 + (angle - 180.0);
    return angle;
  } 

  /// Converts an angle in degrees to an angle in radians
  /**
     @param deg the angle in degrees
     @return the angle in radians
     @see radToDeg
     @ingroup easy
  */     
  static double degToRad(double deg) { return deg * M_PI / 180.0; }

  /// Converts an angle in radians to an angle in degrees
  /**
     @param rad the angle in radians
     @return the angle in degrees
     @see degToRad
     @ingroup easy
  */
  static double radToDeg(double rad) { return rad * 180.0 / M_PI; }

  /// Finds the cos, from angles in degrees
  /**
     @param angle angle to find the cos of, in degrees
     @return the cos of the angle
     @see sin
     @ingroup easy
  */
  static double cos(double angle) { return ::cos(MvrMath::degToRad(angle)); }

  /// Finds the sin, from angles in degrees
  /**
     @param angle angle to find the sin of, in degrees
     @return the sin of the angle
     @see cos
     @ingroup easy
  */
  static double sin(double angle) { return ::sin(MvrMath::degToRad(angle)); }

  /// Finds the tan, from angles in degrees
  /**
     @param angle angle to find the tan of, in degrees
     @return the tan of the angle
     @ingroup easy
  */
  static double tan(double angle) { return ::tan(MvrMath::degToRad(angle)); }

  /// Finds the arctan of the given y/x pair
  /**
     @param y the y distance
     @param x the x distance
     @return the angle y and x form
     @ingroup easy
  */
  static double atan2(double y, double x) 
  { return MvrMath::radToDeg(::atan2(y, x)); }


  /// Finds if one angle is between two other angles
  /// @ingroup easy
  static bool angleBetween(double angle, double startAngle, double endAngle)
  {
    angle = fixAngle(angle);
    startAngle = fixAngle(startAngle);
    endAngle = fixAngle(endAngle);
    if ((startAngle < endAngle && angle > startAngle && angle < endAngle) ||
	  (startAngle > endAngle && (angle > startAngle || angle < endAngle)))
	    return true;
    else
	    return false;
  }

  /// Finds the absolute value of a double
  /**
     @param val the number to find the absolute value of
     @return the absolute value of the number
  */
  static double fabs(double val) 
  {
    if (val < 0.0)
	    return -val;
    else
	    return val;
  }

  /// Finds the closest integer to double given
  /**
     @param val the double to find the nearest integer to
     @return the integer the value is nearest to (also caps it within 
     int bounds)
  */
  static int roundInt(double val) 
  { 
    val += .49;
    if (val > INT_MAX)
	    return (int) INT_MAX;
    else if (val < INT_MIN)
	    return (int) INT_MIN;
    else
	    return((int) floor(val)); 
  }
    
  /// Finds the closest short to double given
  /**
     @param val the double to find the nearest short to
     @return the integer the value is nearest to (also caps it within 
     short bounds)
  */
  static short roundShort(double val) 
  { 
    val += .49;
    if (val > 32767)
	    return (short) 32767;
    else if (val < -32768)
	    return (short) -32768;
    else
	    return((short) floor(val)); 
  }
    

  /// Rotates a point around 0 by degrees given
  static void pointRotate(double *x, double *y, double th)
  {
    double cs, sn, xt, yt;
    cs = cos(th);
    sn = sin(th);
    xt = *x;  
    yt = *y;
    *x = cs*xt + sn*yt;
    *y = cs*yt - sn*xt;
  }
  
  /** Returns a random number between 0 and RAND_MAX on Windows, 2^31 on Linux
   * (see MvrUtil::getRandMax()). On Windows, rand() is used, on Linux, lrand48(). */
  static long random(void)
  {
#ifdef WIN32
    return(rand());
#else
    return(lrand48());
#endif
  }
  
  /// Maximum of value returned by random()
  MVREXPORT static long getRandMax();

  /** Returns a random number between @a m and @a n. On Windows, rand() is used,
   * on Linux lrand48(). 
   * @ingroup easy
  */
  MVREXPORT static long randomInRange(long m, long n);

  /// Finds the distance between two coordinates
  /**
     @param x1 the first coords x position
     @param y1 the first coords y position
     @param x2 the second coords x position
     @param y2 the second coords y position
     @return the distance between (x1, y1) and (x2, y2)
   * @ingroup easy
  **/
  static double distanceBetween(double x1, double y1, double x2, double y2)
  { return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));  }

  /// Finds the squared distance between two coordinates
  /**
     use this only where speed really matters
     @param x1 the first coords x position
     @param y1 the first coords y position
     @param x2 the second coords x position
     @param y2 the second coords y position
     @return the distance between (x1, y1) and (x2, y2)
  **/
  static double squaredDistanceBetween(double x1, double y1, double x2, double y2)
  { return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);  }

  /** Base-2 logarithm */
  static double log2(double x)
  {
    return log10(x) / 0.3010303;  // 0.301... is log10(2.0).
  }

  /// Platform-independent call to determine whether the given double is not-a-number.
  static bool isNan(double d) 
  {
#ifdef WIN32
    return _isnan(d);
#else 
    return isnan(d);
#endif
  }

  static bool isNan(float f) 
  {
#ifdef WIN32
	  return _isnan(f);
#else
	  return isnan(f);
#endif
  }

  static bool isFinite(float f) 
  {
#ifdef WIN32
	  return _finite(f);
#else
	  return finitef(f);
#endif
  }

  static bool isFinite(double d) 
  {
#ifdef WIN32
	  return _finite(d);
#else
	  return finite(d);
#endif
  }

  static bool compareFloats(double f1, double f2, double epsilon)
  {
    return (fabs(f2-f1) <= epsilon);
  }

  static bool compareFloats(double f1, double f2)
  {
    return compareFloats(f1, f2, epsilon());
  }
};  // end class MvrMath

#endif  // MVRIAUTIL_H