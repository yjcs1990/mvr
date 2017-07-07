#ifndef MVRROBOTJOYHANDLER_H
#define MVRROBOTJOYHANDLER_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

class MvrRobot;
class MvrRobotPacket;

/// Interfaces to a joystick on the robot's microcontroller
/** 
    This is largely meant to be about the same as the normal joy
    handler but gets the data back from the robot about the joystick,
    but this sameness is why it reports things as it does.

    Also note that x is usually rotational velocity (since it right/left),
    whereas Y is translational (since it is up/down).

  When created, this class requests continuous joystick packets from the robot if already
  connected, or if not, requests joystick packets upon robot connection.  In
  requests that the joystick data be stopped upon normal robot disconnection, ARIA
  program exit, or when this object is destroyed.

  @ingroup OptionalClasses
**/
class MvrRobotJoyHandler
{
 public:
  /// Constructor
  MVREXPORT MvrRobotJoyHandler(MvrRobot *robot);
  /// Destructor
  MVREXPORT ~MvrRobotJoyHandler();
  /// Gets the adjusted reading, as floats
  MVREXPORT void getDoubles(double *x, double *y, double *z);
  /// Gets the first button 
  bool getButton1(void) { return myButton1; }
  /// Gets the second button 
  bool getButton2(void) { return myButton2; }
  /// Gets the time we last got information back
  MVREXPORT MvrTime getDataReceivedTime(void) { return myDataReceived; }
  /// If we've ever gotten a packet back
  MVREXPORT bool gotData(void) { return myGotData; }
  /// Adds to a section in a config
  MVREXPORT void addToConfig(MvrConfig *config, const char *section);
  /// Gets the X value (only use for information, or with the robot locked, getDoubles is preferred)
  int getRawX(void) { return myRawX; }
  /// Gets the Y value (only use for information, or with the robot locked, getDoubles is preferred)
  int getRawY(void) { return myRawY; }
  /// Gets the throttle value (only use for information, or with the robot locked, getDoubles is preferred)
  int getRawThrottle(void) { return myRawThrottle; }

 protected:
  MVREXPORT bool handleJoystickPacket(MvrRobotPacket *packet);
  MVREXPORT void connectCallback(void);

  MvrRobot *myRobot;
  MvrTime myDataReceived;
  bool myButton1;
  bool myButton2;
  double myJoyX;
  double myJoyY;
  double myThrottle;
  bool myGotData;

  int myJoyXCenter;
  int myJoyYCenter;

  int myRawX;
  int myRawY;
  int myRawThrottle;

  MvrTime myStarted;
  MvrRetFunctor1C<bool, MvrRobotJoyHandler,
      MvrRobotPacket *> myHandleJoystickPacketCB;
  MvrFunctorC<MvrRobotJoyHandler> myConnectCB;

  MvrFunctorC<MvrRobotJoyHandler> myStopPacketsCB;

  void stopPackets();

};


#endif // ARJOYHANDLER_H

