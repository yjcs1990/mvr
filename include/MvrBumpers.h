#ifndef MVRBUMPERS_H
#define MVRBUMPERS_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"


/// A class that treats the robot's bumpers as a range device.
/**
   The class treats bumpers like a range device.  When a bumper
   is bumped, it reports the approximate position of the bump
   in a buffer.  The positions are kept current for a specified 
   length of time.

   @ingroup DeviceClasses
*/
class MvrBumpers : public MvrRangeDevice
{
public:
  MVREXPORT MvrBumpers(size_t currentBufferSize = 30, 
		     size_t cumulativeBufferSize = 30,
		     const char *name = "bumpers",
		     int maxSecondsToKeepCurrent = 15,
		     double angleRange = 135);
  MVREXPORT virtual ~MvrBumpers(void);

  MVREXPORT virtual void setRobot(MvrRobot *robot);
  MVREXPORT void processReadings(void);
  MVREXPORT void addBumpToBuffer(int bumpValue, int whichBumper);

protected:
  MvrFunctorC<MvrBumpers> myProcessCB;
  MvrRobot *myRobot;
  int myBumpMask;
  double myAngleRange;
};


#endif // ARBUMPERS_H
