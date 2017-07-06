/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRangeDevice.h"
#include "MvrRobot.h"

/**
   @param currentBufferSize number of readings to store in the current
   buffer

   @param cumulativeBufferSize number of readings to store in the
   cumulative buffer

   @param name the name of this device

   @param maxRange the maximum range of this device. If the device
   can't find a reading in a specified section, it returns this
   maxRange

   @param maxSecondsToKeepCurrent this is the number of seconds to
   keep current readings in the current buffer. If less than 0, then
   readings are not automatically removed based on time (but can be
   replaced or removed for other reasons). If 0, readings are removed
   immediately when a new set of readings is received and placed in the current buffer.

   @param maxSecondsToKeepCumulative this is the number of seconds to
   keep cumulative readings in the cumulative buffer. If less than 0
   then readings are not automatically based on time (but can be
   replaced or removed for other reasons).
   
   @param maxDistToKeepCumulative if cumulative readings are further
   than this distance from the current robot pose, then they are
   removed. If this is less than 0 they are not removed because of
   this

   @param locationDependent if the data in this range device is
   dependent on the robot's location or not...  For instance, a laser
   would not be dependent on location, because it'll be correct in a
   relative manner, whereas forbidden lines are dependent on location,
   because if the robot isn't where it thinks it is then the forbidden
   lines will be avoided in the wrong spots... this is mainly just a
   flag for other things to use when deciding what range devices to
   avoid
   
**/
MVREXPORT MvrRangeDevice::MvrRangeDevice(size_t currentBufferSize,
				      size_t cumulativeBufferSize, 
				      const char *name, 
				      unsigned int maxRange,
				      int maxSecondsToKeepCurrent, 
				      int maxSecondsToKeepCumulative,
				      double maxDistToKeepCumulative,
				      bool locationDependent) :
  myCurrentBuffer(currentBufferSize),
  myCumulativeBuffer(cumulativeBufferSize),
  myFilterCB(this, &MvrRangeDevice::filterCallback)
{
  myDeviceMutex.setLogName("MvrRangeDevice::myDeviceMutex");
  myRobot = NULL;
  myName = name;
  myMaxRange = maxRange;
  myRawReadings = NULL;
  myAdjustedRawReadings = NULL;

  // take out any spaces in the name since that'll break things
  int i;
  int len = myName.size();
  for (i = 0; i < len; i++)
  {
    if (isspace(myName[i]))
      myName[i] = '_';
  }

  setMaxSecondsToKeepCurrent(maxSecondsToKeepCurrent);
  setMinDistBetweenCurrent(0);
  setMaxSecondsToKeepCumulative(maxSecondsToKeepCumulative);
  setMaxDistToKeepCumulative(maxDistToKeepCumulative);
  setMinDistBetweenCumulative(0);
  setMaxInsertDistCumulative(0);

  myCurrentDrawingData = NULL;
  myOwnCurrentDrawingData = false;
  myCumulativeDrawingData = NULL;
  myOwnCumulativeDrawingData = false;
  myIsLocationDependent = locationDependent;

  //setMinDistBetweenCurrent();
  //setMinDistBetweenCumulative();
}

MVREXPORT MvrRangeDevice::~MvrRangeDevice()
{
  if (myRobot != NULL)
    myRobot->remSensorInterpTask(&myFilterCB);

  if (myCurrentDrawingData != NULL && myOwnCurrentDrawingData)
  {
    delete myCurrentDrawingData;
    myCurrentDrawingData = NULL;
    myOwnCurrentDrawingData = false;
  }
  if (myCumulativeDrawingData != NULL && myOwnCumulativeDrawingData)
  {
    delete myCumulativeDrawingData;
    myCumulativeDrawingData = NULL;
    myOwnCumulativeDrawingData = false;
  }
}


MVREXPORT const char * MvrRangeDevice::getName(void) const
{ 
  return myName.c_str(); 
}

MVREXPORT void MvrRangeDevice::setRobot(MvrRobot *robot) 
{ 
  char buf[512];
  sprintf(buf, "filter %s", getName());

  if (myRobot != NULL)
    myRobot->remSensorInterpTask(&myFilterCB);

  myRobot = robot;

  if (myRobot != NULL)
    myRobot->addSensorInterpTask(buf, 100, &myFilterCB);
}

MVREXPORT MvrRobot *MvrRangeDevice::getRobot(void) 
{
  return myRobot; 
}

MVREXPORT void MvrRangeDevice::filterCallback(void)
{
  std::list<MvrPoseWithTime *>::iterator it;
  lockDevice();

  myMaxInsertDistCumulativePose = myRobot->getPose();
  
  // first filter the current readings based on time
  if (myMaxSecondsToKeepCurrent > 0 && 
      myCurrentBuffer.getSize() > 0)
  {
    // just walk through and make sure nothings too far away
    myCurrentBuffer.beginInvalidationSweep();
    for (it = getCurrentBuffer()->begin(); 
	 it != getCurrentBuffer()->end(); 
	 ++it)
    {
      if ((*it)->getTime().secSince() >= myMaxSecondsToKeepCurrent)
	myCurrentBuffer.invalidateReading(it);
    }
    myCurrentBuffer.endInvalidationSweep();
  }
  
  if (myCumulativeBuffer.getSize() == 0)
  {
    unlockDevice();
    return;
  }

  // okay done with current, now do the cumulative
  bool doingDist = true;
  bool doingAge = true;

  if (myMaxDistToKeepCumulativeSquared < 1)
    doingDist = false;
  if (myMaxSecondsToKeepCumulative <= 0)
    doingAge = false;
		    
  if (!doingDist && !doingAge)
  {
    unlockDevice();
    return;
  }

  // just walk through and make sure nothings too far away
  myCumulativeBuffer.beginInvalidationSweep();
  for (it = getCumulativeBuffer()->begin(); 
       it != getCumulativeBuffer()->end(); 
       ++it)
  {
    // if its closer to a reading than the filter near dist, just return
    if (doingDist && 
	(myRobot->getPose().squaredFindDistanceTo(*(*it)) > 
	 myMaxDistToKeepCumulativeSquared))
      myCumulativeBuffer.invalidateReading(it);
    else if (doingAge && 
	     (*it)->getTime().secSince() >= myMaxSecondsToKeepCumulative)
      myCumulativeBuffer.invalidateReading(it);
  }
  myCumulativeBuffer.endInvalidationSweep();
  unlockDevice();
}

/**
   If the @a size is smaller than the current buffer size, then 
   the oldest readings are discarded, leaving only @a size
   newest readings. If @a size is larger than the current size,
   then the buffer size will be allowed to grow to that size as new readings
   are added.
   @param size number of readings to set the buffer's maximum size to
*/
MVREXPORT void MvrRangeDevice::setCurrentBufferSize(size_t size)
{
  myCurrentBuffer.setSize(size);
}

MVREXPORT size_t MvrRangeDevice::getCurrentBufferSize(void) const
{
  return myCurrentBuffer.getSize();
}

/**
   If the @a size is smaller than the cumulative buffer size, then 
   the oldest readings are discarded, leaving only @a size
   newest readings. If @a size is larger than the cumulative buffer size,
   then the buffer size will be allowed to grow to that size as new readings
   are added.
   @param size number of readings to set the buffer to
*/
MVREXPORT void MvrRangeDevice::setCumulativeBufferSize(size_t size)
{
  myCumulativeBuffer.setSize(size);
}


MVREXPORT size_t MvrRangeDevice::getCumulativeBufferSize(void) const
{
  return myCumulativeBuffer.getSize();
}

MVREXPORT void MvrRangeDevice::addReading(double x, double y, bool *wasAdded)
{
  myCurrentBuffer.addReadingConditional(x, y, 
					myMinDistBetweenCurrentSquared,
					wasAdded);

  // make sure we have a cumulative buffer 
  if (myCumulativeBuffer.getSize() == 0)
    return;
  
  // see if we're doing a max distance
  if (myRobot != NULL && myMaxInsertDistCumulativeSquared > 0 && 
      MvrMath::squaredDistanceBetween(
	      myMaxInsertDistCumulativePose.getX(), 
	      myMaxInsertDistCumulativePose.getY(), 
	      x, y) > myMaxInsertDistCumulativeSquared)
    return;
    
  myCumulativeBuffer.addReadingConditional(
	  x, y, myMinDistBetweenCumulativeSquared);
}

/**
 * The closest reading within a polar region (sector or slice) defined by the given
 * angle range is returned.  Optionally, the specific angle of the found may be
 * placed in @a angle, if not NULL.
 * The region searched is the region between @a startAngle, sweeping
 * counter-clockwise to @a endAngle (0 is straight ahead of the device,
 * -90 to the right, 90 to the left).  Note that therefore there is a difference between
 *  e.g. the regions (0, 10) and (10, 0).  (0, 10) is a 10-degree span near the front 
 *  of the device, while (10, 0) is a 350 degree span covering the sides and
 *  rear.  Similarly, (-60, -30) covers 30 degrees on the right hand side, while
 *  (-30, -60) covers 330 degrees.   (-90, 90) is 180 degrees in front. (-180,
 *  180) covers all sides of the robot.
 *  In other words, if you want the smallest
 *  section between the two angles, ensure that @a startAngle < @a endAngle.
 *
   @param startAngle where to start the slice
   @param endAngle where to end the slice, going counterclockwise from startAngle
   @param angle if given, a pointer to a value in which to put the specific angle to the found reading
   @return the range to the obstacle (a value >= the maximum range indicates that no reading was detected in the specified region)

   @python @a angle is ignored

  Example:
   @image html MvrRangeDevice_currentReadingPolar.png This figure illustrates an example range device and the meanings of arguments and return value.
*/
MVREXPORT double MvrRangeDevice::currentReadingPolar(double startAngle,
						   double endAngle,
						   double *angle) const
{
  MvrPose pose;
  if (myRobot != NULL)
    pose = myRobot->getPose();
  else
    {
      MvrLog::log(MvrLog::Normal, "MvrRangeDevice %s: NULL robot, won't get polar reading correctly", getName());
      pose.setPose(0, 0);
    }
  return myCurrentBuffer.getClosestPolar(startAngle, endAngle, 
					 pose,
					 myMaxRange,
					 angle);
}

/**
 * The closest reading in this range device's cumulative buffer
 * within a polar region or "slice" defined by the given
 * angle range is returned.  Optionally return the specific angle of the found reading as
 * well. The region searched is the region between a starting angle, sweeping
 * counter-clockwise to the ending angle (0 is straight ahead of the device,
 * -90 to the right, 90 to the left).  Note that there is a difference between
 *  the region (0, 10) and (10, 0).  (0, 10) is a 10-degree span near the front 
 *  of the device, while (10, 0) is a 350 degree span covering the sides and
 *  rear.  Similarly, (-60, -30) covers 30 degrees on the right hand side, while
 *  (-30, -60) covers 330 degrees.
 *  In other words, if you want the smallest
 *  section between the two angles, ensure than startAngle < endAngle.
   @param startAngle where to start the slice
   @param endAngle where to end the slice, going counterclockwise from startAngle
   @param angle if given, a pointer to a value in which to put the specific angle to the found reading
   @return the range to the obstacle (a value >= the maximum range indicates that no reading was detected in the specified region)

   @python @a angle is ignored

  Example:
   @image html MvrRangeDevice_currentReadingPolar.png This figure illustrates an example range device and the meanings of arguments and return value.
*/
MVREXPORT double MvrRangeDevice::cumulativeReadingPolar(double startAngle,
						      double endAngle,
						      double *angle) const
{
  MvrPose pose;
  if (myRobot != NULL)
    pose = myRobot->getPose();
  else
    {
      MvrLog::log(MvrLog::Normal, "MvrRangeDevice %s: NULL robot, won't get polar reading correctly", getName());
      pose.setPose(0, 0);
    }
  return myCumulativeBuffer.getClosestPolar(startAngle, endAngle, 
					    pose,
					    myMaxRange,
					    angle);
}

/**
   Get the closest reading in the current buffer within a rectangular region
   defined by two points (opposite corners of the rectangle).
   @param x1 the x coordinate of one of the rectangle points
   @param y1 the y coordinate of one of the rectangle points
   @param x2 the x coordinate of the other rectangle point
   @param y2 the y coordinate of the other rectangle point
   @param pose a pointer to an MvrPose object in which to store the location of
   the closest position
   @return The range to the reading from the device, or a value >= maxRange if
   no reading was found in the box.
*/
MVREXPORT double MvrRangeDevice::currentReadingBox(double x1, double y1, 
						 double x2, double y2,
						 MvrPose *pose) const
{
  MvrPose robotPose;
  if (myRobot != NULL)
      robotPose = myRobot->getPose();
  else
    {
      MvrLog::log(MvrLog::Normal, "MvrRangeDevice %s: NULL robot, won't get reading box correctly", getName());
      robotPose.setPose(0, 0);
    }
  return myCurrentBuffer.getClosestBox(x1, y1, x2, y2, robotPose,
				       myMaxRange, pose);
}

/**
   Get the closest reading in the cumulative buffer within a rectangular region 
   around the range device, defined by two points (opposeite points
   of a rectangle).
   @param x1 the x coordinate of one of the rectangle points
   @param y1 the y coordinate of one of the rectangle points
   @param x2 the x coordinate of the other rectangle point
   @param y2 the y coordinate of the other rectangle point
   @param pose a pointer to an MvrPose object in which to store the location of
   the closest position
   @return The range to the reading from the device, or a value >= maxRange if
   no reading was found in the box.
*/
MVREXPORT double MvrRangeDevice::cumulativeReadingBox(double x1, double y1, 
						 double x2, double y2,
						 MvrPose *pose) const
{
  MvrPose robotPose;
  if (myRobot != NULL)
    robotPose = myRobot->getPose();
  else
    {
      MvrLog::log(MvrLog::Normal, "MvrRangeDevice %s: NULL robot, won't get reading box correctly", getName());
      robotPose.setPose(0, 0);
    }
  return myCumulativeBuffer.getClosestBox(x1, y1, x2, y2, 
					  robotPose,
					  myMaxRange, pose);
}

/** 
    Applies a coordinate transformation to some or all buffers. 
    This is mostly useful for translating
    to/from local/global coordinate systems, but may have other uses.
    @param trans the transform to apply to the data
    @param doCumulative whether to transform the cumulative buffer or not
*/    
MVREXPORT void MvrRangeDevice::applyTransform(MvrTransform trans, 
					    bool doCumulative)
{
  myCurrentBuffer.applyTransform(trans);
  if (doCumulative)
    myCumulativeBuffer.applyTransform(trans);
}

/** Copies the list into a vector.
 *  @swignote The return type will be named MvrSensorReadingVector instead
 *    of the std::vector template type.
 */
MVREXPORT std::vector<MvrSensorReading> *MvrRangeDevice::getRawReadingsAsVector(void)
{
  
  std::list<MvrSensorReading *>::const_iterator it;
  myRawReadingsVector.clear();
  // if we don't have any return an empty list
  if (myRawReadings == NULL)
    return &myRawReadingsVector;
  myRawReadingsVector.reserve(myRawReadings->size());
  for (it = myRawReadings->begin(); it != myRawReadings->end(); it++)
    myRawReadingsVector.insert(myRawReadingsVector.begin(), *(*it));
  return &myRawReadingsVector;
}

/** Copies the list into a vector.
 *  @swignote The return type will be named MvrSensorReadingVector instead
 *    of the std::vector template type.
 */
MVREXPORT std::vector<MvrSensorReading> *MvrRangeDevice::getAdjustedRawReadingsAsVector(void)
{
  
  std::list<MvrSensorReading *>::const_iterator it;
  myAdjustedRawReadingsVector.clear();
  // if we don't have any return an empty list
  if (myAdjustedRawReadings == NULL)
    return &myRawReadingsVector;
  myAdjustedRawReadingsVector.reserve(myRawReadings->size());
  for (it = myAdjustedRawReadings->begin(); 
       it != myAdjustedRawReadings->end(); 
       it++)
    myAdjustedRawReadingsVector.insert(myAdjustedRawReadingsVector.begin(), 
				       *(*it));
  return &myAdjustedRawReadingsVector;
}


MVREXPORT void MvrRangeDevice::setCurrentDrawingData(MvrDrawingData *data, 
						   bool takeOwnershipOfData)
{
  if (myCurrentDrawingData != NULL && myOwnCurrentDrawingData)
  {
    delete myCurrentDrawingData;
    myCurrentDrawingData = NULL;
    myOwnCurrentDrawingData = false;
  }
  myCurrentDrawingData = data; 
  myOwnCurrentDrawingData = takeOwnershipOfData; 
}

MVREXPORT  void MvrRangeDevice::setCumulativeDrawingData(
	MvrDrawingData *data, 
	bool takeOwnershipOfData)
{
  if (myCumulativeDrawingData != NULL && myOwnCumulativeDrawingData)
  {
    delete myCumulativeDrawingData;
    myCumulativeDrawingData = NULL;
    myOwnCumulativeDrawingData = false;
  }
  myCumulativeDrawingData = data; 
  myOwnCumulativeDrawingData = takeOwnershipOfData; 
}

MVREXPORT void MvrRangeDevice::adjustRawReadings(bool interlaced)
{
  std::list<MvrSensorReading *>::iterator rawIt;

  // make sure we have raw readings and a robot, and a delay to
  // correct for (note that if we don't have a delay to correct for
  // but have already been adjusting (ie someone changed the delay)
  // we'll just keep adjusting)
  if (myRawReadings == NULL || myRobot == NULL || 
      (myAdjustedRawReadings == NULL && myRobot->getOdometryDelay() == 0))
    return;
  

  // if we don't already have a list then make one
  if (myAdjustedRawReadings == NULL)
    myAdjustedRawReadings = new std::list<MvrSensorReading *>;
  
  // if we've already adjusted these readings then don't do it again
  if (myRawReadings->begin() != myRawReadings->end() &&
      myRawReadings->front()->getAdjusted())
    return;

  std::list<MvrSensorReading *>::iterator adjIt;
  MvrSensorReading *adjReading;
  MvrSensorReading *rawReading;

  MvrTransform trans;
  MvrTransform encTrans;
  MvrTransform interlacedTrans;
  MvrTransform interlacedEncTrans;

  bool first = true;
  bool second = true;

  int onReading;
  for (rawIt = myRawReadings->begin(), adjIt = myAdjustedRawReadings->begin(), 
       onReading = 0; 
       rawIt != myRawReadings->end(); 
       rawIt++, onReading++)
  {
    rawReading = (*rawIt);
    if (adjIt != myAdjustedRawReadings->end())
    {
      adjReading = (*adjIt);
      adjIt++;
    }
    else
    {
      adjReading = new MvrSensorReading;
      myAdjustedRawReadings->push_back(adjReading);
    }
    (*adjReading) = (*rawReading);
    if (first || (interlaced && second))
    {
      MvrPose origPose;
      MvrPose corPose;
      MvrPose origEncPose;
      MvrPose corEncPose;
      MvrTime corTime;


      corTime = rawReading->getTimeTaken();
      //corTime.addMSec(-myRobot->getOdometryDelay());
      if (myRobot->getPoseInterpPosition(corTime, 
					 &corPose) == 1 && 
	  myRobot->getEncoderPoseInterpPosition(corTime, 
						&corEncPose) == 1)
      {
	origPose = rawReading->getPoseTaken();
	origEncPose = rawReading->getEncoderPoseTaken();
	/*
	printf("Difference was %g %g %g (rotVel %.0f, rotvel/40 %g)\n", 
	       origEncPose.getX() - corEncPose.getX(),
	       origEncPose.getY() - corEncPose.getY(),
	       origEncPose.getTh() - corEncPose.getTh(),
	       myRobot->getRotVel(), myRobot->getRotVel() / 40);
	*/
	if (first)
	{
	  trans.setTransform(origPose, corPose);
	  encTrans.setTransform(origEncPose, corEncPose);
	}
	else if (interlaced && second)
	{
	  interlacedTrans.setTransform(origPose, corPose);
	  interlacedEncTrans.setTransform(origEncPose, corEncPose);
	}
      }
      else
      {
	//printf("Couldn't correct\n");
      }

      if (first)
	first = false;
      else if (interlaced && second)
	second = false;

    }
    if (!interlaced && (onReading % 2) == 0)
    {
      adjReading->applyTransform(trans);
      adjReading->applyEncoderTransform(encTrans);
    }
    else
    {
      adjReading->applyTransform(interlacedTrans);
      adjReading->applyEncoderTransform(interlacedEncTrans);
    }
    /*
    if (fabs(adjReading->getEncoderPoseTaken().getX() - 
	     corEncPose.getX()) > 1 ||
	fabs(adjReading->getEncoderPoseTaken().getY() - 
	     corEncPose.getY()) > 1 || 
	fabs(MvrMath::subAngle(adjReading->getEncoderPoseTaken().getTh(), 
			      corEncPose.getTh())) > .2)
      printf("(%.0f %.0f %.0f) should be (%.0f %.0f %.0f)\n", 
	     adjReading->getEncoderPoseTaken().getX(),
	     adjReading->getEncoderPoseTaken().getY(),
	     adjReading->getEncoderPoseTaken().getTh(),
	     corEncPose.getX(), corEncPose.getY(),  corEncPose.getTh());
    */
    adjReading->setAdjusted(true);
    rawReading->setAdjusted(true);
  }  
}



