#ifndef MVRROBOTPACKETSENDER_H
#define MVRROBOTPACKETSENDER_H

#include "mvriaTypedefs.h"
#include "MvrRobotPacket.h"

class MvrDeviceConnection;

/// Given a device connection this sends commands through it to the robot

class MvrRobotPacketSender
{
public:
  /// Constructor without an already assigned device connection
  MVREXPORT MvrRobotPacketSender(unsigned char sync1 = 0xfa,
			       unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection
  MVREXPORT MvrRobotPacketSender(MvrDeviceConnection *deviceConnection,
			       unsigned char sync1 = 0xfa,
			       unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection and tracking
  MVREXPORT MvrRobotPacketSender(MvrDeviceConnection *deviceConnection,
			       unsigned char sync1,
			       unsigned char sync2,
						bool tracking,
						const char *trackingLogName);
  /// Destructor
  MVREXPORT virtual ~MvrRobotPacketSender();

  /// Sends a command to the robot with no arguments
  MVREXPORT bool com(unsigned char command);
  /// Sends a command to the robot with an int for argument
  MVREXPORT bool comInt(unsigned char command, short int argument);
  /// Sends a command to the robot with two bytes for argument
  MVREXPORT bool com2Bytes(unsigned char command, char high, char low);
  /// Sends a command to the robot with a length-prefixed string for argument
  MVREXPORT bool comStr(unsigned char command, const char *argument);
  /// Sends a command to the robot with a length-prefixed string for argument
  MVREXPORT bool comStrN(unsigned char command, const char *str, int size);
  /// Sends a command containing exactly the data in the given buffer as argument
  MVREXPORT bool comDataN(unsigned char command, const char *data, int size);
  /// Sends a MvrRobotPacket
  MVREXPORT bool sendPacket(MvrRobotPacket *packet);
  
  /// Sets the device this instance sends commands to
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *deviceConnection);
  /// Gets the device this instance sends commands to
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void);

  /// Sets the callback that gets called with the finalized version of
  /// every packet set... this is ONLY for very internal very
  /// specialized use
  MVREXPORT void setPacketSentCallback(MvrFunctor1<MvrRobotPacket *> *functor);

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

  MvrFunctor1<MvrRobotPacket *> *myPacketSentCallback;

  enum { INTARG = 0x3B, NINTARG = 0x1B, STRARG = 0x2B };
};


#endif //MVRROBOTPACKETSENDER_H
