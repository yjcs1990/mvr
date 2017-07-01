#ifndef MVRLASERCONNECTOR_H
#define MVRLASERCONNECTOR_H

#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "mvriaUtil.h"
#include "MvrRobotConnector.h"

class MvrLaser;
class MvrRobot;



/// Create laser interface objects (for any kind of laser supported by ARIA) and connect to lasers based on parameters from robot parameter file and command-line arguments
/**

   MvrLaserConnector makes a laser connection (e.g. through serial port, 
   TCP network connection, or to simulator connection as a special case if
   the robot connection is to a simulator.)
   Parameters are configurable through command-line arguments or in the robot
   parameter file. 

  
   When you create your MvrLaserConnector, pass it command line parameters via
   either the argc and argv variables from main(), or pass it an
   MvrArgumentBuilder or MvrArgumentParser object. (MvrArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   MvrLaserConnector registers a callback with the global Mvria class. Use
   Mvria::parseArgs() to parse all command line parameters to the program, and
   Mvria::logOptions() to print out information about all registered command-line parameters.
   MvrLaserConnector will be included in these.

   Then, to connect to any lasers that were set up in the robot parameter file or
   via command line arguments, call connectLasers().  If successful, 
   connectLasers() will return true and add an entry for each laser connected
   in the MvrRobot object's list of lasers.  These MvrLaser objects can be
   accessed from your MvrRobot object via MvrRobot::findLaser() or MvrRobot::getLaserMap(). 
   
   (The internal interface used by ARIA to connect to configured lasers and add
   them to MvrRobot is also
   available if you need to use it: See addLaser(); but this is normally not
neccesary for almost all cases.)

   The following command-line arguments are checked:
   @verbinclude MvrLaserConnector_options

   @since 2.7.0
   @ingroup ImportantClasses
   @ingroup DeviceClasses
 **/
class MvrLaserConnector
{
public:
  /// Constructor that takes argument parser
  MVREXPORT MvrLaserConnector(
	  MvrArgumentParser *parser, 
	  MvrRobot *robot, MvrRobotConnector *robotConnector,
	  bool autoParseArgs = true,
	  MvrLog::LogLevel infoLogLevel = MvrLog::Verbose,
	  MvrRetFunctor1<bool, const char *> *turnOnPowerOutputCB = NULL,
	  MvrRetFunctor1<bool, const char *> *turnOffPowerOutputCB = NULL);
  /// Destructor
  MVREXPORT ~MvrLaserConnector(void);
  /// Connects all the lasers the robot has that should be auto connected
  MVREXPORT bool connectLasers(bool continueOnFailedConnect = false,
			      bool addConnectedLasersToRobot = true,
			      bool addAllLasersToRobot = false,
			      bool turnOnLasers = true,
			      bool powerCycleLaserOnFailedConnect = true,
			      int *failedOnLaser = NULL);
  /// Sets up a laser to be connected
  MVREXPORT bool setupLaser(MvrLaser *laser, 
			   int laserNumber = 1);
  /// Connects the laser synchronously (will take up to a minute)
  MVREXPORT bool connectLaser(MvrLaser *laser,
			     int laserNumber = 1,
			     bool forceConnection = true);
  /// Adds a laser so parsing will get it
  MVREXPORT bool addLaser(MvrLaser *laser,
			 int laserNumber = 1);
  /// Adds a laser for parsing but where connectLaser will be called later
  MVREXPORT bool addPlaceholderLaser(MvrLaser *placeholderLaser,
				    int laserNumber = 1,
				    bool takeOwnershipOfPlaceholder = false);
  /// Function to parse the arguments given in the constructor
  MVREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  MVREXPORT bool parseArgs(MvrArgumentParser *parser);
  /// Log the command-line options available to the user
  MVREXPORT void logOptions(void) const;
  /// Internal function to get the laser (only useful between parseArgs and connectLasers)
  MVREXPORT MvrLaser *getLaser(int laserNumber);

  /// Internal function to replace the laser (only useful between parseArgs and connectLasers) but not the laser data
  MVREXPORT bool replaceLaser(MvrLaser *laser, int laserNumber);

  /// Log all currently set paramter values
  MVREXPORT void logLaserData();
  
protected:
  /// Class that holds information about the laser data
  class LaserData
  {
  public:
    LaserData(int number, MvrLaser *laser, 
	      bool laserIsPlaceholder = false, bool ownPlaceholder = false)
      { 
	myNumber = number; 
	myLaser = laser; 
	myConn = NULL;
	myLaserIsPlaceholder = laserIsPlaceholder;
	myOwnPlaceholder = ownPlaceholder;
	myConnect = false; myConnectReallySet = false;
	myPort = NULL; 
	myPortType = NULL;
	myRemoteTcpPort = 0; myRemoteTcpPortReallySet = false;
	myFlipped = false; myFlippedReallySet = false; 
	myDegreesStart = HUGE_VAL; myDegreesStartReallySet = false; 
	myDegreesEnd = -HUGE_VAL; myDegreesEndReallySet = false; 
	myDegrees = NULL; 
	myIncrementByDegrees = -HUGE_VAL; myIncrementByDegreesReallySet = false; 
	myIncrement = NULL; 
	myUnits = NULL; 
	myReflectorBits = NULL;
	myPowerControlled = true; myPowerControlledReallySet = false; 
	myStartingBaud = NULL;
	myAutoBaud = NULL;
	myMaxRange = INT_MAX; myMaxRangeReallySet = false; 
	myAdditionalIgnoreReadings = NULL;
      }
    virtual ~LaserData() {}
    /// The number of this laser
    int myNumber;
    /// The actual pointer to this laser
    MvrLaser *myLaser;
    // our connection
    MvrDeviceConnection *myConn;
    /// If the laser is a placeholder for parsing
    bool myLaserIsPlaceholder;
    /// If we own the placeholder laser
    bool myOwnPlaceholder;
    // if we want to connect the laser
    bool myConnect;
    // if myConnect was really set
    bool myConnectReallySet;
    // the port we want to connect the laser on
    const char *myPort;
    // the type of port we want to connect to the laser on
    const char *myPortType;
    // laser tcp port if we're doing a remote host
    int myRemoteTcpPort;  
    // if our remote laser tcp port was really set
    bool myRemoteTcpPortReallySet;
    // if we have the laser flipped
    bool myFlipped;
    // if our flipped was really set
    bool myFlippedReallySet;
    // what degrees to start at 
    double myDegreesStart;
    // if our start degrees was really set
    bool myDegreesStartReallySet;
    // what degrees to end at 
    double myDegreesEnd;
    // if our end degrees was really set
    bool myDegreesEndReallySet;
    // the degrees we want wto use
    const char *myDegrees;
    // what increment to use
    double myIncrementByDegrees;
    // if our end degrees was really set
    bool myIncrementByDegreesReallySet;
    // the increment we want to use
    const char *myIncrement;
    /// the units we want to use 
    const char *myUnits;
    /// the reflector bits we want to use 
    const char *myReflectorBits;
    // if we are controlling the laser power
    bool myPowerControlled;
    // if our flipped was really set
    bool myPowerControlledReallySet;
    /// the starting baud we want to use
    const char *myStartingBaud;
    /// the auto baud we want to use
    const char *myAutoBaud;
    // if we set a new max range from the command line
    int myMaxRange;
    // if our new max range was really set
    bool myMaxRangeReallySet;
    /// the additional laser ignore readings
    const char *myAdditionalIgnoreReadings;
  };
  std::map<int, LaserData *> myLasers;
  
  /// Parses the laser arguments
  MVREXPORT bool parseLaserArgs(MvrArgumentParser *parser, 
			       LaserData *laserData);
  /// Logs the laser command line option help text. 
  MVREXPORT void logLaserOptions(LaserData *laserdata, bool header = true, bool metaOpts = true) const;
  // Sets the laser parameters
  bool internalConfigureLaser(LaserData *laserData);

  std::string myLaserTypes;

  // our parser
  MvrArgumentParser *myParser;
  bool myOwnParser;
  // if we should autoparse args or toss errors 
  bool myAutoParseArgs;
  bool myParsedArgs;

  MvrRobot *myRobot;
  MvrRobotConnector *myRobotConnector;

  MvrLog::LogLevel myInfoLogLevel;

  MvrRetFunctor1<bool, const char *> *myTurnOnPowerOutputCB;
  MvrRetFunctor1<bool, const char *> *myTurnOffPowerOutputCB;

  MvrRetFunctorC<bool, MvrLaserConnector> myParseArgsCB;
  MvrConstFunctorC<MvrLaserConnector> myLogOptionsCB;
};

#endif // MVRLASERCONNECTOR_H
