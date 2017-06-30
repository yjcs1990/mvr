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
#ifndef ARFORBIDDENRANGEDEVICE_H
#define ARFORBIDDENRANGEDEVICE_H

#include "ariaTypedefs.h"
#include "MvrRangeDevice.h"
#include "MvrMapInterface.h"

/// Class that takes forbidden lines and turns them into range readings
///  @ingroup OptionalClasses
class MvrForbiddenRangeDevice : public MvrRangeDevice
{
public:
  /// Constructor
  AREXPORT MvrForbiddenRangeDevice(MvrMapInterface *armap, 
                                  double distanceIncrement = 100,
				                          unsigned int maxRange = 4000,
				                          const char *name = "forbidden");
  /// Destructor
  AREXPORT virtual ~ArForbiddenRangeDevice();
  /// Saves the forbidden lines from the map
  AREXPORT void processMap(void);
  /// Remakes the readings 
  AREXPORT void processReadings(void);
  /// Sets the robot pointer and attachs its process function
  AREXPORT virtual void setRobot(MvrRobot *robot);

  /// Enable readings 
  AREXPORT void enable(void);
  /// Disables readings until reenabled
  AREXPORT void disable(void);
  /// Sees if this device is active or not
  AREXPORT bool isEnabled(void) const { return myIsEnabled;; }
  /// Gets a callback to enable the device
  AREXPORT MvrFunctor *getEnableCB(void) { return &myEnableCB; } 
  /// Gets a callback to disable the device
  AREXPORT MvrFunctor *getDisableCB(void) { return &myDisableCB; } 
protected:
  MvrMutex myDataMutex;
  MvrMapInterface *myMap;
  double myDistanceIncrement;
  std::list<ArLineSegment *> mySegments;
  MvrFunctorC<ArForbiddenRangeDevice> myProcessCB;
  MvrFunctorC<ArForbiddenRangeDevice> myMapChangedCB;
  bool myIsEnabled;
  MvrFunctorC<ArForbiddenRangeDevice> myEnableCB;
  MvrFunctorC<ArForbiddenRangeDevice> myDisableCB;
};

#endif // ARFORBIDDENRANGEDEVICE_H
