#ifndef MVRS3SERIES_H
#define MVRS3SERIES_H

#include "mvriaTypedefs.h"
#include "mvriaOSDef.h"
#include "MvrRobotPacket.h"
#include "MvrLaser.h"   
#include "MvrFunctor.h"

/** @internal */
class MvrS3SeriesPacket : public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrS3SeriesPacket();
  /// Destructor
  MVREXPORT virtual ~MvrS3SeriesPacket();
  
  /// Gets the time the packet was received at
  MVREXPORT MvrTime getTimeReceived(void);
  /// Sets the time the packet was received at
  MVREXPORT void setTimeReceived(MvrTime timeReceived);

  MVREXPORT virtual void duplicatePacket(MvrS3SeriesPacket *packet);
  MVREXPORT virtual void empty(void);
  

  void setDataLength(int x)
  { myDataLength = x; }
  int getDataLength()
  { return myDataLength; }
  void setNumReadings(int x)
  { myNumReadings = x; }
  int getNumReadings()
  { return myNumReadings; }
  void setStatusByte(unsigned char c)
  { myStatusByte = c; }
  unsigned char getStatusByte()
  { return myStatusByte; }
  void setTimeStampByte1(unsigned char c)
  { myTimeStampByte1 = c; }
  unsigned char getTimeStampByte1()
  { return myTimeStampByte1; }
  void setTimeStampByte2(unsigned char c)
  { myTimeStampByte2 = c; }
  unsigned char getTimeStampByte2()
  { return myTimeStampByte2; }
  void setTimeStampByte3(unsigned char c)
  { myTimeStampByte3 = c; }
  unsigned char getTimeStampByte3()
  { return myTimeStampByte3; }
  void setTimeStampByte4(unsigned char c)
  { myTimeStampByte4 = c; }
  unsigned char getTimeStampByte4()
  { return myTimeStampByte4; }
  unsigned char getTelegramNumByte1()
  { return myTelegramNumByte1; }
  unsigned char getTelegramNumByte2()
  { return myTelegramNumByte2; }
  void setTelegramNumByte2(unsigned char c)
  { myTelegramNumByte2 = c; }
  void setTelegramNumByte1(unsigned char c)
  { myTelegramNumByte1 = c; }
  unsigned char getCrcByte1()
  { return myCrcByte1; }
  void setCrcByte1(unsigned char c)
  { myCrcByte1 = c; }
  unsigned char getCrcByte2()
  { return myCrcByte2; }
  void setCrcByte2(unsigned char c)
  { myCrcByte2 = c; }
  unsigned char getMonitoringDataByte1()
  { return myMonitoringDataByte1; }
  void setMonitoringDataByte1(unsigned char c)
  { myMonitoringDataByte1 = c; }
  unsigned char getMonitoringDataByte2()
  { return myMonitoringDataByte2; }
  void setMonitoringDataByte2(unsigned char c)
  { myMonitoringDataByte2 = c; }
  bool getMonitoringDataAvailable()
  { return myMonitoringDataAvailable; }
  void setMonitoringDataAvailable(bool c)
  { myMonitoringDataAvailable = c; }
  unsigned char getProtocolVersionByte1()
  { return myProtocolVersionByte1; }
  void setProtocolVersionByte1(unsigned char c)
  { myProtocolVersionByte1 = c; }
  unsigned char getProtocolVersionByte2()
  { return myProtocolVersionByte2; }
  void setProtocolVersionByte2(unsigned char c)
  { myProtocolVersionByte2 = c; }

protected:

  MvrTime myTimeReceived;

  // S3S specific
  int myDataLength;
  int myNumReadings;
  unsigned char myStatusByte;
  unsigned char myTimeStampByte1;
  unsigned char myTimeStampByte2;
  unsigned char myTimeStampByte3;
  unsigned char myTimeStampByte4;
  unsigned char myTelegramNumByte1;
  unsigned char myTelegramNumByte2;
  unsigned char myCrcByte1;
  unsigned char myCrcByte2;
	bool myMonitoringDataAvailable;
	unsigned char myMonitoringDataByte1;
	unsigned char myMonitoringDataByte2;
	unsigned char myProtocolVersionByte1;
	unsigned char myProtocolVersionByte2;

};


/// Given a device connection it receives packets from the sick through it
/// @internal
class MvrS3SeriesPacketReceiver
{
public:
  /// Constructor with assignment of a device connection
  MVREXPORT MvrS3SeriesPacketReceiver();
  /// Destructor
  MVREXPORT virtual ~MvrS3SeriesPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  MVREXPORT MvrS3SeriesPacket *receivePacket(unsigned int msWait = 0,
					 bool shortcut = false);

  /// Sets the device this instance receives packets from
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void);
  unsigned short CRC16(unsigned char *, int);

  // PS - added to pass info to this class
  MVREXPORT void	setInfoLogLevel(MvrLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
  MVREXPORT void setIsS300(bool isS300)
  { myIsS300 = isS300; }
  MVREXPORT void setName(const char *name )
  { strcpy(myName, name); }

protected:
  MvrDeviceConnection *myConn;
  MvrS3SeriesPacket myPacket;
  
  char myName[1024];
  unsigned int myNameLength;
  unsigned char myReadBuf[100000];
  int myReadCount;
  bool myIsS300;
  MvrLog::LogLevel myInfoLogLevel;



};

/**
  @see MvrLaserConnector
  Use MvrLaserConnector to connect to a laser, determining type based on robot and program configuration  parameters.
*/
class MvrS3Series : public MvrLaser
{
public:
  /// Constructor
  MVREXPORT MvrS3Series(int laserNumber,
		 const char *name = "S3Series");
  /// Destructor
  MVREXPORT ~MvrS3Series();
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

  /// Disables the monitoring data
  void sendFakeMonitoringData(bool sendFakeMonitoringData) 
    { mySendFakeMonitoringData = sendFakeMonitoringData; } 
  /// sees if the monitoring data is available
  bool isMonitoringDataAvailable(void)
		{ return myIsMonitoringDataAvailable; }
  // gets the monitoring data
  int getMonitoringData(void)
		{ return myMonitoringData; }
  // compares the monitoring data against a specific value
  bool compareMonitoringDataAgainst(int val)
    { return myMonitoringData == val; }

  /// Logs the information about the sensor
  MVREXPORT void log(void);
protected:
  MVREXPORT virtual void laserSetName(const char *name);
  MVREXPORT virtual void * runThread(void *arg);
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  void sensorInterp(void);
  void failedToConnect(void);
  void clear(void);
  MVREXPORT bool packetHandler(MvrRobotPacket *packet);
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;

  int myNumChans;

	bool myIsMonitoringDataAvailable;
	int myMonitoringData;
  bool mySendFakeMonitoringData;

  MvrLog::LogLevel myLogLevel;

  MvrS3SeriesPacketReceiver myReceiver;

  MvrMutex myPacketsMutex;
  MvrMutex myDataMutex;

  MvrMutex mySafetyDebuggingTimeMutex;
  MvrTime mySafetyDebuggingTime;

  std::list<MvrS3SeriesPacket *> myPackets;

  MvrFunctorC<MvrS3Series> mySensorInterpTask;
  MvrRetFunctorC<bool, MvrS3Series> myMvriaExitCB;
  MvrRetFunctor1C<bool, MvrS3Series, MvrRobotPacket *> myPacketHandlerCB;
};

#endif 
