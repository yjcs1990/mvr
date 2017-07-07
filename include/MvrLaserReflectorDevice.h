#ifndef MVRLASERREFLECTORDEVICE_H
#define MVRLASERREFLECTORDEVICE_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctor.h"

class MvrSick;
class MvrRobot;

/// A class for keeping track of laser reflectors that we see right now
/** 
    This class is for showing the laser reflectors in MobileEyes.
    This requires that the range device you pass in uses the
    'extraInt' in the rawReadings MvrSensorReading to note reflector
    value and that anything greater than 0 is a reflector.
*/
class MvrLaserReflectorDevice : public MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrLaserReflectorDevice(MvrRangeDevice *laser, MvrRobot *robot,
				  const char * name = "reflector");
  /// Destructor
  MVREXPORT virtual ~MvrLaserReflectorDevice();
  /// Grabs the new readings from the robot and adds them to the buffers
  MVREXPORT void processReadings(void);
  /// Specifically does nothing since it was done in the constructor
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  /// Adds a reflector threshold to the task
  MVREXPORT void addToConfig(MvrConfig *config, const char *section);
protected:
  MvrRangeDevice *myLaser;
  int myReflectanceThreshold;
  MvrFunctorC<MvrLaserReflectorDevice> myProcessCB;
};


#endif // ARLASERREFLECTORDEVICE_H
