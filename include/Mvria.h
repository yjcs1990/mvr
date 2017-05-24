/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : Mvria.h
 > Description  : Some head file 
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月17日
***************************************************************************************************/

#ifndef MVRIA_H
#define MVRIA_H

#include "mvriaOSDef.h"
#include "mvriaTypedefs.h"
#include "MvrSerialConnection.h"
#include "MvrTcpConnection.h"
#include "MvrSimpleConnection.h"
#include "MvrLogFileConnection.h"
#include "MvrLog.h"
#include "MvrRobotPacket.h"
#include "MvrRobotPacketSender.h"
#include "MvrRobotPacketReceiver.h"
#include "MvrRobotConfigPacketReader.h"
#include "MvrRobotTypes.h"
#include "mvriaUtil.h"
#include "MvrArgumentBuilder.h"
#include "MvrArgumentParser.h"
#include "MvrFileParser.h"
#include "MvrConfig.h"
#include "MvrConfigArg.h"
#include "MvrConfigGroup.h"
#include "MvrRobot.h"
#include "MvrCommands.h"
#include "MvrJoyHandler.h"
#include "MvrSyncTask.h"
#include "MvrTaskState.h"
#include "MvriaInternal.h"
#include "MvrSonarDevice.h"
#include "MvrPriorityResolver.h"
#include "MvrAction.h"
#include "MvrActionGroup.h"
#include "MvrActionGroups.h"
#include "MvrActionDeceleratingLimiter.h"
#include "MvrActionLimiterForwards.h"
/// TODO
#endif  // MVRIA_H