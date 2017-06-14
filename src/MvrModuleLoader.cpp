/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrModuleLoader.cpp
 > Description  : Dynamic MvrModule loader
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年06月13日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#ifdef WIN32
#else
#include <dlfcn.h>
#endif
#include "MvrModule.h"
#include "MvrLog.h"
#include "MvrModuleLoader.h"

std::map<std::string, MvrModuleLoader::DllRef> MvrModuleLoader::ourModMap;

#ifdef WIN32
#define RTLD_NOW 0
#define RTLD_GLOBAL 0

HINSTANCE dlopen(const char *fileName, int flag)
{
  return(LoadLibrary(fileName));
}

int dlclose(HINSTANCE handle)
{
  FreeLibrary(handle);
  return(0);
}

void *dlsym(HINSTANCE handle, char *symbol)
{
  return (void*)(GetProcAddress(handle, symbol));
}

const char *dlerror(void)
{
  return(0);
}
#endif // WIN32

/**
   THIS ONLY LOADS one init on the module right now, if its called
   again it'll load the same init over.  I'll fix it later... read the
   more verbose description in MvrModule.h.

   @param modName fileName of the module without the extension (.dll
   or .so) 
   
   @param robot MvrRobot reference which the module is to use, this can
   be NULL
   
   @param modArgument A void pointer argument to pass to the module,
   if its a const value you'll need to cast it to a non-const value to
   get it to work (for example if you were using a constant string).
   This value defaults to NULL.

   @param quiet whether to print out a message if this fails or not,
   defaults to false
**/
MVREXPORT MvrModuleLoader::Status MvrModuleLoader::load(const char *modName,
                                                        MvrRobot *robot,
                                                        void *modArgument,
                                                        bool quiet)
{
  std::string name;
  std::map<std::string, DllRef>::iterator iter;
  DllRef handle;
  bool (*func)(MvrRobot*,void*);
  bool ret;

  name=modName;
#ifdef WIN32
  if (strstr(modName, ".dll") == 0)
    name+=".dll";
#else
  if (strstr(modName, ".so") == 0)
    name+=".so";
#endif

  iter=ourModMap.find(name);
  if (iter != ourModMap.end())
    return(STATUS_ALREADY_LOADED);

  handle=dlopen(name.c_str(), RTLD_NOW | RTLD_GLOBAL);

  if (!handle || dlerror() != NULL)
  {
    if (!quiet)
      MvrLog::log(MvrLog::Terse, "Failure to load module '%s': %s",
		 name.c_str(), dlerror());
    return(STATUS_FAILED_OPEN);
  }

  func=(bool(*)(MvrRobot*,void*))dlsym(handle, "ariaInitModule");
  if (!func || dlerror() != NULL)
  {
    if (!quiet)
      MvrLog::log(MvrLog::Terse, "No module initializer for %s.", modName);
    ourModMap.insert(std::map<std::string, DllRef>::value_type(name,
							       handle));
    return(STATUS_SUCCESS);
  }
  ret=(*func)(robot, modArgument);
  
  if (ret)
  {
    ourModMap.insert(std::map<std::string, DllRef>::value_type(name,
							       handle));
    return(STATUS_SUCCESS);
  }
  else
  {
    if (!quiet)
      MvrLog::log(MvrLog::Terse, "Module '%s' failed its init sequence",
		 name.c_str());
    dlclose(handle);
    return(STATUS_INIT_FAILED);
  }
}


/**
   reload() is similar to load(), except that it will call close() on the
   module and then call load().
   @param modName fileName of the module without the extension (.dll or .so)
   @param robot MvrRobot instance to provide to the module
   @param modArgument application-specific data to provide to the module
   @param quiet If true, do not log errors.
*/
MVREXPORT MvrModuleLoader::Status MvrModuleLoader::reload(const char *modName,
                                                          MvrRobot *robot,
                                                          void *modArgument,
                                                          bool quiet)
{
  close(modName, quiet);
  return(load(modName, robot, modArgument, quiet));
}

/**
   Calls MvrModule::exit() on the module, then closes the library.
   @param modName fileName of the module without the extension (.dll or .so)
   @param quiet whether to print out a message if this fails or not,
   defaults to false
*/
MVREXPORT MvrModuleLoader::Status MvrModuleLoader::close(const char *modName,
						                                             bool quiet)
{
  std::string name;
  std::map<std::string, DllRef>::iterator iter;
  bool (*func)();
  bool funcRet;
  DllRef handle;
  Status ret=STATUS_SUCCESS;

  name=modName;
#ifdef WIN32
  if (strstr(modName, ".dll") == 0)
    name+=".dll";
#else
  if (strstr(modName, ".so") == 0)
    name+=".so";
#endif

  iter=ourModMap.find(name.c_str());
  if (iter == ourModMap.end())
  {
    MvrLog::log(MvrLog::Terse, "Module '%s' could not be found to be closed.",
	       modName);
    return(STATUS_NOT_FOUND);
  }
  else
  {
    handle=(*iter).second;
    func=(bool(*)())dlsym(handle, "ariaExitModule");
    if (!func)
    {
      if (!quiet)
        MvrLog::log(MvrLog::Verbose, 
            "Failure to find module exit function for '%s'", (*iter).first.c_str());
      ourModMap.erase(name);
      return STATUS_SUCCESS;
    }
    funcRet=(*func)();
    if (funcRet)
      ret=STATUS_SUCCESS;
    else
    {
      if (!quiet)
        MvrLog::log(MvrLog::Terse, "Module '%s' failed its exit sequence",
            modName);
      ret=STATUS_INIT_FAILED;
    }
    dlclose(handle);
    ourModMap.erase(name);
  }

  return(ret);
}

MVREXPORT void MvrModuleLoader::closeAll()
{
  std::map<std::string, DllRef>::iterator iter;

  while ((iter = ourModMap.begin()) != ourModMap.end())
    close((*iter).first.c_str());
}
