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

#ifndef ARGPSCONNECTOR_H
#define ARGPSCONNECTOR_H

#include <string>
#include <vector>

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "MvrFunctor.h"
#include "MvrGPS.h"

class MvrDeviceConnection;
class MvrRobot;

/** 
 *  @brief Factory for creating GPS interface object (for any kind of GPS supported by ARIA) based on robot parameter file and command-line arguments.
 *
 *  First, create an MvrGPSConnector object before 
 *  calling Mvria::parseArgs().  After connecting to the robot, call
 *  Mvria::parseArgs().  Then, call createGPS() to create the GPS object.
 *  
 *  MvrGPSConnector can connect to a Novatel GPS ("novatel" type), Trimble AgGPS
 *  ("trimble" type), or any GPS
 *  supporting the NMEA standard protocol ("standard" type), if that GPS does not need any special
 *  commands to initialize.
 *
 *  @note The device connection object created by 
 *  MvrGPSConnector is destroyed  when MvrGPSConnector is 
 *  destroyed. Therefore, you must not destroy an MvrGPSConnector
 *  while its associated MvrGPS is in use.
 *
 * The following command-line arguments are checked:
 * @verbinclude MvrGPSConnector_options
 *
 * Only one GPS device may be configured and connected to by this object.
 *
  @ingroup OptionalClasses
   @ingroup DeviceClasses
*/

class MvrGPSConnector {
public:
  AREXPORT MvrGPSConnector(MvrArgumentParser* argParser);
  AREXPORT ~ArGPSConnector();

  /** Gets command line arguments */
  AREXPORT bool parseArgs();


  /** Create a new GPS object (may be an MvrGPS subclass based on device type)
   * and a device connection for that GPS. Use MvrGPS::blockingConnect() to open the connection.
   *
   * @param robot If not NULL, obtain default values for GPS type, port and baud
   * from this robot's parameters (given in parameter file), for any of these
   * not set from command-line arguments in parseArgs().
   *
   * @return NULL if there was an error creating a GPS object or an error
   * creating and opening its device connection. Otherwise, return the new GPS
   * object.  
   */
  AREXPORT MvrGPS* createGPS(MvrRobot *robot = NULL);
  /** @copydoc createGPS() */
  AREXPORT MvrGPS* create(MvrRobot *robot = NULL) { return createGPS(robot); }

#if 0  

//doesn't really do anything :
  /** Try to establish a device connection between @a gps (created by calling
   * createGPS() and the GPS receiver.
   */
  AREXPORT bool connectGPS(MvrGPS *gps);
  /** @copydoc connectGPS() */
  AREXPORT bool connect(MvrGPS *gps) { return connectGPS(gps) ; }
#endif

  /** @brief Device type identifiers */
  typedef enum {
      /// For a standard NMEA GPS device (no extra initialization or interpretation needed) accessible using MvrGPS 
      Standard, 
      /// For a Novatel device accessible using MvrNovatelGPS 
      Novatel,
      /// For a Trimble device accessible using MvrTrimbleGPS
      Trimble,
      /// Not set or invalid
      Invalid,
      /// Novatel SPAN
      /// @since Mvria 2.7.2
      NovatelSPAN,
      /// Simulated (program must set dummy positions)
      /// @since Mvria 2.7.6
      Simulator
  } GPSType;

  AREXPORT GPSType getGPSType() const { return myDeviceType; }

protected:
  MvrDeviceConnection *myDeviceCon;
  MvrArgumentParser *myArgParser;
  MvrRetFunctorC<bool, MvrGPSConnector> myParseArgsCallback;
  MvrFunctorC<ArGPSConnector> myLogArgsCallback;
  int myBaud;
  const char *myPort;
  const char *myTCPHost;
  int myTCPPort;
  GPSType myDeviceType;


  /** Log argument option information */
  AREXPORT void logOptions();
  AREXPORT GPSType deviceTypeFromString(const char *str);
};


#endif  // ifdef ARGPSCONNECTOR_H


