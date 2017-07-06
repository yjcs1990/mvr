#ifndef MVRSONYPTZ_H
#define MVRSONYPTZ_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "MvrPTZ.h"

/// A class for for making commands to send to the sony
/** There are only two functioning ways to put things into this packet, you
    MUST use thse, if you use anything else your commands won't work.  You 
    must use uByteToBuf and byte2ToBuf.  
*/
class MvrSonyPacket: public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrSonyPacket(MvrTypes::UByte2 bufferSize = 15);
  MVREXPORT virtual ~MvrSonyPacket();
  
  MVREXPORT virtual void uByteToBuf(MvrTypes::UByte val);
  MVREXPORT virtual void byte2ToBuf(MvrTypes::Byte2 val);
  /// This is a new function, read the details before you try to use it
  MVREXPORT void byte2ToBufAtPos(MvrTypes::Byte2 val, MvrTypes::UByte2 pose);
};

class MvrRobot;

/// A class to use the sony pan tilt zoom unit

class MvrSonyPTZ : public MvrPTZ
{
public:
  MVREXPORT MvrSonyPTZ(MvrRobot *robot);
  MVREXPORT virtual ~MvrSonyPTZ();
  
  MVREXPORT virtual bool init(void);
  MVREXPORT virtual const char  *getTypeName() { return "sony"; }
protected:
  MVREXPORT virtual bool pan_i(double degrees);
  MVREXPORT virtual bool panRel_i(double degrees);
  MVREXPORT virtual bool tilt_i(double degrees);
  MVREXPORT virtual bool tiltRel_i(double degrees);
  MVREXPORT virtual bool panTilt_i(double degreesPan, double degreesTilt);
  MVREXPORT virtual bool panTiltRel_i(double degreesPan, double degreesTilt);
public:
  MVREXPORT virtual bool canZoom(void) const { return true; }
  MVREXPORT virtual bool zoom(int zoomValue);
  MVREXPORT virtual bool zoomRel(int zoomValue);
protected:
  MVREXPORT virtual double getPan_i(void) const { return myPan; }
  MVREXPORT virtual double getTilt_i(void) const { return myTilt; }
public:
  MVREXPORT virtual int getZoom(void) const { return myZoom; }


  MVREXPORT virtual bool canGetFOV(void) { return true; }
  /// Gets the field of view at maximum zoom
  MVREXPORT virtual double getFOVAtMaxZoom(void) { return 4.4; }
  /// Gets the field of view at minimum zoom
  MVREXPORT virtual double getFOVAtMinZoom(void) { return 48.8; }

  MVREXPORT bool backLightingOn(void);
  MVREXPORT bool backLightingOff(void);
  //MVREXPORT bool packetHandler(MvrRobotPacket *packet);
  /* unused?
  enum {
    MAX_PAN = 95, ///< maximum degrees the unit can pan (either direction)
    MAX_TILT = 25, ///< maximum degrees the unit can tilt (either direction)
    MIN_ZOOM = 0, ///< minimum value for zoom
    MAX_ZOOM = 1023 ///< maximum value for zoom
  };
  */
  
  /// called automatically by Mvria::init()
  ///@internal
#ifndef SWIG
  static void registerPTZType();
#endif

protected:
  void initializePackets(void);
  MvrRobot *myRobot;
  double myPan;
  double myTilt;
  int myZoom;
  double myDegToTilt;
  double myDegToPan;
  MvrSonyPacket myPacket;
  MvrSonyPacket myZoomPacket; 
  MvrSonyPacket myPanTiltPacket;

  static MvrPTZ* create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot);
  static MvrPTZConnector::GlobalPTZCreateFunc ourCreateFunc;
};

#endif // MVRSONYPTZ_H
