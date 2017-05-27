/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrBasePacket.h
 > Description  : Base packet class
 > Author       : Yu Jie
 > Create Time  : 2017年04月17日
 > Modify Time  : 2017年05月17日
***************************************************************************************************/

#ifndef MVRBASEPACKET_H
#define MVRBASEPACKET_H

#include <string>
#include "mvriaTypedefs.h"

/// Base packet class
/** This class is a base class for specific packet types implemented by base
    classes.  In most cases, you would not instantiate this class directly, but instead 
    use a subclass. However, MvrBasePacket contains many of the functions used to
    access the packet's data.

    A packet is a sequence of values stored in a buffer.  The contents 
    of a packet's data buffer is read from a device or other program or written to the
    device (for example, a serial port or TCP port
    using an MvrDeviceConnection or using MvrNetworking), optionally preceded
    by a header with some identifying data and a length, and optionally followed by a 
    footer with a checksum of the data. (If the
    header length of a particular packet type is 0, no header is written or expected on read, and likewise
    with footer.)

    Values are added to the buffer or removed from the buffer in sequence. 
    The "bufTo" methods are used to remove values from the buffer, and the
    "ToBuf" methods are used to add values to the buffer. There are different
    methods for different sized values.

    MvrBasePacket keeps a current position index in the buffer, which is the position
    at which new values are added or values are removed. 

    A buffer may be statically allocated externally and supplied to the
    constructor
    (also give a buffer size to determine the maximum amount of data that can be
    placed in that buffer),
    or automatically and dynamically allocated by MvrBasePacket as needed
    (the default behavior).

    When it is time to write out a packet, call finalizePacket() to set 
    up the footer if neccessary.
    To reuse a packet, use empty() to reset the buffer; new data will
    then be added to the beginning of the buffer again.
*/
class MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrBasePacket(MvrTypes::UByte2 bufferSize = 0, 
                          MvrTypes::UByte2 headerLength = 0, 
                          char * buf = NULL, 
                          MvrTypes::UByte2 footerLength = 0);
  /// Copy constructor
  MVREXPORT MvrBasePacket(const MvrBasePacket &other);
  /// Assignment operator
  MVREXPORT MvrBasePacket &operator=(const MvrBasePacket &other);

  /// Destructor
  MVREXPORT virtual ~MvrBasePacket();
  /// resets the length for more data to be added
  MVREXPORT virtual void empty(void);
  /// MakeFinals the packet in preparation for sending, must be done
  MVREXPORT virtual void finalizePacket(void) {}
  /// MvrLogs the hex and decimal values of each byte of the packet, and possibly extra metadata as well
  MVREXPORT virtual void log(void);
  /// MvrLogs the hex value of each byte in the packet 
  MVREXPORT virtual void printHex(void);
  /// Returns whether the packet is valid, i.e. no error has occurred when reading/writing.
  MVREXPORT virtual bool isValid(void);
  /// Resets the valid state of the packet.
  MVREXPORT virtual void resetValid();

  // Utility functions to write different data types to a buffer. They will
  // increment the length.

  /// Puts MvrTypes::Byte into packets buffer
  MVREXPORT virtual void byteToBuf(MvrTypes::Byte val);
  /// Puts MvrTypes::Byte2 into packets buffer
  MVREXPORT virtual void byte2ToBuf(MvrTypes::Byte2 val);
  /// Puts MvrTypes::Byte4 into packets buffer
  MVREXPORT virtual void byte4ToBuf(MvrTypes::Byte4 val);
  /// Puts MvrTypes::Byte8 into packets buffer
  MVREXPORT virtual void byte8ToBuf(MvrTypes::Byte8 val);

  /// Puts MvrTypes::UByte into packets buffer
  MVREXPORT virtual void uByteToBuf(MvrTypes::UByte val);
  /// Puts MvrTypes::UByte2 into packet buffer
  MVREXPORT virtual void uByte2ToBuf(MvrTypes::UByte2 val);
  /// Puts MvrTypes::UByte4 into packet buffer
  MVREXPORT virtual void uByte4ToBuf(MvrTypes::UByte4 val);
  /// Puts MvrTypes::UByte8 into packet buffer
  MVREXPORT virtual void uByte8ToBuf(MvrTypes::UByte8 val);
  /// Puts a NULL-terminated string into packet buffer
  MVREXPORT virtual void strToBuf(const char *str);

  /**
   * @brief Copies the given number of bytes from str into packet buffer
   * @deprecated use strToBufPadded(), strToBuf(), or dataToBuf() instead
  **/
  MVREXPORT virtual void strNToBuf(const char *str, int length);
  /// Copies length bytes from str, if str ends before length, pads data with 0s
  MVREXPORT virtual void strToBufPadded(const char *str, int length);
  /// Copies length bytes from data into packet buffer
  MVREXPORT virtual void dataToBuf(const char *data, int length);
  /// Copies length bytes from data into packet buffer
  MVREXPORT virtual void dataToBuf(const unsigned char *data, int length);

  // Utility functions to read different data types from a buffer. Each read
  // will increment the myReadLength.
  /// Gets a MvrTypes::Byte from the buffer
  MVREXPORT virtual MvrTypes::Byte bufToByte(void);
  /// Gets a MvrTypes::Byte2 from the buffer
  MVREXPORT virtual MvrTypes::Byte2 bufToByte2(void);
  /// Gets a MvrTypes::Byte4 from the buffer
  MVREXPORT virtual MvrTypes::Byte4 bufToByte4(void);
  /// Gets a MvrTypes::Byte8 from the buffer
  MVREXPORT virtual MvrTypes::Byte8 bufToByte8(void);

  /// Gets a MvrTypes::UByte from the buffer
  MVREXPORT virtual MvrTypes::UByte bufToUByte(void);
  /// Gets a MvrTypes::UByte2 from the buffer
  MVREXPORT virtual MvrTypes::UByte2 bufToUByte2(void);
  /// Gets a MvrTypes::UByte4 from the buffer
  MVREXPORT virtual MvrTypes::UByte4 bufToUByte4(void);
  /// Gets a MvrTypes::UByte8 from the buffer
  MVREXPORT virtual MvrTypes::UByte8 bufToUByte8(void);

  /// Gets a null-terminated string from the buffer
  MVREXPORT virtual void bufToStr(char *buf, int maxlen);
  /// Gets a null-terminated string from the buffer
  MVREXPORT std::string bufToString();
  /// Gets length bytes from buffer and puts them into data
  MVREXPORT virtual void bufToData(char * data, int length);
  /// Gets length bytes from buffer and puts them into data
  MVREXPORT virtual void bufToData(unsigned char * data, int length);

  /// Restart the reading process
  MVREXPORT virtual void resetRead(void);
  /// Gets the total length of the packet
  virtual MvrTypes::UByte2 getLength(void) const { return myLength; }
  /// Gets the length of the data in the packet
  MVREXPORT virtual MvrTypes::UByte2 getDataLength(void) const;

  /// Gets how far into the packet that has been read
  virtual MvrTypes::UByte2 getReadLength(void) const { return myReadLength; }
  /// Gets how far into the data of the packet that has been read
  virtual MvrTypes::UByte2 getDataReadLength(void) const { return myReadLength - myHeaderLength; }
  /// Gets the length of the header
  virtual MvrTypes::UByte2 getHeaderLength(void) const { return myHeaderLength; }
  /// Gets the length of the header
  virtual MvrTypes::UByte2 getFooterLength(void) const { return myFooterLength; }
  /// Gets the maximum length packet
  virtual MvrTypes::UByte2 getMaxLength(void) const { return myMaxLength; }

  /// Gets a const pointer to the buffer the packet uses 
  MVREXPORT virtual const char * getBuf(void) const;
  /// Gets a pointer to the buffer the packet uses 
  MVREXPORT virtual char * getBuf(void);

  /// Sets the buffer the packet is using
  MVREXPORT virtual void setBuf(char *buf, MvrTypes::UByte2 bufferSize);
  /// Sets the maximum buffer size (if new size is <= current does nothing)
  MVREXPORT virtual void setMaxLength(MvrTypes::UByte2 bufferSize);
  /// Sets the length of the packet
  MVREXPORT virtual bool setLength(MvrTypes::UByte2 length);
  /// Sets the read length
  MVREXPORT virtual void setReadLength(MvrTypes::UByte2 readLength);
  /// Sets the length of the header
  MVREXPORT virtual bool setHeaderLength(MvrTypes::UByte2 length);
  /// Makes this packet a duplicate of another packet
  MVREXPORT virtual void duplicatePacket(MvrBasePacket *packet);
protected:
  // internal function to make sure we have enough length left to read in the packet
  MVREXPORT bool isNextGood(int bytes);

  /// Returns true if there is enough room in the packet to add the specified number of bytes
  MVREXPORT bool hasWriteCapacity(int bytes);

  // internal data
  MvrTypes::UByte2 myHeaderLength;
  MvrTypes::UByte2 myFooterLength;
  MvrTypes::UByte2 myMaxLength;

  MvrTypes::UByte2 myReadLength;
  bool myOwnMyBuf;

  // Actual packet data
  char *myBuf;
  MvrTypes::UByte2 myLength;

  // Whether no error has occurred in reading/writing the packet.
  bool myIsValid;
};

#endif  // MVRBASEPACKET_H