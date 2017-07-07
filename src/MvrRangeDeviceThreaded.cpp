#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRangeDeviceThreaded.h"

MVREXPORT MvrRangeDeviceThreaded::MvrRangeDeviceThreaded(
	size_t currentBufferSize, size_t cumulativeBufferSize,
	const char *name, unsigned int maxRange,
	int maxSecondsToKeepCurrent, int maxSecondsToKeepCumulative,
	double maxDistToKeepCumulative, bool locationDependent) :
  MvrRangeDevice(currentBufferSize, cumulativeBufferSize, name, maxRange,
		maxSecondsToKeepCurrent, maxSecondsToKeepCumulative, 
		maxDistToKeepCumulative, locationDependent),
  myRunThreadCB(this, &MvrRangeDeviceThreaded::runThread),
  myTask(&myRunThreadCB)
{
  myTask.setThreadName(name);
}

MVREXPORT MvrRangeDeviceThreaded::~MvrRangeDeviceThreaded()
{
}
