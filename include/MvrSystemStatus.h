/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSystemStatus.h
 > Description  : Utility to get statistics about the  host operating system
 > Author       : Yu Jie
 > Create Time  : 2017年06月04日
 > Modify Time  : 2017年06月05日
***************************************************************************************************/
#ifndef MVRSYSTEMSTATUS_H
#define MVRSYSTEMSTATUS_H

#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "mvriaUtil.h"
#include "MvrMutex.h"
#include <string>

class MvrSystemStatusRefreshThread;


/*  @brief Utility to get statistics about the  host operating system
 *  (CPU usage, wireless link data, etc).
 *
 *  Normally, calling any accessor to read a value will query the operating
 *  system to get the most recent value.  However, if you will be accessing
 *  data very frequently and want those calls to be faster, you can start 
 *  a thread by calling startPeriodicUpdate() which will periodically query
 *  new values from the operating system and cache them for accessors to 

 *  return.
  @ingroup UtilityClasses
 */
class MvrSystemStatus{
public:
  /*
  *  Create a new thread which periodically invalidates cached data,
  *  causing it to be recalculated when next accessed.  Starting
  *  this thread is optional; start it if you 
  *  will be accessing the data frequently, so that is doesn't need to
  *  be re-read and re-calculated on each access. If you will only be 
  *  accessing the data occasionally, you do not need to start the update
  *  thread, it will be updated each time you read a value.
  */
  MVREXPORT static void startPeriodicUpdate(int refreshFrequency=5000, MvrLog::Loglevel logLevel=MvrLog::Verbose);

  /// Stop periodic update thread . Henceforth any access of data will
  /// cause it to be re-read and recalculated
  MVREXPORT static void stopPeriodicUpdate();

  /// use startPeriodicUpdate() 
  /// @deprecated
  MVREXPORT static void runRefreshThread(int refreshFrequency = 5000){
    startPeriodicUpdate(refreshFrequency);
  }

 /*
  * Get CPU work to idle ratio since last refresh.
  *  This is a value ranging from (0 .. 1) X (Num. CPUs). (Therefore
  *  if you have two CPUs, the maximum value will be 2.0, or 200%.)
  *  This value is calculated as the percentage 
  *  of time the CPU spent doing work (not in "idle" state) since the 
  *  previous calculation.
  *  @return CPU usage value, or -1 if unable to determine
  */
  MVREXPORT static double getCPU();

 /*
  * Get CPU usage as percentage since last refresh. This is a value ranging from
  *  (0..100) X (Num. CPUs). (Therefore if you have two CPUs, the maximum value
  *  will be 200%).
  *  @sa getCPU()
  *  @return CPU usage as percentage, or -1 if not able to determine
  */
  MVREXPORT static double getCPUPercent();

  /// Get CPU percentage in a string
  MVREXPORT static std::string getCPUPercentAsString();

  /// Get total system uptime (seconds)
  MVREXPORT static unsigned long getUptime();

  /// Get program's uptiem (seconds)
  MVREXPORT static unsigned long getProgramUptime();

  /// Get total system uptime (hours)
  MVREXPORT static double getUptimeHours();

  /// Get total system uptime in a string (hours)
  MVREXPORT static std::string getUptimeHoursAsString();

  /// @return Pointer to a functor which can be used to retrieve the current CPU percentage
  MVREXPORT static MvrRetFunctor<double>* getCPUPercentFunctor();

  /// @return Pointer to a functor which can be used to retrieve the current uptime (hours)
  MVREXPORT static MvrRetFunctor<double>* getUptimeHoursFunctor();

  /// @return Pointer to a functor which can be used to retrieve the current uptime (hours)
  MVREXPORT static MvrRetFunctor<unsigned long>* getUptimeFunctor();

  /// @return Pointer to a functor which can be used to retrieve the current uptime (hours)
  MVREXPORT static MvrRetFunctor<unsigned long>* getProgramUptimeFunctor();

  /// Get wireless network general link quality heuristic (for first configured wireless device).
  MVREXPORT static int getWirelessLinkQuality();

  /// Get wireless netork signal level (for first configured wireless device).
  MVREXPORT static int getWirelessLinkSignal();

  /// Get wireless network noise level (for first configured wireless device). */
  MVREXPORT static int getWirelessLinkNoise();

  /// Get wireless network total discarded packets (for first configured  wireless device). */
  MVREXPORT static int getWirelessDiscardedPackets();

  /// Get wireless network packets discarded because of a conflict with another
  /// network (for first configured wireless device).
  MVREXPORT static int getWirelessDiscardedPacketsBecauseNetConflict();

  /// Get if the wireless has a link */
  MVREXPORT static int getMTXWirelessLink();

  /// Get wireless network quality (for first configured wireless device).
  MVREXPORT static int getMTXWirelessQuality();

  /// Get wireless network ip address (for first configured wireless device).
  MVREXPORT static int getMTXWirelessIpAddress1();
  MVREXPORT static int getMTXWirelessIpAddress2();
  MVREXPORT static int getMTXWirelessIpAddress3();
  MVREXPORT static int getMTXWirelessIpAddress4();

  /// Gets the wireless IP address as a string
  MVREXPORT static const char *getMTXWirelessIpAddressString();

  MVREXPORT static MvrRetFunctor<int>* getWirelessLinkQualityFunctor();
  MVREXPORT static MvrRetFunctor<int>* getWirelessLinkNoiseFunctor();
  MVREXPORT static MvrRetFunctor<int>* getWirelessLinkSignalFunctor();

  MVREXPORT static MvrRetFunctor<int>* getMTXWirelessLinkFunctor();
  MVREXPORT static MvrRetFunctor<int>* getMTXWirelessQualityFunctor();

  /// @internal
  MVREXPORT static void invalidate();

  /// @deprecated
  MVREXPORT static void refresh() {}
private:

  static MvrMutex ourCPUMutex;
	static double ourCPU;
	static unsigned long ourUptime;
	static unsigned long ourFirstUptime;
	static unsigned long ourLastCPUTime;
	static MvrTime ourLastCPURefreshTime;
	static MvrGlobalRetFunctor<double> ourGetCPUPercentCallback;
	static MvrGlobalRetFunctor<double> ourGetUptimeHoursCallback;
	static MvrGlobalRetFunctor<unsigned long> ourGetUptimeCallback;
	static MvrGlobalRetFunctor<unsigned long> ourGetProgramUptimeCallback;

  static MvrMutex ourWirelessMutex;
  static int ourLinkQuality, ourLinkSignal, ourLinkNoise, ourDiscardedTotal, ourDiscardedDecrypt, ourDiscardedConflict;
  static MvrGlobalRetFunctor<int> ourGetWirelessLinkQualityCallback;
  static MvrGlobalRetFunctor<int> ourGetWirelessLinkNoiseCallback;
  static MvrGlobalRetFunctor<int> ourGetWirelessLinkSignalCallback;

  static MvrMutex ourMTXWirelessMutex;
  static int ourMTXWirelessLink, ourMTXWirelessQuality, ourMTXIp1, ourMTXIp2, ourMTXIp3, ourMTXIp4;
  static std::string ourMTXIpString;
  static MvrGlobalRetFunctor<int> ourGetMTXWirelessLinkCallback;
  static MvrGlobalRetFunctor<int> ourGetMTXWirelessQualityCallback;

  static void refreshCPU();          ///< Refresh CPU, if necessary
  static void refreshWireless();     ///< Refresh Wireless stats, if necessary

  static void refreshMTXWireless();  ///< Refresh MTX Wireless stats, if necessary

  static MvrSystemStatusRefreshThread* ourPeriodicUpdateThread;
  static bool ourShouldRefreshWireless;
  static bool ourShouldRefreshCPU;

  static bool ourShouldRefreshMTXWireless;
};
#endif  // MVRSYSTEMSTATUS_H