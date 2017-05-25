/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrModule.h
 > Description  : Dynamicly loaded module base class, read warning in more
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRMODULE_H
#define MVRMODULE_H

#include "mvriaTypedefs.h"
#include "MvrRobot.h"

class MvrModule
{
public:

  /// Constructor
  MVREXPORT MvrModule();
  /// Destructor
  MVREXPORT virtual ~MvrModule();

  /// Initialize the module. The module should use the supplied MvrRobot pointer
  MVREXPORT virtual bool init(MvrRobot *robot, void *argument = NULL) = 0;

  /// Close down the module and have it exit
  MVREXPORT virtual bool exit() = 0;

  /// Get the MvrRobot pointer the module should be using
  MVREXPORT MvrRobot * getRobot() {return(myRobot);}

  /// Set the MvrRobot pointer
  MVREXPORT void setRobot(MvrRobot *robot) {myRobot=robot;}

protected:

  /// Stored MvrRobot pointer that the module should use
  MvrRobot *myRobot;
};


/*
  Beware ye all who pass beyond this point. Ugly macros abound and you might
  get eaten by a gru if your light fails.  */


#ifdef MVRIA_STATIC

#define MVRDEF_MODULE(mod)

#else

#ifdef WIN32

#define MVRDEF_MODULE(mod) \
extern "C" {\
static MvrModule *__MvriaModule_##mod = &mod; \
_declspec(dllexport) bool \
ariaInitModule(MvrRobot *robot, void *argument = NULL) \
{ \
  if (__MvriaModule_##mod) \
  { \
    __MvriaModule__->setRobot(robot); \
    return(__MvriaModule_##mod->init(robot, argument)); \
  } \
  else \
    return(false); \
} \
_declspec(dllexport) bool ariaExitModule() \
{ \
  if (__MvriaModule_##mod) \
    return(__MvriaModule_##mod->exit()); \
  return(false); \
} \
}
#else // WIN32

#define MVRDEF_MODULE(mod) \
static MvrModule *__MvriaModule_##mod = &mod; \
extern "C" {\
bool ariaInitModule(MvrRobot *robot, void *argument = NULL) \
{ \
  if (__MvriaModule_##mod) \
  { \
    __MvriaModule_##mod->setRobot(robot); \
    return(__MvriaModule_##mod->init(robot, argument)); \
  } \
  else \
    return(false); \
} \
bool ariaExitModule() \
{ \
  if (__MvriaModule_##mod) \
    return(__MvriaModule_##mod->exit()); \
  return(false); \
} \
}

#endif // WIN32

#endif // MVRIA_STATIC

#endif  // MVRMODULE_H