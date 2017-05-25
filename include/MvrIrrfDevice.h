/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrIrrfDevice.h
 > Description  : A class for connecting to a PB-9 and managing the resulting data
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRIRRFDEVICE_H
#define MVRIRRFDEVICE_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctor.h"

#include "MvrRobot.h"

/// A class for connecting to a PB-9 and managing the resulting data

class MvrIrrfDevice : public MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrIrrfDevice(size_t currentBufferSize = 91,
                          size_t cumulativeBufferSize = 273,
                          const char * name = "irrf");
  /// Destructor
  MVREXPORT virtual ~MvrIrrfDevice();

  /// The packet handler for use when connecting to an H8 micro-controller
  MVREXPORT bool packetHandler(MvrRobotPacket *packet);

  /// Maximum range for a reading to be added to the cumulative buffer (mm)
  MVREXPORT void setCumulativeMaxRange(double r) { myCumulativeMaxRange = r; }
  MVREXPORT virtual void setRobot(MvrRobot *);
protected:
  MvrRetFunctor1C<bool, MvrIrrfDevice, MvrRobotPacket *> myPacketHandler;
  MvrTime myLastReading;
  MVREXPORT void processReadings(void);
  double myCumulativeMaxRange;
  double myFilterNearDist;
  double myFilterFarDist;
  std::map<int, MvrSensorReading *> myIrrfReadings;
};

#endif // MVRIRRFDEVICE_H
