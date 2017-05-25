/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrIRs.h
 > Description  : A class that treats a robot's infrared sensors as a range device.
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef ARIRS_H
#define ARIRS_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"


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


#endif // ARIRS_H
