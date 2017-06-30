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
#ifndef ARROBOTBATTERYPACKETREADER_H
#define ARROBOTBATTERYPACKETREADER_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "MvrFunctor.h"

class MvrRobot;
class MvrRobotPacket;

/// This class will read a config packet from the robot
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
  MvrFunctorC<ArRobotBatteryPacketReader> myConnectCB;
};

#endif // ARROBOTBATTERYPACKETREADER_H
