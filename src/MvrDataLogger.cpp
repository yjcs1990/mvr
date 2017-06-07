/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrDataLogger.cpp
 > Description  : This class will log data, but you have to use it through an MvrConfig right now
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年06月07日
***************************************************************************************************/
#include "mvriaOSDef.h"
#include "MvrExport.h"
#include "MvrRobot.h"
#include "MvrConfig.h"
#include "MvrDataLogger.h"
#include "MvrRobotBatteryPacketReader.h"

#include <vector>

/*
   @param robot the robot to log information from
   
   @param fileName if NULL then the file name is detered by the
   config, if not NULL then the file name data is put into.
 */
MVREXPORT MvrDataLogger::MvrDataLogger(MvrRobot *robot, const char *fileName) :
          myAddStringFunctor(this, &MvrDataLogger::addString),
          myConnectCB(this, &MvrDataLogger::connectCallback),
          myProcessFileCB(this, &MvrDataLogger::processFile),
          myUserTaskCB(this, &MvrDataLogger::userTask)
{
  myMutex.setLogName("MvrDataLogger::myMutex");
  myRobot = robot;
  if (fileName == NULL || fileName[0] == '\0')
    myPermanentFileName = "";
  else
    myPermanentFileName = fileName;
  myRobot->addUserTask("DataLogger", 50, &myUserTaskCB);
  myRobot->requestIOPackets();
  myConfig = NULL;
  myAddToConfigAtConnect = false;
  myAddedToConfig        = false;
  myConfigLogging        = false;
  myConfigLogInterval    = 0;
  myConfigFileName       = '\0';
  myOpenedFileName       = '\0';
  myAnalogCount          = 0;
  myAnalogEnabled        = NULL;
  myAnalogVoltageCount   = 0;
  myAnalogVoltageEnabled = NULL;
  myDigInCount           = 0;
  myDigInEnabled         = NULL;
  myDigOutCount          = 0;
  myDigOutEnabled        = NULL;
  myStringsCount         = 0;

  myLogVoltage           = false;
  myLogStateOfCharge     = false;
  myLogLeftVel           = false;
  myLogRightVel          = false;
  myLogTransVel          = false;
  myLogRotVel            = false;
  myLogLeftStalled       = false;
  myLogRightStalled      = false;
  myLogStallBits         = false;
  myLogFlags             = false;
  myLogEncoderPose       = false;
  myLogCorrectedEncoderPose = false;
  myLogEncoders          = false;
  myLogChargeState       = false;
  myLogBatteryInfo       = false;

  myFile                 = NULL;
}                  

MVREXPORT MvrDataLogger::~MvrDataLogger(void)
{

}

MVREXPORT void MvrDataLogger::addToConfig(MvrConfig *config)
{
  if (config == NULL || myAddedToConfig)
    return;
  myConfig = config;
  if (!myRobot->isConnected())
  {
    myAddToConfigAtConnect = true;
    myRobot->addConnectCB(&myConnectCB);
    return;
  }
  else
  {
    connectCallback();
  }

  myAddedToConfig = true;
  MvrLog::log(MvrLog::Verbose, "MvrDataLogger:L addToConfig");
  std::string section;
  char name[512];
  char desc[512];
  int i;
  section = "Data logging";
  myConfig->addParam(MvrConfigArg("DataLog", &myConfigLogging, "True to log data, false not to"),
                     section.c_str(), MvrPriority::NORMAL);
  myConfig->addParam(MvrConfigArg("DataLogInterval", &myConfigLoggingInterval, "Seconds between logs",0),
                     section.c_str(), MvrPriority::NORMAL);                     
  if (myPermanentFileName.size() == 0)
      myConfig->addParam(MvrConfigArg("DataLogFileName", myConfigFileName, "File to log data into",
                         sizeof(myConfigFileName)), section.c_str(), MvrPriority::NORMAL);  
  for (i=0; i< myStringsCount; i++)
  {
    snprintf(name, sizeof(name), "DataLog%s", myStrings[i]->getName());
    snprintf(name, sizeof(desc), "Logs %s", myStrings[i]->getName());
    myConfig->addParam(MvrConfigArg(name, myStringsEnabled[i],desc),
                       "Custom data logging", MvrPriority::NORMAL);   
  }
  myConfig->addParam(MvrConfigArg("DataLogBatteryVoltage", &myLogVoltage, "True to log battery voltage"),
	                   section.c_str(), MvrPriority::DETAILED);
  if (myRobot->haveStateOfCharge())
    myConfig->addParam(MvrConfigArg("DataLogStateOfCharge", &myLogStateOfCharge, "True to log state of charge"),
	                     section.c_str(), MvrPriority::DETAILED);
  
  myConfig->addParam(MvrConfigArg("DataLogChargeState", &myLogChargeState, "True to log charge state"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogBatteryInfo", &myLogBatteryInfo, "True to log battery info (if available)"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogPose", &myLogPose, "True to log robot's pose"), section.c_str(), MvrPriority::NORMAL);
  myConfig->addParam(MvrConfigArg("DataLogEncoderPose", &myLogEncoderPose, "True to log robot's raw encoder pose"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogCorrectedEncoderPose", &myLogCorrectedEncoderPose, "True to log robot's corrected (by gyro, etc) encoder pose"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogEncoders", &myLogEncoders, "True to log the raw encoder readings"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogLeftVel", &myLogLeftVel, "True to log left wheel velocity"),
                     section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogRightVel", &myLogRightVel, "True to log right wheel velocity"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogTransVel", &myLogTransVel, "True to log translational wheel velocity"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogRotVel", &myLogRotVel, "True to log rotational wheel velocity"),
	                   section.c_str(), MvrPriority::DETAILED);
  if (myRobot->hasLatVel())
    myConfig->addParam(MvrConfigArg("DataLogLatVel", &myLogRotVel, "True to log lateral wheel velocity"),
	                     section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogLeftStalled", &myLogLeftStalled, "True to log if the left wheel is stalled"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogRightStalled", &myLogRightStalled, "True to log if the right wheel is stalled"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogStallBits", &myLogStallBits, "True to log all the stall bits is stalled"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogFlags", &myLogFlags, "True to log all the flags"),
	                   section.c_str(), MvrPriority::DETAILED);
  myConfig->addParam(MvrConfigArg("DataLogFaultFlags", &myLogFaultFlags, "True to log all the fault flags"),
	                   section.c_str(), MvrPriority::DETAILED);
  for (i=0; i<myAnalogCount; i++)
  {
    snprintf(name, sizeof(name), "DataLogAnalog%d",i);
    snprintf(decs, sizeof(decs), "Logs the value of analog %d as a 10 bit (0-1024) value",i);
    myConfig->addParam(MvrConfigArg(name, &myAnalogEnabled[i], desc),
                       section.c_str(), MvrPriority::DETAILED);
  }
  for (i=0; i<myAnalogVoltageCount; i++)
  {
    snprintf(name, sizeof(name), "DataLogAnalogVoltage%d",i);
    snprintf(decs, sizeof(decs), "Logs the value of analog %d as voltage from 0 to 5",i);
    myConfig->addParam(MvrConfigArg(name, &myAnalogVoltageEnabled[i], desc),
                       section.c_str(), MvrPriority::DETAILED);   
  }
  for (i = 0; i < myDigInCount; i++)
  {
    snprintf(name, sizeof(name), "DataLogDigIn%d", i);
    snprintf(desc, sizeof(desc), "Logs digital in %d", i);
    myConfig->addParam(MvrConfigArg(name, &myDigInEnabled[i], desc),
	                     section.c_str(), MvrPriority::DETAILED);
  }
  for (i = 0; i < myDigOutCount; i++)
  {
    snprintf(name, sizeof(name), "DataLogDigOut%d", i);
    snprintf(desc, sizeof(desc), "Logs digital out %d", i);
    myConfig->addParam(MvrConfigArg(name, &myDigOutEnabled[i], desc),
	                     section.c_str(), MvrPriority::DETAILED);
  }
  myProcessFileCB.setName("MvrDataLogger");
  myConfig->addProcessFileWithErrorCB(&myProcessFileCB, 100);
}

MVREXPORT void MvrDataLogger::connectCallback(void)
{
  int i;
  MvrLog::log(MvrLog::Verbose, "MvrDataLogger::connectCallback");
  if (myAnalogEnabled != NULL)
  {
    delete myAnalogEnabled;
    myAnalogEnabled = NULL;
  }

  if (myAnalogVoltageEnabled != NULL)
  {
    delete myAnalogVoltageEnabled;
    myAnalogVoltageEnabled = NULL;
  }
  if (myDigInEnabled != NULL)
  {
    delete myDigInEnabled;
    myDigInEnabled = NULL;
  }
  if (myDigOutEnabled != NULL)
  {
    delete myDigOutEnabled;
    myDigOutEnabled = NULL;
  }

  myAnalogCount          = myRobot->getIOAnalogSize();
  myAnalogVoltageCount   = myRobot->getIOAnalogSize();
  myDigInCount           = myRobot->getIODigInSize();
  myDigOutCount          = myRobot->getIODigOutSize();

  if (myAnalogCount > 0)
  {
    myAnalogEnabled = new bool[myAnalogCount];
    for (i=0; i<myAnalogCount; i++){
      myAnalogEnabled[i] = false;
    }
  }
  if (myAnalogVoltageCount > 0)
  {
    myAnalogVoltrageEnabled = new bool[myAnalogVoltageCount];
    for (i=0; i<myAnalogVoltageCount; i++){
      myAnalogVoltrageEnabled[i] = false;
    }  
  }
  if (myDigInCount > 0)
  {
    myDigInEnabled = new bool[myDigInCount];
    for (i = 0; i < myDigInCount; i++)
      myDigInEnabled[i] = false;
  }
  if (myDigOutCount > 0)
  {
    myDigOutEnabled = new bool[myDigOutCount];
    for (i = 0; i < myDigOutCount; i++)
      myDigOutEnabled[i] = false;
  }    
  if (myAddToConfigAtConnect && !myAddedToConfig)
  {
    myAddToConfigAtConnect = false;
    addToConfig(myConfig);
  }
}

MVREXPORT bool MvrDataLogger::processFile(char *errorBuffer, size_t errorBufferLen)
{
  myMutex.lock();
  // if our file name is different and we're not using a permanent
  // file name or if we're disabled close the old one
  if ((strcmp(myOpenedFileName, myConfigFileName) != 0 && myFile != NULL && myPermanentFileName.size() == 0 )||
       (myFile != NULL && !myConfigLogging))
  {
    MvrLog::log(MvrLog::Normal, "Closed data log file '%s'", myOpenedFileName);
    fclose(myFile);
    myFile = NULL;
  }
  // Try to open the file
  if (myConfigLogging && myFile == NULL)
  {
    if (myPermanentFileName.size() == 0 && strlen(myConfigFileName) == 0)
    {
      MvrLog::log(MvrLog::Verbose, "MvrDataLogger: no log file to open");
      myMutex.unlock();
      return true;
    }
    std::string fileName;
    if (myPermanentFileName.size() > 0)
    {
      if ((myFile == MvrUtil::fopen(myPermanentFileName.c_str(), "a")) != NULL)
      {
        MvrLog::log(MvrLog::Normal, "Opened data log file '%s'", myPermanentFileName.c_str());
      }
      else
      {
        MvrLog::log(MvrLog::Normal, "Could not open data log file '%s'", myPermanentFileName.c_str());
        myMutex.unlock();
        return true;
      }
    }
    else
    {
      if ((myFile == MvrUtil::fopen(myConfigFileName, "w")) != NULL)
      {
        strcpy(myOpenedFileName, myConfigFileName);
        MvrLog::log(MvrLog::Normal, "Opened data log file '%s'", myOpenedFileName);
      }
      else
      {
        MvrLog::log(MvrLog::Normal, "Could not open data log file '%s'", myConfigFileName);
        myMutex.unlock();
        if (errorBuffer != NULL)
          snprintf(errorBuffer, errorBufferLen, "DataLogFileName of '%s' cannont be opened", myConfigFileName);
        return false;
      }
    }
  }
  else if (!myConfigLogging)
  {
    myMutex.unlock();
    return true;
  }

  int i;
  // if we could then dump in the header
  fprintf(myFile, ";%12s", "Time");
  std::map<std::string, bool *, MvrStrCaseCmpOp>::iterator it;
  for (i=0; i<myStringsCount; i++)
  {
    if (*myStringsEnabled[i]){
      char formatBuf[64];
      sprintf(formatBuf, "\t%%0%ds", myStrings[i]->getMaxLength());
      fprintf(myFile, formatBuf, myStrings[i]->getName());
  }
  if (myLogVoltage)
    fprintf(myFile, "\tVolt");
  if (myLogStateOfCharge)
    fprintf(myFile, "\tSoC");
  if (myLogChargeState)
    fprintf(myFile, "\t%015s\t%5s", "ChargeStageName", "csNum");
  if (myLogBatteryInfo && myRobot->getBatteryPacketReader() != NULL)
  {
    myRobot->getBatteryPacketReader()->requestContinuousCellInfoPackets();
    int battery;
    for (battery=1; battery<=myRobot->getBatteryPacketReader()->getNumBatteries(); battery++)
    {
      fprintf(myFile, "\tbat%02dflags1 \tbat%02dflags2 \tbat%02dflags3 \tbat%02drelsoc\tbat%02dabssoc", battery, battery, battery, battery, battery);
    }
  }
  if (myLogPose)
    fprintf(myFile, "\t%010s\t%010s\t%010s", "X", "Y","Th");
  if (myLogEncoderPose)
    fprintf(myFile, "\t%010s\t%010s\t%010s", "encX", "encY", "encTh");
  if (myLogCorrectedEncoderPose)
    fprintf(myFile, "\t%010s\t%010s\t%010s", "corrEncX", "corrEncY", "corrEncTh");   
  if (myLogEncoders)
  {
    fprintf(myFile, "\t010s\t%010s", "encL", "encR");
    myRobot->requestEncoderPackets();
  }
  if (myLogLeftVel)
    fprintf(myFile, "\tLeftV");
  if (myLogRightVel)
    fprintf(myFile, "\tRightV");
  if (myLogTransVel)
    fprintf(myFile, "\tTransV");
  if (myLogRotVel)
    fprintf(myFile, "\tRotV");
  if (myLogLatVel)
    fprintf(myFile, "\tLatV"); 
  if (myLogLeftStalled)
    fprintf(myFile, "\LStall"); 
  if (myLogRightStalled)
    fprintf(myFile, "\RStall");     
  if (myLogStallBits)
    fprintf(myFile, "\tStllBts%16s", "");
  if (myLogFlags)
    fprintf(myFile, "\tFlags%16s", "");
  if (myLogFaultFlags)
    fprintf(myFile, "\tFault Flags%10s", "");
  for (i=0; i<myAnalogCount; i++)
  {
    if (myAnalogEnabled[i])
      fprintf(myFile, "\tAn%d", i);
  }
    for (i=0; i<myAnalogVoltageCount; i++)
  {
    if (myAnalogVoltageEnabled[i])
      fprintf(myFile, "\tAnV%d", i);
  }
  for (i = 0; i < myDigInCount; i++)
  {
    if (myDigInEnabled[i])
      fprintf(myFile, "\tDigIn%d%8s", i, "");
  }
  for (i = 0; i < myDigOutCount; i++)
  {
    if (myDigOutEnabled[i])
      fprintf(myFile, "\tDigOut%d%8s", i, "");
  }

  fprintf(myFile, "\n");
  fflush(myFile);
  myMutex.unlock();
  return true;
}

MVREXPORT void MvrDataLogger::userTask(void)
{
  myMutex.lock();
  // if we don't need to do anything just return
  if (myFile == NULL || myLastLogged.secSince() < myConfigLogInterval)
  {
    myMutex.unlock();
    return;
  }
  int i;
  int j;
  int val;

  fprintf(myFile, "%ld", time(NULL));

  char *buf;
  buf = new char [myMaxMaxLength];
  MvrStringInfoHolder *infoHolder;
  for (i=0; i<myStringsCount; i++)
  {
    if (*(myStringsEnabled[i]))
    {
      char formatBuf[64];
      infoHolder = myStrings[i];
      sprintf(formatBuf, "\t%%0%ds", myStrings[i]->getMaxLength());
      infoHolder->getFunctor->invoke(buf, inforHolder->getMaxLength());
      fprintf(myFile, formatBuf, buf);
    }
  }
  delete buf;

  if (myLogVoltage)
    fprintf(myFile, "\t%.2f", myRobot->getRealBatteryVoltageNow());
  if (myLogStateOfCharge)
    fprintf(myFile, "\t%.0f", myRobot->getStateOfCharge());
  if (myLogChargeState)
  {
    MvrRobot::ChargeState chargeStage = myRobot->getChargeState();
    std::string chargeString;
    if (chargeState == MvrRobot::CHARGING_UNKNOWN)
      chargeString = "Unknowable";
    else if (chargeState == MvrRobot::CHARGING_NOT)
      chargeString = "Not";
    else if (chargeState == MvrRobot::CHARGING_BULK)
      chargeString = "Bulk";
    else if (chargeState == MvrRobot::CHARGING_OVERCHARGE)
      chargeString = "Overcharge";
    else if (chargeState == MvrRobot::CHARGING_FLOAT)
      chargeString = "Float";
    else if (chargeState == MvrRobot::CHARGING_BALANCE)
      chargeString = "Balance";
    else
      chargeString = "Unknown";
    fprintf(myFile, "\t%15s\t%5d", chargeString.c_str(), chargeState);
  }
  if (myLogBatteryInfo && myRobot->getBatteryPacketReader() != NULL)
  {
    int battery;
    int flags;
    for (battery=1; battery <= myRobot.getBatteryPacketReader()->getNumBatteries(); battery++)
    {
      fprintf(myFile, "\t");
      flags = myRobot->getBatteryPacketReader()->getFlags1(battery);
      for (i = 0, val = 1; i < 8; i++, val *= 2)
	      printf(myFile, "%d", (bool) (flags & val));
      fprintf(myFile, "   ");

      fprintf(myFile, "\t");
      flags = myRobot->getBatteryPacketReader()->getFlags2(battery);
      for (i = 0, val = 1; i < 8; i++, val *= 2)
	      fprintf(myFile, "%d", (bool) (flags & val));
      fprintf(myFile, "   ");

      fprintf(myFile, "\t");
      flags = myRobot->getBatteryPacketReader()->getFlags3(battery);
      for (i = 0, val = 1; i < 8; i++, val *= 2)
	      fprintf(myFile, "%d", (bool) (flags & val));
      fprintf(myFile, "   ");
      
      fprintf(myFile, "\t%11d\t%11d", 
              myRobot->getBatteryPacketReader()->getRelSOC(battery),
              myRobot->getBatteryPacketReader()->getAbsSOC(battery));   
    }
  }

  if (myLogPose)
    fprintf(myFile, "\t%10.0f\t%10.0f\t%10.0f", 
            myRobot->getX(),
            myRobot->getY(),
            myRobot->getTh());
  if (myLogEncoderPose)
    fprintf(myFile, "\t%10.0f\t%10.0f\t%10.0f",
            myRobot->getRawEncoderPose().getX(),
            myRobot->getRawEncoderPose().getY(),
            myRobot->getRawEncoderPose().getTh());
  if (myLogCorrectedEncoderPose)
    fprintf(myFile, "\t%10.0f\t%10.0f\t%10.0f",
            myRobot->getEncoderPose().getX(),
            myRobot->getEncoderPose().getY(),
            myRobot->getEncoderPose().getTh());  
  if (myLogEncoders)
    fprintf(myFile, "\t%10ld\t%10ld",
            myRobot->getLeftEncoder(),
            myRobot->getRightEncoder());
  if (myLogLeftVel)
    fprintf(myFile, "\t%.0f", myRobot->getLeftVel());
  if (myLogRightVel)
    fprintf(myFile, "\t%.0f", myRobot->getRightVel());
  if (myLogTransVel)
    fprintf(myFile, "\t%.0f", myRobot->getVel());  
  if (myLogRotVel)
    fprintf(myFile, "\t%.0f", myRobot->getRotVel());          
  if (myLogLatVel)
    fprintf(myFile, "\t%.0f", myRobot->getLatVel());    
  if (myLogLeftStalled)
    fprintf(myFile, "\t%d", (bool)myRobot->isLeftMotorStalled());
  if (myLogRightStalled)
    fprintf(myFile, "\t%d", (bool)myRobot->isRightMotorStalled());       
  if (myLogStallBits)
  {
    fprintf(myFile, "\t");
    for (i=0, val=1; i<16; i++, val *= 2)
      fprintf(myFile, "%d",  (bool)(myRobot->getStallValue() &val));
  } 
  if (myLogFlags)
  {
    fprintf(myFile, "\t");
    for (i=0, val=1; i<16; i++, val *= 2)
      fprintf(myFile, "%d",  (bool)(myRobot->getFlags() &val));
  }
  if (myLogFaultFlags)
  {
    for (i=0, val=1; i<16; i++, val *= 2)
      fprintf(myFile, "%d",  (bool)(myRobot->getFaultFlags() &val));
  }
  for (i=0; i<myAnalogCount; i++){
    if (myAnalogEnabled[i])
      fprintf(myFile, "\t%d", myRobot->getIOAnalog(i));
  }
  for (i = 0; i < myAnalogVoltageCount; i++)
  {
    if (myAnalogVoltageEnabled[i])
      fprintf(myFile, "\t%.2f", myRobot->getIOAnalogVoltage(i));
  }
  for (i = 0; i < myDigInCount; i++)
  {
    if (myDigInEnabled[i])
    {
      fprintf(myFile, "\t");
      for (j = 0, val = 1; j < 8; j++, val *= 2)
	fprintf(myFile, "%d", (bool)(myRobot->getIODigIn(i) & val));
    }
  }
  for (i = 0; i < myDigOutCount; i++)
  {
    if (myDigOutEnabled[i])
    {
      fprintf(myFile, "\t");
      for (j = 0, val = 1; j < 8; j++, val *= 2)
	fprintf(myFile, "%d", (bool)(myRobot->getIODigOut(i) & val));
    }
  }

  fprintf(myFile, "\n");
  fflush(myFile);
  myLastLogged.setToNow();
  myMutex.unlock();  
}

MVREXPORT void MvrDataLogger::addString(const char *name, MvrTypes::UByte2 maxLength,
                                        MvrFunctor2<char *, MvrTypes::UByte2> *functor)
{
  MvrTypes::UByte2 len;

  myMutex.lock();
  if (maxLength < stlen(name))
    len = (MvrTypes::UByte2) strlen(name);
  else
    len = maxLength;
  if (myMaxMaxLength < len)
    myMaxMaxLength = len;

  myStrings.push_back(new MvrStringInfoHolder(name, len, functor));
  bool *boolPtr;
  boolPtr = new bool;

  if (myAddedToConfig)
    *boolPtr = true;
  else
    *boolPtr = false;
  myStringsEnabled.push_back(boolPtr);
  myStringsCount++;
  myMutex.unlock();
  if (myAddedToConfig)
    processFile(NULL, 0);
}                                        
