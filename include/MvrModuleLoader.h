/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrModuleLoader.h
 > Description  : Dynamic MvrModule loader
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRMODULELOADER_H
#define MVRMODULELOADER_H

#include <map>
#include <string>
#include "mvriaTypedefs.h"
#include "MvrRobot.h"

class MvrModuleLoader
{
public:

#ifdef WIN32
  typedef HINSTANCE DllRef;
#else
  typedef void * DllRef;
#endif

  typedef enum {
    STATUS_SUCCESS=0,       ///< Load succeded
    STATUS_ALREADY_LOADED,  ///< Module already loaded
    STATUS_FAILED_OPEN,     ///< Could not find or open the module
    STATUS_INVALID,         ///< Invalid module file format
    STATUS_INIT_FAILED,     ///< The module failed its init stage
    STATUS_EXIT_FAILED,     ///< The module failed its exit stage
    STATUS_NOT_FOUND        ///< The module was not found
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


#endif  // MVRMODULELOADER_H