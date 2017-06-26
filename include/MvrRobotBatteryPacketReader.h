/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotBatteryPacketReader.h
 > Description  : This class will read a config packet from the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月19日
 > Modify Time  : 2017年05月19日
***************************************************************************************************/
#ifndef MVRROBOTBATTERYPACKETREADER_H
#define MVRROBOTBATTERYPACKETREADER_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"

class MvrRobot;
class MvrRobotPacket;

class MvrRobotBatteryPacketReader
{
public:
  /// Constructor
  MVREXPORT MvrRobotBatteryPacketReader(MvrRobot *robot);
  /// Destructor
  MVREXPORT ~MvrRobotBatteryPacketReader();
  /// Request a single packet.. 
  MVREXPORT void requestSinglePacket(void);
  /// Request a continous stream of packets
  MVREXPORT void requestContinuousPackets(void);
  /// Stop the stream of packets
  MVREXPORT void stopPackets(void);
  /// See if we've requested packets
  MVREXPORT bool haveRequestedPackets(void);
  /// See if we've gotten the data
  bool hasPacketArrived(void) const { return myPacketArrived; }
  /// Gets the number of batteries
  int getNumBatteries(void) const { return myNumBatteries; }
  /// Gets the number of bytes per battery
  int getNumBytesPerBattery(void) const { return myNumBytesPerBattery; }
  /// Gets the flags1 for a particular battery
  int getFlags1(int battery) { return myFlags1[battery]; }
  /// Gets the flags2 for a particular battery
  int getFlags2(int battery) { return myFlags2[battery]; }
  /// Gets the flags3 for a particular battery
  int getFlags3(int battery) { return myFlags3[battery]; }
  /// Gets the relative state of charge for a particular battery
  int getRelSOC(int battery) { return myRelSOC[battery]; }
  /// Gets the absolute state of charge for a particular battery
  int getAbsSOC(int battery) { return myAbsSOC[battery]; }

protected:
  /// internal, packet handler
  MVREXPORT bool packetHandler(MvrRobotPacket *packet);
  /// internal, packet handler
  MVREXPORT void connectCallback(void);

  // the robot
  MvrRobot *myRobot;

  int myNumBatteries;
  int myNumBytesPerBattery;
  std::map<int, int> myFlags1;
  std::map<int, int> myFlags2;
  std::map<int, int> myFlags3;
  std::map<int, int> myRelSOC;
  std::map<int, int> myAbsSOC;


  // whether our data has been received or not
  bool myPacketArrived;
  // last time we requested a packet (we'll only ask every 200 ms)
  MvrTime myLastPacketRequest;

  bool myRequestedBatteryPackets;

  // the callback
  MvrRetFunctor1C<bool, MvrRobotBatteryPacketReader, MvrRobotPacket *> myPacketHandlerCB;
  MvrFunctorC<MvrRobotBatteryPacketReader> myConnectCB;
};

#endif // MVRROBOTBATTERYPACKETREADER_H
