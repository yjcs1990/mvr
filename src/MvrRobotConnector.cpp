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
#include "MvrRobotConnector.h"
#include "MvrRobot.h"
#include "MvrSick.h"
#include "mvriaInternal.h"
#include "MvrCommands.h"
#include "MvrSonarConnector.h"
#include "MvrBatteryConnector.h"
//#include "MvrLCDConnector.h"
#include <assert.h>


/** @warning do not delete @a parser during the lifetime of this
 * MvrRobotConnector, which may need to access its contents later.

 * @param parser argument parser object (see parseArgs() for list of command
 * line arguments recognized)
 * @param robot MvrRobot object to connect to the robot
 * @param autoParseArgs if true (default), trigger argument parsing before
 * connection if not already done. (normally, a program will call
 * Mvria::parseArgs() to trigger argument parsing for all created objects.
 * @param connectAllComponents if true (default), then connect to all robot
 * components. If false, then your program may need to create additional
 * connector objects for
 * components such as sonar and battery.  (MTX-series robots have separate
 * connections to components such as battery, sonar, etc. Other robots do
 * not require this.)  If you wish to disconnect all components including the robot, call
 * disconnectAll(); Use MvrRobot::disconnect() to disconnect from just
 * the robot.
 */
MVREXPORT MvrRobotConnector::MvrRobotConnector(
	MvrArgumentParser *parser, MvrRobot *robot, bool autoParseArgs, bool connectAllComponents) :
  myParseArgsCB(this, &MvrRobotConnector::parseArgs),
  myLogOptionsCB(this, &MvrRobotConnector::logOptions),
  myBatteryConnector(NULL),
//  myLCDConnector(NULL),
  mySonarConnector(NULL)
{
  myParser = parser;
  myOwnParser = false;
  myRobot = robot;
  myAutoParseArgs = autoParseArgs;
  myHaveParsedArgs = false;
  myConnectAllComponents = connectAllComponents;

  myParseArgsCB.setName("MvrRobotConnector");
  Mvria::addParseArgsCB(&myParseArgsCB, 75);
  myLogOptionsCB.setName("MvrRobotConnector");
  Mvria::addLogOptionsCB(&myLogOptionsCB, 90);

  myRemoteHost = NULL;
  myRobotPort = NULL;
  myRemoteRobotTcpPort = 8101;
  myRobotBaud = 9600;
  myRemoteIsSim = false;
  myRemoteIsNotSim = false;
  myRobotLogPacketsReceived = false;
  myRobotLogPacketsSent = false;
  myRobotLogMovementReceived = false;
  myRobotLogMovementSent = false;
  myRobotLogVelocitiesReceived = false;
  myRobotLogActions = false;

  if(myConnectAllComponents)
  {
    myBatteryConnector = new MvrBatteryConnector(myParser, myRobot, this);
//    myLCDConnector = new MvrLCDConnector(myParser, myRobot, this);
    mySonarConnector = new MvrSonarConnector(myParser, myRobot, this, true/*autoParseArgs*/, MvrLog::Verbose);
  }

}

MVREXPORT MvrRobotConnector::~MvrRobotConnector(void)
{
  if(myBatteryConnector)
    delete myBatteryConnector;
//  if(myLCDConnector)
//    delete myLCDConnector;
  if(mySonarConnector)
    delete mySonarConnector;
//  Mvria::remParseArgsCB(&myParseArgsCB);
//  Mvria::remLogOptionsCB(&myLogOptionsCB);
}

/**
 * Parse command line arguments using the MvrArgumentParser given in the MvrRobotConnector constructor.
 *
 * See parseArgs(MvrArgumentParser*) for details about argument parsing.
 * 
  @return true if the arguments were parsed successfully false if not
 **/

MVREXPORT bool MvrRobotConnector::parseArgs(void)
{
  if(myParser)
    return parseArgs(myParser);
  else
    return false;
}

/**
 * Parse command line arguments for MvrRobotConnector held by the given MvrArgumentParser.
 * Normally called via global Mvria::parseArgs() method.
 *
  @return true if the arguments were parsed successfully false if not

   The following arguments are used for the robot connection:

   <dl>
    <dt><code>-robotPort</code> <i>port</i></dt>
    <dt><code>-rp</code> <i>port</i></dt>
    <dd>Use the given serial port device name for a serial port connection (e.g. <code>COM1</code>, or <code>/dev/ttyS0</code> if on Linux.)
    The default is the first serial port, or COM1, which is the typical Pioneer setup.
    </dd>

    <dt><code>-remoteHost</code> <i>hostname</i></dt>
    <dt><code>-rh</code> <i>hostname</i></dt>
    <dd>Use a TCP connection to a remote computer with the given network host name instead of a serial port connection</dd>

    <dt><code>-remoteRobotTcpPort</code> <i>port</i></dt>
    <dt><code>-rrtp</code> <i>port</i></dt>
    <dd>Use the given TCP port number if connecting to a remote robot using TCP due to <code>-remoteHost</code> having been given.</dd>

    <dt><code>-remoteIsSim</code></dt>
    <dt><code>-ris</code></dt>
    <dd>The remote TCP robot given by <code>-remoteHost</code> or <code>-rh</code> is actually a simulator. Use any alternative
     behavior intended for the simulator (e.g. tell the laser device object to request laser data from the simulator rather
     than trying to connect to a real laser device on the local computer)</dd>

    <dt><code>-remoteIsNotSim</code></dt>
    <dt><code>-rins</code></dt>
    <dd>The remote TCP robot given by <code>-remoteHost</code> or
     <code>-rh</code> (or their defaults) is actually a simulator but
     DO NOT treat it like one. Use any the default robot behavior, instead of the simulator behavior (ie open a real laser device for the laser type given and try and talk to it over it's actual local port or remote port if given)</dd>

    <dt><code>-robotBaud</code> <i>baudrate</i></dt>
    <dt><code>-rb</code> <i>baudrate</i></dt>
    <dd>Use the given baud rate when connecting over a serial port, instead of trying to use the normal rate.</dd>
  </dl>

 **/

MVREXPORT bool MvrRobotConnector::parseArgs(MvrArgumentParser *parser)
{
  myHaveParsedArgs = true;

  bool wasReallySetOnlyTrue = parser->getWasReallySetOnlyTrue();
  parser->setWasReallySetOnlyTrue(true);

  if (parser->checkArgument("-remoteIsSim") ||
      parser->checkArgument("-ris"))      
    myRemoteIsSim = true;

  if (parser->checkArgument("-remoteIsNotSim") ||
      parser->checkArgument("-rins"))      
    myRemoteIsNotSim = true;

  if (parser->checkArgument("-robotLogPacketsReceived") || 
      parser->checkArgument("-rlpr")) 

    myRobotLogPacketsReceived = true;
  
  if (parser->checkArgument("-robotLogPacketsSent") || 
      parser->checkArgument("-rlps"))      
    myRobotLogPacketsSent = true;

  if (parser->checkArgument("-robotLogMovementReceived") || 
      parser->checkArgument("-rlmr"))      
    myRobotLogMovementReceived = true;

  if (parser->checkArgument("-robotLogMovementSent") || 
      parser->checkArgument("-rlms"))

    myRobotLogMovementSent = true;
  
  if (parser->checkArgument("-robotLogVelocitiesReceived") || 
      parser->checkArgument("-rlvr"))
      
    myRobotLogVelocitiesReceived = true;
  
  if (parser->checkArgument("-robotLogActions") || 
      parser->checkArgument("-rla"))
    myRobotLogActions = true;

  if (!parser->checkParameterArgumentString("-remoteHost", 
					    &myRemoteHost) ||
      !parser->checkParameterArgumentString("-rh", 
					    &myRemoteHost) ||

      !parser->checkParameterArgumentString("-robotPort",
					     &myRobotPort) ||
      !parser->checkParameterArgumentString("-rp",
					     &myRobotPort) ||

      !parser->checkParameterArgumentInteger("-remoteRobotTcpPort",
					      &myRemoteRobotTcpPort) ||
      !parser->checkParameterArgumentInteger("-rrtp",
					      &myRemoteRobotTcpPort) ||

      !parser->checkParameterArgumentInteger("-robotBaud",
					     &myRobotBaud) || 
      !parser->checkParameterArgumentInteger("-rb",
					     &myRobotBaud))      
  {
    parser->setWasReallySetOnlyTrue(wasReallySetOnlyTrue);
    return false;
  }

  parser->setWasReallySetOnlyTrue(wasReallySetOnlyTrue);
  return true;
}

/** Normally called by Mvria::logOptions(). */
MVREXPORT void MvrRobotConnector::logOptions(void) const
{
  MvrLog::log(MvrLog::Terse, "Options for MvrRobotConnector (see docs for more details):");
  MvrLog::log(MvrLog::Terse, "");

  MvrLog::log(MvrLog::Terse, "Robot options:");
  MvrLog::log(MvrLog::Terse, "-remoteHost <remoteHostNameOrIP>");
  MvrLog::log(MvrLog::Terse, "-rh <remoteHostNameOrIP>");
  MvrLog::log(MvrLog::Terse, "-robotPort <robotSerialPort>");
  MvrLog::log(MvrLog::Terse, "-rp <robotSerialPort>");
  MvrLog::log(MvrLog::Terse, "-robotBaud <baud>");
  MvrLog::log(MvrLog::Terse, "-rb <baud>");
  MvrLog::log(MvrLog::Terse, "-remoteRobotTcpPort <remoteRobotTcpPort>");
  MvrLog::log(MvrLog::Terse, "-rrtp <remoteRobotTcpPort>");
  MvrLog::log(MvrLog::Terse, "-remoteIsSim");
  MvrLog::log(MvrLog::Terse, "-ris");
  MvrLog::log(MvrLog::Terse, "-remoteIsNotSim");
  MvrLog::log(MvrLog::Terse, "-rins");
  MvrLog::log(MvrLog::Terse, "-robotLogPacketsReceived");
  MvrLog::log(MvrLog::Terse, "-rlpr");
  MvrLog::log(MvrLog::Terse, "-robotLogPacketsSent");
  MvrLog::log(MvrLog::Terse, "-rlps");
  MvrLog::log(MvrLog::Terse, "-robotLogMovementReceived");
  MvrLog::log(MvrLog::Terse, "-rlmr");
  MvrLog::log(MvrLog::Terse, "-robotLogMovementSent");
  MvrLog::log(MvrLog::Terse, "-rlms");
  MvrLog::log(MvrLog::Terse, "-robotLogVelocitiesReceived");
  MvrLog::log(MvrLog::Terse, "-rlvr");
  MvrLog::log(MvrLog::Terse, "-robotLogActions");
  MvrLog::log(MvrLog::Terse, "-rla");
}

/**
 * This method is normally used internally by connectRobot(), but you may 
 * use it if you wish.
 *
 * If -remoteHost was given, then open that TCP port. If it was not given,
 * then try to open a TCP port to the simulator on localhost.
 * If that fails, then use a local serial port connection.
 * Sets the given MvrRobot's device connection pointer to this object.
 * Sets up internal settings determined by command line arguments such
 * as serial port and baud rate, etc.
 *
 * After calling this function  (and it returns true), then you may connect
 * MvrRobot to the robot using MvrRobot::blockingConnect() (or similar).
 *
 * @return false if -remoteHost was given and there was an error connecting to
 * the remote host, true otherwise.
 **/
MVREXPORT bool MvrRobotConnector::setupRobot(void)
{
  return setupRobot(myRobot);
}

/**
 * @deprecated
 * 
 * This method is normally used internally by connectRobot(), but you may 
 * use it if you wish.
 *
 * If -remoteHost was given, then open that TCP port. If it was not given,
 * then try to open a TCP port to the simulator on localhost.
 * If that fails, then use a local serial port connection.
 * Sets the given MvrRobot's device connection pointer to this object.
 * Sets up internal settings determined by command line arguments such
 * as serial port and baud rate, etc.
 *
 * After calling this function  (and it returns true), then you may connect
 * MvrRobot to the robot using MvrRobot::blockingConnect() (or similar).
 *
 * @return false if -remoteHost was given and there was an error connecting to
 * the remote host, true otherwise.
 **/
MVREXPORT bool MvrRobotConnector::setupRobot(MvrRobot *robot)
{
  if (myRobot == NULL)
    myRobot = robot;

  if (myRobot == NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobotConnector::setupRobot: NULL robot, cannot setup robot");
    return false;
  }

  if (myAutoParseArgs)
  {
    parseArgs();
  }
  else if (!myHaveParsedArgs)
  {
    MvrLog::log(MvrLog::Normal, "MvrRobotConnector: Args not parsed and are not autoparsed, so connection may fail and command line arguments won't be used");
  }

  // set up all that logging
  if (myRobotLogPacketsReceived)
    robot->setPacketsReceivedTracking(true);
  if (myRobotLogPacketsSent)
    robot->setPacketsSentTracking(true);
  if (myRobotLogMovementReceived)
    robot->setLogMovementReceived(true);
  if (myRobotLogMovementSent)
    robot->setLogMovementSent(true);
  if (myRobotLogVelocitiesReceived)
    robot->setLogVelocitiesReceived(true);
  if (myRobotLogActions)
    robot->setLogActions(true);

  if (myRobot->getDeviceConnection() != NULL)
  {
    MvrLog::log(MvrLog::Normal, 
	       "MvrRobotConnector::setupRobot: robot already has device connection, will not setup robot");
    return true;
  }

  // We see if we can open the tcp connection, if we can we'll assume
  // we're connecting to the sim, and just go on...  if we can't open
  // the tcp it means the sim isn't there, so just try the robot

  // see if we're doing remote host or not
  if (myRemoteHost != NULL)
    myRobotTcpConn.setPort(myRemoteHost, myRemoteRobotTcpPort);
  else
    myRobotTcpConn.setPort("localhost", myRemoteRobotTcpPort);

  // see if we can get to the simulator  (true is success)
  MvrLog::log(MvrLog::Normal, "Connnecting to robot using TCP connection to %s...", myRobotTcpConn.getPortName());// , myRemoteHost, myRobotTcpConn.getPort());
  if (myRobotTcpConn.openSimple())
  {
    robot->setDeviceConnection(&myRobotTcpConn);
    // we could get to the sim, so set the robots device connection to the sim
    if (myRemoteHost != NULL)
    {
      MvrLog::log(MvrLog::Normal, "Connected to remote host %s through tcp.\n", 
		 myRemoteHost);
      if (myRemoteIsSim)
	myUsingSim = true;
      else
	myUsingSim = false;
    }
    else
    {
      MvrLog::log(MvrLog::Normal, "Connecting to simulator through tcp.\n");
      myUsingSim = true;
    }
  }
  else
  {
    // if we were trying for a remote host and it failed, just exit
    if (myRemoteHost != NULL)
    {
      MvrLog::log(MvrLog::Terse, "Could not connect robot to remote host %s, port %d.\n", myRemoteHost, myRemoteRobotTcpPort);
      return false;
    }
    // we couldn't get to the sim, so set the port on the serial
    // connection and then set the serial connection as the robots
    // device

    myRobotSerConn.setPort(myRobotPort);
    myRobotSerConn.setBaud(myRobotBaud);
    MvrLog::log(MvrLog::Normal,
	       "Could not connect to simulator, connecting to robot through serial port %s.", 
	       myRobotSerConn.getPort());
    robot->setDeviceConnection(&myRobotSerConn);
    myUsingSim = false;
  }
  return true;
}

MVREXPORT bool MvrRobotConnector::disconnectAll()
{
  bool r = true;
  if(myBatteryConnector)
    r = r && myBatteryConnector->disconnectBatteries();
  if(mySonarConnector)
    r = r && mySonarConnector->disconnectSonars();
  //if(myLCDConnector)
  // r = r && myLCDConnector->disconnectLCDs();
  if(myRobot)
    r = r && myRobot->disconnect();
  return r;
}

/** Prepares the given MvrRobot object for connection, then begins
 * a blocking connection attempt.
 * If you wish to simply prepare the MvrRobot object, but not begin
 * the connection, then use setupRobot().
 */
MVREXPORT bool MvrRobotConnector::connectRobot(void)
{
  if(! connectRobot(myRobot) )
    return false;


  if(myConnectAllComponents)
  {
    if(getRemoteIsSim() )
    {
      MvrLog::log(MvrLog::Normal, "MvrRobotConnector: Connected to simulator, not connecting to additional hardware components.");
    }
    else
    {
      
      if(myBatteryConnector)
      {
        MvrLog::log(MvrLog::Normal, "MvrRobotConnector: Connecting to MTX batteries (if neccesary)...");
        if(!myBatteryConnector->connectBatteries())
        {
          MvrLog::log(MvrLog::Terse, "MvrRobotConnector: Error: Could not connect to robot batteries.");
          return false;
        }
      }

/*
      if(myLCDConnector)
      {
        MvrLog::log(MvrLog::Normal, "MvrRobotConnector: Connecting to MTX LCD (if neccesary)...");
        if(!myLCDConnector->connectLCDs())
        {
          MvrLog::log(MvrLog::Terse, "MvrRobotConnector: Error: Could not connect to robot LCD interface.");
          return false;
        }
      }
*/

assert(mySonarConnector);
      if(mySonarConnector)
      {
        MvrLog::log(MvrLog::Normal, "MvrRobotConnector: Connecting to MTX sonar (if neccesary)...");
        if(!mySonarConnector->connectSonars())
        {
          MvrLog::log(MvrLog::Terse, "MvrRobotConnector: Error: Could not connect to sonar(s).");
          return false;
        }
        
      } 
    }
  }

  return true;
}

/** Prepares the given MvrRobot object for connection, then begins
 * a blocking connection attempt.
 * If you wish to simply prepare the MvrRobot object, but not begin
 * the connection, then use setupRobot().
 */
MVREXPORT bool MvrRobotConnector::connectRobot(MvrRobot *robot)
{
  if (!setupRobot(robot))
    return false;
  else
    return robot->blockingConnect();
}

MVREXPORT const char *MvrRobotConnector::getRemoteHost(void) const
{
  return myRemoteHost;
}


MVREXPORT bool MvrRobotConnector::getRemoteIsSim(void) const
{
  if (myRemoteIsSim) 
    return true;
  else if (myRemoteIsNotSim)
    return false;
  else if (myRobot != NULL && 
	   strcasecmp(myRobot->getRobotName(), "MobileSim") == 0)
    return true;
  else
    return false;
}

MVREXPORT void MvrRobotConnector::setRemoteIsSim(bool remoteIsSim) 
{
  if (remoteIsSim)
  {
    myRemoteIsSim = true;
    myRemoteIsNotSim = false;
  }
  else 
  {
    myRemoteIsSim = false;
    myRemoteIsNotSim = true;
  }
}

MVREXPORT MvrRobot *MvrRobotConnector::getRobot(void) 
{
  return myRobot;
}
