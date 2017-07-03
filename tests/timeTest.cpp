#include "Mvria.h"

int main(void)
{
  Mvria::init();

  MvrLog::log(MvrLog::Terse, "\nTesting MvrTime with large values:");
  
  MvrTime startLarge, testlarge;
  MvrLog::log(MvrLog::Normal, "Setting an MvrTime object \"startLarge\" to now...");
  startLarge.setToNow();
  MvrLog::log(MvrLog::Normal, "mSecSince %ld secSince %ld", 
              startLarge.mSecSince(), startLarge.secSince());

  long adding = pow(2, 31) - 10 * 1000;
  MvrLog::log(MvrLog::Normal, "Adding %d milliseconds", adding);
  startLarge.addMSec(adding);
  MvrLog::log(MvrLog::Normal, "mSecSince %ld secSince %ld", 
              startLarge.mSecSince(), startLarge.secSince());
  MvrLog::log(MvrLog::Normal, "mSecSinceLL %lld secSince %lld",
              startLarge.mSecSinceLL(), startLarge.secSinceLL());

  MvrLog::log(MvrLog::Normal, "Adding %d milliseconds", adding);
  startLarge.addMSec(adding);
  MvrLog::log(MvrLog::Normal, "mSecSince %ld secSince %ld", 
              startLarge.mSecSince(), startLarge.secSince());
  MvrLog::log(MvrLog::Normal, "mSecSinceLL %lld secSince %lld",
              startLarge.mSecSinceLL(), startLarge.secSinceLL()); 

  MvrLog::log(MvrLog::Normal, "\nTesting platform localtime (broken-down) struct:");
  struct tm t;
  MvrUtil::localtime(&t);
  MvrLog::log(MvrLog::Normal, "MvrUtil::localtime() returned: year=%d mon=%d mday=%d hour=%d min=%d sec=%d",
    t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  time_t yesterday = time(NULL) - (24*60*60);
  MvrUtil::localtime(&yesterday, &t);
  MvrLog::log(MvrLog::Normal, "MvrUtil::localtime(time(NULL) - 24hours, struct tm*) returned: year=%d mon=%d mday=%d hour=%d min=%d sec=%d",
    t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

  MvrLog::log(MvrLog::Normal, "\nTesting MvrTime:");
  MvrTime start, test;

  MvrLog::log(MvrLog::Normal, "Setting an MvrTime object \"start\" to now ...");
  start.setToNow();
  start.log();
  MvrLog::log(MvrLog::Normal, "Sleeping 4 secs");
  MvrUtil::sleep(4000);
  MvrLog::log(MvrLog::Normal, "Seting an MvrTime object \"test\" to now ... ");
  test.setToNow();
  test.log();  
  
  MvrLog::log(MvrLog::Normal, "ms of \"test\" since start %ld", test.mSecSince(start));
  MvrLog::log(MvrLog::Normal, "seconds of \"test\" since start %ld", test.secSince(start));
  
  MvrLog::log(MvrLog::Normal, "ms of start since \"test\" %ld", start.mSecSince(test));
  MvrLog::log(MvrLog::Normal, "seconds of start since \"test\" %ld", start.secSince(test));

  MvrLog::log(MvrLog::Normal, "\"start\" is before \"test\"? %d", test.isBefore(start));
  MvrLog::log(MvrLog::Normal, "\"start\" is after \"test\"? %d", test.isAfter(start));

  MvrLog::log(MvrLog::Normal, "\"test\" is before \"start\"? %d", start.isBefore(test));
  MvrLog::log(MvrLog::Normal, "\"test\" is after \"start\"? %d", start.isAfter(test));

  MvrLog::log(MvrLog::Normal, "\"test\" is before \"start\"? %d", test.mSecSince(start));
  MvrLog::log(MvrLog::Normal, "\"test\" is after \"start\"? %d", test.secSince(start));

  MvrLog::log(MvrLog::Normal, "ms from \"start\" to now %ld", start.mSecTo());
  MvrLog::log(MvrLog::Normal, "s from \"start\" to now %ld", start.secTo());

  MvrLog::log(MvrLog::Normal, "ms since \"start\" %ld", start.mSecSince());
  MvrLog::log(MvrLog::Normal, "s since \"start\" %ld", start.secSince());

  MvrLog::log(MvrLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  MvrLog::log(MvrLog::Normal, "s from \"test\" stamp to now %ld", test.secTo());

  MvrLog::log(MvrLog::Normal, "Testing addMSec, adding 200 mSec");
  test.addMSec(200);
  MvrLog::log(MvrLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  MvrLog::log(MvrLog::Normal, "Testing addMSec, subtracting 300 mSec");
  test.addMSec(-300);
  MvrLog::log(MvrLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  MvrLog::log(MvrLog::Normal, "Testing addMSec, adding 20.999 seconds");
  test.addMSec(20999);
  MvrLog::log(MvrLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());
  MvrLog::log(MvrLog::Normal, "Testing addMSec, subtracting 23.5 seconds");
  test.addMSec(-23500);
  MvrLog::log(MvrLog::Normal, "ms from \"test\" stamp to now %ld", test.mSecTo());

  MvrTime timeDone;
  MvrLog::log(MvrLog::Normal, "Setting MvrTime object \"done\" to now.");
  timeDone.setToNow();
  timeDone.addMSec(1000);
  MvrLog::log(MvrLog::Normal, "Making sure the add works in the right direction, adding a second to a timestamp set now");
  MvrLog::log(MvrLog::Normal, "Reading: %ld", timeDone.mSecTo());
  MvrLog::log(MvrLog::Normal, "Sleeping 20 ms");
  MvrUtil::sleep(20);
  MvrLog::log(MvrLog::Normal, "Reading: %ld", timeDone.mSecTo());
  MvrLog::log(MvrLog::Normal, "Sleeping 2 seconds");
  MvrUtil::sleep(2000);
  MvrLog::log(MvrLog::Normal, "Reading: %ld", timeDone.mSecTo());


  // puts("\nslamming MvrUtil::localtime() from a bunch of threads with the same input time...");
  // time_t now = time(NULL);
  // class LocaltimeTestThread : public virtual MvrASyncTask 
  // {
  // private:
  //   time_t time;
  // public:
  //   LocaltimeTestThread(time_t t) : time(t) {}
  //   virtual void *runThread(void *) 
  //   {
  //     struct tm t;
  //     MvrUtil::localtime(&time, &t);
  //     MvrLog::log(MvrLog::Normal, "MvrUtil::localtime() returned: year=%d mon=%d mday=%d hour=%d min=%d sec=%d", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  //     return 0;
  //   }
  // };

  // for(int i = 0; i < 200; ++i)
  //   (new LocaltimeTestThread(now))->runAsync();
  // MvrUtil::sleep(5000);
 

  MvrLog::log(MvrLog::Normal, "test is done.");
}