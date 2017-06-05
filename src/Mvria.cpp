/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : Mvria.cpp
 > Description  : Some head file 
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年06月04日
***************************************************************************************************/
#include "MvrExport.h"
#include "Mvria.h"
#include "MvrSocket.h"
#include "MvrSingalHandler.h"
#include "MvrKeyHandler.h"
#include "MvrJoyHandler.h"
#ifndef MVRINTERFACE
#include "MvrModuleLoader.h"
#include "MvrRobotJoyHandler.h"
#include "MvrSystemStatus.h"
#endif  // MVRINTERFACE
