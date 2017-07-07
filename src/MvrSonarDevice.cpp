#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSonarDevice.h"
#include "MvrSensorReading.h"
#include "MvrRobot.h"

MVREXPORT MvrSonarDevice::MvrSonarDevice(size_t currentBufferSize,
			     size_t cumulativeBufferSize, const char *name) :
  MvrRangeDevice(currentBufferSize, cumulativeBufferSize, name, 5000), 
  myProcessCB(this, &MvrSonarDevice::processReadings),
  myIgnoreReadingCB(NULL)
{
  // Remove readings from cumulative buffer if far away from robot after it
  // moves:
  setMaxDistToKeepCumulative(3000); 

  // Do our own filtering of readings that are closer than 50 mm  to each other from 
  //cumulative buffer between cumulative readings, at least:
  myFilterNearDist = 50;

  // Do our own filtering of cumulative readings this far from robot:
  myFilterFarDist = 3000;

  // Remove readings from current buffer if more than 5 seconds old:
  setMaxSecondsToKeepCurrent(5);

  // Remove readings from cumulatiive buffer if more than 15 seconds old:
  setMaxSecondsToKeepCumulative(15);

  // Visualization properties for GUI clients such as MobileEyes:
  setCurrentDrawingData(new MvrDrawingData("polyArrows", 
                                          MvrColor(0x33, 0xCC, 0xFF), 
                                          200,  // mm length of arrow
                                          70),  // first sensor layer
                        true);
}

MVREXPORT MvrSonarDevice::~MvrSonarDevice()
{
  if (myRobot != NULL)
  {
    myRobot->remSensorInterpTask(&myProcessCB);
    myRobot->remRangeDevice(this);
  }
}

MVREXPORT void MvrSonarDevice::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 10, &myProcessCB);
  MvrRangeDevice::setRobot(robot);
}

MVREXPORT void MvrSonarDevice::processReadings(void)
{
  int i;
  MvrSensorReading *reading;
  lockDevice();

  for (i = 0; i < myRobot->getNumSonar(); i++)
  {
    // Get a reference to the MvrSensorReading object for sonar sensor #i, which
    // will provide an X,Y position and timestamp for the most recently received
    // obstacle detected by that sonar sensor. MvrRobot created a set of 
    // MvrSensorReading objects for each sonar sensor  when it connected to the 
    // robot, configured them with the position of the sonar sensor relative to the
    // center of the robot. It calls MvrSensorReading::newData() when new sonar
    // data is received to cause the MvrSensorReading object to update the X,Y
    // position based on the range value received from the sonar sensor.

    reading = myRobot->getSonarReading(i);
    if (reading == NULL || !reading->isNew(myRobot->getCounter()))
      continue;
    // make sure we don't want to ignore the reading
    if (myIgnoreReadingCB == NULL || !myIgnoreReadingCB->invokeR(reading->getPose()))
      addReading(reading->getX(), reading->getY());
  }

  // delete too-far readings
  std::list<MvrPoseWithTime *> *readingList;
  std::list<MvrPoseWithTime *>::iterator it;
  double dx, dy, rx, ry;
    
  myCumulativeBuffer.beginInvalidationSweep();
  readingList = myCumulativeBuffer.getBuffer();
  rx = myRobot->getX();
  ry = myRobot->getY();
  // walk through the list and see if this makes any old readings bad
  if (readingList != NULL)
    {
      for (it = readingList->begin(); it != readingList->end(); ++it)
	{
	  dx = (*it)->getX() - rx;
	  dy = (*it)->getY() - ry;
	  if ((dx*dx + dy*dy) > (myFilterFarDist * myFilterFarDist)) 
	    myCumulativeBuffer.invalidateReading(it);
	}
    }
  myCumulativeBuffer.endInvalidationSweep();
  // leave this unlock here or the world WILL end
  unlockDevice();
}

/**
   Adds a sonar reading with the global coordinates x,y.  Makes sure the
   reading is within the proper distance to the robot, for
   both current and cumulative buffers.  Filters buffer points 
   Note: please lock the device using lockDevice() / unlockDevice() if
   calling this from outside process().
   @param x the global x coordinate of the reading
   @param y the global y coordinate of the reading
*/
MVREXPORT void MvrSonarDevice::addReading(double x, double y)
{
  double rx = myRobot->getX();
  double ry = myRobot->getY();
  double dx = x - rx;		
  double dy = y - ry;
  double dist2 = dx*dx + dy*dy;
  
  if (dist2 < myMaxRange*myMaxRange)
    myCurrentBuffer.addReading(x,y);
  
  if (dist2 < myMaxDistToKeepCumulative * myMaxDistToKeepCumulative)
    {
      std::list<MvrPoseWithTime *> *readingList;
      std::list<MvrPoseWithTime *>::iterator it;

      myCumulativeBuffer.beginInvalidationSweep();

      readingList = myCumulativeBuffer.getBuffer();
      // walk through the list and see if this makes any old readings bad
      if (readingList != NULL)
	{
	  for (it = readingList->begin(); it != readingList->end(); ++it)
	    {
	      dx = (*it)->getX() - x;
	      dy = (*it)->getY() - y;
	      if ((dx*dx + dy*dy) < (myFilterNearDist * myFilterNearDist)) 
		myCumulativeBuffer.invalidateReading(it);
	    }
	}
      myCumulativeBuffer.endInvalidationSweep();

      myCumulativeBuffer.addReading(x,y);
    }
}


MVREXPORT void MvrSonarDevice::setIgnoreReadingCB(
	MvrRetFunctor1<bool, MvrPose> *ignoreReadingCB)
{
  lockDevice();
  myIgnoreReadingCB = ignoreReadingCB;
  unlockDevice();
}
