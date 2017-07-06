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
#ifndef ARROBOTPACKETRECEIVER_H
#define ARROBOTPACKETRECEIVER_H

#include "mvriaTypedefs.h"
#include "MvrRobotPacket.h"


class MvrDeviceConnection;

/// Given a device connection it receives packets from the robot through it
class MvrRobotPacketReceiver
{
public:
  /// Constructor without an already assigned device connection
  MVREXPORT MvrRobotPacketReceiver(bool allocatePackets = false,
				 unsigned char sync1 = 0xfa, 
				 unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection
  MVREXPORT MvrRobotPacketReceiver(MvrDeviceConnection *deviceConnection, 
				 bool allocatePackets = false,
				 unsigned char sync1 = 0xfa, 
				 unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection and tracking
  MVREXPORT MvrRobotPacketReceiver(MvrDeviceConnection *deviceConnection, 
				 bool allocatePackets,
				 unsigned char sync1, 
				 unsigned char sync2,
					bool tracking,
					const char *trackingLogName);
  /// Destructor
  MVREXPORT virtual ~MvrRobotPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  MVREXPORT MvrRobotPacket *receivePacket(unsigned int msWait = 0);

  /// Sets the device this instance receives packets from
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *deviceConnection);
  /// Gets the device this instance receives packets from
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void);
  
  /// Gets whether or not the receiver is allocating packets
  MVREXPORT bool isAllocatingPackets(void) { return myAllocatePackets; }
  /// Sets whether or not the receiver is allocating packets
  MVREXPORT void setAllocatingPackets(bool allocatePackets) 
    { myAllocatePackets = allocatePackets; }

#ifdef DEBUG_SPARCS_TESTING
  MVREXPORT void setSync1(unsigned char s1) { mySync1 = s1; }
  MVREXPORT void setSync2(unsigned char s2) { mySync2 = s2; }
#endif

	void setTracking(bool tracking)
  {
    myTracking = tracking;
  }

	void setTrackingLogName(const char *trackingLogName)
  {
    myTrackingLogName = trackingLogName;
  }

  /// Sets the callback that gets called with the finalized version of
  /// every packet set... this is ONLY for very internal very
  /// specialized use
  MVREXPORT void setPacketReceivedCallback(MvrFunctor1<MvrRobotPacket *> *functor);
protected:
  MvrDeviceConnection *myDeviceConn;
	bool myTracking;
	std::string myTrackingLogName;

  bool myAllocatePackets;
  MvrRobotPacket myPacket;
  enum { STATE_SYNC1, STATE_SYNC2, STATE_ACQUIRE_DATA };
  unsigned char mySync1;
  unsigned char mySync2;

  MvrFunctor1<MvrRobotPacket *> *myPacketReceivedCallback;
};

#endif // ARROBOTPACKETRECEIVER_H
