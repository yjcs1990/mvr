
#include "Mvria.h"

/* Tests the MOVE command */

int main(int argc, char **argv)
{
  MvrRobot robot;
  Mvria::init();

  robot.setLogSIPContents(true);

  MvrSimpleConnector connector(&argc, argv);
  if (!connector.parseArgs() || argc > 1)
  {
    connector.logOptions();
    return 1;
  }

  if (!connector.connectRobot(&robot))
  {
    MvrLog::log(MvrLog::Terse, "moveCommandTest: Could not connect to robot... exiting.");
    return 2;
  }

  robot.runAsync(true);

  robot.lock();
  robot.enableMotors();
  robot.unlock();

  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sending command to MOVE 500 mm...");
  robot.lock();
  robot.move(500);
  robot.unlock();
  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sleeping for 5 seconds...\n");
  MvrUtil::sleep(5000);

  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sending command to MOVE 2000 mm...");
  robot.lock();
  robot.move(2000);
  robot.unlock();
  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sleeping for 10 seconds...\n");
  MvrUtil::sleep(10000);

  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sending command to MOVE 5000 mm...");
  robot.lock();
  robot.move(5000);
  robot.unlock();
  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sleeping for 15 seconds...\n");
  MvrUtil::sleep(15000);

  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sending command to MOVE 10000 mm... The robot may ignore this command, since the argument is so large.");
  robot.lock();
  robot.move(10000);
  robot.unlock();
  MvrLog::log(MvrLog::Normal, "moveCommandTest: Sleeping for 30 seconds...\n");
  MvrUtil::sleep(30000);

  MvrLog::log(MvrLog::Normal, "moveCommandTest: Ending robot thread and exiting.");
  robot.stopRunning();
  return 0;
}
