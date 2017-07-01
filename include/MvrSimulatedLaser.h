#ifndef MVRSIMULATEDLASER_H
#define MVRSIMULATEDLASER_H

#include "mvriaTypedefs.h"
#include "MvrLaser.h"

class MvrRobot;
class MvrRobotPacket;

/**
   This class is a subclass of MvrRangeDeviceThreaded meant for any
   planar scanning lasers, like the SICK lasers, Hokoyo URG series
   lasers, etc.  Unlike most base classes this contains the superset
   of everything that may need to be configured on any of the sensors,
   this is so that the configuration and parameter files don't have to
   deal with anything sensor specific.

   To see the different things you can set on a laser, call the
   functions canSetDegrees, canChooseRange, canSetIncrement,
   canChooseIncrement, canChooseUnits, canChooseReflectorBits,
   canSetPowerControlled, canChooseStartBaud, and canChooseAutoBaud to
   see what is available (the help for each of those tells you what
   functions they are associated with, and for each function
   associated with one of those it tells you to see the associated
   function).

   @since 2.7.0
**/

class MvrSimulatedLaser : public MvrLaser
{
public:
  /// Constructor
  MVREXPORT MvrSimulatedLaser(MvrLaser *laser);
  /// Destructor
  MVREXPORT virtual ~MvrSimulatedLaser();

  MVREXPORT virtual bool blockingConnect(void);
  MVREXPORT virtual bool asyncConnect(void);
  MVREXPORT virtual bool disconnect(void);
  MVREXPORT virtual bool isConnected(void)
    { return myIsConnected; }
  MVREXPORT virtual bool isTryingToConnect(void)
    { 
      if (myStartConnect)
	return true;
      else if (myTryingToConnect)
	return true; 
      else
	return false;
    }  

protected:
  MVREXPORT virtual void * runThread(void *arg);
  MVREXPORT virtual bool laserCheckParams(void);
  MVREXPORT bool finishParams(void);
  MVREXPORT bool simPacketHandler(MvrRobotPacket *packet);
  MvrLaser *myLaser;

  double mySimBegin;
  double mySimEnd;
  double mySimIncrement;

  // stuff for the sim packet
  MvrPose mySimPacketStart;
  MvrTransform mySimPacketTrans;
  MvrTransform mySimPacketEncoderTrans;
  unsigned int mySimPacketCounter;
  unsigned int myWhichReading;
  unsigned int myTotalNumReadings;

  bool myStartConnect;
  bool myIsConnected;
  bool myTryingToConnect;
  bool myReceivedData;

  std::list<MvrSensorReading *>::iterator myIter;
  // range buffers to hold current range set and assembling range set
  std::list<MvrSensorReading *> *myAssembleReadings;
  std::list<MvrSensorReading *> *myCurrentReadings;

  MvrRetFunctor1C<bool, MvrSimulatedLaser, MvrRobotPacket *> mySimPacketHandler;
};

#endif // MVRSIMULATEDLASER_H
