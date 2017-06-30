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
#ifndef ARLASERFILTER_H
#define ARLASERFILTER_H

#include "MvrLaser.h"
#include "MvrFunctor.h"

class MvrRobot;
class MvrConfig;

/// Range device with data obtained from another range device and filtered 
/**
   This is a class for generically filtering laser readings, either to
   separate out ones that are too far or too close compared to their
   neighbors.  The filtering parameters can be adjusted on line
   through MvrConfig options.

   This implements MvrLaser and so can be used like any other laser,
   though you have to set all its options before you create this and
   probably should connect it too.  Then you should replace the
   original laser on MvrRobot with this one, and replace the original
   laser as a range device too.
**/
class MvrLaserFilter : public MvrLaser
{
public:
  /// Constructor
  MVREXPORT MvrLaserFilter(MvrLaser *laser, const char *name = NULL);
  /// Destructor
  MVREXPORT ~MvrLaserFilter();
  /// Set robot
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  /// Add to the config
  MVREXPORT void addToConfig(MvrConfig *config, const char *sectionName,
			    const char *prefix = "");

  MVREXPORT virtual bool blockingConnect(void) 
    { return myLaser->blockingConnect(); }
  MVREXPORT virtual bool asyncConnect(void)
    { return myLaser->asyncConnect(); }
  MVREXPORT virtual bool disconnect(void)
    { return myLaser->disconnect(); }
  MVREXPORT virtual bool isConnected(void)
    { return myLaser->isConnected(); }
  MVREXPORT virtual bool isTryingToConnect(void)
    { return myLaser->isTryingToConnect(); }

  MVREXPORT virtual void *runThread(void *arg) { return NULL; } 
  MVREXPORT virtual bool laserCheckParams(void) 
    { 
      if (!myLaser->laserCheckParams())
	return false;
      
      laserSetAbsoluteMaxRange(myLaser->getAbsoluteMaxRange());
      return true; 
    }
  
  /// Gets the base laser this is filtering
  MvrLaser *getBaseLaser(void) { return myLaser; }
protected:
  MVREXPORT int selfLockDevice(void);
  MVREXPORT int selfTryLockDevice(void);
  MVREXPORT int selfUnlockDevice(void);

  MvrLaser *myLaser;

  // Parameters
  double myAngleToCheck;
  double myAllFactor;
  double myAnyFactor;
  double myAnyMinRange;
  double myAnyMinRangeLessThanAngle;
  double myAnyMinRangeGreaterThanAngle;

  /// Does the check against all neighbor factor
  bool checkRanges(int thisReading, int otherReading, double factor);
  
  // Callback to do the actual filtering
  void processReadings(void);

  MvrFunctorC<ArLaserFilter> myProcessCB;
};

#endif // ARLASERFILTER
