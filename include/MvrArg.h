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
#ifndef ARARG_H
#define ARARG_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "MvrFunctor.h"

class MvrArgumentBuilder;

/// Mvrgument class, mostly for actions, could be used for other things
/** 
    This is designed to be easy to add another type to the arguments... 
    All you have to do to do so, is add an enum to the Type enum, add a 
    newType getNewType(void), add a void setNewType(newType nt), and add
    a case statement for the newType to MvrArg::print.  You should probably also
    add an @see newType to the documentation for MvrArg::getType.

*/
class MvrArg
{
public:
  typedef enum 
  { 
    INVALID, ///< An invalid argument, the argument wasn't created correctly
    INT, ///< Integer argument
    DOUBLE, ///< Double argument
    STRING, ///< String argument
    BOOL, ///< Boolean argument
    POSE, ///< MvrPose argument
    FUNCTOR, ///< Mvrgument that handles things with functors
    DESCRIPTION_HOLDER, ///< Mvrgument that just holds a description
    
    LAST_TYPE = DESCRIPTION_HOLDER  ///< Last value in the enumeration
  } Type;


  enum {
    TYPE_COUNT = LAST_TYPE + 1 ///< Number of argument types
  };

  /// Default empty contructor
  AREXPORT MvrArg();
  /// Constructor for making an integer argument
  AREXPORT MvrArg(const char * name, int *pointer, 
		 const char * description = "", 
		 int minInt = INT_MIN, 
		 int maxInt = INT_MAX); 
  /// Constructor for making a double argument
  AREXPORT MvrArg(const char * name, double *pointer,
		 const char * description = "", 
		 double minDouble = -HUGE_VAL,
		 double maxDouble = HUGE_VAL); 
  /// Constructor for making a boolean argument
  AREXPORT MvrArg(const char * name, bool *pointer,
		 const char * description = ""); 
  /// Constructor for making a position argument
  AREXPORT MvrArg(const char * name, MvrPose *pointer,
		 const char * description = ""); 
  /// Constructor for making an argument of a string
  AREXPORT MvrArg(const char *name, char *pointer, 
		 const char *description,
		 size_t maxStrLen);
  /// Constructor for making an argument that has functors to handle things
  AREXPORT MvrArg(const char *name, 
		 MvrRetFunctor1<bool, MvrArgumentBuilder *> *setFunctor, 
		 MvrRetFunctor<const std::list<ArArgumentBuilder *> *> *getFunctor,
		 const char *description);
  /// Constructor for just holding a description (for MvrConfig)
  AREXPORT MvrArg(const char *description);
  /// Copy constructor
  AREXPORT MvrArg(const MvrArg & arg);
  /// Assignment operator
  AREXPORT MvrArg &operator=(const MvrArg &arg);
  /// Destructor
  AREXPORT virtual ~ArArg();

  /// Gets the type of the argument
  AREXPORT Type getType(void) const;
  /// Gets the name of the argument
  AREXPORT const char *getName(void) const;
  /// Gets the long description of the argument
  AREXPORT const char *getDescription(void) const;
  /// Sets the argument value, for int arguments
  AREXPORT bool setInt(int val);
  /// Sets the argument value, for double arguments
  AREXPORT bool setDouble(double val);
  /// Sets the argument value, for bool arguments
  AREXPORT bool setBool(bool val);
  /// Sets the argument value, for MvrPose arguments
  AREXPORT bool setPose(MvrPose pose);
  /// Sets the argument value for MvrArgumentBuilder arguments
  AREXPORT bool setString(const char *str);
  /// Sets the argument by calling the setFunctor callback
  AREXPORT bool setArgWithFunctor(MvrArgumentBuilder *argument);
  /// Gets the argument value, for int arguments
  AREXPORT int getInt(void) const; 
  /// Gets the argument value, for double arguments
  AREXPORT double getDouble(void) const;
  /// Gets the argument value, for bool arguments
  AREXPORT bool getBool(void) const;
  /// Gets the argument value, for pose arguments
  AREXPORT MvrPose getPose(void) const;
  /// Gets the argument value, for string arguments
  AREXPORT const char *getString(void) const;
  /// Gets the argument value, which is a list of argumentbuilders here
  AREXPORT const std::list<ArArgumentBuilder *> *getArgsWithFunctor(void) const;
  /// Logs the type, name, and value of this argument
  AREXPORT void log(void) const;
  /// Gets the minimum int value
  AREXPORT int getMinInt(void) const;
  /// Gets the maximum int value
  AREXPORT int getMaxInt(void) const;
  /// Gets the minimum double value
  AREXPORT double getMinDouble(void) const;
  /// Gets the maximum double value
  AREXPORT double getMaxDouble(void) const;
  /// Gets if the config priority is set
  AREXPORT bool getConfigPrioritySet(void) const;
  /// Gets the priority (only used by MvrConfig)
  AREXPORT MvrPriority::Priority getConfigPriority(void) const;
  /// Sets the priority (only used by MvrConfig)
  AREXPORT void setConfigPriority(MvrPriority::Priority priority);

private:
  /// Internal helper function
  AREXPORT void clear(void);

protected:

  MvrArg::Type myType;
  std::string myName;
  std::string myDescription;
  int *myIntPointer;
  int myMinInt, myMaxInt;
  double *myDoublePointer;
  double myMinDouble, myMaxDouble;
  bool *myBoolPointer;
  MvrPose *myPosePointer;
  char *myStringPointer;
  size_t myMaxStrLen;
  bool myConfigPrioritySet;
  MvrPriority::Priority myConfigPriority;
  MvrRetFunctor1<bool, MvrArgumentBuilder *> *mySetFunctor;
  MvrRetFunctor<const std::list<ArArgumentBuilder *> *> *myGetFunctor;
};

#endif // ARARGUMENT_H
