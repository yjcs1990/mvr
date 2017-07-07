#ifndef MVRROBOTPACKETRECEIVER_H
#define MVRROBOTPACKETRECEIVER_H

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
