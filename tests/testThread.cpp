#include "Mvria.h"

class TestThread : public MvrASyncTask
{
public:
  TestThread(int num, MvrMutex &mutex);
  ~TestThread(void) {}

  virtual void * runThread(void *arg);

  int myNum;
  MvrMutex &myMutex;
};

TestThread::TestThread(int number, MvrMutex &mutex) :
  myNum(num),
  myMutex(mutex)
{

}

void *TestThread::runThread(void *arg)
{
  unsigned int interval;

  puts("New thread running ...");
  while(myRunning)
  {
    // Yield the processor here so that the other threads get a chance
    // to get the lock. calling MvrThread::yield() will give sequential
    // running of the threads.
    //yield();

    // Sleeping of 1 microsecond randomizes the order of running of the
    // threads.
    MvrUtil::sleep(1);

    myMutex.lock();
    interval = rand() % 10000;
    printf("Thread %d: Locked. Going to sleep for %dms\n", myNum, interval);
    MvrUtil::sleep(interval);
    printf("Thread %d: Unlocking...\n", myNum);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  puts("Mvria init ...");
  fflush(stdout);

  Mvria::init(Mvria::SIGHANDLE_THREAD, false);

  MvrMutex mutex;
  mutex.setLogName("mutex");

  MvrMutex::setLockWarningTime(1);
  MvrMutex::setUnLockWarningTime(1);;

  TestThread thread1(1, mutex), thread2(2, mutex), thread3(3, mutex), thread4(4, mutex);

  thread1.setThreadName("thread1");
  thread2.setThreadName("thread2");
  thread3.setThreadName("thread3");
  thread4.setThreadName("thread4");
  puts("srand ...");
  srand(time(0));

  puts("creating 3 thread ...");
  thread1.create();
  thread2.create();
  thread3.create();

  printf("main    thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", MvrThread::getThisThreadName(), MvrThread::getThisOSThread(), (unsigned int*) MvrThread::getThisThread());
  printf("thread1 thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread1.getThreadName(), thread1.getOSThread(), (unsigned int) thread1.getThread());
  printf("thread2 thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread2.getThreadName(), thread2.getOSThread(), (unsigned int) thread2.getThread());
  printf("thread3 thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread3.getThreadName(), thread3.getOSThread(), (unsigned int) thread3.getThread());
  printf("thread4 (not created yet) thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread4.getThreadName(), thread4.getOSThread(), (unsigned int) thread4.getThread());

#ifndef MINGW
  if (MvrThread::getThisOSThread() == thread1.getThisOSThread() ||
      MvrThread::getThisOSThread() == thread2.getThisOSThread() ||
      MvrThread::getThisOSThread() == thread3.getThisOSThread() ||
      MvrThread::getThisOSThread() == thread4.getThisOSThread() ||
      thread1::getThisOSThread() == thread2.getThisOSThread() ||
      thread1::getThisOSThread() == thread3.getThisOSThread() ||
      thread1::getThisOSThread() == thread4.getThisOSThread() ||
      thread2::getThisOSThread() == thread1.getThisOSThread() ||)
#endif  // MINGW

  return 0;
}