/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrGPS.h"
#include "MvrDeviceConnection.h"
#include "MvrRobotPacket.h"
#include "MvrRobot.h"
#include "MvrCommands.h"
#include "mvriaInternal.h"

#include <iostream>


//#define DEBUG_ARGPS 1
//#define DEBUG_ARGPS_GPRMC

#ifdef DEBUG_ARGPS
void MvrGPS_printBuf(FILE *fp, const char *data, int size){  for(int i = 0; i < size; ++i)  {    if(data[i] < ' ' || data[i] > '~')    {      fprintf(fp, "[0x%X]", data[i] & 0xff);    }    else    {      fputc(data[i], fp);    }  }}
#endif

/* 
 * How to add support for new message handlers
 * -------------------------------------------
 *
 *  You can do this by modifying this class, or (recommended) creating a
 *  subclass of MvrGPS.
 *
 *  1. Create a handler method and functor for the NMEA message that provides
 *     the data. Initialize the functor in the class constructor.
 *
 *  2. Add the functor using addNMEAHandler() in the constructor.
 *
 *  3. Implement the handler method to examine the fields and extract the data (don't forget that 
 *     NMEA does not require that all fields be given).
 *
 *  4. Add the new GPS type to MvrGPSConnector.
 *
 * Some possible new NMEA message types to add are 
 * PTNLDG (Trimble proprietary
 * DGPS status information), and GPZDA (time of day information).
 *
 *
 *
 * How to add support for new GPS types:
 * -------------------------------------
 *
 * If your GPS device uses NMEA and does not require any special initialization
 * commands, then it will probably work with MvrGPS as a "Standard" GPS if you use the right BAUD rate. See
 * above for how to add support for new NMEA messages.
 *
 * If your GPS device does not support NMEA, or it requires special
 * initialization commands to start sending NMEA data etc., then you can
 * define a subclass of MvrGPS. Override connect(), setDeviceType(), and/or read() to do 
 * special things. See MvrNovatelGPS as an example. Then add support to
 * it to MvrGPSConnector: add a new member of the GPSType enum, a check for
 * it in parseArgs(), mention it in logArgs(), and create your MvrGPS subclass
 * in createGPS().
 *
 * You can find out the NMEA messages MvrGPS wants by accessing "myHandlers",
 * of type HandlersMap (a std::map).
 * 
 */



MVREXPORT MvrGPS::MvrGPS() :


  // objects
  myDevice(NULL),
  myNMEAParser("GPS"),

  // handler functors
  myGPRMCHandler(this, &MvrGPS::handleGPRMC),
  myGPGGAHandler(this, &MvrGPS::handleGPGGA),
  myPGRMEHandler(this, &MvrGPS::handlePGRME),
  myPGRMZHandler(this, &MvrGPS::handlePGRMZ),
  myHCHDxHandler(this, &MvrGPS::handleHCHDx),
  myGPGSAHandler(this, &MvrGPS::handleGPGSA),
  myGPGSVHandler(this, &MvrGPS::handleGPGSV),
  mySNRSum(0),
  mySNRNum(0),
  myGPMSSHandler(this, &MvrGPS::handleGPMSS),
  myGPGSTHandler(this, &MvrGPS::handleGPGST)
{
  addNMEAHandler("RMC", &myGPRMCHandler);
  addNMEAHandler("GGA", &myGPGGAHandler);
  addNMEAHandler("RME", &myPGRMEHandler);
  addNMEAHandler("RMZ", &myPGRMZHandler);
  addNMEAHandler("HDG", &myHCHDxHandler);
  addNMEAHandler("HDM", &myHCHDxHandler);
  addNMEAHandler("HDT", &myHCHDxHandler);
  //addNMEAHandler("HDG", &myHCHDxHandler);
  //addNMEAHandler("HDM", &myHCHDxHandler);
  //ddNMEAHandler("HDT", &myHCHDxHandler);
  addNMEAHandler("GSA", &myGPGSAHandler);
  addNMEAHandler("GSV", &myGPGSVHandler);
  addNMEAHandler("MSS", &myGPMSSHandler);
  addNMEAHandler("GST", &myGPGSTHandler);

  myMutex.setLogName("MvrGPS::myMutex");
}




MVREXPORT MvrGPS::Data::Data() :
  latitude(0.0),
  longitude(0.0),
  havePosition(false),
  speed(0.0),
  haveSpeed(false),
  fixType(NoFix), 
  numSatellitesTracked(0),
  altitude(0.0),
  haveAltitude(false),
  altimeter(0.0),
  haveAltimeter(false),
  DGPSStationID(0),
  haveDGPSStation(false),
  garminPositionError(0.0),
  haveGarminPositionError(false),
  garminVerticalPositionError(0.0),
  haveGarminVerticalPositionError(false),
  compassHeadingMag(0.0),
  compassHeadingTrue(0.0),
  haveCompassHeadingMag(false),
  haveCompassHeadingTrue(false),
  compassMagCounter(0),
  compassTrueCounter(0),
  haveHDOP(false), 
  HDOP(0.0),
  haveVDOP(false), 
  VDOP(0.0),
  havePDOP(false),
  PDOP(0.0),
  qualityFlag(false),
  meanSNR(0.0),
  haveSNR(false),
  beaconSignalStrength(0.0),
  beaconSNR(0.0),
  beaconFreq(0.0),
  beaconBPS(0),
  beaconChannel(0),
  haveBeaconInfo(false),
  inputsRMS(0.0),
  haveInputsRMS(false),
  haveErrorEllipse(false),
  haveLatLonError(false),
  altitudeError(0.0),
  haveAltitudeError(false)
{}



MVREXPORT bool MvrGPS::connect(unsigned long connectTimeout)
{
  if (!myDevice)
  {
    MvrLog::log(MvrLog::Terse, "GPS Error: Cannot connect, device connection invalid.");
    return false;
  }

  if (myDevice->getStatus() != MvrDeviceConnection::STATUS_OPEN) 
  {
    MvrLog::log(MvrLog::Terse, "GPS Error: Cannot connect, device connection not open.");
    return false;
  }

  if (!initDevice()) return false;

  MvrLog::log(MvrLog::Normal, "MvrGPS: Opened connection, waiting for initial data...");
  if(!waitForData(connectTimeout))
  {
    MvrLog::log(MvrLog::Terse, "MvrGPS: Error: No response from GPS after %dms.", connectTimeout);
    return false;
  }
  return true;
}

MVREXPORT bool MvrGPS::waitForData(unsigned long timeout)
{
  MvrTime start;
  start.setToNow();
  while ((unsigned long)start.mSecSince() <= timeout)
  {
    if (read(40) & ReadUpdated)  // read until data is sucessfully parsed 
      return true;
    MvrUtil::sleep(100);
  }
  return false;
}



MVREXPORT int MvrGPS::read(unsigned long maxTime)
{
  if (!myDevice) return ReadError;
  MvrTime startTime;
  startTime.setToNow();
  int result = 0;
  while(maxTime == 0 || startTime.mSecSince() < (long)maxTime) 
  {
    result |= myNMEAParser.parse(myDevice);
    if(result & ReadError || result & ReadFinished)
    {
#ifdef DEBUG_ARGPS
      std::cerr << "MvrGPS: finished reading all available data (or error reading).\n";
#endif
      return result;
    }
  }
#ifdef DEBUG_ARGPS
  if(maxTime != 0)
    fprintf(stderr, "MvrGPS::read() reached maxTime %lu (time=%lu), returning.\n", maxTime, startTime.mSecSince());
#endif
    
  return result;
}

// Key navigation data (position, etc.)
void MvrGPS::handleGPRMC(MvrNMEAParser::Message msg)
{
  parseGPRMC(msg, myData.latitude, myData.longitude, myData.qualityFlag, myData.havePosition, myData.timeGotPosition, myData.GPSPositionTimestamp, myData.haveSpeed, myData.speed);
}

void MvrGPS::parseGPRMC(const MvrNMEAParser::Message &msg, double &latitudeResult, double &longitudeResult, bool &qualityFlagResult, bool &gotPositionResult, MvrTime &timeGotPositionResult, MvrTime &gpsTimestampResult, bool &gotSpeedResult, double &speedResult)
{

  MvrNMEAParser::MessageVector *message = msg.message;
#if defined(DEBUG_ARGPS) || defined(DEBUG_ARGPS_GPRMC)
  fprintf(stderr, "MvrGPS: XXX GPRMC size=%d, stat=%s latDegMin=%s, latNS=%s, lonDegMin=%s, lonEW=%s\n", message->size(), 
    (message->size() > 2) ? (*message)[2].c_str() : "(missing)", 
    (message->size() > 3) ? (*message)[3].c_str() : "(missing)", 
    (message->size() > 4) ? (*message)[4].c_str() : "(missing)", 
    (message->size() > 5) ? (*message)[5].c_str() : "(missing)", 
    (message->size() > 6) ? (*message)[6].c_str() : "(missing)"
  );
#endif

  // Enough data?:
  if (message->size() < 3) return;

  // Data quality warning flag. Most GPS's use "V" when there's simply no fix, but
  // Trimble uses "V" when there's a GPS fix but num. satellites or DOP are
  // below some thresholds.
  bool flag = ((*message)[2] == "A");

  double lat, lon;

  if (!readFloatFromStringVec(message, 3, &lat, &gpsDegminToDegrees)) return;

  if (message->size() < 5) return;
  if ((*message)[4] == "S") lat *= -1;
  else if((*message)[4] != "N") return;  // bad value for field

  if (!readFloatFromStringVec(message, 5, &lon, &gpsDegminToDegrees)) return;

  if (message->size() < 7) return;
  if ((*message)[6] == "W") lon *= -1;
  else if((*message)[6] != "E") return; // bad value for field

  // Only set data after above stuff was properly parsed
  latitudeResult = lat;
  longitudeResult = lon;
  qualityFlagResult = flag;
  gotPositionResult = true;
  timeGotPositionResult = msg.timeParseStarted;

  // timestamp
  readTimeFromString((*message)[1], &gpsTimestampResult);

  // speed
  gotSpeedResult = readFloatFromStringVec(message, 7, &speedResult, &knotsToMPS);

}


// Fix type, number of satellites tracked, DOP and also maybe altitude
void MvrGPS::handleGPGGA(MvrNMEAParser::Message msg)
{
#ifdef DEBUG_ARGPS
fprintf(stderr, "MvrGPS: Got GPGGA\n");
#endif

  MvrNMEAParser::MessageVector *message = msg.message;
  if (message->size() < 7) return;
  switch((*message)[6].c_str()[0])
  {
    case '0':
      myData.fixType = BadFix;
      break;
    case '1': 
      myData.fixType = GPSFix;
      break;
    case '2':
    case '9':  // Novatel extension, means using WAAS
      myData.fixType = DGPSFix;
      break;
    case '3':
      myData.fixType = PPSFix;
      break;
    case '4':
      myData.fixType = RTKinFix;
      break;
    case '5':
      myData.fixType = FloatRTKinFix;
      break;
    case '6':
      myData.fixType = DeadReckFix;
      break;
    case '7': 
      myData.fixType = ManualFix;
      break;
    case '8':
      myData.fixType = SimulatedFix;
      break;
    default:
      myData.fixType = UnknownFixType;
  }
  
  readUShortFromStringVec(message, 7, &(myData.numSatellitesTracked));
  myData.haveHDOP = readFloatFromStringVec(message, 8, &myData.HDOP); // note redundant with GPGSA
  myData.haveAltitude = readFloatFromStringVec(message, 9, &myData.altitude); 
  // TODO get altitude geoidal seperation
  myData.haveDGPSStation = readUShortFromStringVec(message, 14, &myData.DGPSStationID);
}


// Error estimation in ground distance units (actually a proprietary message)
void MvrGPS::handlePGRME(MvrNMEAParser::Message msg)
{
  MvrNMEAParser::MessageVector *message = msg.message;
  myData.haveGarminPositionError = readFloatFromStringVec(message, 1, &myData.garminPositionError);
  myData.haveGarminVerticalPositionError = readFloatFromStringVec(message, 3, &myData.garminVerticalPositionError);
}

// Altitude (actually a Garmin proprietary message)
void MvrGPS::handlePGRMZ(MvrNMEAParser::Message msg)
{
  MvrNMEAParser::MessageVector *message = msg.message;
  // This is redundant with GPGGA and often a different value (plus the
  // conversion...) Favor this over that one, or separate into two values?
  // (this is specifically from an altimeter and the value in GGA is
  // from the satellite positions.)
  myData.haveAltimeter = readFloatFromStringVec(message, 1, &myData.altimeter);
  if (myData.haveAltimeter && message->size() >= 3 && strcasecmp((*message)[2].c_str(), "f") == 0)
    myData.altimeter = feetToMeters(myData.altimeter);
}

// Compass heading messages
void MvrGPS::handleHCHDx(MvrNMEAParser::Message msg)
{
  MvrNMEAParser::MessageVector *message = msg.message;
  if(msg.id == "HDT") // true north
  {
    myData.haveCompassHeadingTrue = readFloatFromStringVec(message, 1, &myData.compassHeadingTrue);
    if(myData.haveCompassHeadingTrue) ++(myData.compassTrueCounter);
  }

  if(msg.id == "HDM" || msg.id == "HDG")  // magnetic north
  {
    myData.haveCompassHeadingMag = readFloatFromStringVec(message, 1, &myData.compassHeadingMag);
    if(myData.haveCompassHeadingMag) ++(myData.compassMagCounter);
  }
}

// GPS DOP and satellite IDs
void MvrGPS::handleGPGSA(MvrNMEAParser::Message msg)
{
#ifdef DEBUG_ARGPS
fprintf(stderr, "MvrGPS: XXX GPGSA received\n");
#endif

  MvrNMEAParser::MessageVector *message = msg.message;
  // This message alse has satellite IDs, not sure if that information is
  // useful though.
  
  myData.havePDOP = readFloatFromStringVec(message, 15, &myData.PDOP);
  myData.haveHDOP = readFloatFromStringVec(message, 16, &myData.HDOP);
  myData.haveVDOP = readFloatFromStringVec(message, 17, &myData.VDOP);
}

MVREXPORT const char* MvrGPS::getFixTypeName() const 
{
  return getFixTypeName(getFixType());
}

MVREXPORT const char* MvrGPS::getFixTypeName(FixType type) 
{
  switch (type)
  {
    case NoFix: return "None";
    case BadFix: return "Bad";
    case GPSFix: return "GPS";
    case DGPSFix: return "DGPS";
    case PPSFix: return "PPS";
    case RTKinFix: return "Omnistar/RTK Converged fix";
    case FloatRTKinFix: return "Converging Omnistar/RTK float";
    case DeadReckFix: return "Dead Reckoning";
    case ManualFix: return "Manual";
    case SimulatedFix: return "Simulated";
    default: return "Unknown";
  }
}

MVREXPORT void MvrGPS::logData() const
{
  MvrLog::log(MvrLog::Normal, "GPS Fix=%s Num. Satellites=%d Mean SNR=%.4f", getFixTypeName(), getNumSatellitesTracked(), getMeanSNR());
  
  if (havePosition())
  {
    MvrLog::log(MvrLog::Normal, "GPS Latitude=%0.4fdeg Longitude=%0.4fdeg Timestamp=%d", getLatitude(), getLongitude(), getGPSPositionTimestamp().getMSec());
    // for  fun... 
    MvrLog::log(MvrLog::Normal, "GPS Maps: <http://www.topozone.com/map.asp?lat=%f&lon=%f&datum=nad83&u=5>  <http://maps.google.com/maps?q=%f,+%f>", getLatitude(), getLongitude(), getLatitude(), getLongitude());
  }
  
  if (haveSpeed())
    MvrLog::log(MvrLog::Normal, "GPS Speed=%0.4fm/s (%0.4fmi/h)", getSpeed(), mpsToMph(getSpeed()));

  if (haveAltitude())
    MvrLog::log(MvrLog::Normal, "GPS Altitude=%0.4fm (%0.4fft)", getAltitude(), metersToFeet(getAltitude()));

  if (haveCompassHeadingMag())
    MvrLog::log(MvrLog::Normal, "GPS Compass Heading (Mag)=%0.4fdeg", getCompassHeadingMag());

  if (haveCompassHeadingTrue())
    MvrLog::log(MvrLog::Normal, "GPS Compass Heading (True)=%0.4fdeg", getCompassHeadingTrue());

  if(haveErrorEllipse())
    MvrLog::log(MvrLog::Normal, "GPS Error Ellipse=%0.4fm X %0.4fm at %0.4fdeg", getErrorEllipse().getY(), getErrorEllipse().getX(), getErrorEllipse().getTh());

  if(haveLatLonError())
    MvrLog::log(MvrLog::Normal, "GPS Latitude Error=%0.4fm, Londitude Error=%0.4fm", getLatLonError().getX(), getLatLonError().getY());
  else if (haveGarminPositionError())
    MvrLog::log(MvrLog::Normal, "GPS Position Error Estimate=%0.4fm", getGarminPositionError());

  if(haveAltitudeError())
    MvrLog::log(MvrLog::Normal, "GPS Altitude Erro=%0.4fm", getAltitudeError());
  else if (haveGarminVerticalPositionError())
    MvrLog::log(MvrLog::Normal, "GPS Vertical Position Error Estimate=%0.4fm", getGarminVerticalPositionError());

  if (havePDOP())
    MvrLog::log(MvrLog::Normal, "GPS PDOP=%0.4f", getPDOP());
  if (haveHDOP())
    MvrLog::log(MvrLog::Normal, "GPS HDOP=%0.4f", getHDOP());
  if (haveVDOP())
    MvrLog::log(MvrLog::Normal, "GPS VDOP=%0.4f", getVDOP());

  if (haveDGPSStation())
    MvrLog::log(MvrLog::Normal, "GPS DGPS Station ID=%d", getDGPSStationID());

}

MVREXPORT void MvrGPS::printDataLabelsHeader() const 
{
    printf("Latitude Longitude Speed Altitude CompassHeadingMag/True NumSatellites AvgSNR Lat.Err Lon.Err Alt.Err HDOP VDOP PDOP Fix GPSTimeSec:MSec\n");
}

MVREXPORT void MvrGPS::printData(bool labels) const
{
  if(labels) printf("GPS: ");
  if (!havePosition())
  {
    if(labels) printf("Pos:- -");
    else printf("? ?");
  }
  else
  {
    if(labels) printf("Pos:% 2.6f % 2.6f", getLatitude(), getLongitude());
    else printf("%2.10f %2.10f", getLatitude(), getLongitude());
  }

  if (!haveAltitude()) 
  {
    if(labels) printf("   Alt:-");
    else printf(" ?");
  } 
  else 
  {
    if(labels) printf("   Alt:%4.2fm (%4.2fft)", getAltitude(), metersToFeet(getAltitude()));
    else printf(" %4.6f", getAltitude());
  }

  /*
  if (!haveCompassHeadingMag() && !haveCompassHeadingTrue()) 
  {
    if(labels) printf("   Compass:-/-");
    else printf(" ?/?");
  } 
  else 
  {
    if(haveCompassHeadingMag() && !haveCompassHeadingTrue()) 
    {
      if(labels) printf("   Compass:%3.1f/-", getCompassHeadingMag());
      else printf(" %.6f/?", getCompassHeadingMag());
    } 
    else if(!haveCompassHeadingMag() && haveCompassHeadingTrue()) 
    {
      if(labels) printf("   Compass:-/%3.1f",  getCompassHeadingTrue());
      else printf(" ?/%.6f", getCompassHeadingTrue());
    } 
    else 
    {
      if(labels) printf("   Compass:%3.1f/%3.1f", getCompassHeadingMag(), getCompassHeadingTrue());
      else printf(" %.6f/%.6f", getCompassHeadingMag(), getCompassHeadingTrue());
    }
  }
  */

  if(labels) printf("   NSats:%2d", getNumSatellitesTracked());
  else printf(" %2d", getNumSatellitesTracked());

  if(haveSNR())
  {
    if(labels) printf("   AvgSNR:%.4fdB", getMeanSNR());
    else printf(" %.4f", getMeanSNR());
  }
  else
  {
    if(labels) printf("   AvgSNR:-");
    else printf(" ?");
  }

  if (!haveLatLonError()) 
  {
    if(labels) printf("   LatErr:-    LonErr:-");
    else printf(" ? ?");
  } 
  else 
  {
    if(labels) printf("   LatErr:%2.4fm    LonErr:%2.4fm", getLatLonError().getX(), getLatLonError().getY());
    else printf(" %.16f %.16f", getLatLonError().getX(), getLatLonError().getY());
  }

  if (haveHDOP()) 
  {
    if (labels) printf("   HDOP:%2.4f", getHDOP());
    else printf(" %2.6f", getHDOP());
  } 
  else 
  {
    if(labels) printf("   HDOP:-");
    else printf(" ?");
  }

  if (haveVDOP()) 
  {
    if (labels) printf("   VDOP:%2.4f", getVDOP());
    else printf(" %2.6f", getVDOP());
  } 
  else 
  {
    if(labels) printf("   VDOP:-");
    else printf(" ?");
  }

  if (havePDOP()) 
  {
    if (labels) printf("   PDOP:%2.4f", getPDOP());
    else printf(" %2.6f", getPDOP());
  } 
  else 
  {
    if(labels) printf("   PDOP:-");
    else printf(" ?");
  }

  if(labels) printf("   Fix:%-10s", getFixTypeName());
  else printf(" %-10s", getFixTypeName());

  if(labels) printf("   (%lu:%lu)", getGPSPositionTimestamp().getSec(), getGPSPositionTimestamp().getMSec());
  else printf(" %lu:%lu", getGPSPositionTimestamp().getSec(), getGPSPositionTimestamp().getMSec());
}


double MvrGPS::gpsDegminToDegrees(double degmin) 
{
  double degrees;
  double minutes = modf(degmin / (double)100.0, &degrees) * (double)100.0;
  return degrees + (minutes / (double)60.0);
}


double MvrGPS::knotsToMPS(double knots) 
{
  return(knots * (double)0.514444444);
}


bool MvrGPS::readFloatFromString(const std::string& str, double* target, double (*convf)(double)) const
{
  if (str.length() == 0) return false;
  if (convf)
    *target = (*convf)(atof(str.c_str()));
  else
    *target = atof(str.c_str());
  return true;
}

bool MvrGPS::readUShortFromString(const std::string& str, unsigned short* target, unsigned short (*convf)(unsigned short)) const
{
  if (str.length() == 0) return false;
  if (convf)
    *target = (*convf)((unsigned short)atoi(str.c_str()));
  else
    *target = (unsigned short) atoi(str.c_str());
  return true;
}


bool MvrGPS::readFloatFromStringVec(const std::vector<std::string>* vec, size_t i, double* target, double (*convf)(double)) const
{
  if (vec->size() < (i+1)) return false;
  return readFloatFromString((*vec)[i], target, convf);
}

bool MvrGPS::readUShortFromStringVec(const std::vector<std::string>* vec, size_t i, unsigned short* target, unsigned short (*convf)(unsigned short)) const
{
  if (vec->size() < (i+1)) return false;
  return readUShortFromString((*vec)[i], target, convf);
}

bool MvrGPS::readTimeFromString(const std::string& s, MvrTime* time) const
{
  std::string::size_type dotpos = s.find('.');
  time_t timeSec = atoi(s.substr(0, dotpos).c_str());
  time_t timeMSec = 0;
  if(dotpos != std::string::npos)
    timeMSec = atoi(s.substr(dotpos+1).c_str()) * 100;
  time->setSec(timeSec);
  time->setMSec(timeMSec);
  return true;
}

void MvrGPS::handleGPGSV(MvrNMEAParser::Message msg)
{
  MvrNMEAParser::MessageVector *message = msg.message;
  if(message->size() < 8) return;
  unsigned short numMsgs;
  unsigned short thisMsg;
  if(!readUShortFromStringVec(message, 1, &numMsgs)) return;
  if(!readUShortFromStringVec(message, 2, &thisMsg)) return;
  for(unsigned short offset = 0; (MvrNMEAParser::MessageVector::size_type)(offset + 7) < message->size(); offset+=4) // should be less than 5 sets of data per message though
  {
    unsigned short snr = 0;
    if((*message)[7+offset].length() == 0) continue;  // no SNR for this satellite.
    if(!readUShortFromStringVec(message, offset+7, &snr)) break; // no more data avail.
    mySNRSum += snr;
    ++mySNRNum;
  }
  if(thisMsg == numMsgs) // last message in set
  {
    myData.meanSNR = (double)mySNRSum / (double)mySNRNum;
    myData.haveSNR = true;
    mySNRSum = 0;
    mySNRNum = 0;
  }
}

void MvrGPS::handleGPMSS(MvrNMEAParser::Message msg)
{
  MvrNMEAParser::MessageVector *message = msg.message;
  if(message->size() < 5) return;
  if(!readFloatFromStringVec(message, 1, &(myData.beaconSignalStrength))) return;
  if(!readFloatFromStringVec(message, 2, &(myData.beaconSNR))) return;
  if(!readFloatFromStringVec(message, 3, &(myData.beaconFreq))) return;
  if(!readUShortFromStringVec(message, 4, &(myData.beaconBPS))) return;
  if(!readUShortFromStringVec(message, 5, &(myData.beaconChannel))) return;
  myData.haveBeaconInfo = true;
}

void MvrGPS::handleGPGST(MvrNMEAParser::Message msg)
{
  MvrNMEAParser::MessageVector *message = msg.message;  
  // vector is:
  // 0,       1,    2,         3,             4,             5,              6,       7,       8
  // "GPGST", time, inputsRMS, ellipse major, ellipse minor, ellipse orient, lat err, lon err, alt err
#ifdef DEBUG_ARGPS
  printf("MvrGPS: XXX GPGST size=%d\n", message->size());
#endif
  if(message->size() < 3) return;
  myData.haveInputsRMS = readFloatFromStringVec(message, 2, &(myData.inputsRMS));
  if(message->size() < 6) return;
#ifdef DEBUG_ARGPS
  printf("MvrGPS: XXX GPGST inputsRMS=%s, ellipseMajor=%s, ellipseMinor=%s, ellipseOrient=%s\n", 
      (*message)[2].c_str(), (*message)[3].c_str(), (*message)[4].c_str(), (*message)[5].c_str());
#endif
  double major, minor, orient;
  myData.haveErrorEllipse = (
    readFloatFromStringVec(message, 3, &major)
    &&
    readFloatFromStringVec(message, 4, &minor)
    &&
    readFloatFromStringVec(message, 5, &orient)
  );
  if(myData.haveErrorEllipse) myData.errorEllipse.setPose(minor, major, orient);
  else myData.errorEllipse.setPose(0,0,0);
  if(message->size() < 7) return;
#ifdef DEBUG_ARGPS
  printf("MvrGPS: XXX GPGST latErr=%s, lonErr=%s\n",
      (*message)[6].c_str(), (*message)[7].c_str());
#endif
  double lat, lon;
  myData.haveLatLonError = (
    readFloatFromStringVec(message, 6, &lat)
    &&
    readFloatFromStringVec(message, 7, &lon)
  );
//printf("MvrGPS: XXX GPGST haveLLE=%d, latErr=%f, lonErr=%f\n", myData.haveLatLonError, lat, lon);
  if(myData.haveLatLonError) myData.latLonError.setPose(lat, lon);
  else myData.latLonError.setPose(0,0,0);
//printf("MvrGPS: XXX GPGST lle.getX=%f, lle.getY=%f\n", myData.latLonError.getX(), myData.latLonError.getY());
  if(message->size() < 9) return;
#ifdef DEBUG_ARGPS
  printf("MvrGPS: XXX GPGST altErr=%s", (*message)[8].c_str());
#endif
  myData.haveAltitudeError = readFloatFromStringVec(message, 8, &(myData.altitudeError));
}
  
MVREXPORT MvrSimulatedGPS::MvrSimulatedGPS(MvrRobot *robot) :
    MvrGPS(), myHaveDummyPosition(false), mySimStatHandlerCB(this, &MvrSimulatedGPS::handleSimStatPacket),
    myRobot(robot)
  {
    myData.havePosition = false;
    myData.fixType = NoFix;   // need to set a position with setDummyPosition() or get data from MobileSim to get a (simulated) fix
  }

MVREXPORT void MvrSimulatedGPS::setDummyPosition(MvrArgumentBuilder *args)
{
  double lat = 0;
  double lon = 0;
  double alt = 0;
  bool haveArg = false;
  lat = args->getArgDouble(0, &haveArg);
  if(!haveArg) {
    MvrLog::log(MvrLog::Terse, "MvrSimulatedGPS: Can't set dummy position: No valid double precision numeric value given as first argument for latitude.");
    return;
  }
  lon = args->getArgDouble(1, &haveArg);
  if(!haveArg)  {
    MvrLog::log(MvrLog::Terse, "MvrSimulatedGPS: Can't set dummy position: No valid double precision numeric value given as second argument for longitude.");
    return;
  }
  alt = args->getArgDouble(2, &haveArg);
  if(haveArg) {
    MvrLog::log(MvrLog::Normal, "MvrSimulatedGPS: Setting dummy position %f, %f, %f", lat, lon, alt);
    setDummyPosition(lat, lon, alt);
  } else {
    MvrLog::log(MvrLog::Normal, "MvrSimulatedGPS: Setting dummy position %f, %f", lat, lon);
    setDummyPosition(lat, lon);
  }
}

bool MvrSimulatedGPS::handleSimStatPacket(MvrRobotPacket *pkt)
{
  if(pkt->getID() != 0x62) return false;
  //puts("SIMSTAT");
  /*char c =*/ pkt->bufToByte(); // skip
  /*c =*/ pkt->bufToByte(); // skip
  MvrTypes::UByte4 flags = pkt->bufToUByte4();
  if(flags&MvrUtil::BIT1)   // bit 1 is set if map has OriginLLA georeference point, and this packet will contain latitude and longitude.
  {
    myData.timeGotPosition.setToNow();
    //myData.numSatellitesTracked = 6;
    myData.numSatellitesTracked = 0;
    pkt->bufToUByte2(); // skip simint
    pkt->bufToUByte2(); // skip realint
    pkt->bufToUByte2(); // skip lastint
    pkt->bufToByte4(); // skip truex
    pkt->bufToByte4(); // skip truey
    pkt->bufToByte4(); // skip truez
    pkt->bufToByte4(); // skip trueth
    // TODO check if packet is still long enough to contain latitude and longitude.
    myData.havePosition = true;
    myData.latitude = pkt->bufToByte4() / 10e6; 
    myData.longitude = pkt->bufToByte4() / 10e6; 
    myData.GPSPositionTimestamp.setToNow();
    // TODO check if packet is still long enough to contain altitude
    myData.haveAltitude = true;
    myData.altitude = pkt->bufToByte4() / 100.0;
    // TODO check if packet is still long enough to contain dop
    int d = pkt->bufToByte();
    if(d == -1) {
      myData.fixType = NoFix;
      myData.haveHDOP = myData.haveVDOP = myData.havePDOP = false;
      myData.HDOP = myData.VDOP = myData.PDOP = 0.0;
    } else if(d == 0) {
      myData.fixType = BadFix;
      myData.haveHDOP = myData.haveVDOP = myData.havePDOP = true;
      myData.HDOP = myData.VDOP = myData.PDOP = 0.0;
    } else {
      myData.fixType = SimulatedFix;
      myData.haveHDOP = myData.haveVDOP = myData.havePDOP = true;
      myData.HDOP = myData.VDOP = myData.PDOP = (double)d/100.0;
    }
      
  }
  else
  {
   // puts("no GPS data in SIMSTAT packet");
    if(myData.havePosition && !myHaveDummyPosition)
      clearPosition();
  }
  return true;
}

MVREXPORT MvrSimulatedGPS::~MvrSimulatedGPS()
{
  if(myRobot)
    myRobot->remPacketHandler(&mySimStatHandlerCB);
}

bool MvrSimulatedGPS::connect(unsigned long connectTimeout) 
{
  /*
  std::list<MvrRobot*> *robots = Mvria::getRobotList();
  std::list<MvrRobot*>::const_iterator first = robots->begin();
  if(first != robots->end())
    myRobot = *(first);
  */
  if(myRobot)
  {
    myRobot->addPacketHandler(&mySimStatHandlerCB);
    MvrLog::log(MvrLog::Normal, "MvrSimulatedGPS: Requesting data from the simulated robot.");
    myRobot->comInt(MvrCommands::SIM_STAT, 2);
  }
  else
  {
    MvrLog::log(MvrLog::Normal, "MvrSimulatedGPS: Have no robot connection, can't receive data from a simulated robot; dummy position must be set manually instead");
  }
  return true;
}
