/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRobot.h
 > Description  : This class will read a config packet from the robot.
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRROBOT_H
#define MVRROBOT_H

#include "mvriaTypedefs.h"
#include "MvrRobotPacketSender.h"
#include "MvrRobotPacketReceiver.h"
#include "MvrFunctor.h"
#include "MvrSyncTask.h"
#include "MvrSensorReading.h"
#include "MvrMutex.h"
#include "MvrCondition.h"
#include "MvrSyncLoop.h"
#include "MvrRobotPacketReaderThread.h"
#include "MvrRobotParams.h"
#include "MvrActionDesired.h"

/// TODO
#endif  // MVRROBOT_H