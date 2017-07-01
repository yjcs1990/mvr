#include "MvrExport.h"
#include "mvriaInternal.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrSystemStatus.h"
#include "MvrASyncTask.h"
#include <stdio.h>


double MvrSystemStatus::ourCPU = -1.0;
unsigned long MvrSystemStatus::ourUptime = 0;
unsigned long MvrSystemStatus::ourFirstUptime = 0;
unsigned long MvrSystemStatus::ourLastCPUTime = 0;
MvrTime MvrSystemStatus::ourLastCPURefreshTime;
MvrGlobalRetFunctor<double> MvrSystemStatus::ourGetCPUPercentCallback(&MvrSystemStatus::getCPUPercent);
MvrGlobalRetFunctor<double> MvrSystemStatus::ourGetUptimeHoursCallback(&MvrSystemStatus::getUptimeHours);
MvrGlobalRetFunctor<unsigned long> MvrSystemStatus::ourGetUptimeCallback(&MvrSystemStatus::getUptime);
MvrGlobalRetFunctor<unsigned long> MvrSystemStatus::ourGetProgramUptimeCallback(&MvrSystemStatus::getProgramUptime);

int MvrSystemStatus::ourLinkQuality = -1;
int MvrSystemStatus::ourLinkSignal = -1;
int MvrSystemStatus::ourLinkNoise = -1;
int MvrSystemStatus::ourDiscardedTotal = -1;
int MvrSystemStatus::ourDiscardedConflict = -1;
int MvrSystemStatus::ourDiscardedDecrypt = -1;
int MvrSystemStatus::ourMTXWirelessLink = -1;
int MvrSystemStatus::ourMTXWirelessQuality = -1;
int MvrSystemStatus::ourMTXIp1 = -1;
int MvrSystemStatus::ourMTXIp2 = -1;
int MvrSystemStatus::ourMTXIp3 = -1;
int MvrSystemStatus::ourMTXIp4 = -1;
std::string MvrSystemStatus::ourMTXIpString = "";
MvrMutex MvrSystemStatus::ourCPUMutex;
MvrMutex MvrSystemStatus::ourWirelessMutex;
MvrMutex MvrSystemStatus::ourMTXWirelessMutex;
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetWirelessLinkQualityCallback(&MvrSystemStatus::getWirelessLinkQuality);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetWirelessLinkNoiseCallback(&MvrSystemStatus::getWirelessLinkNoise);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetWirelessLinkSignalCallback(&MvrSystemStatus::getWirelessLinkSignal);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetMTXWirelessLinkCallback(&MvrSystemStatus::getMTXWirelessLink);
MvrGlobalRetFunctor<int> MvrSystemStatus::ourGetMTXWirelessQualityCallback(&MvrSystemStatus::getMTXWirelessQuality);
MvrSystemStatusRefreshThread* MvrSystemStatus::ourPeriodicUpdateThread = 0;
bool MvrSystemStatus::ourShouldRefreshWireless = true;
bool MvrSystemStatus::ourShouldRefreshMTXWireless = true;
bool MvrSystemStatus::ourShouldRefreshCPU = true;



void MvrSystemStatus::refreshCPU()
{
#ifndef WIN32
	if (ourPeriodicUpdateThread && !ourShouldRefreshCPU) return;
	unsigned long interval = ourLastCPURefreshTime.mSecSince();
	FILE* statfp = MvrUtil::fopen("/proc/stat", "r");
	FILE* uptimefp = MvrUtil::fopen("/proc/uptime", "r");
	if (!statfp) {
		MvrLog::log(MvrLog::Terse, "MvrSystemStatus: Error: Failed to open /proc/stat!");
	}
	if (!uptimefp) {
		MvrLog::log(MvrLog::Terse, "MvrSystemStatus: Error: Failed to open /proc/uptime!");
	}
	if (!statfp || !uptimefp)
	{
		ourCPU = -1.0;
		ourLastCPUTime = ourUptime = 0;
		ourShouldRefreshCPU = false;
		if (statfp)
			fclose(statfp);
		if (uptimefp)
			fclose(uptimefp);
		return;
	}
	//char line[512];
	//fgets(line,  512, uptimefp);
	//printf("read uptime file: %s\n", line);
	//double uptime = 0, idle_uptime = 0;
	//fscanf(uptimefp, "%lf %lf", &uptime, &idle_uptime);
	//ourUptime = (unsigned long)uptime;
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
	total = user + nice + sys; // total non-idle cpu time in 100ths of a sec
	if (ourLastCPUTime == 0 || interval == 0)
	{
		// no time has past since last refresh
		ourLastCPUTime = total;
		ourShouldRefreshCPU = false;
		return;
	}
	ourCPU = (double)(total - ourLastCPUTime) / ((double)interval / 10.0); // convert 'interval' to 1/100 sec units
	ourLastCPUTime = total;
	ourLastCPURefreshTime.setToNow();
	ourShouldRefreshCPU = false;
#endif // WIN32
}



/** @cond INTERNAL_CLASSES */
class MvrSystemStatusRefreshThread : public virtual MvrASyncTask {
public:
	MvrSystemStatusRefreshThread(int refreshFrequency) :
		myRefreshFrequency(refreshFrequency)
	{
		setThreadName("MvrSystemStatusRefreshThread");
	}
	void runAsync() { create(false); }
	void setRefreshFreq(int freq) { myRefreshFrequency = freq; }
private:
	int myRefreshFrequency;
	virtual void* runThread(void* arg)
	{
		threadStarted();
		while (Mvria::getRunning() && getRunning())
		{
			MvrSystemStatus::invalidate();
			MvrUtil::sleep(myRefreshFrequency);
		}
		threadFinished();
		return NULL;
	}
};
/** @endcond INTERNAL_CLASSES */


MVREXPORT void MvrSystemStatus::startPeriodicUpdate(int refreshFrequency, MvrLog::LogLevel logLevel)
{
	ourCPUMutex.setLogName("MvrSystemStatusRefreshThread::ourCPUMutex");
	ourWirelessMutex.setLogName("MvrSystemStatusRefreshThread::ourWirelessMutex");

	if (ourPeriodicUpdateThread) {
		// If we already have a thread, just change its refresh frequency
		ourPeriodicUpdateThread->setRefreshFreq(refreshFrequency);
		ourPeriodicUpdateThread->setLogLevel(logLevel);
		return;
	}
	// Otherwise, start a new thread, with the desired refresh frequency
	ourPeriodicUpdateThread = new MvrSystemStatusRefreshThread(refreshFrequency);
	ourPeriodicUpdateThread->setLogLevel(logLevel);
	ourPeriodicUpdateThread->runAsync();
}

MVREXPORT void MvrSystemStatus::stopPeriodicUpdate()
{
	if (!ourPeriodicUpdateThread) return;
	ourPeriodicUpdateThread->stopRunning();
	delete ourPeriodicUpdateThread;
	ourPeriodicUpdateThread = 0;
}



MVREXPORT double MvrSystemStatus::getCPU() {
	MvrScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourCPU;
}

MVREXPORT double MvrSystemStatus::getCPUPercent() {
	MvrScopedLock lock(ourCPUMutex);
	refreshCPU();
	if (ourCPU < 0)
	{
		return ourCPU;  // invalid value indicator
	}
	return ourCPU * 100.0;
}

// Get CPU percentage in a string
MVREXPORT std::string MvrSystemStatus::getCPUPercentAsString() {
	MvrScopedLock lock(ourCPUMutex);
	refreshCPU();
	if (ourCPU < 0)
	{
		return std::string("n/a");
	}
	char tmp[32];
	snprintf(tmp, 31, "%.2f", getCPUPercent());
	return std::string(tmp);
}

// Get total system uptime (seconds)
MVREXPORT unsigned long MvrSystemStatus::getUptime() {
	MvrScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourUptime;
}

// Get total system uptime (hours)
MVREXPORT double MvrSystemStatus::getUptimeHours() {
	MvrScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourUptime / 3600.0;
}

// Get total system uptime (seconds)
MVREXPORT unsigned long MvrSystemStatus::getProgramUptime() {
	MvrScopedLock lock(ourCPUMutex);
	refreshCPU();
	return ourUptime - ourFirstUptime;
}

// Get total system uptime in a string (hours)
MVREXPORT std::string MvrSystemStatus::getUptimeHoursAsString() {
	MvrScopedLock lock(ourCPUMutex);
	refreshCPU();
	char tmp[32];
	snprintf(tmp, 31, "%.2f", getUptimeHours());
	return std::string(tmp);
}

// return Pointer to a functor which can be used to retrieve the current CPU percentage
MVREXPORT MvrRetFunctor<double>* MvrSystemStatus::getCPUPercentFunctor() {
	return &ourGetCPUPercentCallback;
}

// return Pointer to a functor which can be used to retrieve the current uptime (hours)
MVREXPORT MvrRetFunctor<double>* MvrSystemStatus::getUptimeHoursFunctor() {
	return &ourGetUptimeHoursCallback;
}

// return Pointer to a functor which can be used to retrieve the current uptime (seconds)
MVREXPORT MvrRetFunctor<unsigned long>* MvrSystemStatus::getUptimeFunctor() {
	return &ourGetUptimeCallback;
}

// return Pointer to a functor which can be used to retrieve the current program uptime (seconds)
MVREXPORT MvrRetFunctor<unsigned long>* MvrSystemStatus::getProgramUptimeFunctor() {
	return &ourGetProgramUptimeCallback;
}

MVREXPORT MvrRetFunctor<int>* MvrSystemStatus::getWirelessLinkQualityFunctor() {
	return &ourGetWirelessLinkQualityCallback;
}
MVREXPORT MvrRetFunctor<int>* MvrSystemStatus::getWirelessLinkNoiseFunctor() {
	return &ourGetWirelessLinkNoiseCallback;
}
MVREXPORT MvrRetFunctor<int>* MvrSystemStatus::getWirelessLinkSignalFunctor() {
	return &ourGetWirelessLinkSignalCallback;
}

MVREXPORT MvrRetFunctor<int>* MvrSystemStatus::getMTXWirelessLinkFunctor() {
	return &ourGetMTXWirelessLinkCallback;
}
MVREXPORT MvrRetFunctor<int>* MvrSystemStatus::getMTXWirelessQualityFunctor() {
	return &ourGetMTXWirelessQualityCallback;
}

// Get wireless stats from /proc/net/wireless:

void MvrSystemStatus::refreshWireless()
{
#ifndef WIN32
	if (ourPeriodicUpdateThread && !ourShouldRefreshWireless) return;
	FILE* fp = MvrUtil::fopen("/proc/net/wireless", "r");
	if (!fp)
	{
		MvrLog::log(MvrLog::Terse, "MvrSystemStatus: Error: Failed to open /proc/net/wireless!");
		ourShouldRefreshWireless = false;
		return;
	}

	// first two lines are header info
	char line[256];
	if (!(fgets(line, 256, fp) && fgets(line, 256, fp)))
	{
		fclose(fp);
		ourLinkQuality = ourLinkSignal = ourLinkNoise =
			ourDiscardedTotal = ourDiscardedDecrypt = -1;
		ourShouldRefreshWireless = false;
		return;
	}


	// next line is info for first device
	char id[32];
	unsigned int stat;
	int disc_frag, disc_retry, disc_misc, missed;
	disc_frag = disc_retry = disc_misc = missed = 0;
	int r = fscanf(fp, "%31s %x %d. %d. %d. %d %d %d %d %d %d",
		id, &stat,
		&ourLinkQuality, &ourLinkSignal, &ourLinkNoise,
		&ourDiscardedConflict, &ourDiscardedDecrypt,
		&disc_frag, &disc_retry, &disc_misc, &missed);
	fclose(fp);
	if (r < 11)
		MvrLog::log(MvrLog::Verbose, "MvrSystemStatus: Warning: Failed to parse /proc/net/wireless (only %d out of 11 values parsed).", r);
	if (ourDiscardedConflict == -1 || ourDiscardedDecrypt == -1)
		ourDiscardedTotal = -1;
	else
		ourDiscardedTotal = ourDiscardedConflict + ourDiscardedDecrypt
		+ disc_frag + disc_retry + disc_misc;
	ourShouldRefreshWireless = false;
#endif // WIN32
}


// Get wireless stats from /var/robot/status/network/wireless

void MvrSystemStatus::refreshMTXWireless()
{
#ifndef WIN32
	if (ourPeriodicUpdateThread && !ourShouldRefreshMTXWireless) return;
	FILE* fpIp = MvrUtil::fopen("/mnt/status/network/wireless/ip", "r");
	FILE* fpLink = MvrUtil::fopen("/mnt/status/network/wireless/link", "r");
	FILE* fpQuality = MvrUtil::fopen("/mnt/status/network/wireless/quality", "r");

	if ((!fpIp) || (!fpLink) || (!fpQuality))
	{
		MvrLog::log(MvrLog::Terse, "MvrSystemStatus: Error: Failed to open /mnt/status/network/wireless/ files!");
		ourShouldRefreshMTXWireless = false;
		return;
	}

#if 0
	// grab the data from each file
	char lineIp[256];
	if (!(fgets(lineIp, 256, fpIp)))
	{
		fclose(fpIp);
		ourMTXIP1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourMTXWirelessLink = -1;
		ourMTXWirelessQuality = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}
#endif


	// ?? - need to store IP somewhere don't know if we need it

	if (!(fscanf(fpLink, "%d", &ourMTXWirelessLink)))
	{
		fclose(fpLink);
		fclose(fpIp);
		ourMTXIp1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourMTXWirelessLink = -1;
		ourMTXWirelessQuality = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}


	if (!((fscanf(fpQuality, "%d", &ourMTXWirelessQuality))))
	{
		fclose(fpQuality);
		fclose(fpLink);
		fclose(fpIp);
		ourMTXIp1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourMTXWirelessQuality = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}

	if (!(fscanf(fpIp, "%d.%d.%d.%d", &ourMTXIp1, &ourMTXIp2, &ourMTXIp3, &ourMTXIp4)))
	{
		fclose(fpIp);
		ourMTXIp1 = ourMTXIp2 = ourMTXIp3 = ourMTXIp4 = -1;
		ourShouldRefreshMTXWireless = false;
		return;
	}


	/*
	MvrLog::log(MvrLog::Normal, "MvrSystemStatus: %d.%d.%d.%d %d %d ",
	ourMTXIp1, ourMTXIp2, ourMTXIp3, ourMTXIp4, ourMTXLinkQuality, ourMTXLinkSignal);
	*/

	char buf[1024];
	sprintf(buf, "%d.%d.%d.%d", ourMTXIp1, ourMTXIp2, ourMTXIp3, ourMTXIp4);
	ourMTXIpString = buf;

	fclose(fpQuality);
	fclose(fpLink);
	fclose(fpIp);
	ourShouldRefreshMTXWireless = false;
#endif // WIN32
}

MVREXPORT int MvrSystemStatus::getWirelessLinkQuality() {
	MvrScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourLinkQuality;
}

MVREXPORT int MvrSystemStatus::getWirelessLinkSignal() {
	MvrScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourLinkSignal;
}

MVREXPORT int MvrSystemStatus::getWirelessLinkNoise() {
	MvrScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourLinkNoise;
}

MVREXPORT int MvrSystemStatus::getWirelessDiscardedPackets() {
	MvrScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourDiscardedTotal;
}

MVREXPORT int MvrSystemStatus::getWirelessDiscardedPacketsBecauseNetConflict() {
	MvrScopedLock lock(ourWirelessMutex);
	refreshWireless();
	return ourDiscardedConflict;
}

MVREXPORT int MvrSystemStatus::getMTXWirelessLink() {
	MvrScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXWirelessLink;
}

MVREXPORT int MvrSystemStatus::getMTXWirelessQuality() {
	MvrScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXWirelessQuality;
}

MVREXPORT int MvrSystemStatus::getMTXWirelessIpAddress1() {
	MvrScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp1;
}

MVREXPORT int MvrSystemStatus::getMTXWirelessIpAddress2() {
	MvrScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp2;
}
MVREXPORT int MvrSystemStatus::getMTXWirelessIpAddress3() {
	MvrScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp3;
}

MVREXPORT int MvrSystemStatus::getMTXWirelessIpAddress4() {
	MvrScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIp4;
}

MVREXPORT const char * MvrSystemStatus::getMTXWirelessIpAddressString() {
	MvrScopedLock lock(ourMTXWirelessMutex);
	refreshMTXWireless();
	return ourMTXIpString.c_str();
}

MVREXPORT void MvrSystemStatus::invalidate()
{
	MvrScopedLock lockc(ourCPUMutex);
	MvrScopedLock lockw(ourWirelessMutex);
	ourShouldRefreshCPU = true;
	ourShouldRefreshWireless = true;
	ourShouldRefreshMTXWireless = true;
}

