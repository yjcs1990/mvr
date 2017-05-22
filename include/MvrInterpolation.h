/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrInterpolation.h
 > Description  : Store a buffer of positions (ArPose objects) with associated timestamps, can
                  be queried to interpolate (or optionally extrapolate) a pose for any arbitrary
                  timestamp. 
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRINTERPOLATION_H
#define MVRINTERPOLATION_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

/// @ingroup UtilityClasses
class MvrInterpolation
{
public:
  /// Constructor
  MVREXPORT MvrInterpolation(size_t numberOfReadings=100);
  /// Destructor
  MVREXPORT virtual ~MvrInterpolation();
  /// Adds a new reading
  MVREXPORT bool addReading(MvrTime timeOfReading, MvrPose position);
  /// Finds a position
  MVREXPORT int getPose(MvrTime timeStamp, MvrPose *position, MvrPoseWithTime *lastData=NULL);
  /// Sets the name
  MVREXPORT void setName(const char *name);
  /// Gets the name
  MVREXPORT const char *getName(void);
  /// Sets the aloowed milliseconds for predition
  MVREXPORT void setAllowedMSForPredition(int ms=-1);
  /// Gets the allowed millseconds for predition
  MVREXPORT int getAllowedMSForPredition(void);
  /// Sets the allowed percentage for prediction
  MVREXPORT void setAllowedPercentageForPrediction(int percentage=5000);
  /// Gets the allowed percentage for predictionGet
  MVREXPORT int getAllowedPercentageForPrediction(void);
  /// Sets if we should log events for this interpolation
  MVREXPORT void setLogPrediction(bool logPrediction=false);
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