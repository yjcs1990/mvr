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
#ifndef ARSERIALCONNECTION_H
#define ARSERIALCONNECTION_H

#include <string>
#include "mvriaTypedefs.h"
#include "MvrDeviceConnection.h"

/// Utility for connecting to and communicating with devices through a serial port
/** @ingroup UtilityClasses
  @ingroup DeviceClasses
*/
class MvrSerialConnection: public MvrDeviceConnection
{
 public:
  /// Constructor
  MVREXPORT MvrSerialConnection(bool is422 = false);
  /// Destructor also closes the connection
  MVREXPORT virtual ~MvrSerialConnection();

  /** Opens the serial port
   *  @return 0 on success. Return one of the error codes in the 
   *          MvrSerialConnection::Open enumeration on error.
   *  @sa MvrUtil::COM1, MvrUtil::COM2, MvrUtil::COM3, MvrUtil::COM4
   */
  MVREXPORT int open(const char * port = NULL);

  /** Sets the port this connection will use
   *  @sa MvrUtil::COM1, MvrUtil::COM2, MvrUtil::COM3, MvrUtil::COM4
   */
  MVREXPORT void setPort(const char *port = NULL);

  /** Gets the port this is using
   *  @sa MvrUtil::COM1, MvrUtil::COM2, MvrUtil::COM3, MvrUtil::COM4
   */
  MVREXPORT const char * getPort(void);
  
  MVREXPORT virtual bool openSimple(void);  
  MVREXPORT virtual int getStatus(void);
  MVREXPORT virtual bool close(void);
  MVREXPORT virtual int read(const char *data, unsigned int size, 
			    unsigned int msWait = 0);
  MVREXPORT virtual int write(const char *data, unsigned int size);
  MVREXPORT virtual const char * getOpenMessage(int messageNumber);

   /**
      Sets the BAUD rate to use when connection is opened, or change the baud rate if already opened
      @param baud the baud rate to set the connection to
      @return true if rate was sucessfully set, false otherwise (unsupported rate, system error changing rate on port, etc.)
      @sa getBaud()
   */
  MVREXPORT bool setBaud(int baud);
  /// Gets what the current baud rate is set to
  MVREXPORT int getBaud(void);

  /// Sets whether to enable or disable the hardware control lines
  MVREXPORT bool setHardwareControl(bool hardwareControl);
  /// Gets whether the hardware control lines are enabled or disabled
  MVREXPORT bool getHardwareControl(void);

  /// Sees how the CTS line is set (true = high)
  MVREXPORT bool getCTS(void);

  /// Sees how the DSR line is set (true = high)
  MVREXPORT bool getDSR(void);

  /// Sees how the DCD line is set (true = high)
  MVREXPORT bool getDCD(void);

  /// Sees how the Ring line is set (true = high)
  MVREXPORT bool getRing(void);

  // Internal open, for use by open and openSimple
  MVREXPORT int internalOpen(void);

  enum Open { 
      OPEN_COULD_NOT_OPEN_PORT = 1, ///< Could not open the port
      OPEN_COULD_NOT_SET_UP_PORT, ///< Could not set up the port
      OPEN_INVALID_BAUD_RATE, ///< Baud rate is not valid
      OPEN_COULD_NOT_SET_BAUD, ///< Baud rate valid, but could not set it
      OPEN_ALREADY_OPEN ///< Connection was already open
  };
  MVREXPORT virtual MvrTime getTimeRead(int index);
  MVREXPORT virtual bool isTimeStamping(void);

 protected:
  void buildStrMap(void);

#ifndef WIN32
  // these both return -1 for errors
  int rateToBaud(int rate);
  int baudToRate(int baud);
  // this just tries
  void startTimeStamping(void);
  bool myTakingTimeStamps;
#endif
    
  MvrStrMap myStrMap;
  std::string myPortName;
  int myBaudRate;
  int myStatus;
  bool myHardwareControl;

#ifndef WIN32
  int myPort;
#endif // ifdef linux

  bool myIs422;

#ifdef WIN32
  HANDLE myPort;
#endif // ifdef WIN32


};

#endif
