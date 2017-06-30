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
#ifndef ARLASERREFLECTORDEVICE_H
#define ARLASERREFLECTORDEVICE_H

#include "ariaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrFunctor.h"

class MvrSick;
class MvrRobot;

/// A class for keeping track of laser reflectors that we see right now
/** 
    This class is for showing the laser reflectors in MobileEyes.
    This requires that the range device you pass in uses the
    'extraInt' in the rawReadings MvrSensorReading to note reflector
    value and that anything greater than 0 is a reflector.
*/
class MvrLaserReflectorDevice : public MvrRangeDevice
{
public:
  /// Constructor
  MVREXPORT MvrLaserReflectorDevice(MvrRangeDevice *laser, MvrRobot *robot,
				  const char * name = "reflector");
  /// Destructor
  MVREXPORT virtual ~MvrLaserReflectorDevice();
  /// Grabs the new readings from the robot and adds them to the buffers
  MVREXPORT void processReadings(void);
  /// Specifically does nothing since it was done in the constructor
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  /// Adds a reflector threshold to the task
  MVREXPORT void addToConfig(MvrConfig *config, const char *section);
protected:
  MvrRangeDevice *myLaser;
  int myReflectanceThreshold;
  MvrFunctorC<ArLaserReflectorDevice> myProcessCB;
};


#endif // ARLASERREFLECTORDEVICE_H
