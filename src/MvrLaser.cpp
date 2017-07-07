#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLaser.h"
#include "MvrRobot.h"
#include "MvrDeviceConnection.h"

bool MvrLaser::ourUseSimpleNaming = false;

MVREXPORT MvrLaser::MvrLaser(
	int laserNumber, const char *name, 
	unsigned int absoluteMaxRange, bool locationDependent, 
	bool appendLaserNumberToName) :
  MvrRangeDeviceThreaded(
	  361, 200, name, absoluteMaxRange,
	  0, 0, 0, locationDependent)
{
  myLaserNumber = laserNumber;

  if (appendLaserNumberToName)
  {
    char buf[1024];
    snprintf(buf, sizeof(buf) - 20, "%s", name);
    sprintf(buf, "%s_%d", buf, myLaserNumber);
    myName = buf;
  }
  else
  {
    if (laserNumber != 1)
      MvrLog::log(MvrLog::Verbose, "MvrLaser::%s: Laser created with number %d, but the number is not appended to the name which may break things (especially since this number is greater than 1_", name, laserNumber);

    myName = name;
  }


  laserSetName(myName.c_str());

  myAbsoluteMaxRange = absoluteMaxRange;
  myMaxRangeSet = false;

  setSensorPosition(0, 0, 0, 0);
  myTimeoutSeconds = 8;

  myHaveSensorPose = false;

  myFlipped = false;
  myFlippedSet = false;

  myCanSetDegrees = false;
  myStartDegreesMin = HUGE_VAL;
  myStartDegreesMax = -HUGE_VAL;
  myStartDegreesSet = false;
  myStartDegrees = 0;
  myEndDegreesMin = HUGE_VAL;
  myEndDegreesMax = -HUGE_VAL;
  myEndDegreesSet = false;
  myEndDegrees = 0;

  myCanChooseDegrees = false;
  myDegreesChoiceDouble = -HUGE_VAL;

  myCanSetIncrement = 0;
  myIncrementMin = HUGE_VAL; 
  myIncrementMax = -HUGE_VAL; 
  myIncrementSet = false;
  myIncrement = 0;

  myCanChooseIncrement = false; 
  myIncrementChoiceDouble = -HUGE_VAL;

  myCanChooseUnits = false;

  myCanChooseReflectorBits = false;

  myCanSetPowerControlled = false;
  myPowerControlled = true;
  myPowerControlledSet = false;

  myCanChooseStartingBaud = false;

  myCanChooseAutoBaud = false;
  
  myDefaultTcpPort = 8102;

  myInfoLogLevel = MvrLog::Verbose;
  myRobotRunningAndConnected = false;
}

MVREXPORT MvrLaser::~MvrLaser()
{
}

/**
   This can be used to set the name on mutexes and such to match the
   laser's new name.
**/
MVREXPORT void MvrLaser::laserSetName(const char *name)
{
  if (ourUseSimpleNaming)
  {
    myName = "Laser_";
    char buf[1024];
    sprintf(buf, "%d", myLaserNumber);
    myName += buf;
  }    
  else
  {
    myName = name;
  }


  myTask.setThreadName(myName.c_str());

  myConnectCBList.setNameVar("%s::myConnectCBList", myName.c_str());
  myFailedConnectCBList.setNameVar("%s::myFailedConnectCBList", myName.c_str());
  myDisconnectOnErrorCBList.setNameVar(
	  "%s::myDisconnectOnErrorCBList", myName.c_str());
  myDisconnectNormallyCBList.setNameVar(
	  "%s::myDisconnectNormallyCBList", myName.c_str());
  myDataCBList.setNameVar("%s::myDataCBList", myName.c_str());
  myDataCBList.setLogging(false); // supress debug logging since it drowns out all other logging 
}

MVREXPORT void MvrLaser::setMaxRange(unsigned int maxRange)
{
  if (maxRange > myAbsoluteMaxRange)
  {
    MvrLog::log(MvrLog::Terse, "%s::setMaxRange: Tried to set the max range to %u which is above the absoluteMaxRange on the device of %d, capping it", 
	       getName(), maxRange, getAbsoluteMaxRange());
    MvrRangeDevice::setMaxRange(myAbsoluteMaxRange);
  }
  else
    MvrRangeDevice::setMaxRange(maxRange);

  myMaxRangeSet = true;
}

MVREXPORT void MvrLaser::setCumulativeBufferSize(size_t size)
{
  MvrRangeDevice::setCumulativeBufferSize(size);
  myCumulativeBufferSizeSet = true;
}


MVREXPORT void MvrLaser::laserSetAbsoluteMaxRange(unsigned int absoluteMaxRange)
{ 
  MvrLog::log(myInfoLogLevel, "%s: Setting absolute max range to %u", 
	     getName(), absoluteMaxRange);
  myAbsoluteMaxRange = absoluteMaxRange; 
  setMaxRange(getMaxRange());
}

/**
   Filter readings, moving them from the raw current buffer to
   filtered current buffer (see MvrRangeDevice), and then also to the
   cumulative buffer.

   This must be called for the laser subclass to work right.

   This also calls the reading callbacks.
**/

void MvrLaser::laserProcessReadings(void)
{
  // if we have no readings... don't do anything
  if (myRawReadings == NULL || myRawReadings->begin() == myRawReadings->end())
    return;

  std::list<MvrSensorReading *>::iterator sensIt;
  MvrSensorReading *sReading;
  double x, y;
  double lastX = 0.0, lastY = 0.0;
  //unsigned int i = 0;
  MvrTime len;
  len.setToNow();

  bool clean;
  if (myCumulativeCleanInterval <= 0 ||
      (myCumulativeLastClean.mSecSince() > 
       myCumulativeCleanInterval))
  {
    myCumulativeLastClean.setToNow();
    clean = true;
  }
  else
  {
    clean = false;
  }
  
  myCurrentBuffer.setPoseTaken(myRawReadings->front()->getPoseTaken());
  myCurrentBuffer.setEncoderPoseTaken(
	  myRawReadings->front()->getEncoderPoseTaken());
  myCurrentBuffer.beginRedoBuffer();	  

  // walk the buffer of all the readings and see if we want to add them
  for (sensIt = myRawReadings->begin(); 
       sensIt != myRawReadings->end(); 
       ++sensIt)
  {
    sReading = (*sensIt);

    // if we have ignore readings then check them here
    if (!myIgnoreReadings.empty() && 
	(myIgnoreReadings.find(
		(int) ceil(sReading->getSensorTh())) != 
	 myIgnoreReadings.end()) || 
	myIgnoreReadings.find(
		(int) floor(sReading->getSensorTh())) != 
	myIgnoreReadings.end())
      sReading->setIgnoreThisReading(true);

    // see if the reading is valid
    if (sReading->getIgnoreThisReading())
      continue;

    // if we have a max range then check it here... 
    if (myMaxRange != 0 && 
	sReading->getRange() > myMaxRange)
    {
      sReading->setIgnoreThisReading(true);
    }

    // see if the reading is valid... this is set up this way so that
    // max range readings can cancel out other readings, but will
    // still be ignored other than that... ones ignored for other
    // reasons were skipped above
    if (sReading->getIgnoreThisReading())
    {
      internalProcessReading(sReading->getX(), sReading->getY(), 
			     sReading->getRange(), clean, true);
      continue;
    }

    // get our coords
    x = sReading->getX();
    y = sReading->getY();
    
    // see if we're checking on the filter near dist... if we are
    // and the reading is a good one we'll check the cumulative
    // buffer
    if (myMinDistBetweenCurrentSquared > 0.0000001)
    {
      // see where the last reading was
      //squaredDist = (x-lastX)*(x-lastX) + (y-lastY)*(y-lastY);
      // see if the reading is far enough from the last reading
      if (MvrMath::squaredDistanceBetween(x, y, lastX, lastY) > 
	  myMinDistBetweenCurrentSquared)
      {
	lastX = x;
	lastY = y;
	// since it was a good reading, see if we should toss it in
	// the cumulative buffer... 
	internalProcessReading(x, y, sReading->getRange(), clean, false);
	
	/* we don't do this part anymore since it wound up leaving
	// too many things not really tehre... if its outside of our
	// sensor angle to use to filter then don't let this one
	// clean  (MvrMath::fabs(sReading->getSensorTh()) > 50)
	// filterAddAndCleanCumulative(x, y, false); else*/
      }
      // it wasn't far enough, skip this one and go to the next one
      else
      {
	continue;		
      }
    }
    // we weren't filtering the readings, but see if it goes in the
    // cumulative buffer anyways
    else
    {
      internalProcessReading(x, y, sReading->getRange(), clean, false);
    }
    // now drop the reading into the current buffer
    myCurrentBuffer.redoReading(x, y);
    //i++;
  }
  myCurrentBuffer.endRedoBuffer();
  /*  Put this in to see how long the cumulative filtering is taking  
  if (clean)
    printf("### %ld %d\n", len.mSecSince(), myCumulativeBuffer.getBuffer()->size());
    */
  internalGotReading();
}


void MvrLaser::internalProcessReading(double x, double y, 
				     unsigned int range, bool clean,
				     bool onlyClean)
{
  if (myCumulativeBuffer.getSize() == 0)
    return;

  // make sure we really want to clean
  if (clean && myCumulativeCleanDistSquared < 1)
    clean = false;


  std::list<MvrPoseWithTime *>::iterator cit;
  bool addReading = true;

  //double squaredDist;

  MvrLineSegment line;
  double xTaken = myCurrentBuffer.getPoseTaken().getX();
  double yTaken = myCurrentBuffer.getPoseTaken().getY();
  MvrPose intersection;
  MvrPoseWithTime reading(x, y);

  // if we're not cleaning and its further than we're keeping track of
  // readings ignore it... replaced with the part thats 'until here'
  /*
  if (!clean && 
      myMaxInsertDistCumulative > 1 && 
      range > myMaxInsertDistCumulative)
    return;
  */

  if (onlyClean)
    addReading = false;

  if (myMaxInsertDistCumulative > 1 && 
      range > myMaxInsertDistCumulative)
    addReading = false;

  if (!clean && !addReading)
    return;
  // until here
  
  // if we're cleaning we start our sweep
  if (clean)
    myCumulativeBuffer.beginInvalidationSweep();
  // run through all the readings
  for (cit = getCumulativeBuffer()->begin(); 
       cit != getCumulativeBuffer()->end(); 
       ++cit)
  {
    // if its closer to a reading than the filter near dist, just return
    if (addReading && myMinDistBetweenCumulativeSquared < .0000001 ||
	(MvrMath::squaredDistanceBetween(x, y, (*cit)->getX(), (*cit)->getY()) <
	 myMinDistBetweenCumulativeSquared))
    {
      // if we're not cleaning it and its too close just return,
      // otherwise keep going (to clear out invalid readings)
      if (!clean)
	return;
      addReading = false;
    }
    // see if this reading invalidates some other readings by coming too close
    if (clean)
    {
      // set up our line
      line.newEndPoints(x, y, xTaken, yTaken);
      // see if the cumulative buffer reading perpindicular intersects
      // this line segment, and then see if its too close if it does,
      // but if the intersection is very near the endpoint then leave it
      if (line.getPerpPoint((*cit), &intersection) &&
	  (intersection.squaredFindDistanceTo(*(*cit)) < 
	   myCumulativeCleanDistSquared) &&
	  (intersection.squaredFindDistanceTo(reading) > 
	   50 * 50))
      {
	//printf("Found one too close to the line\n");
	myCumulativeBuffer.invalidateReading(cit);
      }
    }
  }
  // if we're cleaning finish the sweep
  if (clean)
    myCumulativeBuffer.endInvalidationSweep();
  // toss the reading in
  if (addReading)
    myCumulativeBuffer.addReading(x, y);

}

MVREXPORT bool MvrLaser::laserPullUnsetParamsFromRobot(void)
{
  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Normal, "%s: Trying to connect, but have no robot, continuing under the assumption this is intentional", getName());
    return true;
  }

  const MvrRobotParams *params = myRobot->getRobotParams();
  if (params == NULL)
  {
    MvrLog::log(MvrLog::Terse, 
	       "%s: Robot has no params, cannot pull unset params from robot",
	       getName());
    return false;
  }

  const char *paramStr;
  char *endPtr;
  double paramDouble;
  int paramInt;
  bool paramBool;

  paramBool = params->getLaserFlipped(getLaserNumber());
  if (!myFlippedSet)
  {
    if (paramBool)
    {
      MvrLog::log(myInfoLogLevel, 
		 "%s: Setting flipped to true from robot params",
		 getName());
      setFlipped(true);
    }
    else if (!paramBool)
    {
      MvrLog::log(myInfoLogLevel, 
		 "%s: Setting flipped to false from robot params",
		 getName());
      setFlipped(false);
    }
  }

  paramInt = params->getLaserMaxRange(getLaserNumber());
  if (!myMaxRangeSet)
  {
    if(paramInt < 0)
    {
      MvrLog::log(MvrLog::Terse, "%s: LaserMaxRange in robot param file was negative but shouldn't be (it was '%d'), failing", getName(), paramInt);
      return false;
    }
    if (paramInt > 0)
    {
      MvrLog::log(myInfoLogLevel, 
		 "%s: Setting max range to %d from robot params",
		 getName(), paramInt);
      setMaxRange(paramInt);
    }
  }

  paramInt = params->getLaserCumulativeBufferSize(getLaserNumber());
  if (!myCumulativeBufferSizeSet)
  {
    if(paramInt < 0)
    {
      MvrLog::log(MvrLog::Terse, "%s: LaserCumulativeBufferSize in robot param file was negative but shouldn't be (it was '%d'), failing", getName(), paramInt);
      return false;
    }
    if (paramInt > 0)
    {
      MvrLog::log(myInfoLogLevel, 
		 "%s: Setting cumulative buffer size to %d from robot params",
		 getName(), paramInt);
      setCumulativeBufferSize(paramInt);
    }
  }

  paramStr = params->getLaserStartDegrees(getLaserNumber());
  if (canSetDegrees() && !myStartDegreesSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    paramDouble = strtod(paramStr, &endPtr);
    if(endPtr == paramStr)
    {
      MvrLog::log(MvrLog::Terse, "%s: LaserStartDegrees in robot param file was not a double (it was '%s'), failing", getName(), paramStr);
      return false;
    }
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting start degrees to %g from robot params",
	       getName(), paramDouble);
    setStartDegrees(paramDouble);
  }

  paramStr = params->getLaserEndDegrees(getLaserNumber());
  if (canSetDegrees() && !myEndDegreesSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    paramDouble = strtod(paramStr, &endPtr);
    if(endPtr == paramStr)
    {
      MvrLog::log(MvrLog::Terse, 
		 "%s: LaserEndDegrees in robot param file was not a double (it was '%s'), failing", 
		 getName(), paramStr);
      return false;
    }
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting end degrees to %g from robot params",
	       getName(), paramDouble);
    setEndDegrees(paramDouble);
  }

  paramStr = params->getLaserDegreesChoice(getLaserNumber());
  if (canChooseDegrees() && !myDegreesChoiceSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting degrees choice to %s from robot params",
	       getName(), paramStr);
    chooseDegrees(paramStr);
  }

  paramStr = params->getLaserIncrement(getLaserNumber());
  if (canSetDegrees() && !myIncrementSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    paramDouble = strtod(paramStr, &endPtr);
    if(endPtr == paramStr)
    {
      MvrLog::log(MvrLog::Terse, 
		 "%s: LaserIncrement in robot param file was not a double (it was '%s'), failing", 
		 getName(), paramStr);
      return false;
    }
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting increment to %g from robot params",
	       getName(), paramDouble);
    setIncrement(paramDouble);
  }

  paramStr = params->getLaserIncrementChoice(getLaserNumber());
  if (canChooseIncrement() && !myIncrementChoiceSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting increment choice to %s from robot params",
	       getName(), paramStr);
    chooseIncrement(paramStr);
  }

  paramStr = params->getLaserUnitsChoice(getLaserNumber());
  if (canChooseUnits() && !myUnitsChoiceSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting units choice to %s from robot params",
	       getName(), paramStr);
    chooseUnits(paramStr);
  }

  paramStr = params->getLaserReflectorBitsChoice(getLaserNumber());
  if (canChooseReflectorBits() && !myReflectorBitsChoiceSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting reflectorBits choice to %s from robot params",
	       getName(), paramStr);
    chooseReflectorBits(paramStr);
  }

  paramBool = params->getLaserPowerControlled(getLaserNumber());
  if (canSetPowerControlled() && !myPowerControlledSet)
  {
    if (paramBool)
    {
      MvrLog::log(myInfoLogLevel, 
		 "%s: Setting powerControlled to true from robot params",
		 getName());
      setPowerControlled(true);
    }
    else if (!paramBool)
    {
      MvrLog::log(myInfoLogLevel, 
		 "%s: Setting powerControlled to false from robot params",
		 getName());
      setPowerControlled(false);
    }
  }

  paramStr = params->getLaserStartingBaudChoice(getLaserNumber());
  if (canChooseStartingBaud() && !myStartingBaudChoiceSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    MvrLog::log(myInfoLogLevel,
	       "%s: Setting startingBaud choice to %s from robot params",
	       getName(), paramStr);
    chooseStartingBaud(paramStr);
  }

  paramStr = params->getLaserAutoBaudChoice(getLaserNumber());
  if (canChooseAutoBaud() && !myAutoBaudChoiceSet && 
      paramStr != NULL && paramStr[0] != '\0')
  {
    MvrLog::log(myInfoLogLevel, 
	       "%s: Setting autoBaud choice to %s from robot params",
	       getName(), paramStr);
    chooseAutoBaud(paramStr);
  }

  if (!addIgnoreReadings(params->getLaserIgnore(getLaserNumber())))
    return false;

  setSensorPosition(params->getLaserX(getLaserNumber()), 
		    params->getLaserY(getLaserNumber()), 
		    params->getLaserTh(getLaserNumber()),
		    params->getLaserZ(getLaserNumber()));

  return true;
}

MVREXPORT void MvrLaser::setDeviceConnection(MvrDeviceConnection *conn)
{
  myConnMutex.lock();
  myConn = conn; 
  myConn->setDeviceName(getName());
  myConnMutex.unlock();
}

MVREXPORT MvrDeviceConnection *MvrLaser::getDeviceConnection(void)
{
  return myConn;
}

/**
   Sets the time to go without a response from the laser
   until it is assumed that the connection with the laser has been
   broken and the disconnect on error events will happen.

   If there is no robot then there is a straightforward check of last
   reading time against this value.  If there is a robot then it will
   not start the check until the laser is running and connected.

   @param seconds if 0 or less then the connection timeout feature
   will be disabled, otherwise disconnect on error will be triggered
   after this number of miliseconds...  
**/
MVREXPORT void MvrLaser::setConnectionTimeoutSeconds(double seconds)
{
  MvrLog::log(MvrLog::Normal, 
	     "%s::setConnectionTimeoutSeconds: Setting timeout to %g secs", 
	     getName(), seconds);
  myLastReading.setToNow();

  if (seconds > 0)
    myTimeoutSeconds = seconds;
  else
    myTimeoutSeconds = 0;
}

/**
   Gets the time (miliseconds) to go without response from the laser
   until it is assumed that the connection with the laser has been
   broken and the disconnect on error events will happen.
   If 0, then the timeout is disabled.

   If there is no robot then there is a straightforward check of last
   reading time against this value.  If there is a robot then it will
   not start the check until the laser is running and connected.
**/
double MvrLaser::getConnectionTimeoutSeconds(void)
{
  return myTimeoutSeconds;
}

MVREXPORT void MvrLaser::laserConnect(void)
{
  // figure out how many readings we can have and set the current
  // buffer size to that
  double degrees;

  myLastReading.setToNow();

  if (canSetDegrees())
  {
    //degrees = fabs(MvrMath::subAngle(getStartDegrees(), getEndDegrees()));
    degrees = fabs(getStartDegrees() - getEndDegrees());
    MvrLog::log(myInfoLogLevel, 
	       "%s: Using degrees settings of %g to %g for %g degrees",
	       getName(), getStartDegrees(), getEndDegrees(), 
	       degrees);
  }
  else if (canChooseDegrees())
  {
    degrees = getDegreesChoiceDouble();
    MvrLog::log(myInfoLogLevel, "%s: Using choice of %g degrees",
	       getName(), degrees);
  }
  else
  {
    degrees = 360;
    MvrLog::log(MvrLog::Terse, "%s: Don't have any settings for degrees, arbitrarily using 360", getName());
  }

  double increment;
  if (canSetIncrement())
  {
    increment = getIncrement();
    MvrLog::log(myInfoLogLevel, "%s: Using increment setting of %g degrees",
	       getName(), increment);
  }
  else if (canChooseIncrement())
  {
    increment = getIncrementChoiceDouble();
    MvrLog::log(myInfoLogLevel, "%s: Using increment setting of %g degrees",
	       getName(), increment);
  }
  else
  {
	// PS 10/20/11 - This was missing causing buffer size to be very large
	// set this to the lowest, note both the SZ and S3 are setting the buffer
	// size but it's being overriden by this procedure - do we want to fix
	// this or just leave it at the max value 360/.25=1440???
	increment = .25;
    MvrLog::log(MvrLog::Terse, "%s: Don't have any settings for increment, arbitrarily using .25", getName());
  }
  
  int size = (int)ceil(degrees / increment) + 1;
  MvrLog::log(myInfoLogLevel, "%s: Setting current buffer size to %d", 
	     getName(), size);
  setCurrentBufferSize(size);
  
  MvrLog::log(myInfoLogLevel, "%s: Connected", getName());
  myConnectCBList.invoke();
}

MVREXPORT void MvrLaser::laserFailedConnect(void)
{
  MvrLog::log(myInfoLogLevel, "%s: Failed to connect", getName());
  myFailedConnectCBList.invoke();
}

MVREXPORT void MvrLaser::laserDisconnectNormally(void)
{
  MvrLog::log(myInfoLogLevel, "%s: Disconnected normally", getName());
  myDisconnectNormallyCBList.invoke();
}

MVREXPORT void MvrLaser::laserDisconnectOnError(void)
{
  MvrLog::log(MvrLog::Normal, "%s: Disconnected because of error", getName());
  myDisconnectOnErrorCBList.invoke();
}

MVREXPORT void MvrLaser::internalGotReading(void)
{
  if (myTimeLastReading != time(NULL)) 
  {
    myTimeLastReading = time(NULL);
    myReadingCount = myReadingCurrentCount;
    myReadingCurrentCount = 0;
  }
  myReadingCurrentCount++;

  myLastReading.setToNow();
  
  myDataCBList.invoke();
}

MVREXPORT int MvrLaser::getReadingCount()
{
  if (myTimeLastReading == time(NULL))
    return myReadingCount;
  if (myTimeLastReading == time(NULL) - 1)
    return myReadingCurrentCount;
  return 0;
}





MVREXPORT void MvrLaser::setSensorPosition(
	double x, double y, double th, double z)
{
  setSensorPosition(MvrPose(x, y, th), z);
}

MVREXPORT void MvrLaser::setSensorPosition(MvrPose pose, double z)
{
  myHaveSensorPose = true;
  mySensorPose.setPose(pose);
  mySensorZ = z;
}

bool MvrLaser::internalCheckChoice(const char *check, const char *choice, 
				  std::list<std::string> *choices,
				  const char *choicesStr)
{
  if (check == NULL || choices == NULL || choice == NULL || choice[0] == '\0')
  {
    MvrLog::log(MvrLog::Terse, "%s::%s: Internal error in setup");
    return false;
  }
  
  std::list<std::string>::iterator it;
  std::string str;
  for (it = choices->begin(); it != choices->end(); it++)
  {
    str = (*it);
    if (MvrUtil::strcasecmp(choice, str) == 0)
      return true;
  }
  
  MvrLog::log(MvrLog::Terse, "%s::%s: Invalid choice, choices are <%s>.",
	     myName.c_str(), check, choicesStr);
  return false;
}

bool MvrLaser::internalCheckChoice(const char *check, const char *choice, 
				  std::map<std::string, double> *choices,
				  const char *choicesStr, 
				  double *choiceDouble)
{
  if (check == NULL || choices == NULL || choice == NULL || choice[0] == '\0')
  {
    MvrLog::log(MvrLog::Terse, "%s::%s: Internal error in setup");
    return false;
  }
  
  std::map<std::string, double>::iterator it;
  std::string str;
  for (it = choices->begin(); it != choices->end(); it++)
  {
    str = (*it).first;
    if (MvrUtil::strcasecmp(choice, str) == 0)
    {
      *choiceDouble = (*it).second;
      return true;
    }
  }

  MvrLog::log(MvrLog::Terse, "%s::%s: Invalid choice, choices are <%s>.",
	     myName.c_str(), check, choicesStr);
  return false;
}

void MvrLaser::internalBuildChoicesString(
	std::list<std::string> *choices, std::string *str)
{
  std::list<std::string>::iterator it;
  bool first;
  std::string choiceStr;

  for (it = choices->begin(), first = true; it != choices->end(); it++)
  {
    choiceStr = (*it);
    if (!first)
      (*str) += "|";
    first = false;
    (*str) += choiceStr;
  }
}

void MvrLaser::internalBuildChoices(
	std::map<std::string, double> *choices, std::string *str,
	std::list<std::string> *choicesList)
{
  std::map<std::string, double>::iterator it;
  bool first;
  std::string choiceStr;

  for (it = choices->begin(), first = true; it != choices->end(); it++)
  {
    choiceStr = (*it).first;
    choicesList->push_back(choiceStr);
    if (!first)
      (*str) += "|";
    first = false;
    (*str) += choiceStr;
  }
}

/**
   This allows the setting of the degrees the laser will use from a
   range for both starting and ending degrees.  Only one of this and
   laserAllowDegreesChoices should be used.

   @param defaultStartDegrees The default start degrees to use, this
   default should probably be for the max range.

   @param startDegreesMin The minimum value for start degrees

   @param startDegreesMax The maximum value for start degrees

   @param defaultEndDegrees The default end degrees to use, this
   default should probably be for the max range.

   @param endDegreesMin The minimum value for end degrees

   @param endDegreesMax The maximum value for end degrees
**/
MVREXPORT void MvrLaser::laserAllowSetDegrees(double defaultStartDegrees, double startDegreesMin, double startDegreesMax, double defaultEndDegrees, double endDegreesMin, double endDegreesMax)
{
  myCanSetDegrees = true;
  myStartDegreesMin = startDegreesMin;
  myStartDegreesMax = startDegreesMax;
  setStartDegrees(defaultStartDegrees);
  myStartDegreesSet = false;

  myEndDegreesMin = endDegreesMin;
  myEndDegreesMax = endDegreesMax;
  setEndDegrees(defaultEndDegrees);
  myEndDegreesSet = false;

}

MVREXPORT bool MvrLaser::setStartDegrees(double startDegrees)
{
  if (!myCanSetDegrees)
  {
    MvrLog::log(MvrLog::Terse, "%s::setStartDegrees: Cannot set angles on this laser", myName.c_str());
    return false;
  }
  if (startDegrees < myStartDegreesMin)
  {
    MvrLog::log(MvrLog::Terse, "%s::setStartDegrees: Start degrees (%g) tried to be set to less than the minimum (%g))", myName.c_str(), startDegrees, myStartDegreesMin);
    return false;
  }
  if (startDegrees > myStartDegreesMax)
  {
    MvrLog::log(MvrLog::Terse, "%s::setStartDegrees: Start degrees (%g) tried to be set to greater than the minimum (%g))", myName.c_str(), startDegrees, myStartDegreesMax);
    return false;
  }
  if (myEndDegreesSet && startDegrees >= myEndDegrees)
  {
    MvrLog::log(MvrLog::Terse, "%s::setStartDegrees: Start degrees (%g) tried to be set to greater than or equal to end degrees %g)", myName.c_str(), startDegrees, myEndDegrees);
    return false;
  }
  myStartDegreesSet = true;
  myStartDegrees = startDegrees;
  return true;
}
    
MVREXPORT bool MvrLaser::setEndDegrees(double endDegrees)
{
  if (!myCanSetDegrees)
  {
    MvrLog::log(MvrLog::Terse, "%s::setEndDegrees: Cannot set angles on this laser", myName.c_str());
    return false;
  }
  if (endDegrees < myEndDegreesMin)
  {
    MvrLog::log(MvrLog::Terse, "%s::setEndDegrees: End degrees (%g) tried to be set to less than the minimum (%g))", myName.c_str(), endDegrees, myEndDegreesMin);
    return false;
  }
  if (endDegrees > myEndDegreesMax)
  {
    MvrLog::log(MvrLog::Terse, "%s::setEndDegrees: End degrees (%g) tried to be set to greater than the minimum (%g))", myName.c_str(), endDegrees, myEndDegreesMax);
    return false;
  }
  if (myStartDegreesSet && endDegrees <= myStartDegrees)
  {
    MvrLog::log(MvrLog::Terse, "%s::setEndDegrees: End degrees (%g) tried to be set to less than or equal to end degrees %g)", myName.c_str(), endDegrees, myStartDegrees);
    return false;
  }
  myEndDegreesSet = true;
  myEndDegrees = endDegrees;
  return true;
}

/**
   Allows the choice of the laser degrees from one of a number of
   choices, only one of this and laserAllowSetDegrees should be used.

   @param defaultDegreesChoice The default degrees, this should be
   the largest value.

   @param degreesChoices this is a mapping of std::strings to
   doubles, the strings should be the actual available choices, and
   the doubles should be the numerical representation... this is so
   the simulated laser can behave more easily like the real
   lasers... and because the original sick driver used words typed out
   (to make problems more obvious).
**/
MVREXPORT void MvrLaser::laserAllowDegreesChoices(
	const char *defaultDegreesChoice, 
	std::map<std::string, double> degreesChoices)
{
  myCanChooseDegrees = true;
  myDegreesChoices = degreesChoices;
  internalBuildChoices(&myDegreesChoices, &myDegreesChoicesString, &myDegreesChoicesList);
  chooseDegrees(defaultDegreesChoice);
  myDegreesChoiceSet = false;
}

MVREXPORT bool MvrLaser::chooseDegrees(
	const char *degreesChoice)
{
  if (!myCanChooseDegrees)
  {
    MvrLog::log(MvrLog::Terse, "%s::chooseDegrees: Cannot choose degrees on this laser", myName.c_str());
    return false;
  }

  double degreesChoiceDouble;
  if (!internalCheckChoice("chooseDegrees", degreesChoice, &myDegreesChoices, 
		   myDegreesChoicesString.c_str(), &degreesChoiceDouble))
    return false;

  myDegreesChoice = degreesChoice;
  myDegreesChoiceDouble = degreesChoiceDouble;
  return true;      
}

/**
   Allows the choice of increment from a range, only one of this and
   laserAllowIncrementChoices should be used.

   @param defaultIncrement The default increment to use, this
   default should be a reasonable value.

   @param incrementMin The minimum value for the increment

   @param incrementMax The maximum value for the increment
**/
MVREXPORT void MvrLaser::laserAllowSetIncrement(
	double defaultIncrement, double incrementMin, double incrementMax)
{
  myCanSetIncrement = true;
  myIncrementMin = incrementMin;
  myIncrementMax = incrementMax;
  setIncrement(defaultIncrement);
  myIncrementSet = false;

}

MVREXPORT bool MvrLaser::setIncrement(double increment)
{
  if (!myCanSetIncrement)
  {
    MvrLog::log(MvrLog::Terse, "%s::setIncrement: Cannot set increment on this laser", myName.c_str());
    return false;
  }

  if (increment < myIncrementMin)
  {
    MvrLog::log(MvrLog::Terse, "%s::setIncrement: Increment (%g) tried to be set to less than the minimum (%g))", myName.c_str(), increment, myIncrementMin);
    return false;
  }
  if (increment > myIncrementMax)
  {
    MvrLog::log(MvrLog::Terse, "%s::setIncrement: End degrees (%g) tried to be set to greater than the maximum (%g))", myName.c_str(), increment, myIncrementMax);
    return false;
  }
  myIncrementSet = true;
  myIncrement = increment;
  return true;
}

/**
   Allows the choice of increment from a limited set of values, only
   one of this and laserAllowSetIncrement should be used.

   @param defaultIncrementChoice The default increment, this should be
   a reasonable value.

   @param incrementChoices this is a mapping of std::strings to
   doubles, the strings should be the actual available choices, and
   the doubles should be the numerical representation... this is so
   the simulated laser can behave more easily like the real
   lasers... and because the original sick driver used words typed out
   (to make problems more obvious).
**/
MVREXPORT void MvrLaser::laserAllowIncrementChoices(
	const char *defaultIncrementChoice, 
	std::map<std::string, double> incrementChoices)
{
  myCanChooseIncrement = true;
  myIncrementChoices = incrementChoices;
  internalBuildChoices(&myIncrementChoices, &myIncrementChoicesString, &myIncrementChoicesList);
  chooseIncrement(defaultIncrementChoice);
  myIncrementChoiceSet = false;
}

MVREXPORT bool MvrLaser::chooseIncrement(const char *incrementChoice)
{
  if (!myCanChooseIncrement)
  {
    MvrLog::log(MvrLog::Terse, "%s::chooseIncrement: Cannot choose increment on this laser", myName.c_str());
    return false;
  }

  double incrementChoiceDouble;
  if (!internalCheckChoice("chooseIncrement", incrementChoice, 
		   &myIncrementChoices, myIncrementChoicesString.c_str(), 
		   &incrementChoiceDouble))
    return false;
  myIncrementChoice = incrementChoice;
  myIncrementChoiceDouble = incrementChoiceDouble;
  return true;      
}

/**
   @param defaultUnitsChoice This is the default units choice, it
   should be a reasonable value.

   @param unitsChoices The possible choices for units.
**/
MVREXPORT void MvrLaser::laserAllowUnitsChoices(
	const char *defaultUnitsChoice, 
	std::list<std::string> unitsChoices)
{
  myCanChooseUnits = true;
  myUnitsChoices = unitsChoices;
  internalBuildChoicesString(&myUnitsChoices, &myUnitsChoicesString);
  chooseUnits(defaultUnitsChoice);
  myUnitsChoiceSet = false;

}

MVREXPORT bool MvrLaser::chooseUnits(const char *unitsChoice)
{
  if (!myCanChooseUnits)
  {
    MvrLog::log(MvrLog::Terse, "%s::chooseUnits: Cannot choose units on this laser", myName.c_str());
    return false;
  }

  if (!internalCheckChoice("chooseUnits", unitsChoice, &myUnitsChoices,
		   myUnitsChoicesString.c_str()))
    return false;

  myUnitsChoice = unitsChoice;
  return true;      
}

/**
   @param defaultReflectorBitsChoice The default choice for reflector
   bits, should be a reasonable value.
   
   @param reflectorBitsChoices The possible choices for reflector bits
**/
MVREXPORT void MvrLaser::laserAllowReflectorBitsChoices(
	const char *defaultReflectorBitsChoice, 
	std::list<std::string> reflectorBitsChoices)
{
  myCanChooseReflectorBits = true;
  myReflectorBitsChoices = reflectorBitsChoices;
  internalBuildChoicesString(&myReflectorBitsChoices, &myReflectorBitsChoicesString);
  chooseReflectorBits(defaultReflectorBitsChoice);
  myReflectorBitsChoiceSet = false;
}

MVREXPORT bool MvrLaser::chooseReflectorBits(const char *reflectorBitsChoice)
{
  if (!myCanChooseReflectorBits)
  {
    MvrLog::log(MvrLog::Terse, "%s::chooseReflectorBits: Cannot choose reflectorBits on this laser", myName.c_str());
    return false;
  }

  if (!internalCheckChoice("chooseReflectorBits", reflectorBitsChoice, 
		   &myReflectorBitsChoices, 
		   myReflectorBitsChoicesString.c_str()))
    return false;

  myReflectorBitsChoice = reflectorBitsChoice;
  return true;      
}

/**
   Allows settings of whether the power can be controlled or not.
   This is mostly for devices that respond differently at power up
   than they do if they are already on (ie the lms2xx where it doesn't
   respond at all while powering up).  If the communication is the
   same either way, you can just not set this.

   @param defaultPowerControlled The default value for power
   controlled.
**/
MVREXPORT void MvrLaser::laserAllowSetPowerControlled(bool defaultPowerControlled)
{
  myCanSetPowerControlled = true;
  setPowerControlled(defaultPowerControlled);
  myPowerControlledSet = false;
}

MVREXPORT bool MvrLaser::setPowerControlled(
	bool powerControlled)
{
  if (!myCanSetPowerControlled)
  {
    MvrLog::log(MvrLog::Terse, "%s::setPowerControlled: Cannot set if the laser power is controlled on this laser", myName.c_str());
    return false;
  }

  myPowerControlledSet = true;
  myPowerControlled = powerControlled;
  return true;      
}

/**
   @param defaultStartingBaudChoice Default starting baud choice.
   This should probably stay the same as what the sensor ships with.

   @param startingBaudChoices The available choices for starting baud
**/
MVREXPORT void MvrLaser::laserAllowStartingBaudChoices(
	const char *defaultStartingBaudChoice, 
	std::list<std::string> startingBaudChoices)
{
  myCanChooseStartingBaud = true;
  myStartingBaudChoices = startingBaudChoices;
  internalBuildChoicesString(&myStartingBaudChoices, &myStartingBaudChoicesString);
  chooseStartingBaud(defaultStartingBaudChoice);
  myStartingBaudChoiceSet = false;
}

MVREXPORT bool MvrLaser::chooseStartingBaud(const char *startingBaudChoice)
{
  if (!myCanChooseStartingBaud)
  {
    MvrLog::log(MvrLog::Terse, "%s::chooseStartingBaud: Cannot choose startingBaud on this laser", myName.c_str());
    return false;
  }

  if (!internalCheckChoice("chooseStartingBaud", startingBaudChoice, 
		   &myStartingBaudChoices, 
		   myStartingBaudChoicesString.c_str()))
    return false;

  myStartingBaudChoice = startingBaudChoice;
  return true;      
}

/**
   @param defaultAutoBaudChoice Default auto baud choice.  This should
   probably be the maximum reasonable reliable robust rate that the
   laser supports.  The laser should autobaud up to this choice after
   it connects.

   @param autoBaudChoices The available choices for auto baud
**/
MVREXPORT void MvrLaser::laserAllowAutoBaudChoices(
	const char *defaultAutoBaudChoice, 
	std::list<std::string> autoBaudChoices)
{
  myCanChooseAutoBaud = true;
  myAutoBaudChoices = autoBaudChoices;
  internalBuildChoicesString(&myAutoBaudChoices, &myAutoBaudChoicesString);
  chooseAutoBaud(defaultAutoBaudChoice);
  myAutoBaudChoiceSet = false;
}

MVREXPORT bool MvrLaser::chooseAutoBaud(const char *autoBaudChoice)
{
  if (!myCanChooseAutoBaud)
  {
    MvrLog::log(MvrLog::Terse, "%s::chooseAutoBaud: Cannot choose autoBaud on this laser", myName.c_str());
    return false;
  }

  if (!internalCheckChoice("chooseAutoBaud", autoBaudChoice, 
		   &myAutoBaudChoices, myAutoBaudChoicesString.c_str()))
    return false;

  myAutoBaudChoice = autoBaudChoice;
  return true;      
}

MVREXPORT void MvrLaser::laserSetDefaultTcpPort(int defaultTcpPort)
{
  myDefaultTcpPort = defaultTcpPort;
}

MVREXPORT void MvrLaser::laserSetDefaultPortType(const char *defaultPortType)
{
  myDefaultPortType = defaultPortType;
}

MVREXPORT bool MvrLaser::addIgnoreReadings(const char *ignoreReadings)
{
  // if we have , then use it as the separator, otherwise use space
  // like normal
  char separator = ' ';
  if (strstr(ignoreReadings, ",") != NULL)
    separator = ',';

  MvrArgumentBuilder args(1024, separator);
  
  args.add(ignoreReadings);
  if (args.getArgc() == 0)
    return true;

  size_t i;
  const char *str;
  float begin, end;
  float ignore;
  for (i = 0; i < args.getArgc(); i++)
  {
    if (args.isArgDouble(i))
    {
      ignore = args.getArgDouble(i);
      addIgnoreReading(ignore);
      MvrLog::log(MvrLog::Verbose, "%s: Added ignore reading %g", 
		 getName(), ignore);
    }
    else
    {
      str = args.getArg(i);
      if (sscanf(str, "%f:%f", &begin, &end) == 2 || 
	  sscanf(str, "%f-%f", &begin, &end) == 2)
      {
	MvrLog::log(MvrLog::Verbose, "%s: Adding ignore reading from %g to %g", 
		   getName(), begin, end);
	// reorder them for easier looping
	if (begin > end)
	{
	  ignore = begin;
	  begin = end;
	  end = ignore;
	}

	MvrLog::log(MvrLog::Verbose, "%s: Added ignore reading (beginning) %g", 
		   getName(), begin);
	addIgnoreReading(begin);
	for (ignore = begin; ignore <= end; ignore += 1.0)
	{
	  MvrLog::log(MvrLog::Verbose, "%s: Added ignore reading %g", 
		     getName(), ignore);
	  addIgnoreReading(ignore);
	}
	MvrLog::log(MvrLog::Verbose, "%s: Added ignore reading (ending) %g", 
		   getName(), end);
	addIgnoreReading(end);
      }
      else
      {
	MvrLog::log(MvrLog::Terse, "%s: Bad syntax for ignore readings, had string '%s' as one of the arguments (the values need to either be individual doubles, or begin:end (75:77) or begin-end (75-77))", getName(), str);
	return false;
      }
    }
  }

  return true;
}

/** 
    Applies a transform to the buffers. this is mostly useful for translating
    to/from local/global coordinates, but may have other uses.  
    This is different from
    the class because it also transforms the raw readings.
    @param trans the transform to apply to the data
    @param doCumulative whether to transform the cumulative buffer or not
*/
MVREXPORT void MvrLaser::applyTransform(MvrTransform trans,
				       bool doCumulative)
{
  myCurrentBuffer.applyTransform(trans);
  std::list<MvrSensorReading *>::iterator it;

  for (it = myRawReadings->begin(); it != myRawReadings->end(); ++it)
    (*it)->applyTransform(trans);

  if (doCumulative)
    myCumulativeBuffer.applyTransform(trans);
}

/**
   This will check if the laser has lost connection.  If there is no
   robot it is a straightforward check of last reading time against
   getConnectionTimeoutSeconds.  If there is a robot then it will not
   start the check until the laser is running and connected.
**/
MVREXPORT bool MvrLaser::laserCheckLostConnection(void)
{
	
  if ((myRobot == NULL || myRobotRunningAndConnected) && 
      getConnectionTimeoutSeconds() > 0 && 
      myLastReading.mSecSince() >  getConnectionTimeoutSeconds() * 1000)
    return true;

  if (!myRobotRunningAndConnected && myRobot != NULL && 
      myRobot->isRunning() && myRobot->isConnected())
  {
    myRobotRunningAndConnected = true;
    myLastReading.setToNow();
  }

  return false;
}

MVREXPORT void MvrLaser::copyReadingCount(const MvrLaser* laser)
{
  myTimeLastReading = laser->myTimeLastReading;
  myReadingCurrentCount = laser->myReadingCurrentCount;
  myReadingCount = laser->myReadingCount;
}

MVREXPORT void MvrLaser::useSimpleNamingForAllLasers(void)
{
  MvrLog::log(MvrLog::Normal, "MvrLaser: Will use simple naming for all lasers");
  ourUseSimpleNaming = true;
}
