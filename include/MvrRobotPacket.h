#ifndef MVRROBOTPACKET_H
#define MVRROBOTPACKET_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "mvriaUtil.h"

/// Represents the packets sent to the robot as well as those received from it
/**
   This class reimplements some of the buf operations since the robot is 
   opposeite endian from intel.  Also has the getID for convenience.  
   
   You can just look at the documentation for the MvrBasePacket except for
   the 4 new functions here, verifyCheckSum, getID, print, and calcCheckSum.
 */
class MvrRobotPacket: public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrRobotPacket(unsigned char sync1 = 0xfa, 
			 unsigned char sync2 = 0xfb);
  /// Destructor
  MVREXPORT virtual ~MvrRobotPacket();

  /// Assignment operator
  MVREXPORT MvrRobotPacket &operator=(const MvrRobotPacket &other);

  /// returns true if the checksum matches what it should be
  MVREXPORT bool verifyCheckSum(void);

  /// returns the ID of the packet 
  MVREXPORT MvrTypes::UByte getID(void);

  /// Sets the ID of the packet 
  MVREXPORT void setID(MvrTypes::UByte id);

  /// returns the checksum, probably used only internally
  MVREXPORT MvrTypes::Byte2 calcCheckSum(void);
  
  // only call finalizePacket before a send
  MVREXPORT virtual void finalizePacket(void);
  
  /// Gets the time the packet was received at
  MVREXPORT MvrTime getTimeReceived(void);
  /// Sets the time the packet was received at
  MVREXPORT void setTimeReceived(MvrTime timeReceived);

  MVREXPORT virtual void log();

protected:
  unsigned char mySync1;
  unsigned char mySync2;
  MvrTime myTimeReceived;
};

#endif // ARROBOTPACKET_H
