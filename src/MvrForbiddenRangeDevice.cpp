#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRobot.h"
#include "mvriaUtil.h"
#include "MvrForbiddenRangeDevice.h"

/**
   This will take a map and then convert the forbidden lines into
   range device readings every cycle.

   It doesn't add any data for drawing the current range device
   readings since the lines themselves show up in the map.

   @param armap the map to use forbidden lines from
   @param distanceIncrement how many mm to increment search in sensor readings for forbidden lines
   @param maxRange how far out to look total
   @param name the name of the device
 **/
MVREXPORT MvrForbiddenRangeDevice::MvrForbiddenRangeDevice(
	MvrMapInterface *armap, double distanceIncrement, unsigned int maxRange,
	const char *name) :
  MvrRangeDevice(INT_MAX, 0, name, maxRange, 0, 0, 0, true),
  myDataMutex(),
  myMap(armap),
  myDistanceIncrement(distanceIncrement),
  mySegments(),
  myProcessCB(this, &MvrForbiddenRangeDevice::processReadings),
  myMapChangedCB(this, &MvrForbiddenRangeDevice::processMap)  ,
  myIsEnabled(true),
  myEnableCB(this, &MvrForbiddenRangeDevice::enable),
  myDisableCB(this, &MvrForbiddenRangeDevice::disable)
{
  myDataMutex.setLogName("MvrForbiddenRangeDevice::myDataMutex");
  
  myMapChangedCB.setName("MvrForbiddenRangeDevice");
}

MVREXPORT MvrForbiddenRangeDevice::~MvrForbiddenRangeDevice()
{

}

MVREXPORT void MvrForbiddenRangeDevice::processMap(void)
{
  std::list<MvrMapObject *>::const_iterator it;
  MvrMapObject *obj;

  myDataMutex.lock();
  MvrUtil::deleteSet(mySegments.begin(), mySegments.end());
  mySegments.clear();

  for (it = myMap->getMapObjects()->begin();
       it != myMap->getMapObjects()->end();
       it++)
  {
    obj = (*it);
    if (strcmp(obj->getType(), "ForbiddenLine") == 0 &&
	obj->hasFromTo())
    {
      mySegments.push_back(new MvrLineSegment(obj->getFromPose(), 
					     obj->getToPose()));
    }
    if (strcmp(obj->getType(), "ForbiddenArea") == 0 &&
	obj->hasFromTo())
    {
      double angle = obj->getPose().getTh();
      double sa = MvrMath::sin(angle);
      double ca = MvrMath::cos(angle);
      double fx = obj->getFromPose().getX();
      double fy = obj->getFromPose().getY();
      double tx = obj->getToPose().getX();
      double ty = obj->getToPose().getY();
      MvrPose P0((fx*ca - fy*sa), (fx*sa + fy*ca));
      MvrPose P1((tx*ca - fy*sa), (tx*sa + fy*ca));
      MvrPose P2((tx*ca - ty*sa), (tx*sa + ty*ca));
      MvrPose P3((fx*ca - ty*sa), (fx*sa + ty*ca));
      mySegments.push_back(new MvrLineSegment(P0, P1));
      mySegments.push_back(new MvrLineSegment(P1, P2));
      mySegments.push_back(new MvrLineSegment(P2, P3));
      mySegments.push_back(new MvrLineSegment(P3, P0));
    }
  }
  myDataMutex.unlock();
}

MVREXPORT void MvrForbiddenRangeDevice::processReadings(void)
{
  MvrPose intersection;
  std::list<MvrLineSegment *>::iterator it;
  
  lockDevice();
  myDataMutex.lock();

  myCurrentBuffer.beginRedoBuffer();

  if (!myIsEnabled)
  {
    myCurrentBuffer.endRedoBuffer();
    myDataMutex.unlock();
    unlockDevice();
    return;
  }

  MvrLineSegment *segment;
  MvrPose start;
  double startX;
  double startY;
  MvrPose end;
  double angle;
  double length;
  double gone;
  double sin;
  double cos;
  double atX;
  double atY;
  double robotX = myRobot->getX();
  double robotY = myRobot->getY();
  double max = (double) myMaxRange;
  double maxSquared = (double) myMaxRange * (double) myMaxRange;
  MvrTime startingTime;
  //startingTime.setToNow();
  // now see if the end points of the segments are too close to us
  for (it = mySegments.begin(); it != mySegments.end(); it++)
  {
    segment = (*it);
    // if either end point or some perpindicular point is close to us
    // add the line's data
    if (MvrMath::squaredDistanceBetween(
	    segment->getX1(), segment->getY1(), 
	    myRobot->getX(), myRobot->getY()) < maxSquared ||
	MvrMath::squaredDistanceBetween(
		segment->getX2(), segment->getY2(), 
		myRobot->getX(), myRobot->getY()) < maxSquared ||
	segment->getPerpDist(myRobot->getPose()) < max)
    {
      start.setPose(segment->getX1(), segment->getY1());
      end.setPose(segment->getX2(), segment->getY2());
      angle = start.findAngleTo(end);
      cos = MvrMath::cos(angle);
      sin = MvrMath::sin(angle);
      startX = start.getX();
      startY = start.getY();
      length = start.findDistanceTo(end);
      // first put in the start point if we should
      if (MvrMath::squaredDistanceBetween(
	      startX, startY, robotX, robotY) < maxSquared)
	myCurrentBuffer.redoReading(start.getX(), start.getY());
      // now walk the length of the line and see if we should put the points in
      for (gone = 0; gone < length; gone += myDistanceIncrement)
      {
	atX = startX + gone * cos;
	atY = startY + gone * sin;
	if (MvrMath::squaredDistanceBetween(
		atX, atY, robotX, robotY) < maxSquared)
	  myCurrentBuffer.redoReading(atX, atY);
      }
      // now check the end point
      if (end.squaredFindDistanceTo(myRobot->getPose()) < maxSquared)
	myCurrentBuffer.redoReading(end.getX(), end.getY());
    }
  }
  myDataMutex.unlock();
  // and we're done
  myCurrentBuffer.endRedoBuffer();
  unlockDevice();
  //printf("%d\n", startingTime.mSecSince());
}

MVREXPORT void MvrForbiddenRangeDevice::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 20, &myProcessCB);
  MvrRangeDevice::setRobot(robot);
  myMap->lock();
  myMap->addMapChangedCB(&myMapChangedCB);
  processMap();
  myMap->unlock();
}

MVREXPORT void MvrForbiddenRangeDevice::enable(void)
{
  myDataMutex.lock();
  myIsEnabled = true;
  myDataMutex.unlock();
}

MVREXPORT void MvrForbiddenRangeDevice::disable(void)
{
  myDataMutex.lock();
  myIsEnabled = false;
  myDataMutex.unlock();
}
