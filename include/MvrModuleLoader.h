#ifndef MVRMODULELOADER_H
#define MVRMODULELOADER_H


#include <map>
#include <string>
#include "mvriaTypedefs.h"
#include "MvrRobot.h"


/// Dynamic MvrModule loader
/**
   The MvrModuleLoader is used to load MvrModules into a program and invoke
   them. 

   See also MvrModule to see how to define an MvrModule.

   See also the example programs advanced/simpleMod.cpp and advanced/simpleModule.cpp. 
*/
class MvrModuleLoader
{
public:

#ifdef WIN32
  typedef HINSTANCE DllRef;
#else
  typedef void * DllRef;
#endif

  typedef enum {
    STATUS_SUCCESS=0, ///< Load succeded
    STATUS_ALREADY_LOADED, ///< Module already loaded
    STATUS_FAILED_OPEN, ///< Could not find or open the module
    STATUS_INVALID, ///< Invalid module file format
    STATUS_INIT_FAILED, ///< The module failed its init stage
    STATUS_EXIT_FAILED, ///< The module failed its exit stage
    STATUS_NOT_FOUND ///< The module was not found
  } Status;

  /// Load an MvrModule
  MVREXPORT static Status load(const char *modName, MvrRobot *robot,
			      void *modArgument = NULL, bool quiet = false);
  /// Close and then reload an MvrModule
  MVREXPORT static Status reload(const char *modName, MvrRobot *robot,
				void * modArgument = NULL, bool quiet = false);
  /// Close an MvrModule
  MVREXPORT static Status close(const char *modName, bool quiet = false);
  /// Close all open MvrModule
  MVREXPORT static void closeAll();

protected:

  static std::map<std::string, DllRef> ourModMap;
};


#endif // MVRMODULELOADER_H
