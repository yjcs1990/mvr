#include "MvrExport.h"
#include "MvrLaserFilter.h"
#include "MvrRobot.h"
#include "MvrConfig.h"

//#define DEBUGRANGEFILTER

MVREXPORT MvrLaserFilter::MvrLaserFilter(
	MvrLaser *laser, const char *name) :
  MvrLaser(laser->getLaserNumber(),
	  name != NULL && name[0] != '\0' ? name : laser->getName(), 
	  laser->getAbsoluteMaxRange(),
	  laser->isLocationDependent(),
	  false),
  myProcessCB(this, &MvrLaserFilter::processReadings)
{
  myLaser = laser;

  if (name == NULL || name[0] == '\0')
  {
    std::string filteredName;
    filteredName = "filtered_";
    filteredName += laser->getName();
    laserSetName(filteredName.c_str());
  }

  myRawReadings = new std::list<MvrSensorReading *>;

  char buf[1024];
  sprintf(buf, "%sProcessCB", getName());
  myProcessCB.setName(buf);

  myAngleToCheck = 1;
  myAnyFactor = -1;
  myAllFactor = -1;
  myAnyMinRange = -1;
  myAnyMinRangeLessThanAngle = -180;
  myAnyMinRangeGreaterThanAngle = 180;
  
  setCurrentDrawingData(
	  new MvrDrawingData(*(myLaser->getCurrentDrawingData())),
	  true);

  setCumulativeDrawingData(
	  new MvrDrawingData(*(myLaser->getCumulativeDrawingData())),
	  true);

  // laser parameters
  setInfoLogLevel(myLaser->getInfoLogLevel());
  setConnectionTimeoutSeconds(myLaser->getConnectionTimeoutSeconds());
  setCurrentBufferSize(myLaser->getCurrentBufferSize());
  setCumulativeBufferSize(myLaser->getCumulativeBufferSize());
  setCumulativeCleanDist(myLaser->getCumulativeCleanDist());
  setCumulativeCleanInterval(myLaser->getCumulativeCleanInterval());
  setCumulativeCleanOffset(myLaser->getCumulativeCleanOffset());

  setSensorPosition(myLaser->getSensorPosition(), 
		    myLaser->getSensorPositionZ());
  laserSetAbsoluteMaxRange(myLaser->getAbsoluteMaxRange());
  // set our max range to the laser we're filtering... then set the
  // max range on the laser we're filtering to 0, so that it's over
  // max range values don't get set to ignore, since then we can't
  // clear cumulatives beyond that value
  setMaxRange(myLaser->getMaxRange());
  myLaser->setMaxRange(0);

  // base range device parameters
  setMaxSecondsToKeepCurrent(myLaser->getMaxSecondsToKeepCurrent());
  setMinDistBetweenCurrent(getMinDistBetweenCurrent());
  setMaxSecondsToKeepCumulative(myLaser->getMaxSecondsToKeepCumulative());
  setMaxDistToKeepCumulative(myLaser->getMaxDistToKeepCumulative());
  setMinDistBetweenCumulative(myLaser->getMinDistBetweenCumulative());
  setMaxInsertDistCumulative(myLaser->getMaxInsertDistCumulative());
  setCurrentDrawingData(myLaser->getCurrentDrawingData(), false);
  setCumulativeDrawingData(myLaser->getCumulativeDrawingData(), false);

  // turn off the cumulative buffer on the original to save CPU
  myLaser->setCumulativeBufferSize(0);


  // now all the specific laser settings (this should already be taken
  // care of when this is created, but the code existed for the
  // simulated laser so I put it here too)
  if (myLaser->canSetDegrees())
    laserAllowSetDegrees(
	    myLaser->getStartDegrees(), myLaser->getStartDegreesMin(), 
	    myLaser->getStartDegreesMax(), myLaser->getEndDegrees(), 
	    myLaser->getEndDegreesMin(), myLaser->getEndDegreesMax());

  if (myLaser->canChooseDegrees())
    laserAllowDegreesChoices(myLaser->getDegreesChoice(), 
			myLaser->getDegreesChoicesMap());

  if (myLaser->canSetIncrement())
    laserAllowSetIncrement(myLaser->getIncrement(), 
			   myLaser->getIncrementMin(), 
			   myLaser->getIncrementMax());

  if (myLaser->canChooseIncrement())
    laserAllowIncrementChoices(myLaser->getIncrementChoice(), 
			  myLaser->getIncrementChoicesMap());

  if (myLaser->canChooseUnits())
    laserAllowUnitsChoices(myLaser->getUnitsChoice(), 
			   myLaser->getUnitsChoices());

  if (myLaser->canChooseReflectorBits())
    laserAllowReflectorBitsChoices(myLaser->getReflectorBitsChoice(), 
			      myLaser->getReflectorBitsChoices());
  
  if (canSetPowerControlled())
    laserAllowSetPowerControlled(myLaser->getPowerControlled());

  if (myLaser->canChooseStartingBaud())
    laserAllowStartingBaudChoices(myLaser->getStartingBaudChoice(), 
			      myLaser->getStartingBaudChoices());

  if (myLaser->canChooseAutoBaud())
    laserAllowAutoBaudChoices(myLaser->getAutoBaudChoice(), 
			      myLaser->getAutoBaudChoices());
  
  laserSetDefaultTcpPort(myLaser->getDefaultTcpPort());
  laserSetDefaultPortType(myLaser->getDefaultPortType());

}

MVREXPORT MvrLaserFilter::~MvrLaserFilter()
{
  if (myRobot != NULL)
  {
    myRobot->remSensorInterpTask(&myProcessCB);
    myRobot->remLaser(this);
  }
}

MVREXPORT void MvrLaserFilter::addToConfig(MvrConfig *config, 
					       const char *sectionName,
					       const char *prefix)
{
  std::string name;
  
  config->addSection(MvrConfig::CATEGORY_ROBOT_OPERATION,
                     sectionName,
                     "");

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), sectionName,
		     MvrPriority::FACTORY);
  name = prefix;
  name += "AngleSpread";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myAngleToCheck,
	      "Filter settings.  The angle spread to check on either side of each reading",
		      0),
	  sectionName, MvrPriority::FACTORY);

  name = prefix;
  name += "AnyNeighborFactor";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myAnyFactor,
	      "Filter settings.  If a reading (decided by the anglespread) is further than any of its neighbor reading times this factor, it is ignored... so a value between 0 and 1 will check if they're all closer, a value greater than 1 will see if they're all further, negative values means this factor won't be used",
		      -1),
	  sectionName, MvrPriority::FACTORY);

  name = prefix;
  name += "AllNeighborFactor";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myAllFactor,
	      "Filter settings.  If a reading (decided by the anglespread) is further than all of its neighbor reading times this factor, it is ignored... so a value between 0 and 1 will check if they're all closer, a value greater than 1 will see if they're all further, negative values means this factor won't be used",
		      -1),
	  sectionName, MvrPriority::FACTORY);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), sectionName,
		   MvrPriority::FACTORY);

  name = prefix;
  name += "AnyNeighborMinRange";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myAnyMinRange,
	      "Filter settings.  If a reading itself, or if it has a neighbor (decided by the anglespread) that is closer than this value (in mm) it is ignored... negative values means this factor won't be used",
		      -1),
	  sectionName, MvrPriority::FACTORY);

  name = prefix;
  name += "AnyNeighborMinRangeLessThanAngle";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myAnyMinRangeLessThanAngle,
	      "Filter settings.  The AnyNeighborMinRange will only be applied to angles LESS than this (so the AnyNeighborMinRange filter will only apply angles below this, or above GreatestAngle)"),
	  sectionName, MvrPriority::FACTORY);

  name = prefix;
  name += "AnyNeighborMinRangeGreaterThanAngle";
  config->addParam(
	  MvrConfigArg(name.c_str(), &myAnyMinRangeGreaterThanAngle,
	      "Filter settings.  The AnyNeighborMinRange will only be applied to angles GREATER than this (so the AnyNeighborMinRange filter will only apply above this, or below LeastAngle)"),
	  sectionName, MvrPriority::FACTORY);

  config->addParam(MvrConfigArg(MvrConfigArg::SEPARATOR), sectionName,
		   MvrPriority::FACTORY);

}

MVREXPORT void MvrLaserFilter::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
  {
    myRobot->remSensorInterpTask(&myProcessCB);
    myRobot->addSensorInterpTask(myName.c_str(), 51, &myProcessCB);
  }
  MvrLaser::setRobot(robot);
}

void MvrLaserFilter::processReadings(void)
{
  myLaser->lockDevice();
  selfLockDevice();

  const std::list<MvrSensorReading *> *rdRawReadings;
  std::list<MvrSensorReading *>::const_iterator rdIt;
  
  if ((rdRawReadings = myLaser->getRawReadings()) == NULL)
  {
    selfUnlockDevice();
    myLaser->unlockDevice();
    return;
  }

  size_t rawSize = myRawReadings->size();
  size_t rdRawSize = myLaser->getRawReadings()->size();
  
  while (rawSize < rdRawSize)
  {
    myRawReadings->push_back(new MvrSensorReading);
    rawSize++;
  }

  // set where the pose was taken
  myCurrentBuffer.setPoseTaken(
	  myLaser->getCurrentRangeBuffer()->getPoseTaken());
  myCurrentBuffer.setEncoderPoseTaken(
	  myLaser->getCurrentRangeBuffer()->getEncoderPoseTaken());


  std::list<MvrSensorReading *>::iterator it;
  MvrSensorReading *rdReading;
  MvrSensorReading *reading;

#ifdef DEBUGRANGEFILTER
  FILE *file = NULL;
  //file = MvrUtil::fopen("/mnt/rdsys/tmp/filter", "w");
  file = MvrUtil::fopen("/tmp/filter", "w");
#endif

  std::map<int, MvrSensorReading *> readingMap;
  int numReadings = 0;

  // first pass to copy the readings and put them into a map
  for (rdIt = rdRawReadings->begin(), it = myRawReadings->begin();
       rdIt != rdRawReadings->end() && it != myRawReadings->end();
       rdIt++, it++)
  {
    rdReading = (*rdIt);
    reading = (*it);
    *reading = *rdReading;

    readingMap[numReadings] = reading;
    numReadings++;
  }

  // if we're not doing any filtering, just short circuit out now
  if (myAllFactor <= 0 && myAnyFactor <= 0 && myAnyMinRange <= 0)
  {
    laserProcessReadings();
    copyReadingCount(myLaser);

    selfUnlockDevice();
    myLaser->unlockDevice();
#ifdef DEBUGRANGEFILTER
    if (file != NULL)
      fclose(file);
#endif
    return;
  }
  
  char buf[1024];
  int i;
  int j;
  //MvrSensorReading *lastAddedReading = NULL;
  
  // now walk through the readings to filter them
  for (i = 0; i < numReadings; i++)
  {
    reading = readingMap[i];

    // if we're ignoring this reading then just get on with life
    if (reading->getIgnoreThisReading())
      continue;

    /* Taking this check out since the base class does it now and if
     * it gets marked ignore now it won't get used for clearing
     * cumulative readings

    if (myMaxRange >= 0 && reading->getRange() > myMaxRange)
    {
#ifdef DEBUGRANGEFILTER
      if (file != NULL)
	fprintf(file, "%.1f beyond max range at %d\n", 
		reading->getSensorTh(), reading->getRange());
#endif
      reading->setIgnoreThisReading(true);
      continue;
    }
    */
    if (myAnyMinRange >= 0 && reading->getRange() < myAnyMinRange &&
	(reading->getSensorTh() < myAnyMinRangeLessThanAngle ||
	 reading->getSensorTh() > myAnyMinRangeGreaterThanAngle))
    {
#ifdef DEBUGRANGEFILTER
      if (file != NULL)
	fprintf(file, "%.1f within min range at %d\n", 
		reading->getSensorTh(), reading->getRange());
#endif
      reading->setIgnoreThisReading(true);
      continue;
    }

    /*
    if (lastAddedReading != NULL)
    {

      if (lastAddedReading->getPose().findDistanceTo(reading->getPose()) < 50)
      {
#ifdef DEBUGRANGEFILTER
	if (file != NULL)
	  fprintf(file, "%.1f too close from last %6.0f\n", 
		  reading->getSensorTh(),
		  lastAddedReading->getPose().findDistanceTo(
			  reading->getPose()));
#endif
	reading->setIgnoreThisReading(true);
	continue;
      }
#ifdef DEBUGRANGEFILTER
      else if (file != NULL)
	fprintf(file, "%.1f from last %6.0f\n", 
		reading->getSensorTh(),
		lastAddedReading->getPose().findDistanceTo(
			reading->getPose()));
#endif
    }
    */

    buf[0] = '\0';
    bool goodAll = true;
    bool goodAny = false;
    bool goodMinRange = true;
    if (myAnyFactor <= 0)
      goodAny = true;
    for (j = i - 1; 
	 (j >= 0 && //good && 
	  fabs(MvrMath::subAngle(readingMap[j]->getSensorTh(),
				reading->getSensorTh())) <= myAngleToCheck);
	 j--)
    {
      /* You can't skip these, or you get onesided filtering
      if (readingMap[j]->getIgnoreThisReading())
      {
#ifdef DEBUGRANGEFILTER
	sprintf(buf, "%s %6s", buf, "i");
#endif
	continue;
      }
      */
#ifdef DEBUGRANGEFILTER
      sprintf(buf, "%s %6d", buf, readingMap[j]->getRange());
#endif
      if (myAllFactor > 0 && 
	  !checkRanges(reading->getRange(), 
		       readingMap[j]->getRange(), myAllFactor))
	goodAll = false;
      if (myAnyFactor > 0 &&
	  checkRanges(reading->getRange(), 
		      readingMap[j]->getRange(), myAnyFactor))
	goodAny = true;
      if (myAnyMinRange > 0 && 
	  (reading->getSensorTh() < myAnyMinRangeLessThanAngle ||
	   reading->getSensorTh() > myAnyMinRangeGreaterThanAngle) &&
	  readingMap[j]->getRange() <= myAnyMinRange)
	goodMinRange = false;
	
    }
#ifdef DEBUGRANGEFILTER
    sprintf(buf, "%s %6d*", buf, reading->getRange());
#endif 
    for (j = i + 1; 
	 (j < numReadings && //good &&
	  fabs(MvrMath::subAngle(readingMap[j]->getSensorTh(),
				reading->getSensorTh())) <= myAngleToCheck);
	 j++)
    {
      // you can't ignore these or you get one sided filtering
      /*
      if (readingMap[j]->getIgnoreThisReading())
      {
#ifdef DEBUGRANGEFILTER
	sprintf(buf, "%s %6s", buf, "i");
#endif
	continue;
      }
      */
#ifdef DEBUGRANGEFILTER
      sprintf(buf, "%s %6d", buf, readingMap[j]->getRange());
#endif
      if (myAllFactor > 0 && 
	  !checkRanges(reading->getRange(), 
		       readingMap[j]->getRange(), myAllFactor))
	goodAll = false;
      if (myAnyFactor > 0 &&
	  checkRanges(reading->getRange(), 
		       readingMap[j]->getRange(), myAnyFactor))
	goodAny = true;
      if (myAnyMinRange > 0 && 
	  (reading->getSensorTh() < myAnyMinRangeLessThanAngle ||
	   reading->getSensorTh() > myAnyMinRangeGreaterThanAngle) &&
	  readingMap[j]->getRange() <= myAnyMinRange)
	goodMinRange = false;
    }
    

    if (!goodAll || !goodAny || !goodMinRange)
      reading->setIgnoreThisReading(true);
    /*
    else
      lastAddedReading = reading;
    */
#ifdef DEBUGRANGEFILTER
    if (file != NULL)
      fprintf(file, 
	      "%5.1f %6d %c\t%s\n", reading->getSensorTh(), reading->getRange(),
	      goodAll && goodAny && goodMinRange ? 'g' : 'b', buf);
#endif
	    
  }


#ifdef DEBUGRANGEFILTER
  if (file != NULL)
    fclose(file);
#endif

  laserProcessReadings();
  copyReadingCount(myLaser);

  selfUnlockDevice();
  myLaser->unlockDevice();
}

/**
   @return Return true if the reading is good, false if the reading is bad
**/
bool MvrLaserFilter::checkRanges(int thisReading, int otherReading,
				double factor)
{
  if (thisReading == otherReading || factor <= 0)
    return true;

  if ((factor >= 1 && thisReading > otherReading * factor) || 
      (factor < 1 && thisReading < otherReading * factor))
    return false;
  else
    return true;
}


MVREXPORT int MvrLaserFilter::selfLockDevice(void)
{
  return lockDevice();
}

MVREXPORT int MvrLaserFilter::selfTryLockDevice(void)
{
  return tryLockDevice();
}

MVREXPORT int MvrLaserFilter::selfUnlockDevice(void)
{
  return unlockDevice();
}
