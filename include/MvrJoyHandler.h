#ifndef MVRJOYHANDLER_H
#define MVRJOYHANDLER_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

#ifdef WIN32
#include <mmsystem.h> // for JOYINFO
#else // if not win32
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif
#ifdef linux
#include <linux/joystick.h>  // for JS_DATA_TYPE
#endif



/// Interfaces to a computer joystick
/** 
  This class is used to read data from a joystick device attached to the computer
  (usually via USB).
  The joystick handler keeps track of the minimum and maximums for both
  axes, updating them to constantly be better calibrated.  The speeds set 
  with setSpeed() influence what is returned by getAdjusted() or getDoubles().
    
  The joystick device is not opened until init() is called.  If there was
	an error connecting to the joystick device, it will return false, and
	haveJoystick() will return false. After calling
	init(), use getAdjusted() or getDoubles()
	to get values, and getButton() to check whether a button is pressed. setSpeed() may be
	called at any time to configure or reconfigure the range of the values returned by
	getAdjusted() and getDoubles().

	To get the raw data instead, use getUnfiltered() or getAxis().

	For example, if you want the X axis output to range from -1000 to 1000, and the Y axis
	output to range from -100 to 100, call <code>setSpeed(1000, 100);</code>.

    The X joystick axis is usually the left-right axis, and Y is forward-back.
  If a joystick has a Z axis, it is usually a "throttle" slider or dial on the
  joystick. The usual way to 
	drive a robot with a joystick is to use the X joystick axis for rotational velocity,
	and Y for translational velocity (note the robot coordinate system, this is its local
	X axis), and use the Z axis to adjust the robot's maximum driving speed.

  @ingroup OptionalClasses
*/
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
  MVREXPORT void getStats(int *maxX, int *minX, int *maxY, int *minY, 
		 int *cenX, int *cenY);
  /// Sets the stats for the joystick, useful for restoring calibrated settings
  MVREXPORT void setStats(int maxX, int minX, int maxY, int minY, 
		int cenX, int cenY);
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


#endif // ARJOYHANDLER_H

