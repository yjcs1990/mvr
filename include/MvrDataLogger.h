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
#ifndef ARDATALOGGER_H
#define ARDATALOGGER_H

#include "ariaUtil.h"
#include "MvrMutex.h"
#include "MvrFunctor.h"
#include <vector>

class MvrRobot;
class MvrConfig;


/// This class will log data, but you have to use it through an MvrConfig right now
/**
   This class doesn't log anything by default, but can be set up to
   log all sorts of data.  Note that if you do an addString after you
   do an addToConfig it'll automatically be enabled (since right now
   we don't want to change config after loading since the values would
   wind up wierd).

  @ingroup OptionalClasses
 **/
class MvrDataLogger
{
public:
  /// Constructor
  MVREXPORT MvrDataLogger(MvrRobot *robot, const char *fileName = NULL);
  /// Destructor
  MVREXPORT ~MvrDataLogger();
  /// Adds the data logger information to the config
  MVREXPORT void addToConfig(MvrConfig *config);
  /// Adds a string to the list of options in the raw format
  MVREXPORT void addString(const char *name, MvrTypes::UByte2 maxLen, 
			  MvrFunctor2<char *, MvrTypes::UByte2> *functor);

  /// Gets the functor for adding a string (for MvrStringInfoGroup)
  MvrFunctor3<const char *, MvrTypes::UByte2,
				    MvrFunctor2<char *, MvrTypes::UByte2> *> *
                     getAddStringFunctor(void) { return &myAddStringFunctor; }

protected:
  MVREXPORT void connectCallback(void);
  MVREXPORT bool processFile(char *errorBuffer, size_t errorBufferLen);
  MVREXPORT void userTask(void);
  MvrRobot *myRobot;
  MvrTime myLastLogged;
  MvrConfig *myConfig;
  bool myAddToConfigAtConnect;
  bool myAddedToConfig;

  FILE *myFile;
  bool myConfigLogging;
  int myConfigLogInterval;
  char myOpenedFileName[512];
  char myConfigFileName[512];
  std::string myPermanentFileName;

  // for what we're logging
  bool myLogVoltage;
  bool myLogStateOfCharge;
  bool myLogLeftVel;
  bool myLogRightVel;
  bool myLogTransVel;
  bool myLogRotVel;
  bool myLogLatVel;
  bool myLogLeftStalled;
  bool myLogRightStalled;
  bool myLogStallBits;
  bool myLogFlags;
  bool myLogFaultFlags;
  int myAnalogCount;
  bool *myAnalogEnabled;
  int myAnalogVoltageCount;
  bool *myAnalogVoltageEnabled;
  int myDigInCount;
  bool *myDigInEnabled;
  int myDigOutCount;
  bool *myDigOutEnabled;
  bool myLogPose;
  bool myLogEncoderPose;
  bool myLogCorrectedEncoderPose;
  bool myLogEncoders;
  bool myLogChargeState;
  bool myLogBatteryInfo;
  int myStringsCount;
  std::vector<bool *> myStringsEnabled;

  MvrMutex myMutex;

  std::vector<ArStringInfoHolder *> myStrings;
  MvrTypes::UByte2 myMaxMaxLength;
  MvrFunctor3C<ArDataLogger, const char *, MvrTypes::UByte2,
		    MvrFunctor2<char *, MvrTypes::UByte2> *> myAddStringFunctor;
  

  MvrFunctorC<ArDataLogger> myConnectCB;  
  MvrRetFunctor2C<bool, MvrDataLogger, char *, size_t> myProcessFileCB;
  MvrFunctorC<ArDataLogger> myUserTaskCB;
};

#endif // ARDATALOGGER_H
