/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrModes.h
 > Description  : A class for different modes, mostly as related to keyboard input
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRMODES_H
#define MVRMODES_H

#include "mvriaTypedefs.h"
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
  MVREXPOR MvrModeTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPOR virtual ~MvrModeTeleop();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void help(void);
  MVREXPOR virtual void userTask(void);
protected:
  //MvrActionGroupTeleop myGroup;
  // use our new ratio drive instead
  MvrActionGroupRatioDrive myGroup;
  MvrFunctorC<MvrRobot> myEnableMotorsCB;
};

/// Mode for teleoping the robot with joystick + keyboard
class MvrModeUnguardedTeleop : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeUnguardedTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPOR virtual ~MvrModeUnguardedTeleop();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void help(void);
  MVREXPOR virtual void userTask(void);
protected:
  //MvrActionGroupUnguardedTeleop myGroup;
  // use our new ratio drive instead
  MvrActionGroupRatioDriveUnsafe myGroup;
  MvrFunctorC<MvrRobot> myEnableMotorsCB;
};

/// Mode for wandering around
class MvrModeWander : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeWander(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPOR virtual ~MvrModeWander();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void help(void);
  MVREXPOR virtual void userTask(void);
protected:
  MvrActionGroupWander myGroup;
};

/// Mode for controlling the gripper
class MvrModeGripper : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeGripper(MvrRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  MVREXPOR virtual ~MvrModeGripper();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void help(void);
  MVREXPOR void open(void);
  MVREXPOR void close(void);
  MVREXPOR void up(void);
  MVREXPOR void down(void);
  MVREXPOR void stop(void);
  MVREXPOR void exercise(void);
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
  MvrFunctorC<MvrModeGripper> myOpenCB;
  MvrFunctorC<MvrModeGripper> myCloseCB;
  MvrFunctorC<MvrModeGripper> myUpCB;
  MvrFunctorC<MvrModeGripper> myDownCB;
  MvrFunctorC<MvrModeGripper> myStopCB;
  MvrFunctorC<MvrModeGripper> myExerciseCB;
  
};

/// Mode for controlling the camera
class MvrModeCamera : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeCamera(MvrRobot *robot, const char *name, char key,
			 char key2);
  /// Destructor
  MVREXPOR virtual ~MvrModeCamera();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void help(void);
  MVREXPOR void up(void);
  MVREXPOR void down(void);
  MVREXPOR void left(void);
  MVREXPOR void right(void);
  MVREXPOR void center(void);
  MVREXPOR void zoomIn(void);
  MVREXPOR void zoomOut(void);
  MVREXPOR void exercise(void);
  MVREXPOR void toggleAutoFocus();
  MVREXPOR void sony(void);
  MVREXPOR void canon(void);
  MVREXPOR void dpptu(void);
  MVREXPOR void amptu(void);
  MVREXPOR void canonInverted(void);
  MVREXPOR void sonySerial(void);
  MVREXPOR void canonSerial(void);
  MVREXPOR void dpptuSerial(void);
  MVREXPOR void amptuSerial(void);
  MVREXPOR void canonInvertedSerial(void);
  MVREXPOR void rvisionSerial(void);
  MVREXPOR void com1(void);
  MVREXPOR void com2(void);
  MVREXPOR void com3(void);
  MVREXPOR void com4(void);
  MVREXPOR void usb0(void);
  MVREXPOR void usb9(void);
  MVREXPOR void aux1(void);
  MVREXPOR void aux2(void);
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
  MvrFunctorC<MvrModeCamera> myUpCB;
  MvrFunctorC<MvrModeCamera> myDownCB;
  MvrFunctorC<MvrModeCamera> myLeftCB;
  MvrFunctorC<MvrModeCamera> myRightCB;
  MvrFunctorC<MvrModeCamera> myCenterCB;
  MvrFunctorC<MvrModeCamera> myZoomInCB;
  MvrFunctorC<MvrModeCamera> myZoomOutCB;
  MvrFunctorC<MvrModeCamera> myExerciseCB;
  MvrFunctorC<MvrModeCamera> mySonyCB;
  MvrFunctorC<MvrModeCamera> myCanonCB;
  MvrFunctorC<MvrModeCamera> myDpptuCB;
  MvrFunctorC<MvrModeCamera> myAmptuCB;
  MvrFunctorC<MvrModeCamera> myCanonInvertedCB;
  MvrFunctorC<MvrModeCamera> mySonySerialCB;
  MvrFunctorC<MvrModeCamera> myCanonSerialCB;
  MvrFunctorC<MvrModeCamera> myDpptuSerialCB;
  MvrFunctorC<MvrModeCamera> myAmptuSerialCB;
  MvrFunctorC<MvrModeCamera> myCanonInvertedSerialCB;
  MvrFunctorC<MvrModeCamera> myRVisionSerialCB;
  MvrFunctorC<MvrModeCamera> myCom1CB;
  MvrFunctorC<MvrModeCamera> myCom2CB;
  MvrFunctorC<MvrModeCamera> myCom3CB;
  MvrFunctorC<MvrModeCamera> myCom4CB;
  MvrFunctorC<MvrModeCamera> myUSBCom0CB;
  MvrFunctorC<MvrModeCamera> myUSBCom9CB;
  MvrFunctorC<MvrModeCamera> myAux1CB;
  MvrFunctorC<MvrModeCamera> myAux2CB;
  const int myPanAmount;
  const int myTiltAmount;
  bool myAutoFocusOn;
  MvrFunctorC<MvrModeCamera> myToggleAutoFocusCB;
};

/// Mode for displaying the sonar
class MvrModeSonar : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeSonar(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPOR virtual ~MvrModeSonar();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void help(void);
  MVREXPOR void allSonar(void);
  MVREXPOR void firstSonar(void);
  MVREXPOR void secondSonar(void);
  MVREXPOR void thirdSonar(void);
  MVREXPOR void fourthSonar(void);
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
  MvrFunctorC<MvrModeSonar> myAllSonarCB;
  MvrFunctorC<MvrModeSonar> myFirstSonarCB;
  MvrFunctorC<MvrModeSonar> mySecondSonarCB;
  MvrFunctorC<MvrModeSonar> myThirdSonarCB;
  MvrFunctorC<MvrModeSonar> myFourthSonarCB;
};

class MvrModeBumps : public MvrMode
{
public:
  MVREXPOR MvrModeBumps(MvrRobot *robot, const char *name, char key, char key2);
  MVREXPOR ~MvrModeBumps();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void help(void);
};

class MvrModePosition : public MvrMode
{
public:
  MVREXPOR MvrModePosition(MvrRobot *robot, const char *name, char key,
			  char key2, MvrAnalogGyro *gyro = NULL);
  MVREXPOR ~MvrModePosition();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void help(void);
  MVREXPOR void up(void);
  MVREXPOR void down(void);
  MVREXPOR void left(void);
  MVREXPOR void right(void);
  MVREXPOR void stop(void);
  MVREXPOR void reset(void);
  MVREXPOR void mode(void);
  MVREXPOR void gyro(void);
  MVREXPOR void incDistance(void);
  MVREXPOR void decDistance(void);
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
  MvrFunctorC<MvrModePosition> myUpCB;
  MvrFunctorC<MvrModePosition> myDownCB;
  MvrFunctorC<MvrModePosition> myLeftCB;
  MvrFunctorC<MvrModePosition> myRightCB;
  MvrFunctorC<MvrModePosition> myStopCB;  
  MvrFunctorC<MvrModePosition> myResetCB;  
  MvrFunctorC<MvrModePosition> myModeCB;
  MvrFunctorC<MvrModePosition> myGyroCB;
  MvrFunctorC<MvrModePosition> myIncDistCB;
  MvrFunctorC<MvrModePosition> myDecDistCB;
};

class MvrModeIO : public MvrMode
{
public:
  MVREXPOR MvrModeIO(MvrRobot *robot, const char *name, char key,
			  char key2);
  MVREXPOR ~MvrModeIO();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void help(void);
protected:
  bool myExplanationReady;
  bool myExplained;
  MvrTime myLastPacketTime;
  char myExplanation[1024];
  char myOutput[1024];
  MvrFunctorC<MvrModeIO> myProcessIOCB;
};

class MvrModeLaser : public MvrMode
{
public:
  MVREXPOR MvrModeLaser(MvrRobot *robot, const char *name, char key,
		       char key2, MvrSick *obsolete = NULL);
  MVREXPOR ~MvrModeLaser();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void help(void);
  MVREXPOR virtual void switchToLaser(int laserNumber);

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

  MvrFunctorC<MvrModeLaser> myTogMiddleCB;

  std::map<int, MvrLaser *> myLasers;
  std::map<int, MvrFunctor1C<MvrModeLaser, int> *> myLaserCallbacks;
};

/// Mode for following a color blob using ACTS
class MvrModeActs : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeActs(MvrRobot *robot, const char *name, char key, char key2,
		      MvrACTS_1_2 *acts = NULL);
  /// Destructor
  MVREXPOR virtual ~MvrModeActs();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void help(void);
  MVREXPOR virtual void userTask(void);
  MVREXPOR virtual void channel1(void);
  MVREXPOR virtual void channel2(void);
  MVREXPOR virtual void channel3(void);
  MVREXPOR virtual void channel4(void);
  MVREXPOR virtual void channel5(void);
  MVREXPOR virtual void channel6(void);
  MVREXPOR virtual void channel7(void);
  MVREXPOR virtual void channel8(void);
  MVREXPOR virtual void stop(void);
  MVREXPOR virtual void start(void);
  MVREXPOR virtual void toggleAcquire(void);
  
protected:
  MvrActionGroupColorFollow *myGroup;
  MvrPTZ *camera;
  MvrACTS_1_2 *myActs;
  MvrRobot *myRobot;

  MvrFunctorC<MvrModeActs> myChannel1CB;
  MvrFunctorC<MvrModeActs> myChannel2CB;
  MvrFunctorC<MvrModeActs> myChannel3CB;
  MvrFunctorC<MvrModeActs> myChannel4CB;
  MvrFunctorC<MvrModeActs> myChannel5CB;
  MvrFunctorC<MvrModeActs> myChannel6CB;
  MvrFunctorC<MvrModeActs> myChannel7CB;
  MvrFunctorC<MvrModeActs> myChannel8CB;
  MvrFunctorC<MvrModeActs> myStopCB;
  MvrFunctorC<MvrModeActs> myStartCB;
  MvrFunctorC<MvrModeActs> myToggleAcquireCB;
};

class MvrModeCommand : public MvrMode
{
public:
  MVREXPOR MvrModeCommand(MvrRobot *robot, const char *name, char key,
			  char key2);
  MVREXPOR ~MvrModeCommand();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void help(void);
protected:
  void takeKeys(void);
  void giveUpKeys(void);
  void addChar(int ch);
  void finishParsing(void);
  void reset(bool print = true);
  char myCommandString[70];
  MvrFunctor1C<MvrModeCommand, int> my0CB;
  MvrFunctor1C<MvrModeCommand, int> my1CB;
  MvrFunctor1C<MvrModeCommand, int> my2CB;
  MvrFunctor1C<MvrModeCommand, int> my3CB;
  MvrFunctor1C<MvrModeCommand, int> my4CB;
  MvrFunctor1C<MvrModeCommand, int> my5CB;
  MvrFunctor1C<MvrModeCommand, int> my6CB;
  MvrFunctor1C<MvrModeCommand, int> my7CB;
  MvrFunctor1C<MvrModeCommand, int> my8CB;
  MvrFunctor1C<MvrModeCommand, int> my9CB;
  MvrFunctor1C<MvrModeCommand, int> myMinusCB;
  MvrFunctor1C<MvrModeCommand, int> myBackspaceCB;
  MvrFunctor1C<MvrModeCommand, int> mySpaceCB;
  MvrFunctorC<MvrModeCommand> myEnterCB;

};

/// Mode for following a color blob using ACTS
class MvrModeTCM2 : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeTCM2(MvrRobot *robot, const char *name, char key, char key2, MvrTCM2 *tcm2 = NULL);
  /// Destructor
  MVREXPOR virtual ~MvrModeTCM2();
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void help(void);
  MVREXPOR virtual void userTask(void);
  
protected:
  void init();
  MvrTCM2 *myTCM2;
  MvrCompassConnector *connector;
  MvrRobot *myRobot;
  MvrFunctorC<MvrTCM2> *myOffCB;
  MvrFunctorC<MvrTCM2> *myCompassCB;
  MvrFunctorC<MvrTCM2> *myOnePacketCB;
  MvrFunctorC<MvrTCM2> *myContinuousPacketsCB;
  MvrFunctorC<MvrTCM2> *myUserCalibrationCB;
  MvrFunctorC<MvrTCM2> *myAutoCalibrationCB;
  MvrFunctorC<MvrTCM2> *myStopCalibrationCB;
  MvrFunctorC<MvrTCM2> *myResetCB;

};


/// Mode for requesting config packet
class MvrModeConfig : public MvrMode
{
public:
  /// Constructor
  MVREXPOR MvrModeConfig(MvrRobot *robot, const char *name, char key, char key2);
  MVREXPOR virtual void activate(void);
  MVREXPOR virtual void deactivate(void);
  MVREXPOR virtual void help(void);
  
protected:
  MvrRobot *myRobot;
  MvrRobotConfigPacketReader myConfigPacketReader;
  MvrFunctorC<MvrModeConfig> myGotConfigPacketCB;

  void gotConfigPacket();
};



/// Mode for displaying status and diagnostic info
class MvrModeRobotStatus : public MvrMode
{
public:
  MVREXPOR MvrModeRobotStatus(MvrRobot *robot, const char *name, char key, char key2);
  MVREXPOR virtual void activate();
  MVREXPOR virtual void deactivate();
  MVREXPOR virtual void help();
  MVREXPOR virtual void userTask();
  
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


#endif // MVRMODES_H
