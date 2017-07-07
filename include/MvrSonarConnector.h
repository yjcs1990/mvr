#ifndef MVRSONARCONNECTOR_H
#define MVRSONARCONNECTOR_H

#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "mvriaUtil.h"
#include "MvrRobotConnector.h"

class MvrSonarMTX;
class MvrRobot;



/// Connect to sonar based on robot parameters and command-line arguments
/**

   MvrSonarConnector makes a sonar connection either through a serial port 
   connection.
   When you create your MvrSonarConnector, pass it command line parameters via
   either the argc and argv vmvriables from main(), or pass it an
   MvrArgumentBuilder or MvrArgumentParser object. (MvrArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   MvrSonarConnector registers a callback with the global Mvr class. Use
   Mvria::parseArgs() to parse all command line parameters to the program, and
   Mvria::logOptions() to print out information about all registered command-line parameters.

   The following command-line arguments are checked:
   @verbinclude MvrSonarConnector_options

   To connect to any sonars that were set up in the robot parameter file or
   via command line arguments, call connectSonars().  If successful, 
   connectSonars() will return true and add an entry for each sonar connected
   in the MvrRobot object's list of sonars.  These MvrSonarMTX objects can be
   accessed from your MvrRobot object via MvrRobot::findSonar() or MvrRobot::getSonarMap(). 
   

   @since 2.8.0

 **/
class MvrSonarConnector
{
public:
  /// Constructor that takes argument parser
  MVREXPORT MvrSonarConnector(
	  MvrArgumentParser *parser, 
	  MvrRobot *robot, MvrRobotConnector *robotConnector,
	  bool autoParseArgs = true,
	  MvrLog::LogLevel infoLogLevel = MvrLog::Verbose,
      	  MvrRetFunctor1<bool, const char *> *turnOnPowerOutputCB = NULL,
	  MvrRetFunctor1<bool, const char *> *turnOffPowerOutputCB = NULL);
  /// Destructor
  MVREXPORT ~MvrSonarConnector(void);
  /// Connects all the sonars the robot has that should be auto connected
  MVREXPORT bool connectSonars(
						bool continueOnFailedConnect = false,
			      bool addConnectedSonarsToRobot = true,
			      bool addAllSonarsToRobot = false,
			      bool turnOnSonars = true,
			      bool powerCycleSonarOnFailedConnect = true);
	// Connects all the sonars in replay mode
	MVREXPORT bool connectReplaySonars(
						bool continueOnFailedConnect = false,
			      bool addConnectedSonarsToRobot = true,
			      bool addAllSonarsToRobot = false,
			      bool turnOnSonars = true,
			      bool powerCycleSonarOnFailedConnect = true);
  /// Sets up a sonar to be connected
  MVREXPORT bool setupSonar(MvrSonarMTX *sonar, 
			   int sonarNumber = 1);
  /// Connects the sonar synchronously (will take up to a minute)
  MVREXPORT bool connectSonar(MvrSonarMTX *sonar,
			     int sonarNumber = 1,
			     bool forceConnection = true);
  /// Adds a sonar so parsing will get it
  MVREXPORT bool addSonar(MvrSonarMTX *sonar,
			 int sonarNumber = 1);
  /// Function to parse the arguments given in the constructor
  MVREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  MVREXPORT bool parseArgs(MvrArgumentParser *parser);
  /// Log the options the simple connector has
  MVREXPORT void logOptions(void) const;
  /// Internal function to get the sonar (only useful between parseArgs and connectSonars)
  MVREXPORT MvrSonarMTX *getSonar(int sonarNumber);

  /// Internal function to replace the sonar (only useful between parseArgs and connectSonars) but not the sonar data
  MVREXPORT bool replaceSonar(MvrSonarMTX *sonar, int sonarNumber);
  
  MVREXPORT bool disconnectSonars();
protected:
/// Class that holds information about the sonar data
class SonarData
{
	public:
		SonarData (int number, MvrSonarMTX *sonar) {
			myNumber = number;
			mySonar = sonar;
			myConn = NULL;
			myConnect = false; myConnectReallySet = false;
			myPort = NULL;
			myPortType = NULL;
			myType = NULL;
			myRemoteTcpPort = 0; myRemoteTcpPortReallySet = false;
			myBaud = NULL;
			myAutoConn = NULL;
		}
		virtual ~SonarData() {}
		/// The number of this sonar
		int myNumber;
		/// The actual pointer to this sonar
		MvrSonarMTX *mySonar;
		// our connection
		MvrDeviceConnection *myConn;
		// if we want to connect the sonar
		bool myConnect;
		// if myConnect was really set
		bool myConnectReallySet;
		// the port we want to connect the sonar on
		const char *myPort;
		// the type of port we want to connect to the sonar on
		const char *myPortType;
		// sonar Type
		const char *myType;
		// wheather to auto conn
		const char *myAutoConn;
		// sonar tcp port if we're doing a remote host
		int myRemoteTcpPort;
		// if our remote sonar tcp port was really set
		bool myRemoteTcpPortReallySet;
		/// the baud we want to use
		const char *myBaud;
    /// Set baud convert from integer
    void setBaud(int baud) 
    {
      snprintf(myBaudBuf, 256, "%d", baud);
      myBaud = (const char*)myBaudBuf;
    }
    /// set AutoConn from boolean
    void setAutoConn(bool a)
    {
      snprintf(myAutoConnBuf, 256, "%s", a?"true":"false");
      myAutoConn = (const char *)myAutoConnBuf;
    }
private:
    /// Stores baud string if converted from an integer by setBaud()
    char myBaudBuf[256];
    /// Stores AutoConn string if converted from a boolean by setAutConn()
    char myAutoConnBuf[256];
};

	/// Turns on the power for the specific board in the firmware
	MVREXPORT bool turnOnPower(SonarData *sonarData);

  std::map<int, SonarData *> mySonars;
  
  /// Parses the sonar arguments
  MVREXPORT bool parseSonarArgs(MvrArgumentParser *parser, 
			       SonarData *sonarData);
  /// Logs the sonar command line option help text. 
  MVREXPORT void logSonarOptions(SonarData *sonardata, bool header = true, bool metaOpts = true) const;
  // Sets the sonar parameters
  bool internalConfigureSonar(SonarData *sonarData);

  std::string mySonarTypes;

  // our parser
  MvrArgumentParser *myParser;
  bool myOwnParser;
  // if we should autoparse args or toss errors 
  bool myAutoParseArgs;
  bool myParsedArgs;

  MvrRobot *myRobot;
  MvrRobotConnector *myRobotConnector;

  // vmvriables to hold if we're logging or not
  bool mySonarLogPacketsReceived;
  bool mySonarLogPacketsSent;

  MvrLog::LogLevel myInfoLogLevel;

  MvrRetFunctor1<bool, const char *> *myTurnOnPowerOutputCB;
  MvrRetFunctor1<bool, const char *> *myTurnOffPowerOutputCB;

  MvrRetFunctorC<bool, MvrSonarConnector> myParseArgsCB;
  MvrConstFunctorC<MvrSonarConnector> myLogOptionsCB;
};

#endif // ARLASERCONNECTOR_H
