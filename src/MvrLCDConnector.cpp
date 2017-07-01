#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrLCDConnector.h"
#include "MvrRobot.h"
#include "MvrLCDMTX.h"
#include "mvriaInternal.h"
#include "MvrCommands.h"
#include "MvrRobotConfigPacketReader.h"

#include <sys/types.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <dirent.h>
#endif

#include <ctype.h>

/** @warning do not delete @a parser during the lifetime of this
 MvrLCDConnector, which may need to access its contents later.
 @param parser the parser with the arguments to parse
 @param robot the robot these lcds are attached to (or NULL for none)
 @param robotConnector the connector used for connecting to the robot
 (so we can see if it was a sim or not)
 @param autoParseArgs if this class should autoparse the args if they
 aren't parsed explicitly
 @param infoLogLevel The log level for information about creating
 lcds and such, this is also passed to all the lcds created as
 their infoLogLevel too
 */
MVREXPORT MvrLCDConnector::MvrLCDConnector (
  MvrArgumentParser *parser, MvrRobot *robot,
  MvrRobotConnector *robotConnector, bool autoParseArgs,
  MvrLog::LogLevel infoLogLevel,
  MvrRetFunctor1<bool, const char *> *turnOnPowerOutputCB,
  MvrRetFunctor1<bool, const char *> *turnOffPowerOutputCB) :
  myParseArgsCB (this, &MvrLCDConnector::parseArgs),
  myLogOptionsCB (this, &MvrLCDConnector::logOptions),
	myTurnOnPowerCB(this, &MvrLCDConnector::turnOnPowerCB),
	myTurnOffPowerCB(this, &MvrLCDConnector::turnOffPowerCB)

{
	myParser = parser;
	myOwnParser = false;
	myRobot = robot;
  myLCDLogPacketsReceived = false;
  myLCDLogPacketsSent = false;
	myRobotConnector = robotConnector;
	myAutoParseArgs = autoParseArgs;
	myParsedArgs = false;
	myInfoLogLevel = infoLogLevel;

  myTurnOnPowerOutputCB = turnOnPowerOutputCB;
  myTurnOffPowerOutputCB = turnOffPowerOutputCB;

	myParseArgsCB.setName ("MvrLCDConnector");
	Mvria::addParseArgsCB (&myParseArgsCB, 60);
	myLogOptionsCB.setName ("MvrLCDConnector");
	Mvria::addLogOptionsCB (&myLogOptionsCB, 80);
}
MVREXPORT MvrLCDConnector::~MvrLCDConnector (void)
{
//  Mvria::remParseArgsCB(&myParseArgsCB);
//  Mvria::remLogOptionsCB(&myLogOptionsCB);
}
/**
 * Parse command line arguments using the MvrArgumentParser given in the MvrLCDConnector constructor.
 *
 * See parseArgs(MvrArgumentParser*) for details about argument parsing.
 *
  @return true if the arguments were parsed successfully false if not
 **/
MVREXPORT bool MvrLCDConnector::parseArgs (void)
{
	return parseArgs (myParser);
}
/**
 * Parse command line arguments held by the given MvrArgumentParser.
 *
  @return true if the arguments were parsed successfully false if not

  The following arguments are accepted for lcd connections.  A program may request support for more than one lcd
  using setMaxNumLCDs(); if multi-lcd support is enabled in this way, then these arguments must have the lcd index
  number appended. For example, "-lcdPort" for lcd 1 would instead by "-lcdPort1", and for lcd 2 it would be
  "-lcdPort2".
  <dl>
    <dt>-lcdPort <i>port</i></dt>
    <dt>-lcdp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a lcd. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
    The default lcd port is COM2, which is the typical Pioneer lcd port setup.
    </dd>
    <dt>-connectLCD</dt>
    <dt>-clcd</dt>
    <dd>Explicitly request that the client program connect to a lcd, if it does not always do so</dd>
  </dl>
 **/
MVREXPORT bool MvrLCDConnector::parseArgs (MvrArgumentParser *parser)
{

	if (myParsedArgs)
		return true;
	myParsedArgs = true;
	bool typeReallySet;
	const char *type;
	char buf[1024];
	int i;
	std::map<int, LCDData *>::iterator it;
	LCDData *lcdData;
	bool wasReallySetOnlyTrue = parser->getWasReallySetOnlyTrue();
	parser->setWasReallySetOnlyTrue (true);

	for (i = 1; i <= Mvria::getMaxNumLCDs(); i++) {
		if (i == 1)
			buf[0] = '\0';
		else
			sprintf (buf, "%d", i);
		typeReallySet = false;
		// see if the lcd is being added from the command line
		if (!parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		    "-lcdType%s", buf) ||
		    !parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		        "-lcdt%s", buf)) {
			MvrLog::log (MvrLog::Normal,
			            "MvrLCDConnector: Bad lcd type given for lcd number %d",
			            i);
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}

		// if we didn't have an argument then just return
		if (!typeReallySet)
			continue;
		if ( (it = myLCDs.find (i)) != myLCDs.end()) {
			MvrLog::log (MvrLog::Normal, "MvrLCDConnector: A lcd already exists for lcd number %d, replacing it with a new one of type %s",
			            i, type);
			lcdData = (*it).second;
			delete lcdData;
			myLCDs.erase (i);
		}

		if (typeReallySet && type != NULL) {
			MvrLCDMTX *lcd = NULL;
			if ( (lcd = Mvria::lcdCreate (type, i, "MvrLCDConnector: ")) != NULL) {
				MvrLog::log (myInfoLogLevel,
				            "MvrLCDConnector: Created %s as lcd %d from arguments",
				            lcd->getName(), i);
				myLCDs[i] = new LCDData (i, lcd);
				lcd->setInfoLogLevel (myInfoLogLevel);
			} else {
				MvrLog::log (MvrLog::Normal,
				            "Unknown lcd type %s for lcd %d, choices are %s",
				            type, i, Mvria::lcdGetTypes());
				parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
				return false;
			}
		}
	}

  if (parser->checkArgument("-lcdLogPacketsReceived") || 
      parser->checkArgument("-lcdlpr")) 
    myLCDLogPacketsReceived = true;
  
  if (parser->checkArgument("-lcdLogPacketsSent") || 
      parser->checkArgument("-lcdlps"))      
    myLCDLogPacketsSent = true;

	// go through the robot param list and add the lcds defined
	// in the parameter file.
	const MvrRobotParams *params = NULL;
	if (myRobot != NULL) {
		params = myRobot->getRobotParams();
		if (params != NULL) {


			for (i = 1; i <= Mvria::getMaxNumLCDs(); i++) {
				// if we already have a lcd for this then don't add one from
				// the param file, since it was added either explicitly by a
				// program or from the command line
				if (myLCDs.find (i) != myLCDs.end())
					continue;
				type = params->getLCDMTXBoardType (i);
				// if we don't have a lcd type for that number continue
				if (type == NULL || type[0] == '\0')
					continue;

				int baud = params->getLCDMTXBoardBaud(i);
				if (baud == 0)
					continue;

				MvrLCDMTX *lcd = NULL;
				if ( (lcd =
				        Mvria::lcdCreate (type, i, "MvrLCDConnector: ")) != NULL) {

					MvrLog::log (myInfoLogLevel,
					            "MvrLCDConnector::parseArgs() Created %s as lcd %d from parameter file",
					            lcd->getName(), i);
					myLCDs[i] = new LCDData (i, lcd);
					lcd->setInfoLogLevel (myInfoLogLevel);
				} else {
					MvrLog::log (MvrLog::Normal,
					            "MvrLCDConnector::parseArgs() Unknown lcd type %s for lcd %d from the .p file, choices are %s",
					            type, i, Mvria::lcdGetTypes());
					parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
					return false;
				}
			}
		} else {
			MvrLog::log (MvrLog::Normal, "MvrLCDConnector::parseArgs() Have robot, but robot has NULL params, so cannot configure its lcd");
		}
	}
	// now go through and parse the args for any lcd that we have


	for (it = myLCDs.begin(); it != myLCDs.end(); it++) {
		lcdData = (*it).second;
		if (!parseLCDArgs (parser, lcdData)) {
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}
	}
	parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
	return true;

} // end parseArgs

MVREXPORT bool MvrLCDConnector::parseLCDArgs (MvrArgumentParser *parser,
    LCDData *lcdData)
{
	char buf[512];
	if (lcdData == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrLCDConnector::parseLCDArgs() Was given NULL lcd");
		return false;
	}
	if (lcdData->myLCD == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "MvrLCDConnector::parseLCDArgs() There is no lcd for lcd number %d but there should be",
		            lcdData->myNumber);
		return false;
	}
	MvrLCDMTX *lcd = lcdData->myLCD;
	if (lcdData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", lcdData->myNumber);

#if 0
	// see if we want to connect to the lcd automatically
	if (parser->checkArgumentVar ("-connectLCD%s", buf) ||
	    parser->checkArgumentVar ("-clcd%s", buf)) {
		lcdData->myConnect = true;
		lcdData->myConnectReallySet = true;
	}
#endif

	// see if we do not want to connect to the lcd automatically
	if (parser->checkArgumentVar ("-doNotConnectLCD%s", buf) ||
	    parser->checkArgumentVar ("-dnclcd%s", buf)) {
		lcdData->myConnect = false;
		lcdData->myConnectReallySet = true;
	}
	if (!parser->checkParameterArgumentStringVar (NULL, &lcdData->myPort,
	    "-lcdPort%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myPort,
	        "-lcdp%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myPortType,
	        "-lcdPortType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myPortType,
	        "-lcdpt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myType,
	        "-lcdType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myType,
	        "-lcdt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myBaud,
	        "-lcdBaud%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myBaud,
	        "-lcdb%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myAutoConn,
	        "-lcdAutoConn%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myAutoConn,
	        "-lcdac%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myConnFailOption,
	        "-lcdDisconnectOnConnectFailure%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &lcdData->myConnFailOption,
	        "-lcddocf%s", buf))
		{
		return false;
	}
	// PS - command line needs to set this to true
#if 0
	if (strcasecmp(lcdData->myAutoConn, "true") == 0) {
		lcdData->myConnect = true;
		lcdData->myConnectReallySet = true;
	}
#endif
	return internalConfigureLCD (lcdData);

} // end parseLCDArgs


bool MvrLCDConnector::internalConfigureLCD (
  LCDData *lcdData)
{
	MvrLCDMTX *lcd = lcdData->myLCD;
	if (lcd == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrLCDConnector::internalConfigureLCD() No lcd for number %d",
		            lcdData->myNumber);
		return false;
	}
	// the rest handles all the connection stuff
	const MvrRobotParams *params;
	char portBuf[1024];
	if (lcdData->myLCD == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrLCDConnector::internalConfigureLCD() There is no lcd, cannot connect");
		return false;
	}
	sprintf (portBuf, "%d", lcdData->myLCD->getDefaultTcpPort());
	if (myRobotConnector == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrLCDConnector::internalConfigureLCD() No MvrRobotConnector is passed in so simulators and remote hosts will not work correctly");
	}
	if ( (lcdData->myPort == NULL || strlen (lcdData->myPort) == 0) &&
	     (lcdData->myPortType != NULL && strlen (lcdData->myPortType) > 0)) {
		MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() There is a lcd port type given ('%s') for lcd %d (%s), but no lcd port given, cannot configure lcd",
		            lcdData->myPortType, lcdData->myNumber, lcd->getName());
		return false;
	}

	MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() command line lcd #%d type= %s port=%s portType=%s baud=%d autoconnect=%d disconnectonfailure=%d ",
							lcdData->myNumber, 
							lcdData->myType,
							lcdData->myPort,
							lcdData->myPortType,
							lcdData->myBaud,
							lcdData->myAutoConn,
							lcdData->myConnFailOption);


	if ( (lcdData->myPort != NULL && strlen (lcdData->myPort) > 0) &&
	     (lcdData->myPortType != NULL && strlen (lcdData->myPortType) > 0)) {
		MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() Connection type and port given for lcd %d (%s), so overriding everything and using that information",
		            lcdData->myNumber, lcd->getName());
		
		if ( (lcdData->myConn = Mvria::deviceConnectionCreate (
		                              lcdData->myPortType, lcdData->myPort, portBuf,
		                              "MvrLCDConnector:")) == NULL) {
			return false;
		}
		lcd->setDeviceConnection (lcdData->myConn);
		return true;
	}
	if ( (lcdData->myPort != NULL && strlen (lcdData->myPort) > 0) &&
	     (lcdData->myPortType == NULL || strlen (lcdData->myPortType) == 0)) {
		if (myRobot != NULL && (params = myRobot->getRobotParams()) != NULL) {
			if (params->getLCDMTXBoardPortType (lcdData->myNumber) != NULL &&
			    params->getLCDMTXBoardPortType (lcdData->myNumber) [0] != '\0') {
				MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() There is a port given, but no port type given so using the robot parameters port type");
				if ( (lcdData->myConn = Mvria::deviceConnectionCreate (
				                              params->getLCDMTXBoardPortType (lcdData->myNumber),
				                              lcdData->myPort, portBuf,
				                              "MvrLCDConnector: ")) == NULL) {
					return false;
				}
			} else if (lcd->getDefaultPortType() != NULL &&
			           lcd->getDefaultPortType() [0] != '\0') {
				MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() There is a port given for lcd %d (%s), but no port type given and no robot parameters port type so using the lcd's default port type", lcdData->myNumber, lcd->getName());
				if ( (lcdData->myConn = Mvria::deviceConnectionCreate (
				                              lcd->getDefaultPortType(),
				                              lcdData->myPort, portBuf,
				                              "MvrLCDConnector: ")) == NULL) {
					return false;
				}
			} else {
				MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() There is a port given for lcd %d (%s), but no port type given, no robot parameters port type, and no lcd default port type, so using serial",
				            lcdData->myNumber, lcd->getName());
				if ( (lcdData->myConn = Mvria::deviceConnectionCreate (
				                              "serial",
				                              lcdData->myPort, portBuf,
				                              "MvrLCDConnector: ")) == NULL) {
					return false;
				}
			}
			lcd->setDeviceConnection (lcdData->myConn);
			return true;
		} else {
			MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() There is a lcd port given ('%s') for lcd %d (%s), but no lcd port type given and there are no robot params to find the information in, so assuming serial",
			            lcdData->myPort, lcdData->myNumber, lcd->getName());
			if ( (lcdData->myConn = Mvria::deviceConnectionCreate (
			                              lcdData->myPortType, lcdData->myPort, portBuf,
			                              "MvrLCDConnector: ")) == NULL) {
				return false;
			}
			lcd->setDeviceConnection (lcdData->myConn);
			return true;
		}
	}
	// if we get down here there was no information provided by the command line or in a lcd connector, so see if we have params... if not then fail, if so then use those
	if (myRobot == NULL || (params = myRobot->getRobotParams()) == NULL) {
		MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() No robot params are available, and no command line information given on how to connect to the lcd %d (%s), so cannot connect", lcdData->myNumber, lcd->getName());
		return false;
	}

	MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() .p lcd #%d type= %s port=%s portType=%s baud=%d autoconnect=%d disconnectonfailure=%d",
							lcdData->myNumber, 
							params->getLCDMTXBoardType (lcdData->myNumber),
							params->getLCDMTXBoardPort (lcdData->myNumber),
							params->getLCDMTXBoardPortType (lcdData->myNumber),
							params->getLCDMTXBoardBaud (lcdData->myNumber),
							params->getLCDMTXBoardAutoConn (lcdData->myNumber),
							params->getLCDMTXBoardConnFailOption (lcdData->myNumber));

	// see if auto connect is on
	if (params->getLCDMTXBoardAutoConn (lcdData->myNumber)) {

		lcdData->myConnect = true;
		lcdData->myConnectReallySet = true;
	}

	MvrLog::log (MvrLog::Normal, "MvrLCDConnector::internalConfigureLCD() Using robot params for connecting to lcd %d (%s)", lcdData->myNumber, lcd->getName());

	if ( (lcdData->myConn = Mvria::deviceConnectionCreate (
	                              params->getLCDMTXBoardPortType (lcdData->myNumber),
	                              params->getLCDMTXBoardPort (lcdData->myNumber), portBuf,
	                              "MvrLCDConnector: ")) == NULL) {
		return false;
	}
	lcd->setDeviceConnection (lcdData->myConn);
	return true;
}
MVREXPORT void MvrLCDConnector::logOptions (void) const
{
	MvrLog::log (MvrLog::Terse, "Options for MvrLCDConnector:");
  MvrLog::log(MvrLog::Terse, "-lcdLogPacketsReceived");
  MvrLog::log(MvrLog::Terse, "-lcdlpr");
  MvrLog::log(MvrLog::Terse, "-lcdLogPacketsSent");
  MvrLog::log(MvrLog::Terse, "-lcdlps");
	MvrLog::log (MvrLog::Terse, "\nOptions shown are for currently set up lcds.  Activate lcds with -lcdType<N> option");
	MvrLog::log (MvrLog::Terse, "to see options for that lcd (e.g. \"-help -lcdType1 lcdMTX\").");
	MvrLog::log (MvrLog::Terse, "Valid lcd types are: %s", Mvria::lcdGetTypes());
	MvrLog::log (MvrLog::Terse, "\nSee docs for details.");
	std::map<int, LCDData *>::const_iterator it;
	LCDData *lcdData;
	for (it = myLCDs.begin(); it != myLCDs.end(); it++) {
		lcdData = (*it).second;
		logLCDOptions (lcdData);
	}
}
MVREXPORT void MvrLCDConnector::logLCDOptions (
  LCDData *lcdData, bool header, bool metaOpts) const
{
	char buf[512];
	if (lcdData == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "Tried to log lcd options with NULL lcd data");
		return;
	}
	if (lcdData->myLCD == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "MvrLCDConnector: There is no lcd for lcd number %d but there should be",
		            lcdData->myNumber);
		return;
	}
	MvrLCDMTX *lcd = lcdData->myLCD;
	if (lcdData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", lcdData->myNumber);
	if (header) {
		MvrLog::log (MvrLog::Terse, "");
		MvrLog::log (MvrLog::Terse, "LCD%s: (\"%s\")", buf, lcd->getName());
	}
	if (metaOpts) {
		MvrLog::log (MvrLog::Terse, "-lcdType%s <%s>", buf, Mvria::lcdGetTypes());
		MvrLog::log (MvrLog::Terse, "-lcdt%s <%s>", buf, Mvria::lcdGetTypes());
		MvrLog::log (MvrLog::Terse, "-connectLCD%s", buf);
		MvrLog::log (MvrLog::Terse, "-clcd%s", buf);
	}
	MvrLog::log (MvrLog::Terse, "-lcdPort%s <lcdPort>", buf);
	MvrLog::log (MvrLog::Terse, "-lcdp%s <lcdPort>", buf);
	MvrLog::log (MvrLog::Terse, "-lcdPortType%s <%s>", buf, Mvria::deviceConnectionGetTypes());
	MvrLog::log (MvrLog::Terse, "-lcdpt%s <%s>", buf, Mvria::deviceConnectionGetTypes());
	MvrLog::log (MvrLog::Terse, "-remoteLCDTcpPort%s <remoteLCDTcpPort>", buf);
	MvrLog::log (MvrLog::Terse, "-rbtp%s <remoteLCDTcpPort>", buf);
}
/**
   Normally adding lcds is done from the .p file, you can use this
   if you want to add them explicitly in a program (which will
   override the .p file, and may cause some problems).
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrLCDConnector::connectLCDs.()
   @internal
**/
MVREXPORT bool MvrLCDConnector::addLCD (
  MvrLCDMTX *lcd, int lcdNumber)
{
	std::map<int, LCDData *>::iterator it;
	LCDData *lcdData = NULL;
	if ( (it = myLCDs.find (lcdNumber)) != myLCDs.end())
		lcdData = (*it).second;
	if (lcdData != NULL) {
		if (lcdData->myLCD != NULL)
			MvrLog::log (MvrLog::Terse,
			            "MvrLCDConnector::addLCD: Already have lcd for number #%d of type %s but a replacement lcd of type %s was passed in",
			            lcdNumber, lcdData->myLCD->getName(), lcd->getName());
		else
			MvrLog::log (MvrLog::Terse,
			            "MvrLCDConnector::addLCD: Already have lcd for number #%d but a replacement lcd of type %s was passed in",
			            lcdNumber, lcd->getName());
		delete lcdData;
		myLCDs.erase (lcdNumber);
	}
	myLCDs[lcdNumber] = new LCDData (lcdNumber, lcd);
	return true;
}
MVREXPORT MvrLCDMTX *MvrLCDConnector::getLCD (int lcdNumber)
{
	std::map<int, LCDData *>::iterator it;
	LCDData *lcdData = NULL;
	if ( (it = myLCDs.find (lcdNumber)) != myLCDs.end())
		lcdData = (*it).second;
	// if we have no lcd, we can't get it so just return
	if (lcdData == NULL)
		return NULL;
	// otherwise, return the lcd
	return lcdData->myLCD;
}
MVREXPORT bool MvrLCDConnector::replaceLCD (
  MvrLCDMTX *lcd, int lcdNumber)
{
	std::map<int, LCDData *>::iterator it;
	LCDData *lcdData = NULL;
	if ( (it = myLCDs.find (lcdNumber)) != myLCDs.end())
		lcdData = (*it).second;
	// if we have no lcd, we can't replace it so just return
	if (lcdData == NULL)
		return false;
	if (lcdData->myLCD != NULL)
		MvrLog::log (myInfoLogLevel,
		            "MvrLCDConnector::replaceLCD: Already have lcd for number #%d of type %s but a replacement lcd of type %s was passed in",
		            lcdNumber, lcdData->myLCD->getName(), lcd->getName());
	else
		MvrLog::log (MvrLog::Normal,
		            "MvrLCDConnector::replaceLCD: Replacing a non existant lcd number #%d with a lcd of type %s passed in",
		            lcdNumber, lcd->getName());
	lcdData->myLCD = lcd;
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrLCDConnector::connectLCDs().
   @internal
**/
MVREXPORT bool MvrLCDConnector::setupLCD (MvrLCDMTX *lcd,
    int lcdNumber)
{
	if (myRobot == NULL && myRobotConnector != NULL)
		myRobot = myRobotConnector->getRobot();
	std::map<int, LCDData *>::iterator it;
	LCDData *lcdData = NULL;
	//const MvrRobotParams *params;
	if ( (it = myLCDs.find (lcdNumber)) != myLCDs.end())
		lcdData = (*it).second;
	if (lcdData == NULL && lcd == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrLCDConnector::setupLCD: Do not have lcd #%d", lcdNumber) ;
		return false;
	}
	if (lcdData != NULL && lcd != NULL &&
	    lcdData->myLCD != lcd) {
		if (lcdData->myLCD != NULL)
			MvrLog::log (MvrLog::Terse, "MvrLCDConnector::setupLCD: Already have lcd for number #%d (%s) but a replacement lcd (%s) was passed in, this will replace all of the command line arguments for that lcd",
			            lcdNumber, lcdData->myLCD->getName(), lcd->getName());
		else
			MvrLog::log (MvrLog::Terse, "MvrLCDConnector::setupLCD: Already have lcd for number #%d but a replacement lcd (%s) was passed in, this will replace all of the command line arguments for that lcd",
			            lcdNumber, lcd->getName());
		delete lcdData;
		myLCDs.erase (lcdNumber);
		myLCDs[lcdNumber] = new LCDData (lcdNumber, lcd);
	}
	if (lcdData == NULL && lcd != NULL) {
		lcdData = new LCDData (lcdNumber, lcd);
		myLCDs[lcdNumber] = lcdData;
		if (myAutoParseArgs && !parseLCDArgs (myParser, lcdData)) {
			MvrLog::log (MvrLog::Verbose, "MvrLCDConnector: Auto parsing args for lcd %s (num %d)", lcdData->myLCD->getName(), lcdNumber);
			return false;
		}
	}
	// see if there is no lcd (ie if it was a sick done in the old
	// style), or if the lcd passed in doesn't match the one this
	// class created (I don't know how it'd happen, but...)... and then
	// configure it
	if ( (lcdData->myLCD == NULL || lcdData->myLCD != lcd)) {
		if (!internalConfigureLCD (lcdData))
			return false;
	}
	// setupLCD automatically adds this to the robot, since the
	// connectlcd stuff is the newer more supported way and is more
	// configurable.. it only adds it as a lcd since the legacy code
	// won't add it that way, but will add it as a range device
	if (myRobot != NULL) {
		myRobot->addLCD (lcd, lcdNumber);
		//myRobot->addRangeDevice(lcd);
	} else {
		MvrLog::log (MvrLog::Normal, "MvrLCDConnector::setupLCD: No robot, so lcd cannot be added to robot");
	}
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrLCDConnector::connectLCDs().
   @internal
**/
MVREXPORT bool MvrLCDConnector::connectLCD (MvrLCDMTX *lcd,
    int lcdNumber,
    bool forceConnection)
{
	std::map<int, LCDData *>::iterator it;
	LCDData *lcdData = NULL;
	lcd->lockDevice();
	// set up the lcd regardless
	if (!setupLCD (lcd, lcdNumber)) {
		lcd->unlockDevice();
		return false;
	}
	lcd->unlockDevice();
	if ( (it = myLCDs.find (lcdNumber)) != myLCDs.end())
		lcdData = (*it).second;
	if (lcdData == NULL) {
		MvrLog::log (MvrLog::Normal, "MvrLCDConnector::connectLCD: Some horrendous error in connectLCD with lcd number %d", lcdNumber);
		return false;
	}
	// see if we want to connect
	if (!forceConnection && !lcdData->myConnect)
		return true;
	else
		return lcd->blockingConnect(myLCDLogPacketsSent, myLCDLogPacketsReceived, lcdData->myNumber,
												&myTurnOnPowerCB, &myTurnOffPowerCB);

}
MVREXPORT bool MvrLCDConnector::connectLCDs (
  bool continueOnFailedConnect, bool addConnectedLCDsToRobot,
  bool addAllLCDsToRobot, bool turnOnLCDs,
  bool powerCycleLCDOnFailedConnect)
{
	std::map<int, LCDData *>::iterator it;
	LCDData *lcdData = NULL;
	MvrLog::log (myInfoLogLevel,
	            "MvrLCDConnector::connectLCDs() Connecting lcds");
	if (myAutoParseArgs && !myParsedArgs) {
		MvrLog::log (MvrLog::Verbose,
		            "MvrLCDConnector::connectLCDs() Auto parsing args for lcds");
		if (!parseArgs()) {
			return false;
		}
	}
	if (addAllLCDsToRobot) {

			MvrLog::log (MvrLog::Normal,
		            "MvrLCDConnector::connectLCDs() addAllLCDsToRobot");

		if (myRobot != NULL) {

			for (it = myLCDs.begin(); it != myLCDs.end(); it++) {
				lcdData = (*it).second;
				myRobot->addLCD (lcdData->myLCD, lcdData->myNumber);
				MvrLog::log (MvrLog::Verbose,
				            "MvrLCDConnector::connectLCDs: Added %s to robot as lcd %d",
				            lcdData->myLCD->getName(), lcdData->myNumber);
			}
		} else {
			MvrLog::log (MvrLog::Normal, "MvrLCDConnector::connectLCDs: Supposed to add all lcds to robot, but there is no robot");
			return false;
		}
	}
	for (it = myLCDs.begin(); it != myLCDs.end(); it++) {

		lcdData = (*it).second;

		if (lcdData == NULL)
			continue;

		//verifyFirmware(lcdData);

		lcdData->myConn->close();

		turnOnPower(lcdData);

		if (lcdData->myConnectReallySet && lcdData->myConnect) {
			MvrLog::log (myInfoLogLevel,
			            "MvrLCDConnector::connectLCDs: Connecting %s",
			            lcdData->myLCD->getName());
			lcdData->myLCD->setRobot (myRobot);
			bool connected = false;
			connected = lcdData->myLCD->blockingConnect(myLCDLogPacketsSent, 
												myLCDLogPacketsReceived, lcdData->myNumber,
												&myTurnOnPowerCB, &myTurnOffPowerCB);
			if (connected) {
				if (!addAllLCDsToRobot && addConnectedLCDsToRobot) {
					if (myRobot != NULL) {
						myRobot->addLCD (lcdData->myLCD, lcdData->myNumber);
						//myRobot->addRangeDevice(lcdData->myLCD);
						MvrLog::log (MvrLog::Verbose,
						            "MvrLCDConnector::connectLCDs: Added %s to robot",
						            lcdData->myLCD->getName());
					} else {
						MvrLog::log (MvrLog::Normal,
						            "MvrLCDConnector::connectLCDs: Could not add %s to robot, since there is no robot",
						            lcdData->myLCD->getName());
					}
				} else if (addAllLCDsToRobot && myRobot != NULL) {
					MvrLog::log (MvrLog::Verbose,
					            "MvrLCDConnector::connectLCDs: %s already added to robot)",
					            lcdData->myLCD->getName());
				} else if (myRobot != NULL) {
					MvrLog::log (MvrLog::Verbose,
					            "MvrLCDConnector::connectLCDs: Did not add %s to robot",
					            lcdData->myLCD->getName());
				}
			} else {
				if (!continueOnFailedConnect) {
					MvrLog::log (MvrLog::Normal,
					            "MvrLCDConnector::connectLCDs: Could not connect %s, stopping",
					            lcdData->myLCD->getName());
					return false;
				} else
					MvrLog::log (MvrLog::Normal,
					            "MvrLCDConnector::connectLCDs: Could not connect %s, continuing with remainder of lcds",
					            lcdData->myLCD->getName());
			}
		}
	}
	MvrLog::log (myInfoLogLevel,
	            "MvrLCDConnector: Done connecting lcds");
	return true;
}

MVREXPORT bool MvrLCDConnector::turnOnPower (LCDData *LCDData)
{
	/// MPL the new way
	if (myTurnOnPowerOutputCB != NULL) {
		if (myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
		      LCDData->myNumber) == NULL ||
		    myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
		      LCDData->myNumber) [0] == '\0') {
			MvrLog::log (MvrLog::Normal,
			            "MvrLCDConnector::turnOnPower: LCD %d has no power output set so can't be turned on (things may still work).",
			            LCDData->myNumber);
			return false;
		} else {
			if (myTurnOnPowerOutputCB->invokeR (
			      myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
			        LCDData->myNumber))) {
				MvrLog::log (myInfoLogLevel,
				            "MvrLCDConnector::turnOnPower: Turned on power output %s for lcd %d",
				            myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
				              LCDData->myNumber),
				            LCDData->myNumber);
				return true;
			} else {
				MvrLog::log (MvrLog::Normal,
				            "MvrLCDConnector::turnOnPower: Could not turn on power output %s for lcd %d (things may still work).",
				            myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
				              LCDData->myNumber),
				            LCDData->myNumber);
				return false;
			}
		}
	}
	return false;
}

MVREXPORT bool MvrLCDConnector::turnOffPower (LCDData *LCDData)
{
	/// MPL the new way
	if (myTurnOffPowerOutputCB != NULL) {
		if (myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
		      LCDData->myNumber) == NULL ||
		    myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
		      LCDData->myNumber) [0] == '\0') {
			MvrLog::log (MvrLog::Normal,
			            "MvrLCDConnector::turnOffPower: LCD %d has no power output set so can't be turned off (things may still work).",
			            LCDData->myNumber);
			return false;
		} else {
			if (myTurnOffPowerOutputCB->invokeR (
			      myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
			        LCDData->myNumber))) {
				MvrLog::log (myInfoLogLevel,
				            "MvrLCDConnector::turnOffPower: Turned off power output %s for lcd %d",
				            myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
				              LCDData->myNumber),
				            LCDData->myNumber);
				return true;
			} else {
				MvrLog::log (MvrLog::Normal,
				            "MvrLCDConnector::turnOffPower: Could not turn off power output %s for lcd %d (things may still work).",
				            myRobot->getRobotParams()->getLCDMTXBoardPowerOutput (
				              LCDData->myNumber),
				            LCDData->myNumber);
				return false;
			}
		}
	}
	return false;
}

MVREXPORT bool MvrLCDConnector::verifyFirmware (LCDData *LCDData)

{


	// first we need to turn off the power, then turn it back on
	if (!turnOffPower(LCDData))
		return false;

	MvrUtil::sleep(3000);

	if (!turnOnPower(LCDData))
		return false;



	// now connect to the serial port

	MvrSerialConnection *serConn = NULL;
	serConn = dynamic_cast<MvrSerialConnection *> (LCDData->myConn);
	if (serConn != NULL)
		serConn->setBaud (115200);
	if (LCDData->myConn->getStatus() != MvrDeviceConnection::STATUS_OPEN
	    && !LCDData->myConn->openSimple()) {
		MvrLog::log (
		  MvrLog::Normal,
		  "MvrLCDConnector::verifyFirmware: Could not connect (%d) because the connection was not open and could not open it",
		  LCDData->myNumber);

		return false;
	}



	//  now send hello - and see if we get a response

	unsigned char hello = 0xc1;
  
 //// MvrBasePacket *packet;
 // while ((packet = readPacket()) != NULL)

	MvrTime timeDone;

	if (!timeDone.addMSec(30 * 1000))
	{
		MvrLog::log(MvrLog::Normal,
				"MvrLCDConnector::verifyFirmware(%d) error adding msecs (30 * 1000)",
				LCDData->myNumber);
	}

	unsigned char helloResp[4];
	bool gotResponse= false;
	int hmiVersion = 0;
	int hmiRevision = 0;

	while (timeDone.mSecTo() > 0) {

		if ((LCDData->myConn->write((char *)&hello, 1)) == -1) {

			MvrLog::log(MvrLog::Normal,
					"MvrLCDConnector::verifyFirmware(%d) Could not send hello to LCD", LCDData->myNumber);
			return false;
		}

		if ((LCDData->myConn->read((char *) &helloResp[0], 4, 500)) > 0) {

			MvrLog::log(MvrLog::Normal,
					"MvrLCDConnector::verifyFirmware(%d) received hello response 0x%02x 0x%02x 0x%02x 0x%02x",
					LCDData->myNumber, helloResp[0],  helloResp[1],  helloResp[2],  helloResp[3] );

			if ((helloResp[0] == 0xc0) && (helloResp[3] == 0x4b)) {
				MvrLog::log(MvrLog::Normal,
					"MvrLCDConnector::verifyFirmware(%d) received hello response",
					LCDData->myNumber);

				gotResponse = true;
				hmiVersion = helloResp[1];
				hmiRevision = helloResp[2];

				break;

			}

		}
		else {
			MvrLog::log(MvrLog::Normal,
					"MvrLCDConnector::verifyFirmware(%d) read failed",
					LCDData->myNumber);

		}
	}

	if (!gotResponse) {
		MvrLog::log(MvrLog::Normal,
				"MvrLCDConnector::verifyFirmware(%d) Received no hello response", LCDData->myNumber);
		return false;
	}

	// now make sure we have a file

	std::string hmiFile;
	hmiFile.clear();

	bool foundHmi = false;
	char hmiFilePrefix[100];
	hmiFilePrefix[0] = '\0';
	std::string baseDir = "/usr/local/apps/marcDownload/";

			
	sprintf(hmiFilePrefix,"AdeptHmi");


	hmiFile = searchForFile(baseDir.c_str(), hmiFilePrefix, ".ds");

	char hmiFileOut[100];
	hmiFileOut[0] = '\0';

	if (hmiFile.empty()) {
		MvrLog::log(MvrLog::Normal,
				"MvrLCDConnector::verifyFirmware(%d) can't find hmi file with prefix = %s", 
						LCDData->myNumber, hmiFilePrefix);
		return false;
	}
	else {
		// validate that the file dosn't match the current version

		sprintf(hmiFileOut,"AdeptHmi%02x%02x.ds", 
							hmiVersion, hmiRevision);
				
		if (strcmp(hmiFile.c_str(), hmiFileOut) != 0) { 
			foundHmi = true;
		}
		else {
			MvrLog::log(MvrLog::Normal,
				"MvrLCDConnector::verifyFirmware(%d) hmi file found but version matches (%s)",
						LCDData->myNumber, hmiFileOut);
			return false;
		}
	}



	if (!foundHmi) {
		MvrLog::log(MvrLog::Normal,
				"MvrLCDConnector::verifyFirmware(%d) Can not find %s.ds file", LCDData->myNumber, hmiFileOut);
		return false;
	}




	FILE *file;
	char line[10000];
	unsigned char c;

	std:: string hmiDirFile = baseDir + hmiFile;
	if ( (file = MvrUtil::fopen (hmiDirFile.c_str(), "r")) == NULL) {
		MvrLog::log (MvrLog::Normal,
			            "MvrLCDConnector::verifyFirmware(%d) Could not open file %s for reading errno (%d)",
			            LCDData->myNumber, hmiDirFile.c_str(), errno);
		return false;
	}

	line[0] = '\0';

	while (fgets (line, sizeof (line), file) != NULL) {

		MvrArgumentBuilder builder;
		builder.add(line);

		int i;

		std::string data;
		data.clear();

		for (i = 0; i < builder.getArgc(); i++) {
	
			if (!builder.isArgInt(i, true)) {

				MvrLog::log(MvrLog::Normal,
							"MvrLCDConnector::verifyFirmware(%d) Could not convert file", LCDData->myNumber);
				return false;

			}
	
			char ch = builder.getArgInt(i, NULL, true);
			data.push_back(ch);
			


		} // end for

		
				//MvrLog::log(MvrLog::Normal,
			//			"MvrLCDConnector::verifyFirmware(%d) data = %s size = %d", LCDData->myNumber, data.c_str(), data.size());

				//MvrLog::log(MvrLog::Normal,
				//		"MvrLCDConnector::verifyFirmware(%d) %d %c 0x%02x", LCDData->myNumber, data, data, data);
			if ((LCDData->myConn->write((char *)data.c_str(), data.size())) == -1) {
				MvrLog::log(MvrLog::Normal,
						"MvrLCDConnector::verifyFirmware(%d) Could not send data 0x%02x size(%d) to LCD errno (%d)", LCDData->myNumber, data.c_str(), data.size(), errno);
				return false;
			}


			// wait a sec for the response
			if ((LCDData->myConn->read((char *) &c, 1, 1000)) > 0) {
		
				if (c == 0x4b) 
					continue;
				else {
					MvrLog::log(MvrLog::Normal,
							"MvrLCDConnector::verifyFirmware(%d) Invalid response %x02%x from LCD to load data", 
							LCDData->myNumber, c);
					return false;
				}

			}
			else {
				MvrLog::log(MvrLog::Normal,
						"MvrLCDConnector::verifyFirmware(%d) Did not get response from LCD to load data", LCDData->myNumber);
				return false;

			}

	} // end while
	if (feof(file)) {
		// end of file reached
		MvrLog::log (MvrLog::Normal,
			            "MvrLCDConnector::verifyFirmware(%d) LCD firmware updated",
			            LCDData->myNumber);
		fclose (file);

		MvrUtil::sleep(5000);
		return true;

	}
	else {

		MvrLog::log (MvrLog::Normal,
			            "MvrLCDConnector::verifyFirmware(%d) failed updating LCD firmware",
			            LCDData->myNumber);
		fclose (file);

		return false;

	}
}


// TODO move searchForFile to ariaUtil
#ifdef WIN32

MVREXPORT std::string MvrLCDConnector::searchForFile(const char *dirname, const char *prefix, const char *suffix)
{
  // todo recurse into directories? (bool flag?)
  std::string filepattern = std::string(dirname) + "\\" + prefix + "*" + suffix;
  printf("MvrLCDConnector searching for %s...\n", filepattern.c_str());
  WIN32_FIND_DATA fileData;
  HANDLE h = FindFirstFileEx(filepattern.c_str(), FindExInfoBasic, &fileData, FindExSearchNameMatch, NULL, 0);
  if(h == INVALID_HANDLE_VALUE) 
  {
	  // nothing found
	  return "";
  }
  printf("found %s.\n", fileData.cFileName);
  FindClose(h);
  return fileData.cFileName;
}

#else

MVREXPORT std::string MvrLCDConnector::searchForFile(
	const char *dirToLookIn, const char *prefix, const char *suffix)
{

#ifdef WIN32
	return "";
#else
  /***
  MvrLog::log(MvrLog::Normal, 
             "MvrUtil::matchCase() dirToLookIn = \"%s\" fileName = \"%s\"",
             dirToLookIn,
             fileName);
  ***/

  DIR *dir;
  struct dirent *ent;

  /*
  for (it = split.begin(); it != split.end(); it++)
  {
    printf("@@@@@@@@ %s\n", (*it).c_str());
  }
  */
  

  // how this works is we start at the base dir then read through
  // until we find what the next name we need, if entry is a directory
  // and we're not at the end of our string list then we change into
  // that dir and the while loop keeps going, if the entry isn't a
  // directory and matchs and its the last in our string list we've
  // found what we want
  if ((dir = opendir(dirToLookIn)) == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvramUtil::findFile: No such directory '%s' for base", 
	       dirToLookIn);
    return "";
  }

  while ((ent = readdir(dir)) != NULL)
  {
    // ignore some of these
    if (ent->d_name[0] == '.')
    {
      //printf("Ignoring %s\n", ent->d_name[0]);
      continue;
    }
    //printf("NAME %s finding %s\n", ent->d_name, finding.c_str());

    // continue if the prefix should be searched for and doesn't match
    if (prefix != NULL && prefix[0] != '\0' && 
	strncasecmp(ent->d_name, prefix, strlen(prefix)) != 0)
      continue;

    // continue if the suffix should be searched for and doesn't match
    if (suffix != NULL && suffix[0] != '\0' &&
	strlen(ent->d_name) > strlen(suffix) + 1 &&
	strncasecmp(&ent->d_name[strlen(ent->d_name) - strlen(suffix)], 
		    suffix, strlen(suffix)) != 0)
      continue;
    
    std::string ret = ent->d_name;
    closedir(dir);
    return ret;
  }
  //printf("!!!!!!!! %s", finding.c_str());
  closedir(dir);
  return "";
#endif
} 
#endif

MVREXPORT void MvrLCDConnector::turnOnPowerCB (int lcdNumber)
{
std::map<int, LCDData *>::iterator it;
LCDData *lcdData;

	if ( (it = myLCDs.find (lcdNumber)) != myLCDs.end()) {
		lcdData = (*it).second;

		if (lcdData != NULL) {
			turnOnPower(lcdData);
		}
	}

}

MVREXPORT void MvrLCDConnector::turnOffPowerCB (int lcdNumber)
{

std::map<int, LCDData *>::iterator it;
LCDData *lcdData;

	if ( (it = myLCDs.find (lcdNumber)) != myLCDs.end()) {
		lcdData = (*it).second;

		if (lcdData != NULL) {
			turnOffPower(lcdData);
		}
	}

}

MVREXPORT void MvrLCDConnector::setIdentifier(const char *identifier)
{

	// go thru list of lcd's and set the identifiers

	std::map<int, LCDData *>::const_iterator it;
	LCDData *lcdData;
	for (it = myLCDs.begin(); it != myLCDs.end(); it++) {
		lcdData = (*it).second;
		lcdData->myLCD->setIdentifier(identifier);
	}

}
