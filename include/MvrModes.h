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
#ifndef ARMODES_H
#define ARMODES_H

#include "ariaTypedefs.h"
#include "MvrMode.h"
#include "MvrActionGroups.h"
#include "MvrGripper.h"
#include "MvrTcpConnection.h"
#include "MvrSerialConnection.h"
#include "MvrPTZ.h"
#include "MvrTCMCompassRobot.h"
#include "MvrRobotConfigPacketReader.h"

class MvrTCMCompassRobot;
class MvrACTS_1_2;
class MvrRobotPacket;
class MvrSick;
class MvrAnalogGyro;

/// Mode for teleoping the robot with joystick + keyboard
class MvrModeTeleop : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeTeleop();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
protected:
  //ArActionGroupTeleop myGroup;
  // use our new ratio drive instead
  MvrActionGroupRatioDrive myGroup;
  MvrFunctorC<ArRobot> myEnableMotorsCB;
};

/// Mode for teleoping the robot with joystick + keyboard
class MvrModeUnguardedTeleop : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeUnguardedTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeUnguardedTeleop();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
protected:
  //ArActionGroupUnguardedTeleop myGroup;
  // use our new ratio drive instead
  MvrActionGroupRatioDriveUnsafe myGroup;
  MvrFunctorC<ArRobot> myEnableMotorsCB;
};

/// Mode for wandering around
class MvrModeWander : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeWander(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeWander();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
protected:
  MvrActionGroupWander myGroup;
};

/// Mode for controlling the gripper
class MvrModeGripper : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeGripper(MvrRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  AREXPORT virtual ~ArModeGripper();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void open(void);
  AREXPORT void close(void);
  AREXPORT void up(void);
  AREXPORT void down(void);
  AREXPORT void stop(void);
  AREXPORT void exercise(void);
protected:
  enum ExerState {
    UP_OPEN,
    UP_CLOSE,
    DOWN_CLOSE,
    DOWN_OPEN
  };
  MvrGripper myGripper;
  bool myExercising;
  ExerState myExerState;
  MvrTime myLastExer;
  MvrFunctorC<ArModeGripper> myOpenCB;
  MvrFunctorC<ArModeGripper> myCloseCB;
  MvrFunctorC<ArModeGripper> myUpCB;
  MvrFunctorC<ArModeGripper> myDownCB;
  MvrFunctorC<ArModeGripper> myStopCB;
  MvrFunctorC<ArModeGripper> myExerciseCB;
  
};

/// Mode for controlling the camera
class MvrModeCamera : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeCamera(MvrRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  AREXPORT virtual ~ArModeCamera();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void up(void);
  AREXPORT void down(void);
  AREXPORT void left(void);
  AREXPORT void right(void);
  AREXPORT void center(void);
  AREXPORT void zoomIn(void);
  AREXPORT void zoomOut(void);
  AREXPORT void exercise(void);
  AREXPORT void toggleAutoFocus();
  AREXPORT void sony(void);
  AREXPORT void canon(void);
  AREXPORT void dpptu(void);
  AREXPORT void amptu(void);
  AREXPORT void canonInverted(void);
  AREXPORT void sonySerial(void);
  AREXPORT void canonSerial(void);
  AREXPORT void dpptuSerial(void);
  AREXPORT void amptuSerial(void);
  AREXPORT void canonInvertedSerial(void);
  AREXPORT void rvisionSerial(void);
  AREXPORT void com1(void);
  AREXPORT void com2(void);
  AREXPORT void com3(void);
  AREXPORT void com4(void);
  AREXPORT void usb0(void);
  AREXPORT void usb9(void);
  AREXPORT void aux1(void);
  AREXPORT void aux2(void);
protected:
  void takeCameraKeys(void);
  void giveUpCameraKeys(void);
  void helpCameraKeys(void);
  void takePortKeys(void);
  void giveUpPortKeys(void);
  void helpPortKeys(void);
  void takeAuxKeys(void);
  void giveUpAuxKeys(void);
  void helpAuxKeys(void);
  void takeMovementKeys(void);
  void giveUpMovementKeys(void);
  void helpMovementKeys(void);
  enum State {
    STATE_CAMERA,
    STATE_PORT,
    STATE_MOVEMENT
  };
  void cameraToMovement(void);
  void cameraToPort(void);
  void cameraToAux(void);
  void portToMovement(void);
  void auxToMovement(void);
  enum ExerState {
    CENTER,
    UP_LEFT,
    UP_RIGHT,
    DOWN_RIGHT,
    DOWN_LEFT
  };

  bool myExercising;
  State myState;
  ExerState myExerState;
  MvrTime myLastExer;
  bool myExerZoomedIn;
  MvrTime myLastExerZoomed;
  MvrSerialConnection myConn;
  MvrPTZ *myCam;
  MvrFunctorC<ArModeCamera> myUpCB;
  MvrFunctorC<ArModeCamera> myDownCB;
  MvrFunctorC<ArModeCamera> myLeftCB;
  MvrFunctorC<ArModeCamera> myRightCB;
  MvrFunctorC<ArModeCamera> myCenterCB;
  MvrFunctorC<ArModeCamera> myZoomInCB;
  MvrFunctorC<ArModeCamera> myZoomOutCB;
  MvrFunctorC<ArModeCamera> myExerciseCB;
  MvrFunctorC<ArModeCamera> mySonyCB;
  MvrFunctorC<ArModeCamera> myCanonCB;
  MvrFunctorC<ArModeCamera> myDpptuCB;
  MvrFunctorC<ArModeCamera> myAmptuCB;
  MvrFunctorC<ArModeCamera> myCanonInvertedCB;
  MvrFunctorC<ArModeCamera> mySonySerialCB;
  MvrFunctorC<ArModeCamera> myCanonSerialCB;
  MvrFunctorC<ArModeCamera> myDpptuSerialCB;
  MvrFunctorC<ArModeCamera> myAmptuSerialCB;
  MvrFunctorC<ArModeCamera> myCanonInvertedSerialCB;
  MvrFunctorC<ArModeCamera> myRVisionSerialCB;
  MvrFunctorC<ArModeCamera> myCom1CB;
  MvrFunctorC<ArModeCamera> myCom2CB;
  MvrFunctorC<ArModeCamera> myCom3CB;
  MvrFunctorC<ArModeCamera> myCom4CB;
  MvrFunctorC<ArModeCamera> myUSBCom0CB;
  MvrFunctorC<ArModeCamera> myUSBCom9CB;
  MvrFunctorC<ArModeCamera> myAux1CB;
  MvrFunctorC<ArModeCamera> myAux2CB;
  const int myPanAmount;
  const int myTiltAmount;
  bool myAutoFocusOn;
  MvrFunctorC<ArModeCamera> myToggleAutoFocusCB;
};

/// Mode for displaying the sonar
class MvrModeSonar : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeSonar(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  AREXPORT virtual ~ArModeSonar();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void allSonar(void);
  AREXPORT void firstSonar(void);
  AREXPORT void secondSonar(void);
  AREXPORT void thirdSonar(void);
  AREXPORT void fourthSonar(void);
protected:
  enum State 
  {
    STATE_ALL,
    STATE_FIRST,
    STATE_SECOND,
    STATE_THIRD,
    STATE_FOURTH
  };
  State myState;
  MvrFunctorC<ArModeSonar> myAllSonarCB;
  MvrFunctorC<ArModeSonar> myFirstSonarCB;
  MvrFunctorC<ArModeSonar> mySecondSonarCB;
  MvrFunctorC<ArModeSonar> myThirdSonarCB;
  MvrFunctorC<ArModeSonar> myFourthSonarCB;
};

class MvrModeBumps : public MvrMode
{
public:
  AREXPORT MvrModeBumps(MvrRobot *robot, const char *name, char key, char key2);
  AREXPORT ~ArModeBumps();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
};

class MvrModePosition : public MvrMode
{
public:
  AREXPORT MvrModePosition(MvrRobot *robot, const char *name, char key,
			  char key2, MvrAnalogGyro *gyro = NULL);
  AREXPORT ~ArModePosition();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void up(void);
  AREXPORT void down(void);
  AREXPORT void left(void);
  AREXPORT void right(void);
  AREXPORT void stop(void);
  AREXPORT void reset(void);
  AREXPORT void mode(void);
  AREXPORT void gyro(void);
  AREXPORT void incDistance(void);
  AREXPORT void decDistance(void);
protected:
  enum Mode { MODE_BOTH, MODE_EITHER };
  MvrAnalogGyro *myGyro;
  double myGyroZero;
  double myRobotZero;
  Mode myMode;
  std::string myModeString;
  bool myInHeadingMode;
  double myHeading;
  double myDistance;
  MvrFunctorC<ArModePosition> myUpCB;
  MvrFunctorC<ArModePosition> myDownCB;
  MvrFunctorC<ArModePosition> myLeftCB;
  MvrFunctorC<ArModePosition> myRightCB;
  MvrFunctorC<ArModePosition> myStopCB;  
  MvrFunctorC<ArModePosition> myResetCB;  
  MvrFunctorC<ArModePosition> myModeCB;
  MvrFunctorC<ArModePosition> myGyroCB;
  MvrFunctorC<ArModePosition> myIncDistCB;
  MvrFunctorC<ArModePosition> myDecDistCB;
};

class MvrModeIO : public MvrMode
{
public:
  AREXPORT MvrModeIO(MvrRobot *robot, const char *name, char key,
			  char key2);
  AREXPORT ~ArModeIO();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
protected:
  bool myExplanationReady;
  bool myExplained;
  MvrTime myLastPacketTime;
  char myExplanation[1024];
  char myOutput[1024];
  MvrFunctorC<ArModeIO> myProcessIOCB;
};

class MvrModeLaser : public MvrMode
{
public:
  AREXPORT MvrModeLaser(MvrRobot *robot, const char *name, char key,
		       char key2, MvrSick *obsolete = NULL);
  AREXPORT ~ArModeLaser();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void switchToLaser(int laserNumber);

protected:
  void togMiddle(void);
  void togConnect(void);

  enum State {
    STATE_UNINITED,
    STATE_CONNECTING,
    STATE_CONNECTED
  };
  
  State myState;
  MvrLaser *myLaser;
  int myLaserNumber;

  bool myPrintMiddle;

  MvrFunctorC<ArModeLaser> myTogMiddleCB;

  std::map<int, MvrLaser *> myLasers;
  std::map<int, MvrFunctor1C<ArModeLaser, int> *> myLaserCallbacks;
};

/// Mode for following a color blob using ACTS
class MvrModeActs : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeActs(MvrRobot *robot, const char *name, char key, char key2,
		      MvrACTS_1_2 *acts = NULL);
  /// Destructor
  AREXPORT virtual ~ArModeActs();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void channel1(void);
  AREXPORT virtual void channel2(void);
  AREXPORT virtual void channel3(void);
  AREXPORT virtual void channel4(void);
  AREXPORT virtual void channel5(void);
  AREXPORT virtual void channel6(void);
  AREXPORT virtual void channel7(void);
  AREXPORT virtual void channel8(void);
  AREXPORT virtual void stop(void);
  AREXPORT virtual void start(void);
  AREXPORT virtual void toggleAcquire(void);
  
protected:
  MvrActionGroupColorFollow *myGroup;
  MvrPTZ *camera;
  MvrACTS_1_2 *myActs;
  MvrRobot *myRobot;

  MvrFunctorC<ArModeActs> myChannel1CB;
  MvrFunctorC<ArModeActs> myChannel2CB;
  MvrFunctorC<ArModeActs> myChannel3CB;
  MvrFunctorC<ArModeActs> myChannel4CB;
  MvrFunctorC<ArModeActs> myChannel5CB;
  MvrFunctorC<ArModeActs> myChannel6CB;
  MvrFunctorC<ArModeActs> myChannel7CB;
  MvrFunctorC<ArModeActs> myChannel8CB;
  MvrFunctorC<ArModeActs> myStopCB;
  MvrFunctorC<ArModeActs> myStartCB;
  MvrFunctorC<ArModeActs> myToggleAcquireCB;
};

class MvrModeCommand : public MvrMode
{
public:
  AREXPORT MvrModeCommand(MvrRobot *robot, const char *name, char key,
			  char key2);
  AREXPORT ~ArModeCommand();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
protected:
  void takeKeys(void);
  void giveUpKeys(void);
  void addChar(int ch);
  void finishParsing(void);
  void reset(bool print = true);
  char myCommandString[70];
  MvrFunctor1C<ArModeCommand, int> my0CB;
  MvrFunctor1C<ArModeCommand, int> my1CB;
  MvrFunctor1C<ArModeCommand, int> my2CB;
  MvrFunctor1C<ArModeCommand, int> my3CB;
  MvrFunctor1C<ArModeCommand, int> my4CB;
  MvrFunctor1C<ArModeCommand, int> my5CB;
  MvrFunctor1C<ArModeCommand, int> my6CB;
  MvrFunctor1C<ArModeCommand, int> my7CB;
  MvrFunctor1C<ArModeCommand, int> my8CB;
  MvrFunctor1C<ArModeCommand, int> my9CB;
  MvrFunctor1C<ArModeCommand, int> myMinusCB;
  MvrFunctor1C<ArModeCommand, int> myBackspaceCB;
  MvrFunctor1C<ArModeCommand, int> mySpaceCB;
  MvrFunctorC<ArModeCommand> myEnterCB;

};

/// Mode for following a color blob using ACTS
class MvrModeTCM2 : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeTCM2(MvrRobot *robot, const char *name, char key, char key2, MvrTCM2 *tcm2 = NULL);
  /// Destructor
  AREXPORT virtual ~ArModeTCM2();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  AREXPORT virtual void userTask(void);
  
protected:
  void init();
  MvrTCM2 *myTCM2;
  MvrCompassConnector *connector;
  MvrRobot *myRobot;
  MvrFunctorC<ArTCM2> *myOffCB;
  MvrFunctorC<ArTCM2> *myCompassCB;
  MvrFunctorC<ArTCM2> *myOnePacketCB;
  MvrFunctorC<ArTCM2> *myContinuousPacketsCB;
  MvrFunctorC<ArTCM2> *myUserCalibrationCB;
  MvrFunctorC<ArTCM2> *myAutoCalibrationCB;
  MvrFunctorC<ArTCM2> *myStopCalibrationCB;
  MvrFunctorC<ArTCM2> *myResetCB;

};


/// Mode for requesting config packet
class MvrModeConfig : public MvrMode
{
public:
  /// Constructor
  AREXPORT MvrModeConfig(MvrRobot *robot, const char *name, char key, char key2);
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void help(void);
  
protected:
  MvrRobot *myRobot;
  MvrRobotConfigPacketReader myConfigPacketReader;
  MvrFunctorC<ArModeConfig> myGotConfigPacketCB;

  void gotConfigPacket();
};



/// Mode for displaying status and diagnostic info
class MvrModeRobotStatus : public MvrMode
{
public:
  AREXPORT MvrModeRobotStatus(MvrRobot *robot, const char *name, char key, char key2);
  AREXPORT virtual void activate();
  AREXPORT virtual void deactivate();
  AREXPORT virtual void help();
  AREXPORT virtual void userTask();
  
protected:
  MvrRobot *myRobot;
  MvrRetFunctor1C<bool, MvrModeRobotStatus, MvrRobotPacket*> myDebugMessageCB;
  MvrRetFunctor1C<bool, MvrModeRobotStatus, MvrRobotPacket*> mySafetyStateCB;
  MvrRetFunctor1C<bool, MvrModeRobotStatus, MvrRobotPacket*> mySafetyWarningCB;

  void printFlags();
  void printFlagsHeader();

  std::string byte_as_bitstring(unsigned char byte);
  std::string int16_as_bitstring(MvrTypes::Byte2 n);
  std::string int32_as_bitstring(MvrTypes::Byte4 n);

  bool handleDebugMessage(MvrRobotPacket *p);
  bool handleSafetyStatePacket(MvrRobotPacket *p);
  const char *safetyStateName(int state);
  bool handleSafetyWarningPacket(MvrRobotPacket *p);

};


#endif // ARMODES_H
