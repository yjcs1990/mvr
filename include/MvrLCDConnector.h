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
#ifndef ARLCDCONNECTOR_H
#define ARLCDCONNECTOR_H

#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "mvriaUtil.h"
#include "MvrRobotConnector.h"

class MvrLCDMTX;
class MvrRobot;



/// Connect to robot and lcd based on run-time availablitily and command-line arguments
/**

   MvrLCDConnector makes a lcd connection either through a serial port 
   connection, or through a TCP
   port (for the simulator or for robots with Ethernet-serial bridge
   devices instead of onboard computers).
   Normally, it first attempts a TCP connection on 
   @a localhost port 8101, to use a simulator if running. If the simulator
   is not running, then it normally then connects using the serial port
   Various connection
   parameters are configurable through command-line arguments or in the robot
   parameter file. (Though the internal interface used by ARIA to do this is also
   available if you need to use it: See addLCD(); otherwise don't use
   addLCD(), setupLCD(), etc.).
  
   When you create your MvrLCDConnector, pass it command line parameters via
   either the argc and argv vmvriables from main(), or pass it an
   MvrArgumentBuilder or MvrArgumentParser object. (MvrArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   MvrLCDConnector registers a callback with the global Mvr class. Use
   Mvria::parseArgs() to parse all command line parameters to the program, and
   Mvria::logOptions() to print out information about all registered command-line parameters.

   The following command-line arguments are checked:
   @verbinclude MvrLCDConnector_options

   To connect to any lcds that were set up in the robot parameter file or
   via command line arguments, call connectLCDs().  If successful, 
   connectLCDs() will return true and add an entry for each lcd connected
   in the MvrRobot object's list of lcds.  These MvrLCDMTX objects can be
   accessed from your MvrRobot object via MvrRobot::findLCD() or MvrRobot::getLCDMap(). 
   

   @since 2.8.0

 **/
class MvrLCDConnector
{
public:
  /// Constructor that takes argument parser
  MVREXPORT MvrLCDConnector(
	  MvrArgumentParser *parser, 
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
  MVREXPORT bool setupLCD(MvrLCDMTX *lcd, 
			   int lcdNumber = 1);
  /// Connects the lcd synchronously (will take up to a minute)
  MVREXPORT bool connectLCD(MvrLCDMTX *lcd,
			     int lcdNumber = 1,
			     bool forceConnection = true);
  /// Adds a lcd so parsing will get it
  MVREXPORT bool addLCD(MvrLCDMTX *lcd,
			 int lcdNumber = 1);
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

	MVREXPORT std::string searchForFile(
			const char *dirToLookIn, const char *prefix, const char *suffix);

  
  /// Parses the lcd arguments
  MVREXPORT bool parseLCDArgs(MvrArgumentParser *parser, 
			       LCDData *lcdData);
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

  // vmvriables to hold if we're logging or not
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

#endif // ARLASERCONNECTOR_H
