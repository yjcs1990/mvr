/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLog.h
 > Description  : Classes for robot log
 > Author       : Yu Jie
 > Create Time  : 2017年04月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/

#ifndef MVRLOG_H
#define MVRLOG_H

#ifndef WIN32
#include <stdio.h>
#endif  // WIN32
#include <string>
#include "mvriaTypedefs.h"
#include "MvrMutex.h"
#include "MvrFunctor.h"

class MvrConfig;

/// Logging utility class
/**
   MvrLog is a utility class to log all messages from Mvria to a choosen
   destintation. Messages can be logged to stdout, stderr, a file, and
   turned off completely. Logging by default is set to stdout. The level
   of logging can be changed as well. Allowed levels are Terse, Normal,
   and Verbose. By default the level is set to Normal.

   @ingroup ImportantClasses
   @ingroup easy
*/
class MvrLog
{
public:
  typedef enum {
    StdOut, ///< Use stdout for logging
    StdErr, ///< Use stderr for logging
    File, ///< Use a file for logging
    Colbert, ///< Use a Colbert stream for logging
    None ///< Disable logging
  } LogType;
  typedef enum {
    Terse, ///< Use terse logging
    Normal, ///< Use normal logging
    Verbose ///< Use verbose logging
  } LogLevel;

#ifndef SWIG
  /** @brief Log a message, with formatting and variable number of arguments
   *  @swignote In Java and Python, this function only takes one 
   *    string argument. Use Java or Python's native facities
   *    for constructing a formatted string, e.g. the % and + string
   *    operators in Python, and the methods of the Java String class.
   */
  MVREXPORT static void log(LogLevel level, const char *str, ...);
#endif
  /// Log a message containing just a plain string
  MVREXPORT static void logPlain(LogLevel level, const char *str);
  /// Initialize the logging utility with options
  MVREXPORT static bool init(LogType type, LogLevel level, const char *fileName="",
			      bool logTime = false, bool alsoPrint = false, bool printThisCall = true);
  /// Close the logging utility
  MVREXPORT static void close();

  /// Logs an error, adding the error and string the error mean at the
  /// end of this message
  MVREXPORT static void logErrorFromOS(LogLevel level, const char *str, ...);
  /// Logs an error, adding the error and string the error mean at the
  /// end of this message
  MVREXPORT static void logErrorFromOSPlain(LogLevel level, const char *str);
#ifndef SWIG // these is internal we don't need to wrap it
  /// Logs an error, adding the error and string the error mean at the
  /// end of this message... internal version, don't use it
  MVREXPORT static void logErrorFromOSNoLock(LogLevel level, const char *str, ...);
  /// Logs an error, adding the error and string the error mean at the
  /// end of this message... internal version, dont' use it
  MVREXPORT static void logErrorFromOSPlainNoLock(LogLevel level, const char *str);
  // Do not use this unless you know what you are doing...
  /** @internal
   * @swigomit */
  MVREXPORT static void logNoLock(LogLevel level, const char *str, ...);
#endif 
  /// Log function call backtrace for debugging 
  /// @linuxonly
  MVREXPORT static void logBacktrace(LogLevel level);
  /// Read the contents of @fileName and print a log message for each line. File should be plain text.
  MVREXPORT static bool logFileContents(LogLevel level, const char *fileName);

  // We use this to print to a Colbert stream, if available
  /// @deprecated
  MVREXPORT static void (* colbertPrint)(int i, const char *str);

  /// Use an MvrConfig object to control MvrLog's options
  MVREXPORT static void addToConfig(MvrConfig *config);

  /// Set log level
  MVREXPORT static void setLogLevel(LogLevel level);

#ifndef MVRINTERFACE
  // Init for aram behavior
  /// @internal
  MVREXPORT static void aramInit(const char *prefix, MvrLog::LogLevel defaultLevel = MvrLog::Normal, 
				    double defaultSize = 10, bool daemonized = false);
#endif
  
  /// Set a functor to be called when a log message is made 
  /// Call clearFunctor() to unset.
  MVREXPORT static void setFunctor(MvrFunctor1<const char *> *functor);
  /// Clear functor set by setFunctor().
  MVREXPORT static void clearFunctor();
  /// Internal function to force a lockup, only for debugging
  /// @internal
  MVREXPORT static void internalForceLockup(void);

  /// Convenience function to log a message at Terse log level with "Warning: " prepended
  MVREXPORT static void warning(const char *str, ...);
  /// Convenience function to log a message at Terse log level with "Error: " prepended
  MVREXPORT static void error(const char *str, ...);
  /// Convenience function to log a message at Normal log level 
  MVREXPORT static void info(const char *str, ...);
  /// Convenience function to log a message at Verbose log level 
  MVREXPORT static void debug(const char *str, ...);

#ifndef SWIG
  /// @internal
  MVREXPORT static void log_v(LogLevel level, const char *prefix, const char *format, va_list vaptr);
#endif

protected:
  MVREXPORT static bool processFile(void);
#ifndef MVRINTERFACE
  MVREXPORT static bool aramProcessFile(void);
  MVREXPORT static void filledAramLog(void);
#endif
  MVREXPORT static void invokeFunctor(const char *message);
  MVREXPORT static void checkFileSize(void);

  static MvrLog *ourLog;
  static MvrMutex ourMutex;
  static LogType ourType;
  static LogLevel ourLevel;
  static bool ourLoggingTime;
  static FILE *ourFP;
  static int ourColbertStream;
  static std::string ourFileName;
  static bool ourAlsoPrint;
  static int ourCharsLogged;
  
  static LogType ourConfigLogType;
  static LogLevel ourConfigLogLevel;
  static char ourConfigFileName[1024];
  static bool ourConfigLogTime;
  static bool ourConfigAlsoPrint;
  static MvrGlobalRetFunctor<bool> ourConfigProcessFileCB;

#ifndef MVRINTERFACE
  static char ourAramConfigLogLevel[1024];
  static double ourAramConfigLogSize;
  static MvrGlobalRetFunctor<bool> ourMvramConfigProcessFileCB;
  static bool ourUseAramBehavior;
  static double ourAramLogSize;
  static std::string ourMvramPrefix;
#endif

  static bool ourMvramDaemonized;
  
  static MvrFunctor1<const char *> *ourFunctor;
};

#endif  //MVRLOG_H