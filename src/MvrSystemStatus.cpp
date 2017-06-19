/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSystemStatus.cpp
 > Description  : Utility to get statistics about the  host operating system
 > Author       : Yu Jie
 > Create Time  : 2017年06月04日
 > Modify Time  : 2017年06月19日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "mvriaInternal.h"
#include "MvrASyncTask.h"
#include "MvrSystemStatus.h"

#include <stdio.h>

double MvrSystemStatus::ourCPU = -1.0;
unsigned long MvrSystemStatus::ourUptime = 0;
unsigned long MvrSystemStatus::ourFirstUptime = 0;
unsigned long MvrSystemStatus::ourLastCPUTime = 0;
MvrTime MvrSystemStatus::ourLastCPURefreshTime;
MvrGlobalRetFunctor<double> MvrSystemStatus::ourGetCPUPecentCallback(&MvrSystemStatus::getCPUPercent);
MvrGlobalRetFunctor<double> MvrSystemStatus::ourGetUptimeHoursCallback(&MvrSystemStatus::getUptimeHours);
MvrGlobalRetFunctor<unsigned long> MvrSystemStatus::ourGetUptimeCallback(&MvrSystemStatus::getUptime);
MvrGlobalRetFunctor<unsigned long> MvrSystemStatus::ourGetProgramUptimeCallback(&MvrSystemStatus::getProgramUptime);

int MvrSystemStatus::ourLinkQuality       = -1;
int MvrSystemStatus::ourLinkSignal        = -1;
int MvrSystemStatus::ourLinkNoise         = -1;
int MvrSystemStatus::ourDiscardedTotal    = -1;
int MvrSystemStatus::ourDiscardedConflict = -1;
int MvrSystemStatus::ourDiscardedDecrypt  = -1;
int MvrSystemStatus::ourMTXIp1            = -1;
int MvrSystemStatus::ourMTXIp2            = -1;
int MvrSystemStatus::ourMTXIp3            = -1;
int MvrSystemStatus::ourMTXIp4            = -1;
std::string MvrSystemStatus::ourMTXIpString = "";
MvrMutex MvrSystemStatus::ourCPUMutex;
MvrMutex MvrSystemStatus::ourWirelessMutex;
MvrMutex MvrSystemStatus::ourMTXWirelessMutex;
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetWirelessLinkQualityCallback(&MvrSystemStatus::getWirelessLinkQuality);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetWirelessLinkNoiseCallback(&MvrSystemStatus::getWirelessLinkNoise);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetWirelessLinkNoiseCallback(&MvrSystemStatus::getWirelessLinkSignal);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetMTXWirelessLinkCallback(&MvrSystemStatus::getMTXWirelessLink);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetMTXWirelessQualityCallback(&MvrSystemStatus::getMTXWirelessQuality);
MvrSystemStatusRefreshThread *MvrSystemStatus::ourPeriodicUpdateThread = 0;
bool MvrSystemStatus::ourShouldRefreshCPU = true;
bool MvrSystemStatus::ourShouldRefreshWireless = true;
bool MvrSystemStatus::ourShouldRefreshMTXWireless = true;

void MvrSystemStatus::refreshCPU()
{
#ifndef WIN32
  if (ourPeriodicUpdateThread && !ourShouldRefreshCPU) 
    return;
  unsigned long interval = ourLastCPURefreshTime.mSecSince();
  FILE *statfp   = MvrUtil::fopen("/proc/stat", "r");
  FILE *uptimefp = MvrUtil::fopen("/proc/uptime", "r");
  if (!statfp)
    MvrLog::log(MvrLog::Terse, "MvrSystemStatus: Error: Failed to open /proc/stat!") ;
  if (!uptimefp)
    MvrLog::log(MvrLog::Terse, "MvrSystemStatus: Error: Failed to open /proc/uptime!") ;

  if (!statfp || !uptimefp)
  {
    ourCPU = -1;
    ourLastCPUTime = ourUptime = 0;
    ourShouldRefreshCPU = false;
    if (statfp)
      fclose(statfp);
    if (uptimefp)
      fclose(uptimefp);
    return;
  }

  unsigned long uptime;
  fscanf(uptimefp, "%ld", &uptime);
  ourUptime = uptime;
  fclose(uptimefp);

  if (ourFirstUptime == 0)
    ourFirstUptime = ourUptime;
  
  unsigned long user, nice, sys, idle, total;
  char tag[32];
	fscanf(statfp, "%s %lu %lu %lu %lu", tag, &user, &nice, &sys, &idle);
  fclose(statfp);
  total = user + nice + sys; // total non-idle cpu time in 100 ths of a sec
  if (ourLastCPUTime == 0 || interval == 0)
  {
    // no time has past since last refesh
    ourLastCPUTime = total;
    ourShouldRefreshCPU = false;
    return;
  }
  ourCPU = (double)(total - ourLastCPUTime) / ((double)interval / 10.0);
  ourLastCPUTime = total;
  ourLastCPURefreshTime.setToNow();
  ourShouldRefreshCPU = false;
#endif  
}

/// Internal class



