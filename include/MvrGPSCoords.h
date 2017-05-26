/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrGPSCoords.h
 > Description  : Base class for points in 3 dimensional cartesian space. 
 > Author       : Yu Jie
 > Create Time  : 2017年05月20日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRGPSCOORDS_H
#define MVRGPSCOORDS_H

#include "mvriaTypedefs.h"

class Mvr3DPoint;
class MvrLLACoords;
class MvrECEFCoords;
class MvrENUCoords;
class MvrWGS84;

class Mvr3DPoint 
{
public:
  Mvr3DPoint(void) : myX(0), myY(0), myZ(0) {}
  Mvr3DPoint(double x, double y, double z) : myX(x), myY(y), myZ(z) {}
  /// Destructor.
  ~Mvr3DPoint() {}
  /// Add
  Mvr3DPoint operator+(Mvr3DPoint c)
  {
    Mvr3DPoint sum(myX + c.myX, myY + c.myY, myZ + c.myZ);
    return sum;
  }
  /// Diff
  Mvr3DPoint operator-(Mvr3DPoint c)
  {
    Mvr3DPoint dif(myX - c.myX, myY - c.myY, myZ - c.myZ);
    return dif;
  }
  /// Mult
  Mvr3DPoint operator*(double c)
  {
    Mvr3DPoint pro(myX*c, myY*c, myZ*c);
    return pro;
  }
  /// Dot product
  double dot(Mvr3DPoint c)
  {
    double dotP = myX * c.myX + myY * c.myY + myZ * c.myZ;
    return dotP;
  }
  /// Cross product
  Mvr3DPoint cross(Mvr3DPoint c)
  {
    Mvr3DPoint crossP;
    crossP.myX = myY * c.myZ - myZ * c.myY;
    crossP.myY = myZ * c.myX - myX * c.myZ;
    crossP.myZ = myX * c.myY - myY * c.myX;
    return crossP;
  }
  /// Print.
  MVREXPORT void print(const char* head=NULL);

  double getX() const {return myX;}
  double getY() const {return myY;}
  double getZ() const {return myZ;}
  void setX(double x) { myX = x; }
  void setY(double y) { myY = y; }
  void setZ(double z) { myZ = z; }

protected:

  double myX;
  double myY;
  double myZ;

};

#ifdef WIN32
// Need to export some variables on Windows because they are used in inline methods (which is good), but they can't be exported if const.
#define ARGPSCOORDS_CONSTANT 
#else
#define ARGPSCOORDS_CONSTANT const
#endif


/**
 * All the constants defined by the World Geodetic System 1984.
 * @ingroup UtilityClasses
 */
class MvrWGS84
{
  public:
  MvrWGS84(void) {}
  
  static double getE()     {return mye;}
  static double getA()     {return mya;}
  static double getB()     {return myb;}
  static double getEP()    {return myep;}
  static double get1byf()  {return my1byf;}
  static double getOmega() {return myOmega;}
  static double getGM()    {return myGM;}
  

private:
  MVREXPORT static ARGPSCOORDS_CONSTANT double mya;     // meters
  MVREXPORT static ARGPSCOORDS_CONSTANT double myb;     // meters
  MVREXPORT static ARGPSCOORDS_CONSTANT double myep; 
  MVREXPORT static ARGPSCOORDS_CONSTANT double myc;     // m/sec
  MVREXPORT static ARGPSCOORDS_CONSTANT double mye;
  MVREXPORT static ARGPSCOORDS_CONSTANT double my1byf;
  MVREXPORT static ARGPSCOORDS_CONSTANT double myOmega; // rad/sec
  MVREXPORT static ARGPSCOORDS_CONSTANT double myGM;    // m^3/sec^2
  MVREXPORT static ARGPSCOORDS_CONSTANT double myg;     // m/sec^2. Ave g.
  MVREXPORT static ARGPSCOORDS_CONSTANT double myM;     // kg. Mass of earth.
};


/**
 * Earth Centered Earth Fixed Coordinates.
   @ingroup UtilityClasses
 */
class MvrECEFCoords : public Mvr3DPoint
{
  public:
  MvrECEFCoords(double x, double y, double z) : Mvr3DPoint(x, y, z) {}
  MVREXPORT MvrLLACoords ECEF2LLA(void);
  MVREXPORT MvrENUCoords ECEF2ENU(MvrECEFCoords ref);
};

/**
 * Latitude, Longitude and Altitude Coordinates.
 * @ingroup UtilityClasses
 */
class MvrLLACoords : public Mvr3DPoint
{
  public:
  MvrLLACoords(void) : Mvr3DPoint(0, 0, 0) {}
  MvrLLACoords(double x, double y, double z) : Mvr3DPoint(x, y, z) {}
  MvrLLACoords(MvrPose pos, double alt) : Mvr3DPoint(pos.getX(), pos.getY(), alt) {}
  MVREXPORT MvrECEFCoords LLA2ECEF(void);
  double getLatitude(void) const {return getX();}
  double getLongitude(void) const {return getY();}
  double getAltitude(void) const {return getZ();}
  void setLatitude(double l) { setX(l); }
  void setLongitude(double l) { setY(l); }
  void setAltitude(double a) { setZ(a); }
};

/**
 * East North Up coordinates.
   @ingroup UtilityClasses
 */
class MvrENUCoords : public Mvr3DPoint
{
  public:
  MvrENUCoords(double x, double y, double z) : Mvr3DPoint(x, y, z) {}
  MVREXPORT MvrECEFCoords ENU2ECEF(MvrLLACoords ref);
  double getEast(void) const {return getX();}
  double getNorth(void) const {return getY();}
  double getUp(void) const {return getZ();}
  void setEast(double e) { setX(e); }
  void setNorth(double n) { setY(n); }
  void setUp(double u) { setZ(u); }
};

/**
 * Coordinates based on a map with origin in LLA coords with conversion
 * methods from LLA to ENU and from ENU to LLA coordinates.
 * @ingroup UtilityClasses
 */
class MvrMapGPSCoords : public MvrENUCoords
{
  public:
  MvrMapGPSCoords(MvrLLACoords org) : MvrENUCoords(0.0, 0.0, 0.0), myOriginECEF(0), myOriginLLA(0), myOriginSet(false) 
  {
    setOrigin(org);
  }
  MvrMapGPSCoords() : MvrENUCoords(0, 0, 0), myOriginECEF(0), myOriginLLA(0), myOriginSet(false)
  {
  }
  MVREXPORT bool convertMap2LLACoords(const double ea, const double no, const double up,
			    double& lat, double& lon, double& alt) const;
  MVREXPORT bool convertLLA2MapCoords(const double lat, const double lon, const double alt,
			    double& ea, double& no, double& up) const;
  bool convertLLA2MapCoords(const MvrLLACoords& lla, double& ea, double& no, double& up)
  {
    return convertLLA2MapCoords(lla.getLatitude(), lla.getLongitude(), lla.getAltitude(), ea, no, up);
  }
  void setOrigin(MvrLLACoords org) 
  {
    if(myOriginLLA)
      delete myOriginLLA;
    if(myOriginECEF)
      delete myOriginECEF;
    myOriginSet = true;
    myOriginLLA = new MvrLLACoords(org);
    myOriginECEF = new MvrECEFCoords(myOriginLLA->LLA2ECEF());
  }
     
  MvrECEFCoords* myOriginECEF;
  MvrLLACoords* myOriginLLA;
  bool myOriginSet;
};

#endif // MVRGPSCOORDS_H
