/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrDPPTU.h
 > Description  : Interface to Directed Perception pan/tilt unit, implementing and extending the ArPTZ interface
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/

#ifndef MVRDPPTU_H
#define MVRDPPTU_H

#include "mvriaTypedefs.h"
#include "MvrRobot.h"
#include "MvrPTZ.h"
#include <vector>

class MvrBasePacket;

/// A class with the commands for the DPPTU
class MvrDPPTUCommands
{
public:

  enum {
    DELIM = 0x20, ///<Space - Carriage return delimeter
    INIT = 0x40, ///<Init character
    ACCEL = 0x61, ///<Acceleration, Await position-command completion
    BASE = 0x62, ///<Base speed
    CONTROL = 0x63, ///<Speed control
    DISABLE = 0x64, ///<Disable character, Delta, Default
    ENABLE = 0x65, ///<Enable character, Echoing
    FACTORY = 0x66, ///<Restore factory defaults
    HALT = 0x68, ///<Halt, Hold, High
    IMMED = 0x69, ///<Immediate position-command execution mode, Independent control mode
    LIMIT = 0x6C, ///<Position limit character, Low
    MONITOR = 0x6D, ///<Monitor, In-motion power mode
    OFFSET = 0x6F, ///<Offset position, Off
    PAN = 0x70, ///<Pan
    RESET = 0x72, ///<Reset calibration, Restore stored defaults, Regular
    SPEED = 0x73, ///<Speed, Slave
    TILT = 0x74, ///<Tilt
    UPPER = 0x75, ///<Upper speed limit
    VELOCITY = 0x76 ///<Velocity control mode
  };

};

/// A class for for making commands to send to the DPPTU

class MvrDPPTUPacket: public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrDPPTUPacket(MvrTypes::UByte2 bufferSize = 30);
  /// Destructor
  MVREXPORT virtual ~MvrDPPTUPacket();

  MVREXPORT virtual void byte2ToBuf(int val);

  MVREXPORT virtual void finalizePacket(void);

protected:
};

/// Driver for the DPPTU
class MvrDPPTU : public MvrPTZ
{
public:
  enum DeviceType {
    PANTILT_DEFAULT, ///< Automatically detect correct settings
    PANTILT_PTUD47,
    PANTILT_PTUD46 
  };

  enum Axis {
    PAN = 'P',
    TILT = 'T'
  };

  /// Constructor
  MVREXPORT MvrDPPTU(MvrRobot *robot, DeviceType deviceType = PANTILT_DEFAULT, int deviceIndex = -1);
  /// Destructor
  MVREXPORT virtual ~MvrDPPTU();

  MVREXPORT bool init(void);
  MVREXPORT virtual const char *getTypeName() { return "dpptu"; }

  virtual bool canZoom(void) const { return false; }
  virtual bool canGetRealPanTilt() const { return myCanGetRealPanTilt; }

  /// Sends a delimiter only
  MVREXPORT bool blank(void);

  /// Perform reset calibration (PTU will move to the limits of pan and tilt axes in turn and return to 0,0)
  MVREXPORT bool resetCalib(void);

/// Change stored configuration options
  /// Configure DPPTU to disable future power-on resets
  MVREXPORT bool disableReset(void);
  /// Configure DPPTU to only reset tilt on future power-up
  MVREXPORT bool resetTilt(void);
  /// Configure DPPTU to only reset pan on future power up
  MVREXPORT bool resetPan(void);

  ///  Configure DPPTU to reset both pan and tilt on future power on
  MVREXPORT bool resetAll(void);

  /// Enables monitor mode at power up
  MVREXPORT bool enMon(void);
  /// Disables monitor mode at power up
  MVREXPORT bool disMon(void);
  /// Save current settings as defaults
  MVREXPORT bool saveSet(void);
  /// Restore stored defaults
  MVREXPORT bool restoreSet(void);
  /// Restore factory defaults
  MVREXPORT bool factorySet(void);


protected:
///Move the pan and tilt axes
  virtual bool panTilt_i(double pdeg, double tdeg) 
  { 
    return pan(pdeg) && tilt(tdeg); 
  }

  MVREXPORT virtual bool pan_i(double deg);

  virtual bool panRel_i(double deg) 
  { 
    return panTilt(myPan+deg, myTilt); 
  }

  MVREXPORT virtual bool tilt_i(double deg);

  virtual bool tiltRel_i(double deg) 
  { 
    return panTilt(myPan, myTilt+deg); 
  }

  virtual bool panTiltRel_i(double pdeg, double tdeg) 
  { 
    return panRel(pdeg) && tiltRel(tdeg);
  }
public:
  /// Instructs unit to await completion of the last issued command
  MVREXPORT bool awaitExec(void);
  /// Halts all pan-tilt movement
  MVREXPORT bool haltAll(void);
  /// Halts pan axis movement
  MVREXPORT bool haltPan(void);
  /// Halts tilt axis movement
  MVREXPORT bool haltTilt(void);

  /// Sets monitor mode - pan pos1/pos2, tilt pos1/pos2
  MVREXPORT bool initMon(double deg1, double deg2, double deg3, double deg4);


  /// Enables or disables the position limit enforcement
  MVREXPORT bool limitEnforce(bool val);

///Set execution modes
  /// Sets unit to immediate-execution mode for positional commands. Commands will be executed by PTU as soon as they are received. 
  MVREXPORT bool immedExec(void);
  /// Sets unit to slaved-execution mode for positional commands. Commands will not be executed by PTU until awaitExec() is used. 
  MVREXPORT bool slaveExec(void);


  double getMaxPanSlew(void) { return myMaxPanSlew; }
  virtual double getMaxPanSpeed() { return getMaxPanSlew(); }
  double getMinPanSlew(void) { return myMinPanSlew; }
  double getMaxTiltSlew(void) { return myMaxTiltSlew; }
  virtual double getMaxTiltSpeed() { return getMaxTiltSlew(); }
  double getMinTiltSlew(void) { return myMinTiltSlew; }
  double getMaxPanAccel(void) { return myMaxPanSlew; }
  double getMinPanAccel(void) { return myMinPanSlew; }
  double getMaxTiltAccel(void) { return myMaxTiltSlew; }
  double getMinTiltAccel(void) { return myMinTiltSlew; }

///Enable/disable moving and holding power modes for pan and tilt
  enum PowerMode {
    OFF = 'O',
    LOW = 'L',
    NORMAL = 'R',
    HIGH = 'H'
  };

  /// Configure power mode for an axis when in motion.
  /// init() sets initial moving power mode to Low, call this method to choose a different mode.
  /// The recomended modes are either Low or Normal.
  MVREXPORT bool setMovePower(Axis axis, PowerMode mode);

  /// Configure power mode for an axis when stationary.
  /// init() sets the initial halted power to Off. Call this method to choose a different mode.
  /// The recommended modes are Off or Low.
  MVREXPORT bool setHoldPower(Axis axis, PowerMode mode); 

  bool offStatPower(void) 
  { return setHoldPower(PAN, OFF) && setHoldPower(TILT, OFF);}

  bool regStatPower(void) 
  { return setHoldPower(PAN, NORMAL) && setHoldPower(TILT, NORMAL); }

  bool lowStatPower(void) 
  { return setHoldPower(PAN, LOW) && setHoldPower(TILT, LOW);}

  bool highMotPower(void) 
  { return setMovePower(PAN, HIGH) && setMovePower(TILT, HIGH); }

  bool regMotPower(void) 
  { return setMovePower(PAN, NORMAL) && setMovePower(TILT, NORMAL); }

  bool lowMotPower(void) 
  { return setMovePower(PAN, LOW) && setMovePower(TILT, LOW); }

  /// Sets acceleration for pan axis
  MVREXPORT bool panAccel(double deg);
  /// Sets acceleration for tilt axis
  MVREXPORT bool tiltAccel(double deg);

  /// Sets the start-up pan slew
  MVREXPORT bool basePanSlew(double deg);
  /// Sets the start-up tilt slew
  MVREXPORT bool baseTiltSlew(double deg);

  /// Sets the upper pan slew
  MVREXPORT bool upperPanSlew(double deg);
  /// Sets the lower pan slew
  MVREXPORT bool lowerPanSlew(double deg);
  /// Sets the upper tilt slew
  MVREXPORT bool upperTiltSlew(double deg);
  /// Sets the lower pan slew
  MVREXPORT bool lowerTiltSlew(double deg);

  /// Sets motion to indenpendent control mode
  MVREXPORT bool indepMove(void);
  /// Sets motion to pure velocity control mode
  MVREXPORT bool velMove(void);

  /// Sets the rate that the unit pans at
  MVREXPORT bool panSlew(double deg);
  /// Sets the rate the unit tilts at 
  MVREXPORT bool tiltSlew(double deg);
  bool canPanTiltSlew() { return true; }
  

  /// Sets the rate that the unit pans at, relative to current slew
  MVREXPORT bool panSlewRel(double deg) { return panSlew(myPanSlew+deg); }
  /// Sets the rate the unit tilts at, relative to current slew
  MVREXPORT bool tiltSlewRel(double deg) { return tiltSlew(myTiltSlew+deg); }

  /// called automatically by Mvria::init()

#ifndef SWIG
  static void registerPTZType();
#endif

protected:
/// Get current pan/tilt position, if receiving from device, otherwise return
/// last position request sent to the device. @see canGetRealPanTilt()

  virtual double getPan_i(void) const { return myPan; }
  virtual double getTilt_i(void) const { return myTilt; }



public:

/// Get last pan/tilt requested. The device may still be moving towards these positions. (@sa getPan(), getTilt(), canGetRealPanTilt())

  double getLastPanRequest() const { return myPanSent; }
  double getLastTiltRequest() const { return myTiltSent; }
  

  /// Gets the current pan slew
  double getPanSlew(void) { return myPanSlew; }
  /// Gets the current tilt slew
  double getTiltSlew(void) { return myTiltSlew; }
  /// Gets the base pan slew
  double getBasePanSlew(void) { return myBasePanSlew; }
  /// Gets the base tilt slew
  double getBaseTiltSlew(void) { return myBaseTiltSlew; }
  /// Gets the current pan acceleration rate
  double getPanAccel(void) { return myPanAccel; }
  /// Gets the current tilt acceleration rate
  double getTiltAccel(void) { return myTiltAccel; }

  MVREXPORT void query(); // called from robot sensor interpretation task, or can be done seperately

protected:
  MvrRobot *myRobot;
  MvrDPPTUPacket myPacket;
  void preparePacket(void); ///< adds on extra delim in front to work on H8
  double myPanSent;  ///< Last pan command sent
  double myTiltSent; ///< Last tilt command sent
  double myPanSlew;
  double myTiltSlew;
  double myBasePanSlew;
  double myBaseTiltSlew;
  double myPanAccel;
  double myTiltAccel;

  DeviceType myDeviceType;
  /*
  int myMaxPan;
  int myMinPan;
  int myMaxTilt;
  int myMinTilt;
  */
  int myMaxPanSlew;
  int myMinPanSlew;
  int myMaxTiltSlew;
  int myMinTiltSlew;
  int myMaxPanAccel;
  int myMinPanAccel;
  int myMaxTiltAccel;
  int myMinTiltAccel;
  float myPanConvert;
  float myTiltConvert;

  float myPan;
  float myPanRecd; ///< Last pan value received from DPPTU from PP command
  float myTilt;
  float myTiltRecd; ///< Last tilt value received from DPPTU from TP command

  bool myCanGetRealPanTilt;

  bool myInit;
  //MVREXPORT virtual bool packetHandler(MvrBasePacket *pkt);
  MVREXPORT virtual MvrBasePacket *readPacket();
  MvrFunctorC<MvrDPPTU> myQueryCB;
  char *myDataBuf;
  

  MvrFunctorC<MvrDPPTU> myShutdownCB;
  int myDeviceIndex;
  void shutdown();

  MvrTime myLastPositionMessageHandled;
  bool myWarnedOldPositionData;
  MvrTime myLastQuerySent;

  std::vector<char> myPowerPorts;

  bool myGotPanRes;
  bool myGotTiltRes;

  static MvrPTZ* create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot);
  static MvrPTZConnector::GlobalPTZCreateFunc ourCreateFunc;
};

#endif // MVRDPPTU_H