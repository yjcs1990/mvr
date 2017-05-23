/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRangeBuffer.h
 > Description  : This class is a buffer that holds ranging information
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRRANGEBUFFER_H
#define MVRRANGEBUFFER_H

#include "mvriaUtil.h"
#include "mvriaTypedefs.h"
#include "MvrTransform.h"
#include <list>
#include <vector>

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
  /// Gets the encode pose of the robot when readings were taken
  MVREXPORT MvrPose getEncoderPoseTaken() const;
  /// Sets the pose of the robot when readings were taken
  MVREXPORT void setEncoderPoseTaken(MvrPose p);
  /// Adds a new reading to the buffer
  MVREXPORT void addReading(double x, double y);
  /// Adds a new reading to the buffer if some coditions are met
  MVREXPORT void addReadingConditional(double x, double y,
              double closeDistSquared, bool *wasAdded=NULL);
#ifndef SWIG
  /// Begins a walk through the getBuffer list of readings
  MVREXPORT void beginInvalidationSweep(void);
  /// While doing an invalidation sweep a reading to the list to be invalidated
  MVREXPORT void invalidateReading(std::list<MvrPoseWithTime *>::iterator readingIt);
  /// Ends the invalidation sweep
  MVREXPORT void endInvalidationSweep(void);
  /// @brief Gets a pointer to a list of readings
  MVREXPORT const std::list<MvrPoseWithTime *> *getBuffer(void) const;  
#endif // SWIG
  /// Gets a pointer to a list of readings
  MVREXPORT std::list<MvrPoseWithTime *> *getBuffer(void);
  /// Gets the closest reading, on a polar system
  MVREXPORT double getClosestPolar(double startAngle, double endAngle,
            MvrPose position, unsigned int maxRange, double *angle=NULL) const;
  /// Gets the closest reading, from a rectranglar box, in robot LOCAL coords
  MVREXPORT double getClosestBox(double x1, double y1, double x2, double y2,
            MvrPose position, unsigned int maxRange,
            MvrPose *readingPos=NULL, MvrPose targetPose=MvrPose(0,0,0)) const;
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
  MvrPose myBufferPose;		        // where the robot was when readings were acquired
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

#endif  // MVRRANGEBUFFER_H