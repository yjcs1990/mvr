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
#ifndef ARRANGEBUFFER_H
#define ARRANGEBUFFER_H

#include "ariaUtil.h"
#include "ariaTypedefs.h"
#include "MvrTransform.h"
#include <list>
#include <vector>

/// This class is a buffer that holds ranging information
class MvrRangeBuffer
{
public:
  /// Constructor
  AREXPORT MvrRangeBuffer(int size);
  /// Destructor
  AREXPORT virtual ~ArRangeBuffer();
  /// Gets the size of the buffer
  AREXPORT size_t getSize(void) const;
  /// Sets the size of the buffer
  AREXPORT void setSize(size_t size);
  /// Gets the pose of the robot when readings were taken
  AREXPORT MvrPose getPoseTaken() const;
  /// Sets the pose of the robot when readings were taken
  AREXPORT void setPoseTaken(MvrPose p);
  /// Gets the encoder pose of the robot when readings were taken
  AREXPORT MvrPose getEncoderPoseTaken() const;
  /// Sets the pose of the robot when readings were taken
  AREXPORT void setEncoderPoseTaken(MvrPose p);
  /// Adds a new reading to the buffer
  AREXPORT void addReading(double x, double y);
  /// Adds a new reading to the buffer if some conditions are met
  AREXPORT void addReadingConditional(double x, double y, 
				      double closeDistSquared, 
				      bool *wasAdded = NULL);
#ifndef SWIG
  /// Begins a walk through the getBuffer list of readings
  AREXPORT void beginInvalidationSweep(void);
  /// While doing an invalidation sweep a reading to the list to be invalidated
  AREXPORT void invalidateReading(std::list<ArPoseWithTime*>::iterator readingIt);
  /// Ends the invalidation sweep
  AREXPORT void endInvalidationSweep(void);

  /** @brief Gets a pointer to a list of readings
   *  @swigomit
   */
  AREXPORT const std::list<ArPoseWithTime *> *getBuffer(void) const;
#endif
  /// Gets a pointer to a list of readings
  AREXPORT std::list<ArPoseWithTime *> *getBuffer(void);

  /// Gets the closest reading, on a polar system 
  AREXPORT double getClosestPolar(double startAngle, double endAngle, 
				  MvrPose position, unsigned int maxRange,
				  double *angle = NULL) const;
  /// Gets the closest reading, from a rectangular box, in robot LOCAL coords
  AREXPORT double getClosestBox(double x1, double y1, double x2, double y2,
				ArPose position, unsigned int maxRange, 
				ArPose *readingPos = NULL,
				ArPose targetPose = MvrPose(0, 0, 0)) const;
  /// Applies a transform to the buffer
  AREXPORT void applyTransform(MvrTransform trans);
  /// Clears all the readings in the range buffer
  AREXPORT void clear(void);
  /// Resets the readings older than this many seconds
  AREXPORT void clearOlderThan(int milliSeconds);
  /// Resets the readings older than this many seconds
  AREXPORT void clearOlderThanSeconds(int seconds);
  /// same as clear, but old name
  AREXPORT void reset(void);
  /// This begins a redoing of the buffer
  AREXPORT void beginRedoBuffer(void);
  /// Add a reading to the redoing of the buffer
  AREXPORT void redoReading(double x, double y);   
  /// End redoing the buffer
  AREXPORT void endRedoBuffer(void);
  /// Gets the buffer as an array instead of as a std::list
  AREXPORT std::vector<ArPoseWithTime> *getBufferAsVector(void);
  /// Gets the closest reading, from an arbitrary buffer
  AREXPORT static double getClosestPolarInList(
	  double startAngle, double endAngle, MvrPose position, 
	  unsigned int maxRange, double *angle, 
	  const std::list<ArPoseWithTime *> *buffer);
  /// Gets the closest reading, from an arbitrary buffer
  AREXPORT static double getClosestBoxInList(
	  double x1, double y1, double x2, double y2, MvrPose position, 
	  unsigned int maxRange, MvrPose *readingPos, 
	  MvrPose targetPose, const std::list<ArPoseWithTime *> *buffer);
protected:
  std::vector<ArPoseWithTime> myVector;
  MvrPose myBufferPose;		// where the robot was when readings were acquired
  MvrPose myEncoderBufferPose;		// where the robot was when readings were acquired

  std::list<ArPoseWithTime *> myBuffer;
  std::list<ArPoseWithTime *> myInvalidBuffer;
  std::list<std::list<ArPoseWithTime *>::iterator > myInvalidSweepList;
  std::list<std::list<ArPoseWithTime *>::iterator >::iterator myInvalidIt;
  std::list<ArPoseWithTime *>::iterator myRedoIt;
  int myNumRedone;
  bool myHitEnd;
  
  size_t mySize;
  std::list<ArPoseWithTime *>::reverse_iterator myRevIterator;
  std::list<ArPoseWithTime *>::iterator myIterator;
  
  MvrPoseWithTime * myReading;
};

#endif // ARRANGEBUFFER_H
