/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrJoyHandler.h
 > Description  : Interfaces to a computer joystick
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年05月23日
***************************************************************************************************/
#ifndef MVRJOYHANDLER_H
#define MVRJOYHANDLER_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

#ifdef WIN32
#include <mmsystem.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif  //WIN32
#ifdef linux
#include <linux/joystick.h>  // For JS_DATA_TYPEs
#endif //linux

class MvrJoyHandler
{
 public:
  /// Constructor
  MVREXPORT MvrJoyHandler(bool useOSCal = true, bool useOldJoystick = false);
  /// Destructor
  MVREXPORT ~MvrJoyHandler();
  /// Intializes the joystick, returns true if successful
  MVREXPORT bool init(void);
  /// Returns if the joystick was successfully initialized or not
  bool haveJoystick(void) { return myInitialized; }
  /// Gets the adjusted reading, as floats, between -1.0 and 1.0
  MVREXPORT void getDoubles(double *x, double *y, double *z = NULL);
  /// Gets the button 
  MVREXPORT bool getButton(unsigned int button);
  /// Returns true if we definitely have a Z axis (we don't know in windows unless it moves)
  bool haveZAxis(void) { return myHaveZ; }

  /// Sets the maximums for the x, y and optionally, z axes.
  void setSpeeds(int x, int y, int z = 0) 
    { myTopX = x; myTopY = y; myTopZ = z; }
  /// Gets the adjusted reading, as integers, based on the setSpeed
  MVREXPORT void getAdjusted(int *x, int *y, int *z = NULL);

  /// Gets the number of axes the joystick has
  MVREXPORT unsigned int getNumAxes(void);
  /// Gets the floating (-1 to 1) location of the given joystick axis
  MVREXPORT double getAxis(unsigned int axis);
  /// Gets the number of buttons the joystick has
  MVREXPORT unsigned int getNumButtons(void);

  /// Sets whether to just use OS calibration or not
  MVREXPORT void setUseOSCal(bool useOSCal);
  /// Gets whether to just use OS calibration or not
  MVREXPORT bool getUseOSCal(void);
  /// Starts the calibration process
  MVREXPORT void startCal(void);
  /// Ends the calibration process
  MVREXPORT void endCal(void);
  /// Gets the unfilitered reading, mostly for internal use, maybe
  /// useful for Calibration
  MVREXPORT void getUnfiltered(int *x, int *y, int *z = NULL);
  /// Gets the stats for the joystick, useful after calibrating to save values
  MVREXPORT void getStats(int *maxX, int *minX, int *maxY, int *minY, int *cenX, int *cenY);
  /// Sets the stats for the joystick, useful for restoring calibrated settings
  MVREXPORT void setStats(int maxX, int minX, int maxY, int minY, int cenX, int cenY);
  /// Gets the maximums for each axis.
  MVREXPORT void getSpeeds(int *x, int *y, int *z);

 protected:
  // function to get the data for OS dependent part
  void getData(void);
  int myMaxX, myMinX, myMaxY, myMinY, myCenX, myCenY, myTopX, myTopY, myTopZ;
  bool myHaveZ;

  std::map<unsigned int, int> myAxes;
  std::map<unsigned int, bool> myButtons;

  int myPhysMax;
  bool myInitialized;
  bool myUseOSCal;
  bool myUseOld;
  bool myFirstData;
  MvrTime myLastDataGathered;
#ifdef WIN32
  unsigned int myJoyID;
  int myLastZ;
  JOYINFO myJoyInfo;
  JOYCAPS myJoyCaps;
#else // if not win32
  int myJoyNumber;
  char myJoyNameTemp[512];
  MvrTime myLastOpenTry;
  void getOldData(void);
  void getNewData(void);
  #ifdef linux 
  struct JS_DATA_TYPE myJoyData; // structure for the buttons and x,y coords
  #else
  int myJoyData;
  #endif
  FILE * myOldJoyDesc;
  int myJoyDesc;
#endif // linux
};
#endif  // MVRJOYHANDLER_H