/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrJoyHandler.h
 > Description  : Interfaces to a computer joystick
 > Author       : Yu Jie
 > Create Time  : 2017年05月23日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#include "mvriaOSDef.h"
#include "MvrExport.h"
#include "mvriaUtil.h"
#include "MvrJoyHandler.h"

#include <errno.h>

/*
 * @param useOSCal if this is set then the joystick will just rely on
 * the OS to calibrate, otherwise it will keep track of center min and
 * max and use those values for calibration
 * @param useOld use the old linux interface to the joystick
 */
MVREXPORT MvrJoyHandler::MvrJoyHandler(bool useOSCal, bool useOld)
{
  myInitialized = false;
  myUseOSCal    = useOSCal;
  myUseOld      = useOld;
  myHaveZ       = false;
  myFirstData   = true;
}

MVREXPORT MvrJoyHandler::~MvrJoyHandler()
{

}

MVREXPORT void MvrJoyHandler::setUseOSCal(bool useOSCal)
{
  myUseOSCal = useOSCal;
}

MVREXPORT bool MvrJoyHandler::getUseOSCal(void)
{
  return myUseOSCal;
}

/*
 * Starts the calibration, which resets all the min and max variables as well
 * as the center variables.
 */
MVREXPORT void MvrJoyHandler::startCal(void)
{
  int x,y;
  getUnfiltered(&x, &y);
  myMaxX = x;
  myMinX = x;
  myMaxY = y;
  myMinX = y;
  myCenX = x;
  myCenY = y;
}

/* 
 * Ends the calibration, which also sets the center to where the joystick is
 * when the function is called... the center is never reset except in this 
 * function, whereas the min and maxes are constantly checked
 */
MVREXPORT void MvrJoyHandler::endCal(void)
{
  int x, y;

  getUnfiltered(&x, &y);
  myCenX = x;
  myCenY = y;
}

MVREXPORT void MvrJoyHandler::getStats(int *maxX, int *minX, int *maxY, int *minY, int *cenX, int *cenY)
{
  *maxX = myMaxX;
  *minX = myMinX;
  *maxY = myMaxY;
  *minY = myMinY;
  *cenX = myCenX;
  *cenY = myCenY;  
}

MVREXPORT void MvrJoyHandler::setStats(int maxX, int minX, int maxY, int minY, int cenX, int cenY)
{
  myMaxX = maxX;
  myMinX = minX;
  myMaxY = maxY;
  myMinY = minY;
  myCenX = cenX;
  myCenY = cenY;  
}

MVREXPORT void MvrJoyHandler::getSpeeds(int *x, int *y, int *z)
{
  *x = myTopX;
  *y = myTopY;
  if (z != NULL)
    *z = myTopZ;
}

/*
 * if useOSCal is true then this returns the readings as calibrated
 * from the OS.  If useOSCal is false this finds the percentage of the
 * distance between center and max (or min) then takes this percentage
 * and multiplies it by the speeds given the class, and returns the
 * values computed from this.
   
 * @param x pointer to an integer in which to store the x value, which
 * will be within the range [-1 * x given in setSpeeds(), x given in setSpeeds()]
 * @param y pointer to an integer in which to store the y value, which
 * will be within the range [-1 * y given in setSpeeds(), y given in setSpeeds()]
 * @param z pointer to an integer in which to store the z value, which
 * will be within the range [-1 * z given in setSpeeds(), z given in setSpeeds()]
 * @java   Use the version of this method returning MvrJoyVec3i
 * @python returns a tuple of (x, y, z) instead
 */
MVREXPORT void MvrJoyHandler::getAdjusted(int *x, int *y, int *z)
{
  int curX, curY, curZ;

  getUnfiltered(&curX, &curY, &curZ);
  if (myUseOld)
  {
    *x = MvrMath::roundInt(((double)curX) / 128.0 * ((double) myTopX));
    *y = MvrMath::roundInt(((double)curY) / 128.0 * ((double) myTopY));
    if (z != NULL)
      *z = MvrMath::roundInt(((double)curZ) / 128.0 * ((double) myTopZ));
    return;
  }

  if (curX > myCenX && myMaxX - myCenX != 0 ) 
  {
    *x = (int)((double)(curX - myCenX)/(double)(myMaxX - myCenX)*(double)myTopX);
  }
  else if (curX <= myCenX && myCenX - myMinX != 0) 
  {
    *x = (int)((double)(myCenX - curX)/(double)(myCenX - myMinX)*(double)-myTopX);
  } 
  else
    *x = 0;
  if (curY > myCenY && myMaxY - myCenY != 0) 
  {
    *y = (int)((double)(curY - myCenY)/(double)(myMaxY - myCenY)*(double)myTopY);
  } 
  else if (curY <= myCenY && myCenY - myMinY != 0) 
  {
    *y = (int)((double)(myCenY - curY)/(double)(myCenY - myMinY)*(double)-myTopY);
  } 
  else 
    *y = 0;
  if (z != NULL)
    *z = MvrMath::roundInt(((double)curZ) / 128.0 * ((double)myTopZ));
}

/*
 * If useOSCal is true then this gets normalized values 
 * (between -1.0 and 1.0)  as calibrated
 * by the OS. In other words, the range [-1.0,1.0] is scaled
 * to the calibrated range determined by the set maximum.   If useOSCal is false this finds the percentage of the
 * distance between center and max (or min) then takes this percentage
 * and multiplies it by the speeds given the class, and returns the
 * values computed from this.
   
 * @param x pointer to an integer in which to store the x value.
 * Will be within the range [-1.0, 1.0]
 * @param y pointer to an integer in which to store the y value.
 * Will be within the range [-1.0, 1.0]
 * @param z pointer to an integer in which to store the z value.
 * Will be within the range [-1.0, 1.0]
 * @python Returns a tuple of (x, y, z) instead
 * @java   Use the version of this method returning MvrJoyVec3f
 * @python Returns a tuple of (x, y, z) instead
 */
MVREXPORT void MvrJoyHandler::getDoubles(double *x, double *y, double *z)
{
  int curX, curY, curZ;

  getUnfiltered(&curX, &curY, &curZ);
  if (myUseOSCal)

  {
    *x = ((double)curX) / 128.0;
    *y = ((double)curY) / 128.0;
    if (z != NULL)
      *z = ((double)curZ) / 128.0;
    return;
  }

  if (curX > myCenX && myMaxX - myCenX != 0 ) 
  {
    *x = (int)((double)(curX - myCenX)/(double)(myMaxX - myCenX));
  }
  else if (curX <= myCenX && myCenX - myMinX != 0) 
  {
    *x = (int)((double)(myCenX - curX)/(double)(myCenX - myMinX));
  } 
  else
    *x = 0;
  if (curY > myCenY && myMaxY - myCenY != 0) 
  {
    *y = (int)((double)(curY - myCenY)/(double)(myMaxY - myCenY));
  } 
  else if (curY <= myCenY && myCenY - myMinY != 0) 
  {
    *y = (int)((double)(myCenY - curY)/(double)(myCenY - myMinY));
  } 
  else 
    *y = 0;
  if (z != NULL)
    *z = curZ;
}

/*
 * This returns the raw value from the joystick... with X and Y 
 * varying between -128 and 128. This data normally shouldn't be used 
 * except in calibration since it can give very uncentered or inconsistent readings.  
 * For example its not uncommon for a joystick to move 10 to the right
 * but 50 or 100 to the left, so if you aren't adjusting for this you get
 * a robot (or whatever) that goes left really fast, but will hardly go right.
 * Instead you should use getAdjusted() exclusively except for calibration,
 * or informational purposes.
 * @param x pointer to an integer in which to store x value
 * @param y pointer to an integer in which to store y value
 * @param z pointer to an integer in which to store z value
 * @java   Use the version of this method returning MvrJoyVec3i
 * @python Returns a tuple of (x, y, z) instead
 */
MVREXPORT void MvrJoyHandler::getUnfiltered(int *x, int *y, int *z)
{
  getData();
  *x = myAxes[1];
  *y = myAxes[2];
  if (z != NULL)
    *z = myAxes[3];
} 

/*
 * @param axis axis to get, should range from 1 through getNumAxes()
 */
MVREXPORT double MvrJoyHandler::getAxis(unsigned int axis)
{
  if (axis < 1 || axis > myAxes.size())
    return 0;
  
  std::map<unsigned int, int>::iterator iter = myAxes.find(axis);
  if (iter != myAxes.end())
  {
    return (iter->second) / 128.0;
  }
  else
    return 0;
}

/*
 * @param button button to test for pressed, within the range 1 through
 * getNumButtons()
 
 * @return true if the button is pressed, false otherwise
 */
MVREXPORT bool MvrJoyHandler::getButton(unsigned int button)
{
  getData();

  // make sure we have that axis
  if (button < 1 || button > myButtons.size())
    return 0;
  
  // now make sure its in there
  if (myButtons.find(button) == myButtons.end())
    return 0;
  return myButtons[button];
}

/*
 * @return the number of axes (axes are indexed as 1 through this number)
*/
MVREXPORT unsigned int MvrJoyHandler::getNumAxes(void)
{
  return (unsigned int) myAxes.size();
}

/*
 * @return the number of buttons (buttons are indexed as 1 through this number) 
 */
MVREXPORT unsigned int MvrJoyHandler::getNumButtons(void)
{
  return (unsigned int) myButtons.size();
} 



bool MvrJoyHandler::init(void)
{
  int i;

  myLastOpenTry.setToNow();
  myJoyNumber = 0;

  if (myUseOld)
  {
    myOldJoyDesc = MvrUtil::fopen("/dev/js0","r");
    if (myOldJoyDesc > 0)
      MvrLog::log(MvrLog::Verbose, 
                  "MvrJoyHandler: Opened /dev/js0 (old Linux device name scheme)");
  }
  else
  {
    for (i=0; i < 32; i++)
    {
      sprintf(myJoyNameTemp, "/dev/input/js%d", i);
      if ((myJoyDesc = MvrUtil::open(myJoyNameTemp,O_RDONLY | O_NONBLOCK)) > 0)
      {
        MvrLog::log(MvrLog::Verbose, "MvrJoyHandler: Opened %s", myJoyNameTemp);
        break;
      }
    }
  }

  if ((myUseOld && myOldJoyDesc != NULL) || (!myUseOld && myJoyDesc > 0))
  {
    myPhysMax     = 255;
    myInitialized = true;
    startCal();
    endCal();
    getData();
    return true;
  }
  else
  {
    myJoyNumber   = 1;
    myPhysMax     = 255;
    myInitialized = true;
    getData();
    return false;
  }
}

void MvrJoyHandler::getData(void)
{
  if (myUseOld && !myInitialized)
    return;
  
  if (!myFirstData && myLastDataGathered.mSecSince() < 5)
    return;
  myFirstData = false;
  myLastDataGathered.setToNow();
  if (myUseOld)
    getOldData();
  else
    getNewData();
}

void MvrJoyHandler::getOldData(void)
{
#ifdef Linux
  int x, y;
 if (myOldJoyDesc == NULL || !myInitialized || fread(&myJoyData, 1, JS_RETURN, myOldJoyDesc) != JS_RETURN) 
  {
    myAxes[1] = 0;
    myAxes[2] = 0;
    myAxes[3] = 0;
    myButtons[1] = false;
    myButtons[2] = false;
    myButtons[3] = false;
    myButtons[4] = false;
  } 
  else 
  {
    x = myJoyData.x - 128; 
    y =  - (myJoyData.y - 128);
    if (x > myMaxX)
      myMaxX = x;
    if (x < myMinX)
      myMinX = x;
    if (y > myMaxY)
      myMaxY = y;
    if (y < myMinY)
      myMinY = y;
    myAxes[1] = x;
    myAxes[2] = y;
    myAxes[3] = 0;
    myButtons[1] = myJoyData.buttons & 1;
    myButtons[2] = myJoyData.buttons & 2;
    myButtons[3] = myJoyData.buttons & 4;
    myButtons[4] = myJoyData.buttons & 8;
  }
#endif // ifdef linux
}

/// Handles the reading of the data into the bins
void MvrJoyHandler::getNewData(void)
{
#ifdef linux
  if (myLastOpenTry.mSecSince() > 125)
  {
    int tempDesc;
    myLastOpenTry.setToNow();
    sprintf(myJoyNameTemp, "/dev/input/js%d", myJoyNumber + 1);
    if ((tempDesc = MvrUtil::open(myJoyNameTemp, O_RDWR | O_NONBLOCK)) > 0)
    {
      MvrLog::log(MvrLog::Verbose, "MvrJoyHandler: Opened next joydev %s", myJoyNameTemp);
      close(myJoyDesc);
      myInitialized = true;
      myJoyDesc = tempDesc;
      myJoyNumber++;
    }
    else if (myJoyNumber > 0)
    {
      if ((tempDesc = MvrUtil::open("/dev/input/js0", O_RDWR | O_NONBLOCK)) > 0)
      {
        myInitialized = true;
        MvrLog::log(MvrLog::Verbose, "MvrJoyHandler: Opened first joydev /dev/input/js0");
        close(myJoyDesc);
        myJoyDesc = tempDesc;
        myJoyNumber = 0;
      }
    }
  }

  struct js_event e;
  while (read (myJoyDesc, &e, sizeof(struct js_event)) > 0)
  {
    // see if its a button even
    if ((e.type & JS_EVENT_BUTTON))
    {
      // if its one of the buttons we want set it
      myButtons[e.number+1] = (bool)e.value;
    }
    // see if its an axis
    if ((e.type & JS_EVENT_AXIS))
    {
      // if its one of the buttons we want set it
      if (e.number == 0)
      	myAxes[e.number+1] = MvrMath::roundInt(e.value * 128.0 / 32767.0);
      else
	myAxes[e.number+1] = MvrMath::roundInt(-e.value * 128.0 / 32767.0);
      if (e.number == 2)
	myHaveZ = true;
    }
    //printf("%d 0x%x 0x%x\n", e.value, e.type, e.number);
  }
  if (errno != EAGAIN)
  {
    //MvrLog::log(MvrLog::Terse, "MvrJoyHandler::getUnfiltered: Trouble reading data.");
  }
#endif // ifdef linux 
}