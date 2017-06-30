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
#ifndef ARACTIONRATIOINPUT_H
#define ARACTIONRATIOINPUT_H

#include "ariaTypedefs.h"
#include "MvrAction.h"

/// Action that requests motion based on abstract ratios provided by diferent input sources 
/**
   This action interprets input drive commands as three abstract ratios, 
   translation, rotation, and throttle. (In this way it mimics many joysticks.)
   The translation speed input ranges from -100 to 100, where -100 requests maximum
   backwards speed, and 100 requests maximum forward speed, 0 requests no
   translational speed, and values in between request a linear percentage of the maximum.
   Similarly, rotation speed input ranges from -100 to 100,
   where -100 indicates maximum rightwards or clockwise rotation, 100 indicates
   maximum leftwards or counter-clockwise rotation, 0 requests no rotation, and
   values in between request a linear percentage of the maximum.  The throttle
   input scales the other speed, and ranges from 0 (no motion) to 100 (maximum motion).
   
   Seperate objects (e.g. MvrRatioInputKeydrive, MvrRatioInputJoydrive, MvrRAtionInputRobotJoydrive) 
   are used to provide input.

   When this action is activated it resets all its input ratios to 0
   (including throttle).

   Configuration parameters are used to map the maximum ratios to actual robot speeds.
   These are set be default to the robot's maximum configured velocities at startup but you can
   override them with MvrConfig parameters (and call addToConfig()) or setParameters().

   @ingroup ActionClasses
   
   @see MvrRatioInputKeydrive
   @see MvrRatioInputJoydrive
   @see MvrRatioInputRobotJoydrive
 **/
class MvrActionRatioInput : public MvrAction
{
public:
  /// Constructor
  AREXPORT MvrActionRatioInput(const char *name = "RatioInput");
  /// Destructor
  AREXPORT virtual ~ArActionRatioInput();
  /// Set ratios
  AREXPORT void setRatios(double transRatio, double rotRatio, 
			  double throttleRatio, double latRatio = 0);
  /// Sets the trans ratio (from -100 (full backwards) to 100 (full forwards)
  AREXPORT void setTransRatio(double transRatio);
  /// Sets the rot ratio (from -100 (full right) to 100 (full left)
  AREXPORT void setRotRatio(double rotRatio);
  /// Sets the lat ratio (from -100 (one way) to 100 (the other))
  AREXPORT void setLatRatio(double latRatio);
  /// Sets the throttle ratio (from 0 (stopped) to 100 (full throttle)
  AREXPORT void setThrottleRatio(double throttleRatio);
  /// Gets the trans ratio (from -100 (full backwards) to 100 (full forwards)
  double getTransRatio(void) { return myTransRatio; }
  /// Gets the rot ratio (from -100 (full right) to 100 (full left)
  double getRotRatio(void) { return myRotRatio; }
  /// Gets the throttle ratio (from 0 (stopped) to 100 (full throttle)
  double getThrottleRatio(void) { return myThrottleRatio; }
  /// Adds a callback that is called from this actions fire call
  AREXPORT void addFireCallback(int priority, MvrFunctor *functor);
  /// Removes a callback that was called from this actions fire callback
  AREXPORT void remFireCallback(MvrFunctor *functor);
  /// Adds a callback that is called when this action is activated
  AREXPORT void addActivateCallback(MvrFunctor *functor, 
				    MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes a callback that was called when this action is activated
  AREXPORT void remActivateCallback(MvrFunctor *functor);
  /// Adds a callback that is called when this action is deactivated
  AREXPORT void addDeactivateCallback(MvrFunctor *functor, 
			      MvrListPos::Pos position = MvrListPos::LAST);
  /// Removes a callback that was called when this action is deactivated
  AREXPORT void remDeactivateCallback(MvrFunctor *functor);
  /// Sets the parameters
  AREXPORT void setParameters(double fullThrottleForwards, 
			      double fullThrottleBackwards, 
			      double rotAtFullForwards,
			      double rotAtFullBackwards,
			      double rotAtStopped,
			      double latAtFullForwards = 0, 
			      double latAtFullBackwards = 0,
			      double latAtStopped = 0);
  /// Adds to a section in a config
  AREXPORT void addToConfig(MvrConfig *config, const char *section);
  AREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  AREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  AREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
protected:
  std::multimap<int, MvrFunctor *> myFireCallbacks;
  std::list<ArFunctor *> myActivateCallbacks;
  std::list<ArFunctor *> myDeactivateCallbacks;
  // if we're printing extra information or not
  bool myPrinting;
  double myTransDeadZone;
  double myRotDeadZone;
  double myLatDeadZone;
  double myFullThrottleForwards;
  double myFullThrottleBackwards; 
  double myRotAtFullForwards;
  double myRotAtFullBackwards;
  double myRotAtStopped;
  double myLatAtFullForwards;
  double myLatAtFullBackwards;
  double myLatAtStopped;
  double myTransRatio;
  double myRotRatio;
  double myThrottleRatio;
  double myLatRatio;
  MvrActionDesired myDesired;
};

#endif // ARACTIONSTOP_H
