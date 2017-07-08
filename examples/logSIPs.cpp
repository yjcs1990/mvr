#include "Mvria.h"

int main(int argc, char **argv)
{
  Mvria::init();
  MvrArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  MvrRobot robot;

  robot.setLogSIPContents(true);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  MvrRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    MvrLog::log(MvrLog::Terse, "Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
      // -help not given
      Mvria::logOptions();
      Mvria::exit(1);
    }
  }

  if (!Mvria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Mvria::logOptions();
    Mvria::exit(1);
  }

  MvrActionGroupRatioDrive driveAct(&robot);
  driveAct.activate();

  robot.enableMotors();

  robot.runAsync(true);

  while(true)
  {
    MvrUtil::sleep(100);
    robot.lock();
    int f = robot.getFlags();
    printf("Charging power good: %d, E-Stall Engaged: %d, Motors Enabled: %d, Stop Button Pressed: %d, Pioner Sonar Mvrray 1: %d\n",  f&MvrUtil::BIT10, f&MvrUtil::BIT6, f&MvrUtil::BIT0, f&MvrUtil::BIT5, f&MvrUtil::BIT1);
    robot.unlock();
  }
  robot.waitForRunExit();

  Mvria::exit(0);
}

