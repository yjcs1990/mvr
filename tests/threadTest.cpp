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
  myNum(number),
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
  MvrMutex::setUnlockWarningTime(5);

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

  printf("main    thread name=\"%s\", OS handle=%lu, OS pointer=0x%p\n", MvrThread::getThisThreadName(), MvrThread::getThisOSThread(), const_cast<const long unsigned int*>(MvrThread::getThisThread()));
  printf("thread1 thread name=\"%s\", OS handle=%lu, OS pointer=0x%p\n", thread1.getThreadName(), thread1.getOSThread(),  const_cast<const long unsigned int*>(thread1.getThread()));
  printf("thread2 thread name=\"%s\", OS handle=%lu, OS pointer=0x%p\n", thread2.getThreadName(), thread2.getOSThread(),  const_cast<const long unsigned int*>(thread2.getThread()));
  printf("thread3 thread name=\"%s\", OS handle=%lu, OS pointer=0x%p\n", thread3.getThreadName(), thread3.getOSThread(),  const_cast<const long unsigned int*>(thread3.getThread()));
  printf("thread4 (not created yet) thread name=\"%s\", OS handle=%lu, OS pointer=0x%p\n", thread4.getThreadName(), thread4.getOSThread(),const_cast<const long unsigned int*>(thread4.getThread()));

#ifndef MINGW
  if(MvrThread::getThisOSThread() == thread1.getOSThread() ||
     MvrThread::getThisOSThread() == thread2.getOSThread() ||
     MvrThread::getThisOSThread() == thread3.getOSThread() ||
     MvrThread::getThisOSThread() == thread4.getOSThread() ||
     thread1.getOSThread() == thread2.getOSThread() ||
     thread1.getOSThread() == thread3.getOSThread() ||
     thread1.getOSThread() == thread4.getOSThread() ||
     thread2.getOSThread() == thread1.getOSThread() ||
     thread2.getOSThread() == thread3.getOSThread() ||
     thread2.getOSThread() == thread4.getOSThread() ||
     thread3.getOSThread() == thread1.getOSThread() ||
     thread3.getOSThread() == thread2.getOSThread() ||
     thread3.getOSThread() == thread4.getOSThread() ||
     thread4.getOSThread() == thread1.getOSThread() ||
     thread4.getOSThread() == thread2.getOSThread() ||
     thread4.getOSThread() == thread3.getOSThread() )
  {
    puts("error, some thread IDs are the same!");
    return 5;
  }
#endif

puts("run thread 4 in main thread");
  thread4.runInThisThread();

puts("exit");
  Mvria::exit(0);

  return(0);
}