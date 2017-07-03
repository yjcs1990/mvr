#include "Mvria.h"

MvrTime lastLoopTime;
int loopTime;

void loopTester(void)
{
  printf("%6ld ms since last loop. ms longer than desired: %6ld.\n", 
         lastLoopTime.mSecSince(), lastLoopTime.mSecSince() - loopTime);
  lastLoopTime.setToNow();
}

int main(void)
{
  MvrRobot robot;
  MvrTcpConnection conn;
  MvrGlobalFunctor loopTesterCB(&loopTester);

  lastLoopTime.setToNow();
  Mvria::init();

  conn.setPort();
  robot.setDeviceConnection(&conn);
  robot.setCycleChained(true);

  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot");
    Mvria::shutdown();
    exit(0);
  }

  robot.comInt(MvrCommands::ENCODER, 1);
  robot.setCycleTime(100);
  loopTime = robot.getCycleTime();
  robot.addUserTask("loopTester", 0, &loopTesterCB);
  robot.run(false);

  Mvria::shutdown();
}