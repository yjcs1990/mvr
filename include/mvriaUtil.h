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
class MvrSonarMTX;
class MvrDevieConnection;

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
   * when comparing MvrArgumentBuilder args.
   * @return int set to 0 if the two strings are equivalent, a negative 
   * number if str1 is "less than" str2, and a positive number if it is
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
  /// Returns true if this string is only alphanumeric (i.e. it contains only letters and numbers), false if it contains any non alphanumeric characters (punctuation, whitespace, control characters, etc.)
  MVREXPORT static bool isOnlyAlphaNumeric(const char *str);

  /// Returns true if this string is only numeric (i.e. it contains only numeric
  //digits), or it's null, or false if it contains any non-numeric characters (alphabetic, punctuation, whitespace, control characters, etc.)
  MVREXPORT static bool isOnlyNumeric(const char *str);

  /// Returns true if the given string is null or of zero length, false otherwise
  MVREXPORT static bool isStrEmpty(const char *str);

  /// Determines whether the given text is contained in the given list of strings.
  MVREXPORT static bool isStrInList(const char *str, const std::list<std::string> &list,
                   bool isIgnoreCase = false);

  /// Returns the floating point number from the string representation of that number in @param nptr, or HUGE_VAL for "inf" or -HUGE_VAL for "-inf".
  MVREXPORT static double atof(const char *nptr);

  /// Converts an integer value into a string for true or false
  MVREXPORT static const char *convertBool(int val);
  
#ifndef SWIG
  /** Invoke a functor with a string generated via sprintf format conversion
      @param functor The functor to invoke with the formatted string
      @param formatstr The format string into which additional argument values are inserted using vsnprintf() 
      @param ... Additional arguments are values to interpolate into @param formatstr to generate the final string passed as the argument in the functor invocation.
      @swigomit
  */
  MVREXPORT static void functorPrintf(MvrFunctor1<const char *> *functor,
				    const char *formatstr, ...);
  /// @deprecated format string should be a const char*
  MVREXPORT static void functorPrintf(MvrFunctor1<const char *> *functor,
				    char *formatstr, ...);
#endif  //SEIG

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
   *  @param len Size of @param s
   */
  MVREXPORT static void putCurrentYearInString(char* s, size_t len);
  /** Put the current month (GMT) in s (e.g. "09" if September). 
   *  @param s String buffer (allocated) to write month into
   *  @param len Size of @param s
   */
  MVREXPORT static void putCurrentMonthInString(char* s, size_t len);
  /** Put the current day (GMT) of the month in s (e.g. "20"). 
   *  @param s String buffer (allocated) to write day into
   *  @param len Size of @param s
   */
  MVREXPORT static void putCurrentDayInString(char* s, size_t len);
  /** Put the current hour (GMT) in s (e.g. "13" for 1 o'clock PM). 
   *  @param s String buffer (allocated) to write hour into
   *  @param len Size of @param s
   */
  MVREXPORT static void putCurrentHourInString(char* s, size_t len);
  /** Put the current minute (GMT) in s (e.g. "05"). 
   *  @param s String buffer (allocated) to write minutes into
   *  @param len Size of @param s
   */
  MVREXPORT static void putCurrentMinuteInString(char* s, size_t len);
  /** Put the current second (GMT) in s (e.g. "59"). 
   *  @param s String buffer (allocated) to write seconds into
   *  @param len Size of @param s
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
#endif  // WIN32
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

  /** Return true if the value of @param f is not NaN and is not infinite (+/- INF) */
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
  static std::list<std::string> splitFileName(const char *fileName);

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
#endif  // WIN32
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
#endif //WIN32
  }
  
  /// Maximum of value returned by random()
  MVREXPORT static long getRandMax();

  /** Returns a random number between @param m and @param n. On Windows, rand() is used,
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
#endif  //WIN32
  }

  static bool isNan(float f) 
  {
#ifdef WIN32
	  return _isnan(f);
#else
	  return isnan(f);
#endif  //WIN32
  }

  static bool isFinite(float f) 
  {
#ifdef WIN32
	  return _finite(f);
#else
	  return finitef(f);
#endif  //WIN32
  }

  static bool isFinite(double d) 
  {
#ifdef WIN32
	  return _finite(d);
#else
	  return finite(d);
#endif  //WIN32
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

/// Represents an x, y position with an orientation
/** 
    This class represents a robot position with heading.  The heading is 
    automatically adjusted to be in the range -180 to 180.  It also defaults
    to 0, and so does not need to be used. (This avoids having 2 types of 
    positions.)  Everything in the class is inline so it should be fast.

  @ingroup UtilityClasses
  @ingroup easy
  @sa MvrPoseWithTime
*/
class MvrPose
{
public:
  /// Constructor, with optional initial values
  /** 
      Sets the pose to the given values.  The constructor can be called with no 
      parameters, with just x and y, or with x, y, and th.  The given heading (th)
      is automatically adjusted to be in the range -180 to 180.

      @param x the double to set the x position to, default of 0
      @param y the double to set the y position to, default of 0
      @param th the double value for the pose's heading (or th), default of 0
  */
  MvrPose(double x = 0, double y = 0, double th = 0) : myX(x), myY(y), myTh(MvrMath::fixAngle(th)) {}
    
  /// Copy Constructor
  MvrPose(const MvrPose &pose) : myX(pose.myX), myY(pose.myY), myTh(pose.myTh) {}

  /// Destructor
  virtual ~MvrPose() {}
  /// Sets the position to the given values
  /** 
      Sets the position with the given three values, but the theta does not
      need to be given as it defaults to 0.
      @param x the position to set the x position to
      @param y the position to set the y position to
      @param th the position to set the th position to, default of 0
  */
  virtual void setPose(double x, double y, double th = 0) 
  { setX(x); setY(y); setTh(th); }
  /// Sets the position equal to the given position
  /** @param position the position value this instance should be set to */
  virtual void setPose(MvrPose position)
  {
    setX(position.getX());
    setY(position.getY());
    setTh(position.getTh());
  }
  /// Sets the x position
  void setX(double x) { myX = x; }
  /// Sets the y position
  void setY(double y) { myY = y; }
  /// Sets the heading
  void setTh(double th) { myTh = MvrMath::fixAngle(th); }
  /// Sets the heading, using radians
  void setThRad(double th) { myTh = MvrMath::fixAngle(MvrMath::radToDeg(th)); }
  /// Gets the x position
  double getX(void) const { return myX; }
  /// Gets the y position
  double getY(void) const { return myY; }
  /// Gets the heading
  double getTh(void) const { return myTh; }
  /// Gets the heading, in radians
  double getThRad(void) const { return MvrMath::degToRad(myTh); }
  /// Gets the whole position in one function call
  /**
     Gets the whole position at once, by giving it 2 or 3 pointers to 
     doubles.  If you give the function a null pointer for a value it won't
     try to use the null pointer, so you can pass in a NULL if you don't 
     care about that value.  Also note that th defaults to NULL so you can 
     use this with just x and y.
     @param x a pointer to a double to set the x position to
     @param y a pointer to a double to set the y position to
     @param th a pointer to a double to set the heading to, defaults to NULL
   */
  void getPose(double *x, double *y, double *th = NULL) const
  { 
    if (x != NULL) 
	    *x = myX;
    if (y != NULL) 
	    *y = myY; 
    if (th != NULL) 
	    *th = myTh; 
  }
  /// Finds the distance from this position to the given position
  /**
     @param position the position to find the distance to
     @return the distance to the position from this instance
  */
  virtual double findDistanceTo(MvrPose position) const
  {
    return MvrMath::distanceBetween(getX(), getY(), 
		     position.getX(), 
		     position.getY());
  }

  /// Finds the square distance from this position to the given position
  /**
     This is only here for speed, if you aren't doing this thousands
     of times a second don't use this one use findDistanceTo

     @param position the position to find the distance to
     @return the distance to the position from this instance 
  **/
  virtual double squaredFindDistanceTo(MvrPose position) const
  {
    return MvrMath::squaredDistanceBetween(getX(), getY(), 
			    position.getX(), 
			    position.getY());
  }
  /// Finds the angle between this position and the given position
  /** 
      @param position the position to find the angle to
      @return the angle to the given position from this instance, in degrees
  */
  virtual double findAngleTo(MvrPose position) const
  {
    return MvrMath::radToDeg(atan2(position.getY() - getY(),
		                            position.getX() - getX()));
  }
  /// Logs the coordinates using MvrLog
  virtual void log(void) const
  { MvrLog::log(MvrLog::Terse, "%.0f %.0f %.1f", myX, myY, myTh); }

  /// Add the other pose's X, Y and theta to this pose's X, Y, and theta (sum in theta will be normalized to (-180,180)), and return the result
  virtual MvrPose operator+(const MvrPose& other) const
  {
    return MvrPose( myX + other.getX(), 
                   myY + other.getY(), 
                   MvrMath::fixAngle(myTh + other.getTh()) );
  }

  /// Substract the other pose's X, Y, and theta from this pose's X, Y, and theta (difference in theta will be normalized to (-180,180)), and return the result
  virtual MvrPose operator-(const MvrPose& other) const
  {
    return MvrPose( myX - other.getX(), 
                   myY - other.getY(), 
                   MvrMath::fixAngle(myTh - other.getTh()) );
  }
  
  /// Adds the given pose to this one.
	MvrPose & operator+= ( const MvrPose & other)
  {
    myX += other.myX;
    myY += other.myY;
    myTh = MvrMath::fixAngle(myTh + other.myTh);
    return *this;
  }

	/** Subtracts the given pose from this one.
     *  @swigomit
     */
	MvrPose & operator-= ( const MvrPose & other)
  {
    myX -= other.myX;
    myY -= other.myY;
    myTh = MvrMath::fixAngle(myTh - other.myTh);
    return *this;
  }

  /// Equality operator (for sets)
  virtual bool operator==(const MvrPose& other) const
  {
    return ((fabs(myX - other.myX) < MvrMath::epsilon()) &&
            (fabs(myY - other.myY) < MvrMath::epsilon()) &&
            (fabs(myTh - other.myTh) < MvrMath::epsilon()));
  }

  virtual bool operator!=(const MvrPose& other) const
  {
    return ((fabs(myX - other.myX) > MvrMath::epsilon()) ||
            (fabs(myY - other.myY) > MvrMath::epsilon()) ||
            (fabs(myTh - other.myTh) > MvrMath::epsilon()));
  }

  /// Less than operator (for sets)
  virtual bool operator<(const MvrPose& other) const
  {
    if (fabs(myX - other.myX) > MvrMath::epsilon()) {
      return myX < other.myX;
    }
    else if (fabs(myY - other.myY) > MvrMath::epsilon()) {
      return myY < other.myY;  
    }
    else if (fabs(myTh - other.myTh) > MvrMath::epsilon()) {
      return myTh < other.myTh;
    }
    // Otherwise... x, y, and th are equal
    return false;
  } // end operator<

  /// Finds the distance between two poses (static function, uses no
  /// data from any instance and shouldn't be able to be called on an
  /// instance)
  /**
     @param pose1 the first coords
     @param pose2 the second coords
     @return the distance between the poses
  **/
  static double distanceBetween(MvrPose pose1, MvrPose pose2)
  { return MvrMath::distanceBetween(pose1.getX(), pose1.getY(), 
				     pose2.getX(), pose2.getY()); }

  /// Return true if the X value of p1 is less than the X value of p2
  static bool compareX(const MvrPose& p1, const MvrPose &p2)
  { return (p1.getX() < p2.getX()); } 
  /// Return true if the Y value of p1 is less than the X value of p2
  static bool compareY(const MvrPose& p1, const MvrPose &p2)
  { return (p1.getY() < p2.getY()); } 

  bool isInsidePolygon(const std::vector<MvrPose>& vertices) const
  {
    bool inside = false;
    const size_t n = vertices.size();
    size_t i = 0;
    size_t j = n-1;
    for(; i < n; j = i++)
    {
      const double x1 = vertices[i].getX();
      const double x2 = vertices[j].getX();
      const double y1 = vertices[i].getY();
      const double y2 = vertices[j].getY();
      if((((y1 < getY()) && (getY() < y2)) || ((y2 <= getY()) && (getY() < y1))) && (getX() <= (x2 - x1) * (getY() - y1) / (y2 - y1) + x1))
        inside = !inside;
    }
    return inside;
  }

protected:
  double myX;
  double myY;
  double myTh;
};


/// A class for time readings and measuring durations
/** 
    This class is for timing durations or time between events.
    The time values it stores are relative to an abritrary starting time; it
    does not correspond to "real world" or "wall clock" time in any way,
    so DON'T use this for keeping track of what time it is, 
    just for timestamps and relative timing (e.g. "this loop needs to sleep another 100 ms").

    The recommended methods to use are setToNow() to reset the time,
    mSecSince() to obtain the number of milliseconds elapsed since it was
    last reset (or secSince() if you don't need millisecond precision), and
    mSecSince(MvrTime) or secSince(MvrTime) to find the difference between 
    two MvrTime objects.

    On systems where it is supported this will use a monotonic clock,
    this is an ever increasing system that is not dependent on what
    the time of day is set to.  Normally for linux gettimeofday is
    used, but if the time is changed forwards or backwards then bad
    things can happen.  Windows uses a time since bootup, which
    functions the same as the monotonic clock anyways.  You can use
    MvrTime::usingMonotonicClock() to see if this is being used.  Note
    that an MvrTime will have had to have been set to for this to be a
    good value... Mvria::init does this however, so that should not be
    an issue.  It looks like the monotonic clocks won't work on linux
    kernels before 2.6.

  @ingroup UtilityClasses
*/

class MvrTime
{
public:
  /// Constructor. Time is initialized to the current time.
  MvrTime() { setToNow(); }

  /// Copy constructor
  MvrTime(const MvrTime &other) : mySec(other.mySec), myMSec(other.myMSec) {}

  /// Assignment operator 
  MvrTime &operator=(const MvrTime &other) 
  {
    if (this != &other) {
      mySec = other.mySec;
      myMSec = other.myMSec;
    }
    return *this;
  }

  //
  /// Destructor
  ~MvrTime() {}
  
  /// Gets the number of milliseconds since the given timestamp to this one
  long mSecSince(MvrTime since) const 
  {
    long long ret = mSecSinceLL(since);
    if (ret > INT_MAX)
	    return INT_MAX;
    if (ret < -INT_MAX)
	    return -INT_MAX;
    return ret;
  }
  /// Gets the number of milliseconds since the given timestamp to this one
  long long mSecSinceLL(MvrTime since) const 
  {
    long long timeSince, timeThis;

    timeSince = since.getSecLL() * 1000 + since.getMSecLL();
    timeThis = mySec * 1000 + myMSec;
    return timeSince - timeThis;
  }
  /// Gets the number of seconds since the given timestamp to this one
  long secSince(MvrTime since) const
  {
    return mSecSince(since)/1000;
  }
  /// Gets the number of seconds since the given timestamp to this one
  long long secSinceLL(MvrTime since) const
  {
    return mSecSinceLL(since)/1000;
  }
  /// Finds the number of millisecs from when this timestamp is set to to now (the inverse of mSecSince())
  long mSecTo(void) const
  {
    MvrTime now;
    now.setToNow();
    return -mSecSince(now);
  }
  /// Finds the number of millisecs from when this timestamp is set to to now (the inverse of mSecSince())
  long long mSecToLL(void) const
  {
    MvrTime now;
    now.setToNow();
    return -mSecSinceLL(now);
  }
  /// Finds the number of seconds from when this timestamp is set to to now (the inverse of secSince())
  long secTo(void) const
  {
    return mSecTo()/1000;
  }
  /// Finds the number of seconds from when this timestamp is set to to now (the inverse of secSince())
  long long secToLL(void) const
  {
    return mSecToLL()/1000;
  }
  /// Finds the number of milliseconds from this timestamp to now
  long mSecSince(void) const
  {
    MvrTime now;
    now.setToNow();
    return mSecSince(now);
  }
  /// Finds the number of milliseconds from this timestamp to now
  long long mSecSinceLL(void) const
  {
    MvrTime now;
    now.setToNow();
    return mSecSinceLL(now);
  }
  /// Finds the number of seconds from when this timestamp was set to now
  long secSince(void) const
  {
    return mSecSince()/1000;
  }
  /// Finds the number of seconds from when this timestamp was set to now
  long long secSinceLL(void) const
  {
    return mSecSinceLL()/1000;
  }
  /// returns whether the given time is before this one or not
  bool isBefore(MvrTime testTime) const
  {
    if (mSecSince(testTime) < 0)
    	return true;
    else
	    return false;
  }
  /// returns whether the given time is equal to this time or not
  bool isAt(MvrTime testTime) const
  {
    if (mSecSince(testTime) == 0)
	    return true;
    else
	    return false;
  }
  /// returns whether the given time is after this one or not
  bool isAfter(MvrTime testTime) const
  {
    if (mSecSince(testTime) > 0)
	    return true;
    else
	    return false;
  }
  /// Resets the time
  /// @ingroup easy
  MVREXPORT void setToNow(void);
  /// Add some milliseconds (can be negative) to this time
  bool addMSec(long ms)
  {
    //unsigned long timeThis;
    long long timeThis;
    timeThis = mySec * 1000 + myMSec;
    //if (ms < 0 && (unsigned)abs(ms) > timeThis)
    if (ms < 0 && -ms > timeThis)
    {
      MvrLog::log(MvrLog::Terse, "MvrTime::addMSec: tried to subtract too many milliseconds, would result in a negative time.");
      mySec = 0;
      myMSec = 0;
      return false;
    }
    else 
    {
      timeThis += ms;
      mySec = timeThis / 1000;
	    myMSec = timeThis % 1000;
    }
    return true;
  } // end method addMSec

  /// Add some milliseconds (can be negative) to this time
  bool addMSecLL(long long ms)
  {
    //unsigned long timeThis;
    long long timeThis;
    timeThis = mySec * 1000 + myMSec;
    //if (ms < 0 && (unsigned)abs(ms) > timeThis)
    if (ms < 0 && -ms > timeThis)
    {
      MvrLog::log(MvrLog::Terse, "MvrTime::addMSec: tried to subtract too many milliseconds, would result in a negative time.");
      mySec = 0;
      myMSec = 0;
      return false;
    }
    else 
    {
      timeThis += ms;
      mySec = timeThis / 1000;
	    myMSec = timeThis % 1000;
    }
    return true;
  } // end method addMSec
  
  /// Sets the seconds value (since the arbitrary starting time)
  void setSec(unsigned long sec) { mySec = sec; }
  /// Sets the milliseconds value (occuring after the seconds value)
  void setMSec(unsigned long msec) { myMSec = msec; }
  /// Gets the seconds value (since the arbitrary starting time)
  unsigned long getSec(void) const { return mySec; }
  /// Gets the milliseconds value (occuring after the seconds value)
  unsigned long getMSec(void) const { return myMSec; }

  /// Sets the seconds value (since the arbitrary starting time)
  void setSecLL(unsigned long long sec) { mySec = sec; }
  /// Sets the milliseconds value (occuring after the seconds value)
  void setMSecLL(unsigned long long msec) { myMSec = msec; }
  /// Gets the seconds value (since the arbitrary starting time)
  unsigned long long getSecLL(void) const { return mySec; }
  /// Gets the milliseconds value (occuring after the seconds value)
  unsigned long long getMSecLL(void) const { return myMSec; }

  /// Logs the time
  void log(const char *prefix = NULL) const
  { MvrLog::log(MvrLog::Terse, "%sTime: %lld.%lld", ((prefix != NULL) ? prefix : ""), mySec, myMSec); }
  /// Gets if we're using a monotonic (ever increasing) clock
  static bool usingMonotonicClock()
  {
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
    return ourMonotonicClock;
#endif  //_POSIX_TIMERS
#ifdef WIN32
    return true;
#endif  // WIN32
    return false;
  }
  
  /// Equality operator (for sets)
  bool operator==(const MvrTime& other) const
  {
    return isAt(other);
  }

  bool operator!=(const MvrTime& other) const
  {
    return (!isAt(other));
  }
 
  // Less than operator for sets
  bool operator<(const MvrTime& other) const
  {
    return isBefore(other);
  } // end operator <

  bool operator>(const MvrTime& other) const
  {
    return isAfter(other);
  }

protected:
  unsigned long long mySec;
  unsigned long long myMSec;
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
  static bool ourMonotonicClock;
#endif   //_POSIX_TIMERS
}; // end class MvrTime

/// A subclass of MvrPose that also stores a timestamp (MvrTime) 
class MvrPoseWithTime : public MvrPose
{
public:
  MvrPoseWithTime(double x = 0, double y = 0, double th = 0,
	                MvrTime thisTime = MvrTime()) : MvrPose(x, y, th)
  { myTime = thisTime; }
  /// Copy Constructor
  MvrPoseWithTime(const MvrPose &pose) : MvrPose(pose) {}
  virtual ~MvrPoseWithTime() {}
  void setTime(MvrTime newTime) { myTime = newTime; }
  void setTimeToNow(void) { myTime.setToNow(); }
  MvrTime getTime(void) const { return myTime; }
protected:
  MvrTime myTime;
};

/// A class for keeping track of if a complete revolution has been attained
/**
   This class can be used to keep track of if a complete revolution has been
   done, it is used by doing doing a clearQuadrants when you want to stat
   the revolution.  Then at each point doing an updateQuadrant with the current
   heading of the robot.  When didAllQuadrants returns true, then all the 
   quadrants have been done.
  @ingroup UtilityClasses
*/
class MvrSectors
{
public:
  /// Constructor
  MvrSectors(int numSectors = 8) 
  { 
    mySectorSize = 360/numSectors;
    mySectors = new int[numSectors]; 
    myNumSectors = numSectors; 
    clear();
  }
  /// Destructor
  virtual ~MvrSectors() { delete mySectors; }
  /// Clears all quadrants
  void clear(void) 
  {
    int i;
    for (i = 0; i < myNumSectors; i++)
	    mySectors[i] = false;
  }
  /// Updates the appropriate quadrant for the given angle
  void update(double angle)
  {
    int angleInt;
    angleInt = MvrMath::roundInt(MvrMath::fixAngle(angle) + 180);
    mySectors[angleInt / mySectorSize] = true;
  }
  /// Returns true if the all of the quadrants have been gone through
  bool didAll(void) const
  {
    int i;
    for (i = 0; i < myNumSectors; i++)
	    if (mySectors[i] == false)
	      return false;
    return true;
  }
protected:
  int *mySectors;
  int myNumSectors;
  int mySectorSize;
};

/// Represents geometry of a line in two-dimensional space.
/**
   Note this the theoretical line, i.e. it goes infinitely. 
   For a line segment with endpoints, use MvrLineSegment.
   @sa MvrLineSegment
  @ingroup UtilityClasses
**/
class MvrLine
{
public:
  ///// Empty constructor
  MvrLine() {}
  /// Constructor with parameters
  MvrLine(double a, double b, double c) { newParameters(a, b, c); }
  /// Constructor with endpoints
  MvrLine(double x1, double y1, double x2, double y2) 
  { newParametersFromEndpoints(x1, y1, x2, y2); }
  /// Destructor
  virtual ~MvrLine() {}
  /// Sets the line parameters (make it not a segment)
  void newParameters(double a, double b, double c) 
  { myA = a; myB = b; myC = c; }
  /// Sets the line parameters from endpoints, but makes it not a segment
  void newParametersFromEndpoints(double x1, double y1, double x2, double y2)
  { myA = y1 - y2; myB = x2 - x1; myC = (y2 *x1) - (x2 * y1); }
  /// Gets the A line parameter
  double getA(void) const { return myA; }
  /// Gets the B line parameter
  double getB(void) const { return myB; }
  /// Gets the C line parameter
  double getC(void) const { return myC; }
  /// finds the intersection of this line with another line
  /** 
      @param line the line to check if it intersects with this line
      @param pose if the lines intersect, the pose is set to the location
      @return true if they intersect, false if they do not 
  **/
  bool intersects(const MvrLine *line, MvrPose *pose) const
    {
      double x, y;
      double n;
      n = (line->getB() * getA()) - (line->getA() * getB());
      // if this is 0 the lines are parallel
      if (fabs(n) < .0000000000001)
      {
	return false;
      }
      // they weren't parallel so see where the intersection is
      x = ((line->getC() * getB()) - (line->getB() * getC())) / n;
      y = ((getC() * line->getA()) - (getA() * line->getC())) / n;
      pose->setPose(x, y);
      return true;
    }
  /// Makes the given line perpendicular to this one through the given pose
  void makeLinePerp(const MvrPose *pose, MvrLine *line) const
  {
    line->newParameters(getB(), -getA(), 
	  (getA() * pose->getY()) - (getB() * pose->getX()));
  }
   /// Calculate the distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of

     @return if the pose does not intersect line it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpDist(const MvrPose &pose) const
  {
    MvrPose perpPose;
    MvrLine perpLine;
    makeLinePerp(&pose, &perpLine);
    if (!intersects(&perpLine, &perpPose))
	    return -1;
    return (perpPose.findDistanceTo(pose));
  }
   /// Calculate the squared distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of

     @return if the pose does not intersect line it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpSquaredDist(const MvrPose &pose) const
  {
    MvrPose perpPose;
    MvrLine perpLine;
    makeLinePerp(&pose, &perpLine);
    if (!intersects(&perpLine, &perpPose))
    	return -1;
    return (perpPose.squaredFindDistanceTo(pose));
  }
  /// Determine the intersection point between this line segment, and a perpendicular line passing through the given pose (i.e. projects the given pose onto this line segment.)
  /**
   * If there is no intersection, false is returned.
     @param pose The X and Y components of this pose object indicate the point to project onto this line segment.
     @param perpPoint The X and Y components of this pose object are set to indicate the intersection point
     @return true if an intersection was found and perpPoint was modified, false otherwise.
     @swigomit
  **/
  bool getPerpPoint(const MvrPose &pose, MvrPose *perpPoint) const
  {
    MvrLine perpLine;
    makeLinePerp(&pose, &perpLine);
    return intersects(&perpLine, perpPoint);
  }

  /// Equality operator
  virtual bool operator==(const MvrLine &other) const
  {
    return ((fabs(myA - other.myA) <= MvrMath::epsilon()) &&
            (fabs(myB - other.myB) <= MvrMath::epsilon()) &&
            (fabs(myC - other.myC) <= MvrMath::epsilon()));
  }
  /// Inequality operator
  virtual bool operator!=(const MvrLine &other) const
  {
    return ((fabs(myA - other.myA) > MvrMath::epsilon()) ||
            (fabs(myB - other.myB) > MvrMath::epsilon()) ||
            (fabs(myC - other.myC) > MvrMath::epsilon()));
  }
protected:
  double myA, myB, myC;
};

/// Represents a line segment in two-dimensional space.
/** The segment is defined by the coordinates of each endpoint. 
  @ingroup UtilityClasses
*/
class MvrLineSegment
{
public:
#ifndef SWIG
  /** @swigomit */
  MvrLineSegment() {}
  /** @brief Constructor with endpoints
   *  @swigomit
   */
  MvrLineSegment(double x1, double y1, double x2, double y2)
  { newEndPoints(x1, y1, x2, y2); }
#endif // SWIG
  /// Constructor with endpoints as MvrPose objects. Only X and Y components of the poses will be used.
  MvrLineSegment(MvrPose pose1, MvrPose pose2)
  { newEndPoints(pose1.getX(), pose1.getY(), pose2.getX(), pose2.getY()); }
  virtual ~MvrLineSegment() {}
  /// Set new end points for this line segment
  void newEndPoints(double x1, double y1, double x2, double y2)
  {
    myX1 = x1; myY1 = y1; myX2 = x2; myY2 = y2; 
    myLine.newParametersFromEndpoints(myX1, myY1, myX2, myY2);
  }
  /// Set new end points for this line segment
  void newEndPoints(const MvrPose& pt1, const MvrPose& pt2)
  {
    newEndPoints(pt1.getX(), pt1.getY(), pt2.getX(), pt2.getY());
  }
  /// Get the first endpoint (X1, Y1)
  MvrPose getEndPoint1(void) const { return MvrPose(myX1, myY1); }
  /// Get the second endpoint of (X2, Y2)
  MvrPose getEndPoint2(void) const { return MvrPose(myX2, myY2); }
  /// Determine where a line intersects this line segment
  /**
      @param line Line to check for intersection against this line segment.
      @param pose if the lines intersect, the X and Y components of this pose are set to the point of intersection.
      @return true if they intersect, false if they do not 
   **/
  bool intersects(const MvrLine *line, MvrPose *pose) const
  {
    MvrPose intersection;
    // see if it intersects, then make sure its in the coords of this line
    if (myLine.intersects(line, &intersection) && linePointIsInSegment(&intersection))
    {
      pose->setPose(intersection);
      return true;
    }
    else
	    return false;
  }

  /** @copydoc intersects(const MvrLine *line, MvrPose *pose) const */
  bool intersects(MvrLineSegment *line, MvrPose *pose) const
  {
    MvrPose intersection;
    // see if it intersects, then make sure its in the coords of this line
    if (myLine.intersects(line->getLine(), &intersection) &&
	  linePointIsInSegment(&intersection) && line->linePointIsInSegment(&intersection))
    {
	    pose->setPose(intersection);
	    return true;
    }
    else
	    return false;
  }
#ifndef SWIG
  /// Determine the intersection point between this line segment, and a perpendicular line passing through the given pose (i.e. projects the given pose onto this line segment.)
  /**
   * If there is no intersection, false is returned.
     @param pose The X and Y components of this pose object indicate the point to project onto this line segment.
     @param perpPoint The X and Y components of this pose object are set to indicate the intersection point
     @return true if an intersection was found and perpPoint was modified, false otherwise.
     @swigomit
  **/
  bool getPerpPoint(const MvrPose &pose, MvrPose *perpPoint) const
  {
    MvrLine perpLine;
    myLine.makeLinePerp(&pose, &perpLine);
    return intersects(&perpLine, perpPoint);
  }
#endif  //SWIG
  bool getPerpPoint(const MvrPose *pose, MvrPose *perpPoint) const
    {
      MvrLine perpLine;
      myLine.makeLinePerp(pose, &perpLine);
      return intersects(&perpLine, perpPoint);
    }
   /// Calculate the distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of
     @return if the pose does not intersect segment it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpDist(const MvrPose &pose) const
  {
    MvrPose perpPose;
    MvrLine perpLine;
    myLine.makeLinePerp(&pose, &perpLine);
    if (!intersects(&perpLine, &perpPose))
    	return -1;
    return (perpPose.findDistanceTo(pose));
  }
  /// Calculate the squared distance from the given point to (its projection on) this line segment
  /**
     @param pose the the pose to find the perp point of

     @return if the pose does not intersect segment it will return < 0
     if the pose intersects the segment it will return the distance to
     the intersection
  **/
  virtual double getPerpSquaredDist(const MvrPose &pose) const
  {
    MvrPose perpPose;
    MvrLine perpLine;
    myLine.makeLinePerp(&pose, &perpLine);
    if (!intersects(&perpLine, &perpPose))
	    return -1;
    return (perpPose.squaredFindDistanceTo(pose));
  }

  /// Gets the distance from this line segment to a point.
  /**
   * If the point can be projected onto this line segment (i.e. a
   * perpendicular line can be drawn through the point), then
   * return that distance. Otherwise, return the distance to the closest
   * endpoint.
     @param pose the pointer of the pose to find the distance to
  **/
  double getDistToLine(const MvrPose &pose) const
  {
    MvrPose perpPose;
    MvrLine perpLine;
    myLine.makeLinePerp(&pose, &perpLine);
    if (!intersects(&perpLine, &perpPose))
    {
	    return MvrUtil::findMin(
	                   MvrMath::roundInt(getEndPoint1().findDistanceTo(pose)),
	                   MvrMath::roundInt(getEndPoint2().findDistanceTo(pose)));
    }
    return (perpPose.findDistanceTo(pose));
  }
  
  /// Determines the length of the line segment
  double getLengthOf() const
  {
    return MvrMath::distanceBetween(myX1, myY1, myX2, myY2);
  }

  /// Determines the mid point of the line segment
  MvrPose getMidPoint() const
  {
    return MvrPose(((myX1 + myX2) / 2.0), ((myY1 + myY2) / 2.0));
  }


  /// Gets the x coordinate of the first endpoint
  double getX1(void) const { return myX1; }
  /// Gets the y coordinate of the first endpoint
  double getY1(void) const { return myY1; } 
  /// Gets the x coordinate of the second endpoint
  double getX2(void) const { return myX2; }
  /// Gets the y coordinate of the second endpoint
  double getY2(void) const { return myY2; }
  /// Gets the A line parameter (see MvrLine)
  double getA(void) const { return myLine.getA(); }
  /// Gets the B line parameter (see MvrLine)
  double getB(void) const { return myLine.getB(); }
  /// Gets the C line parameter (see MvrLine)
  double getC(void) const { return myLine.getC(); }

  /// Internal function for seeing if a point on our line is within our segment
  bool linePointIsInSegment(MvrPose *pose) const
  {
    bool isVertical = (MvrMath::fabs(myX1 - myX2) < MvrMath::epsilon());
    bool isHorizontal = (MvrMath::fabs(myY1 - myY2) < MvrMath::epsilon());

    if (!isVertical || !isHorizontal) 
    {
        return (((isVertical) || (pose->getX() >= myX1 && pose->getX() <= myX2) || 
	              (pose->getX() <= myX1 && pose->getX() >= myX2)) && ((isHorizontal) || 
	              (pose->getY() >= myY1 && pose->getY() <= myY2) || 
	              (pose->getY() <= myY1 && pose->getY() >= myY2)));
    }
    else 
    { // single point segment
          return ((MvrMath::fabs(myX1 - pose->getX()) < MvrMath::epsilon()) &&
                (MvrMath::fabs(myY1 - pose->getY()) < MvrMath::epsilon()));
    } // end else single point segment
  }

  const MvrLine *getLine(void) const { return &myLine; }

  /// Equality operator (for sets)
  virtual bool operator==(const MvrLineSegment& other) const
  {
    return ((fabs(myX1 - other.myX1) < MvrMath::epsilon()) &&
            (fabs(myY1 - other.myY1) < MvrMath::epsilon()) &&
            (fabs(myX2 - other.myX2) < MvrMath::epsilon()) &&
            (fabs(myY2 - other.myY2) < MvrMath::epsilon()));
  }

  virtual bool operator!=(const MvrLineSegment& other) const
  {
    return ((fabs(myX1 - other.myX1) > MvrMath::epsilon()) ||
            (fabs(myY1 - other.myY1) > MvrMath::epsilon()) ||
            (fabs(myX2 - other.myX2) > MvrMath::epsilon()) ||
            (fabs(myY2 - other.myY2) > MvrMath::epsilon()));
}

  /// Less than operator (for sets)
  virtual bool operator<(const MvrLineSegment& other) const
  {
    if (fabs(myX1 - other.myX1) > MvrMath::epsilon()) 
    {
      return myX1 < other.myX1;
    }
    else if (fabs(myY1 - other.myY1) > MvrMath::epsilon()) 
    {
      return myY1 < other.myY1;  
    }
    if (fabs(myX2 - other.myX2) > MvrMath::epsilon()) 
    {
      return myX2 < other.myX2;
    }
    else if (fabs(myY2 - other.myY2) > MvrMath::epsilon()) 
    {
      return myY2 < other.myY2;
    }
    // Otherwise... all coords are equal
    return false;
  }

protected:
  double myX1, myY1, myX2, myY2;
  MvrLine myLine;
};

/**
   @brief Use for computing a running average of a number of elements
   @ingroup UtilityClasses
*/
class MvrRunningAverage
{
public:
  /// Constructor, give it the number of elements to store to compute the average
  MVREXPORT MvrRunningAverage(size_t numToAverage);
  /// Destructor
  MVREXPORT ~MvrRunningAverage();
  /// Gets the average
  MVREXPORT double getAverage(void) const;
  /// Adds a value to the average. An old value is discarded if the number of elements to average has been reached.
  MVREXPORT void add(double val);
  /// Clears the average
  MVREXPORT void clear(void);
  /// Gets the number of elements
  MVREXPORT size_t getNumToAverage(void) const;
  /// Sets the number of elements
  MVREXPORT void setNumToAverage(size_t numToAverage);
  /// Sets if this is using a the root mean square average or just the normal average
  MVREXPORT void setUseRootMeanSquare(bool useRootMeanSquare);
  /// Gets if this is using a the root mean square average or just the normal average
  MVREXPORT bool getUseRootMeanSquare(void);
  /// Gets the number of values currently averaged so far
  MVREXPORT size_t getCurrentNumAveraged(void);
protected:
  size_t myNumToAverage;
  double myTotal;
  size_t myNum;
  bool myUseRootMeanSquare;
  std::list<double> myVals;
};

/// This is a class for computing a root mean square average of a number of elements
class MvrRootMeanSquareCalculator
{
public:
  /// Constructor
  MVREXPORT MvrRootMeanSquareCalculator();
  /// Destructor
  MVREXPORT ~MvrRootMeanSquareCalculator();
  /// Gets the average
  MVREXPORT double getRootMeanSquare (void) const;
  /// Adds a number
  MVREXPORT void add(int val);
  /// Clears the average
  MVREXPORT void clear(void);
  /// Sets the name
  MVREXPORT void setName(const char *name);
  /// Gets the name
  MVREXPORT const char *getName(void);  
  /// Gets the num averaged
  MVREXPORT size_t getCurrentNumAveraged(void);
protected:
  long long myTotal;
  size_t myNum;
  std::string myName;
};


//class MvrStrCaseCmpOp :  public std::binary_function <const std::string&, const std::string&, bool> 
/// strcasecmp for sets
/// @ingroup UtilityClasses
struct MvrStrCaseCmpOp 
{
public:
  bool operator() (const std::string &s1, const std::string &s2) const
  {
    return strcasecmp(s1.c_str(), s2.c_str()) < 0;
  }
};

/// MvrPose less than comparison for sets
/// @ingroup UtilityClasses
struct MvrPoseCmpOp
{
public:
  bool operator() (const MvrPose &pose1, const MvrPose &pose2) const
  {
    return (pose1 < pose2);
  }
};

/// MvrLineSegment less than comparison for sets
struct MvrLineSegmentCmpOp
{
public:
  bool operator() (const MvrLineSegment &line1, 
		               const MvrLineSegment &line2) const
  {
    return (line1 < line2);
  }
};


#if !defined(WIN32) && !defined(SWIG)
/** @brief Switch to running the program as a background daemon (i.e. fork) (Only available in Linux)
  @swigomit
  @notwindows
  @ingroup UtilityClasses
  @ingroup OptionalClasses
 */
class MvrDaemonizer
{
public:
  /// Constructor that sets up for daemonizing if arg checking
  MVREXPORT MvrDaemonizer(int *argc, char **argv, bool closeStdErrAndStdOut);
  /// Destructor
  MVREXPORT ~MvrDaemonizer();
  /// Daemonizes if asked too by arguments
  MVREXPORT bool daemonize(void);
  /// Daemonizes always
  MVREXPORT bool forceDaemonize(void);
  /// Logs the options
  MVREXPORT void logOptions(void) const;
  /// Returns if we're daemonized or not
  bool isDaemonized(void) { return myIsDaemonized; }
protected:
  MvrArgumentParser myParser;
  bool myIsDaemonized;
  bool myCloseStdErrAndStdOut;
  MvrConstFunctorC<MvrDaemonizer> myLogOptionsCB;
};
#endif // !win32 && !swig

/// Contains enumeration of four user-oriented priority levels (used primarily by MvrConfig)
class MvrPriority
{
public:
  enum Priority 
  {
    IMPORTANT, ///< Basic things that should be modified to suit 
    BASIC = IMPORTANT,  ///< Basic things that should be modified to suit 
    FIRST_PRIORITY = IMPORTANT,

    NORMAL,    ///< Intermediate things that users may want to modify
    INTERMEDIATE = NORMAL, ///< Intermediate things that users may want to modify

    DETAILED, ///< Advanced items that probably shouldn't be modified
    TRIVIAL = DETAILED, ///< Advanced items (alias for historic reasons)
    ADVANCED = DETAILED, ///< Advanced items that probably shouldn't be modified

    EXPERT,  ///< Items that should be modified only by expert users or developers
    FACTORY, ///< Items that should be modified at the factory, often apply to a robot model

    CALIBRATION, ///< Items that apply to a particular hardware instance

    LAST_PRIORITY = CALIBRATION ///< Last value in the enumeration
  };

  enum {
    PRIORITY_COUNT = LAST_PRIORITY + 1 ///< Number of priority values
  };

  /// Returns the displayable text string for the given priority
  MVREXPORT static const char * getPriorityName(Priority priority);
   
  /// Returns the priority value that corresponds to the given displayable text string
  /**
   * @param text the char * to be converted to a priority value
   * @param ok an optional bool * set to true if the text was successfully 
   * converted; false if the text was not recognized as a priority
  **/
  MVREXPORT static Priority getPriorityFromName(const char *text, bool *ok = NULL);

protected:
  /// Whether the map of priorities to display text has been initialized
  static bool ourStringsInited;
  /// Map of priorities to displayable text
  static std::map<Priority, std::string> ourPriorityNames;
  /// Map of displayable text to priorities
  static std::map<std::string, MvrPriority::Priority, MvrStrCaseCmpOp> ourNameToPriorityMap;
  /// Display text used when a priority's displayable text has not been defined
  static std::string ourUnknownPriorityName;
};

/// holds information about MvrStringInfo component strings (it's a helper class for other things)
/**
   This class holds information for about different strings that are available 
 **/
class MvrStringInfoHolder
{
public:
  /// Constructor
  MvrStringInfoHolder(const char *name, MvrTypes::UByte2 maxLength, MvrFunctor2<char *, MvrTypes::UByte2> *functor)
  { myName = name; myMaxLength = maxLength; myFunctor = functor; }
  /// Destructor
  virtual ~MvrStringInfoHolder() {}
  /// Gets the name of this piece of info
  const char *getName(void) { return myName.c_str(); }
  /// Gets the maximum length of this piece of info
  MvrTypes::UByte2 getMaxLength(void) { return myMaxLength; }
  /// Gets the function that will fill in this piece of info
  MvrFunctor2<char *, MvrTypes::UByte2> *getFunctor(void) { return myFunctor; }
protected:
  std::string myName;
  MvrTypes::UByte2 myMaxLength;
  MvrFunctor2<char *, MvrTypes::UByte2> *myFunctor;
};

/// This class just holds some helper functions for the MvrStringInfoHolder 
class MvrStringInfoHolderFunctions
{
public:
  static void intWrapper(char * buffer, MvrTypes::UByte2 bufferLen, MvrRetFunctor<int> *functor, const char *format)
  {
    snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
    buffer[bufferLen-1] = '\0'; 
  }
  static void doubleWrapper(char * buffer, MvrTypes::UByte2 bufferLen, MvrRetFunctor<double> *functor, const char *format)
  {
    snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
    buffer[bufferLen-1] = '\0'; 
  }
  static void boolWrapper(char * buffer, MvrTypes::UByte2 bufferLen, MvrRetFunctor<bool> *functor, const char *format)
  {
    snprintf(buffer, bufferLen - 1, format, 
	  MvrUtil::convertBool(functor->invokeR())); 
    buffer[bufferLen-1] = '\0'; 
  }
  static void stringWrapper(char * buffer, MvrTypes::UByte2 bufferLen, MvrRetFunctor<const char *> *functor, const char *format)
  {
    snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
    buffer[bufferLen-1] = '\0'; 
  }

  static void cppStringWrapper(char *buffer, MvrTypes::UByte2 bufferLen, MvrRetFunctor<std::string> *functor)
  { 
    snprintf(buffer, bufferLen - 1, "%s", functor->invokeR().c_str());
    buffer[bufferLen-1] = '\0'; 
  }

  static void unsignedLongWrapper(char * buffer, MvrTypes::UByte2 bufferLen, MvrRetFunctor<unsigned long> *functor, const char *format)
  {
    snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
    buffer[bufferLen-1] = '\0'; 
  }
  static void longWrapper(char * buffer, MvrTypes::UByte2 bufferLen, MvrRetFunctor<long> *functor, const char *format)
  {
    snprintf(buffer, bufferLen - 1, format, functor->invokeR()); 
    buffer[bufferLen-1] = '\0'; 
  }
};


/** @see MvrCallbackList */
template<class GenericFunctor> 
class MvrGenericCallbackList
{
public:
  /// Constructor
  MvrGenericCallbackList(const char *name = "", MvrLog::LogLevel logLevel = MvrLog::Verbose, bool singleShot = false)
  {
    myName = name;
    mySingleShot = singleShot;
    setLogLevel(logLevel);
    std::string mutexName;
    mutexName = "MvrGenericCallbackList::";
    mutexName += name;
    mutexName += "::myDataMutex";
    myDataMutex.setLogName(mutexName.c_str());
    myLogging = true;
  }
  /// Destructor
  virtual ~MvrGenericCallbackList() {}
  /// Adds a callback
  void addCallback(GenericFunctor functor, int position = 50)
  {
    myDataMutex.lock();
    myList.insert(std::pair<int, GenericFunctor>(-position,functor));
    myDataMutex.unlock();
  }
  /// Removes a callback
  void remCallback(GenericFunctor functor)
  {
    myDataMutex.lock();
    typename std::multimap<int, GenericFunctor>::iterator it;
    for (it = myList.begin(); it != myList.end(); it++)
    {
	    if ((*it).second == functor)
	    {
	      myList.erase(it);
	      myDataMutex.unlock();
	      remCallback(functor);
	      return;
	    }
    }
    myDataMutex.unlock();
  }
  /// Sets the name
  void setName(const char *name)
  {
    myDataMutex.lock();
    myName = name;
    myDataMutex.unlock();
  }
#ifndef SWIG
  /// Sets the name with formatting
  void setNameVar(const char *name, ...)
  {
    char arg[2048];
    va_list ptr;
    va_start(ptr, name);
    vsnprintf(arg, sizeof(arg), name, ptr);
    arg[sizeof(arg) - 1] = '\0';
    va_end(ptr);
    return setName(arg);
  }
#endif  // SWIG
  /// Sets the log level
  void setLogLevel(MvrLog::LogLevel logLevel)
  {
    myDataMutex.lock();
    myLogLevel = logLevel;
    myDataMutex.unlock();
  }
  /// Sets if its single shot
  void setSingleShot(bool singleShot)
  {
    myDataMutex.lock();
    mySingleShot = singleShot;
    myDataMutex.unlock();
  }
  /// Enable or disable logging when invoking the list. Logging is enabled by default at the log level given in the constructor.
  void setLogging(bool on) 
  {
    myLogging = on;
  }
protected:
  MvrMutex myDataMutex;
  MvrLog::LogLevel myLogLevel;
  std::string myName;
  std::multimap<int, GenericFunctor> myList;
  bool mySingleShot;
  bool myLogging;
};

/** Stores a list of MvrFunctor objects together.
    invoke() will invoke each of the MvrFunctor objects.
    The functors added to the list must be pointers to the same MvrFunctor subclass.
    Use MvrCallbackList for MvrFunctor objects with no arguments.
    Use MvrCallbackList1 for MvrFunctor1 objects with 1 argument.
    Use MvrCallbackList2 for MvrFunctor2 objects with 2 arguments.
    Use MvrCallbackList3 for MvrFunctor3 objects with 3 arguments.
    Use MvrCallbackListp for MvrFunctor4 objects with 4 arguments.
**/
class MvrCallbackList : public MvrGenericCallbackList<MvrFunctor *>
{
public:
  /// Constructor
  MvrCallbackList(const char *name = "", MvrLog::LogLevel logLevel = MvrLog::Verbose, bool singleShot = false) : 
                  MvrGenericCallbackList<MvrFunctor *>(name, logLevel, singleShot) {}
  /// Destructor
  virtual ~MvrCallbackList() {}
  /// Calls the callback list
  void invoke(void)
  {
    myDataMutex.lock();  
      
    std::multimap<int, MvrFunctor *>::iterator it;
    MvrFunctor *functor;
      
    if(myLogging)
      MvrLog::log( myLogLevel,  "%s: Starting calls to %d functors", myName.c_str(), myList.size());
      
    for (it = myList.begin(); it != myList.end(); it++)
    {
	    functor = (*it).second;
	    if (functor == NULL) 
	      continue;
	    if(myLogging)
	    {
	      if (functor->getName() != NULL && functor->getName()[0] != '\0')
	        MvrLog::log(myLogLevel, "%s: Calling functor '%s' at %d", myName.c_str(), functor->getName(), -(*it).first);
	      else
	        MvrLog::log(myLogLevel, "%s: Calling unnamed functor at %d", myName.c_str(), -(*it).first);
	    }
	    functor->invoke();
    }
    if(myLogging)
	    MvrLog::log(myLogLevel, "%s: Ended calls", myName.c_str());
    if (mySingleShot)
    {
	    if(myLogging)
	      MvrLog::log(myLogLevel, "%s: Clearing callbacks", myName.c_str());
	    myList.clear();
    }
    myDataMutex.unlock();
  }
protected:
};

/** @copydoc MvrCallbackList */
template<class P1>
class MvrCallbackList1 : public MvrGenericCallbackList<MvrFunctor1<P1> *>
{
public:
  /// Constructor
  MvrCallbackList1(const char *name = "", MvrLog::LogLevel logLevel = MvrLog::Verbose, bool singleShot = false) : 
                  MvrGenericCallbackList<MvrFunctor1<P1> *>(name, logLevel, singleShot) {}
  /// Destructor
  virtual ~MvrCallbackList1() {}
  /// Calls the callback list
  void invoke(P1 p1)
  {
    MvrGenericCallbackList<MvrFunctor1<P1> *>::myDataMutex.lock();
      
    typename std::multimap<int, MvrFunctor1<P1> *>::iterator it;
    MvrFunctor1<P1> *functor;
      
    if(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogging)
	    MvrLog::log(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogLevel, 
                  "%s: Starting calls to %d functors", 
                  MvrGenericCallbackList<MvrFunctor1<P1> *>::myName.c_str(),
                  MvrGenericCallbackList<MvrFunctor1<P1> *>::myList.size());
      
    for (it = MvrGenericCallbackList<MvrFunctor1<P1> *>::myList.begin(); 
	       it != MvrGenericCallbackList<MvrFunctor1<P1> *>::myList.end(); it++)
    {
      functor = (*it).second;
      if (functor == NULL) 
        continue;
	    if(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogging)
	    {
        if (functor->getName() != NULL && functor->getName()[0] != '\0')
          MvrLog::log(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogLevel,
              "%s: Calling functor '%s' at %d",
              MvrGenericCallbackList<MvrFunctor1<P1> *>::myName.c_str(), 
              functor->getName(), -(*it).first);
        else
          MvrLog::log(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogLevel, 
              "%s: Calling unnamed functor at %d", 
              MvrGenericCallbackList<MvrFunctor1<P1> *>::myName.c_str(), 
              -(*it).first);
      }
	    functor->invoke(p1);
    }
    if(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogging)
	    MvrLog::log(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogLevel, "%s: Ended calls",
                  MvrGenericCallbackList<MvrFunctor1<P1> *>::myName.c_str());
    if (MvrGenericCallbackList<MvrFunctor1<P1> *>::mySingleShot)
    {
	    if(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogging)
	      MvrLog::log(MvrGenericCallbackList<MvrFunctor1<P1> *>::myLogLevel, "%s: Clearing callbacks", 
		                MvrGenericCallbackList<MvrFunctor1<P1> *>::myName.c_str());
	    MvrGenericCallbackList<MvrFunctor1<P1> *>::myList.clear();
    }
    MvrGenericCallbackList<MvrFunctor1<P1> *>::myDataMutex.unlock();
  }
protected:
};

/** @copydoc MvrCallbackList */
template<class P1, class P2>
class MvrCallbackList2 : public MvrGenericCallbackList<MvrFunctor2<P1, P2> *>
{
public:
  typedef MvrFunctor2<P1, P2> FunctorType;
  typedef MvrGenericCallbackList<FunctorType*> Super;

  MvrCallbackList2(const char *name = "", MvrLog::LogLevel logLevel = MvrLog::Verbose, bool singleShot = false) : 
                  Super(name, logLevel, singleShot) {}

  void invoke(P1 p1, P2 p2)
  {
    Super::myDataMutex.lock();
      
    typename std::multimap<int, FunctorType*>::iterator it;
    FunctorType *functor;
      
    if(Super::myLogging)
      MvrLog::log(Super::myLogLevel, "%s: Starting calls to %d functors", Super::myName.c_str(), Super::myList.size());
      
    for (it = Super::myList.begin(); it != Super::myList.end(); ++it)
    {
      functor = (*it).second;
      if (functor == NULL) 
        continue;
      
      if(Super::myLogging)
      {
        if (functor->getName() != NULL && functor->getName()[0] != '\0')
          MvrLog::log(Super::myLogLevel, "%s: Calling functor '%s' at %d", Super::myName.c_str(), functor->getName(), -(*it).first);
        else
          MvrLog::log(Super::myLogLevel, "%s: Calling unnamed functor at %d", Super::myName.c_str(), -(*it).first);
      }
      functor->invoke(p1, p2);
    }
          
    if(Super::myLogging)
      MvrLog::log(Super::myLogLevel, "%s: Ended calls", Super::myName.c_str());
          
    if (Super::mySingleShot)
    {
      if(Super::myLogging)
        MvrLog::log(Super::myLogLevel, "%s: Clearing callbacks", Super::myName.c_str()); Super::myList.clear();
    }
    Super::myDataMutex.unlock();
  }
};


/** @copydoc MvrCallbackList */
template<class P1, class P2, class P3>
class MvrCallbackList3 : public MvrGenericCallbackList<MvrFunctor3<P1, P2, P3> *>
{
public:
  typedef MvrFunctor3<P1, P2, P3> FunctorType;
  typedef MvrGenericCallbackList<FunctorType*> Super;

  MvrCallbackList3(const char *name = "", MvrLog::LogLevel logLevel = MvrLog::Verbose, bool singleShot = false) : 
                  Super(name, logLevel, singleShot) {}

  void invoke(P1 p1, P2 p2, P3 p3)
  {
    Super::myDataMutex.lock();
      
    typename std::multimap<int, FunctorType*>::iterator it;
    FunctorType *functor;
      
    if(Super::myLogging)
      MvrLog::log(Super::myLogLevel, "%s: Starting calls to %d functors", Super::myName.c_str(), Super::myList.size());
      
    for (it = Super::myList.begin(); it != Super::myList.end(); ++it)
    {
      functor = (*it).second;
      if (functor == NULL) 
        continue;
      
      if(Super::myLogging)
      {
        if (functor->getName() != NULL && functor->getName()[0] != '\0')
          MvrLog::log(Super::myLogLevel, "%s: Calling functor '%s' at %d", Super::myName.c_str(), functor->getName(), -(*it).first);
        else
          MvrLog::log(Super::myLogLevel, "%s: Calling unnamed functor at %d", Super::myName.c_str(), -(*it).first);
      }
      functor->invoke(p1, p2, p3);
    }
          
    if(Super::myLogging)
      MvrLog::log(Super::myLogLevel, "%s: Ended calls", Super::myName.c_str());
          
    if (Super::mySingleShot)
    {
      if(Super::myLogging)
        MvrLog::log(Super::myLogLevel, "%s: Clearing callbacks", Super::myName.c_str()); Super::myList.clear();
    }
    Super::myDataMutex.unlock();
  }
};


/** @copydoc MvrCallbackList */
template<class P1, class P2, class P3, class P4>
class MvrCallbackList4 : public MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>
{
public:
    MvrCallbackList4(const char *name = "", MvrLog::LogLevel logLevel = MvrLog::Verbose, bool singleShot = false) :
                  MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>(name, logLevel, singleShot) {}
  virtual ~MvrCallbackList4() {}
  void invoke(P1 p1, P2 p2, P3 p3, P4 p4)
  {
    // references to members of parent class for clarity below
    MvrMutex &mutex = MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>::myDataMutex;
    MvrLog::LogLevel &loglevel = MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>::myLogLevel;
    const char *name = MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>::myName.c_str();
	  std::multimap< int, MvrFunctor4<P1, P2, P3, P4>* > &list = MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>::myList; 
    bool &singleshot = MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>::mySingleShot;
    bool &logging = MvrGenericCallbackList<MvrFunctor4<P1, P2, P3, P4> *>::myLogging;
      
    mutex.lock();
      
    typename std::multimap<int, MvrFunctor4<P1, P2, P3, P4> *>::iterator it;
    MvrFunctor4<P1, P2, P3, P4> *functor;
      
    if(logging)
      MvrLog::log( loglevel,  "%s: Starting calls to %d functors", name, list.size());
      
    for (it = list.begin();  it != list.end(); ++it)
    {
      functor = (*it).second;
      if (functor == NULL) 
        continue;
	
      if(logging)
      {
        if (functor->getName() != NULL && functor->getName()[0] != '\0')
          MvrLog::log(loglevel, "%s: Calling functor '%s' (0x%x) at %d", name, functor->getName(), functor, -(*it).first);
        else
          MvrLog::log(loglevel, "%s: Calling unnamed functor (0x%x) at %d", name, functor, -(*it).first);
      }
      functor->invoke(p1, p2, p3, p4);
    }
      
    if(logging)
      MvrLog::log(loglevel, "%s: Ended calls", name);
      
    if (singleshot)
    {
      if(logging)
        MvrLog::log(loglevel, "%s: Clearing callbacks", name);
      list.clear();
    }

    mutex.unlock();
  }
};


#ifndef MVRINTERFACE
#ifndef SWIG
/// @internal
class MvrLaserCreatorHelper
{
public:
  /// Creates an MvrLMS2xx
  static MvrLaser *createLMS2xx(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrLMS2xx
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateLMS2xxCB(void);
  /// Creates an MvrUrg
  static MvrLaser *createUrg(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrUrg
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateUrgCB(void);
  /// Creates an MvrLMS1XX
  static MvrLaser *createLMS1XX(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrLMS1XX
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateLMS1XXCB(void);

  /// Creates an MvrUrg using SCIP 2.0
  static MvrLaser *createUrg_2_0(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrUrg
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateUrg_2_0CB(void);
  /// Creates an MvrS3Series
  static MvrLaser *createS3Series(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrS3Series
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateS3SeriesCB(void);
  /// Creates an MvrLMS5XX
  static MvrLaser *createLMS5XX(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrLMS5XX
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateLMS5XXCB(void);
  /// Creates an MvrTiM3XX
  static MvrLaser *createTiM3XX(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrTiM3XX
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateTiM3XXCB(void);
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateTiM551CB(void);
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateTiM561CB(void);
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateTiM571CB(void);
  /// Creates an MvrSZSeries
  static MvrLaser *createSZSeries(int laserNumber, const char *logPrefix);
  /// Gets functor for creating an MvrSZSeries
  static MvrRetFunctor2<MvrLaser *, int, const char *> *getCreateSZSeriesCB(void);

protected:
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourLMS2xxCB;
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourUrgCB;
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourLMS1XXCB;
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourUrg_2_0CB;
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourS3SeriesCB;
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourLMS5XXCB;
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourTiM3XXCB;
  static MvrGlobalRetFunctor2<MvrLaser *, int, const char *> ourSZSeriesCB;
};

/// @internal
class MvrBatteryMTXCreatorHelper
{
public:
  /// Creates an MvrBatteryMTX
  static MvrBatteryMTX *createBatteryMTX(int batteryNumber, const char *logPrefix);
  /// Gets functor for creating an MvrBatteryMTX
  static MvrRetFunctor2<MvrBatteryMTX *, int, const char *> *getCreateBatteryMTXCB(void);
protected:
  static MvrGlobalRetFunctor2<MvrBatteryMTX *, int, const char *> ourBatteryMTXCB;
};

/// @internal
class MvrLCDMTXCreatorHelper
{
public:
  /// Creates an MvrLCDMTX
  static MvrLCDMTX *createLCDMTX(int lcdNumber, const char *logPrefix);
  /// Gets functor for creating an MvrLCDMTX
  static MvrRetFunctor2<MvrLCDMTX *, int, const char *> *getCreateLCDMTXCB(void);

protected:
  static MvrGlobalRetFunctor2<MvrLCDMTX *, int, const char *> ourLCDMTXCB;
};

/// @internal
class MvrSonarMTXCreatorHelper
{
public:
  /// Creates an MvrSonarMTX
  static MvrSonarMTX *createSonarMTX(int sonarNumber, const char *logPrefix);
  /// Gets functor for creating an MvrSonarMTX
  static MvrRetFunctor2<MvrSonarMTX *, int, const char *> *getCreateSonarMTXCB(void);

protected:
  static MvrGlobalRetFunctor2<MvrSonarMTX *, int, const char *> ourSonarMTXCB;
};

#endif // SWIG
#endif // MVRINTERFACE

#ifndef SWIG
/// @internal
class MvrDeviceConnectionCreatorHelper
{
public:
  /// Creates an MvrSerialConnection
  static MvrDeviceConnection *createSerialConnection(const char *port, const char *defaultInfo, const char *logPrefix);
  /// Gets functor for creating an MvrSerialConnection
  static MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *,const char *> *getCreateSerialCB(void);

  /// Creates an MvrTcpConnection
  static MvrDeviceConnection *createTcpConnection( const char *port, const char *defaultInfo, const char *logPrefix);
  /// Gets functor for creating an MvrTcpConnection
  static MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *getCreateTcpCB(void);

  /// Creates an MvrSerialConnection for RS422
  static MvrDeviceConnection *createSerial422Connection( const char *port, const char *defaultInfo, const char *logPrefix);
  /// Gets functor for creating an MvrSerialConnection
  static MvrRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> *getCreateSerial422CB(void);

  /// Sets the success log level
  static void setSuccessLogLevel(MvrLog::LogLevel successLogLevel);
  /// Sets the success log level
  static MvrLog::LogLevel setSuccessLogLevel(void);
protected:
  /// Internal Create MvrSerialConnection
  static MvrDeviceConnection *internalCreateSerialConnection(
	       const char *port, const char *defaultInfo, const char *logPrefix, bool is422);
  static MvrGlobalRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> ourSerialCB;
  static MvrGlobalRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> ourTcpCB;
  static MvrGlobalRetFunctor3<MvrDeviceConnection *, const char *, const char *, const char *> ourSerial422CB;
  static MvrLog::LogLevel ourSuccessLogLevel;
};
#endif // SWIG

/// Class for finding robot bounds from the basic measurements
class MvrPoseUtil
{
public:
  MVREXPORT static std::list<MvrPose> findCornersFromRobotBounds(
	        double radius, double widthLeft, double widthRight, 
	        double lengthFront, double lengthRear, bool fastButUnsafe);
  MVREXPORT static std::list<MvrPose> breakUpDistanceEvenly(MvrPose start, MvrPose end, int resolution);
};

/// class for checking if something took too long and logging it
class MvrTimeChecker
{
public:
  /// Constructor
  MVREXPORT MvrTimeChecker(const char *name = "Unknown", int defaultMSecs = 100);
  /// Destructor
  MVREXPORT virtual ~MvrTimeChecker();
  /// Sets the name
  void setName(const char *name) { myName = name; }
  /// Sets the default mSecs
  void setDefaultMSecs(int defaultMSecs) { myMSecs = defaultMSecs; }
  /// starts the check
  MVREXPORT void start(void);
  /// checks, optionally with a subname (only one subname logged per cycle)
  MVREXPORT void check(const char *subName);
  /// Finishes the check
  MVREXPORT void finish(void);
  /// Gets the last time a check happened (a start counts as a check too)
  MvrTime getLastCheckTime() { return myLastCheck; }
protected:
  std::string myName;
  int myMSecs;
  MvrTime myStarted;
  MvrTime myLastCheck;
};

#endif  // MVRIAUTIL_H