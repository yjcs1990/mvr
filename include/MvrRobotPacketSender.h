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
#ifndef ARROBOTPACKETSENDER_H
#define ARROBOTPACKETSENDER_H

#include "ariaTypedefs.h"
#include "MvrRobotPacket.h"

class MvrDeviceConnection;

/// Given a device connection this sends commands through it to the robot

class MvrRobotPacketSender
{
public:
  /// Constructor without an already assigned device connection
  AREXPORT MvrRobotPacketSender(unsigned char sync1 = 0xfa,
			       unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection
  AREXPORT MvrRobotPacketSender(MvrDeviceConnection *deviceConnection,
			       unsigned char sync1 = 0xfa,
			       unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection and tracking
  AREXPORT MvrRobotPacketSender(MvrDeviceConnection *deviceConnection,
			       unsigned char sync1,
			       unsigned char sync2,
						bool tracking,
						const char *trackingLogName);
  /// Destructor
  AREXPORT virtual ~ArRobotPacketSender();

  /// Sends a command to the robot with no arguments
  AREXPORT bool com(unsigned char command);
  /// Sends a command to the robot with an int for argument
  AREXPORT bool comInt(unsigned char command, short int argument);
  /// Sends a command to the robot with two bytes for argument
  AREXPORT bool com2Bytes(unsigned char command, char high, char low);
  /// Sends a command to the robot with a length-prefixed string for argument
  AREXPORT bool comStr(unsigned char command, const char *argument);
  /// Sends a command to the robot with a length-prefixed string for argument
  AREXPORT bool comStrN(unsigned char command, const char *str, int size);
  /// Sends a command containing exactly the data in the given buffer as argument
  AREXPORT bool comDataN(unsigned char command, const char *data, int size);
  /// Sends a MvrRobotPacket
  AREXPORT bool sendPacket(MvrRobotPacket *packet);
  
  /// Sets the device this instance sends commands to
  AREXPORT void setDeviceConnection(MvrDeviceConnection *deviceConnection);
  /// Gets the device this instance sends commands to
  AREXPORT MvrDeviceConnection *getDeviceConnection(void);

  /// Sets the callback that gets called with the finalized version of
  /// every packet set... this is ONLY for very internal very
  /// specialized use
  AREXPORT void setPacketSentCallback(MvrFunctor1<ArRobotPacket *> *functor);

  void setTracking(bool v = true)
  {
    myTracking = v;
  }
  void setTrackingLogName(const char *n)
  {
    myTrackingLogName = n;
  }
protected:
  bool connValid(void);
  MvrDeviceConnection * myDeviceConn;
  MvrRobotPacket myPacket;

	bool myTracking;
	std::string myTrackingLogName;

  MvrMutex mySendingMutex;

  MvrFunctor1<ArRobotPacket *> *myPacketSentCallback;

  enum { INTARG = 0x3B, NINTARG = 0x1B, STRARG = 0x2B };
};


#endif //ARROBOTPACKETSENDER_H
