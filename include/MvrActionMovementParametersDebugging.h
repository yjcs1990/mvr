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
#ifndef ARACTIONMOVEMENTPARAMTERSDEBUGGING_H
#define ARACTIONMOVEMENTPARAMTERSDEBUGGING_H

#include "ariaTypedefs.h"
#include "MvrAction.h"
#include "MvrMapObject.h"

/// This is a class for setting max velocities and accels and decels via MvrConfig parameters (see addToConfig());
/**
   @ingroup ActionClasses
 **/
class MvrActionMovementParametersDebugging : public MvrAction
{
public: 
  /// Constructor
  MVREXPORT MvrActionMovementParametersDebugging(const char *name = "MovementParametersDebugging");
  /// Destructor
  MVREXPORT virtual ~MvrActionMovementParametersDebugging();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  /// Sees if this action is enabled (separate from activating it)
  MVREXPORT bool isEnabled(void) { return myEnabled; }
  /// Enables this action (separate from activating it)
  MVREXPORT void enable(void) { myEnabled = true; }
  /// Enables this action in a way that'll work from the sector callbacks
  MVREXPORT void enableOnceFromSector(MvrMapObject *mapObject) 
    { myEnableOnce = true; }
  /// Disables this action (separate from deactivating it)
  MVREXPORT void disable(void) { myEnabled = false; }
  /// Adds to the MvrConfig given, in section, with prefix
  MVREXPORT void addToConfig(MvrConfig *config, const char *section,
			    const char *prefix = NULL);
protected:
  bool myEnabled;
  bool myEnableOnce;

  bool mySetMaxVel;
  double myMaxVel;
  bool mySetMaxNegVel;
  double myMaxNegVel;
  bool mySetTransAccel;
  double myTransAccel;
  bool mySetTransDecel;
  double myTransDecel;
  bool mySetMaxRotVel;
  double myMaxRotVel;
  bool mySetRotAccel;
  double myRotAccel;
  bool mySetRotDecel;
  double myRotDecel;
  bool mySetMaxLeftLatVel;
  double myMaxLeftLatVel;
  bool mySetMaxRightLatVel;
  double myMaxRightLatVel;
  bool mySetLatAccel;
  double myLatAccel;
  bool mySetLatDecel;
  double myLatDecel;
  
  MvrActionDesired myDesired;


};

#endif // ARACTIONMOVEMENTPARAMTERS_H
