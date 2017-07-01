#ifndef MVRGPSCONNECTOR_H
#define MVRGPSCONNECTOR_H

#include <string>
#include <vector>

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
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
  MVREXPORT MvrGPSConnector(MvrArgumentParser* argParser);
  MVREXPORT ~MvrGPSConnector();

  /** Gets command line arguments */
  MVREXPORT bool parseArgs();


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
  MVREXPORT MvrGPS* createGPS(MvrRobot *robot = NULL);
  /** @copydoc createGPS() */
  MVREXPORT MvrGPS* create(MvrRobot *robot = NULL) { return createGPS(robot); }

#if 0  

//doesn't really do anything :
  /** Try to establish a device connection between @a gps (created by calling
   * createGPS() and the GPS receiver.
   */
  MVREXPORT bool connectGPS(MvrGPS *gps);
  /** @copydoc connectGPS() */
  MVREXPORT bool connect(MvrGPS *gps) { return connectGPS(gps) ; }
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

  MVREXPORT GPSType getGPSType() const { return myDeviceType; }

protected:
  MvrDeviceConnection *myDeviceCon;
  MvrArgumentParser *myArgParser;
  MvrRetFunctorC<bool, MvrGPSConnector> myParseArgsCallback;
  MvrFunctorC<MvrGPSConnector> myLogArgsCallback;
  int myBaud;
  const char *myPort;
  const char *myTCPHost;
  int myTCPPort;
  GPSType myDeviceType;


  /** Log argument option information */
  MVREXPORT void logOptions();
  MVREXPORT GPSType deviceTypeFromString(const char *str);
};


#endif  // ifdef MVRGPSCONNECTOR_H


