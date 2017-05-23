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
                           int maxSecondsToKeepCumulative=0,)
protected:
}
#endif  // MVRRANGEDEVICE_H