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
#include "MvrRobot.h"
#include "MvrBumpers.h"

/**
   @param currentBufferSize The number of readings to store in the current Buffer
   @param cumulativeBufferSize The number of readings in the cumulative buffer (This currently is not being used)
   @param name The name of this range device
   @param maxSecondsToKeepCurrent How long to keep readings in the current buffer
   @param angleRange the range in front and behind the robot which is divided by the number of bumpers and used to detrmine where the sensor readings will be placed.
*/

MVREXPORT MvrBumpers::MvrBumpers(size_t currentBufferSize, size_t cumulativeBufferSize, 
		     const char *name, int maxSecondsToKeepCurrent, double angleRange) :
  MvrRangeDevice(currentBufferSize, cumulativeBufferSize, name, 5000, maxSecondsToKeepCurrent), 
  myProcessCB(this, &MvrBumpers::processReadings)
{
  // MPL I wrote this code, but checking for BIT8 makes no sense, BIT0 is  the stall, BIT8 would be beyond this data
  myBumpMask = (MvrUtil::BIT1 | MvrUtil::BIT2 | MvrUtil::BIT3 | MvrUtil::BIT4 | 
		MvrUtil::BIT5 | MvrUtil::BIT6 | MvrUtil::BIT7 | MvrUtil::BIT8); 

  myAngleRange = angleRange;

  setCurrentDrawingData(new MvrDrawingData("polyDots", MvrColor(0, 0, 0),
					  120, // mm diameter of dots
					  83), // layer above most everything else
			true);
}

MVREXPORT MvrBumpers::~MvrBumpers()
{
  if (myRobot != NULL)
    {
      myRobot->remSensorInterpTask(&myProcessCB);
      myRobot->remRangeDevice(this);
    }
}

MVREXPORT void MvrBumpers::setRobot(MvrRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 10, &myProcessCB);
  MvrRangeDevice::setRobot(robot);
}

/**
   This function is called every 100 milliseconds.
*/
MVREXPORT void MvrBumpers::processReadings(void)
{
  int frontBump;
  int rearBump;
  int whichBumper;

  if (myRobot->hasFrontBumpers())
    frontBump = ((myRobot->getStallValue() & 0xff00) >> 8) & myBumpMask;
  else
    {
      frontBump = 0;
    }
  if (myRobot->hasRearBumpers())
    rearBump = (myRobot->getStallValue() & 0xff) & myBumpMask;
  else
    {
      rearBump = 0;
    }

  if(frontBump!= 0)
    {
      whichBumper = 1;
      addBumpToBuffer(frontBump, whichBumper);
    }

  if(rearBump != 0)
    {
      whichBumper = 2;
      addBumpToBuffer(rearBump, whichBumper);
    }

}

/**
   @param bumpValue This is the value that tells which individual bumper has been triggered
   @param whichBumper This value tells if the front or rear has been triggered
*/
MVREXPORT void MvrBumpers::addBumpToBuffer(int bumpValue, int whichBumper)
{
  int numBumpers;
  double x;
  double y;
  double degree;
  double radius;

  const MvrRobotParams *params;
  params = myRobot->getRobotParams();

  radius = params->getRobotRadius();

  if(whichBumper == 1) numBumpers = myRobot->getNumFrontBumpers();
  else numBumpers = myRobot->getNumRearBumpers();

  for (int i = 0; i < numBumpers; i++)
    {
      // MPL I wrote this code, but checking for BIT8 makes no sense, BIT0 is  the stall, BIT8 would be beyond this data
      if((i == 0 && (bumpValue & MvrUtil::BIT1)) || 
	 (i == 1 && (bumpValue & MvrUtil::BIT2)) ||
	 (i == 2 && (bumpValue & MvrUtil::BIT3)) || 
	 (i == 3 && (bumpValue & MvrUtil::BIT4)) ||
	 (i == 4 && (bumpValue & MvrUtil::BIT5)) || 
	 (i == 5 && (bumpValue & MvrUtil::BIT6)) ||
	 (i == 6 && (bumpValue & MvrUtil::BIT7)) || 
	 (i == 7 && (bumpValue & MvrUtil::BIT8)))
	{
	  degree = -1 * (i * (myAngleRange / (double)numBumpers) + 
		    ((myAngleRange / (double)numBumpers) / 2) - (myAngleRange / 2));

	  if(whichBumper == 2) degree = degree + 180;

	  x = radius * MvrMath::cos(degree);
	  y = radius * MvrMath::sin(degree);

	  MvrPose pose;
	  pose.setX(x);
	  pose.setY(y);

	  MvrTransform global = myRobot->getToGlobalTransform();
	  pose = global.doTransform(pose);

    MvrLog::log(MvrLog::Verbose, "Bumpers: recording %s bumper hit (bumpflags=%d)", (whichBumper==1?"front":"rear"), bumpValue);

	  myCurrentBuffer.addReading(pose.getX(), pose.getY());
	}
    } 
}
