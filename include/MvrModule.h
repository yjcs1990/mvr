#ifndef MVRMODULE_H
#define MVRMODULE_H


#include "mvriaTypedefs.h"
#include "MvrRobot.h"


/// Dynamicly loaded module base class, read warning in more
/**
   Right now only one module's init will be called, that is the first
   one, its a bug that I just don't have time to fix at the moment.
   I'll get to it when I have time or if someone needs it... someone
   else wrote this code so it'll take me a little longer to fix it.

   This class defines a dyanicmly loaded module of code. This is usefull
   for an application to load piece of code that it does not know about.
   The MvrModule defines and interface in which to invoke that piece of code
   that the program does not know about. For instance, a module could
   contain an MvrAction and the modules init() could instantiate the MvrAction
   and add it to the supplied MvrRobot. The init() takes a reference to an
   MvrRobot. The module should use that robot for its purposes. If the module
   wants to use more robots, assuming there are multiple robots, it can use
   Mvria::getRobotList() to find all the MvrRobot instantiated. The module
   should do all its clean up in exit().

   The user should derive their own class from MvrModule and implement the
   init() and exit() functions. The users code should always clean up
   when exit() is called. exit() is called right before the module (dynamic
   library .dll/.so) is closed and removed from the program.

   The macro ARDEF_MODULE() must be called within the .cpp file of the users
   module. A global instance of the users module must be defined and a
   reference to that instance must be passed to ARDEF_MODULE(). This allows
   the MvrModuleLoader to find the users module class and invoke it.

   One thing to note about the use of code wrapped in MvrModules and staticly
   linking in that code. To be able to staticly link .cpp files which contain
   an MvrModule, the ARIA_STATIC preprocessor symbol should be defined. This will cause
   the ARDEF_MODULE() to do nothing. If it defined its normal functions and
   variables, the linker would fail to staticly link in multiple modules
   since they all have symbols with the same name.

   Refer to MvrModuleLoader to see how to load an MvrModule into a program.

   For examples, see the programs advanced/simpleMod.cpp and advanced/simpleModule.cpp. 
*/
class MvrModule
{
public:

  /// Constructor
  MVREXPORT MvrModule();
  /// Destructor
  MVREXPORT virtual ~MvrModule();

  /// Initialize the module. The module should use the supplied MvrRobot pointer
  /**
     @param robot Robot this module should attach to or operate on. Note, may be NULL.

     @param argument an optional argument passed to the module. Default
     is NULL.  The module can interpret this in any way, it is recommended
     that you make sure this is documented if used. 
  */
  MVREXPORT virtual bool init(MvrRobot *robot, 
			     void *argument = NULL) = 0;

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


#ifdef ARIA_STATIC

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


#endif // MVRMODULE_H
