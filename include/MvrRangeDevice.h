/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRangeDevice.h
 > Description  : The base class for all sensing devices which return range
                  information from the device (mounted on a robot) to an object in
                  the environment.
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRRANGEDEVICE_H
#define MVRRANGEDEVICE_H

#include "mvriaTypedefs.h"
#include "MvrRangeBuffer.h"
#include "MvrSensorReading.h"
#include "MvrDrawingData.h"
#include "MvrMutex.h"
#include <set>

class MvrRobot;

/** 
    @brief The base class for all sensing devices which return range
    information from the device (mounted on a robot) to an object in
    the environment.

    This class maintains two MvrRangeBuffer objects: a current buffer
    (getCurrentBuffer())
    for storing very recent readings, and a cumulative buffer 
    (getCumulativeBuffer()) for a
    longer history of readings.  The maximum sizes of each buffer can
    be set in the constructor or resized later. Range device readings
    are most often represented as a point in space (X,Y) where the
    sensor detected an object.  (Therefore an MvrPose object may only
    have X and Y components set).  

    Some devices provide an original set of "raw" MvrSensorReading 
    objects (getRawReadings()) (that it used to add data to the current buffer) 
    which may also include extra device specific information as well.
    Not all devices provide raw readings.

    Subclasses are used for specific sensor implementations like
    MvrLaser and subclasses for laser rangefinders and MvrSonarDevice for the Pioneer sonar
    array. It can also be useful to treat "virtual" objects like
    forbidden areas specified by the user in a map like range devices.
    Some of these subsclasses may use a separate thread to update the
    range reading buffers, and so this base class provides "lock" and
    "unlock" methods which you should use when accessing device data.

    A range device may have an MvrRobot object associated with it. A
    range device may also be associated with an MvrRobot by calling
    MvrRobot::addRangeDevice().  MvrRobot provides functions which
    operate on all such associated MvrRangeDevice objects.  This is a
    convenient (and thread-safe) way to access all range device data
    without depending on a specific set of individual range
    devices. For example, you can find the closest reading in a box or
    a polar section, no matter if that reading originated from the
    sonar, a laser, or other device.  

    @ingroup ImportantClasses
**/
class MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrRangeDevice(size_t currentBufferSize,
                           size_t cumulativeBufferSize, 
                           const char *name, unsigned int maxRange,
                           int maxSecondsToKeepCurrent=0,
                           int maxSecondsToKeepCumulative=0,
                           double maxDistToKeepCumulative=0,
                           bool locationDepedent = false);
  /// Destructor
  MVREXPORT virtual ~MvrRangeDevice();
  /// Gets the name of the device
  MVREXPORT virtual const char *getName(void) const;
  /// Sets the robot this devices is attatched to
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  /// Gets the robot this device is attatched to
  MVREXPORT virtual MvrRobot *getRobot(void);
  /// Sets the maximum size of the buffer for current readings
  MVREXPORT virtual void setCurrentBufferSize(size_t size);
  /// Gets the maximum size of the buffer for the current readings
  MVREXPORT virtual size_t getCurrentBufferSize(void) const;
  /// Adds a reading to the buffer
  MVREXPORT virtual void addReading(double x, double y, bool *wasAdded = NULL);
  /// Gets if this device is location dependent or not
  bool isLocationDependent(void) { return myIsLocationDependent; }
  /// Gets the closest current reading in the given polar region
  MVREXPORT virtual double currentReadingPolar(double startAngle, 
                                               double endAngle,
                                               double *angle = NULL) const;
  /// Gets the closest cumulative reading in the given polar region
  MVREXPORT virtual double cumulativeReadingPolar(double startAngle, 
                                                  double endAngle,
                                                  double *angle = NULL) const;
  /// Gets the closest current reading from the given box region
  MVREXPORT virtual double currentReadingBox(double x1, double y1, double x2,
                                             double y2, 
                                             MvrPose *readingPos = NULL) const;
  /// Gets the closest current reading from the given box region
  MVREXPORT virtual double cumulativeReadingBox(double x1, double y1, double x2,
                                                double y2, 
                                                MvrPose *readingPos = NULL) const;
#ifndef SWIG
  /// @brief Gets the current range buffer
  virtual const MvrRangeBuffer *getCurrentRangeBuffer(void) const
  { return &myCurrentBuffer; }
  /// @brief Gets the cumulative range buffer
  virtual const MvrRangeBuffer *getCumulativeRangeBuffer(void) const
  { return &myCumulativeBuffer; }
  /// @brief Gets the current buffer of readins
  virtual const std::list<MvrPoseWithTime *> *getCurrentBuffer(void) const
  { return myCurrentBuffer.getBuffer(); }
  /// @brief Gets the current buffer of readings
  virtual const std::list<MvrPoseWithTime *> *getCumulativeBuffer(void) const
  { return myCumulativeBuffer.getBuffer(); }
#endif  // SWIG       
  /// Gets the current range buffer
  virtual MvrRangeBuffer *getCurrentRangeBuffer(void)
  { return &myCurrentBuffer; }                      
  /// Gets the cumulative range buffer
  virtual MvrRangeBuffer *getCumulativeRangeBuffer(void) 
  { return &myCumulativeBuffer; }
  /// Gets the current buffer of readings
  virtual std::list<MvrPoseWithTime *> *getCurrentBuffer(void) 
  { return myCurrentBuffer.getBuffer(); }
  /// @brief Gets the current buffer of readings as a vector
  virtual std::vector<MvrPoseWithTime> *getCurrentBufferAsVector(void) 
  { return myCurrentBuffer.getBufferAsVector(); }
  /// Gets the current buffer of readings
  virtual std::list<MvrPoseWithTime *> *getCumulativeBuffer(void) 
  { return myCumulativeBuffer.getBuffer(); }
  /// @brief Gets the cumulative buffer of readings as a vector

  virtual std::vector<MvrPoseWithTime> *getCumulativeBufferAsVector(void) 
  { return myCumulativeBuffer.getBufferAsVector(); }

  /// Gets the raw unfiltered readings from the device

  virtual const std::list<MvrSensorReading *> *getRawReadings(void) const
  { return myRawReadings; }

  ///  Gets the raw unfiltered readings from the device into a vector 
  MVREXPORT virtual std::vector<MvrSensorReading> *getRawReadingsAsVector(void);

  /// Gets the raw unfiltered readings from the device (but pose takens are corrected)
  virtual const std::list<MvrSensorReading *> *getAdjustedRawReadings(void) const
  { return myAdjustedRawReadings; }
  ///  Gets the raw adjusted readings from the device into a vector 
  MVREXPORT virtual std::vector<MvrSensorReading> *getAdjustedRawReadingsAsVector(void);
  /// Sets the maximum seconds to keep current readings around
  void setMaxSecondsToKeepCurrent(int maxSecondsToKeepCurrent)
  { myMaxSecondsToKeepCurrent = maxSecondsToKeepCurrent; }
  /// gets the maximum seconds to keep current readings around
  int getMaxSecondsToKeepCurrent(void) { return myMaxSecondsToKeepCurrent; }
  /// Sets the minimum distance between current reading
  void setMinDistBetweenCurrent(double minDistBetweenCurrent)
  {
      myMinDistBetweenCurrent = minDistBetweenCurrent;
      myMinDistBetweenCurrentSquared = ( minDistBetweenCurrent * minDistBetweenCurrent);      
  }
  /// Gets the minimum distance between current readings
  double getMinDistBetweenCurrent(void)
  { return myMinDistBetweenCurrent; }
  /// gets the maximum seconds to keep cumulative readings around
  double setMaxSecondsToKeepCumulative(int maxSecondsToKeepCumulative)
  { myMaxSecondsToKeepCumulative = maxSecondsToKeepCumulative; }
  /// gets the maximum seconds to keep current readings around
  int getMaxSecondsToKeepCumulative(void)
  { return myMaxSecondsToKeepCumulative; }
  /// sets the maximum distance cumulative readings can be from current pose
  void setMaxDistToKeepCumulative(double maxDistToKeepCumulative)
  {
    myMaxDistToKeepCumulative = maxDistToKeepCumulative;
    myMaxDistToKeepCumulativeSquared = ( maxDistToKeepCumulative * maxDistToKeepCumulative) ;
  }
  /// sets the maximum distance cumulative readings can be from current pose
  double getMaxDistToKeepCumulative(void) { return myMaxDistToKeepCumulative; }
  /// Sets the minimum distance between cumulative readings
  void setMinDistBetweenCumulative(double minDistBetweenCumulative)
  {
      myMinDistBetweenCumulative = minDistBetweenCumulative;
      myMinDistBetweenCumulativeSquared = (minDistBetweenCumulative * minDistBetweenCumulative);
  }
  /// Gets the minimum distance between cumulative readings
  double getMinDistBetweenCumulative(void)
  { return myMinDistBetweenCumulative; }
  /// Sets the maximum distance a cumulative reading can be from the robot and still be inserted
  void setMaxInsertDistCumulative(double maxInsertDistCumulative)
  {
    myMaxInsertDistCumulative = maxInsertDistCumulative;
    myMaxInsertDistCumulativeSquared = (maxInsertDistCumulative * maxInsertDistCumulative);
  }
  /// Gets the maximum distance a cumulative reading can be from the robot and still be inserted
  double getMaxInsertDistCumulative(void)
  { return myMaxInsertDistCumulative; }

  /// clears all the current readings
  virtual void clearCurrentReadings(void) { myCurrentBuffer.clear(); }
  /// Clears all the cumulative readings
  virtual void clearCumulativeReadings(void) { myCumulativeBuffer.clear(); }
  /// Clears all the cumulative readings older than this number of milliseconds
  virtual void clearCumulativeOlderThan(int milliSeconds)
  { myCumulativeBuffer.clearOlderThan(milliSeconds); }
  /// Clears all the cumulative readings older than this number of seconds
  virtual void clearCumulativeOlderThanSeconds(int seconds)
  { myCumulativeBuffer.clearOlderThanSeconds(seconds); }
  /// Gets the maximum range for this device
  virtual unsigned int getMaxRange(void) const { return myMaxRange; }
  /// Sets the maximum range for this device
  virtual void setMaxRange(unsigned int maxRange) { myMaxRange = maxRange; }

  /// Applies a transform to the buffers
  MVREXPORT virtual void applyTransform(MvrTransform transform, bool doCumulative=true);
  /// Gets data used for visualizing the current buffer
  virtual MvrDrawingData *getCurrentDrawingData(void)
  { return myCurrentDrawingData; }
  /// Gets data used for visualizing the cumulative buffer
  virtual MvrDradingData *getCumulativeDrawingData(void)
  { return myCumulativeDrawingData; }
  /// Sets data for visualizing the current buffer
  MVREXPORT virtual void setCurrentDrawingData(MvrDrawingData *data, bool takeOwnershipOfData);
  /// Sets data for visualizing the cumulative buffer
  MVREXPORT virtual void setCumulativeDrawingData(MvrDrawing *data, bool takeOwnershipOfData);

  /// Lock this device
  MVREXPORT virtual int lockDevice() { return(myDeviceMutex.lock()); }
  /// Try to lock this device
  MVREXPORT virtual int tryLockDevice() { return(myDeviceMutex.tryLock()); }
  /// Unlock this device
  MVREXPORT virtual int unlockDevice() { return(myDeviceMutex.unlock()); }

  /// Internal function to filter the readings based on age and distance
  MVREXPORT void filterCallback(void);
protected:
  /**
    This call should be called by the range device every robot cycle
    before the range device makes new readings (and even if it isn't
    adding any that cycle)... it will adjust the raw readings by the
    robot odometry offset.  The robot should be locked when this
    happens (which should be the case if you're doing it in the robot
    callback). The code currently assumes that all readings were taken
    at the same point, so if that isn't true with your device then you
    can't use this mechanism.
  **/
  MVREXPORT void adjustRawReadings(bool interlaced);
  std::vector<MvrSensorReading> myRawReadingsVector;
  std::vector<MvrSensorReading> myAdjustedRawReadingsVector;
  std::string myName;
  MvrRobot *myRobot;
  unsigned int myMaxRange; 
  MvrRangeBuffer myCurrentBuffer;
  MvrRangeBuffer myCumulativeBuffer;

  int myMaxSecondsToKeepCurrent;
  double myMinDistBetweenCurrent;
  double myMinDistBetweenCurrentSquared;

  int myMaxSecondsToKeepCumulative;
  double myMaxDistToKeepCumulative;
  double myMaxDistToKeepCumulativeSquared;
  double myMinDistBetweenCumulative;
  double myMinDistBetweenCumulativeSquared;
  double myMaxInsertDistCumulative; 
  double myMaxInsertDistCumulativeSquared;
  MvrPose myMaxInsertDistCumulativePose;

  MvrFunctorC<MvrRangeDevice> myFilterCB;
  std::list<MvrSensorReading *> *myRawReadings;
  std::list<MvrSensorReading *> *myAdjustedRawReadings;
  MvrDrawingData *myCurrentDrawingData;
  bool myOwnCurrentDrawingData;
  MvrDrawingData *myCumulativeDrawingData;
  bool myOwnCumulativeDrawingData;
  MvrMutex myDeviceMutex;
  bool myIsLocationDependent;
};
#endif  // MVRRANGEDEVICE_H