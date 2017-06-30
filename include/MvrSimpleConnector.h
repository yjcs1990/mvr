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
#ifndef ARSIMPLECONNECTOR_H
#define ARSIMPLECONNECTOR_H

#include "ariaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "ariaUtil.h"
#include "MvrRobotConnector.h"
#include "MvrLaserConnector.h"

class MvrSick;
class MvrRobot;



/// Legacy connector for robot and laser
/**
   This is deprecated but is left in for compatibility with old code,
   Instead use MvrRobotConnector to set up MvrRobot's connection to the robot, and
   MvrLaserConnector to set up connections with laser rangefinder devices.

   @deprecated Use MvrRobotConnector and MvrLaserConnector instead
 **/
class MvrSimpleConnector
{
public:
  /// Constructor that takes args from the main
  MVREXPORT MvrSimpleConnector(int *argc, char **argv);
  /// Constructor that takes argument builder
  MVREXPORT MvrSimpleConnector(MvrArgumentBuilder *arguments);
  /// Constructor that takes argument parser
  MVREXPORT MvrSimpleConnector(MvrArgumentParser *parser);
  /// Destructor
  MVREXPORT ~MvrSimpleConnector(void);
  /// Sets up the robot to be connected
  MVREXPORT bool setupRobot(MvrRobot *robot);
  /// Sets up the robot then connects it
  MVREXPORT bool connectRobot(MvrRobot *robot);
  /// Sets up the laser to be connected
  MVREXPORT bool setupLaser(MvrSick *laser);
  /// Sets up a second laser to be connected
  MVREXPORT bool setupSecondLaser(MvrSick *laser);
  /// Sets up a laser t obe connected (make sure you setMaxNumLasers)
  MVREXPORT bool setupLaserArbitrary(MvrSick *laser, 
				    int laserNumber);
  /// Connects the laser synchronously (will take up to a minute)
  MVREXPORT bool connectLaser(MvrSick *laser);
  /// Connects the laser synchronously (will take up to a minute)
  MVREXPORT bool connectSecondLaser(MvrSick *laser);
  /// Connects the laser synchronously  (make sure you setMaxNumLasers)
  MVREXPORT bool connectLaserArbitrary(MvrSick *laser, int laserNumber);
  /// Function to parse the arguments given in the constructor
  MVREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  MVREXPORT bool parseArgs(MvrArgumentParser *parser);
  /// Log the options the simple connector has
  MVREXPORT void logOptions(void) const;
  /// Sets the number of possible lasers 
  MVREXPORT void setMaxNumLasers(int maxNumLasers = 1);
protected:
  /// Finishes the stuff the constructor needs to do
  void finishConstructor(void);

  MvrArgumentParser *myParser;
  bool myOwnParser;
  MvrRobotConnector *myRobotConnector;
  MvrLaserConnector *myLaserConnector;
};

#endif // ARSIMPLECONNECTOR_H
