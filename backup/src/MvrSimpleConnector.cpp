/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSimpleConnector.h
 > Description  : Legacy connector for robot and laser
 > Author       : Yu Jie
 > Create Time  : 2017年05月18日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrRobot.h"
#include "MvrLaser.h"
#include "mvriaInternal.h"
#include "MvrSimpleConnector.h"

MVREXPORT MvrSimpleConnector::MvrSimpleConnector(int *argc, char **argv)
{
  myParser    = new MvrArgumentParser(argc, argv);
  myOwnParser = true;
  finishConstructor();
}

/*
 * @warning do not delete @param builder during the lifetime of this
 * MvrSimpleConnector, which may need to access its contents later.
 */
MVREXPORT MvrSimpleConnector::MvrSimpleConnector(MvrArgumentBuilder *builder)
{
  myParser    = new MvrArgumentParser(builder);
  myOwnParser = true;
  finishConstructor();
}

/*
 * @warning do not delete @param builder during the lifetime of this
 * MvrSimpleConnector, which may need to access its contents later.
 */
MVREXPORT MvrSimpleConnector::MvrSimpleConnector(MvrArgumentParser *parser)
{
  myParser    = parser;
  myOwnParser = true;
  finishConstructor();
}

MVREXPORT MvrSimpleConnector::~MvrSimpleConnector()
{

}

void MvrSimpleConnector::finishConstructor(void)
{
  myRobotConnector = new MvrRobotConnector(myParser, NULL);
  myLaserConnector = new MvrLaserConnector(myParser, NULL, myRobotConnector);
  setMaxNumLasers();
  // myLaserConnector->addLaser();
}
 
MVREXPORT void MvrSimpleConnector::setMaxNumLasers(int maxNumLasers)
{
  int i;
  for (i = 0; i <= maxNumLasers; i++)
    myLaserConnector->addPlaceholderLaser(new MvrSick, i, true);
}

MVREXPORT bool MvrSimpleConnector::parseArgs(void)
{
  return parseArgs(myParser);
}

/*
 * Parse command line arguments held by the given MvrArgumentParser.
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

    <dt><code>-robotBaud</code> <i>baudrate</i></dt>
    <dt><code>-rb</code> <i>baudrate</i></dt>
    <dd>Use the given baud rate when connecting over a serial port, instead of trying to use the normal rate.</dd>
  </dl>

  The following arguments are accepted for laser connections.  A program may request support for more than one laser
  using setMaxNumLasers(); if multi-laser support is enabled in this way, then these arguments must have the laser index
  number appended. For example, "-laserPort" for laser 1 would instead by "-laserPort1", and for laser 2 it would be
  "-laserPort2".

  <dl>
    <dt>-laserPort <i>port</i></dt>
    <dt>-lp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a laser. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
    The default laser port is COM2, which is the typical Pioneer laser port setup.
    </dd>

    <dt>-laserFlipped <i>true|false</i></dt>
    <dt>-lf <i>true|false</i></dt>
    <dd>If <code>true</code>, then the laser is mounted upside-down on the robot and the ordering of readings
    should be reversed.</dd>

    <dt>-connectLaser</dt>
    <dt>-cl</dt>
    <dd>Explicitly request that the client program connect to a laser, if it does not always do so</dd>

    <dt>-laserPowerControlled <i>true|false</i></dt>
    <dt>-lpc <i>true|false</i></dt>
    <dd>If <code>true</code>, then the laser is powered on when the serial port is initially opened, so enable
    certain features when connecting such as a waiting period as the laser initializes.</dd>

    <dt>-laserDegrees <i>degrees</i></dt>
    <dt>-ld <i>degrees</i></dt>
    <dd>Indicate the size of the laser field of view, either <code>180</code> (default) or <code>100</code>.</dd>

    <dt>-laserIncrement <i>increment</i></dt>
    <dt>-li <i>increment</i></dt>
    <dd>Configures the laser's angular resolution. If <code>one</code>, then configure the laser to take a reading every degree.
     If <code>half</code>, then configure it for a reading every 1/2 degrees.</dd>

    <dt>-laserUnits <i>units</i></dt>
    <dt>-lu <i>units</i></dt>
    <dd>Configures the laser's range resolution.  May be 1mm for one milimiter, 1cm for ten milimeters, or 10cm for one hundred milimeters.</dd>

    <dt>-laserReflectorBits <i>bits</i></dt>
    <dt>-lrb <i>bits</i></dt>
    <dd>Enables special reflectance detection, and configures the granularity of reflector detection information. Using more bits allows the laser to provide values for several different
    reflectance levels, but also may force a reduction in range.  (Note, the SICK LMS-200 only detects high reflectance on special reflector material
    manufactured by SICK.)
    </dd>
  </dl>
 */

MVREXPORT bool MvrSimpleConnector::parseArgs(MvrArgumentParser *parser)
{
  return myRobotConnector->parseArgs() && myLaserConnector->parseArgs();
}

MVREXPORT void MvrSimpleConnector::logOptions(void) const
{
  myRobotConnector->logOptions();
  myLaserConnector->logOptions();
}

/*
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
 */

MVREXPORT bool MvrSimpleConnector::setupRobot(MvrRobot *robot)
{
  return myRobotConnector->setupRobot(robot);
}

/** Prepares the given MvrRobot object for connection, then begins
 * a blocking connection attempt.
 * If you wish to simply prepare the MvrRobot object, but not begin
 * the connection, then use setupRobot().
 */

MVREXPORT bool MvrSimpleConnector::connectRobot(MvrRobot *robot)
{
  return myRobotConnector->connectRobot(robot);
}
/*
 * Description of the logic for connection to the laser:  If
 * --remoteHost then the laser will a tcp connection will be opened to
 * that remoteHost at port 8102 or --remoteLaserTcpPort if that
 * argument is given, if this connection fails then the setup fails.
 * If --remoteHost wasn't provided and the robot connected to a
 * simulator as described elsewhere then the laser is just configured
 * to be simulated, if the robot isn't connected to a simulator it
 * tries to open a serial connection to MvrUtil::COM3 or --laserPort if
 * that argument is given.
 */

MVREXPORT bool MvrSimpleConnector::setupLaser(MvrSick *laser)
{
  return myLaserConnector->setupLaser(laser, 1);
}

/* Prepares the given MvrRobot object for connection, then begins
 * a blocking connection attempt.
 * If you wish to simply prepare the MvrRobot object, but not begin
 * the connection, then use setupRobot().
 */

MVREXPORT bool MvrSimpleConnector::setupSecondLaser(MvrSick *laser)
{
  return myLaserConnector->setupLaser(laser, 2);
}

MVREXPORT bool MvrSimpleConnector::setupLaserArbitrary(MvrSick *laser, int laserNumber)
{
  return myLaserConnector->setupLaser(laser, laserNumber);
}

/*
 * This will setup and connect the laser if the command line switch
 * was given to do so or simply return true if no connection was
 * wanted.
 */
MVREXPORT bool MvrSimpleConnector::connectLaser(MvrSick *laser)
{
  return myLaserConnector->connectLaser(laser, 1, false);
}


/*
 * This will setup and connect the laser if the command line switch
 * was given to do so or simply return true if no connection was
 * requested.
 */
MVREXPORT bool MvrSimpleConnector::connectSecondLaser(MvrSick *laser)
{
  return myLaserConnector->connectLaser(laser, 1, false);
}

MVREXPORT bool MvrSimpleConnector::connectLaserArbitrary(MvrSick *laser, int laserNumber)
{
  return myLaserConnector->connectLaser(laser, laserNumber, false);
}