#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaUtil.h"
#include "MvrLog.h"
#include "MvrGPSCoords.h"

#include <math.h>

MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::mya = 6378137; // meters
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::myb = 6356752.3142; // meters
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::myep = 8.2094437949696e-2; 
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::myc = 299792458; // m/sec
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::mye = 8.1819190842622e-2;
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::my1byf = 298.257223563;
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::myOmega = 7292115e-11; // rad/sec
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::myGM = 3986004.418e8; // m^3/sec^2
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::myg = 9.7976432222; // m/sec^2. Ave g.
MVREXPORT MVRGPSCOORDS_CONSTANT double ArWGS84::myM = 5.9733328e24; // kg. Mass of earth.



MVREXPORT void Ar3DPoint::print(const char *head)
{
    char buffer[2064];
    if(head != NULL)
      sprintf(buffer, "%s ", head);
    sprintf(buffer, "%s %f %f %f\n", buffer, myX, myY, myZ);
    MvrLog::log(MvrLog::Normal, buffer);
}

/*!
 * Converts Earth Centered Earth Fixed coordinates into Latitude Longitude
 * Altitude coordinates.
 *
 */
MVREXPORT ArLLACoords
MvrECEFCoords::ECEF2LLA(void)
{
  // ECEF2LLA - convert earth-centered earth-fixed (ECEF)
  // cartesian coordinates to latitude, longitude,
  // and altitude
  //
  // USAGE:
  // [lat,lon,alt] = ecef2lla(x,y,z)
  //
  // lat = geodetic latitude (radians)
  // lon = longitude (radians)
  // alt = height above WGS84 ellipsoid (m)
  // x = ECEF X-coordinate (m)
  // y = ECEF Y-coordinate (m)
  // z = ECEF Z-coordinate (m)
  double x = myX;//(*this)(0);
  double y = myY;//(*this)(1);
  double z = myZ;//(*this)(2);
  //
  // Notes: (1) This function assumes the WGS84 model.
  // (2) Latitude is customary geodetic (not geocentric).
  // (3) Inputs may be scalars, vectors, or matrices of the same
  // size and shape. Outputs will have that same size and shape.
  // (4) Tested but no warranty; use at your own risk.
  // (5) Michael Kleder, April 2006

  // function [lat,lon,alt] = ecef2lla(x,y,z)

  // WGS84 ellipsoid constants:
  const double a = ArWGS84::getA();
  const double e = ArWGS84::getE();
  const double b = ArWGS84::getB();
  const double ep = ArWGS84::getEP();
  // Calculations.
  double p = sqrt(x*x + y*y);
  double th = atan2(a*z, b*p);
  double lon = atan2(y, x);
  double lat = atan2((z + ep*ep*b*pow(sin(th), 3)),
		     (p - e*e*a*pow(cos(th), 3)));
  double N = a / sqrt(1 - e*e*pow(sin(lat), 2));
  double alt = p / cos(lat) - N;

  // return lon in range [0,2*pi)
  if(lon < -M_PI)
    lon += 2*M_PI;

  lat *= 180.0/M_PI;
  lon *= 180.0/M_PI;
  // correct for numerical instability in altitude near exact poles:
  // (after this correction, error is about 2 millimeters, which isabout
  // the same as the numerical precision of the overall function)
  if(fabs(x) < 1 && fabs(y) < 1)
    alt = -b;

  return ArLLACoords(lat, lon, alt);
}
/*!
 * Converts Earth Centered Earth Fixed coordinates into the tangential 
 * East North Up coordinates with ref being the origin.
 *
 * @param ref: The ECEF coords of the origin.
 *
 */
MVREXPORT MvrENUCoords
MvrECEFCoords::ECEF2ENU(MvrECEFCoords ref)
{
  //    function [e,n,u] = xyz2enuTest(Xr, Yr, Zr, X, Y, Z)

  double Xr = ref.myX;//ref(0);
  double Yr = ref.myY;//ref(1);
  double Zr = ref.myZ;//ref(2);
  double dx = myX - Xr;//(*this)(0) - Xr;
  double dy = myY - Yr;//(*this)(1) - Yr;
  double dz = myZ - Zr;//(*this)(2) - Zr;

  // convert ECEF coordinates to local east, north, up

  double phiP = atan2(Zr, sqrt(Xr*Xr + Yr*Yr));

  double lambda = atan2(Yr, Xr);

  double e = -sin(lambda)*(dx) + cos(lambda)*(dy);

  double n = (-sin(phiP)*cos(lambda)*(dx) - 
	      sin(phiP)*sin(lambda)*(dy) + cos(phiP)*(dz));

  double u = (cos(phiP)*cos(lambda)*(dx) + 
	      cos(phiP)*sin(lambda)*(dy) + sin(phiP)*(dz)); 

  // Return in mm.
  return MvrENUCoords(e*1000.0, n*1000.0, u*1000.0);
}
/*!
 * Converts Latitude Longitude Altitude coordinates into Earth Centered
 * Earth Fixed coordinates.
 *
 */
MVREXPORT MvrECEFCoords
ArLLACoords::LLA2ECEF(void)
{
  // LLA2ECEF - convert latitude, longitude, and altitude to
  // earth-centered, earth-fixed (ECEF) cartesian
  //
  // USAGE:
  // [x,y,z] = lla2ecef(lat,lon,alt)
  //
  // x = ECEF X-coordinate (m)
  // y = ECEF Y-coordinate (m)
  // z = ECEF Z-coordinate (m)
  // lat = geodetic latitude (radians)
  // lon = longitude (radians)
  // alt = height above WGS84 ellipsoid (m)
  double lat = myX*M_PI/180.0;//(*this)(0)*M_PI/180.0;
  double lon = myY*M_PI/180.0;//(*this)(1)*M_PI/180.0;
  double alt = myZ;//(*this)(2);
  //
  // Notes: This function assumes the WGS84 model.
  // Latitude is customary geodetic (not geocentric).
  //
  // Source: "Department of Defense World Geodetic System 1984"
  // Page 4-4
  // National Imagery and Mapping Agency
  // Last updated June, 2004
  // NIMA TR8350.2
  //
  // Michael Kleder, July 2005

  //function [x,y,z]=lla2ecef(lat,lon,alt)

  // WGS84 ellipsoid constants:
  const double a = ArWGS84::getA();
  const double e = ArWGS84::getE();

  // intermediate calculation
  // (prime vertical radius of curvature)
  double N = a/ sqrt(1 - e*e * pow(sin(lat),2));

  // results:
  double x = (N + alt) * cos(lat) * cos(lon);
  double y = (N + alt) * cos(lat) * sin(lon);
  double z = ((1 - e*e) * N + alt) * sin(lat);

  return MvrECEFCoords(x, y, z);
}
/*!
 * Converts Tangential East North Up coordinates with ref as origin into
 * Earth Centered Earth Fixed Coordinates.
 *
 * @param ref: The coordinates of the origin.
 *
 */
MVREXPORT MvrECEFCoords
MvrENUCoords::ENU2ECEF(ArLLACoords ref)
{

  // function [X, Y, Z] = enu2xyz(refLat, refLong, refH, e, n, u)

  // Convert east, north, up coordinates (labelled e, n, u) to ECEF

  // coordinates. The reference point (phi, lambda, h) must be given. 
  // All distances are in metres

//  double refLat = ref(0)*M_PI/180.0;
//  double refLon = ref(1)*M_PI/180.0;
//  double refH = ref(2);
  double refLon = ref.getY()*M_PI/180.0;
  
  MvrECEFCoords refECEF = ref.LLA2ECEF();
  double Xr = refECEF.getX();//refECEF(0);
  double Yr = refECEF.getY();//refECEF(1);
  double Zr = refECEF.getZ();//refECEF(2);

  double phiP = atan2(Zr, sqrt(Xr*Xr + Yr*Yr)); // Geocentric latitude

  double e = myX/1000.0;//(*this)(0)/1000.0;
  double n = myY/1000.0;//(*this)(1)/1000.0;
  double u = myZ/1000.0;//(*this)(2)/1000.0;
  
  double X = (-sin(refLon)*e - cos(refLon)*sin(phiP)*n + 
	      cos(refLon)*cos(phiP)*u + Xr);

  double Y = (cos(refLon)*e - sin(refLon)*sin(phiP)*n + 
	      cos(phiP)*sin(refLon)*u + Yr);

  double Z = cos(phiP)*n + sin(phiP)*u + Zr; 

  return MvrECEFCoords(X, Y, Z);
}

/*!
 *  Actual function which does the conversion from LLA to Map Coords.
 *
 * @param lat: Latitude coords.
 * @param lon: Longitude coords.
 * @param alt: Altitude coords.
 * @param ea: East coords.
 * @param no: North coords.
 * @param up: Up coords.
 *
 * @return true if conversion is possible else false.
 *
 */
MVREXPORT bool
MvrMapGPSCoords::convertLLA2MapCoords(const double lat, const double lon, const double alt,
				     double& ea, double& no, double& up) const
{						    
  if(!myOriginSet)
    return false;

  ArLLACoords lla(lat, lon, alt);
  MvrECEFCoords ecef = lla.LLA2ECEF();
  MvrENUCoords enu = ecef.ECEF2ENU(*myOriginECEF);
//  MvrLog::log(MvrLog::Normal, "GPSLocaLog: convertLLA2MapCoords: ENU %g %g %g",
//	     enu.getX(), enu.getY(), enu.getZ());
  ea = enu.getX();//enu(0);
  no = enu.getY();//enu(1);
  up = enu.getZ();//enu(2);
//  MvrLog::log(MvrLog::Normal, 
//	     "GPSLocaLog: convertLLA2MapCoords: ENU + offset %g %g %g",
//	     ea, no, up);
  return true;
}
/*!
 *  Actual function which does the conversion from ENU coords to LLA coords.
 *
 * @param ea: East coords.
 * @param no: North coords.
 * @param up: Up coords.
 * @param lat: Latitude coords.
 * @param lon: Longitude coords.
 * @param alt: Altitude coords.
 *
 * @return true if conversion is possible else false.
 */
MVREXPORT bool
MvrMapGPSCoords::convertMap2LLACoords(const double ea, const double no, const double up,
				     double& lat, double& lon, double& alt) const
{						    
  if(!myOriginSet)
    return false;

  MvrENUCoords enu(ea, no, alt);
  MvrECEFCoords ecef = enu.ENU2ECEF(*myOriginLLA);
  ArLLACoords lla = ecef.ECEF2LLA();

//  MvrLog::log(MvrLog::Normal, "GPSLocaLog: convertMap2LLACoords: ENU %g %g %g",
//	     enu.getX(), enu.getY(), enu.getZ());  
  lat = lla.getX();//lla(0);
  lon = lla.getY();//lla(1);
  alt = lla.getZ();//lla(2);
  
//  MvrLog::log(MvrLog::Normal, "GPSLocaLog: convertMap2LLACoords: %g %g %g",
//	     lat, lon, alt);
  return true;
}
