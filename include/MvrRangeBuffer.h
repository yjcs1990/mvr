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

#include "mvriaUtil.h"
#include "mvriaTypedefs.h"
#include "MvrTransform.h"
#include <list>
#include <vector>

/// This class is a buffer that holds ranging information
class MvrRangeBuffer
{
public:
  /// Constructor
  MVREXPORT MvrRangeBuffer(int size);
  /// Destructor
  MVREXPORT virtual ~MvrRangeBuffer();
  /// Gets the size of the buffer
  MVREXPORT size_t getSize(void) const;
  /// Sets the size of the buffer
  MVREXPORT void setSize(size_t size);
  /// Gets the pose of the robot when readings were taken
  MVREXPORT MvrPose getPoseTaken() const;
  /// Sets the pose of the robot when readings were taken
  MVREXPORT void setPoseTaken(MvrPose p);
  /// Gets the encoder pose of the robot when readings were taken
  MVREXPORT MvrPose getEncoderPoseTaken() const;
  /// Sets the pose of the robot when readings were taken
  MVREXPORT void setEncoderPoseTaken(MvrPose p);
  /// Adds a new reading to the buffer
  MVREXPORT void addReading(double x, double y);
  /// Adds a new reading to the buffer if some conditions are met
  MVREXPORT void addReadingConditional(double x, double y, 
				      double closeDistSquared, 
				      bool *wasAdded = NULL);
#ifndef SWIG
  /// Begins a walk through the getBuffer list of readings
  MVREXPORT void beginInvalidationSweep(void);
  /// While doing an invalidation sweep a reading to the list to be invalidated
  MVREXPORT void invalidateReading(std::list<MvrPoseWithTime*>::iterator readingIt);
  /// Ends the invalidation sweep
  MVREXPORT void endInvalidationSweep(void);

  /** @brief Gets a pointer to a list of readings
   *  @swigomit
   */
  MVREXPORT const std::list<MvrPoseWithTime *> *getBuffer(void) const;
#endif
  /// Gets a pointer to a list of readings
  MVREXPORT std::list<MvrPoseWithTime *> *getBuffer(void);

  /// Gets the closest reading, on a polar system 
  MVREXPORT double getClosestPolar(double startAngle, double endAngle, 
				  MvrPose position, unsigned int maxRange,
				  double *angle = NULL) const;
  /// Gets the closest reading, from a rectangular box, in robot LOCAL coords
  MVREXPORT double getClosestBox(double x1, double y1, double x2, double y2,
				MvrPose position, unsigned int maxRange, 
				MvrPose *readingPos = NULL,
				MvrPose targetPose = MvrPose(0, 0, 0)) const;
  /// Applies a transform to the buffer
  MVREXPORT void applyTransform(MvrTransform trans);
  /// Clears all the readings in the range buffer
  MVREXPORT void clear(void);
  /// Resets the readings older than this many seconds
  MVREXPORT void clearOlderThan(int milliSeconds);
  /// Resets the readings older than this many seconds
  MVREXPORT void clearOlderThanSeconds(int seconds);
  /// same as clear, but old name
  MVREXPORT void reset(void);
  /// This begins a redoing of the buffer
  MVREXPORT void beginRedoBuffer(void);
  /// Add a reading to the redoing of the buffer
  MVREXPORT void redoReading(double x, double y);   
  /// End redoing the buffer
  MVREXPORT void endRedoBuffer(void);
  /// Gets the buffer as an array instead of as a std::list
  MVREXPORT std::vector<MvrPoseWithTime> *getBufferAsVector(void);
  /// Gets the closest reading, from an arbitrary buffer
  MVREXPORT static double getClosestPolarInList(
	  double startAngle, double endAngle, MvrPose position, 
	  unsigned int maxRange, double *angle, 
	  const std::list<MvrPoseWithTime *> *buffer);
  /// Gets the closest reading, from an arbitrary buffer
  MVREXPORT static double getClosestBoxInList(
	  double x1, double y1, double x2, double y2, MvrPose position, 
	  unsigned int maxRange, MvrPose *readingPos, 
	  MvrPose targetPose, const std::list<MvrPoseWithTime *> *buffer);
protected:
  std::vector<MvrPoseWithTime> myVector;
  MvrPose myBufferPose;		// where the robot was when readings were acquired
  MvrPose myEncoderBufferPose;		// where the robot was when readings were acquired

  std::list<MvrPoseWithTime *> myBuffer;
  std::list<MvrPoseWithTime *> myInvalidBuffer;
  std::list<std::list<MvrPoseWithTime *>::iterator > myInvalidSweepList;
  std::list<std::list<MvrPoseWithTime *>::iterator >::iterator myInvalidIt;
  std::list<MvrPoseWithTime *>::iterator myRedoIt;
  int myNumRedone;
  bool myHitEnd;
  
  size_t mySize;
  std::list<MvrPoseWithTime *>::reverse_iterator myRevIterator;
  std::list<MvrPoseWithTime *>::iterator myIterator;
  
  MvrPoseWithTime * myReading;
};

#endif // ARRANGEBUFFER_H
