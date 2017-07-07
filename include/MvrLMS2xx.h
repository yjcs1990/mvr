#ifndef MVRLMS2XX_H
#define MVRLMS2XX_H


#include "mvriaTypedefs.h"
#include "MvrLMS2xxPacket.h"
#include "MvrLMS2xxPacketReceiver.h"
#include "MvrRobotPacket.h"
#include "MvrLaser.h"   
#include "MvrFunctor.h"
#include "MvrCondition.h"

/// Interface to a SICK LMS-200 laser range device
/**
 * This class processes incoming data from a SICK LMS-200
 * laser rangefinding device in a background thread, and provides
 * it through the standard MvrRangeDevice API, to be used via MvrRobot
 * (see MvrRobot::addRangeDevice()), used by an MvrAction, or used directly.
 *
 * An MvrSick instance must be connected to the laser through a serial port
 * (or simulator): the typical procedure is to allow your MvrSimpleConnector
 * to configure the laser based on the robot connection type and command
 * line parameters; then initiate the MvrSick background thread; and finally
 * connect MvrSick to the laser device.
 * For example:
 * @code
 *  MvrRobot robot;
 *  MvrSick laser;
 *  MvrSimpleConnector connector(...);
 *  ...
 *   Setup the simple connector and connect to the robot --
 *   see the example programs.
 *  ...
 *  connector.setupLaser(&laser);
 *  laser.runAsync();
 *  if(!laser.blockingConnect())
 *  {
 *    // Error...
 *    ...
 *  }
 *  ...
 * @endcode
 *
 * The most important methods in this class are the constructor, runAsync(), 
 * blockingConnect(), getSensorPosition(), isConnected(), addConnectCB(),
 * asyncConnect(), configure(), in addition to the MvrRangeDevice interface. 
 *
 * @note The "extra int" on the raw readings returned by
 * MvrRangeDevice::getRawReadings() is like other laser
 * devices and is the reflectance value, if enabled, ranging between 0 and 255.
 *
 * MvrLMS2xx uses the following buffer parameters by default (see MvrRangeDevice
 * documentation):
 * <dl>
 *  <dt>MinDistBetweenCurrent <dd>50 mm
 *  <dt>MaxDistToKeepCumulative <dd>6000 mm
 *  <dt>MinDistBetweenCumulative <dd>200 mm
 *  <dt>MaxSecondsToKeepCumulative <dd>30 sec
 *  <dt>MaxINsertDistCumulative <dd>3000 mm
 * </dl>
 * The current buffer is replaced for each new set of readings.
 *
 * @since 2.7.0
**/
class MvrLMS2xx : public MvrLaser
{
public:
  /// Constructor
  MVREXPORT MvrLMS2xx(int laserNumber,
		    const char *name = "lms2xx",
		    bool appendLaserNumberToName = true);

  /// Destructor
  MVREXPORT virtual ~MvrLMS2xx();

  /// Connect to the laser while blocking
  MVREXPORT virtual bool blockingConnect(void);
  /// Connect to the laser asyncronously
  MVREXPORT bool asyncConnect(void);
  /// Disconnect from the laser
  MVREXPORT virtual bool disconnect(void);
  /// Sees if this is connected to the laser
  MVREXPORT virtual bool isConnected(void) 
    { if (myState == STATE_CONNECTED) return true; else return false; }
  MVREXPORT virtual bool isTryingToConnect(void) 
    { 
      if (myState != STATE_CONNECTED && myState != STATE_NONE) 
	return true; 
      else if (myStartConnect)
	return true;
      else 
	return false; 
    }

  /// Sets the device connection
  MVREXPORT virtual void setDeviceConnection(MvrDeviceConnection *conn);

  /** The internal function used by the MvrRangeDeviceThreaded
   *  @internal
   */
  MVREXPORT virtual void * runThread(void *arg);
  MVREXPORT virtual void setRobot(MvrRobot *robot);
protected:
  // The packet handler for when connected to the simulator
  MVREXPORT bool simPacketHandler(MvrRobotPacket * packet);
  // The function called if the laser isn't running in its own thread and isn't simulated
  MVREXPORT void sensorInterpCallback(void);
  // An internal function for connecting to the sim
  MVREXPORT bool internalConnectSim(void);
  /// An internal function, single loop event to connect to laser
  MVREXPORT int internalConnectHandler(void);
  // The internal function which processes the sickPackets
  MVREXPORT void processPacket(MvrLMS2xxPacket *packet, MvrPose pose, 
			      MvrPose encoderPose, unsigned int counter,
			      bool deinterlace, MvrPose deinterlaceDelta);
  // The internal function that gets does the work
  MVREXPORT void runOnce(bool lockRobot);
  // Internal function, shouldn't be used, drops the conn because of error
  MVREXPORT void dropConnection(void);
  // Internal function, shouldn't be used, denotes the conn failed
  MVREXPORT void failedConnect(void);
  // Internal function, shouldn't be used, does the after conn stuff
  MVREXPORT void madeConnection(void);

  /// Internal function that gets whether the laser is simulated or not (just for the old MvrSick)
  MVREXPORT bool sickGetIsUsingSim(void);

  /// Internal function that sets whether the laser is simulated or not (just for the old MvrSick)
  MVREXPORT void sickSetIsUsingSim(bool usingSim);

  /// internal function to runOnRobot so that MvrSick can do that while this class won't
  MVREXPORT bool internalRunOnRobot(void);

  /// Finishes getting the unset parameters from the robot then
  /// setting some internal vmvriables that need it
  bool finishParams(void);

  MVREXPORT virtual bool laserCheckParams(void);

  MVREXPORT virtual void laserSetName(const char *name);

  enum State {
    STATE_NONE, ///< Nothing, haven't tried to connect or anything
    STATE_INIT, ///< Initializing the laser
    STATE_WAIT_FOR_POWER_ON, ///< Waiting for power on
    STATE_CHANGE_BAUD, ///< Change the baud, no confirm here
    STATE_CONFIGURE, ///< Send the width and increment to the laser
    STATE_WAIT_FOR_CONFIGURE_ACK, ///< Wait for the configuration Ack
    STATE_INSTALL_MODE, ///< Switch to install mode
    STATE_WAIT_FOR_INSTALL_MODE_ACK, ///< Wait until its switched to install mode
    STATE_SET_MODE, ///< Set the mode (mm/cm) and extra field bits
    STATE_WAIT_FOR_SET_MODE_ACK, ///< Waiting for set-mode ack
    STATE_START_READINGS, ///< Switch to monitoring mode
    STATE_WAIT_FOR_START_ACK, ///< Waiting for the switch-mode ack
    STATE_CONNECTED ///< We're connected and getting readings
  };
  /// Internal function for switching states
  MVREXPORT void switchState(State state);
  State myState;
  MvrTime myStateStart;
  MvrFunctorC<MvrLMS2xx> myRobotConnectCB;
  MvrRetFunctor1C<bool, MvrLMS2xx, MvrRobotPacket *> mySimPacketHandler;
  MvrFunctorC<MvrLMS2xx> mySensorInterpCB;
  std::list<MvrSensorReading *>::iterator myIter;
  bool myStartConnect;
  bool myRunningOnRobot;

  // range buffers to hold current range set and assembling range set
  std::list<MvrSensorReading *> *myAssembleReadings;
  std::list<MvrSensorReading *> *myCurrentReadings;

  bool myProcessImmediately;
  bool myInterpolation;
  // list of packets, so we can process them from the sensor callback
  std::list<MvrLMS2xxPacket *> myPackets;

  // these two are just for the sim packets
  unsigned int myWhichReading;
  unsigned int myTotalNumReadings;

  // some vmvriables so we don't have to do a tedios if every time
  double myOffsetAmount;
  double myIncrementAmount;

  // packet stuff
  MvrLMS2xxPacket myPacket;
  bool myUseSim;
  
  int myNumReflectorBits;
  bool myInterlaced;

  // stuff for the sim packet
  MvrPose mySimPacketStart;
  MvrTransform mySimPacketTrans;
  MvrTransform mySimPacketEncoderTrans;
  unsigned int mySimPacketCounter;

  // connection
  MvrLMS2xxPacketReceiver myLMS2xxPacketReceiver;

  MvrMutex myStateMutex;
  MvrRetFunctorC<bool, MvrLMS2xx> myMvrExitCB;
};


#endif 
