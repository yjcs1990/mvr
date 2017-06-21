/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobotParams.h
 > Description  : Stores a set of video device parameters read from one of the video sections 
                  of a robot parameter file.
 > Author       : Yu Jie
 > Create Time  : 2017年05月19日
 > Modify Time  : 2017年06月21日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaInternal.h"
#include "MvrRobotParams.h"

#include <sstream>

bool MvrRobotParams::ourUseDefaultBehavior = true;
std::string ourPowerOutputDisplayHint;

void MvrRobotParams::internalSetUseDefaultBehavior(bool useDefaultBehavior, const char *powerOutputDisplayHint)
{
  ourUseDefaultBehavior     = useDefaultBehavior;
  ourPowerOutputDisplayHint = powerOutputDisplayHint;

  MvrLog::log(MvrLog::Normal,
              "MvrRobotParams: Setting use default behavior to %s, with power output display hint '%s'",
              MvrUtil::convertBool(ourUseDefaultBehavior),
              ourPowerOutputDisplayHint);
}

bool MvrRobotParams::internalGetUseDefaultBehavior(void)
{
  return ourUseDefaultBehavior;
}

MVREXPORT MvrRobotParams::MvrRobotParams() :
          MvrConfig(NULL, true),
          mySonarUnitGetFunctor(this, &MvrRobotParams::getSonarUnits),
          mySonarUnitSetFunctor(this, &MvrRobotParams::parseSonarUnit),
          myIRUnitGetFunctor(this, &MvrRobotParams::getIRUnits),
          myIRUnitSetFunctor(this, &MvrRobotParams::parseIRUnit),
          myCommercialProcessFileCB(this, &MvrRobotParams::myCommercialProcessFileCB)
{
  
}          