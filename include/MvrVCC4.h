/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrVCC4.h
 > Description  : Control the pan, tilt, and zoom mechanisms of the Canon VC-C4 and VC-C50i cameras.
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRVCC4_H
#define MVRVCC4_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "MvrPTZ.h"
#include "mvriaUtil.h"
#include "MvrCommands.h"
#include "MvrSerialConnection.h"
class MvrVCC4Commands
{
public:
  enum Command {
    DELIM = 0x00, ///<Delimeter character
    DEVICEID = 0x30, ///<Default device ID
    PANSLEW = 0x50, ///<Sets the pan slew
    TILTSLEW = 0x51, ///<Sets the tilt slew
    STOP = 0x53, ///<Stops current pan/tilt motion
    INIT = 0x58, ///<Initializes the camera
    SLEWREQ = 0x59, ///<Request pan/tilt min/max slew
    ANGLEREQ = 0x5c, ///<Request pan/tilt min/max angle
    PANTILT = 0x62, ///<Pan/tilt command
    SETRANGE = 0x64, ///<Pan/tilt min/max range assignment
    PANTILTREQ = 0x63, ///<Request pan/tilt position
    INFRARED = 0x76, ///<Controls operation of IR lighting
    PRODUCTNAME = 0x87, ///<Requests the product name
    LEDCONTROL = 0x8E, ///<Controls LED status
    CONTROL = 0x90, ///<Puts camera in Control mode
    POWER = 0xA0, ///<Turns on/off power
    AUTOFOCUS = 0xA1, ///<Controls auto-focusing functions
    ZOOMSTOP = 0xA2, ///<Stops zoom motion
    GAIN = 0xA5, ///<Sets gain adjustment on camera
    FOCUS = 0xB0, ///<Manual focus adjustment
    ZOOM = 0xB3, ///<Zooms camera lens
    ZOOMREQ = 0xB4, ///<Requests max zoom position
    IRCUTFILTER = 0xB5, ///<Controls the IR cut filter
    DIGITALZOOM = 0xB7, ///<Controls the digital zoom amount
    FOOTER = 0xEF, ///<Packet Footer
    RESPONSE = 0xFE, ///<Packet header for response
    HEADER = 0xFF ///<Packet Header
  };

};

/// Used by MvrVCC4 to construct command packets
/** 
    There are only a few functioning ways to put things into this packet, you
    MUST use thse, if you use anything else your commands won't work.  You 
    must use only byteToBuf() and byte2ToBuf(), no other MvrBasePacket methods.
*/
class MvrVCC4Packet: public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrVCC4Packet(MvrTypes::UByte2 bufferSize = 30);
  /// Destructor
  MVREXPORT virtual ~MvrVCC4Packet();

  MVREXPORT virtual void byte2ToBuf(MvrTypes::Byte4 val);

  MVREXPORT virtual void finalizePacket(void);

protected:
};

class MvrVCC4 : public MvrPTZ
{
public:
  // the states for communication
  enum CommState {
    COMM_UNKNOWN,
    COMM_BIDIRECTIONAL,
    COMM_UNIDIRECTIONAL
  };

  enum CameraType {
    CAMERA_VCC4,
    CAMERA_C50I
  };

  /// Constructor
  MVREXPORT MvrVCC4(MvrRobot *robot, bool inverted = false, CommState commDirection = COMM_UNKNOWN, bool autoUpdate = true, bool disableLED = false, CameraType cameraType = CAMERA_VCC4);
  /// Destructor
  MVREXPORT virtual ~MvrVCC4();

  MVREXPORT virtual bool power(bool state) { myPowerStateDesired = state; return true; }
  MVREXPORT bool getPower(void) { return myPowerState; }
  MVREXPORT virtual bool init(void) { myInitRequested = true; return true; }
  MVREXPORT virtual void reset(void) { MvrPTZ::reset(); init(); }
  MVREXPORT virtual const char  *getTypeName() { return "vcc4"; }

  /// Returns true if the camera has been initialized
   bool isInitted(void) { return myCameraIsInitted; }
  MVREXPORT virtual void connectHandler(void);
  MVREXPORT virtual bool packetHandler(MvrBasePacket *packet);

protected:
   virtual bool pan_i(double deg) { return panTilt_i(deg, myTiltDesired); }
   virtual bool panRel_i(double deg) { return panTilt_i(myPanDesired + deg, myTiltDesired); }
   virtual bool tilt_i(double deg) { return panTilt_i(myPanDesired, deg); }
   virtual bool tiltRel_i(double deg) { return panTilt_i(myPanDesired, myTiltDesired + deg); }
   virtual bool panTiltRel_i(double pdeg, double tdeg) { return panTilt_i(myPanDesired + pdeg, myTiltDesired + tdeg); }

public:

  /*
  MVREXPORT virtual double getMaxPosPan(void) const 
    { if (myInverted) return invert(MIN_PAN); else return MAX_PAN; }
  MVREXPORT virtual double getMaxNegPan(void) const 
    { if (myInverted) return invert(MAX_PAN); else return MIN_PAN; }
  MVREXPORT virtual double getMaxPosTilt(void) const 
    { if (myInverted) return invert(MIN_TILT); else return MAX_TILT; }
  MVREXPORT virtual double getMaxNegTilt(void) const
    { if (myInverted) return invert(MAX_TILT); else return MIN_TILT; }
 */

  /// Requests that a packet be sent to the camera to retrieve what
  /// the camera thinks are its pan/tilt positions. getPan() and getTilt()
  /// will then return this information instead of your last requested values.
  MVREXPORT void getRealPanTilt(void) { myRealPanTiltRequested = true; }

  /// Requests that a packet be sent to the camera to retrieve what
  /// the camera thinks is its zoom position. getZoom()
  /// will then return this information instead of your last requested value.
  MVREXPORT void getRealZoomPos(void) { myRealZoomRequested = true; }

  MVREXPORT virtual bool canZoom(void) const { return true; }

protected:
  MVREXPORT virtual bool panTilt_i(double pdeg, double tdeg);

public:
  MVREXPORT virtual bool zoom(int deg);
  /// adjust the digital zoom amount.  Has four states, takes 0-3 for:
  /// 1x, 2x, 4x, 8x
  MVREXPORT bool digitalZoom(int deg);

  /// Adds an error callback to a list of callbacks to be called when there
  /// is a serious error in communicating - either the parameters were incorrect,
  /// the mode was incorrect, or there was an unknown error.
  MVREXPORT void addErrorCB(MvrFunctor *functor, MvrListPos::Pos position);

  /// Remove an error callback from the callback list
  MVREXPORT void remErrorCB(MvrFunctor *functor);

  /// Halts all pan-tilt movement
  MVREXPORT bool haltPanTilt(void) { myHaltPanTiltRequested = true; return true; }
  /// Halts zoom movement
  MVREXPORT bool haltZoom(void) { myHaltZoomRequested = true; return true; }

  /// Sets the rate that the unit pans at
  MVREXPORT bool panSlew(double deg) { myPanSlewDesired = deg; return true; }
  /// Sets the rate the unit tilts at 
  MVREXPORT bool tiltSlew(double deg) { myTiltSlewDesired = deg; return true; }
  bool canSetPanTiltSlew() { return true; }

  /// Adds device ID and delimeter to packet buffer
  MVREXPORT void preparePacket(MvrVCC4Packet *packet);

protected:
  MVREXPORT virtual double getPan_i(void) const { return myPanDesired; }
  MVREXPORT virtual double getTilt_i(void) const { return myTiltDesired; }

public:
  MVREXPORT virtual int getZoom(void) const { return myZoomDesired; }
  MVREXPORT double getDigitalZoom(void) const { return myDigitalZoomDesired; }

  MVREXPORT virtual bool canGetRealPanTilt(void) const { return true; }
  MVREXPORT virtual bool canGetRealZoom(void) const { return true; }
  MVREXPORT virtual bool canSetFocus(void) const { return false; }
  /// Set autofocus mode:
  /// @deprecated use setAutoFocus() instead
  MVREXPORT virtual bool autoFocus(void) { myFocusModeDesired = 0; return true;}
  /// set manual focus mode
  /// @deprecated use setAutoFocus() instead
  MVREXPORT virtual bool manualFocus(void) { myFocusModeDesired = 1; return true;}
  /// auto-focus on a near object
  MVREXPORT virtual bool focusNear(void) { myFocusModeDesired = 2; return true;}
  /// auto-focus on a far object
  MVREXPORT virtual bool focusFar(void) { myFocusModeDesired = 3; return true; }

  MVREXPORT virtual bool setAutoFocus(bool af = true) 
  {
    if(af)
      return autoFocus();
    else
      return manualFocus();
  }

  /// Gets the current pan slew
  MVREXPORT double getPanSlew(void) { return myPanSlewDesired; }
  /// Gets the maximum pan slew
  MVREXPORT double getMaxPanSlew(void) { return MAX_PAN_SLEW; }
  /// Gets the minimum pan slew
  MVREXPORT double getMinPanSlew(void) { return MIN_PAN_SLEW; }

  /// Gets the current tilt slew
  MVREXPORT double getTiltSlew(void) { return myTiltSlewDesired; }
  /// Gets the maximum tilt slew
  MVREXPORT double getMaxTiltSlew(void) { return MAX_TILT_SLEW; }
  /// Gets the minimum tilt slew
  MVREXPORT double getMinTiltSlew(void) { return MIN_TILT_SLEW; }

  MVREXPORT virtual int getMaxZoom(void) const;
  MVREXPORT virtual int getMinZoom(void) const { return MIN_ZOOM; }

  MVREXPORT virtual bool canGetFOV(void) { return true; }
  /// Gets the field of view at maximum zoom
  MVREXPORT virtual double getFOVAtMaxZoom(void) { return myFOVAtMaxZoom; }
  /// Gets the field of view at minimum zoom
  MVREXPORT virtual double getFOVAtMinZoom(void) { return myFOVAtMinZoom; }


  /// Returns true if the error callback list was called during the last cycle
  MVREXPORT bool wasError(void) { return myWasError; }

  /// Toggle the state of the auto-update
  MVREXPORT void enableAutoUpdate(void) { myAutoUpdate = true; }
  MVREXPORT void disableAutoUpdate(void) { myAutoUpdate = false; }
  MVREXPORT bool getAutoUpdate(void) { return myAutoUpdate; }

  /// Set the control mode for the status LED on the front of the camera
  /// 0 = auto-control, 1 = Green ON, 2 = All OFF, 3 = Red ON, 4 = Orange ON
  MVREXPORT void setLEDControlMode(int controlMode) { myDesiredLEDControlMode = controlMode; }
  /// Turn on IR LEDs.  IR-filter must be in place for LEDs to turn on
  MVREXPORT void enableIRLEDs(void) { myDesiredIRLEDsMode = true; }
  /// Turn off IR LEDs
  MVREXPORT void disableIRLEDs(void) { myDesiredIRLEDsMode = false; }
  /// Returns true if the IR LEDs are on
  MVREXPORT bool getIRLEDsEnabled(void) { return myIRLEDsEnabled; }
  /// Enable physical IR cutoff filter
  MVREXPORT void enableIRFilterMode(void) { myDesiredIRFilterMode = true; }
  /// Disable IR cutoff filter.  This also turns off the LEDs, if they're on
  MVREXPORT void disableIRFilterMode(void) { myDesiredIRFilterMode = false; }
  /// Returns true if the IR cutoff filter is in place
  MVREXPORT bool getIRFilterModeEnabled (void) { return myIRFilterModeEnabled; }
protected:

  // preset limits on movements.  Based on empirical data
  enum Param {
    MAX_PAN = 98,		// 875 units is max pan assignment
    MIN_PAN = -98,		// -875 units is min pan assignment
    MAX_TILT = 88,		// 790 units is max tilt assignment
    MIN_TILT = -30,		// -267 units is min tilt assignment
    MAX_PAN_SLEW = 90,		// 800 positions per sec (PPS)
    MIN_PAN_SLEW = 1,		// 8 positions per sec (PPS)
    MAX_TILT_SLEW = 69,		// 662 positions per sec (PPS)
    MIN_TILT_SLEW = 1,		// 8 positions per sec (PPS)
    MAX_ZOOM_OPTIC = 1960,
    MIN_ZOOM = 0
  };

  // the various error states that the camera can return
  enum Error {
    CAM_ERROR_NONE = 0x30, ///<No error
    CAM_ERROR_BUSY = 0x31, ///<Camera busy, will not execute the command
    CAM_ERROR_PMVRAM = 0x35, ///<Illegal parameters to function call
    CAM_ERROR_MODE = 0x39,  ///<Not in host control mode
    CAM_ERROR_UNKNOWN = 0xFF ///<Unknown error condition.  Should never happen
 };

  // the states of the FSM
  enum State {
    UNINITIALIZED,
    STATE_UNKNOWN,
    INITIALIZING,
    SETTING_CONTROL_MODE,
    SETTING_INIT_TILT_RATE,
    SETTING_INIT_PAN_RATE,
    SETTING_INIT_RANGE,
    POWERING_ON,
    POWERING_OFF,
    POWERED_OFF,
    POWERED_ON,
    AWAITING_INITIAL_POWERON,
    AWAITING_INITIAL_INIT,
    AWAITING_ZOOM_RESPONSE,
    AWAITING_PAN_TILT_RESPONSE,
    AWAITING_STOP_PAN_TILT_RESPONSE,
    AWAITING_STOP_ZOOM_RESPONSE,
    AWAITING_PAN_SLEW_RESPONSE,
    AWAITING_TILT_SLEW_RESPONSE,
    AWAITING_POS_REQUEST,
    AWAITING_ZOOM_REQUEST,
    AWAITING_LED_CONTROL_RESPONSE,
    AWAITING_IRLEDS_RESPONSE,
    AWAITING_IRFILTER_RESPONSE,
    AWAITING_PRODUCTNAME_REQUEST,
    AWAITING_DIGITAL_ZOOM_RESPONSE,
    AWAITING_FOCUS_RESPONSE,
    STATE_DELAYED_SWITCH,
    STATE_ERROR
  };

  // flips the sign if needed
  //double invert(double before) const
  //  { if (myInverted) return -before; else return before; }
 // bool myInverted;

  // true if there was an error during the last cycle
  bool myWasError;

  // the camera name.  "C50i" for C50i, and "VC-C" for VC-C4
  std::string myProductName;

  MvrRobot *myRobot;
  MvrDeviceConnection *myConn;
  MvrBasePacket *newPacket;
  MvrVCC4Packet myPacket;

  // timers for watching for timeouts
  MvrTime myStateTime;
  MvrTime myPacketTime;
  MvrTime myIdleTime;

  // gets set to true if using an aux port vs computer serial port
  bool myUsingAuxPort;

  // delay variable, if delaying before switching to the next state
  int myStateDelayTime;

  // what type of communication the camera is using
  CommState myCommType;

  // used to read data if the camera is attached directly to a computer
  virtual MvrBasePacket* readPacket(void);

  // the functor to add as a usertask
  MvrFunctorC<MvrVCC4> myTaskCB;

  // the actual task to be added as a usertask
  void camTask(void);

  // true when a response has been received from the camera, but has
  // not yet been acted on by the state machine
  bool myResponseReceived;

  bool myWaitingOnStop;
  bool myWaitingOnPacket;

  // the state of the state machine
  State myState;
  State myPreviousState;
  State myNextState;

  // used to switch between states in the state machine
  void switchState(State state, int delayTime = 0);

  // the max time before a state times out, and the time for a packet response
  // to timeout.  The difference being that a packet reponse can be received
  // immediately, but it could say that the camera is busy, meaning the state
  // has not yet completed
  int myStateTimeout;
  int myPacketTimeout;

  // request a packet from the microcontroller of size num bytes.
  // most camera responses are 6 bytes, so just use the default
  void requestBytes(int num = 6);

  // the buffer to store the incoming packet data in
  unsigned char myPacketBuf[50];
  int myPacketBufLen;

  // how many bytes we're still expecting to receive from the controller
  int myBytesLeft;

  // these all send commands to the camera.
  bool sendPanTilt(void);
  bool sendZoom(void);
  bool sendPanSlew(void);
  bool sendTiltSlew(void);
  bool sendPower(void);
  bool sendHaltPanTilt(void);
  bool sendHaltZoom(void);
  bool sendRealPanTiltRequest(void);
  bool sendRealZoomRequest(void);
  bool sendDigitalZoom(void);
  bool sendFocus(void);
  
  // this is currently not used because it doesn't work right
  bool sendProductNameRequest(void);

  // the camera type is used to specify VC-C4 vs. C50i
  CameraType myCameraType;
  bool myRequestProductName;

  bool sendLEDControlMode(void);
  bool sendCameraNameRequest(void);
  int myDesiredLEDControlMode;

  bool sendIRFilterControl(void);
  bool sendIRLEDControl(void);
  bool myIRLEDsEnabled;
  bool myDesiredIRLEDsMode;
  bool myIRFilterModeEnabled;
  bool myDesiredIRFilterMode;

  // These should only be used by the state machine to initialize the 
  // camera for the first time
  bool setDefaultRange(void);
  bool setControlMode(void);
  bool sendInit(void);

  // process the packet data for a camera response that has accurate
  // pan/tilt positional information in it, and the product name
  void processGetPanTiltResponse(void);
  void processGetZoomResponse(void);
  void processGetProductNameResponse(void);

  // true if autoupdating of camera's position should be used  
  bool myAutoUpdate;

  // cycle for stepping through various autoupdate resquests from the camera
  int myAutoUpdateCycle;

  // returns true if there is no reponse to a packet within the timeout
  // or also if the state times out.  The argument will overrid the default
  // timeout periods
  bool timeout(int mSec = 0);

  // internal reperesenstation of pan, tilt, and zoom positions
  double myPan;
  double myTilt;
  int myZoom;
  int myDigitalZoom;
  int myFocusMode;

  // used to store the returned positional values when requesting the true
  // position from the camera
  double myPanResponse;
  double myTiltResponse;
  int myZoomResponse;

  // the returned product name
  char myProductNameResponse[4];

  // the positions that were last sent to the camera.  These are needed
  // because the desired positions can change between time a command is
  // sent and before it succeeds.
  double myPanSent;
  double myTiltSent;
  int myZoomSent;
  double myPanSlewSent;
  double myTiltSlewSent;

  // internal representation of pan and tilt slew
  double myPanSlew;
  double myTiltSlew;

  // where the user has requested the camera move to
  double myPanDesired;
  double myTiltDesired;
  int myZoomDesired;
  int myDigitalZoomDesired;
  int myFocusModeDesired;

  // the pan an tilt slew that the user requested
  double myPanSlewDesired;
  double myTiltSlewDesired;

  // internal mirror of camera power state, and whether it's be initted
  bool myPowerState;
  bool myCameraIsInitted;

  // whether the user wants the camera on or off, or initialized
  bool myPowerStateDesired;
  bool myInitRequested;

  // whether the user has requested to halt movement
  bool myHaltZoomRequested;
  bool myHaltPanTiltRequested;

  // whether the camera has been initialized since instance inception
  bool myCameraHasBeenInitted;

  // true if the user has requested to update the camera's postion
  // from the data returned from the camera
  bool myRealPanTiltRequested;
  bool myRealZoomRequested;

  // the error state from the last packet received
  unsigned int myError;

  // our FOV numbers (these should change if we use the digital zoom)
  double myFOVAtMaxZoom;
  double myFOVAtMinZoom;

  // run through the list or error callbacks
  void throwError();

  // the list of error callbacks to step through when a error occurs
  std::list<MvrFunctor *> myErrorCBList;

  /// Used by MvrPTZConnector to create an MvrVCC4 object based on robot parameters and program options.

  static MvrPTZ* create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot);
  /// Used by MvrPTZConnector to create an MvrVCC4 object based on robot parameters and program options.
  static MvrPTZConnector::GlobalPTZCreateFunc ourCreateFunc;
public:
#ifndef SWIG
  ///<@internal Called by Mvria::init() toregister this class with MvrPTZConnector for vcc4 and vcc50i PTZ types.
  static void registerPTZType(); 
#endif
};

#endif // MVRVCC4_H