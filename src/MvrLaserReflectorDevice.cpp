#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSick.h"
#include "MvrLaserReflectorDevice.h"
#include "MvrRobot.h"

MVREXPORT MvrLaserReflectorDevice::MvrLaserReflectorDevice(MvrRangeDevice *laser,
							MvrRobot *robot,
							const char *name) :
  /*
  MvrRangeDevice(laser->getCurrentRangeBuffer()->getSize(), 
		laser->getCumulativeRangeBuffer()->getSize(), name,
		laser->getMaxRange()), */
  // MPL 12/11/12 this is how it was... but the part I'm putting in was above commented out, so this may go wrong
  //MvrRangeDevice(361, 361, name, 32000),
  MvrRangeDevice(laser->getCurrentRangeBuffer()->getSize(), 
		laser->getCumulativeRangeBuffer()->getSize(), name,
		laser->getMaxRange()), 
  myProcessCB(this, &MvrLaserReflectorDevice::processReadings)
{
  myLaser = laser;
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 10, &myProcessCB);
  setCurrentDrawingData(new MvrDrawingData("polyDots", 
                                          MvrColor(0xb0, 0xb0, 0xff), 
                                          60,  // mm length of arrow
                                          77,  // above the normal laser
					  200, // default refresh
					  "DefaultOff"), // defaults to off but can be turned on
			true);
  myReflectanceThreshold = 31;
}

MVREXPORT MvrLaserReflectorDevice::~MvrLaserReflectorDevice()
{
  if (myRobot != NULL)
    myRobot->remSensorInterpTask(&myProcessCB);
}

MVREXPORT void MvrLaserReflectorDevice::setRobot(MvrRobot *robot)
{
  // specifically do nothing since this is just here for debugging
}

MVREXPORT void MvrLaserReflectorDevice::addToConfig(MvrConfig *config, 
						  const char *section)
{

  config->addSection(MvrConfig::CATEGORY_ROBOT_PHYSICAL,
                     section,
                     "Settings for using the reflector readings from this laser");

  config->addParam(
	  MvrConfigArg("ReflectanceThreshold", &myReflectanceThreshold,
		      "The threshold to start showing reflector readings at (normalized from 0 to 255, 31 is the default)", 
		      0, 255),
	  section, MvrPriority::DETAILED);
		      
}

MVREXPORT void MvrLaserReflectorDevice::processReadings(void)
{
  //int i;
  MvrSensorReading *reading;
  myLaser->lockDevice();
  lockDevice();
  
  const std::list<MvrSensorReading *> *rawReadings;
  std::list<MvrSensorReading *>::const_iterator rawIt;
  rawReadings = myLaser->getRawReadings();
  myCurrentBuffer.beginRedoBuffer();

  if (myReflectanceThreshold < 0 || myReflectanceThreshold > 255)
    myReflectanceThreshold = 0;

  if (rawReadings->begin() != rawReadings->end())
  {
    for (rawIt = rawReadings->begin(); rawIt != rawReadings->end(); rawIt++)
    {
      reading = (*rawIt);
      if (!reading->getIgnoreThisReading() && 
	  reading->getExtraInt() > myReflectanceThreshold)
	myCurrentBuffer.redoReading(reading->getPose().getX(), 
				    reading->getPose().getY());
    }
  }

  myCurrentBuffer.endRedoBuffer();

  unlockDevice();
  myLaser->unlockDevice();
}

