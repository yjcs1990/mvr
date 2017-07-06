#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrInterpolation.h"

MVREXPORT MvrInterpolation::MvrInterpolation(size_t numberOfReadings)
{
  mySize = numberOfReadings;
  myDataMutex.setLogName("MvrInterpolation");
  setAllowedMSForPrediction();
  setAllowedPercentageForPrediction();
  setLogPrediction();
}

MVREXPORT MvrInterpolation::~MvrInterpolation()
{

}

MVREXPORT bool MvrInterpolation::addReading(MvrTime timeOfReading, MvrPose position)
{
  myDataMutex.lock();
  if (myTimes.size() >= mySize)
  {
    myTimes.pop_back();
    myPoses.pop_back();
  }
  myTimes.push_front(timeOfReading);
  myPoses.push_front(position);
  myDataMutex.unlock();
  return true;
}

/**
   @param timeStamp the time we are interested in
   @param position the pose to set to the given position

   @param mostRecent the most recent data in the interpolation relevant to this call... for a return of 1 this is the near side it interpolated between, for a return of 0 or 1 this is the most recent data in the interpolation.... this is only useful if the return is 1, 0, or -1, and is mostly for use with MvrRobot::applyEncoderOffset 

   @return 1 its good interpolation, 0 its predicting, -1 its too far to 
   predict, -2 its too old, -3 there's not enough data to predict
   
**/

MVREXPORT int MvrInterpolation::getPose(MvrTime timeStamp, MvrPose *position, MvrPoseWithTime *mostRecent)
{
  std::list<MvrTime>::iterator tit;
  std::list<MvrPose>::iterator pit;
  
  MvrPose thisPose;
  MvrTime thisTime;
  MvrPose lastPose;
  MvrTime lastTime;

  //MvrTime nowTime;
  long total;
  long toStamp;
  double percentage;
  MvrPose retPose;
  
  myDataMutex.lock();
  // find the time we want
  for (tit = myTimes.begin(), pit = myPoses.begin();
       tit != myTimes.end() && pit != myPoses.end(); 
       ++tit, ++pit)
  {
    lastTime = thisTime;
    lastPose = thisPose;

    thisTime = (*tit);
    thisPose = (*pit);

    //printf("## %d %d %d b %d at %d after %d\n", timeStamp.getMSec(), thisTime.getMSec(), timeStamp.mSecSince(thisTime), timeStamp.isBefore(thisTime), timeStamp.isAt(thisTime), timeStamp.isAfter(thisTime));
    //if (timeStamp.isBefore(thisTime) || timeStamp.isAt(thisTime))
    if (!timeStamp.isAfter(thisTime))
    {
      //printf("Found one!\n");
      break;
    } 

  }

  if (mostRecent != NULL)
  {
    mostRecent->setPose(thisPose);
    mostRecent->setTime(thisTime);
  }
    

  // if we're at the end then it was too long ago
  if (tit == myTimes.end() || pit == myPoses.end())
  {
    //printf("Too old\n");
    myDataMutex.unlock();
    return -2;
  }
  
  // this is for forecasting (for the brave)
  if ((tit == myTimes.begin() || pit == myPoses.begin()) && 
      !timeStamp.isAt((*tit)))
  {
    //printf("Too new %d %d\n", tit == myTimes.begin(), pit == myPoses.begin());
  
    thisTime = (*tit);
    thisPose = (*pit);
    tit++;
    pit++;  
    if (tit == myTimes.end() || pit == myPoses.end())
    {
      //printf("Not enough data\n");
      myDataMutex.unlock();
      return -3;
    }
    lastTime = (*tit);
    lastPose = (*pit);

    //nowTime.setToNow();
    total = thisTime.mSecSince(lastTime);
    if (total == 0)
      total = 100;
    //toStamp = nowTime.mSecSince(thisTime);
    toStamp = timeStamp.mSecSince(thisTime);
    percentage = (double)toStamp/(double)total;
    //if (percentage > 50)
    if (myAllowedPercentageForPrediction >= 0 && 
	percentage * 100 > myAllowedPercentageForPrediction)
    {
      if (myLogPrediction)
	MvrLog::log(MvrLog::Normal, "%s: returningPercentage Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100, myAllowedPercentageForPrediction);
      
      myDataMutex.unlock();
      return -1;
    }

    if (myAllowedMSForPrediction >= 0 && 
	abs(toStamp) > myAllowedMSForPrediction)
    {
    if (myLogPrediction)
      MvrLog::log(MvrLog::Normal, "%s: returningMS Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100,
	  myAllowedMSForPrediction);
    
      myDataMutex.unlock();
      return -1;
    }

    if (myLogPrediction)
      MvrLog::log(MvrLog::Normal, "%s: Total time %d, to stamp %d, percentage %.2f (allowed %d)", getName(), total, toStamp, percentage * 100,
		 myAllowedPercentageForPrediction);

    retPose.setX(thisPose.getX() + 
		 (thisPose.getX() - lastPose.getX()) * percentage);
    retPose.setY(thisPose.getY() + 
		 (thisPose.getY() - lastPose.getY()) * percentage);
    retPose.setTh(MvrMath::addAngle(thisPose.getTh(),
				   MvrMath::subAngle(thisPose.getTh(),
						    lastPose.getTh())
				   * percentage));

    if (retPose.findDistanceTo(thisPose) > 1000)
      MvrLog::log(MvrLog::Normal, "%s: finaldist %.0f thislastdist %.0f Total time %d, to stamp %d, percentage %.2f", getName(), 
		 retPose.findDistanceTo(thisPose), thisPose.findDistanceTo(lastPose), total, toStamp, percentage * 100);


    *position = retPose;
    myDataMutex.unlock();
    return 0;
  }

  // this is the actual interpolation

  //printf("Woo hoo!\n");

  total = thisTime.mSecSince(lastTime);
  toStamp = thisTime.mSecSince(timeStamp);
  percentage = (double)toStamp/(double)total;

  if (total == 0)
    percentage = 0;

  //if (total == 0)
  //printf("Total time %d, to stamp %d, percentage %.2f\n", 	 total, toStamp, percentage * 100);

  retPose.setX(thisPose.getX() + 
	       (lastPose.getX() - thisPose.getX()) * percentage); 
  retPose.setY(thisPose.getY() + 
	       (lastPose.getY() - thisPose.getY()) * percentage); 
  retPose.setTh(MvrMath::addAngle(thisPose.getTh(),
				 MvrMath::subAngle(lastPose.getTh(), 
						  thisPose.getTh())
				 * percentage));
/*
  printf("original:");
  thisPose.log();
  printf("After:");
  lastPose.log();
  printf("ret:");
  retPose.log();
*/
  *position = retPose;
  myDataMutex.unlock();
  return 1;
  
}

MVREXPORT size_t MvrInterpolation::getNumberOfReadings(void) const
{
  return mySize;
}

MVREXPORT void MvrInterpolation::setNumberOfReadings(size_t numberOfReadings)
{
  myDataMutex.lock();
  while (myTimes.size() > numberOfReadings)
  {
    myTimes.pop_back();
    myPoses.pop_back();
  }
  mySize = numberOfReadings;  
  myDataMutex.unlock();
}

MVREXPORT void MvrInterpolation::reset(void)
{
  myDataMutex.lock();
  while (myTimes.size() > 0)
    myTimes.pop_back();
  while (myPoses.size() > 0)
    myPoses.pop_back();
  myDataMutex.unlock();
}

MVREXPORT void MvrInterpolation::setName(const char *name)
{
  myDataMutex.lock();
  myName = name;
  std::string mutexLogName;
  mutexLogName = myName;
  mutexLogName += "::DataMutex";
  myDataMutex.setLogName(mutexLogName.c_str());
  myDataMutex.unlock();
}

MVREXPORT const char * MvrInterpolation::getName(void)
{
  return myName.c_str();
}

MVREXPORT void MvrInterpolation::setAllowedMSForPrediction(int ms)
{
  myDataMutex.lock();
  myAllowedMSForPrediction = ms;
  myDataMutex.unlock();
}

MVREXPORT int MvrInterpolation::getAllowedMSForPrediction(void)
{
  int ret;
  myDataMutex.lock();
  ret = myAllowedMSForPrediction;
  myDataMutex.unlock();
  return ret;
}

MVREXPORT void MvrInterpolation::setAllowedPercentageForPrediction(int percentage)
{
  myDataMutex.lock();
  myAllowedPercentageForPrediction = percentage;
  myDataMutex.unlock();
}

MVREXPORT int MvrInterpolation::getAllowedPercentageForPrediction(void)
{
  int ret;
  myDataMutex.lock();
  ret = myAllowedPercentageForPrediction;
  myDataMutex.unlock();
  return ret;
}

MVREXPORT void MvrInterpolation::setLogPrediction(bool logPrediction)
{
  myDataMutex.lock();
  myLogPrediction = logPrediction;
  myDataMutex.unlock();
}

MVREXPORT bool MvrInterpolation::getLogPrediction(void)
{
  bool ret;
  myDataMutex.lock();
  ret = myLogPrediction;
  myDataMutex.unlock();
  return ret;
}

