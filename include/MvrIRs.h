#ifndef MVRIRS_H
#define MVRIRS_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"


/// A class that treats a robot's infrared sensors as a range device.
/**
  (Only Peoplebot and some Powerbots have IR sensors.)

  @ingroup OptionalClasses  
  @ingroup DeviceClasses
*/
class MvrIRs : public MvrRangeDevice
{
public:
  MVREXPORT MvrIRs(size_t currentBufferSize = 10, 
		     size_t cumulativeBufferSize = 10,
		     const char *name = "irs",
		     int maxSecondsToKeepCurrent = 15);
  MVREXPORT virtual ~MvrIRs(void);

  MVREXPORT virtual void setRobot(MvrRobot *robot);
  MVREXPORT void processReadings(void);

protected:
  MvrFunctorC<MvrIRs> myProcessCB;
  MvrRobotParams myParams;
  std::vector<int> cycleCounters;
};


#endif // MVRIRS_H
