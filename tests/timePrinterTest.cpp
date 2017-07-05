#include "Mvria.h"

int main(void)
{
  Mvria::init();
  MvrLog::init(MvrLog::StdOut, MvrLog::Normal, "", true);

  if (MvrTime::usingMonotonicClock())
    MvrLog::log(MvrLog::Normal, "Using monotonic clock");
  else
    MvrLog::log(MvrLog::Normal, "Using normal clock that will have issues if time of day is changed");
  
  MvrTime time;
  while(Mvria::getRunning())
  {
    time.setToNow();
    MvrLog::log(MvrLog::Normal, "%10s\t%lu%10s\t%lu", "", time.getSec(), "", time.getMSec());
    MvrUtil::sleep(1000);
  }
}
