#ifndef MVRDEVICECONNECTION_H
#define MVRDEVICECONNECTION_H

#include <string>
#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrBasePacket.h"

/// Base class for device connections
/**
   Base class for device connections, this is mostly for connections to the 
   robot or simulator but could also be used for a connection to a laser
   or other device

   Note that this is mostly a base class, so if you'll want to use one of the
   classes which inherit from this one... also note that in those classes 
   is where you'll find setPort which sets the place the device connection
   will try to connect to... the inherited classes also have an open which
   returns more detailed information about the open attempt, and which takes 
   the parameters for where to connect
*/
class MvrDeviceConnection
{
 public:
  /// constructor
  MVREXPORT MvrDeviceConnection();
  /// destructor also forces a close on the connection
  MVREXPORT virtual ~MvrDeviceConnection();
  /// Reads data from connection
  /**
     Reads data from connection
     @param data pointer to a character array to read the data into
     @param size maximum number of bytes to read
     @param msWait read blocks for this many milliseconds (not at all for == 0)
     @return number of bytes read, or -1 for failure
     @see write, writePacket
  */
  MVREXPORT virtual int read(const char *data, unsigned int size, 
			    unsigned int msWait = 0) = 0;
  /// Writes data to connection
  /**
     Writes data to connection from a packet
     @param packet pointer to a packet to write the data from
     @return number of bytes written, or -1 for failure
     @see read, write
  */
  MVREXPORT virtual int writePacket(MvrBasePacket *packet)
    { if (packet == NULL || packet->getLength() == 0) return 0;
    return write(packet->getBuf(), packet->getLength()); }
  /// Writes data to connection
  /**
     Writes data to connection
     @param data pointer to a character array to write the data from
     @param size number of bytes to write
     @return number of bytes read, or -1 for failure
     @see read, writePacket
  */
  MVREXPORT virtual int write(const char *data, unsigned int size) = 0;
  /// Gets the status of the connection, which is one of the enum status
  /**
     Gets the status of the connection, which is one of the enum status.
     If you want to get a string to go along with the number, use 
     getStatusMessage
     @return the status of the connection
     @see getStatusMessage
  */
  MVREXPORT virtual int getStatus(void) = 0;
  /// Gets the description string associated with the status
  /** 
      @param messageNumber the int from getStatus you want the string for
      @return the description associated with the status
      @see getStatus
  */
  MVREXPORT const char *getStatusMessage(int messageNumber) const;

  /// Opens the connection again, using the values from setLocation or 
  // a previous open
  virtual bool openSimple(void) = 0;
    
  /// Closes the connection
  /**
     @return whether the close succeeded or not
  */
  virtual bool close(void) { return false; }
  /// Gets the string of the message associated with opening the device
  /**
     Each class inherited from this one has an open method which returns 0
     for success or an integer which can be passed into this function to 
     obtain a string describing the reason for failure
     @param messageNumber the number returned from the open
     @return the error description associated with the messageNumber
  */
  MVREXPORT virtual const char * getOpenMessage(int messageNumber) = 0;
  enum Status { 
      STATUS_NEVER_OPENED = 1, ///< Never opened
      STATUS_OPEN,  ///< Currently open
      STATUS_OPEN_FAILED, ///< Tried to open, but failed
      STATUS_CLOSED_NORMALLY, ///< Closed by a close call
      STATUS_CLOSED_ERROR ///< Closed because of error
  };
  /// Gets the time data was read in
  /** 
      @param index looks like this is the index back in the number of bytes
      last read in
      @return the time the last read data was read in 
  */
  MVREXPORT virtual MvrTime getTimeRead(int index) = 0;
  /// sees if timestamping is really going on or not
  /** @return true if real timestamping is happening, false otherwise */
  MVREXPORT virtual bool isTimeStamping(void) = 0;

  /// Gets the port name
  MVREXPORT const char *getPortName(void) const;
  /// Gets the port type
  MVREXPORT const char *getPortType(void) const;
  
  /// Sets the device type (what this is connecting to)
  MVREXPORT void setDeviceName(const char *deviceName);
  /// Gets the device type (what this is connecting to)
  MVREXPORT const char *getDeviceName(void) const;

  /// Notifies the device connection that the start of a packet is
  /// trying to be read
  MVREXPORT void debugStartPacket(void);
  /// Notifies the device connection that some bytes were read (should
  /// call with 0 if it read but got no bytes)
  MVREXPORT void debugBytesRead(int bytesRead);
  /// Notifies the device connection that the end of a packet was
  /// read, which will cause log messages if set to do so
  MVREXPORT void debugEndPacket(bool goodPacket, int type = 0);
  /// Makes all device connections so that they'll dump data
  MVREXPORT static bool debugShouldLog(bool shouldLog);
 protected:
  /// Sets the port name
  MVREXPORT void setPortName(const char *portName);
  /// Sets the port type
  MVREXPORT void setPortType(const char *portType);

  void buildStrMap(void);
  static bool ourStrMapInited;
  static MvrStrMap ourStrMap;

  std::string myDCPortName;
  std::string myDCPortType;
  std::string myDCDeviceName;

  static bool ourDCDebugShouldLog;
  static MvrTime ourDCDebugFirstTime;
  bool myDCDebugPacketStarted;
  MvrTime myDCDebugStartTime;
  MvrTime myDCDebugFirstByteTime;
  MvrTime myDCDebugLastByteTime;
  int myDCDebugBytesRead;
  int myDCDebugTimesRead;
  long long myDCDebugNumGoodPackets;
  long long myDCDebugNumBadPackets;
};

#endif


