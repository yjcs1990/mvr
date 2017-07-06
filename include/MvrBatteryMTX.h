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
#ifndef ARBATTERYMTX_H
#define ARBATTERYMTX_H

#include "mvriaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctor.h"
#include "MvrRobot.h"
#include "MvrRobotPacket.h"
#include "MvrRobotConnector.h"



// Packets are in the format of 
// 2 bytes header (0xfa 0xba)
// 1 byte length
// 1 byte command
// xx bytes command specific / args
// 2 bytes checksum
//


/** 
    @since 2.8.0
*/
class MvrBatteryMTX : public MvrASyncTask
{
public:
  /// Constructor
  MVREXPORT MvrBatteryMTX( 
			 int batteryBoardNum = 0,
				const char * name = "MTXBattery", 
				MvrDeviceConnection *conn = NULL,
				MvrRobot *robot = NULL);
  /// Destructor
  MVREXPORT virtual ~MvrBatteryMTX();
  // Grabs the new readings from the robot and adds them to the buffers
  // (Primarily for internal use.)
  //MVREXPORT void processReadings(void);

	int getBoardNum(void)
		{ return myBoardNum; }

  /// Sets the robot pointer, also attaches its process function to the
  /// robot as a Sensor Interpretation task.
  MVREXPORT virtual void setRobot(MvrRobot *robot);

  /// Very Internal call that gets the packet sender, shouldn't be used
  MvrRobotPacketSender *getPacketSender(void)
    { return mySender; }
  /// Very Internal call that gets the packet sender, shouldn't be used
  MvrRobotPacketReceiver *getPacketReceiver(void)
    { return myReceiver; }

  /// Sets the device this instance receives packets from
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  MVREXPORT MvrDeviceConnection *getDeviceConnection(void);

	MVREXPORT int getAsyncConnectState(void);

	MvrRobotPacket getCellPacket()
	{ return myCellPacket; }

  MVREXPORT virtual bool blockingConnect(bool sendTracking, bool recvTracking);
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

  /// Lock this device
  virtual int lockDevice() { return(myDeviceMutex.lock());}
  /// Try to lock this device
  virtual int tryLockDevice() {return(myDeviceMutex.tryLock());}
  /// Unlock this device
  virtual int unlockDevice() {return(myDeviceMutex.unlock());}

  MVREXPORT void logBatteryInfo(MvrLog::LogLevel level = MvrLog::Normal);
  MVREXPORT void logCellInfo(MvrLog::LogLevel level = MvrLog::Normal);
  void log(MvrLog::LogLevel level = MvrLog::Normal)
  {
    logBatteryInfo(level);
    logCellInfo(level);
  }

  MVREXPORT bool sendPowerOff();
  MVREXPORT bool sendPowerOffCancel();
  MVREXPORT bool sendStopCharging();
  MVREXPORT bool sendStartCharging();
  MVREXPORT bool sendSetPowerOffDelay(unsigned int msDelay);
  MVREXPORT bool sendSetRealTimeClock(unsigned int secSinceEpoch);
  MVREXPORT bool sendResetCellData();
  MVREXPORT bool sendSetReserveValue(unsigned short hundredthOfPercent);
  MVREXPORT bool sendSetBalanceValue(unsigned short hundredthOfPercent);
  MVREXPORT bool sendEmergencyPowerOff();
  MVREXPORT bool sendSystemInfo(unsigned char dataValue);
  MVREXPORT bool sendCellInfo(unsigned char dataValue);
  MVREXPORT bool sendBasicInfo(unsigned char dataValue);

  MVREXPORT void updateSystemInfo(unsigned char *buf);
  MVREXPORT void updateCellInfo(unsigned char *buf);
  MVREXPORT void updateBasicInfo(unsigned char *buf);

	// need to figure out how to pass back the system and cell info 
	//MVREXPORT bool fetchSystemInfo();
	//MVREXPORT bool fetchCellInfo();

	// basic info
  /// Charge estimate (in percentage, 0-100)
	double getChargeEstimate(void) const
		{ return myChargeEstimate; }
  /// Current draw (amps, negative is charging)
	double getCurrentDraw(void) const
		{ return myCurrentDraw; }
  /// volts
	double getPackVoltage(void) const
		{ return myPackVoltage; }
	int getStatusFlags(void) const
		{ return myStatusFlags; }
	int getErrorFlags(void) const
		{ return myErrorFlags; }

  bool onCharger(void) const 
    { return (myStatusFlags & STATUS_ON_CHARGER); }
  MvrRobot::ChargeState getChargeState(void) const
    { return myChargeState; }
  int getChargeStateAsInt(void) const
    { return myChargeState; }

	// system info 
	int getId(void) const
		{ return myId; }
	int getFirmwareVersion(void) const
		{ return myFirmwareVersion; }
	int getSerialNumber(void) const
		{ return mySerialNumber; }
	//int getCurrentTime(void) const
	//	{ return myCurrentTime; }
	long long getCurrentTime(void) const
		{ return myCurrentTime; }
	long long getLastChargeTime(void) const
		{ return myLastChargeTime; }
	int getChargeRemainingEstimate(void) const
		{ return myChargeRemainingEstimate; }
	int getCapacityEstimate(void) const
		{ return myCapacityEstimate; }
	double getDelay(void) const
		{ return myDelay; }
	int getCycleCount(void) const
		{ return myCycleCount; }
	double getTemperature(void) const
		{ return myTemperature; }
	double getPaddleVolts(void) const
		{ return myPaddleVolts; }
	double getVoltage(void) const
		{ return myVoltage; }
	double getFuseVoltage(void) const
		{ return myFuseVoltage; }
	double getChargeCurrent(void) const
		{ return myChargeCurrent; }
	double getDisChargeCurrent(void) const
		{ return myDisChargeCurrent; }
	double getCellImbalance(void) const
		{ return myCellImbalance; }
	double getImbalanceQuality(void) const
		{ return myImbalanceQuality; }
	double getReserveChargeValue(void) const
		{ return myReserveChargeValue; }

	// cell info
	int getNumCells(void) const
		{ return myNumCells; }


	int getCellFlag(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellFlags);
		} }

	int getCellCapacity(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellCapacity);
		} }

	int getCellCharge(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellCharge);
		} }

	double getCellVoltage(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellVoltage);
		} }

  /// Request a continous stream of packets
  MVREXPORT void requestContinuousSysInfoPackets(void);
  /// Stop the stream of packets
  MVREXPORT void stopSysInfoPackets(void);
  /// See if we've requested packets
  MVREXPORT bool haveRequestedSysInfoPackets(void);

  /// Request a continous stream of packets
  MVREXPORT void requestContinuousCellInfoPackets(void);
  /// Stop the stream of packets
  MVREXPORT void stopCellInfoPackets(void);
  /// See if we've requested packets
  MVREXPORT bool haveRequestedCellInfoPackets(void);

  MVREXPORT virtual const char *getName(void) const;

  void	setInfoLogLevel(MvrLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
	
  /// Gets the default port type for the battery
  const char *getDefaultPortType(void) { return myDefaultPortType.c_str(); }

  /// Gets the default port type for the battery
  const char *getDefaultTcpPort(void) { return myDefaultTcpPort.c_str(); }

  /// Sets the numter of seconds without a response until connection assumed lost
  virtual void setConnectionTimeoutSeconds(double seconds)
	{ MvrLog::log(MvrLog::Normal, 
		     "%s::setConnectionTimeoutSeconds: Setting timeout to %g secs", 
		     getName(), seconds);
	  myTimeoutSeconds = seconds; }
  /// Gets the number of seconds without a response until connection assumed lost
  virtual double getConnectionTimeoutSeconds(void)
	{return myTimeoutSeconds; }
	/// check for lost connections
	MVREXPORT bool checkLostConnection(void);
	/// disconnect 
	MVREXPORT void disconnectOnError(void);
  /// Gets the time data was last receieved
  MvrTime getLastReadingTime(void) { return myLastReading; }
  /// Gets the number of battery readings received in the last second
  MVREXPORT int getReadingCount(void);
  // Function called in sensorInterp to indicate that a
  // reading was received
  MVREXPORT virtual void internalGotReading(void);

  /// Adds a callback for when disconnection happens because of an error
  void addDisconnectOnErrorCB(MvrFunctor *functor, 
			     int position = 51) 
    { myDisconnectOnErrorCBList.addCallback(functor, position); }

  /// Removes a callback for when disconnection happens because of an error
  void remDisconnectOnErrorCB(MvrFunctor *functor)
    { myDisconnectOnErrorCBList.remCallback(functor); }


  /// Adds a callback for when the battery is powering off
  void addBatteryPoweringOffCB(MvrFunctor *functor, 
			     int position = 51) 
    { myBatteryPoweringOffCBList.addCallback(functor, position); }

  /// Removes a callback for when the battery is powering off
  void remBatteryPoweringOffCB(MvrFunctor *functor)
    { myBatteryPoweringOffCBList.remCallback(functor); }

  /// Adds a callback for when the battery is powering off
  void addBatteryPoweringOffCancelledCB(MvrFunctor *functor, 
			     int position = 51) 
    { myBatteryPoweringOffCancelledCBList.addCallback(functor, position); }

  /// Removes a callback for when the battery is powering off
  void remBatteryPoweringOffCancelledCB(MvrFunctor *functor)
    { myBatteryPoweringOffCancelledCBList.remCallback(functor); }

  // myStatusFlags 
  enum StatusFlags {
    STATUS_ON_CHARGER=0x0001,
    STATUS_CHARGING=0x0002,
    STATUS_BALANCING_ENGAGED=0x0004,
    STATUS_CHARGER_ON=0x0008,
    STATUS_BATTERY_POWERING_OFF=0x0010,
    /// MPL adding the rest of these since I need one of 'em
    STATUS_MASTER_SWITCH_ON=0x0020,
    STATUS_CHARGE_SWITCH_ON=0x0040,
    STATUS_COMMANDED_SHUTDOWN=0x0080,
    STATUS_OFF_BUTTON_PRESSED=0x0100,
    STATUS_ON_BUTTON_PRESSED=0x0200,
    STATUS_USER_BUTTON_PRESSED=0x0400
  };
  
  // myErrorFlags (if this is updated also change the code in interpBasicInfo
  enum ErrorFlags {
    ERROR_BATTERY_OVERVOLTAGE=0x0001,
    ERROR_BATTERY_UNDERVOLTAGE=0x0002,
    ERROR_OVERCURRENT=0x0004,
    ERROR_BLOWNFUSE=0x0008,
    ERROR_RTC_ERROR=0x0010,
    ERROR_OVER_TEMPERATURE=0x0020,
    ERROR_MASTER_SWITCH_FAULT=0x0040,
    ERROR_SRAM_ERROR=0x0080,
    ERROR_CHARGER_OUT_OF_VOLTAGE_RANGE=0x0100,
    ERROR_CHARGER_CIRCUIT_FAULT=0x0200
  };

enum Headers {
	HEADER1=0xfa,
	HEADER2=0xba
	};


protected:
  MvrDeviceConnection *myConn;
	int myAsyncConnectState;
  std::string myName;
	std::string myDefaultPortType;
	std::string myDefaultTcpPort;

  double myTimeoutSeconds;
  bool myRobotRunningAndConnected;

  MvrTime myLastReading;

  // packet count
  time_t myTimeLastReading;
  int myReadingCurrentCount;
  int myReadingCount;

  MvrCallbackList myDisconnectOnErrorCBList;
  MvrCallbackList myBatteryPoweringOffCBList;
  MvrCallbackList myBatteryPoweringOffCancelledCBList;

	MvrRobot *myRobot;
  MvrFunctorC<MvrBatteryMTX> myProcessCB;

  MVREXPORT virtual void batterySetName(const char *name);
  MVREXPORT virtual void * runThread(void *arg);
		

	MVREXPORT bool getSystemInfo();
	MVREXPORT bool getCellInfo();
	MVREXPORT bool getBasicInfo();

  void interpBasicInfo(void);
  void interpErrors(void);
  void checkAndSetCurrentErrors(ErrorFlags errorFlag, const char *errorString);

	// PS - need this because of debug log - battery won't send continuous cell
  MvrRobotPacket myCellPacket;

  void sensorInterp(void);
  void failedToConnect(void);
  void clear(void);
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;

  MvrRobot::ChargeState myChargeState;

	int myBoardNum;
	unsigned char myVersion;

  MvrLog::LogLevel myLogLevel;

  //MvrBatteryMTXPacketReceiver myReceiver;


	MvrRobotPacketReceiver *myReceiver;
  MvrRobotPacketSender *mySender;

  MvrMutex myPacketsMutex;
  MvrMutex myDataMutex;
	MvrMutex myDeviceMutex;
	
  MvrLog::LogLevel myInfoLogLevel;
	
  //std::list<MvrBatteryMTXPacket *> myPackets;
  std::list<MvrRobotPacket *> myPackets;
  
  MvrTime myPrevBatteryIntTime;

  bool myRequestedSysInfoBatteryPackets;
  bool myRequestedCellInfoBatteryPackets;

	bool mySendTracking;
	bool myRecvTracking;

// Protocol Commands


enum Commands {
	BASIC_INFO=0x00,
	SYSTEM_INFO=0x01,
	CELL_INFO=0x02,
	POWER_OFF_REQUEST=0x10,
	POWER_OFF_CANCEL=0x11,
	STOP_CHARGING=0x12,
	START_CHARGING=0x13,
	SET_POWER_OFF_DELAY=0x20,
	SET_REAL_TIME_CLOCK=0x21,
	RESET_CELL_DATA=0x22,
	SET_RESERVE_VALUE=0x23,
	SET_BALANCE_VALUE=0x24,
	EMERGENCY_OFF=0xff
	};

// SYSTEM_INFO and CELL_INFO Data
enum Data {
	STOP_SENDING=0x00,
	SEND_ONCE=0x01,
	SEND_CONTINUOUS=0x02
	};

// Length fields -
enum Sizes {
	BASIC_INFO_SIZE=16,
	SYSTEM_INFO_SIZE=60,
	CELL_INFO_SIZE=95 // this is for 8 cells
	};

	// System Info
	unsigned char myId;
	unsigned char myFirmwareVersion;
	unsigned int mySerialNumber;
	long long myCurrentTime;
	//unsigned int myCurrentTime;
	//unsigned int myLastChargeTime;
	long long myLastChargeTime;
	unsigned int myChargeRemainingEstimate;
	unsigned int myCapacityEstimate;
	unsigned int myRawDelay;
	double myDelay;
	unsigned int myCycleCount;
	unsigned short myRawTemperature;
	double myTemperature;
	unsigned short myRawPaddleVolts;
	double myPaddleVolts;
	unsigned short myRawVoltage;
	double myVoltage;
	unsigned short myRawFuseVoltage;
	double myFuseVoltage;
	unsigned short myRawChargeCurrent;
	double myChargeCurrent;
	unsigned short myRawDisChargeCurrent;
	double myDisChargeCurrent;
	unsigned short myRawCellImbalance;
	double myCellImbalance;
	unsigned short myRawImbalanceQuality;
	double myImbalanceQuality;
	unsigned short myRawReserveChargeValue;
	double myReserveChargeValue;
	
	// end system info
	
	// Cell Info

	// myCellFlags defines

	enum CellFlags {
		BALANCER_IS_ON=0x01,
		OVER_VOLTAGE=0x02,
		UNDER_VOLTAGE=0x04
	};


	struct CellInfo {
		unsigned char myCellFlags;
		unsigned short myRawCellVoltage;
		double myCellVoltage;
		unsigned short myCellCharge;
		unsigned short myCellCapacity;
	};
	
	unsigned char myNumCells;	
	std::map <int, CellInfo *> myCellNumToInfoMap;
	
	// end cell info

  // Basic Info
  
	unsigned short myRawChargeEstimate;	
  double myChargeEstimate;
	short myRawCurrentDraw;
  double myCurrentDraw;
	unsigned short myRawPackVoltage;
  double myPackVoltage;
	unsigned short myStatusFlags;
	unsigned short myErrorFlags;

  bool myHaveSetRTC;

  int myLastStatusFlags;

  bool myFirstErrorFlagsCheck;
  unsigned short myLastErrorFlags;
  std::string myErrorString;
  int myErrorCount;
  std::string myLastErrorString;
  int myLastErrorCount;

  

	// end basic info
	
  MvrFunctorC<MvrBatteryMTX> mySensorInterpTask;
  MvrRetFunctorC<bool, MvrBatteryMTX> myMvrExitCB;

};



#endif // ARBATTERYMTX_H
