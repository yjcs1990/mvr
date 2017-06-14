/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrModule.cpp
 > Description  : Dynamicly loaded module base class, read warning in more
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年06月13日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrModule.h"

MVREXPORT MvrModule::MvrModule() :
          myRobot(0)
{

} 

MVREXPORT MvrModule::~MvrMdule()
{
  
}         