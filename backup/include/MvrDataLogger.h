/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrDataLogger.h
 > Description  : This class will log data, but you have to use it through an MvrConfig right now
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRDATALOGGER_H
#define MVRDATALOGGER_H

#include "mvriaUtil.h"
#include "MvrMutex.h"
#include "MvrFunctor.h"
#include <vector>

class MvrRobot;
class MvrConfig;

/*
   This class doesn't log anything by default, but can be set up to
   log all sorts of data.  Note that if you do an addString after you
   do an addToConfig it'll automatically be enabled.
*/

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
  MvrFunctor3<const char *, MvrTypes::UByte2, MvrFunctor2<char *, MvrTypes::UByte2> *> *
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

  std::vector<MvrStringInfoHolder *> myStrings;
  MvrTypes::UByte2 myMaxMaxLength;
  MvrFunctor3C<MvrDataLogger, const char *, MvrTypes::UByte2, MvrFunctor2<char *, MvrTypes::UByte2> *> myAddStringFunctor;
  

  MvrFunctorC<MvrDataLogger> myConnectCB;  
  MvrRetFunctor2C<bool, MvrDataLogger, char *, size_t> myProcessFileCB;
  MvrFunctorC<MvrDataLogger> myUserTaskCB;
};
#endif  // MVRDATALOGGER_H