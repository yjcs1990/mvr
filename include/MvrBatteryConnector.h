/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrBatteryConnector.h
 > Description  : Connect to robot and battery based on run-time availability and command-line arguments
 > Author       : Yu Jie
 > Create Time  : 2017年05月17日
 > Modify Time  : 2017年05月17日
***************************************************************************************************/
#ifndef MVRBATTERYCONNECTOR_H
#define MVRBATTERYCONNECTOR_H

#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "mvriaUtil.h"
#include "MvrRobotConnector.h"

class MvrBatteryMTX;
class MvrRobot;



/**
   MvrBatteryConnector makes a battery connection either through a serial port 
   connection, or through a TCP
   port (for the simulator or for robots with Ethernet-serial bridge
   devices instead of on board computers).
   Normally, it first attempts a TCP connection on 
 **/
class MvrBatteryConnector
{
public:
  /// Constructor that takes argument parser
  MVREXPORT MvrBatteryConnector(MvrArgumentParser *parser, 
                                MvrRobot *robot, MvrRobotConnector *robotConnector,
                                bool autoParseArgs = true,
                                MvrLog::LogLevel infoLogLevel = MvrLog::Verbose);
  /// Destructor
  MVREXPORT ~MvrBatteryConnector(void);
  /// Connects all the batteries the robot has that should be auto connected
  MVREXPORT bool connectBatteries(bool continueOnFailedConnect = false,
                                  bool addConnectedBatteriesToRobot = true,
                                  bool addAllBatteriesToRobot = false,
                                  bool turnOnBatteries = true,
                                  bool powerCycleBatteryOnFailedConnect = true);
  MVREXPORT bool disconnectBatteries();
  /// Sets up a battery to be connected
  MVREXPORT bool setupBattery(MvrBatteryMTX *battery, int batteryNumber = 1);
  /// Connects the battery synchronously (will take up to a minute)
  MVREXPORT bool connectBattery(MvrBatteryMTX *battery,
                                int batteryNumber = 1,
                                bool forceConnection = true);
  /// Adds a battery so parsing will get it
  MVREXPORT bool addBattery(MvrBatteryMTX *battery, int batteryNumber = 1);
  /// Function to parse the arguments given in the constructor
  MVREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  MVREXPORT bool parseArgs(MvrArgumentParser *parser);
  /// Log the options the simple connector has
  MVREXPORT void logOptions(void) const;
  /// Internal function to get the battery (only useful between parseArgs and connectBatteries)
  MVREXPORT MvrBatteryMTX *getBattery(int batteryNumber);

  /// Internal function to replace the battery (only useful between parseArgs and connectBatteries) but not the battery data
  MVREXPORT bool replaceBattery(MvrBatteryMTX *battery, int batteryNumber);
  
protected:
/// Class that holds information about the battery data
class BatteryData
{
	public:
		BatteryData (int number, MvrBatteryMTX *battery) {
			myNumber = number;
			myBattery = battery;
			myConn = NULL;
			myConnect = false; myConnectReallySet = false;
			myPort = NULL;
			myPortType = NULL;
			myType = NULL;
			myRemoteTcpPort = 0; myRemoteTcpPortReallySet = false;
			myBaud = NULL;
			myAutoConn = NULL;
		}
		virtual ~BatteryData() {}
		/// The number of this battery
		int myNumber;
		/// The actual pointer to this battery
		MvrBatteryMTX *myBattery;
		// our connection
		MvrDeviceConnection *myConn;
		// if we want to connect the battery
		bool myConnect;
		// if myConnect was really set
		bool myConnectReallySet;
		// the port we want to connect the battery on
		const char *myPort;
		// the type of port we want to connect to the battery on
		const char *myPortType;
		// battery Type
		const char *myType;
		// whether to auto conn
		const char *myAutoConn;
		// battery tcp port if we're doing a remote host
		int myRemoteTcpPort;
		// if our remote battery tcp port was really set
		bool myRemoteTcpPortReallySet;
		/// the baud we want to use
		const char *myBaud;
};
  std::map<int, BatteryData *> myBatteries;
  
  /// Parses the battery arguments
  MVREXPORT bool parseBatteryArgs(MvrArgumentParser *parser, 
			       BatteryData *batteryData);
  /// Logs the battery command line option help text. 
  MVREXPORT void logBatteryOptions(BatteryData *batterydata, bool header = true, bool metaOpts = true) const;
  // Sets the battery parameters
  bool internalConfigureBattery(BatteryData *batteryData);

  std::string myBatteryTypes;

  // our parser
  MvrArgumentParser *myParser;
  bool myOwnParser;
  // if we should auto parse args or toss errors
  bool myAutoParseArgs;
  bool myParsedArgs;

  MvrRobot *myRobot;
  MvrRobotConnector *myRobotConnector;

  // variables to hold if we're logging or not
  bool myBatteryLogPacketsReceived;
  bool myBatteryLogPacketsSent;

  MvrLog::LogLevel myInfoLogLevel;

  MvrRetFunctorC<bool, MvrBatteryConnector> myParseArgsCB;
  MvrConstFunctorC<MvrBatteryConnector> myLogOptionsCB;
};

#endif // MVRBATTERYCONNECTOR_H