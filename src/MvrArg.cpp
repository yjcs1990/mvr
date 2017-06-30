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
#include "ariaOSDef.h"
#include "MvrArg.h"
#include "MvrLog.h"
#include "MvrArgumentBuilder.h"

AREXPORT MvrArg::ArArg()
{
  myType = INVALID;
  myName = "";
  myDescription = "";
  clear();
}

AREXPORT MvrArg::ArArg(const char * name, int *pointer, 
		      const char * description, int minInt, int maxInt) 
{ 
  myType = INT;
  myName = name; 
  myDescription = description;
  clear();
  myMinInt = minInt;
  myMaxInt = maxInt;
  myIntPointer = pointer;
}

AREXPORT MvrArg::ArArg(const char * name, double *pointer,
		      const char * description, double minDouble, 
		      double maxDouble) 
{ 
  myType = DOUBLE;
  myName = name; 
  myDescription = description;
  clear();
  myMinDouble = minDouble;
  myMaxDouble = maxDouble;
  myDoublePointer = pointer;
}

AREXPORT MvrArg::ArArg(const char * name, bool *pointer, 
		      const char * description) 
{ 
  myType = BOOL;
  myName = name; 
  myDescription = description;
  clear();
  myBoolPointer = pointer;
}

AREXPORT MvrArg::ArArg(const char * name, MvrPose *pointer, 
		      const char * description) 
{ 
  myType = POSE;
  myName = name; 
  myDescription = description;
  clear();
  myPosePointer = pointer;
}

AREXPORT MvrArg::ArArg(const char * name, char *pointer, 
		      const char * description, size_t maxStrLen) 
{ 
  myType = STRING;
  myName = name; 
  myDescription = description;
  clear();
  myStringPointer = pointer;
  myMaxStrLen = maxStrLen;
}

/**
   This constructor is for the functor type of argument, this is for
   cases that need to be complicated and have more than one argument
   value per name, such as the sonar in a config file, and where this data
   needs to be used to construct compound data structures rather than
   single variables.

   @param name argument name
   @param description argument description
   
   @param setFunctor when an argument is read it is passed to this
   functor which should set up whatever it needs to from the data
   
   @param getFunctor since parameter files need to be written too,
   this get functor will get a list of strings to be written to the file
**/
AREXPORT MvrArg::ArArg(const char *name, 
		      MvrRetFunctor1<bool, MvrArgumentBuilder *> *setFunctor, 
	      MvrRetFunctor<const std::list<ArArgumentBuilder *> *> *getFunctor,
		      const char *description)
{
  myType = FUNCTOR;
  myName = name;
  myDescription = description;
  clear();
  mySetFunctor = setFunctor;
  myGetFunctor = getFunctor;
}

AREXPORT MvrArg::ArArg(const char * description)
{ 
  myType = DESCRIPTION_HOLDER;
  myDescription = description;
  clear();
}

AREXPORT MvrArg::ArArg(const MvrArg & arg) 
{
  myType = arg.myType;
  myName = arg.myName;
  myDescription = arg.myDescription;
  myIntPointer = arg.myIntPointer;
  myDoublePointer = arg.myDoublePointer;
  myPosePointer = arg.myPosePointer;
  myBoolPointer = arg.myBoolPointer;
  myStringPointer = arg.myStringPointer;
  myMinInt = arg.myMinInt;
  myMaxInt = arg.myMaxInt;
  myMinDouble = arg.myMinDouble;
  myMaxDouble = arg.myMaxDouble;
  myMaxStrLen = arg.myMaxStrLen;
  mySetFunctor = arg.mySetFunctor;
  myGetFunctor = arg.myGetFunctor;
  myConfigPrioritySet = arg.myConfigPrioritySet;
  myConfigPriority = arg.myConfigPriority;
}

AREXPORT MvrArg &ArArg::operator=(const MvrArg & arg) 
{
	if (this != &arg) {
		myType = arg.myType;
		myName = arg.myName;
		myDescription = arg.myDescription;
		myIntPointer = arg.myIntPointer;
		myDoublePointer = arg.myDoublePointer;
		myPosePointer = arg.myPosePointer;
		myBoolPointer = arg.myBoolPointer;
		myStringPointer = arg.myStringPointer;
		myMinInt = arg.myMinInt;
		myMaxInt = arg.myMaxInt;
		myMinDouble = arg.myMinDouble;
		myMaxDouble = arg.myMaxDouble;
		myMaxStrLen = arg.myMaxStrLen;
		mySetFunctor = arg.mySetFunctor;
		myGetFunctor = arg.myGetFunctor;
		myConfigPrioritySet = arg.myConfigPrioritySet;
		myConfigPriority = arg.myConfigPriority;
	}
	return *this;
}


AREXPORT MvrArg::~ArArg()
{
}

AREXPORT void MvrArg::clear(void)
{
  myIntPointer = NULL;
  myDoublePointer = NULL;
  myBoolPointer = NULL;
  myPosePointer = NULL;
  myStringPointer = NULL;
  myMinInt = INT_MIN;
  myMaxInt = INT_MAX;
  myMinDouble = -HUGE_VAL;
  myMaxDouble = HUGE_VAL;
  myMaxStrLen = 0;
  mySetFunctor = NULL;
  myGetFunctor = NULL;  
  myConfigPrioritySet = false;
  myConfigPriority = MvrPriority::NORMAL;
}

/**
   @see INVALID
   @see INT
   @see DOUBLE
   @see BOOL
   @see POSE */
AREXPORT MvrArg::Type MvrArg::getType(void) const
{
  return myType;
}

AREXPORT int MvrArg::getMinInt(void) const
{
  return myMinInt;
}

AREXPORT int MvrArg::getMaxInt(void) const
{
  return myMaxInt;
}

AREXPORT double MvrArg::getMinDouble(void) const
{
  return myMinDouble;
}

AREXPORT double MvrArg::getMaxDouble(void) const
{
  return myMaxDouble;
}

AREXPORT const char *ArArg::getName(void) const
{
  return myName.c_str();
}

AREXPORT const char *ArArg::getDescription(void) const
{
  return myDescription.c_str();
}

AREXPORT int MvrArg::getInt(void) const
{ 
  if (myIntPointer != NULL)
    return *myIntPointer;
  else
    return 0;
}

AREXPORT double MvrArg::getDouble(void) const 
{
  if (myDoublePointer != NULL)
    return *myDoublePointer; 
  else
    return 0;
}

AREXPORT bool MvrArg::getBool(void) const
{
  if (myBoolPointer != NULL)
    return *myBoolPointer;
  else
    return false;
}

AREXPORT const char *ArArg::getString(void) const
{
  if (myStringPointer != NULL)
    return myStringPointer;
  else
    return NULL;
}

AREXPORT MvrPose MvrArg::getPose(void) const
{
  MvrPose pose;
  if (myPosePointer != NULL)
    return *myPosePointer;
  else
    return pose;
}

AREXPORT const std::list<ArArgumentBuilder *> *ArArg::getArgsWithFunctor(void) const
{
  if (myGetFunctor == NULL)
    return NULL;
  else
    return myGetFunctor->invokeR();
}

AREXPORT bool MvrArg::setInt(int val)
{
  if (val < myMinInt)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setInt value %d below range [%d, %d]", getName(), val, myMinInt, myMaxInt);
    return false;
  }
  if (val > myMaxInt)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setInt value %d above range [%d, %d]", getName(), val, myMinInt, myMaxInt);
    return false;
  }
  if (myIntPointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setInt called with NULL int pointer.", getName());
  }
  // if we got to here we're good
  *myIntPointer = val;
  return true;
}

AREXPORT bool MvrArg::setDouble(double val)
{ 
  if (val < myMinDouble)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setDouble value %g below range [%g, %g]", getName(), val, myMinDouble, myMaxDouble);
    return false;
  }
  if (val > myMaxDouble)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setDouble value %g above range [%g, %g]", getName(), val, myMinDouble, myMaxDouble);
    return false;
  }
  if (myDoublePointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setDouble called with NULL pointer.", getName());
    return false;
  }
  // if we got to here we're good
  *myDoublePointer = val;
  return true;
}


AREXPORT bool MvrArg::setBool(bool val)
{
  if (myBoolPointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setBool called with NULL pointer.", getName());
    return false;
  }
  *myBoolPointer = val;
  return true;
}

AREXPORT bool MvrArg::setString(const char *str)
{
  size_t len;
  if (myStringPointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setString called with NULL pointer.", getName());
    return false;
  }
  // this is >= so that if it wouldn't have room with NULL that's
  // taken care of too
  if ((len = strlen(str)) >= myMaxStrLen)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setString called with argument %d long, when max length is %d.", getName(), len, myMaxStrLen);
    return false;
  }
  strcpy(myStringPointer, str);
  return true;
}

AREXPORT bool MvrArg::setPose(MvrPose pose)
{
  if (myPosePointer == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setPose called with NULL pointer.", getName());
    return false;
  }
  *myPosePointer = pose;
  return true;

}

AREXPORT bool MvrArg::setArgWithFunctor(MvrArgumentBuilder *argument)
{
  if (mySetFunctor == NULL)
  {
    MvrLog::log(MvrLog::Normal, "MvrArg of %s: setArgWithFunctor called with NULL pointer.", getName());
    return false;
  }
  return mySetFunctor->invokeR(argument);
}


AREXPORT void MvrArg::log(void) const
{
  std::list<ArArgumentBuilder *>::const_iterator it;
  const std::list<ArArgumentBuilder *> *argList;

  switch (getType()) 
  {
  case MvrArg::INVALID:
    MvrLog::log(MvrLog::Terse, 
	       "\tType: %10s.  This argument was not created properly.", 
	       "invalid");
  case MvrArg::INT:
    MvrLog::log(MvrLog::Terse, "\tType: %10s name: %12s value: %d", "int", 
	       getName(), getInt());
    if (strlen(getDescription()) != 0)
      MvrLog::log(MvrLog::Terse, "\t\tDescription: %s",
		 getDescription());
    break;
  case MvrArg::DOUBLE:
    MvrLog::log(MvrLog::Terse, "\tType: %10s name: %12s value: %f", "double",
	       getName(), getDouble());
    if (strlen(getDescription()) != 0)
      MvrLog::log(MvrLog::Terse, "\t\tDescription: %s",
		 getDescription());
    break; 
  case MvrArg::STRING:
    MvrLog::log(MvrLog::Terse, "\tType: %10s name: %12s value: %s", "string", 
               getName(), getString());
    if (strlen(getDescription()) != 0)
      MvrLog::log(MvrLog::Terse, "\t\tDescription: %s",
                 getDescription());
    break;
  case MvrArg::BOOL:
    MvrLog::log(MvrLog::Terse, "\tType: %10s name: %12s value: %d", "bool",
	       getName(), getBool());
    if (strlen(getDescription()) != 0)
      MvrLog::log(MvrLog::Terse, "\t\tDescription: %s",
		 getDescription());
    break;
  case MvrArg::POSE:
    MvrLog::log(MvrLog::Terse, "\tType: %10s name: %12s value: (%.1f %.1f %.1f)",
	       "pose", getName(), getPose().getX(), getPose().getY(),
	       getPose().getTh());
    if (strlen(getDescription()) != 0)
      MvrLog::log(MvrLog::Terse, "\t\tDescription: %s",
		 getDescription());
    break;
  case MvrArg::FUNCTOR:
    MvrLog::log(MvrLog::Terse, "\tType: %10s name: %12s", 
	       "functor", getName(), getPose().getX(), getPose().getY(),
	       getPose().getTh());
    if (strlen(getDescription()) != 0)
      MvrLog::log(MvrLog::Terse, "\t\tDescription: %s",
		 getDescription());
    MvrLog::log(MvrLog::Terse, "\t\tValues:");
    argList = myGetFunctor->invokeR();
    for (it = argList->begin(); it != argList->end(); it++)
      MvrLog::log(MvrLog::Terse, "\t\t\t%s", (*it)->getFullString());
    break;
  case MvrArg::DESCRIPTION_HOLDER:
    MvrLog::log(MvrLog::Terse, "\tType: %20s Description: %s", 
	       "description_holder", getDescription());

  default:
    MvrLog::log(MvrLog::Terse, 
	       "\tType: %10s.  This type doesn't have a case in MvrArg::print.",
	       "unknown");
    break;
  }

  if (myConfigPrioritySet)
    MvrLog::log(MvrLog::Terse, "\t\tPriority: %s", 
	       MvrPriority::getPriorityName(myConfigPriority));
}

/**
   If this is true then the config priority is set and you can use
   getConfigPriority.
**/
AREXPORT bool MvrArg::getConfigPrioritySet(void) const
{
  return myConfigPrioritySet;
}

/**
   The priority of this argument when used in MvrConfig.
 **/
AREXPORT MvrPriority::Priority MvrArg::getConfigPriority(void) const
{
  return myConfigPriority;
}

/**
   The priority of this argument when used in MvrConfig.
 **/

AREXPORT void MvrArg::setConfigPriority(MvrPriority::Priority priority)
{
  myConfigPriority = priority;
  myConfigPrioritySet = true;
}
