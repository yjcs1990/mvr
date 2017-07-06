#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSonarConnector.h"
#include "MvrRobot.h"
#include "MvrSonarMTX.h"
#include "mvriaInternal.h"
#include "MvrCommands.h"
#include "MvrRobotConfigPacketReader.h"
/** @warning do not delete @a parser during the lifetime of this
 MvrSonarConnector, which may need to access its contents later.
 @param parser the parser with the arguments to parse
 @param robot the robot these sonars are attached to (or NULL for none)
 @param robotConnector the connector used for connecting to the robot
 (so we can see if it was a sim or not)
 @param autoParseArgs if this class should autoparse the args if they
 aren't parsed explicitly
 @param infoLogLevel The log level for information about creating
 sonars and such, this is also passed to all the sonars created as
 their infoLogLevel too
 */
MVREXPORT MvrSonarConnector::MvrSonarConnector (
  MvrArgumentParser *parser, MvrRobot *robot,
  MvrRobotConnector *robotConnector, bool autoParseArgs,
  MvrLog::LogLevel infoLogLevel,
  MvrRetFunctor1<bool, const char *> *turnOnPowerOutputCB,
  MvrRetFunctor1<bool, const char *> *turnOffPowerOutputCB) :
	myParseArgsCB (this, &MvrSonarConnector::parseArgs),
	myLogOptionsCB (this, &MvrSonarConnector::logOptions)
{
	myParser = parser;
	myOwnParser = false;
	myRobot = robot;
  mySonarLogPacketsReceived = false;
  mySonarLogPacketsSent = false;
	myRobotConnector = robotConnector;
	myAutoParseArgs = autoParseArgs;
	myParsedArgs = false;
	myInfoLogLevel = infoLogLevel;

  myTurnOnPowerOutputCB = turnOnPowerOutputCB;
  myTurnOffPowerOutputCB = turnOffPowerOutputCB;

	myParseArgsCB.setName ("MvrSonarConnector");
	Mvria::addParseArgsCB (&myParseArgsCB, 60);
	myLogOptionsCB.setName ("MvrSonarConnector");
	Mvria::addLogOptionsCB (&myLogOptionsCB, 80);
}
MVREXPORT MvrSonarConnector::~MvrSonarConnector (void)
{
//  Mvria::remParseArgsCB(&myParseArgsCB);
//  Mvria::remLogOptionsCB(&myLogOptionsCB);
}
/**
 * Parse command line arguments using the MvrArgumentParser given in the MvrSonarConnector constructor.
 *
 * See parseArgs(MvrArgumentParser*) for details about argument parsing.
 *
  @return true if the arguments were parsed successfully false if not
 **/
MVREXPORT bool MvrSonarConnector::parseArgs (void)
{
	return parseArgs (myParser);
}
/**
 * Parse command line arguments held by the given MvrArgumentParser.
 *
  @return true if the arguments were parsed successfully false if not

  The following arguments are accepted for sonar connections.  A program may request support for more than one sonar
  using setMaxNumSonarBoards(); if multi-sonar support is enabled in this way, then these arguments must have the sonar index
  number appended. For example, "-sonarPort" for sonar 1 would instead by "-sonarPort1", and for sonar 2 it would be
  "-sonarPort2".
  <dl>
    <dt>-sonarPort <i>port</i></dt>
    <dt>-sp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a sonar. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
      </dd>
    <dt>-doNotConnectSonar</dt>
    <dt>-dncs</dt>
  </dl>
 **/
MVREXPORT bool MvrSonarConnector::parseArgs (MvrArgumentParser *parser)
{

	if (myParsedArgs)
		return true;
	myParsedArgs = true;
	bool typeReallySet;
	const char *type;
	char buf[1024];
	int i;
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData;
	bool wasReallySetOnlyTrue = parser->getWasReallySetOnlyTrue();
	parser->setWasReallySetOnlyTrue (true);

	for (i = 1; i <= Mvria::getMaxNumSonarBoards(); i++) {
		if (i == 1)
			buf[0] = '\0';
		else
			sprintf (buf, "%d", i);
		typeReallySet = false;
		// see if the sonar is being added from the command line
		if (!parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		    "-sonarType%s", buf) ||
		    !parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		        "-st%s", buf)) {
			MvrLog::log (MvrLog::Normal,
			            "MvrSonarConnector: Bad sonar type given for sonar number %d",
			            i);
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}
		// if we didn't have an argument then just return
		if (!typeReallySet)
			continue;
		if ( (it = mySonars.find (i)) != mySonars.end()) {
			MvrLog::log (MvrLog::Normal, "MvrSonarConnector: A sonar already exists for sonar number %d, replacing it with a new one of type %s",
			            i, type);
			sonarData = (*it).second;
			delete sonarData;
			mySonars.erase (i);
		}
		if (typeReallySet && type != NULL) {
			MvrSonarMTX *sonar = NULL;
			if ( (sonar = Mvria::sonarCreate (type, i, "MvrSonarConnector: ")) != NULL) {
				MvrLog::log (myInfoLogLevel,
				            "MvrSonarConnector: Created %s as sonar %d from arguments",
				            sonar->getName(), i);
				mySonars[i] = new SonarData (i, sonar);
				sonar->setInfoLogLevel (myInfoLogLevel);
			} else {
				MvrLog::log (MvrLog::Normal,
				            "Unknown sonar type %s for sonar %d, choices are %s",
				            type, i, Mvria::sonarGetTypes());
				parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
				return false;
			}
		}
	}

  if (parser->checkArgument("-sonarLogPacketsReceived") || 
      parser->checkArgument("-slpr")) 
    mySonarLogPacketsReceived = true;
  
  if (parser->checkArgument("-sonarLogPacketsSent") || 
      parser->checkArgument("-slps"))      
    mySonarLogPacketsSent = true;


	// go through the robot param list and add the sonars defined
	// in the parameter file.
	const MvrRobotParams *params = NULL;
	if (myRobot != NULL) {
		params = myRobot->getRobotParams();
		if (params != NULL) {


			for (i = 1; i <= Mvria::getMaxNumSonarBoards(); i++) {
				// if we already have a sonar for this then don't add one from
				// the param file, since it was added either explicitly by a
				// program or from the command line
				if (mySonars.find (i) != mySonars.end())
					continue;
				type = params->getSonarMTXBoardType (i);
				// if we don't have a sonar type for that number continue
				if (type == NULL || type[0] == '\0')
					continue;

				int baud = params->getSonarMTXBoardBaud(i);
				if (baud == 0)
					continue;

				MvrSonarMTX *sonar = NULL;
				if ( (sonar =
				        Mvria::sonarCreate (type, i, "MvrSonarConnector: ")) != NULL) {
					MvrLog::log (myInfoLogLevel,
					            "MvrSonarConnector::parseArgs() Created %s as sonar %d from parameter file",
					            sonar->getName(), i);
					mySonars[i] = new SonarData (i, sonar);
					sonar->setInfoLogLevel (myInfoLogLevel);
				} else {
					MvrLog::log (MvrLog::Normal,
					            "MvrSonarConnector::parseArgs() Unknown sonar type %s for sonar %d from the .p file, choices are %s",
					            type, i, Mvria::sonarGetTypes());
					parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
					return false;
				}
			}
		} else {
			MvrLog::log (MvrLog::Normal, "MvrSonarConnector::parseArgs() Have robot, but robot has NULL params, so cannot configure its sonar");
		}
	}
	// now go through and parse the args for any sonar that we have


	for (it = mySonars.begin(); it != mySonars.end(); it++) {
		sonarData = (*it).second;
		if (!parseSonarArgs (parser, sonarData)) {
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}
	}
	parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
	return true;

} // end parseArgs

MVREXPORT bool MvrSonarConnector::parseSonarArgs (MvrArgumentParser *parser,
    SonarData *sonarData)
{
	char buf[512];
	if (sonarData == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrSonarConnector::parseSonarArgs() Was given NULL sonar");
		return false;
	}
	if (sonarData->mySonar == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "MvrSonarConnector::parseSonarArgs() There is no sonar for sonar number %d but there should be",
		            sonarData->myNumber);
		return false;
	}
	MvrSonarMTX *sonar = sonarData->mySonar;
	if (sonarData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", sonarData->myNumber);

#if 0
	// see if we want to connect to the sonar automatically
	if (parser->checkArgumentVar ("-connectSonar%s", buf) ||
	    parser->checkArgumentVar ("-cb%s", buf)) {
		sonarData->myConnect = true;
		sonarData->myConnectReallySet = true;
	}
#endif

	// see if we do not want to connect to the sonar automatically
	if (parser->checkArgumentVar ("-doNotConnectSonar%s", buf) ||
	    parser->checkArgumentVar ("-dncs%s", buf)) {
		sonarData->myConnect = false;
		sonarData->myConnectReallySet = true;
	}
	if (!parser->checkParameterArgumentStringVar (NULL, &sonarData->myPort,
	    "-sonarPort%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myPort,
	        "-sp%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myPortType,
	        "-sonarPortType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myPortType,
	        "-spt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myType,
	        "-sonarType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myType,
	        "-st%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myBaud,
	        "-sonarBaud%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myBaud,
	        "-sb%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myAutoConn,
	        "-sonarAutoConn%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &sonarData->myAutoConn,
	        "-sac%s", buf))
		{
		return false;
	}
	// PS - command line needs to set this to true
#if 0
	if (strcasecmp(sonarData->myAutoConn, "true") == 0) {
		sonarData->myConnect = true;
		sonarData->myConnectReallySet = true;
	}
#endif
	return internalConfigureSonar (sonarData);

} // end parseSonarArgs


bool MvrSonarConnector::internalConfigureSonar (
  SonarData *sonarData)
{
	if(sonarData->myConnectReallySet && ! sonarData->myConnect)
	{
		MvrLog::log(MvrLog::Terse, "MvrSonarConnector: Warning: connection to sonar %d explicitly disabled by opion", sonarData->myNumber);
		return true;
	}
	MvrSonarMTX *sonar = sonarData->mySonar;
	if (sonar == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrSonarConnector::internalConfigureSonar() No sonar for number %d",
		            sonarData->myNumber);
		return false;
	}
	// the rest handles all the connection stuff
	const MvrRobotParams *params;
	char portBuf[1024];
	if (sonarData->mySonar == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrSonarConnector::internalConfigureSonar() There is no sonar, cannot connect");
		return false;
	}
	sprintf (portBuf, "%s", sonarData->mySonar->getDefaultTcpPort());
	if (myRobotConnector == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrSonarConnector::internalConfigureSonar() No MvrRobotConnector is passed in so simulators and remote hosts will not work correctly");
	}
	if ( (sonarData->myPort == NULL || strlen (sonarData->myPort) == 0) &&
	     (sonarData->myPortType != NULL && strlen (sonarData->myPortType) > 0)) {
		MvrLog::log (MvrLog::Normal, "MvrSonarConnector::internalConfigureSonar() There is a sonar port type given ('%s') for sonar %d (%s), but no sonar port given, cannot configure sonar",
		            sonarData->myPortType, sonarData->myNumber, sonar->getName());
		return false;
	}

	MvrLog::log (MvrLog::Verbose, "MvrSonarConnector::internalConfigureSonar() command line sonar #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							sonarData->myNumber, 
							sonarData->myType,
							sonarData->myPort,
							sonarData->myPortType,
							sonarData->myBaud,
							sonarData->myAutoConn);


	if ( (sonarData->myPort != NULL && strlen (sonarData->myPort) > 0) &&
	     (sonarData->myPortType != NULL && strlen (sonarData->myPortType) > 0)) {
		MvrLog::log (MvrLog::Verbose, "MvrSonarConnector::internalConfigureSonar() Connection type and port given for sonar %d (%s), so overriding everything and using that information",
		            sonarData->myNumber, sonar->getName());
		
		if ( (sonarData->myConn = Mvria::deviceConnectionCreate (
		                              sonarData->myPortType, sonarData->myPort, portBuf,
		                              "MvrSonarConnector:")) == NULL) {
			return false;
		}
		sonar->setDeviceConnection (sonarData->myConn);
		return true;
	}

	if ( (sonarData->myPort != NULL && strlen (sonarData->myPort) > 0) &&
	     (sonarData->myPortType == NULL || strlen (sonarData->myPortType) == 0)) {
		if (myRobot != NULL && (params = myRobot->getRobotParams()) != NULL) {
			if (params->getSonarMTXBoardPortType (sonarData->myNumber) != NULL &&
			    params->getSonarMTXBoardPortType (sonarData->myNumber) [0] != '\0') {
				MvrLog::log (MvrLog::Normal, "MvrSonarConnector::internalConfigureSonar() There is a port given, but no port type given so using the robot parameters port type");
				if ( (sonarData->myConn = Mvria::deviceConnectionCreate (
				                              params->getSonarMTXBoardPortType (sonarData->myNumber),
				                              sonarData->myPort, portBuf,
				                              "MvrSonarConnector: ")) == NULL) {
					return false;
				}
			} else if (sonar->getDefaultPortType() != NULL &&
			           sonar->getDefaultPortType() [0] != '\0') {
				MvrLog::log (MvrLog::Normal, "MvrSonarConnector::internalConfigureSonar() There is a port given for sonar %d (%s), but no port type given and no robot parameters port type so using the sonar's default port type", sonarData->myNumber, sonar->getName());
				if ( (sonarData->myConn = Mvria::deviceConnectionCreate (
				                              sonar->getDefaultPortType(),
				                              sonarData->myPort, portBuf,
				                              "MvrSonarConnector: ")) == NULL) {
					return false;
				}
			} else {
				MvrLog::log (MvrLog::Normal, "MvrSonarConnector::internalConfigureSonar() There is a port given for sonar %d (%s), but no port type given, no robot parameters port type, and no sonar default port type, so using serial",
				            sonarData->myNumber, sonar->getName());
				if ( (sonarData->myConn = Mvria::deviceConnectionCreate (
				                              "serial",
				                              sonarData->myPort, portBuf,
				                              "MvrSonarConnector: ")) == NULL) {
					return false;
				}
			}
			sonar->setDeviceConnection (sonarData->myConn);
			return true;
		} else {
			MvrLog::log (MvrLog::Normal, "MvrSonarConnector::internalConfigureSonar() There is a sonar port given ('%s') for sonar %d (%s), but no sonar port type given and there are no robot params to find the information in, so assuming serial",
			            sonarData->myPort, sonarData->myNumber, sonar->getName());
			if ( (sonarData->myConn = Mvria::deviceConnectionCreate (
			                              sonarData->myPortType, sonarData->myPort, portBuf,
			                              "MvrSonarConnector: ")) == NULL) {
				return false;
			}
			sonar->setDeviceConnection (sonarData->myConn);
			return true;
		}
	}


	// if we get down here there was no information provided by the command line or in a sonar connector, so see if we have params... if not then fail, if so then use those
	if (myRobot == NULL || (params = myRobot->getRobotParams()) == NULL) {
		MvrLog::log (MvrLog::Normal, "MvrSonarConnector::internalConfigureSonar() No robot params are available, and no command line information given on how to connect to the sonar %d (%s), so cannot connect", sonarData->myNumber, sonar->getName());
		return false;
	}

  int i = sonarData->myNumber;
	MvrLog::log (MvrLog::Verbose, "MvrSonarConnector::internalConfigureSonar() .p sonar #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							i,
							params->getSonarMTXBoardType (i),
							params->getSonarMTXBoardPort (i),
							params->getSonarMTXBoardPortType (i),
							params->getSonarMTXBoardBaud(i),
							params->getSonarMTXBoardAutoConn (i));
	
  sonarData->myType = params->getSonarMTXBoardType(i);
  sonarData->myPort = params->getSonarMTXBoardPort(i);
  sonarData->myPortType = params->getSonarMTXBoardPortType(i);
  sonarData->setBaud(params->getSonarMTXBoardBaud(i));
  sonarData->setAutoConn(params->getSonarMTXBoardAutoConn(i));
  if(params->getSonarMTXBoardAutoConn(i))
  {
    sonarData->myConnect = true;
    sonarData->myConnectReallySet = true;
  }

	MvrLog::log (MvrLog::Verbose, "MvrSonarConnector::internalConfigureSonar(): Using robot params for connecting to sonar %d (%s) (port=%s, portType=%s)", sonarData->myNumber, sonar->getName(), sonarData->myPort, sonarData->myPortType);

  sonarData->myConn = Mvria::deviceConnectionCreate(sonarData->myPortType, sonarData->myPort, portBuf, "MvrSonarConnector: ");

	if (sonarData->myConn == NULL)
  {
    MvrLog::log(MvrLog::Terse, "MvrSonarConnector::internalConfigureSonar(): Error creating device connection.");
		return false;
	}

	sonar->setDeviceConnection (sonarData->myConn);

	return true;
}
MVREXPORT void MvrSonarConnector::logOptions (void) const
{
	MvrLog::log (MvrLog::Terse, "Options for MvrSonarConnector:");
  MvrLog::log(MvrLog::Terse, "-sonarLogPacketsReceived");
  MvrLog::log(MvrLog::Terse, "-slpr");
  MvrLog::log(MvrLog::Terse, "-sonarLogPacketsSent");
  MvrLog::log(MvrLog::Terse, "-slps");
	MvrLog::log (MvrLog::Terse, "\nOptions shown are for currently set up sonars.  Activate sonars with -sonarType<N> option");
	MvrLog::log (MvrLog::Terse, "to see options for that sonar (e.g. \"-help -sonarType1 sonarMTX\").");
	MvrLog::log (MvrLog::Terse, "Valid sonar types are: %s", Mvria::sonarGetTypes());
	MvrLog::log (MvrLog::Terse, "\nSee docs for details.");
	std::map<int, SonarData *>::const_iterator it;
	SonarData *sonarData;
	for (it = mySonars.begin(); it != mySonars.end(); it++) {
		sonarData = (*it).second;
		logSonarOptions (sonarData);
	}
}
MVREXPORT void MvrSonarConnector::logSonarOptions (
  SonarData *sonarData, bool header, bool metaOpts) const
{
	char buf[512];
	if (sonarData == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "Tried to log sonar options with NULL sonar data");
		return;
	}
	if (sonarData->mySonar == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "MvrSonarConnector: There is no sonar for sonar number %d but there should be",
		            sonarData->myNumber);
		return;
	}
	MvrSonarMTX *sonar = sonarData->mySonar;
	if (sonarData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", sonarData->myNumber);
	if (header) {
		MvrLog::log (MvrLog::Terse, "");
		MvrLog::log (MvrLog::Terse, "Sonar%s: (\"%s\")", buf, sonar->getName());
	}
	if (metaOpts) {
		MvrLog::log (MvrLog::Terse, "-sonarType%s <%s>", buf, Mvria::sonarGetTypes());
		MvrLog::log (MvrLog::Terse, "-st%s <%s>", buf, Mvria::sonarGetTypes());
		MvrLog::log (MvrLog::Terse, "-doNotConnectSonar%s", buf);
		MvrLog::log (MvrLog::Terse, "-dncs%s", buf);
	}
	MvrLog::log (MvrLog::Terse, "-sonarPort%s <sonarPort>", buf);
	MvrLog::log (MvrLog::Terse, "-sp%s <sonarPort>", buf);
	MvrLog::log (MvrLog::Terse, "-sonarPortType%s <%s>", buf, Mvria::deviceConnectionGetTypes());
	MvrLog::log (MvrLog::Terse, "-spt%s <%s>", buf, Mvria::deviceConnectionGetTypes());
	MvrLog::log (MvrLog::Terse, "-remoteSonarTcpPort%s <remoteSonarTcpPort>", buf);
	MvrLog::log (MvrLog::Terse, "-rstp%s <remoteSonarTcpPort>", buf);
}
/**
   Normally adding sonars is done from the .p file, you can use this
   if you want to add them explicitly in a program (which will
   override the .p file, and may cause some problems).
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrSonarConnector::connectSonars.()
   @internal
**/
MVREXPORT bool MvrSonarConnector::addSonar (
  MvrSonarMTX *sonar, int sonarNumber)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	if (sonarData != NULL) {
		if (sonarData->mySonar != NULL)
			MvrLog::log (MvrLog::Terse,
			            "MvrSonarConnector::addSonar: Already have sonar for number #%d of type %s but a replacement sonar of type %s was passed in",
			            sonarNumber, sonarData->mySonar->getName(), sonar->getName());
		else
			MvrLog::log (MvrLog::Terse,
			            "MvrSonarConnector::addSonar: Already have sonar for number #%d but a replacement sonar of type %s was passed in",
			            sonarNumber, sonar->getName());
		delete sonarData;
		mySonars.erase (sonarNumber);
	}
	mySonars[sonarNumber] = new SonarData (sonarNumber, sonar);
	return true;
}
MVREXPORT MvrSonarMTX *MvrSonarConnector::getSonar (int sonarNumber)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	// if we have no sonar, we can't get it so just return
	if (sonarData == NULL)
		return NULL;
	// otherwise, return the sonar
	return sonarData->mySonar;
}
MVREXPORT bool MvrSonarConnector::replaceSonar (
  MvrSonarMTX *sonar, int sonarNumber)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	// if we have no sonar, we can't replace it so just return
	if (sonarData == NULL)
		return false;
	if (sonarData->mySonar != NULL)
		MvrLog::log (myInfoLogLevel,
		            "MvrSonarConnector::replaceSonar: Already have sonar for number #%d of type %s but a replacement sonar of type %s was passed in",
		            sonarNumber, sonarData->mySonar->getName(), sonar->getName());
	else
		MvrLog::log (MvrLog::Normal,
		            "MvrSonarConnector::replaceSonar: Replacing a non existant sonar number #%d with a sonar of type %s passed in",
		            sonarNumber, sonar->getName());
	sonarData->mySonar = sonar;
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrSonarConnector::connectSonars().
   @internal
**/
MVREXPORT bool MvrSonarConnector::setupSonar (MvrSonarMTX *sonar,
    int sonarNumber)
{
	if (myRobot == NULL && myRobotConnector != NULL)
		myRobot = myRobotConnector->getRobot();
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	//const MvrRobotParams *params;
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	if (sonarData == NULL && sonar == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrSonarConnector::setupSonar: Do not have sonar #%d", sonarNumber) ;
		return false;
	}
	if (sonarData != NULL && sonar != NULL &&
	    sonarData->mySonar != sonar) {
		if (sonarData->mySonar != NULL)
			MvrLog::log (MvrLog::Terse, "MvrSonarConnector::setupSonar: Already have sonar for number #%d (%s) but a replacement sonar (%s) was passed in, this will replace all of the command line arguments for that sonar",
			            sonarNumber, sonarData->mySonar->getName(), sonar->getName());
		else
			MvrLog::log (MvrLog::Terse, "MvrSonarConnector::setupSonar: Already have sonar for number #%d but a replacement sonar (%s) was passed in, this will replace all of the command line arguments for that sonar",
			            sonarNumber, sonar->getName());
		delete sonarData;
		mySonars.erase (sonarNumber);
		mySonars[sonarNumber] = new SonarData (sonarNumber, sonar);
	}
	if (sonarData == NULL && sonar != NULL) {
		sonarData = new SonarData (sonarNumber, sonar);
		mySonars[sonarNumber] = sonarData;
		if (myAutoParseArgs && !parseSonarArgs (myParser, sonarData)) {
			MvrLog::log (MvrLog::Terse, "MvrSonarConnector: Error Auto parsing args for sonar %s (num %d)", sonarData->mySonar->getName(), sonarNumber);
			return false;
		}
	}
	// see if there is no sonar (ie if it was a sick done in the old
	// style), or if the sonar passed in doesn't match the one this
	// class created (I don't know how it'd happen, but...)... and then
	// configure it
	if ( (sonarData->mySonar == NULL || sonarData->mySonar != sonar)) {
		if (!internalConfigureSonar (sonarData))
			return false;
	}
	// setupSonar automatically adds this to the robot, since the
	// connectsonar stuff is the newer more supported way and is more
	// configurable.. it only adds it as a sonar since the legacy code
	// won't add it that way, but will add it as a range device
	if (myRobot != NULL) {
		myRobot->addSonar (sonar, sonarNumber);
		//myRobot->addRangeDevice(sonar);
	} else {
		MvrLog::log (MvrLog::Normal, "MvrSonarConnector::setupSonar: No robot, so sonar cannot be added to robot");
	}
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrSonarConnector::connectSonars().
   @internal
**/
MVREXPORT bool MvrSonarConnector::connectSonar (MvrSonarMTX *sonar,
    int sonarNumber,
    bool forceConnection)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	sonar->lockDevice();
	// set up the sonar regardless
	if (!setupSonar (sonar, sonarNumber)) {
		sonar->unlockDevice();  
		return false;
	}
	sonar->unlockDevice();
	if ( (it = mySonars.find (sonarNumber)) != mySonars.end())
		sonarData = (*it).second;
	if (sonarData == NULL) {
		MvrLog::log (MvrLog::Normal, "MvrSonarConnector::connectSonar: Some horrendous error in connectSonar with sonar number %d", sonarNumber);
		return false;
	}
	// see if we want to connect
	if (!forceConnection && !sonarData->myConnect)
		return true;
	else
		return sonar->blockingConnect(mySonarLogPacketsSent, mySonarLogPacketsReceived);
}


MVREXPORT bool MvrSonarConnector::connectSonars (
  bool continueOnFailedConnect, bool addConnectedSonarsToRobot,
  bool addAllSonarsToRobot, bool turnOnSonars,
  bool powerCycleSonarOnFailedConnect)
{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	MvrLog::log (myInfoLogLevel,
	            "MvrSonarConnector::connectSonars() Connecting sonars... myAutoParseArgs=%d myParsedArgs=%d addAllSonarsToRobot=%d", myAutoParseArgs, myParsedArgs, addAllSonarsToRobot);

	if (myAutoParseArgs && !myParsedArgs) {
		MvrLog::log (myInfoLogLevel,
		            "MvrSonarConnector::connectSonars() Auto parsing args for sonars");
		if (!parseArgs()) {
			return false;
		}
	}

	if (addAllSonarsToRobot) {
		MvrLog::log (myInfoLogLevel,
		            "MvrSonarConnector::connectSonars() addAllSonarsToRobot");
		if (myRobot != NULL) {
			for (it = mySonars.begin(); it != mySonars.end(); it++) {
				sonarData = (*it).second;
				myRobot->addSonar (sonarData->mySonar, sonarData->myNumber);
				MvrLog::log (MvrLog::Verbose,
				            "MvrSonarConnector::connectSonars: Added %s to robot as sonar %d",
				            sonarData->mySonar->getName(), sonarData->myNumber);
			}
		} else {
			MvrLog::log (MvrLog::Normal, "MvrSonarConnector::connectSonars: Error: Supposed to add all sonars to robot, but there is no robot");
			return false;
		}
	}

  MvrLog::log(myInfoLogLevel, "MvrSonarConnector::connectSonars(), finally connecting to each sonar...");
	for (it = mySonars.begin(); it != mySonars.end(); it++) {
		sonarData = (*it).second;
		if ( (sonarData == NULL) || (myRobot == NULL))
			continue;
//	if ( (sonarData->myPort == NULL || strlen (sonarData->myPort) == 0) &&
//	     (sonarData->myPortType != NULL && strlen (sonarData->myPortType) > 0)) {

		MvrLog::log (myInfoLogLevel, "MvrSonarConnector::connectSonars() sonar #%d type= %s port=%s portType=%s baud=%s autoconnect=%s, connect=%d",
		            sonarData->myNumber,
		            sonarData->myType,
		            sonarData->myPort,
		            sonarData->myPortType,
		            sonarData->myBaud,
		            sonarData->myAutoConn,
                sonarData->myConnect && sonarData->myConnectReallySet
    );
//}

		bool connected = false;

		if (sonarData->myConnectReallySet && sonarData->myConnect) {
      if (!turnOnPower(sonarData))
        MvrLog::log(MvrLog::Normal, "MvrSonarConnector: Warning: unable to turn on sonar power. Continuing anyway...");
			MvrLog::log (myInfoLogLevel,
			            "MvrSonarConnector::connectSonars() Connecting %s",
			            sonarData->mySonar->getName());
			sonarData->mySonar->setRobot (myRobot);
			// to turn on packet tracing - uncomment
			//connected = sonarData->mySonar->blockingConnect(true, true);
			connected = sonarData->mySonar->blockingConnect (mySonarLogPacketsSent, mySonarLogPacketsReceived);
		

		if (connected) {
			if (!addAllSonarsToRobot && addConnectedSonarsToRobot) {
				if (myRobot != NULL) {
					myRobot->addSonar (sonarData->mySonar, sonarData->myNumber);
					//myRobot->addRangeDevice(sonarData->mySonar);
					MvrLog::log (myInfoLogLevel,
					            //MvrLog::log (MvrLog::Verbose,
					            "MvrSonarConnector::connectSonars() Added %s to robot",
					            sonarData->mySonar->getName());
				} else {
					MvrLog::log (MvrLog::Normal,
					            "MvrSonarConnector::connectSonars() Could not add %s to robot, since there is no robot",
					            sonarData->mySonar->getName());
				}
			} else if (addAllSonarsToRobot && myRobot != NULL) {
				MvrLog::log (MvrLog::Normal,
//					MvrLog::log (MvrLog::Verbose,
				            "MvrSonarConnector::connectSonars() %s already added to robot)",
				            sonarData->mySonar->getName());
			} else if (myRobot != NULL) {
				MvrLog::log (MvrLog::Normal,
//					MvrLog::log (MvrLog::Verbose,
				            "MvrSonarConnector::connectSonars() Did not add %s to robot",
				            sonarData->mySonar->getName());
			}
		} else {
			if (!continueOnFailedConnect) {
				MvrLog::log (MvrLog::Normal,
				            "MvrSonarConnector::connectSonars() Could not connect %s, stopping",
				            sonarData->mySonar->getName());
				return false;
			} else
				MvrLog::log (MvrLog::Normal,
				            "MvrSonarConnector::connectSonars() Could not connect %s, continuing with remainder of sonars",
				            sonarData->mySonar->getName());
		}
	}
	}

	MvrLog::log (myInfoLogLevel,
            "MvrSonarConnector() Done connecting sonars");
	return true;
}

MVREXPORT bool MvrSonarConnector::turnOnPower(SonarData *sonarData)

{
  if (myTurnOnPowerOutputCB != NULL)
  {
    if (myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
		sonarData->myNumber) == NULL ||
					myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
		sonarData->myNumber)[0] == '\0')
    {
      MvrLog::log(MvrLog::Normal, 
										"MvrSonarConnector::connectSonars: Sonar %d has no power output set so can't be turned on (things may still work).",
										sonarData->myNumber);
			return false;
    }
    else
    {
      if (myTurnOnPowerOutputCB->invokeR(
		  myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
			  sonarData->myNumber)))
      {
			MvrLog::log(myInfoLogLevel, 
					"MvrSonarConnector::connectSonars: Turned on power output %s for sonar %d",

			myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
								sonarData->myNumber),
								sonarData->myNumber);
			return true;
      }
      else
      {
			MvrLog::log(MvrLog::Normal, 
					"MvrSonarConnector::connectSonars: Could not turn on power output %s for sonar %d (things may still work).",
			myRobot->getRobotParams()->getSonarMTXBoardPowerOutput(
									sonarData->myNumber),
									sonarData->myNumber);
			return false;
      }
    }
  }
  
  return true;
}

MVREXPORT bool MvrSonarConnector::connectReplaySonars(
  bool continueOnFailedConnect, bool addConnectedSonarsToRobot,
  bool addAllSonarsToRobot, bool turnOnSonars,
  bool powerCycleSonarOnFailedConnect)

{
	std::map<int, SonarData *>::iterator it;
	SonarData *sonarData = NULL;
	MvrLog::log (myInfoLogLevel,
	            "MvrSonarConnector::connectReplaySonars() Connecting sonars %d %d", myAutoParseArgs, myParsedArgs);
	if (myAutoParseArgs && !myParsedArgs) {
		MvrLog::log (MvrLog::Verbose,
		            "MvrSonarConnector::connectReplaySonars() Auto parsing args for sonars");
		if (!parseArgs()) {
			return false;
		}
	}

	if (addAllSonarsToRobot) {
			MvrLog::log (MvrLog::Verbose,
		            "MvrSonarConnector::connectReplaySonars() addAllSonarsToRobot");

		if (myRobot != NULL) {

			for (it = mySonars.begin(); it != mySonars.end(); it++) {
				sonarData = (*it).second;
				myRobot->addSonar (sonarData->mySonar, sonarData->myNumber);
				MvrLog::log (MvrLog::Verbose,
				            "MvrSonarConnector::connectReplaySonars: Added %s to robot as sonar %d",
				            sonarData->mySonar->getName(), sonarData->myNumber);
			}
		} else {
			MvrLog::log (MvrLog::Normal, "MvrSonarConnector::connectReplaySonars: Supposed to add all sonars to robot, but there is no robot");
			return false;
		}
	}
	for (it = mySonars.begin(); it != mySonars.end(); it++) {

		sonarData = (*it).second;

		if ((sonarData == NULL) || (myRobot == NULL))
			continue;

		MvrLog::log (MvrLog::Verbose, "MvrSonarConnector::connectReplaySonars() sonar #%d type= %s port=%s portType=%s baud=%s autoconnect=%s ",
							sonarData->myNumber, 
							sonarData->myType,
							sonarData->myPort,
							sonarData->myPortType,
							sonarData->myBaud,
							sonarData->myAutoConn);

		sonarData->mySonar->setRobot (myRobot);

		if (!sonarData->mySonar->fakeConnect())
		  return false;
	}
  return true;
}

MVREXPORT bool MvrSonarConnector::disconnectSonars()
{
  
	for(std::map<int, SonarData *>::iterator it = mySonars.begin();
      it != mySonars.end();
      ++it)
  {
	  SonarData *sonarData = (*it).second;
    if(sonarData)
    {
      if(myRobot)
        myRobot->remSonar(sonarData->mySonar);
      if(sonarData->mySonar)
      {
        sonarData->mySonar->disconnect();
      }
    }
  }
  return true;
}

