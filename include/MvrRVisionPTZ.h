#ifndef MVRRVISIONPTZ_H
#define MVRRVISIONPTZ_H

#include "mvriaTypedefs.h"
#include "MvrBasePacket.h"
#include "MvrPTZ.h"
#include "MvrSerialConnection.h"
/// A class for for making commands to send to the RVision camera
/** There are only two functioning ways to put things into this packet,
 * uByteToBuf() and byte2ToBuf;  You
 *  MUST use thse, if you use anything else your commands won't work.  
 *  @since 2.7.0
*/
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

//class MvrRobot;

/** Control the RVision camera pan tilt zoom unit.
   In addition to creating an MvrRvisionPTZ instance, you will also need
   to create an MvrSerialConnection object and open the serial port connection
   (the RVision is normally on COM3 on Seekur and Seekur Jr. robots) and
   use the setDeviceConnection() method to associate the serial connection
   with the MvrRVisionPTZ object.
	@since 2.7.0
*/

class MvrRVisionPTZ : public MvrPTZ
{
public:
  MVREXPORT MvrRVisionPTZ(MvrRobot *robot);
  MVREXPORT virtual ~MvrRVisionPTZ();
  
  MVREXPORT virtual bool init(void);
  MVREXPORT virtual const char *getTypeName() { return "rvision"; }
  /// Set serial port
  /// @since 2.7.6
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
  //MVREXPORT void getRealPanTilt(void);
  //MVREXPORT void getRealZoomPos(void);
  /*
  MVREXPORT virtual double getMaxPosPan(void) const { return MAX_PAN; }
  MVREXPORT virtual double getMaxNegPan(void) const { return MIN_PAN; }
  MVREXPORT virtual double getMaxPosTilt(void) const { return MAX_TILT; }
  MVREXPORT virtual double getMaxNegTilt(void) const { return MIN_TILT; }
  MVREXPORT virtual int getMaxZoom(void) const { return MAX_ZOOM; }
  MVREXPORT virtual int getMinZoom(void) const { return MIN_ZOOM; }
  */

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
  ///@since 2.7.6
  ///@internal
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

  ///@since 2.7.6
  static MvrPTZ* create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot);
  ///@since 2.7.6
  static MvrPTZConnector::GlobalPTZCreateFunc ourCreateFunc;

};

#endif // MVRRVISIONPTZ_H
