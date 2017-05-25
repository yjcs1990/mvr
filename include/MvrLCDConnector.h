/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLCDConnector.h
 > Description  : Connect to robot and lcd based on run-time availablitily and command-line arguments
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRLCDCONNECTOR_H
#define MVRLCDCONNECTOR_H

#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "mvriaUtil.h"
#include "MvrRobotConnector.h"

class MvrLCDMTX;
class MvrRobot;

/**

   MvrLCDConnector makes a lcd connection either through a serial port 
   connection, or through a TCP
   port (for the simulator or for robots with Ethernet-serial bridge
   devices instead of onboard computers).
   Normally, it first attempts a TCP connection on 
**/
class MvrLCDConnector
{
public:
  /// Constructor that takes argument parser
  MVREXPORT MvrLCDConnector(MvrArgumentParser *parser, 
                            MvrRobot *robot, MvrRobotConnector *robotConnector,
                            bool autoParseArgs = true,
                            MvrLog::LogLevel infoLogLevel = MvrLog::Verbose,
                            MvrRetFunctor1<bool, const char *> *turnOnPowerOutputCB = NULL,
                            MvrRetFunctor1<bool, const char *> *turnOffPowerOutputCB = NULL);
  /// Destructor
  MVREXPORT ~MvrLCDConnector(void);
  /// Connects all the lcds the robot has that should be auto connected
  MVREXPORT bool connectLCDs(bool continueOnFailedConnect = false,
                             bool addConnectedLCDsToRobot = true,
                             bool addAllLCDsToRobot = false,
                             bool turnOnLCDs = true,
                             bool powerCycleLCDOnFailedConnect = true);
  /// Sets up a lcd to be connected
  MVREXPORT bool setupLCD(MvrLCDMTX *lcd, int lcdNumber = 1);
  /// Connects the lcd synchronously (will take up to a minute)
  MVREXPORT bool connectLCD(MvrLCDMTX *lcd, int lcdNumber = 1, bool forceConnection = true);
  /// Adds a lcd so parsing will get it
  MVREXPORT bool addLCD(MvrLCDMTX *lcd, int lcdNumber = 1);
  /// Function to parse the arguments given in the constructor
  MVREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  MVREXPORT bool parseArgs(MvrArgumentParser *parser);
  /// Log the options the simple connector has
  MVREXPORT void logOptions(void) const;
  /// Internal function to get the lcd (only useful between parseArgs and connectLCDs)
  MVREXPORT MvrLCDMTX *getLCD(int lcdNumber);

  /// Internal function to replace the lcd (only useful between parseArgs and connectLCDs) but not the lcd data
  MVREXPORT bool replaceLCD(MvrLCDMTX *lcd, int lcdNumber);

	MVREXPORT void turnOnPowerCB (int);
	MVREXPORT void turnOffPowerCB (int);

	MVREXPORT void setIdentifier(const char *identifier);
  
protected:
/// Class that holds information about the lcd data
class LCDData
{
	public:
		LCDData (int number, MvrLCDMTX *lcd) {
			myNumber = number;
			myLCD = lcd;
			myConn = NULL;
			myConnect = false; myConnectReallySet = false;
			myPort = NULL;
			myPortType = NULL;
			myType = NULL;
			myRemoteTcpPort = 0; myRemoteTcpPortReallySet = false;
			myBaud = NULL;
			myAutoConn = NULL;
			myConnFailOption = NULL;
		}
		virtual ~LCDData() {}
		/// The number of this lcd
		int myNumber;
		/// The actual pointer to this lcd
		MvrLCDMTX *myLCD;
		// our connection
		MvrDeviceConnection *myConn;
		// if we want to connect the lcd
		bool myConnect;
		// if myConnect was really set
		bool myConnectReallySet;
		// the port we want to connect the lcd on
		const char *myPort;
		// the type of port we want to connect to the lcd on
		const char *myPortType;
		// lcd Type
		const char *myType;
		// wheather to auto conn
		const char *myAutoConn;
		// wheather to disconnect on conn faiure 
		const char *myConnFailOption;
		// lcd tcp port if we're doing a remote host
		int myRemoteTcpPort;
		// if our remote lcd tcp port was really set
		bool myRemoteTcpPortReallySet;
		/// the baud we want to use
		const char *myBaud;
};

  std::map<int, LCDData *> myLCDs;

	/// Turns on the power for the specific board in the firmware
	MVREXPORT bool turnOnPower(LCDData *LCDData);

	/// Turns off the power for the specific board in the firmware
	MVREXPORT bool turnOffPower(LCDData *LCDData);

	/// Verifies the firmware version on the LCD and loads new firmware 
	/// if there is no match
	MVREXPORT bool verifyFirmware(LCDData *LCDData);

	MVREXPORT std::string searchForFile(const char *dirToLookIn, const char *prefix, const char *suffix);

  
  /// Parses the lcd arguments
  MVREXPORT bool parseLCDArgs(MvrArgumentParser *parser, LCDData *lcdData);
  /// Logs the lcd command line option help text. 
  MVREXPORT void logLCDOptions(LCDData *lcddata, bool header = true, bool metaOpts = true) const;
  // Sets the lcd parameters
  bool internalConfigureLCD(LCDData *lcdData);

  std::string myLCDTypes;

  // our parser
  MvrArgumentParser *myParser;
  bool myOwnParser;
  // if we should autoparse args or toss errors 
  bool myAutoParseArgs;
  bool myParsedArgs;

  MvrRobot *myRobot;
  MvrRobotConnector *myRobotConnector;

  // variables to hold if we're logging or not
  bool myLCDLogPacketsReceived;
  bool myLCDLogPacketsSent;

  MvrLog::LogLevel myInfoLogLevel;

  MvrRetFunctor1<bool, const char *> *myTurnOnPowerOutputCB;
  MvrRetFunctor1<bool, const char *> *myTurnOffPowerOutputCB;

  MvrRetFunctorC<bool, MvrLCDConnector> myParseArgsCB;
  MvrConstFunctorC<MvrLCDConnector> myLogOptionsCB;

	MvrFunctor1C<MvrLCDConnector, int> myTurnOnPowerCB;
	MvrFunctor1C<MvrLCDConnector, int> myTurnOffPowerCB;

};

#endif // MVRLCDCONNECTOR_H
