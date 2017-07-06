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
#ifndef ARLMS1XX_H
#define ARLMS1XX_H

#include "mvriaTypedefs.h"
#include "MvrRobotPacket.h"
#include "MvrLaser.h"   
#include "MvrFunctor.h"

/** @internal 
  Constructs packets for LMS1xx ASCII protocol. 
  The various ...ToBuf() methods select argument types and how
they are written as ascii strings, the protocol is space delimited not fixed
width values (as in other Packet implementations), so they don't imply number of bytes used in packet output.
*/
class MvrLMS1XXPacket : public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrLMS1XXPacket();
  /// Destructor
  MVREXPORT virtual ~MvrLMS1XXPacket();

  /// Gets the command type 
  MVREXPORT const char *getCommandType(void);
  /// Gets the command name
  MVREXPORT const char *getCommandName(void);
  
  // only call finalizePacket before a send
  MVREXPORT virtual void finalizePacket(void);
  MVREXPORT virtual void resetRead(void);
  
  /// Gets the time the packet was received at
  MVREXPORT MvrTime getTimeReceived(void);
  /// Sets the time the packet was received at
  MVREXPORT void setTimeReceived(MvrTime timeReceived);

  MVREXPORT virtual void duplicatePacket(MvrLMS1XXPacket *packet);
  MVREXPORT virtual void empty(void);
  
  MVREXPORT virtual void byteToBuf(MvrTypes::Byte val);
  MVREXPORT virtual void byte2ToBuf(MvrTypes::Byte2 val);
  MVREXPORT virtual void byte4ToBuf(MvrTypes::Byte4 val);
  MVREXPORT virtual void uByteToBuf(MvrTypes::UByte val);
  MVREXPORT virtual void uByte2ToBuf(MvrTypes::UByte2 val);
  MVREXPORT virtual void uByte4ToBuf(MvrTypes::UByte4 val);
  MVREXPORT virtual void strToBuf(const char *str);

  MVREXPORT virtual MvrTypes::Byte bufToByte(void);
  MVREXPORT virtual MvrTypes::Byte2 bufToByte2(void);
  MVREXPORT virtual MvrTypes::Byte4 bufToByte4(void);
  MVREXPORT virtual MvrTypes::UByte bufToUByte(void);
  MVREXPORT virtual MvrTypes::UByte2 bufToUByte2(void);
  MVREXPORT virtual MvrTypes::UByte4 bufToUByte4(void);
  MVREXPORT virtual void bufToStr(char *buf, int len);

  // adds a raw char to the buf
  MVREXPORT virtual void rawCharToBuf(unsigned char c);
protected:
  int deascii(char c);

  MvrTime myTimeReceived;
  bool myFirstAdd;

  char myCommandType[1024]; 
  char myCommandName[1024]; 
};


/// Given a device connection it receives packets from the sick through it
/// @internal
class MvrLMS1XXPacketReceiver
{
public:
  /// Constructor with assignment of a device connection
  MVREXPORT MvrLMS1XXPacketReceiver();
  /// Destructor
  MVREXPORT virtual ~MvrLMS1XXPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  MVREXPORT MvrLMS1XXPacket *receivePacket(unsigned int msWait = 0,
					 bool shortcut = false, 
					 bool ignoreRemainders = false);

  MVREXPORT MvrLMS1XXPacket *receiveTiMPacket(unsigned int msWait = 0,
					 bool shortcut = false, 
					 bool ignoreRemainders = false);

  /// Sets the device this instance receives packets from
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void);

  // PS - added to pass info to this class
  MVREXPORT void	setmyInfoLogLevel(MvrLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
  MVREXPORT void setLaserModel(int laserModel)
  { myLaserModel = laserModel; }
  MVREXPORT void setmyName(const char *name )
  { 
    strncpy(myName, name, sizeof(myName)); 
    myName[sizeof(myName)-1] = '\0';
  }
  MVREXPORT void setReadTimeout(int timeout )
  { myReadTimeout = timeout; }


protected:
  MvrDeviceConnection *myConn;
  MvrLMS1XXPacket myPacket;
  
  enum State 
  {
    STARTING, ///< Looking for beginning
    DATA, ///< Read the data in a big whack
    REMAINDER ///< Have extra data from reading in data
  };
  State myState;
  char myName[1024];
  unsigned int myNameLength;
  char myReadBuf[100000];
  int myReadCount;
	int myReadTimeout;

	int myLaserModel;

  MvrLog::LogLevel myInfoLogLevel;
};

/**
  @since Mvr 2.7.2
  @see MvrLaserConnector
  Use MvrLaserConnector to connect to a laser, determining type based on robot and program configuration  parameters.

  This is the MvrLaser implementation for SICK LMS1xx, LMS5xx, TiM310/510
  (aka TiM3xx), TiM551, TiM561, and TiM571  lasers. To use these lasers with MvrLaserConnector, specify 
  the appropriate type in program configuration (lms1xx, lms5xx, tim3xx or
  tim510, tim551, tim561, tim571).
*/
class MvrLMS1XX : public MvrLaser
{
public:

	enum LaserModel
	{
		LMS1XX, 
		LMS5XX,
		TiM3XX,
    TiM551,
    TiM561,
    TiM571
	};

  /// Constructor
	  MVREXPORT MvrLMS1XX(int laserNumber,
			    const char *name,
					LaserModel laserModel);

  /// Destructor
  MVREXPORT ~MvrLMS1XX();
  MVREXPORT virtual bool blockingConnect(void);

	// specific init routine per laser
  MVREXPORT virtual bool lms5xxConnect(void);
  MVREXPORT virtual bool lms1xxConnect(void);
  MVREXPORT virtual bool timConnect(void);

  MVREXPORT virtual bool asyncConnect(void);
  MVREXPORT virtual bool disconnect(void);
  MVREXPORT virtual bool isConnected(void) { return myIsConnected; }
  MVREXPORT virtual bool isTryingToConnect(void) 
    { 
      if (myStartConnect)
	return true;
      else if (myTryingToConnect)
	return true; 
      else
	return false;
    }  

  /// Logs the information about the sensor
  MVREXPORT void log(void);


protected:
  MVREXPORT virtual void laserSetName(const char *name);
  MVREXPORT virtual void * runThread(void *arg);
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  MVREXPORT MvrLMS1XXPacket *sendAndRecv(
	  MvrTime timeout, MvrLMS1XXPacket *sendPacket, const char *recvName);
  void sensorInterp(void);
  void failedToConnect(void);
  void clear(void);

  /// @return true if message contents matches checksum, false otherwise.
  bool validateCheckSum(MvrLMS1XXPacket *packet);

  LaserModel myLaserModel;

  enum LaserModelFamily
  {
    LMS,
    TiM,
  };

  LaserModelFamily myLaserModelFamily;

  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;
  int myScanFreq;

  int myVersionNumber;
  int myDeviceNumber;
  int mySerialNumber;
  int myDeviceStatus1;
  int myDeviceStatus2;
  int myMessageCounter;
  int myScanCounter;
  int myPowerUpDuration;
  int myTransmissionDuration;
  int myInputStatus1;
  int myInputStatus2;
  int myOutputStatus1;
  int myOutputStatus2;
  int myReserved;
  int myScanningFreq;
  int myMeasurementFreq;
  int myNumberEncoders;
  int myNumChans16Bit;
  int myNumChans8Bit;
  int myFirstReadings;
  int myYear;
  int myMonth;
  int myMonthDay;
  int myHour;
  int myMinute;
  int mySecond;
  int myUSec;

  MvrLog::LogLevel myLogLevel;

  MvrLMS1XXPacketReceiver myReceiver;

  MvrMutex myPacketsMutex;
  MvrMutex myDataMutex;

  std::list<MvrLMS1XXPacket *> myPackets;

  MvrFunctorC<MvrLMS1XX> mySensorInterpTask;
  MvrRetFunctorC<bool, MvrLMS1XX> myMvrExitCB;
};

#endif 
