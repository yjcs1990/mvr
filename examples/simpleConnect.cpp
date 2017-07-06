#include "Mvria.h"

/** @example simpleConnect.cpp example showing how to connect to the robot with MvrRobotConnector
 *
 * One of the simplest MVRIA programs possible:
 * Connects with MvrRobotConnector, waits 3 seconds doing
 * nothing, then exits.
 *
 * This program will work either with the MobileSim simulator or on a real
 * robot's onboard computer.  (Or use -remoteHost to connect to a wireless
 * ethernet-serial bridge.)
 */

int main(int argc, char **argv)
{
  Mvria::init();
  MvrLog::log(MvrLog::Terse, "Mvria::getDirectory(): %s",Mvria::getDirectory());
  
  MvrArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  MvrRobot robot;

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  MvrRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    MvrLog::log(MvrLog::Terse, "simpleConnect: Could not connect to the robot.");
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

  MvrLog::log(MvrLog::Normal, "simpleConnect: Connected to robot.");

  robot.enableMotors();

  // Start the robot processing cycle running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  // Print out some data from the SIP.  We must "lock" the MvrRobot object
  // before calling its methods, and "unlock" when done, to prevent conflicts
  // with the background thread started by the call to robot.runAsync() above.
  // See the section on threading in the manual for more about this.
  robot.lock();
  bool soc = robot.hasStateOfCharge();
  float battv = 0.0;
  if(soc)
    battv = robot.getStateOfCharge();
  else
    battv = robot.getBatteryVoltage();
  MvrLog::log(MvrLog::Normal, "simpleConnect: Pose=(%.2f,%.2f,%.2f), Trans.  Vel=%.2f, Battery=%.2f%c",
    robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), battv, soc?'%':'V');
  robot.unlock();

  // Sleep for 3 seconds.
  MvrLog::log(MvrLog::Normal, "simpleConnect: Sleeping for 3 seconds...");
  MvrUtil::sleep(3000);

  
  MvrLog::log(MvrLog::Normal, "simpleConnect: Ending robot thread...");
  robot.stopRunning();

  // wait for the thread to stop
  robot.waitForRunExit();

  // exit
  MvrLog::log(MvrLog::Normal, "simpleConnect: Exiting.");
  Mvria::exit(0);
  return 0;
}
