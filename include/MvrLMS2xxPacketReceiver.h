#ifndef MVRLMS2XXPACKETRECEIVER_H
#define MVRLMS2XXPACKETRECEIVER_H

#include "mvriaTypedefs.h"
#include "MvrDeviceConnection.h"
#include "MvrLMS2xxPacket.h"

/// Given a device connection it receives packets from the sick through it
class MvrLMS2xxPacketReceiver
{
public:
  /// Constructor without an already assigned device connection
  MVREXPORT MvrLMS2xxPacketReceiver(unsigned char receivingAddress = 0, 
				bool allocatePackets = false,
				bool useBase0Address = false);
  /// Constructor with assignment of a device connection
  MVREXPORT MvrLMS2xxPacketReceiver(MvrDeviceConnection *deviceConnection, 
				unsigned char receivingAddress = 0,
				bool allocatePackets = false,
				bool useBase0Address = false);
  /// Destructor
  MVREXPORT virtual ~MvrLMS2xxPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  MVREXPORT MvrLMS2xxPacket *receivePacket(unsigned int msWait = 0);

  /// Sets the device this instance receives packets from
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *deviceConnection);
  /// Gets the device this instance receives packets from
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void);
  
  /// Gets whether or not the receiver is allocating packets
  MVREXPORT bool isAllocatingPackets(void) { return myAllocatePackets; }

protected:
  MvrDeviceConnection *myDeviceConn;
  bool myAllocatePackets;
  MvrLMS2xxPacket myPacket;
  unsigned char myReceivingAddress;
  bool myUseBase0Address;
  enum { STATE_START, STATE_ADDR, STATE_START_COUNT, STATE_ACQUIRE_DATA };
};

typedef MvrLMS2xxPacketReceiver MvrSickPacketReceiver;

#endif // MVRSICKPACKETRECEIVER_H
