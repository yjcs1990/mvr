#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrCondition.h"
#include "MvrLog.h"


MvrStrMap MvrCondition::ourStrMap;


MVREXPORT MvrCondition::MvrCondition() :
  myFailedInit(false),
  myCond(),
  myCount(0)
{
  myCond=CreateEvent(0, FALSE, FALSE, 0);
  if (myCond == NULL)
  {
	DWORD err = GetLastError(); // see http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx
    MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::MvrCondition: Error %d trying to create the condition.", getLogName(), err);
    myFailedInit=true;
  }

  ourStrMap[STATUS_FAILED]="General failure";
  ourStrMap[STATUS_FAILED_DESTROY]=
  "Another thread is waiting on this condition so it can not be destroyed";
  ourStrMap[STATUS_FAILED_INIT] =
  "Failed to initialize thread. Requested action is imposesible";
  ourStrMap[STATUS_MUTEX_FAILED_INIT]="The underlying mutex failed to init";
  ourStrMap[STATUS_MUTEX_FAILED]="The underlying mutex failed in some fashion";
}

MVREXPORT MvrCondition::~MvrCondition()
{
  if (!myFailedInit && !CloseHandle(myCond))
    MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::~MvrCondition: Unknown error while trying to destroy the condition.", getLogName());
}

MVREXPORT int MvrCondition::signal()
{
  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::signal: Initialization of condition failed, failed to signal", getLogName());
    return(STATUS_FAILED_INIT);
  }

  if (!PulseEvent(myCond))
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::signal: Unknown error while trying to signal the condition.", getLogName());
    return(STATUS_FAILED);
  }

  return(0);
}

MVREXPORT int MvrCondition::broadcast()
{
  int ret=0;

  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::broadcast: Initialization of condition failed, failed to broadcast", getLogName());
    return(STATUS_FAILED_INIT);
  }

  //MvrLog::log(MvrLog::Normal, "broadcasting %s", getLogName());
  for (; myCount != 0; --myCount)
  {
    if (PulseEvent(myCond) == 0) // PulseEvent returns 09 on error according to http://msdn.microsoft.com/en-us/library/windows/desktop/ms684914(v=vs.85).aspx
    {
	  DWORD err = GetLastError(); // see http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx
      MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::broadcast: Error %d while trying to broadcast the condition.", getLogName(), err);
      ret=STATUS_FAILED;
    }
  }

  return(ret);
}

MVREXPORT int MvrCondition::wait()
{
  DWORD ret;

  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::wait: Initialization of condition failed, failed to wait", getLogName());
    return(STATUS_FAILED_INIT);
  }

  ++myCount;
  ret=WaitForSingleObject(myCond, INFINITE);
  if (ret == WAIT_OBJECT_0)
    return(0);
  else
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrCondition(%s)::wait: Failed to lock due to an unknown error", getLogName());
    return(STATUS_FAILED);
  }
}

MVREXPORT int MvrCondition::timedWait(unsigned int msecs)
{
  int ret;

  if (myFailedInit)
  {
    MvrLog::log(MvrLog::Terse, "MvrCondition(%s)::wait: Initialization of condition failed, failed to wait", getLogName());
    return(STATUS_FAILED_INIT);
  }

  ++myCount;
  ret=WaitForSingleObject(myCond, msecs);
  if (ret == WAIT_OBJECT_0)
    return(0);
  else if (ret == WAIT_TIMEOUT)
    return(STATUS_WAIT_TIMEDOUT);
  else
  {
    MvrLog::logNoLock(MvrLog::Terse, "MvrCondition(%s)::timedWait: Failed to lock due to an unknown error", getLogName());
    return(STATUS_FAILED);
  }
}

MVREXPORT const char *MvrCondition::getError(int messageNumber) const
{
  MvrStrMap::const_iterator it;
  if ((it = ourStrMap.find(messageNumber)) != ourStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}
