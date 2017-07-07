#ifndef MVRSZSERIES_H
#define MVRSZSERIES_H

#include "mvriaTypedefs.h"
#include "mvriaOSDef.h"
#include "MvrRobotPacket.h"
#include "MvrLaser.h"   
#include "MvrFunctor.h"

/** @internal */
class MvrSZSeriesPacket : public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrSZSeriesPacket();
  /// Destructor
  MVREXPORT virtual ~MvrSZSeriesPacket();
  
  /// Gets the time the packet was received at
  MVREXPORT MvrTime getTimeReceived(void);
  /// Sets the time the packet was received at
  MVREXPORT void setTimeReceived(MvrTime timeReceived);

  MVREXPORT virtual void duplicatePacket(MvrSZSeriesPacket *packet);
  MVREXPORT virtual void empty(void);

  MVREXPORT virtual void byteToBuf(MvrTypes::Byte val);

  MVREXPORT virtual MvrTypes::Byte bufToByte(void);
  
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
  MVREXPORT MvrSZSeriesPacketReceiver();
  /// Destructor
  MVREXPORT virtual ~MvrSZSeriesPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  MVREXPORT MvrSZSeriesPacket *receivePacket(unsigned int msWait = 0,
					 bool shortcut = false);

  /// Sets the device this instance receives packets from
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void);
  unsigned short CRC16(unsigned char *, int);

  // PS - added to pass info to this class
  MVREXPORT void	setmyInfoLogLevel(MvrLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
  MVREXPORT void setmyIsSZ00(bool isSZ00)
  { myIsSZ00 = isSZ00; }
  MVREXPORT void setmyName(const char *name )
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
  @since Mvr 2.7.4
  @see MvrLaserConnector
  Use MvrLaserConnector to connect to a laser, determining type based on robot and program configuration  parameters.
*/
class MvrSZSeries : public MvrLaser
{
public:
  /// Constructor
  MVREXPORT MvrSZSeries(int laserNumber,
		 const char *name = "SZSeries");
  /// Destructor
  MVREXPORT ~MvrSZSeries();
  MVREXPORT virtual bool blockingConnect(void);
  MVREXPORT virtual bool asyncConnect(void);
  MVREXPORT virtual bool disconnect(void);
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
  MVREXPORT void log(void);
protected:
  MVREXPORT virtual void laserSetName(const char *name);
  MVREXPORT virtual void * runThread(void *arg);
  MVREXPORT virtual void setRobot(MvrRobot *robot);
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

  std::list<MvrSZSeriesPacket *> myPackets;
  
  MvrTime myPrevSensorIntTime;

  MvrFunctorC<MvrSZSeries> mySensorInterpTask;
  MvrRetFunctorC<bool, MvrSZSeries> myMvrExitCB;
};

#endif 
