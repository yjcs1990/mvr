#ifndef MVRINTERPOLATION_H
#define MVRINTERPOLATION_H
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
/// A class for time readings and measuring durations
/** 
    This class is for timing durations or time between events.
    The time values it stores are relative to an abritrary starting time; it
    does not correspond to "real world" or "wall clock" time in any way,
    so DON'T use this for keeping track of what time it is, 
    just for timestamps and relative timing (e.g. "this loop needs to sleep another 100 ms").

    The recommended methods to use are setToNow() to reset the time,
    mSecSince() to obtain the number of milliseconds elapsed since it was
    last reset (or secSince() if you don't need millisecond precision), and
    mSecSince(MvrTime) or secSince(MvrTime) to find the difference between 
    two MvrTime objects.

    On systems where it is supported this will use a monotonic clock,
    this is an ever increasing system that is not dependent on what
    the time of day is set to.  Normally for linux gettimeofday is
    used, but if the time is changed forwards or backwards then bad
    things can happen.  Windows uses a time since bootup, which
    functions the same as the monotonic clock anyways.  You can use
    MvrTime::usingMonotonicClock() to see if this is being used.  Note
    that an MvrTime will have had to have been set to for this to be a
    good value... Mvria::init does this however, so that should not be
    an issue.  It looks like the monotonic clocks won't work on linux
    kernels before 2.6.

  @ingroup UtilityClasses
*/
class MvrInterpolation
{
public:
  /// Constructor
  MVREXPORT MvrInterpolation(size_t numberOfReadings = 100);
  /// Destructor
  MVREXPORT virtual ~MvrInterpolation();
  /// Adds a new reading
  MVREXPORT bool addReading(MvrTime timeOfReading, MvrPose position);
  /// Finds a position
  MVREXPORT int getPose(MvrTime timeStamp, MvrPose *position, MvrPoseWithTime *lastData = NULL);
  /// Sets the name
  MVREXPORT void setName(const char *name);
  /// Gets the name
  MVREXPORT const char *getName(void);
  /// Sets the allowed milliseconds for prediction
  MVREXPORT void setAllowedMSForPrediction(int ms = -1);
  /// Sets the allowed milliseconds for prediction
  MVREXPORT int getAllowedMSForPrediction(void);
  /// Sets the allowed percentage for prediction
  MVREXPORT void setAllowedPercentageForPrediction(int percentage = 5000);
  /// Sets the allowed milliseconds for prediction
  MVREXPORT int getAllowedPercentageForPrediction(void);
  /// Sets if we should log events for this interpolation
  MVREXPORT void setLogPrediction(bool logPrediction = false);
  /// Gets if we should log events for this interpolation
  MVREXPORT bool getLogPrediction(void);
  /// Sets the number of readings this instance holds back in time
  MVREXPORT void setNumberOfReadings(size_t numberOfReadings);
  /// Gets the number of readings this instance holds back in time
  MVREXPORT size_t getNumberOfReadings(void) const;
  /// Empties the interpolated positions
  MVREXPORT void reset(void);
protected:
  MvrMutex myDataMutex;
  std::string myName;
  std::list<MvrTime> myTimes;
  std::list<MvrPose> myPoses;
  size_t mySize;
  bool myLogPrediction;
  int myAllowedMSForPrediction;
  int myAllowedPercentageForPrediction;
};

#endif  // MVRINTERPOLATION_H
