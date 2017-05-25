/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrAMPTU.h
 > Description  : A class with the commands for the AMPTU
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRAMPTU_H
#define MVRAMPTU_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "MvrPTZ.h"

/// A class with the commands for the AMPTU
class MvrAMPTUCommands
{
public:
  enum {
    ABSTILT = 0x35, ///< Absolute tilt
    RELTILTU = 0x36, ///< Relative tilt, up
    RELTILTD = 0x37, ///< Relative tilt, down
    ABSPAN = 0x31, ///< Absolute pan
    RELPANCW = 0x32, ///< Relative pan, clockwise
    RELPANCCW = 0x33, ///< Relative pan, counter clockwise
    PANTILT = 0x28, ///< Pan and tilt absolute
    PANTILTUCW = 0x29, ///< Relative tilt up, pan clockwise
    PANTILTDCW = 0x2A, ///< Relative tilt down, pan clockwise
    PANTILTUCCW = 0x2B, ///< Relative tilt up, pan counter-clockwise
    PANTILTDCCW = 0x2C, ///< Relative tilt down, pan counter-clockwise
    ZOOM = 0x3F, ///< Zoom
    PAUSE = 0x39, ///< Pause the current movement
    CONT = 0x3A, ///< Continue paused movement
    PURGE = 0x3B, ///< Stops movement and purges commands
    STATUS = 0x3C, ///< Requests a status packet
    INIT = 0x3D, ///< Initializes the camera
    RESP = 0x3E, ///< Response
    PANSLEW = 0x34, ///< Sets the pan slew rate
    TILTSLEW = 0x38 ///< Sets the tilt slew rate
  };

};

/// A class for for making commands to send to the AMPTU

class MvrAMPTUPacket: public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrAMPTUPacket(MvrTypes::UByte2 bufferSize = 30);
  /// Destructor
  MVREXPORT virtual ~MvrAMPTUPacket();

  /// Gets the unit number this packet is for
  MVREXPORT unsigned char getUnitNumber(void);
  /// Sets the unit number htis packet is for
  MVREXPORT bool setUnitNumber(unsigned char unitNumber);
  
  MVREXPORT virtual void byteToBuf(MvrTypes::Byte val);
  MVREXPORT virtual void byte2ToBuf(MvrTypes::Byte2 val);
  
  MVREXPORT virtual void finalizePacket(void);

protected:
  unsigned char myUnitNumber;
};

/// Driver for the AMPTU

class MvrAMPTU : public MvrPTZ
{
public:
  /// Constructor
  MVREXPORT MvrAMPTU(MvrRobot *robot, int unitNumber = 0);
  /// Destructor
  MVREXPORT virtual ~MvrAMPTU();
  MVREXPORT virtual bool init(void);
  MVREXPORT virtual const char *getTypeName() { return "amptu"; }
protected:
  MVREXPORT virtual bool pan_i(double deg);
  MVREXPORT virtual bool panRel_i(double deg);
  MVREXPORT virtual bool tilt_i(double deg);
  MVREXPORT virtual bool tiltRel_i(double deg);
  MVREXPORT virtual bool panTilt_i(double panDeg, double tiltDeg);
  MVREXPORT virtual bool panTiltRel_i(double panDeg, double tiltDeg);
public:
  /// Sets the rate that the camera pans at
  MVREXPORT bool panSlew(double deg);
  /// Sets the rate the camera tilts at 
  MVREXPORT bool tiltSlew(double deg);
  virtual bool canZoom(void) const { return false; }
  virtual bool canPanTiltSlew() { return true; }
  


  /// Stops current pan/tilt, can be resumed later
  MVREXPORT bool pause(void);
  /// Resumes a previously paused pan/tilt
  MVREXPORT bool resume(void);
  /// Stops motion and purges last command
  MVREXPORT bool purge(void);
  /// Retrieves the camera status
  MVREXPORT bool requestStatus(void);
  /// Gets the angle the camera is panned to
  MVREXPORT virtual double getPan_i(void) const { return myPan; }
  /// Gets the angle the camera is tilted to
  MVREXPORT virtual double getTilt_i(void) const { return myTilt; }
  enum {
    MIN_SLEW = 15,
    MAX_TILT_SLEW = 200,
    MAX_PAN_SLEW = 120
  };
protected:
  MvrRobot *myRobot;
  MvrAMPTUPacket myPacket;
  int myUnitNumber;
  double myPan;
  double myTilt;
  double myPanSlew;
  double myTiltSlew;

};

#endif // MVRAMPTU_H

