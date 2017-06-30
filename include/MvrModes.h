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
  MVREXPORT MvrModeTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeTeleop();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void userTask(void);
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
  MVREXPORT MvrModeUnguardedTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeUnguardedTeleop();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void userTask(void);
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
  MVREXPORT MvrModeWander(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeWander();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void userTask(void);
protected:
  MvrActionGroupWander myGroup;
};

/// Mode for controlling the gripper
class MvrModeGripper : public MvrMode
{
public:
  /// Constructor
  MVREXPORT MvrModeGripper(MvrRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeGripper();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void help(void);
  MVREXPORT void open(void);
  MVREXPORT void close(void);
  MVREXPORT void up(void);
  MVREXPORT void down(void);
  MVREXPORT void stop(void);
  MVREXPORT void exercise(void);
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
  MVREXPORT MvrModeCamera(MvrRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeCamera();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void help(void);
  MVREXPORT void up(void);
  MVREXPORT void down(void);
  MVREXPORT void left(void);
  MVREXPORT void right(void);
  MVREXPORT void center(void);
  MVREXPORT void zoomIn(void);
  MVREXPORT void zoomOut(void);
  MVREXPORT void exercise(void);
  MVREXPORT void toggleAutoFocus();
  MVREXPORT void sony(void);
  MVREXPORT void canon(void);
  MVREXPORT void dpptu(void);
  MVREXPORT void amptu(void);
  MVREXPORT void canonInverted(void);
  MVREXPORT void sonySerial(void);
  MVREXPORT void canonSerial(void);
  MVREXPORT void dpptuSerial(void);
  MVREXPORT void amptuSerial(void);
  MVREXPORT void canonInvertedSerial(void);
  MVREXPORT void rvisionSerial(void);
  MVREXPORT void com1(void);
  MVREXPORT void com2(void);
  MVREXPORT void com3(void);
  MVREXPORT void com4(void);
  MVREXPORT void usb0(void);
  MVREXPORT void usb9(void);
  MVREXPORT void aux1(void);
  MVREXPORT void aux2(void);
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
  MVREXPORT MvrModeSonar(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeSonar();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void help(void);
  MVREXPORT void allSonar(void);
  MVREXPORT void firstSonar(void);
  MVREXPORT void secondSonar(void);
  MVREXPORT void thirdSonar(void);
  MVREXPORT void fourthSonar(void);
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
  MVREXPORT MvrModeBumps(MvrRobot *robot, const char *name, char key, char key2);
  MVREXPORT ~MvrModeBumps();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void help(void);
};

class MvrModePosition : public MvrMode
{
public:
  MVREXPORT MvrModePosition(MvrRobot *robot, const char *name, char key,
			  char key2, MvrAnalogGyro *gyro = NULL);
  MVREXPORT ~MvrModePosition();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void help(void);
  MVREXPORT void up(void);
  MVREXPORT void down(void);
  MVREXPORT void left(void);
  MVREXPORT void right(void);
  MVREXPORT void stop(void);
  MVREXPORT void reset(void);
  MVREXPORT void mode(void);
  MVREXPORT void gyro(void);
  MVREXPORT void incDistance(void);
  MVREXPORT void decDistance(void);
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
  MVREXPORT MvrModeIO(MvrRobot *robot, const char *name, char key,
			  char key2);
  MVREXPORT ~MvrModeIO();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void help(void);
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
  MVREXPORT MvrModeLaser(MvrRobot *robot, const char *name, char key,
		       char key2, MvrSick *obsolete = NULL);
  MVREXPORT ~MvrModeLaser();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void switchToLaser(int laserNumber);

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
  MVREXPORT MvrModeActs(MvrRobot *robot, const char *name, char key, char key2,
		      MvrACTS_1_2 *acts = NULL);
  /// Destructor
  MVREXPORT virtual ~MvrModeActs();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void userTask(void);
  MVREXPORT virtual void channel1(void);
  MVREXPORT virtual void channel2(void);
  MVREXPORT virtual void channel3(void);
  MVREXPORT virtual void channel4(void);
  MVREXPORT virtual void channel5(void);
  MVREXPORT virtual void channel6(void);
  MVREXPORT virtual void channel7(void);
  MVREXPORT virtual void channel8(void);
  MVREXPORT virtual void stop(void);
  MVREXPORT virtual void start(void);
  MVREXPORT virtual void toggleAcquire(void);
  
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
  MVREXPORT MvrModeCommand(MvrRobot *robot, const char *name, char key,
			  char key2);
  MVREXPORT ~MvrModeCommand();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
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
  MVREXPORT MvrModeTCM2(MvrRobot *robot, const char *name, char key, char key2, MvrTCM2 *tcm2 = NULL);
  /// Destructor
  MVREXPORT virtual ~MvrModeTCM2();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void userTask(void);
  
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
  MVREXPORT MvrModeConfig(MvrRobot *robot, const char *name, char key, char key2);
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  
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
  MVREXPORT MvrModeRobotStatus(MvrRobot *robot, const char *name, char key, char key2);
  MVREXPORT virtual void activate();
  MVREXPORT virtual void deactivate();
  MVREXPORT virtual void help();
  MVREXPORT virtual void userTask();
  
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
