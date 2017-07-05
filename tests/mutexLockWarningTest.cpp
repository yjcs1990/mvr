#include "Mvria.h"

int main(int argc, char **argv)
{
  Mvria::init();
  MvrMutex mutex;
  mutex.setLogName("test mutex");
  MvrLog::log(MvrLog::Normal, "This test succeeds if three (and only three) mutex lock/unlock time warning follow.");
  puts("setting test_mutex warning time to 1 sec");
  mutex.setUnlockWarningTime(1);  // 1 sec
  puts("locking and unlocking immediately, should not warn...");
  mutex.lock();
  mutex.unlock();
  puts("locking and unlocking after 2 sec, should warn...");
  mutex.lock();
  MvrUtil::sleep(2000);
  mutex.unlock(); // should warn
  puts("locking and unlocking after 0.5 sec, should not warn...");
  mutex.lock();
  MvrUtil::sleep(500);
  mutex.unlock();
  puts("setting test_mutex warning time to 0.5 sec");
  mutex.setUnlockWarningTime(0.5); // 0.5 sec
  puts("locking and unlocking after 0.6 sec, should warn...");
  mutex.lock();
  MvrUtil::sleep(600); // 0.6 sec
  mutex.unlock(); // should warn
  puts("locking and unlocking after 0.2 sec, should not warn...");
  mutex.lock();
  MvrUtil::sleep(200); // 0.2 sec
  mutex.unlock(); // should not warn
  puts("locking and unlocking immediately, should not warn...");
  mutex.lock();
  mutex.unlock(); // should not warn
  puts("setting test_mutex warning time to 0.1 sec");
  mutex.setUnlockWarningTime(0.1);  // 0.1 sec
  puts("locking and unlocking after 0.2 sec, should warn...");
  mutex.lock();
  MvrUtil::sleep(200); // 0.2 sec
  mutex.unlock(); // should warn
  mutex.setUnlockWarningTime(0.0); // off
  mutex.lock();
  MvrUtil::sleep(100); // should not warn
  mutex.unlock();

  // Create and destroy a few mutexes, locking them, etc.
  MvrMutex *m1 = new MvrMutex();
  m1->setLogName("m1");
  m1->lock();
  MvrMutex *m2 = new MvrMutex();
  m2->setLogName("m2");
  m2->lock();
  puts("unlocking m1 before destroying it...");
  m1->unlock();
  delete m1;
  puts("\nNOT unlocking m2 before destroying it...");
  delete m2;

  puts("exiting with Mvria::exit(0)...");
  Mvria::exit(0);
}