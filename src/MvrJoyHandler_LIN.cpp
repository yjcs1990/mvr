#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrJoyHandler.h"
#include "MvrLog.h"
#include <errno.h>
#include "mvriaUtil.h"

bool MvrJoyHandler::init(void)
{
  int i;

  myLastOpenTry.setToNow();
  myJoyNumber = 0;

  if (myUseOld)
  {
    myOldJoyDesc = MvrUtil::fopen("/dev/js0", "r");
    if(myOldJoyDesc > 0)
      MvrLog::log(MvrLog::Verbose, "MvrJoyHandler: Opened /dev/js0 (old Linux device name scheme)");
  }
  else
  {
    for (i = 0; i < 32; i++)
    {
      sprintf(myJoyNameTemp, "/dev/input/js%d", i);
      if ((myJoyDesc = MvrUtil::open(myJoyNameTemp, O_RDONLY | O_NONBLOCK)) > 0)
      {
        MvrLog::log(MvrLog::Verbose, "MvrJoyHandler: Opened %s", myJoyNameTemp);
        break;
      }
    }
  }
  
  if ((myUseOld && myOldJoyDesc != NULL) || (!myUseOld && myJoyDesc > 0))
  {
    myPhysMax = 255;
    myInitialized = true;
    startCal();
    endCal();
    getData();
    return true;
  } 
  else 
  {
    myJoyNumber = -1;
    myPhysMax = 255;
    myInitialized = false;
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
#ifdef linux
  int x, y;
  if (myOldJoyDesc == NULL || !myInitialized || 
      fread(&myJoyData, 1, JS_RETURN, myOldJoyDesc) != JS_RETURN) 
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

