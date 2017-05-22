/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrTaskState.h
 > Description  : Class with the different states a task can be in
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRTASKSTATE_H
#define MVRTASKSTATE_H

class MvrTaskState
{
public:
  enum State{
    INIT = 0,  ///< Initialized (running)
    RESUME,    ///< Resumed after being suspended (running)
    ACTIVE,    ///< Active (running)
    SUSPEND,   ///< Suspended (not running) 
    SUCCESS,   ///< Succeeded and done (not running)
    FAILURE,    ///< Failed and done (not running)
    USER_START = 20 ///< This is where the user states should start (they will all be run)   
  };
};

#endif  // MVRTASKSTATE_H