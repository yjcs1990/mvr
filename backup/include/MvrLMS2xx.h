/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrLMS2xx.h
 > Description  : Interface to a SICK LMS-200 laser range device
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRLMS2XX_H
#define MVRLMS2XX_H


#include "mvriaTypedefs.h"
#include "MvrLMS2xxPacket.h"
#include "MvrLMS2xxPacketReceiver.h"
#include "MvrRobotPacket.h"
#include "MvrLaser.h"   
#include "MvrFunctor.h"
#include "MvrCondition.h"

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
  /// setting some internal variables that need it
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

  // some variables so we don't have to do a tedios if every time
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
  MvrRetFunctorC<bool, MvrLMS2xx> myMvriaExitCB;
};


#endif  // MVRLMS2XX_H