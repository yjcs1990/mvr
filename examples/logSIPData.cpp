
#include "Mvria.h"
#include <string>

/*
 * This is useful as a diagnostic tool, plus it shows all the many accessor
 * methods of MvrRobot for robot state. It makes the robot wander, using sonar
 * to avoid obstacles, and prints out various pieces of robot state information
 * each second. Refer to the ARIA MvrRobot documentation and to your robot manual
 * (section on standard ARCOS SIP packet contents) for details on the data.
 */



/* function to display a byte as a string of 8 '1' and '0' characters. */
std::string byte_as_bitstring(char byte) 
{
  char tmp[9];
  int bit; 
  int ch;
  for(bit = 7, ch = 0; bit >= 0; bit--,ch++)
    tmp[ch] = ((byte>>bit)&1) ? '1' : '0';
  tmp[8] = 0;
  return std::string(tmp);
}

/* function to display a 2-byte int as a string of 16 '1' and '0' characters. */
std::string int_as_bitstring(MvrTypes::Byte2 n) 
{
  char tmp[17];
  int bit;
  int ch;
  for(bit = 15, ch = 0; bit >= 0; bit--, ch++)
    tmp[ch] = ((n>>bit)&1) ? '1' : '0';
  tmp[16] = 0;
  return std::string(tmp);
}

/* Some events might only be detectable in one robot cycle, not over the
 * 1-second period that the main thread sleeps. This cycle callback will detect
 * those and save them in some global variables. */
bool wasLeftMotorStalled = false;
bool wasRightMotorStalled = false;
MvrTypes::UByte2 cumulativeStallVal = 0;
MvrTypes::UByte2 cumulativeRobotFlags = 0;
bool wasLeftIRTriggered = false;
bool wasRightIRTriggered = false;
bool wasEStopTriggered = false;

bool cycleCallback(MvrRobot* robot)
{
  cumulativeStallVal |= robot->getStallValue();
  wasLeftMotorStalled = wasLeftMotorStalled || robot->isLeftMotorStalled();
  wasRightMotorStalled = wasRightMotorStalled || robot->isRightMotorStalled();
  wasEStopTriggered = wasEStopTriggered || robot->getEstop();
  wasLeftIRTriggered = wasLeftIRTriggered || (robot->hasTableSensingIR() && robot->isLeftTableSensingIRTriggered());
  wasRightIRTriggered = wasRightIRTriggered || (robot->hasTableSensingIR() && robot->isRightTableSensingIRTriggered());
  return true;
}


/* main function */
int main(int argc, char **argv)
{
  // robot and devices
  MvrRobot robot;
  MvrSonarDevice sonar;
  MvrBumpers bumpers;
  MvrIRs ir;

  // initialize aria and aria's logging destination and level
  Mvria::init();
  MvrLog::init(MvrLog::StdErr, MvrLog::Normal);

  // connector
  MvrArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  MvrRobotConnector connector(&parser, &robot);
  if (!Mvria::parseArgs())
  {
    Mvria::logOptions();
    Mvria::exit(1);
  }

  printf("This program will continously print some data and events.\nPress Ctrl-C to exit.\n");
  
  // add the range devices to the robot
  robot.addRangeDevice(&sonar);
  robot.addRangeDevice(&bumpers);
  robot.addRangeDevice(&ir);
  
  // try to connect, if we fail exit
  if (!connector.connectRobot())
  {
    printf("Could not connect to robot... exiting\n");
    Mvria::exit(1);
    return 1;
  }

  // turn on the motors, sonar, turn off amigobot sound effects (for old h8-model amigobots)
  robot.enableMotors();
  robot.comInt(MvrCommands::SOUNDTOG, 0);
  robot.comInt(MvrCommands::SONAR, 1);

  // Cycle callback to check for events
  robot.addUserTask("checkevents", 1, new MvrGlobalRetFunctor1<bool, MvrRobot*>(&cycleCallback, &robot));

  // start the robot running, true means that if we lose robot connection the 
  // MvrRobot runloop stops
  robot.runAsync(true);
  
  // Print data header
#define HEADFORMAT "%-24s %-5s %-16s %-5s %-6s %-6s %-16s %-8s %-8s %-8s %-8s %-8s %-8s %-10s %-10s %-5s %-5s %-8s %-8s %-12s %-12s %s"
#define DATAFORMAT "%-24s % 03.02f %-16s %-5s %-6s %-6s %-16s %-8d %-8d %-8g %-8g %-8s %-8s %-10lu %-10lu %-5s %-5s %-8s %-8s %-12s %-12s" 
// note DATAFORMAT doesn't include bumps details on end, they are printed separately
  printf("\n" HEADFORMAT "\n\n",
        "Time",
        "Volts",
        "Flags",
        "EStop",
        "StallL",
        "StallR",
        "StallVal",
        "#SIP/sec",
        "#Son/sec",
        "Vel L",
        "Vel R",
        "DigIns",
        "DigOuts",
        "Enc L",
        "Enc R",
        "IR L",
        "IR R",
        "Faults",
        "Flags3",
        "Mot.Enable",
        "Son.Enable",
        "Cur Bumps, (Last Bump Pose)"
    );
//  robot.comInt(MvrCommands::CONFIG,1);
  // Request that we will want encoder data
  robot.requestEncoderPackets();

  // Print data every second
  char timestamp[24];
  while(robot.isRunning()) {
    robot.lock();
    time_t t = time(NULL);
    strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&t));
    printf( DATAFORMAT,
        timestamp,
        robot.getRealBatteryVoltage(),
        int_as_bitstring(cumulativeRobotFlags).c_str(),
        (wasEStopTriggered ? "YES" : "no "),
        (wasLeftMotorStalled?"YES":"no "), 
        (wasRightMotorStalled?"YES":"no "),
        int_as_bitstring(cumulativeStallVal).c_str(),
        robot.getMotorPacCount(),
        robot.getSonarPacCount(),
        robot.getLeftVel(), 
        robot.getRightVel(),
        byte_as_bitstring(robot.getDigIn()).c_str(),
        byte_as_bitstring(robot.getDigOut()).c_str(),
        robot.getLeftEncoder(),
        robot.getRightEncoder(),
        wasLeftIRTriggered?"YES": "no ",
        wasRightIRTriggered?"YES":"no ",
        robot.hasFaultFlags()?int_as_bitstring(robot.getFaultFlags()).c_str():"none",
        robot.hasFlags3()?int_as_bitstring(robot.getFlags3()).c_str():"none",
        robot.areMotorsEnabled()?"yes":"NO ",
        robot.areSonarsEnabled()?"yes":"no "
      );

    // list indices of bumpers flaged in stallval
    // skip the last bit which is a motor stall flag
    MvrTypes::UByte2 bumpmask = MvrUtil::BIT15;
    int bump = 0;
    for(int bit = 16; bit > 0; bit--) 
    {
      if(bit == 9) // this is also a motor stall bit
      {
        bumpmask = bumpmask >> 1; 
        bit--;
        continue;
      }
      //printf("\n\tComparing stallval=%s to bumpmask=%s... ", int_as_bitstring(stallval).c_str(), int_as_bitstring(bumpmask).c_str());
      if(cumulativeStallVal & bumpmask)
        printf("%d ", bump);
      bumpmask = bumpmask >> 1;
      bump++;
    }

    // print pose of last bump sensor reading
    const std::list<MvrPoseWithTime*>* bumpsensed = bumpers.getCurrentBuffer();
    if(bumpsensed)
    {
      //printf("%d readings. ", bumpsensed->size());
      if(bumpsensed->size() > 0 && bumpsensed->front()) {
        printf("(%.0f,%.0f)", bumpsensed->front()->getX(), bumpsensed->front()->getY());
      }
    }
    puts("");


    // clear events to accumulate for the next second
    cumulativeRobotFlags = cumulativeStallVal = 0;
    wasLeftMotorStalled = wasRightMotorStalled = wasLeftIRTriggered = wasRightIRTriggered = wasEStopTriggered = false;

    robot.unlock();
    MvrUtil::sleep(1000);
  }
  
  // robot cycle stopped, probably because of lost robot connection
  Mvria::shutdown();
  return 0;
}
