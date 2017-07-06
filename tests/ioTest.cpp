#include "Mvria.h"
/* 
 * This tests the response time for IO packet requests sent to the robot.  It can
 * also test response for velocity commands if the section is uncommented.
 *
 * This can be used to see what the delay is in the time a command is sent to the
 * time the command takes effect.
 */

MvrTime requestTime;
MvrTime velTime;
int n = 0;
int vel = 0 ;
int ABOUT_RIGHT = 5;

void userTask(MvrRobot *robot)
{
  if (robot->getIOPacketTime().mSecSince() > requestTime.mSecSince())
    return;
  else
  {
    MvrLog::log(MvrLog::Terse, "last packet time: %d", requestTime.mSecSince() - robot.getIOPacketTime());
    fflush(stdout);
    robot->comInt(MvrCommands::IOREQUEST, 1);
    requestTime.setToNow();
  }
  /*  Uncomment the next section to test response in velocity commands. */
  // if (abs((int)(robot->getVel() - vel)) <= ABOUT_RIGHT &&
      // velTime.secSince() > 5)
  // {
    // if (vel == 0)
      // vel = 200;
    // else if (vel == 200)
      // vel = 400;
    // else if (vel == 400)
      // vel = 0;
    // robot->setVel(vel);
    // velTime.setToNow();
  // }
// 
  // MvrLog::log(MvrLog::Terse, "vel requested: %d\trobot vel: %3.2f\tvel time: %d", vel, robot->getVel(), velTime.mSecSince());
  MvrLog::log(MvrLog::Terse, "Mvria cycle time:  %d", robot->getCycleTime());
  fflush(stdout);
}

int main(int argc, char **argv)
{
  // set up our simpleConnector
  MvrSimpleConnector simpleConnector(&argv, argc);
  //robot
  MvrRobot robot;
  // A key handler so we can do our key handling
  MvrKeyHandler keyHandler;

  MvrLog::init(MvrLog::StdOut, MvrLog::Verbose);

  // if there are more arguments left then it means we didn't understand an option
  if (!simpleConnector.parseMvrgs() || argc > 1)
  {
    simpleConnector.logOptions();
    keyHandler.restore();
    exit(1);
  }

  // mandatory init
  Mvria::init();
  MvrLog::init(MvrLog::StdOut, MvrLog::Verbose, NULL, true);

  // let the global mvria stuff know about it
  Mvria::setKeyHandler(&keyHandler);
  // toss it on the robot
  robot.attachKeyHandler(&keyHandler);

  // set up the robot for connecting
  if (!simpleConnector.connectRobot(&robot))
  {
    printf("Counld not connect to the robot ... exiting\n");
    Mvria::shutdown();
    keyHandler.restore();
    return 1;
  }

  // turn on the motors for the velocity response test
  robot.comInt(MvrCommands::ENABLE, 1);
  velTime.setToNow();

  // turn off the sonar
  robot.comInt(MvrCommands::SONAR, 0);

  MvrGlobalFunctor1<MvrRobot *> userTaskCB(&userTask, &robot);
  robot.addUserTask("iotest", 100, &userTaskCB);

  robot.comInt(MvrCommands::IOREQUEST, 1);
  requestTime.setToNow();

  //start the robot running, true so that if we lose connection the run stops
  robot.run(true);
  
  // now exit
  Mvria::shutdown();
  return 0;
}