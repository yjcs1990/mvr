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
#ifndef ARSONARDEVICE_H
#define ARSONARDEVICE_H

#include "ariaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctor.h"

#include "MvrRobot.h"

/// Keep track of recent sonar readings from a robot as an MvrRangeDevice
/** 
    This class is for keeping a sonar history, which you may use for obstacle 
    avoidance, display, etc.
    Simply use MvrRobot::addRangeDevice()  (or MvrSonarDevice::setRobot())
    to attach an MvrSonarDevice object to an MvrRobot
    robot object; MvrSonarDevice will add a Sensor Interpretation task to the
    MvrRobot which will read new sonar readings each robot cycle and add
    them to its sonar history.

    (Note that sonar range readings are from the surface of the sonar transducer disc,
    not from the center of the robot.)

    @ingroup ImportantClasses
   @ingroup DeviceClasses
*/
class MvrSonarDevice : public MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrSonarDevice(size_t currentBufferSize = 24, 
			 size_t cumulativeBufferSize = 64, 
			 const char * name = "sonar");
  /// Destructor
  MVREXPORT virtual ~MvrSonarDevice();
  /// Grabs the new readings from the robot and adds them to the buffers
  /// (Primarily for internal use.)
  MVREXPORT void processReadings(void);

  /// Sets the robot pointer, also attaches its process function to the
  /// robot as a Sensor Interpretation task.
  MVREXPORT virtual void setRobot(MvrRobot *robot);

  /// Adds sonar readings to the current and cumulative buffers
  /// Overrides the MvrRangeDevice default action.
  /// (This method is primarily for internal use.)
  MVREXPORT virtual void addReading(double x, double y);

  /// Sets a callback which if it returns true will ignore the reading
  MVREXPORT void setIgnoreReadingCB(MvrRetFunctor1<bool, MvrPose> *ignoreReadingCB);
 
  /// Gets the callback which if it returns true will ignore the reading
  MVREXPORT MvrRetFunctor1<bool, MvrPose> *getIgnoreReadingCB(void)
    { return myIgnoreReadingCB; }

  /** @deprecated
   *  @sa MvrRangeDevice::setMaxDistToKeepCumulative()
   */
  MVREXPORT void setCumulativeMaxRange(double range) 
    { setMaxDistToKeepCumulative(range); }
protected:
  MvrFunctorC<ArSonarDevice> myProcessCB;
  double myFilterNearDist;	// we throw out cumulative readings this close to current one
  double myFilterFarDist;	// throw out cumulative readings beyond this far from robot

  MvrRetFunctor1<bool, MvrPose> *myIgnoreReadingCB;
};


#endif // ARSONARDEVICE_H
