/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLMS2xxPacket.h
 > Description  : Represents the packets sent to the LMS2xx as well as those received from it
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef ARLMS2XXPACKET_H
#define ARLMS2XXPACKET_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "mvriaUtil.h"

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

#endif // ARSICKPACKET_H
