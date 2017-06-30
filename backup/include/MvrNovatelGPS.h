/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrNovatelGPS.h
 > Description  : GPS subclass to support the NovAtel ProPak G2 and other NovAtel "OEM4" devices.
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/
#ifndef MVRNOVATELGPS_H
#define MVRNOVATELGPS_H


#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "mvriaUtil.h"
#include "MvrGPS.h"

/*  This subclass overrides initDevice() to send initialization commands specific
 *  to NovAtel GPS devices.
 *  On connect, it initiazes SBAS (satellite-based augmentation) to
 *  "auto", which uses WAAS, EGNOS, or MSAS if available,
 *  and then requests NMEA messages to be interpreted by MvrGPS.
 */
class MvrNovatelGPS : public virtual MvrGPS {
protected:
  void handleNovatelGPGGA(MvrNMEAParser::Message msg);
  MvrFunctor1C<MvrNovatelGPS, MvrNMEAParser::Message> myNovatelGPGGAHandler;
public:
  MVREXPORT MvrNovatelGPS();
  MVREXPORT virtual ~MvrNovatelGPS();
protected:
  MVREXPORT virtual bool initDevice();
};

/** @brief GPS subclass to support the NovAtel SPAN GPS with integrated IMU. 

    This subclass requests the INGLL NMEA message for combined IMU and GPS
    position, and updates the GPS position in MvrGPS using this data.
    It replaces the normal GPRMC handler from MvrGPS, and instead saves the uncorrected
    GPS data separately. (use dynamic_cast to cast an MvrGPS object to an
    MvrNovatelSPAN object to access this data).
*/
class MvrNovatelSPAN : public virtual MvrNovatelGPS {
protected:
  /** overrides MvrNovatelGPS::handleGPRMC(), and keeps results of parsing the
  * message in this class rather than MvrGPS. */
  void handleGPRMC(MvrNMEAParser::Message msg);
  void handleINGLL(MvrNMEAParser::Message msg);
  MVREXPORT virtual bool initDevice();
  MvrFunctor1C<MvrNovatelSPAN, MvrNMEAParser::Message> myGPRMCHandler;
  MvrFunctor1C<MvrNovatelSPAN, MvrNMEAParser::Message> myINGLLHandler;
public:
  double GPSLatitude, GPSLongitude;
  bool haveGPSPosition, GPSValidFlag;
  MvrTime timeGotGPSPosition, GPSTimestamp;
  MVREXPORT MvrNovatelSPAN();
  MVREXPORT virtual ~MvrNovatelSPAN();
};

#endif  // MVRNOVATELGPS_H


