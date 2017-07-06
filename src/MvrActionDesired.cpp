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
#include "MvrActionDesired.h"

/* Need to export symbols but can't prior to VS 2010 (The VS 2003 _MSC_VER is version 1310, 2005 is 14xx, 2008 is 15xx, 2010 is 1600)*/
//#if (_MSC_VER < 1600)
//#define STATIC_CONST_MVREXPORT // defined to nothing
//#else
//#define STATIC_CONST_MVREXPORT MVREXPORT
//#endif

MVREXPORT const double MvrActionDesired::NO_STRENGTH = 0.0;
MVREXPORT const double MvrActionDesired::MIN_STRENGTH = .000001;
MVREXPORT const double MvrActionDesired::MAX_STRENGTH = 1.0;

MVREXPORT const double MvrActionDesiredChannel::NO_STRENGTH =
                                                 MvrActionDesired::NO_STRENGTH;
MVREXPORT const double MvrActionDesiredChannel::MIN_STRENGTH = 
                                                MvrActionDesired::MIN_STRENGTH;
MVREXPORT const double MvrActionDesiredChannel::MAX_STRENGTH = 
                                                MvrActionDesired::MAX_STRENGTH;


MVREXPORT void MvrActionDesired::log(void) const
{
  // all those maxes and movement parameters
  if (getMaxVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "\tMaxTransVel %.0f", getMaxVel());
  if (getMaxNegVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "\tMaxTransNegVel %.0f", 
	       getMaxNegVel());
  if (getTransAccelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "\tTransAccel %.0f", getTransAccel());
  if (getTransDecelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "\tTransDecel %.0f", getTransDecel());

  if (getMaxRotVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tMaxRotVel %.0f", "",
	       getMaxRotVel());
  if (getMaxRotVelPosStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tMaxRotVelPos %.0f", "",
	       getMaxRotVelPos());
  if (getMaxRotVelNegStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tMaxRotVelNeg %.0f", "",
	       getMaxRotVelNeg());
  if (getRotAccelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tRotAccel %.0f", "",
	       getRotAccel());
  if (getRotDecelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tRotDecel %.0f", "",
	       getRotDecel());

  if (getMaxLeftLatVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%12s\tMaxLeftLatVel %.0f", "",
	       getMaxLeftLatVel());
  if (getMaxRightLatVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%12s\tMaxRightLatVel %.0f", "",
	       getMaxRightLatVel());
  if (getLatAccelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%12s\tLatAccel %.0f", "",
	       getLatAccel());
  if (getLatDecelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%12s\tLatDecel %.0f", "",
	       getLatDecel());
  
  // the actual movement part
  if (getVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "\tVel %.0f", getVel());
  if (getHeadingStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tHeading %.0f", "", 
	       getHeading());
  if (getDeltaHeadingStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tDeltaHeading %.0f", "", 
	       getDeltaHeading());
  if (getRotVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%25s\tRotVel %.0f", "", 
	       getRotVel());
  if (getLatVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    MvrLog::log(MvrLog::Normal, "%12s\tLatVel %.0f", "", 
	       getLatVel());
}


MVREXPORT bool MvrActionDesired::isAnythingDesired(void) const
{
  if (getVelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getMaxVelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getMaxNegVelStrength() >= MvrActionDesired::MIN_STRENGTH || 
      getTransAccelStrength() >= MvrActionDesired::MIN_STRENGTH || 
      getTransDecelStrength() >= MvrActionDesired::MIN_STRENGTH ||

      getHeadingStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getDeltaHeadingStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getRotVelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getMaxRotVelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getMaxRotVelPosStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getMaxRotVelNegStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getRotAccelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getRotDecelStrength() >= MvrActionDesired::MIN_STRENGTH ||

      getMaxLeftLatVelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getMaxRightLatVelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getLatAccelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getLatDecelStrength() >= MvrActionDesired::MIN_STRENGTH ||
      getLatVelStrength() >= MvrActionDesired::MIN_STRENGTH)
    return true;
  else
    return false;
}

MVREXPORT void MvrActionDesired::sanityCheck(const char *actionName)
{
  myMaxVelDes.checkLowerBound(actionName, "TransMaxVel", 0);
  myMaxNegVelDes.checkUpperBound(actionName, "TransMaxNegVel", 0);

  myTransAccelDes.checkLowerBound(actionName, "TransAccel", 1);
  myTransDecelDes.checkLowerBound(actionName, "TransDecel", 1);

  if (myMaxRotVelDes.getStrength() >= MvrActionDesired::MIN_STRENGTH && 
      MvrMath::roundInt(myMaxRotVelDes.getDesired()) == 0)
    MvrLog::log(MvrLog::Normal, 
	  "ActionSanityChecking: '%s' setting %s to %g which winds up as 0 (this is just a warning)",
	       actionName, "MaxRotVel", myMaxRotVelDes.getDesired());
  myMaxRotVelDes.checkLowerBound(actionName, "MaxRotVel", 0);


  myMaxRotVelPosDes.checkLowerBound(actionName, "MaxRotVelPos", 1); 
  myMaxRotVelNegDes.checkLowerBound(actionName, "MaxRotVelNeg", 1);

  myRotAccelDes.checkLowerBound(actionName, "RotAccel", 1);
  myRotDecelDes.checkLowerBound(actionName, "RotDecel", 1);

  myMaxLeftLatVelDes.checkLowerBound(actionName, "MaxLeftLatVel", 0);
  myMaxRightLatVelDes.checkLowerBound(actionName, "MaxRightLatVel", 0);

  myLatAccelDes.checkLowerBound(actionName, "LatAccel", 1);
  myLatDecelDes.checkLowerBound(actionName, "LatDecel", 1);
};
