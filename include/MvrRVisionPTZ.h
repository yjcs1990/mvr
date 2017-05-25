/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRVisionPTZ.h
 > Description  : A class for for making commands to send to the RVision camera
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRRVISIONPTZ_H
#define MVRRVISIONPTZ_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "MvrPTZ.h"
#include "MvrSerialConnection.h"
/// A class for for making commands to send to the RVision camera
class MvrRVisionPacket: public MvrBasePacket
{
public:
  /// Constructor
  MVREXPORT MvrRVisionPacket(MvrTypes::UByte2 bufferSize = 15);
  MVREXPORT virtual ~MvrRVisionPacket();
  
  MVREXPORT virtual void uByteToBuf(MvrTypes::UByte val);
  MVREXPORT virtual void byte2ToBuf(MvrTypes::Byte2 val);
  /// This is a new function, read the details before you try to use it
  MVREXPORT void byte2ToBufAtPos(MvrTypes::Byte2 val, MvrTypes::UByte2 pose);
};


class MvrRVisionPTZ : public MvrPTZ
{
public:
  MVREXPORT MvrRVisionPTZ(MvrRobot *robot);
  MVREXPORT virtual ~MvrRVisionPTZ();
  
  MVREXPORT virtual bool init(void);
  MVREXPORT virtual const char *getTypeName() { return "rvision"; }
  /// Set serial port
  void setPort(const char *port)
  {
	  mySerialPort = port;
  }
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

  MVREXPORT virtual bool canGetRealPanTilt(void) const { return false; }
  MVREXPORT virtual bool canGetRealZoom(void) const { return false; }
  MVREXPORT virtual bool canGetFOV(void) { return true; }
  /// Gets the field of view at maximum zoom
  MVREXPORT virtual double getFOVAtMaxZoom(void) { return 4.4; }
  /// Gets the field of view at minimum zoom
  MVREXPORT virtual double getFOVAtMinZoom(void) { return 48.8; }

  virtual MvrBasePacket* readPacket(void);
  enum {
    MAX_PAN = 180, ///< maximum degrees the unit can pan (clockwise from top)
    MIN_PAN = -180, ///< minimum degrees the unit can pan (counterclockwise from top)
    MIN_TILT = -30, ///< minimum degrees the unit can tilt
    MAX_TILT = 60, ///< maximum degrees the unit can tilt
    MIN_ZOOM = 0, ///< minimum value for zoom
    MAX_ZOOM = 32767, ///< maximum value for zoom
    TILT_OFFSET_IN_DEGREES = 38, ///< offset value to convert internal camera coords to world
    PAN_OFFSET_IN_DEGREES = 190 ///< offset value to convert internal camera coords to world
  };
  
  /// called automatically by Mvria::init()
#ifndef SWIG
  static void registerPTZType();
#endif
protected:
  void initializePackets(void);
  double myPan;
  double myTilt;
  int myZoom;
  double myDegToTilt;
  double myDegToPan;
  double myPanOffsetInDegrees;
  double myTiltOffsetInDegrees;
  MvrRVisionPacket myPacket;
  MvrRVisionPacket myZoomPacket; 
  MvrRVisionPacket myPanTiltPacket;
  MvrRVisionPacket myInquiryPacket;
  const char *mySerialPort;

  static MvrPTZ* create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot);
  static MvrPTZConnector::GlobalPTZCreateFunc ourCreateFunc;

};

#endif // MVRRVISIONPTZ_H