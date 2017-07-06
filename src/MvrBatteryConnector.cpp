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
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrBatteryConnector.h"
#include "MvrRobot.h"
#include "MvrBatteryMTX.h"
#include "mvriaInternal.h"
#include "MvrCommands.h"
#include "MvrRobotConfigPacketReader.h"
/** @warning do not delete @a parser during the lifetime of this
 MvrBatteryConnector, which may need to access its contents later.
 @param parser the parser with the arguments to parse
 @param robot the robot these batteries are attached to (or NULL for none)
 @param robotConnector the connector used for connecting to the robot
 (so we can see if it was a sim or not)
 @param autoParseArgs if this class should autoparse the args if they
 aren't parsed explicitly
 @param infoLogLevel The log level for information about creating
 batteries and such, this is also passed to all the batteries created as
 their infoLogLevel too
 */
MVREXPORT MvrBatteryConnector::MvrBatteryConnector (
  MvrArgumentParser *parser, MvrRobot *robot,
  MvrRobotConnector *robotConnector, bool autoParseArgs,
  MvrLog::LogLevel infoLogLevel) :
	myParseArgsCB (this, &MvrBatteryConnector::parseArgs),
	myLogOptionsCB (this, &MvrBatteryConnector::logOptions)
{
	myParser = parser;
	myOwnParser = false;
	myRobot = robot;
	myRobotConnector = robotConnector;
	myAutoParseArgs = autoParseArgs;
	myParsedArgs = false;
  myBatteryLogPacketsReceived = false;
  myBatteryLogPacketsSent = false;
	myInfoLogLevel = infoLogLevel;
	myParseArgsCB.setName ("MvrBatteryConnector");
	Mvria::addParseArgsCB (&myParseArgsCB, 60);
	myLogOptionsCB.setName ("MvrBatteryConnector");
	Mvria::addLogOptionsCB (&myLogOptionsCB, 80);
}
MVREXPORT MvrBatteryConnector::~MvrBatteryConnector (void)
{
//  Mvria::remLogOptionsCB(&myLogOptionsCB);
//  Mvria::remParseArgsCB(&myParseArgsCB);
}
/**
 * Parse command line arguments using the MvrArgumentParser given in the MvrBatteryConnector constructor.
 *
 * See parseArgs(MvrArgumentParser*) for details about argument parsing.
 *
  @return true if the arguments were parsed successfully false if not
 **/
MVREXPORT bool MvrBatteryConnector::parseArgs (void)
{
	return parseArgs (myParser);
}
/**
 * Parse command line arguments held by the given MvrArgumentParser.
 *
  @return true if the arguments were parsed successfully false if not

  The following arguments are accepted for battery connections.  A program may request support for more than one battery
  using setMaxNumBatteries(); if multi-battery support is enabled in this way, then these arguments must have the battery index
  number appended. For example, "-batteryPort" for battery 1 would instead by "-batteryPort1", and for battery 2 it would be
  "-batteryPort2".
  <dl>
    <dt>-batteryPort <i>port</i></dt>
    <dt>-bp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a battery. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
    </dd>
    <dt>-doNotConnectBattery</dt>
    <dt>-dncb</dt>
  </dl>
 **/
MVREXPORT bool MvrBatteryConnector::parseArgs (MvrArgumentParser *parser)
{

	if (myParsedArgs)
		return true;
	myParsedArgs = true;
	bool typeReallySet;
	const char *type;
	char buf[1024];
	int i;
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData;
	bool wasReallySetOnlyTrue = parser->getWasReallySetOnlyTrue();
	parser->setWasReallySetOnlyTrue (true);

	for (i = 1; i <= Mvria::getMaxNumBatteries(); i++) {
		if (i == 1)
			buf[0] = '\0';
		else
			sprintf (buf, "%d", i);
		typeReallySet = false;
		// see if the battery is being added from the command line
		if (!parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		    "-batteryType%s", buf) ||
		    !parser->checkParameterArgumentStringVar (&typeReallySet, &type,
		        "-bt%s", buf)) {
			MvrLog::log (MvrLog::Normal,
			            "MvrBatteryConnector: Bad battery type given for battery number %d",
			            i);
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);

			return false;
		}
		// if we didn't have an argument then just return
		if (!typeReallySet)
			continue;
		if ( (it = myBatteries.find (i)) != myBatteries.end()) {
			MvrLog::log (MvrLog::Normal, "MvrBatteryConnector: A battery already exists for battery number %d, replacing it with a new one of type %s",
			            i, type);
			batteryData = (*it).second;
			delete batteryData;
			myBatteries.erase (i);
		}
		if (typeReallySet && type != NULL) {
			MvrBatteryMTX *battery = NULL;
			if ( (battery = Mvria::batteryCreate (type, i, "MvrBatteryConnector: ")) != NULL) {
				MvrLog::log (myInfoLogLevel,
				            "MvrBatteryConnector: Created %s as battery %d from arguments",
				            battery->getName(), i);
				myBatteries[i] = new BatteryData (i, battery);
				battery->setInfoLogLevel (myInfoLogLevel);
			} else {
				MvrLog::log (MvrLog::Normal,
				            "Unknown battery type %s for battery %d, choices are %s",
				            type, i, Mvria::batteryGetTypes());
				parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
				return false;
			}
		}
	}

  if (parser->checkArgument("-batteryLogPacketsReceived") || 
      parser->checkArgument("-blpr")) 
    myBatteryLogPacketsReceived = true;
  
  if (parser->checkArgument("-batteryLogPacketsSent") || 
      parser->checkArgument("-blps"))      
    myBatteryLogPacketsSent = true;

	// go through the robot param list and add the batteries defined
	// in the parameter file.
	const MvrRobotParams *params = NULL;
	if (myRobot != NULL) {
		params = myRobot->getRobotParams();
		if (params != NULL) {


			for (i = 1; i <= Mvria::getMaxNumBatteries(); i++) {
				// if we already have a battery for this then don't add one from
				// the param file, since it was added either explicitly by a
				// program or from the command line
				if (myBatteries.find (i) != myBatteries.end())
					continue;
				type = params->getBatteryMTXBoardType (i);

				// if we don't have a battery type for that number continue
				if (type == NULL || type[0] == '\0')
					continue;

				int baud = params->getBatteryMTXBoardBaud(i);

				if (baud == 0)
					continue;

				MvrBatteryMTX *battery = NULL;
				if ( (battery =
				        Mvria::batteryCreate (type, i, "MvrBatteryConnector: ")) != NULL) {
					MvrLog::log (myInfoLogLevel,
					            "MvrBatteryConnector::parseArgs() Created %s as battery %d from parameter file",
					            battery->getName(), i);
					myBatteries[i] = new BatteryData (i, battery);
					battery->setInfoLogLevel (myInfoLogLevel);
				} else {
					MvrLog::log (MvrLog::Normal,
					            "MvrBatteryConnector::parseArgs() Unknown battery type %s for battery %d from the .p file, choices are %s",
					            type, i, Mvria::batteryGetTypes());
					parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
					return false;
				}
			}
		} else {
			MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::parseArgs() Have robot, but robot has NULL params, so cannot configure its battery");
		}
	}
	// now go through and parse the args for any battery that we have


	for (it = myBatteries.begin(); it != myBatteries.end(); it++) {
		batteryData = (*it).second;
		if (!parseBatteryArgs (parser, batteryData)) {
			parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
			return false;
		}
	}
	parser->setWasReallySetOnlyTrue (wasReallySetOnlyTrue);
	return true;

} // end parseArgs

MVREXPORT bool MvrBatteryConnector::parseBatteryArgs (MvrArgumentParser *parser,
    BatteryData *batteryData)
{
	char buf[512];
	if (batteryData == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrBatteryConnector::parseBatteryArgs() Was given NULL battery");
		return false;
	}
	if (batteryData->myBattery == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "MvrBatteryConnector::parseBatteryArgs() There is no battery for battery number %d but there should be",
		            batteryData->myNumber);
		return false;
	}
	MvrBatteryMTX *battery = batteryData->myBattery;
	if (batteryData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", batteryData->myNumber);

#if 0
	// see if we want to connect to the battery automatically
	if (parser->checkArgumentVar ("-connectBattery%s", buf) ||
	    parser->checkArgumentVar ("-cb%s", buf)) {
		batteryData->myConnect = true;
		batteryData->myConnectReallySet = true;
	}
#endif

	// see if we do not want to connect to the battery automatically
	if (parser->checkArgumentVar ("-doNotConnectBattery%s", buf) ||
	    parser->checkArgumentVar ("-dncb%s", buf)) {
		batteryData->myConnect = false;
		batteryData->myConnectReallySet = true;
	}
	if (!parser->checkParameterArgumentStringVar (NULL, &batteryData->myPort,
	    "-batteryPort%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myPort,
	        "-bp%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myPortType,
	        "-batteryPortType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myPortType,
	        "-bpt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myType,
	        "-batteryType%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myType,
	        "-bt%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myBaud,
	        "-batteryBaud%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myBaud,
	        "-bb%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myAutoConn,
	        "-batteryAutoConn%s", buf) ||
	    !parser->checkParameterArgumentStringVar (NULL, &batteryData->myAutoConn,
	        "-bac%s", buf))
		{
		return false;
	}
	// PS - command line needs to set this to true
#if 0
	if (strcasecmp(batteryData->myAutoConn, "true") == 0) {
		batteryData->myConnect = true;
		batteryData->myConnectReallySet = true;
	}
#endif
	return internalConfigureBattery (batteryData);

} // end parseBatteryArgs


bool MvrBatteryConnector::internalConfigureBattery (
  BatteryData *batteryData)
{
	if(batteryData->myConnectReallySet && ! batteryData->myConnect)
	{
		MvrLog::log(MvrLog::Terse, "MvrBatteryConnector::internalConfigure: Warning: connection to battery %d explicitly disabled by options", batteryData->myNumber);
		return true;
	}
	MvrBatteryMTX *battery = batteryData->myBattery;
	if (battery == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrBatteryConnector::internalConfigureBattery() No battery for number %d",
		            batteryData->myNumber);
		return false;
	}
	// the rest handles all the connection stuff
	const MvrRobotParams *params;
	if (batteryData->myBattery == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrBatteryConnector::internalConfigureBattery() There is no battery, cannot connect");
		return false;
	}
	char portBuf[8];
	strncpy(portBuf, batteryData->myBattery->getDefaultTcpPort(), 7); portBuf[8] = 0;
	if (myRobotConnector == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrBatteryConnector::internalConfigureBattery() No MvrRobotConnector is passed in so simulators and remote hosts will not work correctly");
	}
	if ( (batteryData->myPort == NULL || strlen (batteryData->myPort) == 0) &&
	     (batteryData->myPortType != NULL && strlen (batteryData->myPortType) > 0)) {
		MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::internalConfigureBattery() There is a battery port type given ('%s') for battery %d (%s), but no battery port given, cannot configure battery",
		            batteryData->myPortType, batteryData->myNumber, battery->getName());
		return false;
	}

	MvrLog::log (MvrLog::Verbose, "MvrBatteryConnector::internalConfigureBattery() command line battery #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							batteryData->myNumber, 
							batteryData->myType,
							batteryData->myPort,
							batteryData->myPortType,
							batteryData->myBaud,
							batteryData->myAutoConn,
							batteryData->myConnect);


	if ( (batteryData->myPort != NULL && strlen (batteryData->myPort) > 0) &&
	     (batteryData->myPortType != NULL && strlen (batteryData->myPortType) > 0)) {
		MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::internalConfigureBattery() Connection type and port given for battery %d (%s), so overriding everything and using that information",
		            batteryData->myNumber, battery->getName());
		
		if ( (batteryData->myConn = Mvria::deviceConnectionCreate (
		                              batteryData->myPortType, batteryData->myPort, portBuf,
		                              "MvrBatteryConnector:")) == NULL) {
			return false;
		}
		battery->setDeviceConnection (batteryData->myConn);
		return true;
	}
	if ( (batteryData->myPort != NULL && strlen (batteryData->myPort) > 0) &&
	     (batteryData->myPortType == NULL || strlen (batteryData->myPortType) == 0)) {
		if (myRobot != NULL && (params = myRobot->getRobotParams()) != NULL) {
			if (params->getBatteryMTXBoardPortType (batteryData->myNumber) != NULL &&
			    params->getBatteryMTXBoardPortType (batteryData->myNumber) [0] != '\0') {
				MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::internalConfigureBattery() There is a port given, but no port type given so using the robot parameters port type");
				if ( (batteryData->myConn = Mvria::deviceConnectionCreate (
				                              params->getBatteryMTXBoardPortType (batteryData->myNumber),
				                              batteryData->myPort, portBuf,
				                              "MvrBatteryConnector: ")) == NULL) {
					return false;
				}
			} else if (battery->getDefaultPortType() != NULL &&
			           battery->getDefaultPortType() [0] != '\0') {
				MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::internalConfigureBattery() There is a port given for battery %d (%s), but no port type given and no robot parameters port type so using the battery's default port type", batteryData->myNumber, battery->getName());
				if ( (batteryData->myConn = Mvria::deviceConnectionCreate (
				                              battery->getDefaultPortType(),
				                              batteryData->myPort, portBuf,
				                              "MvrBatteryConnector: ")) == NULL) {
					return false;
				}
			} else {
				MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::internalConfigureBattery() There is a port given for battery %d (%s), but no port type given, no robot parameters port type, and no battery default port type, so using serial",
				            batteryData->myNumber, battery->getName());
				if ( (batteryData->myConn = Mvria::deviceConnectionCreate (
				                              "serial",
				                              batteryData->myPort, portBuf,
				                              "MvrBatteryConnector: ")) == NULL) {
					return false;
				}
			}
			battery->setDeviceConnection (batteryData->myConn);
			return true;
		} else {
			MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::internalConfigureBattery() There is a battery port given ('%s') for battery %d (%s), but no battery port type given and there are no robot params to find the information in, so assuming serial",
			            batteryData->myPort, batteryData->myNumber, battery->getName());
			if ( (batteryData->myConn = Mvria::deviceConnectionCreate (
			                              batteryData->myPortType, batteryData->myPort, portBuf,
			                              "MvrBatteryConnector: ")) == NULL) {
				return false;
			}
			battery->setDeviceConnection (batteryData->myConn);
			return true;
		}
	}
	// if we get down here there was no information provided by the command line or in a battery connector, so see if we have params... if not then fail, if so then use those
	if (myRobot == NULL || (params = myRobot->getRobotParams()) == NULL) {
		MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::internalConfigureBattery() No robot params are available, and no command line information given on how to connect to the battery %d (%s), so cannot connect", batteryData->myNumber, battery->getName());
		return false;
	}

	MvrLog::log (MvrLog::Verbose, "MvrBatteryConnector::internalConfigureBattery() .p battery #%d type= %s port=%s portType=%s baud=%d autoconnect=%d ",
							batteryData->myNumber, 
							params->getBatteryMTXBoardType (batteryData->myNumber),
							params->getBatteryMTXBoardPort (batteryData->myNumber),
							params->getBatteryMTXBoardPortType (batteryData->myNumber),
							params->getBatteryMTXBoardBaud (batteryData->myNumber),
							params->getBatteryMTXBoardAutoConn (batteryData->myNumber));

	// see if auto connect is on
	if (params->getBatteryMTXBoardAutoConn (batteryData->myNumber)) {

		batteryData->myConnect = true;
		batteryData->myConnectReallySet = true;
	}

	MvrLog::log (MvrLog::Verbose, "MvrBatteryConnector: Using robot params for connecting to battery %d (%s)", batteryData->myNumber, battery->getName());

	if ( (batteryData->myConn = Mvria::deviceConnectionCreate (
	                              params->getBatteryMTXBoardPortType (batteryData->myNumber),
	                              params->getBatteryMTXBoardPort (batteryData->myNumber), portBuf,
	                              "MvrBatteryConnector: ")) == NULL) {
		return false;
	}
	battery->setDeviceConnection (batteryData->myConn);
	return true;
}
MVREXPORT void MvrBatteryConnector::logOptions (void) const
{
	MvrLog::log (MvrLog::Terse, "Options for MvrBatteryConnector:");
  MvrLog::log(MvrLog::Terse, "-batteryLogPacketsReceived");
  MvrLog::log(MvrLog::Terse, "-blpr");
  MvrLog::log(MvrLog::Terse, "-batteryLogPacketsSent");
  MvrLog::log(MvrLog::Terse, "-blps");
	MvrLog::log (MvrLog::Terse, "\nOptions shown are for currently set up batteries.  Activate batteries with -batteryType<N> option");
	MvrLog::log (MvrLog::Terse, "to see options for that battery (e.g. \"-help -batteryType1 batteryMTX\").");
	MvrLog::log (MvrLog::Terse, "Valid battery types are: %s", Mvria::batteryGetTypes());
	MvrLog::log (MvrLog::Terse, "\nSee docs for details.");
	std::map<int, BatteryData *>::const_iterator it;
	BatteryData *batteryData;
	for (it = myBatteries.begin(); it != myBatteries.end(); it++) {
		batteryData = (*it).second;
		logBatteryOptions (batteryData);
	}
}
MVREXPORT void MvrBatteryConnector::logBatteryOptions (
  BatteryData *batteryData, bool header, bool metaOpts) const
{
	char buf[512];
	if (batteryData == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "Tried to log battery options with NULL battery data");
		return;
	}
	if (batteryData->myBattery == NULL) {
		MvrLog::log (MvrLog::Normal,
		            "MvrBatteryConnector: There is no battery for battery number %d but there should be",
		            batteryData->myNumber);
		return;
	}
	MvrBatteryMTX *battery = batteryData->myBattery;
	if (batteryData->myNumber == 1)
		buf[0] = '\0';
	else
		sprintf (buf, "%d", batteryData->myNumber);
	if (header) {
		MvrLog::log (MvrLog::Terse, "");
		MvrLog::log (MvrLog::Terse, "Battery%s: (\"%s\")", buf, battery->getName());
	}
	if (metaOpts) {
		MvrLog::log (MvrLog::Terse, "-batteryType%s <%s>", buf, Mvria::batteryGetTypes());
		MvrLog::log (MvrLog::Terse, "-bt%s <%s>", buf, Mvria::batteryGetTypes());
		MvrLog::log (MvrLog::Terse, "-doNotConnectBattery%s", buf);
		MvrLog::log (MvrLog::Terse, "-dncb%s", buf);
	}
	MvrLog::log (MvrLog::Terse, "-batteryPort%s <batteryPort>", buf);
	MvrLog::log (MvrLog::Terse, "-bp%s <batteryPort>", buf);
	MvrLog::log (MvrLog::Terse, "-batteryPortType%s <%s>", buf, Mvria::deviceConnectionGetTypes());
	MvrLog::log (MvrLog::Terse, "-bpt%s <%s>", buf, Mvria::deviceConnectionGetTypes());
	MvrLog::log (MvrLog::Terse, "-remoteBatteryTcpPort%s <remoteBatteryTcpPort>", buf);
	MvrLog::log (MvrLog::Terse, "-rbtp%s <remoteBatteryTcpPort>", buf);
}
/**
   Normally adding batteries is done from the .p file, you can use this
   if you want to add them explicitly in a program (which will
   override the .p file, and may cause some problems).
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrBatteryConnector::connectBatteries.()
   @internal
**/
MVREXPORT bool MvrBatteryConnector::addBattery (
  MvrBatteryMTX *battery, int batteryNumber)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	if (batteryData != NULL) {
		if (batteryData->myBattery != NULL)
			MvrLog::log (MvrLog::Terse,
			            "MvrBatteryConnector::addBattery: Already have battery for number #%d of type %s but a replacement battery of type %s was passed in",
			            batteryNumber, batteryData->myBattery->getName(), battery->getName());
		else
			MvrLog::log (MvrLog::Terse,
			            "MvrBatteryConnector::addBattery: Already have battery for number #%d but a replacement battery of type %s was passed in",
			            batteryNumber, battery->getName());
		delete batteryData;
		myBatteries.erase (batteryNumber);
	}
	myBatteries[batteryNumber] = new BatteryData (batteryNumber, battery);
	return true;
}
MVREXPORT MvrBatteryMTX *MvrBatteryConnector::getBattery (int batteryNumber)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	// if we have no battery, we can't get it so just return
	if (batteryData == NULL)
		return NULL;
	// otherwise, return the battery
	return batteryData->myBattery;
}
MVREXPORT bool MvrBatteryConnector::replaceBattery (
  MvrBatteryMTX *battery, int batteryNumber)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	// if we have no battery, we can't replace it so just return
	if (batteryData == NULL)
		return false;
	if (batteryData->myBattery != NULL)
		MvrLog::log (myInfoLogLevel,
		            "MvrBatteryConnector::replaceBattery: Already have battery for number #%d of type %s but a replacement battery of type %s was passed in",
		            batteryNumber, batteryData->myBattery->getName(), battery->getName());
	else
		MvrLog::log (MvrLog::Normal,
		            "MvrBatteryConnector::replaceBattery: Replacing a non existant battery number #%d with a battery of type %s passed in",
		            batteryNumber, battery->getName());
	batteryData->myBattery = battery;
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrBatteryConnector::connectBatteries().
   @internal
**/
MVREXPORT bool MvrBatteryConnector::setupBattery (MvrBatteryMTX *battery,
    int batteryNumber)
{
	if (myRobot == NULL && myRobotConnector != NULL)
		myRobot = myRobotConnector->getRobot();
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	//const MvrRobotParams *params;
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	if (batteryData == NULL && battery == NULL) {
		MvrLog::log (MvrLog::Terse, "MvrBatteryConnector::setupBattery: Do not have battery #%d", batteryNumber) ;
		return false;
	}
	if (batteryData != NULL && battery != NULL &&
	    batteryData->myBattery != battery) {
		if (batteryData->myBattery != NULL)
			MvrLog::log (MvrLog::Terse, "MvrBatteryConnector::setupBattery: Already have battery for number #%d (%s) but a replacement battery (%s) was passed in, this will replace all of the command line arguments for that battery",
			            batteryNumber, batteryData->myBattery->getName(), battery->getName());
		else
			MvrLog::log (MvrLog::Terse, "MvrBatteryConnector::setupBattery: Already have battery for number #%d but a replacement battery (%s) was passed in, this will replace all of the command line arguments for that battery",
			            batteryNumber, battery->getName());
		delete batteryData;
		myBatteries.erase (batteryNumber);
		myBatteries[batteryNumber] = new BatteryData (batteryNumber, battery);
	}
	if (batteryData == NULL && battery != NULL) {
		batteryData = new BatteryData (batteryNumber, battery);
		myBatteries[batteryNumber] = batteryData;
		if (myAutoParseArgs && !parseBatteryArgs (myParser, batteryData)) {
			MvrLog::log (MvrLog::Verbose, "MvrBatteryConnector: Auto parsing args for battery %s (num %d)", batteryData->myBattery->getName(), batteryNumber);
			return false;
		}
	}
	// see if there is no battery (ie if it was a sick done in the old
	// style), or if the battery passed in doesn't match the one this
	// class created (I don't know how it'd happen, but...)... and then
	// configure it
	if ( (batteryData->myBattery == NULL || batteryData->myBattery != battery)) {
		if (!internalConfigureBattery (batteryData))
			return false;
	}
	// setupBattery automatically adds this to the robot, since the
	// connectbattery stuff is the newer more supported way and is more
	// configurable.. it only adds it as a battery since the legacy code
	// won't add it that way, but will add it as a range device
	if (myRobot != NULL) {
		myRobot->addBattery (battery, batteryNumber);
		//myRobot->addRangeDevice(battery);
	} else {
		MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::setupBattery: No robot, so battery cannot be added to robot");
	}
	return true;
}
/**
   This is mainly for backwards compatibility (ie used for
   MvrSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just MvrBatteryConnector::connectBatteries().
   @internal
**/
MVREXPORT bool MvrBatteryConnector::connectBattery (MvrBatteryMTX *battery,
    int batteryNumber,
    bool forceConnection)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	battery->lockDevice();
	// set up the battery regardless
	if (!setupBattery (battery, batteryNumber)) {
		battery->unlockDevice();
		return false;
	}
	battery->unlockDevice();
	if ( (it = myBatteries.find (batteryNumber)) != myBatteries.end())
		batteryData = (*it).second;
	if (batteryData == NULL) {
		MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::connectBattery: Some horrendous error in connectBattery with battery number %d", batteryNumber);
		return false;
	}
	// see if we want to connect
	if (!forceConnection && !batteryData->myConnect)
		return true;
	else
		return battery->blockingConnect(myBatteryLogPacketsSent, myBatteryLogPacketsReceived);
}

MVREXPORT bool MvrBatteryConnector::connectBatteries (
  bool continueOnFailedConnect, bool addConnectedBatteriesToRobot,
  bool addAllBatteriesToRobot, bool turnOnBatteries,
  bool powerCycleBatteryOnFailedConnect)
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	MvrLog::log (myInfoLogLevel,
	            "MvrBatteryConnector::connectBatteries() Connecting batteries");
	if (myAutoParseArgs && !myParsedArgs) {
		MvrLog::log (MvrLog::Verbose,
		            "MvrBatteryConnector::connectBatteries() Auto parsing args for batteries");
		if (!parseArgs()) {
			return false;
		}
	}
	if (addAllBatteriesToRobot) {

			MvrLog::log (MvrLog::Verbose,
		            "MvrBatteryConnector::connectBatteries() addAllBatteriesToRobot");

		if (myRobot != NULL) {

			for (it = myBatteries.begin(); it != myBatteries.end(); it++) {
				batteryData = (*it).second;
				myRobot->addBattery (batteryData->myBattery, batteryData->myNumber);
				MvrLog::log (MvrLog::Verbose,
				            "MvrBatteryConnector::connectBatteries: Added %s to robot as battery %d",
				            batteryData->myBattery->getName(), batteryData->myNumber);
			}
		} else {
			MvrLog::log (MvrLog::Normal, "MvrBatteryConnector::connectBatteries: Supposed to add all batteries to robot, but there is no robot");
			return false;
		}
	}
	for (it = myBatteries.begin(); it != myBatteries.end(); it++) {

		batteryData = (*it).second;

		if (batteryData == NULL)
			continue;

		if (batteryData->myConnectReallySet && batteryData->myConnect) {
			MvrLog::log (myInfoLogLevel,
			            "MvrBatteryConnector::connectBatteries: Connecting %s",
			            batteryData->myBattery->getName());
			batteryData->myBattery->setRobot (myRobot);
			bool connected = false;
			connected = batteryData->myBattery->blockingConnect(myBatteryLogPacketsSent, myBatteryLogPacketsReceived);
			if (connected) {
				if (!addAllBatteriesToRobot && addConnectedBatteriesToRobot) {
					if (myRobot != NULL) {
						myRobot->addBattery (batteryData->myBattery, batteryData->myNumber);
						//myRobot->addRangeDevice(batteryData->myBattery);
						MvrLog::log (MvrLog::Verbose,
						            "MvrBatteryConnector::connectBatteries: Added %s to robot",
						            batteryData->myBattery->getName());
					} else {
						MvrLog::log (MvrLog::Normal,
						            "MvrBatteryConnector::connectBatteries: Could not add %s to robot, since there is no robot",
						            batteryData->myBattery->getName());
					}
				} else if (addAllBatteriesToRobot && myRobot != NULL) {
					MvrLog::log (MvrLog::Verbose,
					            "MvrBatteryConnector::connectBatteries: %s already added to robot)",
					            batteryData->myBattery->getName());
				} else if (myRobot != NULL) {
					MvrLog::log (MvrLog::Verbose,
					            "MvrBatteryConnector::connectBatteries: Did not add %s to robot",
					            batteryData->myBattery->getName());
				}
			} else {
				if (!continueOnFailedConnect) {
					MvrLog::log (MvrLog::Normal,
					            "MvrBatteryConnector::connectBatteries: Could not connect %s, stopping",
					            batteryData->myBattery->getName());
					return false;
				} else
					MvrLog::log (MvrLog::Normal,
					            "MvrBatteryConnector::connectBatteries: Could not connect %s, continuing with remainder of batteries",
					            batteryData->myBattery->getName());
			}
		}
	}
	MvrLog::log (myInfoLogLevel,
	            "MvrBatteryConnector: Done connecting batteries");
	return true;
}

MVREXPORT bool MvrBatteryConnector::disconnectBatteries()
{
	std::map<int, BatteryData *>::iterator it;
	BatteryData *batteryData = NULL;
	MvrLog::log (myInfoLogLevel, "MvrBatteryConnector: Disconnecting from batteries");
  for (it = myBatteries.begin(); it != myBatteries.end(); it++) 
  {
    batteryData = (*it).second;
    if(batteryData)
    {
      if(myRobot)
        myRobot->remBattery(batteryData->myBattery);
      if(batteryData->myBattery)
        batteryData->myBattery->disconnect();
    }
  }

  return true;
}

