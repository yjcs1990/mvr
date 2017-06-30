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
#ifndef ARSZSERIES_H
#define ARSZSERIES_H

#include "ariaTypedefs.h"
#include "ariaOSDef.h"
#include "MvrRobotPacket.h"
#include "MvrLaser.h"   
#include "MvrFunctor.h"

/** @internal */
class MvrSZSeriesPacket : public MvrBasePacket
{
public:
  /// Constructor
  AREXPORT MvrSZSeriesPacket();
  /// Destructor
  AREXPORT virtual ~ArSZSeriesPacket();
  
  /// Gets the time the packet was received at
  AREXPORT MvrTime getTimeReceived(void);
  /// Sets the time the packet was received at
  AREXPORT void setTimeReceived(MvrTime timeReceived);

  AREXPORT virtual void duplicatePacket(MvrSZSeriesPacket *packet);
  AREXPORT virtual void empty(void);

  AREXPORT virtual void byteToBuf(MvrTypes::Byte val);

  AREXPORT virtual MvrTypes::Byte bufToByte(void);
  
  void setDataLength(int x)
  { myDataLength = x; }
  int getDataLength()
  { return myDataLength; }
  void setNumReadings(int x)
  { myNumReadings = x; }
  int getNumReadings()
  { return myNumReadings; }
  void setScanFrequency(int x)
  { myScanFrequency = x; }
  int getScanFrequency()
  { return myScanFrequency; }

  void setPrevScanFrequency(int x)
  { myPrevScanFrequency = x; }
  int getPrevScanFrequency()
  { return myPrevScanFrequency; }


  unsigned char getCrcByte1()
  { return myCrcByte1; }
  void setCrcByte1(unsigned char c)
  { myCrcByte1 = c; }
  unsigned char getCrcByte2()
  { return myCrcByte2; }
  void setCrcByte2(unsigned char c)
  { myCrcByte2 = c; }

protected:
  int deascii(char c);

  MvrTime myTimeReceived;

  // SZS specific
  int myDataLength;
  int myNumReadings;
  int myScanFrequency;
  unsigned char myCrcByte1;
  unsigned char myCrcByte2;
  
  int myPrevScanFrequency;


};


/// Given a device connection it receives packets from the sick through it
/// @internal
class MvrSZSeriesPacketReceiver
{
public:
  /// Constructor with assignment of a device connection
  AREXPORT MvrSZSeriesPacketReceiver();
  /// Destructor
  AREXPORT virtual ~ArSZSeriesPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  AREXPORT MvrSZSeriesPacket *receivePacket(unsigned int msWait = 0,
					 bool shortcut = false);

  /// Sets the device this instance receives packets from
  AREXPORT void setDeviceConnection(MvrDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  AREXPORT MvrDeviceConnection *getDeviceConnection(void);
  unsigned short CRC16(unsigned char *, int);

  // PS - added to pass info to this class
  AREXPORT void	setmyInfoLogLevel(MvrLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
  AREXPORT void setmyIsSZ00(bool isSZ00)
  { myIsSZ00 = isSZ00; }
  AREXPORT void setmyName(const char *name )
  { strcpy(myName, name); }

protected:
  MvrDeviceConnection *myConn;
  MvrSZSeriesPacket myPacket;
  
  char myName[1024];
  unsigned int myNameLength;
  unsigned char myReadBuf[100000];
  int myReadCount;
  bool myIsSZ00;
  MvrLog::LogLevel myInfoLogLevel;

  unsigned short myPrevCrc;


};

/**
  @since Mvria 2.7.4
  @see MvrLaserConnector
  Use MvrLaserConnector to connect to a laser, determining type based on robot and program configuration  parameters.
*/
class MvrSZSeries : public MvrLaser
{
public:
  /// Constructor
  AREXPORT MvrSZSeries(int laserNumber,
		 const char *name = "SZSeries");
  /// Destructor
  AREXPORT ~ArSZSeries();
  AREXPORT virtual bool blockingConnect(void);
  AREXPORT virtual bool asyncConnect(void);
  AREXPORT virtual bool disconnect(void);
  virtual bool isConnected(void) { return myIsConnected; }
  virtual bool isTryingToConnect(void) 
    { 
      if (myStartConnect)
	return true;
      else if (myTryingToConnect)
	return true; 
      else
	return false;
    }  

  /// Logs the information about the sensor
  AREXPORT void log(void);
protected:
  AREXPORT virtual void laserSetName(const char *name);
  AREXPORT virtual void * runThread(void *arg);
  AREXPORT virtual void setRobot(MvrRobot *robot);
  void sensorInterp(void);
  void failedToConnect(void);
  void clear(void);
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;

  int myNumChans;


  MvrLog::LogLevel myLogLevel;

  MvrSZSeriesPacketReceiver myReceiver;

  MvrMutex myPacketsMutex;
  MvrMutex myDataMutex;

  std::list<ArSZSeriesPacket *> myPackets;
  
  MvrTime myPrevSensorIntTime;

  MvrFunctorC<ArSZSeries> mySensorInterpTask;
  MvrRetFunctorC<bool, MvrSZSeries> myAriaExitCB;
};

#endif 
