/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSonarDevice.h
 > Description  : Keep track of recent sonar readings from a robot as an MvrRangeDevice
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRSONARDEVICE_H
#define MVRSONARDEVICE_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctor.h"

#include "MvrRobot.h"
/** 
    This class is for keeping a sonar history, which you may use for obstacle 
    avoidance, display, etc.
    Simply use MvrRobot::addRangeDevice()  (or MvrSonarDevice::setRobot())
    to attach an MvrSonarDevice object to an MvrRobot
    robot object; MvrSonarDevice will add a Sensor Interpretation task to the
    MvrRobot which will read new sonar readings each robot cycle and add
    them to its sonar history.

    @ingroup ImportantClasses
    @ingroup DeviceClasses
*/
class MvrSonarDevice : public MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrSonarDevice(size_t currentBufferSize=24,
                           size_t cumulativeBufferSize=64,
                           const char *name="sonar");
  /// Destructor
  MVREXPORT virtual ~MvrSonarDevice();
  /// Grabs the new readings from the robot and adds them to the buffers
  MVREXPORT void processReadings(void);
  /// Sets the robot pointer, also attaches its process functions to the
  /// robot as a Sensor Interpretation task
  MVREXPORT virtual void setRobot(MvrRobot *robot);

  /// Adds sonar reading to the current and cumulative buffers
  MVREXPORT virtual void addReading(double x, double y);

  /// Sets a callback which if it returns true will ignore the reading
  MVREXPORT void setIgnoreReadingCB(MvrRetFunctor1<bool, MvrPose> *ignoreReadingCB);

  /// Gets the callback which if it returns true will ignore the reading
  MVREXPORT MvrRetFunctor1<bool, MvrPose> *getIgnoreReadingCB(void)
  { return myIgnoreReadingCB; }

  ///@deprecated
  MVREXPORT void setCumulativeMaxRange(double range)
  { setMaxDistToKeepCumulative(range); }
protected:
  MvrFunctorC<MvrSonarDevice> myProcessCB;
  double myFilterNearDist;      /// we throw out cumulative readings this close to current one
  double myFilterFarDist;       /// we throw out cumulative reading beyond this far from robot

  MvrRetFunctor1<bool,MvrPose> *myIgnoreReadingCB;
};
#endif  // MVRSONARDEVICE_H