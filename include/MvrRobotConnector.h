/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotConnector.h
 > Description  : Connect to robot or simulator based on program command line parameters.
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/

#ifndef MVRROBOTCONNECTOR_H
#define MVRROBOTCONNECTOR_H

#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "mvriaUtil.h"

class MvrRobot;
class MvrSonarConnector;
class MvrBatteryConnector;
//class MvrLCDConnector;

/** Connect to robot or simulator based on program command line parameters.

   MvrRobotConnector makes a robot connection either through a TCP
   port (for the simulator or for robots with Ethernet-serial bridge
   devices instead of onboard computers), or through a direct serial 
   port connection.  Normally, it first attempts a TCP connection on 
   @param localhost port 8101, to use a simulator if running. If the simulator
   is not running, then it normally then connects using the serial port
   (the first serial port, COM1, by default).  Various other connection
   parameters are configurable through command-line arguments.
  
   When you create your MvrRobotConnector, pass it command line parameters via
   either the argc and argv variables from main(), or pass it an
   MvrArgumentBuilder or MvrArgumentParser object. (MvrArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   MvrRobotConnector registers a callback with the global Mvria class. Use
   Mvria::parseArgs() to parse all command line parameters to the program, and
   Mvria::logOptions() to print out information about all registered command-line parameters.

   The following command-line arguments are checked:
   @verbinclude MvrRobotConnector_options

   You can then prepare an MvrRobot object for connection (with various connection
   options configured via the command line parameters) and initiate the connection
   attempt by that object by calling connectRobot().
    
   After it's connected, you must then begin the robot processing cycle by calling
   MvrRobot::runAsync() or MvrRobot::run().

   @ingroup ImportantClasses
   @ingroup DeviceClasses

**/
class MvrRobotConnector
{
public:
  /// Constructor that takes argument parser
  MVREXPORT MvrRobotConnector(MvrArgumentParser *parser, MvrRobot *robot, 
			      bool autoParseArgs = true, bool connectAllComponents = true);
  /// Destructor
  MVREXPORT ~MvrRobotConnector(void);
  /// Sets up the given robot to be connected
  MVREXPORT bool setupRobot(void);
  /// Sets up an arbitrary robot to be connected
  MVREXPORT bool setupRobot(MvrRobot *robot);
  /// Sets up the robot then connects it
  MVREXPORT bool connectRobot(void);
  /// Sets up the robot then connects it
  MVREXPORT bool connectRobot(MvrRobot *robot);
  /// Function to parse the arguments given in the constructor
  MVREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  MVREXPORT bool parseArgs(MvrArgumentParser *parser);
  /// Log the options the simple connector has
  MVREXPORT void logOptions(void) const;
  /// Gets the remote host, if one was used, or NULL if it wasn't
  MVREXPORT const char *getRemoteHost(void) const;
  /// Gets if the remote connection is a sim
  MVREXPORT bool getRemoteIsSim(void) const;
  /// Call for forcing the remote to be a sim (mostly for internal use)
  MVREXPORT void setRemoteIsSim(bool remoteIsSim);
  /// Gets the robot this connector is using (mostly for backwards compatibility stuff)
  MVREXPORT MvrRobot *getRobot(void);
  /// Disconnect from any robot components connected to. If connectAllComponents
  /// was not set to false in constructor, then connections to all robot components, and the
  /// robot itself, are disconnected.
  MVREXPORT bool disconnectAll();
protected:
  // the robot we've set up (so we can find its params)
  MvrRobot *myRobot; 
  // if we're using the sim or not
  bool myUsingSim;
  // if we're connecting via tcp (not to the sim), what remote host
  const char *myRemoteHost;
  // robot port, if there isn't one this'll be NULL, which will just
  // be the default of MvrUtil::COM1
  const char *myRobotPort;
  // baud for the serial
  int myRobotBaud;
  // robot tcp port if we're doing a remote host (defaults to 8101)
  int myRemoteRobotTcpPort;
  
  // whether we're connecting to a remote sim or not (so we don't try
  // to open a port for the laser)
  bool myRemoteIsSim;
  // force a sim to not look like a sim (for laser debugging mostly)
  bool myRemoteIsNotSim;
  // variables to hold if we're logging or not
  bool myRobotLogPacketsReceived;
  bool myRobotLogPacketsSent;
  bool myRobotLogMovementReceived;
  bool myRobotLogMovementSent;
  bool myRobotLogVelocitiesReceived;
  bool myRobotLogActions;

  // if we're auto parsing the arguments when setupRobot or
  // connectRobot are called
  bool myAutoParseArgs;
  // If we've parsed the args already or not
  bool myHaveParsedArgs;

  // If true, create additional connectors such as sonar, battery, lcd.
  bool myConnectAllComponents;
  

  // our parser
  MvrArgumentParser *myParser;
  bool myOwnParser;

  // a few device connections to use to connect to the robot
  MvrTcpConnection myRobotTcpConn;
  MvrSerialConnection myRobotSerConn;

  MvrRetFunctorC<bool, MvrRobotConnector> myParseArgsCB;
  MvrConstFunctorC<MvrRobotConnector> myLogOptionsCB;

//  MvrLCDConnector *myLCDConnector;
  MvrBatteryConnector *myBatteryConnector;
  MvrSonarConnector *mySonarConnector;
};
#endif  // MVRROBOTCONNECTOR_H