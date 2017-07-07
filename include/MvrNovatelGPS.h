#ifndef MVRNOVATELGPS_H
#define MVRNOVATELGPS_H


#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "mvriaUtil.h"
#include "MvrGPS.h"

/** @brief GPS subclass to support the NovAtel ProPak G2 and other NovAtel
 * "OEM4" devices.
 *  
 *  This subclass overrides initDevice() to send initialization commands specific
 *  to NovAtel GPS devices.
 *  On connect, it initiazes SBAS (satellite-based augmentation) to
 *  "auto", which uses WAAS, EGNOS, or MSAS if available,
 *  and then requests NMEA messages to be interpreted by MvrGPS.
 *
 *  It does not initialize corrections sent by your own DGPS fixed
 *  base station over radio, or the wide-area L-Band services such as OmniStar or 
 *  the Canada-wide DGPS (supported by the NovaTel ProPak LBplus). 
 *  To configure DGPS, you must connect to the GPS with a terminal
 *  program (e.g. "minicom" in Linux), configure the device for
 *  your specific service, region and radio settings, and save 
 *  with the "saveconfig" command. See the GPS setup notes 
 *  at http://robots.mobilerobots.com/tech_notes/GPS_Setup_Notes.txt
 *  and the NovAtel GPS Reference Manual Volume 1, Sections 4.3 and 4.5 for
 *  ("Transmitting and Receiving Corrections" and "Enabling L-Band Positioning")
 *  for more information on doing this.
 *
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

    @since ARIA 2.7.2
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

#endif 


