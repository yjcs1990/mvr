#ifndef MVRLMS2XXPACKET_H
#define MVRLMS2XXPACKET_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "mvriaUtil.h"

/// Represents the packets sent to the LMS2xx as well as those received from it
/**
   This class reimplements some of the buf operations since the robot is 
   little endian. 
   
   You can just look at the documentation for the MvrBasePacket except
   for these functions here, setAddress, getAddress, verifyCheckSum,
   print, getID, and calcCheckSum.  
*/

class MvrLMS2xxPacket: public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrLMS2xxPacket(unsigned char sendingAddress = 0);
  /// Destructor
  MVREXPORT virtual ~MvrLMS2xxPacket();

  /// Sets the address to send this packet to (only use for sending)
  MVREXPORT void setSendingAddress(unsigned char address);

  /// Sets the address to send this packet to (only use for sending)
  MVREXPORT unsigned char getSendingAddress(void);

  /// Gets the address this packet was sent from (only use for receiving)
  MVREXPORT unsigned char getReceivedAddress(void);
  
  /// returns true if the crc matches what it should be
  MVREXPORT bool verifyCRC(void);
  
  /// returns the ID of the packet (first byte of data)
  MVREXPORT MvrTypes::UByte getID(void);

  /// returns the crc, probably used only internally
  MVREXPORT MvrTypes::Byte2 calcCRC(void);
  
  // only call finalizePacket before a send
  MVREXPORT virtual void finalizePacket(void);
  MVREXPORT virtual void resetRead(void);
  
  /// Gets the time the packet was received at
  MVREXPORT MvrTime getTimeReceived(void);
  /// Sets the time the packet was received at
  MVREXPORT void setTimeReceived(MvrTime timeReceived);

  /// Duplicates the packet
  MVREXPORT virtual void duplicatePacket(MvrLMS2xxPacket *packet);
protected:
  MvrTime myTimeReceived;
  unsigned char mySendingAddress;
};

typedef MvrLMS2xxPacket MvrSickPacket;

#endif // MVRSICKPACKET_H
