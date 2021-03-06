#include "MvrExport.h"
#include "mvriaUtil.h"

#include "MvrCameraCollection.h"

MVREXPORT MvrCameraCollection::MvrCameraCollection() :
  myMutex(),
	myCameraToInfoMap()
{
  myMutex.setLogName("MvrCameraCollection::myMutex");
} // end ctor

MVREXPORT MvrCameraCollection::~MvrCameraCollection()
{
  MvrUtil::deleteSetPairs(myCameraToInfoMap.begin(),
                         myCameraToInfoMap.end());
  myCameraToInfoMap.clear();

} // end dtor

MVREXPORT bool MvrCameraCollection::addCamera(const char *cameraName,
											                      const char *cameraType,
											                      const char *displayName,
											                      const char *displayType)
{
  if (cameraName == NULL) {
    return false;
  }

  lock();
  CameraInfo *info = findCameraInfo(cameraName);
  if (info != NULL) {
    unlock();
    return false;
  }

  info = new CameraInfo();
  
  info->myCameraName = cameraName;
  info->myCameraType = cameraType;
  info->myDisplayName = ((displayName != NULL) ? displayName : cameraName);
  info->myDisplayType = ((displayType != NULL) ? displayType : cameraType);

  myCameraToInfoMap[cameraName] = info;

  unlock();
  return true;

} // end method addCamera


MVREXPORT bool MvrCameraCollection::removeCamera(const char *cameraName)
{
  if (cameraName == NULL) {
    return false;
  }
  lock();

  std::map<std::string, CameraInfo*>::iterator iter = 
                                            myCameraToInfoMap.find(cameraName);
  if (iter == myCameraToInfoMap.end()) {
    unlock();
    return false;
  }

  delete iter->second;
  iter->second = NULL;

  myCameraToInfoMap.erase(iter);

  setModified();

  unlock();

  return true;

} // end method removeCamera


MVREXPORT bool MvrCameraCollection::addCameraCommand(const char *cameraName,
												                           const char *command,
												                           const char *cameraCommandName,
                                                   int requestInterval)
{
  if (command == NULL) {
    return false;
  }
  lock();

  CameraInfo *cameraInfo = findCameraInfo(cameraName);
  if (cameraInfo == NULL) {
    unlock();
    return false;
  }

  CommandInfo *info = findCommandInfo(cameraName, command);
  if (info != NULL) {
    unlock();
    return false;
  }

  info = new CommandInfo();
  info->myCommand = command;
  info->myCameraCommandName = cameraCommandName;
  info->myRequestInterval = requestInterval;


  cameraInfo->myCommandToInfoMap[command] = info;

  setModified();
  unlock();

  return true;

} // end method addCameraCommand


MVREXPORT bool MvrCameraCollection::removeCameraCommand(const char *cameraName,
													                            const char *command)
{
  lock();

  CameraInfo *info = findCameraInfo(cameraName);
  if (info == NULL) {
    unlock();
    return false;
  }

  std::map<std::string, CommandInfo*>::iterator iter = 
                                        info->myCommandToInfoMap.find(command);

  if (iter == info->myCommandToInfoMap.end()) {
    unlock();
    return false;
  }

  delete iter->second;
  iter->second = NULL;

  info->myCommandToInfoMap.erase(iter);

  setModified();
  unlock();

  return true;

} // end method removeCameraCommand


MVREXPORT bool MvrCameraCollection::addParameter(const char *cameraName,
                                              MvrCameraParameterSource *source,
                                              const MvrConfigArg &param)
{
  lock();
  CameraInfo *camInfo = findCameraInfo(cameraName);
  if ((camInfo == NULL) || (param.getName() == NULL)) {
    unlock();
    return false;
  }

  ParamInfo *info = findParamInfo(cameraName, param.getName());
  if (info != NULL) {
    unlock();
    return false;
  }

  info = new ParamInfo();
  info->mySource = source;
  info->myParam = param;

  camInfo->myParamToInfoMap[param.getName()] = info;

  unlock();
  return true;

} // end method addParameter


MVREXPORT bool MvrCameraCollection::removeParameter(const char *cameraName,
                                                  const char *paramName)
{
  lock();
  CameraInfo *camInfo = findCameraInfo(cameraName);
  if ((camInfo == NULL) || (paramName == NULL)) {
    unlock();
    return false;
  }

  std::map<std::string, ParamInfo*>::iterator iter = 
        camInfo->myParamToInfoMap.find(paramName);

  if (iter == camInfo->myParamToInfoMap.end()) {
    unlock();
    return false;
  }

  camInfo->myParamToInfoMap.erase(iter);

  unlock();
  return true;

} // end method removeParameter 


MVREXPORT void MvrCameraCollection::getCameraNames(std::list<std::string> &outList)
{
  lock();
  outList.clear();

  for (std::map<std::string, CameraInfo*>::iterator iter = myCameraToInfoMap.begin();
       iter != myCameraToInfoMap.end();
       iter++) {
    outList.push_back(iter->first);

  } // end for each map entry

  unlock();

} // end method getCameraNames


MVREXPORT const char *MvrCameraCollection::getCameraType(const char *cameraName)
{
  const char *type = NULL;

  lock();

  CameraInfo *info = findCameraInfo(cameraName);
  if (info != NULL) {
    type = info->myCameraType.c_str();
  }
  unlock();

  return type;

} // end method getCameraType


MVREXPORT const char *MvrCameraCollection::getDisplayName(const char *cameraName)
{
  const char *displayName = NULL;

  lock();

  CameraInfo *info = findCameraInfo(cameraName);
  if (info != NULL) {
    displayName = info->myDisplayName.c_str();
  }
  unlock();

  return displayName;

} // end method getDisplayName


MVREXPORT const char *MvrCameraCollection::getDisplayType(const char *cameraName)
{
  const char *displayType = NULL;

  lock();

  CameraInfo *info = findCameraInfo(cameraName);
  if (info != NULL) {
    displayType = info->myDisplayType.c_str();
  }
  unlock();

  return displayType;

} // end method getDisplayType


MVREXPORT void MvrCameraCollection::getCameraCommands(const char *cameraName,
                                                    std::list<std::string> &outList)
{
  lock();
  outList.clear();

  CameraInfo *info = findCameraInfo(cameraName);
  if (info != NULL) {
    for (std::map<std::string, CommandInfo*>::iterator iter = info->myCommandToInfoMap.begin();
        iter != info->myCommandToInfoMap.end();
        iter++) {
      outList.push_back(iter->first);

    } // end for each map entry
  } // end if info found

  unlock();

} // end method getCameraCommands


MVREXPORT const char *MvrCameraCollection::getCommandName(const char *cameraName,
														                            const char *command)
{
  const char *cameraCommandName = NULL;

  lock();

  CommandInfo *info = findCommandInfo(cameraName, command);
  if (info != NULL) {
    cameraCommandName = info->myCameraCommandName.c_str();
  } // end if info found

  unlock();

  return cameraCommandName;

} // end method getCommandName


MVREXPORT int MvrCameraCollection::getRequestInterval(const char *cameraName,
														                        const char *command)
{
  int interval = -1;

  lock();

  CommandInfo *info = findCommandInfo(cameraName, command);
  if (info != NULL) {
    interval = info->myRequestInterval;
  } // end if info found

  unlock();

  return interval;

} // end method getRequestInterval


MVREXPORT void MvrCameraCollection::getParameterNames
                                      (const char *cameraName,
                                       std::list<std::string> &outList)
{
  lock();
  outList.clear();

  CameraInfo *info = findCameraInfo(cameraName);
  if (info != NULL) {
    for (std::map<std::string, ParamInfo*>::iterator iter = info->myParamToInfoMap.begin();
         iter != info->myParamToInfoMap.end();
         iter++) {
      outList.push_back(iter->first);

    } // end for each map entry
  } // end if info found

  unlock();

} // end method getParameterNames

MVREXPORT bool MvrCameraCollection::getParameter(const char *cameraName,
                                               const char *parameterName,
                                               MvrConfigArg &paramOut)
{
  lock();

  ParamInfo *info = findParamInfo(cameraName, parameterName);
  if (info == NULL) {
    unlock();
    return false;
  }
  paramOut = info->myParam;
  unlock();

  return true;

} // end method getParameter


  
MVREXPORT bool MvrCameraCollection::setParameter(const char *cameraName,
                                               const MvrConfigArg &param)
{
  lock();

  ParamInfo *info = findParamInfo(cameraName, param.getName());
  if (info == NULL) {
    unlock();
    return false;
  }
  info->myParam = param;
  
  if (info->mySource != NULL) {
    info->mySource->setParameter(param);
  }

  unlock();
  return true;

} // end method changeParameter


MVREXPORT bool MvrCameraCollection::exists(const char *cameraName)
{
  lock();

  CameraInfo *info = findCameraInfo(cameraName);

  unlock();

  return (info != NULL);

} // end method exists


MVREXPORT bool MvrCameraCollection::exists(const char *cameraName,
				         const char *command)
{
  if ((cameraName == NULL) || (command == NULL)) {
    return false;
  }

  lock();

  CommandInfo *info = findCommandInfo(cameraName, command);

  unlock();

  return (info != NULL);

} // end method exists
  

MVREXPORT bool MvrCameraCollection::parameterExists(const char *cameraName,
					          const char *paramName)
{
  if ((cameraName == NULL) || (paramName == NULL)) {
    return false;
  }
  lock();

  ParamInfo  *info = findParamInfo(cameraName, paramName);

  unlock();

  return (info != NULL);

}


MVREXPORT void MvrCameraCollection::startUpdate()
{
  lock();
  myIsUpdatesEnabled = false;
  unlock();

} // end method startUpdate


MVREXPORT void MvrCameraCollection::endUpdate()
{
  lock();
  myIsUpdatesEnabled = true;
  // Now that updates have been re-enabled, check whether the collection was
  // modified and notify any listeners.
  notifyModifiedListeners();
  unlock();

} // end method endUpdate


MVREXPORT bool MvrCameraCollection::addModifiedCB(MvrFunctor *functor,
                                                MvrListPos::Pos position)
{
  if (functor == NULL) {
    return false;
  }

  bool isSuccess = true;
  lock();

  switch (position) {
  case MvrListPos::FIRST:
    myModifiedCBList.push_front(functor);
    break;

  case MvrListPos::LAST:
    myModifiedCBList.push_back(functor);
    break;

  default:
    isSuccess = false;
    MvrLog::log(MvrLog::Terse, 
               "MvrCameraCollection::addModifiedCB: Unrecognized position = %i.", 
               position);
    break;
  }

  unlock();

  return isSuccess;

} // end method addModifiedCB


MVREXPORT bool MvrCameraCollection::removeModifiedCB(MvrFunctor *functor)
{
  if (functor == NULL) {
    return false;
  }

  lock();
  // Could/should add a check to make sure functor is really there in order
  // to return a more accurate status... (But this probably isn't really 
  // an issue since functors aren't removed all that often.)
  myModifiedCBList.remove(functor);
  unlock();

  return true;

} // end method removeModifiedCB


MvrCameraCollection::CameraInfo *MvrCameraCollection::findCameraInfo(const char *cameraName)
{
  if (cameraName == NULL) {
    return NULL;
  }

  CameraInfo *info = NULL;

  std::map<std::string, CameraInfo*>::iterator iter = 
                                            myCameraToInfoMap.find(cameraName);

  if (iter != myCameraToInfoMap.end()) {
    info = iter->second;
  }

  return info;

} // end method findCameraInfo


MvrCameraCollection::CommandInfo *MvrCameraCollection::findCommandInfo
                                                        (const char *cameraName,
                                                         const char *commandName)
{
  CameraInfo *cameraInfo = findCameraInfo(cameraName);
  if (cameraInfo == NULL) {
    return NULL;
  }

  CommandInfo *info = NULL;

  std::map<std::string, CommandInfo*>::iterator iter = 
                                     cameraInfo->myCommandToInfoMap.find(commandName);

  if (iter != cameraInfo->myCommandToInfoMap.end()) {
    info = iter->second;
  }

  return info;

} // end method findCommandInfo

MvrCameraCollection::ParamInfo *MvrCameraCollection::findParamInfo
                                                      (const char *cameraName,
                                                       const char *paramName)
{
  CameraInfo *cameraInfo = findCameraInfo(cameraName);
  if (cameraInfo == NULL) {
    return NULL;
  }

  ParamInfo *info = NULL;

  std::map<std::string, ParamInfo*>::iterator iter = 
                                     cameraInfo->myParamToInfoMap.find(paramName);

  if (iter != cameraInfo->myParamToInfoMap.end()) {
    info = iter->second;
  }

  return info;

} // end method findParamInfo



void MvrCameraCollection::setModified()
{
  myIsModified = true;
  notifyModifiedListeners();
}


void MvrCameraCollection::notifyModifiedListeners()
{
  if (!myIsUpdatesEnabled || !myIsModified) {
    return;
  }


  myIsModified = false;

} // end method notifyModifiedListeners


MvrCameraCollection::CameraInfo::CameraInfo() :
    myCameraName(),
    myCameraType(),
    myDisplayName(),
    myDisplayType(),
    myCommandToInfoMap(),
    myParamToInfoMap()
{
}

MvrCameraCollection::CameraInfo::~CameraInfo()
{
  MvrUtil::deleteSetPairs(myCommandToInfoMap.begin(), 
                         myCommandToInfoMap.end());
  myCommandToInfoMap.clear();
}


MvrCameraCollection::CommandInfo::CommandInfo() :
  myCommand(),
  myCameraCommandName(),
  myRequestInterval(-1)
{
}

MvrCameraCollection::CommandInfo::~CommandInfo()
{
}


MvrCameraCollection::ParamInfo::ParamInfo() :
  mySource(NULL),
  myParam()
{
}

MvrCameraCollection::ParamInfo::~ParamInfo()
{
}


