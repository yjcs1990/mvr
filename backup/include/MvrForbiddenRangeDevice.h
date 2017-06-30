/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrForbiddenRangeDevice.h
 > Description  : Class that takes forbidden lines and turns them into range readings
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRFORBIDDENRANGEDEVICE_H
#define MVRFORBIDDENRANGEDEVICE_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrMapInterface.h"

/// Class that takes forbidden lines and turns them into range readings
///  @ingroup OptionalClasses
class MvrForbiddenRangeDevice : public MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrForbiddenRangeDevice(MvrMapInterface *armap, 
                                    double distanceIncrement = 100,
				                    unsigned int maxRange = 4000,
				                    const char *name = "forbidden");
  /// Destructor
  MVREXPORT virtual ~MvrForbiddenRangeDevice();
  /// Saves the forbidden lines from the map
  MVREXPORT void processMap(void);
  /// Remakes the readings 
  MVREXPORT void processReadings(void);
  /// Sets the robot pointer and attaches its process function
  MVREXPORT virtual void setRobot(MvrRobot *robot);

  /// Enable readings 
  MVREXPORT void enable(void);
  /// Disables readings until reenabled
  MVREXPORT void disable(void);
  /// Sees if this device is active or not
  MVREXPORT bool isEnabled(void) const { return myIsEnabled;; }
  /// Gets a callback to enable the device
  MVREXPORT MvrFunctor *getEnableCB(void) { return &myEnableCB; } 
  /// Gets a callback to disable the device
  MVREXPORT MvrFunctor *getDisableCB(void) { return &myDisableCB; } 
protected:
  MvrMutex myDataMutex;
  MvrMapInterface *myMap;
  double myDistanceIncrement;
  std::list<MvrLineSegment *> mySegments;
  MvrFunctorC<MvrForbiddenRangeDevice> myProcessCB;
  MvrFunctorC<MvrForbiddenRangeDevice> myMapChangedCB;
  bool myIsEnabled;
  MvrFunctorC<MvrForbiddenRangeDevice> myEnableCB;
  MvrFunctorC<MvrForbiddenRangeDevice> myDisableCB;
};

#endif // MVRFORBIDDENRANGEDEVICE_H