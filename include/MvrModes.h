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
  MVREXPORT MvrModeTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeTeleop();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void userTask(void);
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
  MVREXPORT MvrModeUnguardedTeleop(MvrRobot *robot, const char *name, char key, char key2);
  /// Destructor
  MVREXPORT virtual ~MvrModeUnguardedTeleop();
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  MVREXPORT virtual void userTask(void);
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
  MvrFunctorC<MvrModeSonar> myAllSonarCB;
  MvrFunctorC<MvrModeSonar> myFirstSonarCB;
  MvrFunctorC<MvrModeSonar> mySecondSonarCB;
  MvrFunctorC<MvrModeSonar> myThirdSonarCB;
  MvrFunctorC<MvrModeSonar> myFourthSonarCB;
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
  MvrFunctorC<MvrModeIO> myProcessIOCB;
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

  MvrFunctorC<MvrModeLaser> myTogMiddleCB;

  std::map<int, MvrLaser *> myLasers;
  std::map<int, MvrFunctor1C<MvrModeLaser, int> *> myLaserCallbacks;
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
  MVREXPORT MvrModeConfig(MvrRobot *robot, const char *name, char key, char key2);
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void help(void);
  
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


#endif // MVRMODES_H
