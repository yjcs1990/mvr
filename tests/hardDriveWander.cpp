
#include "Mvria.h"

/*
  This program will just have the robot wander around, it uses some avoidance 
  routines, then just has a constant velocity.
*/

int main(int argc, char **argv)
{
  // robot
  MvrRobot robot;
  // the laser
  MvrSick sick;


  // sonar, must be added to the robot
  //MvrSonarDevice sonar;

  // the actions we'll use to wander
  // recover from stalls
  //MvrActionStallRecover recover;
  // react to bumpers
  //MvrActionBumpers bumpers;
  // limiter for close obstacles
  MvrActionLimiterForwards limiter("speed limiter near", 1600, 0, 0, 1.3);
  // limiter for far away obstacles
  //MvrActionLimiterForwards limiterFar("speed limiter near", 300, 1000, 450, 1.1);
  //MvrActionLimiterForwards limiterFar("speed limiter far", 300, 1100, 600, 1.1);
  // limiter for the table sensors
  //MvrActionLimiterTableSensor tableLimiter;
  // actually move the robot
  MvrActionConstantVelocity constantVelocity("Constant Velocity", 1500);
  // turn the orbot if its slowed down
  MvrActionTurn turn;

  // mandatory init
  Mvria::init();

  // Parse all our args
  MvrSimpleConnector connector(&argc, argv);
  connector.parseArgs();
  
  if (argc > 1)
  {
    connector.logOptions();
    exit(1);
  }
  
  // add the sonar to the robot
  //robot.addRangeDevice(&sonar);
  // add the laser to the robot
  robot.addRangeDevice(&sick);

  // try to connect, if we fail exit
  if (!connector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Mvria::shutdown();
    return 1;
  }

  robot.comInt(MvrCommands::SONAR, 0);

  // turn on the motors, turn off amigobot sounds
  //robot.comInt(MvrCommands::SONAR, 0);
  robot.comInt(MvrCommands::SOUNDTOG, 0);

  // add the actions
  //robot.addAction(&recover, 100);
  //robot.addAction(&bumpers, 75);
  robot.addAction(&limiter, 49);
  //robot.addAction(&limiter, 48);
  //robot.addAction(&tableLimiter, 50);
  robot.addAction(&turn, 30);
  robot.addAction(&constantVelocity, 20);

  limiter.activate();
  turn.activate();
  constantVelocity.activate();

  robot.clearDirectMotion();
  //robot.setStateReflectionRefreshTime(50);
  robot.setRotVelMax(50);
  robot.setTransAccel(1500);
  robot.setTransDecel(100);

  // start the robot running, true so that if we lose connection the run stops
  robot.runAsync(true);

  connector.setupLaser(&sick);

  // now that we're connected to the robot, connect to the laser
  sick.runAsync();

  if (!sick.blockingConnect())
  {
    printf("Could not connect to SICK laser... exiting\n");
    Mvria::shutdown();
    return 1;
  }
  
  sick.lockDevice();
  sick.setMinRange(250);
  sick.unlockDevice();
  robot.lock();
  robot.comInt(MvrCommands::ENABLE, 1);
  robot.unlock();

  robot.waitForRunExit();
  // now exit
  Mvria::shutdown();
  return 0;
}
