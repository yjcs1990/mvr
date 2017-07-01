#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRangeBuffer.h"
#include "MvrLog.h"

/** @param size The size of the buffer, in number of readings */
MVREXPORT MvrRangeBuffer::MvrRangeBuffer(int size)
{
  mySize = size;
}

MVREXPORT MvrRangeBuffer::~MvrRangeBuffer()
{
  MvrUtil::deleteSet(myBuffer.begin(), myBuffer.end());
  MvrUtil::deleteSet(myInvalidBuffer.begin(), myInvalidBuffer.end());
}

MVREXPORT size_t MvrRangeBuffer::getSize(void) const
{
  return mySize;
}

MVREXPORT MvrPose MvrRangeBuffer::getPoseTaken() const
{
  return myBufferPose;
}

MVREXPORT void MvrRangeBuffer::setPoseTaken(MvrPose p)
{
  myBufferPose = p;
}

MVREXPORT MvrPose MvrRangeBuffer::getEncoderPoseTaken() const
{
  return myEncoderBufferPose;
}

MVREXPORT void MvrRangeBuffer::setEncoderPoseTaken(MvrPose p)
{
  myEncoderBufferPose = p;
}

/**
   If the new size is smaller than the current buffer it chops off the 
   readings that are excess from the oldest readings... if the new size
   is larger then it just leaves room for the buffer to grow
   @param size number of readings to set the buffer to
*/
MVREXPORT void MvrRangeBuffer::setSize(size_t size) 
{
  mySize = size;
  // if its smaller then chop the lists down to size
  while (myInvalidBuffer.size() + myBuffer.size() > mySize)
  {
    if ((myRevIterator = myInvalidBuffer.rbegin()) != myInvalidBuffer.rend())
    {
      myReading = (*myRevIterator);
      myInvalidBuffer.pop_back();
      delete myReading;
    }
    else if ((myRevIterator = myBuffer.rbegin()) != myBuffer.rend())
    {
      myReading = (*myRevIterator);
      myBuffer.pop_back();
      delete myReading;
    }
  }
}

/** 
    This function returns a pointer to a list that has all of the readings
    in it.  This list is mostly for reference, ie for finding some 
    particular value or for using the readings to draw them.  Don't do 
    any modification at all to the list unless you really know what you're 
    doing... and if you do you'd better lock the rangeDevice this came from
    so nothing messes with the list while you are doing so.
    @return the list of positions this range buffer has
*/
MVREXPORT const std::list<MvrPoseWithTime *> *MvrRangeBuffer::getBuffer(void) const
{ 
  return &myBuffer; 
}

/** 
    This function returns a pointer to a list that has all of the readings
    in it.  This list is mostly for reference, ie for finding some 
    particular value or for using the readings to draw them.  Don't do 
    any modification at all to the list unless you really know what you're 
    doing... and if you do you'd better lock the rangeDevice this came from
    so nothing messes with the list while you are doing so.
    @return the list of positions this range buffer has
*/
MVREXPORT std::list<MvrPoseWithTime *> *MvrRangeBuffer::getBuffer(void)
{ 
  return &myBuffer; 
}


/**
   Gets the closest reading in a region defined by startAngle going to 
   endAngle... going counterclockwise (neg degrees to poseitive... with 
   how the robot is set up, thats counterclockwise)... from -180 to 180... 
   this means if you want the slice between 0 and 10 degrees,
   you must enter it as 0, 10, if you do 10, 0 you'll get the 350 degrees
   between 10 and 0... be especially careful with negative... for example
   -30 to -60 is everything from -30, around through 0, 90, and 180 back to 
   -60... since -60 is actually to clockwise of -30
   @param startAngle where to start the slice
   @param endAngle where to end the slice, going clockwise from startAngle
   @param startPos the position to find the closest reading to (usually
   the robots position)
   @param maxRange the maximum range to return (and what to return if nothing
   found)
   @param angle a pointer return of the angle to the found reading
   @return if the return is >= 0 and <= maxRange then this is the distance
   to the closest reading, if it is >= maxRange, then there was no reading 
   in the given section
*/
MVREXPORT double MvrRangeBuffer::getClosestPolar(double startAngle, 
					       double endAngle, 
					       MvrPose startPos, 
					       unsigned int maxRange,
					       double *angle) const
{
  return getClosestPolarInList(startAngle, endAngle, 
			       startPos, maxRange, angle, &myBuffer);
}

MVREXPORT double MvrRangeBuffer::getClosestPolarInList(
	double startAngle, double endAngle, MvrPose startPos, 
	unsigned int maxRange, double *angle, 
	const std::list<MvrPoseWithTime *> *buffer)
{
  double closest;
  bool foundOne = false;
  std::list<MvrPoseWithTime *>::const_iterator it;
  MvrPoseWithTime *reading;
  double th;
  double closeTh;
  double dist;
  double angle1, angle2;

  startAngle = MvrMath::fixAngle(startAngle);
  endAngle = MvrMath::fixAngle(endAngle);

  for (it = buffer->begin(); it != buffer->end(); ++it)
  {
    reading = (*it);

    angle1=startPos.findAngleTo(*reading);
    angle2=startPos.getTh();
    th = MvrMath::subAngle(angle1, angle2);
    if (MvrMath::angleBetween(th, startAngle, endAngle))
    {
      if (!foundOne || (dist = reading->findDistanceTo(startPos)) < closest)
      {
	closeTh = th;	
	if (!foundOne)
	  closest = reading->findDistanceTo(startPos);
	else
	  closest = dist;
	foundOne = true;
      }
    }
  }
  if (!foundOne)
    return maxRange;
  if (angle != NULL)
    *angle = closeTh;
  if (closest > maxRange)
    return maxRange;
  else
    return closest;  
}

/**
   Gets the closest reading in a region defined by two points (opposeite points
   of a rectangle).
   @param x1 the x coordinate of one of the rectangle points
   @param y1 the y coordinate of one of the rectangle points
   @param x2 the x coordinate of the other rectangle point
   @param y2 the y coordinate of the other rectangle point
   @param startPos the position to find the closest reading to (usually
   the robots position)
   @param maxRange the maximum range to return (and what to return if nothing
   found)
   @param readingPos a pointer to a position in which to store the location of
   the closest position
   @param targetPose the origin of the local coords for the definition of the 
   coordinates, e.g. MvrRobot::getPosition() to center the box on the robot
   @return if the return is >= 0 and <= maxRange then this is the distance
   to the closest reading, if it is >= maxRange, then there was no reading 
   in the given section
*/
MVREXPORT double MvrRangeBuffer::getClosestBox(double x1, double y1, double x2,
					     double y2, MvrPose startPos,
					     unsigned int maxRange, 
					     MvrPose *readingPos,
					     MvrPose targetPose) const
{
  return getClosestBoxInList(x1, y1, x2, y2, startPos, maxRange, readingPos, 
			     targetPose, &myBuffer);
}

/**
   Get closest reading in a region defined by two points (opposeite points
   of a rectangle) from a given list readings (rather than the readings
   stored in an MvrRangeBuffer)

   @param x1 the x coordinate of one of the rectangle points
   @param y1 the y coordinate of one of the rectangle points
   @param x2 the x coordinate of the other rectangle point
   @param y2 the y coordinate of the other rectangle point
   @param startPos the position to find the closest reading to (usually
   the robots position)
   @param maxRange the maximum range to return (and what to return if nothing
   found)
   @param readingPos a pointer to a position in which to store the location of
   the closest position
   @param targetPose the origin of the local coords for the definition of the 
   coordinates, normally just MvrRobot::getPosition()
   @param buffer Use the reading positions from this list 
   @param targetPose the pose to see if we're closest too (in local coordinates), this should nearly always be the default of 0 0 0
   @return if the return is >= 0 and <= maxRange then this is the distance
   to the closest reading, if it is >= maxRange, then there was no reading 
   in the given section
*/
MVREXPORT double MvrRangeBuffer::getClosestBoxInList(
	double x1, double y1, double x2, double y2, MvrPose startPos,
	unsigned int maxRange, MvrPose *readingPos, MvrPose targetPose,
	const std::list<MvrPoseWithTime *> *buffer)

{
  double closest = maxRange;
  double dist;
  MvrPose closestPos;
  std::list<MvrPoseWithTime *>::const_iterator it;
  MvrTransform trans;
  MvrPoseWithTime pose;
  MvrPose zeroPos;
  
  double temp;

  zeroPos.setPose(0, 0, 0);
  trans.setTransform(startPos, zeroPos);

  if (x1 >= x2)
  {
    temp = x1, 
    x1 = x2;
    x2 = temp;
  }
  if (y1 >= y2)
  {
    temp = y1, 
    y1 = y2;
    y2 = temp;
  }
  
  for (it = buffer->begin(); it != buffer->end(); ++it)
  {
    pose = trans.doTransform(*(*it));

    // see if its in the box
    if (pose.getX() >= x1 && pose.getX() <= x2 &&
	pose.getY() >= y1 && pose.getY() <= y2)
    {
      dist = pose.findDistanceTo(targetPose);
      //pose.log();
      if (dist < closest)
      {
	closest = dist;
	closestPos = pose;
      }
    }
  }

  if (readingPos != NULL)
    *readingPos = closestPos;
  if (closest > maxRange)
    return maxRange;
  else
    return closest;
}

/** 
    Applies a transform to the buffers.. this is mostly useful for translating
    to/from local/global coords, but may have other uses
    @param trans the transform to apply to the data
*/    
MVREXPORT void MvrRangeBuffer::applyTransform(MvrTransform trans)
{
  trans.doTransform(&myBuffer);
}

MVREXPORT void MvrRangeBuffer::clear(void)
{
  beginRedoBuffer();
  endRedoBuffer();
}

MVREXPORT void MvrRangeBuffer::reset(void)
{
  clear();
}

MVREXPORT void MvrRangeBuffer::clearOlderThan(int milliSeconds)
{
  std::list<MvrPoseWithTime *>::iterator it;

  beginInvalidationSweep();
  for (it = myBuffer.begin(); it != myBuffer.end(); ++it)
  {
    if ((*it)->getTime().mSecSince() > milliSeconds)
      invalidateReading(it);
  }
  endInvalidationSweep();
}

MVREXPORT void MvrRangeBuffer::clearOlderThanSeconds(int seconds)
{
  clearOlderThan(seconds*1000);
}

/**
   To redo the buffer means that you want to replace all
   of the readings in the buffer with new pose values, and get rid of the 
   readings that you didn't update with new values (invalidate them).  
   The three functions beginRedoBuffer(), 
   redoReading(), and endRedoBuffer() are all made to enable you to do this.
   First call beginRedoBuffer(). Then for each reading you want
   to update in the buffer, call redoReading(double x, double y), then
   when you are done, call endRedoBuffer().
**/     
MVREXPORT void MvrRangeBuffer::beginRedoBuffer(void)
{
  myRedoIt = myBuffer.begin();
  myHitEnd = false;
  myNumRedone = 0;
}

/**
   For a description of how to use this, see beginRedoBuffer()
   @param x the x param of the coord to add to the buffer
   @param y the x param of the coord to add to the buffer
*/
MVREXPORT void MvrRangeBuffer::redoReading(double x, double y)
{
  if (myRedoIt != myBuffer.end() && !myHitEnd)
  {
    (*myRedoIt)->setPose(x, y);
    myRedoIt++;
  }
  // if we don't, add more (its just moving from buffers here, 
  //but let the class for this do the work
  else
  {
    addReading(x,y);
    myHitEnd = true;
  }
  myNumRedone++;
}

/**
   For a description of how to use this, see beginRedoBuffer()
**/
MVREXPORT void MvrRangeBuffer::endRedoBuffer(void)
{
  if (!myHitEnd)
  {
    // now we get rid of the extra readings on the end
    beginInvalidationSweep();
    while (myRedoIt != myBuffer.end())
    {
      invalidateReading(myRedoIt);
      myRedoIt++;
    }
    endInvalidationSweep();
  } 
}

/**
   @param x the x position of the reading
   @param y the y position of the reading

   @param closeDistSquared if the new reading is within
   closeDistSquared distanceSquared of an old point the old point is
   just updated for time

   @param wasAdded pointed to set to true if the reading was added, or false if not
*/
MVREXPORT void MvrRangeBuffer::addReadingConditional(
	double x, double y, double closeDistSquared, bool *wasAdded)
{
  if (closeDistSquared >= 0)
  {  
    std::list<MvrPoseWithTime *>::iterator it;
    MvrPoseWithTime *pose;
    for (it = myBuffer.begin(); it != myBuffer.end(); ++it)
    {
      pose = (*it);
      if (MvrMath::squaredDistanceBetween(pose->getX(), pose->getY(),
					 x, y) < closeDistSquared)
      {
	pose->setTimeToNow();
	if (wasAdded != NULL)
	  *wasAdded = false;
	return;
      }
    }
  }

  if (wasAdded != NULL)
    *wasAdded = true;
  addReading(x, y);    
}

/**
   @param x the x position of the reading
   @param y the y position of the reading
*/
MVREXPORT void MvrRangeBuffer::addReading(double x, double y) 
{
  if (myBuffer.size() < mySize)
  {
    if ((myIterator = myInvalidBuffer.begin()) != myInvalidBuffer.end())
    {
      myReading = (*myIterator);
      myReading->setPose(x, y);
      myReading->setTimeToNow();
      myBuffer.push_front(myReading);
      myInvalidBuffer.pop_front();
    }
    else
      myBuffer.push_front(new MvrPoseWithTime(x, y));
  }
  else if ((myRevIterator = myBuffer.rbegin()) != myBuffer.rend())
  {
    myReading = (*myRevIterator);
    myReading->setPose(x, y);
    myReading->setTimeToNow();
    myBuffer.pop_back();
    myBuffer.push_front(myReading);
  }
}

/**
   This is a set of funkiness used to invalid readings in the buffer.
   It is fairly complicated.  But what you need to do, is set up the invalid
   sweeping with beginInvalidationSweep, then walk through the list of 
   readings, and pass the iterator to a reading you want to invalidate to 
   invalidateReading, then after you are all through walking the list call 
   endInvalidationSweep.  Look at the description of getBuffer for additional
   warnings.
   @see invalidateReading
   @see endInvalidationSweep
*/
void MvrRangeBuffer::beginInvalidationSweep(void)
{
  myInvalidSweepList.clear();
}

/**
   See the description of beginInvalidationSweep, it describes how to use
   this function.
   @param readingIt the ITERATOR to the reading you want to get rid of
   @see beginInvaladationSweep
   @see endInvalidationSweep
*/
MVREXPORT void MvrRangeBuffer::invalidateReading(
	std::list<MvrPoseWithTime*>::iterator readingIt)
{
  myInvalidSweepList.push_front(readingIt);
}

/**
   See the description of beginInvalidationSweep, it describes how to use
   this function.
   @see beginInvalidationSweep
   @see invalidateReading
*/
void MvrRangeBuffer::endInvalidationSweep(void)
{
  while ((myInvalidIt = myInvalidSweepList.begin()) != 
	 myInvalidSweepList.end())
  {
    //printf("nuked one before %d %d\n", myBuffer.size(), myInvalidBuffer.size());
    myReading = (*(*myInvalidIt));
    myInvalidBuffer.push_front(myReading);
    myBuffer.erase((*myInvalidIt));
    myInvalidSweepList.pop_front();
    //printf("after %d %d\n", myBuffer.size(), myInvalidBuffer.size());
  }
}

/**
   Copy the readings from this buffer to a vector stored within
   this object, and return a pointer to that vector. 
   Note that the actual vector object is stored within MvrRangeBuffer,
   be careful if accessing it from multiple threads.
   @return Pointer to reading vector.
*/
MVREXPORT std::vector<MvrPoseWithTime> *MvrRangeBuffer::getBufferAsVector(void)
{
  std::list<MvrPoseWithTime *>::iterator it;

  myVector.reserve(myBuffer.size());
  myVector.clear();
  // start filling the array with the buffer until we run out of
  // readings or its full
  for (it = myBuffer.begin(); it != myBuffer.end(); it++)
  {
    myVector.insert(myVector.begin(), *(*it));
  }
  return &myVector;
}


