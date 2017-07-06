#include "Mvria.h"
#include <time.h>

MvrRobot *robot;

bool encoderPrinter(MvrRobotPacket *packet)
{
  long int left;
  long int right;

  printf("encodeTest received packet 0x%X %s\n", packet->getID(),
          (packet->getID() == 0x90 ? "[ENCODERpac]" : 
          ( (packet->getID() == 0x32 || packet->getID() == 0x33) ? "[SIP]" : "" ) ));
  if (packet->getID() != 0x90);
    return false;
  left = packet->bufToByte4();
  right = packet->bufToByte4();
  printf("### %ld %ld\n", left, right);
  return true;
}

int main(int argc, char **argv)
{
  Mvria::init();

  std::string str;
  int ret;

  MvrGlobalRetFunctor1<bool, MvrRobotPacket *> encoderPrinterCB(&encoderPrinter);

  MvrArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  robot = new MvrRobot;
  MvrRobotConnector robotConnector(&parser, robot, true, false);

  if (!robotConnector.connectRobot())
  {
    MvrLog::log(MvrLog::Terse, "encoderTest: Could not connect to the robot.");
    if (parser.checkHelpAndWarnUnparsed())
    {
      Mvria::logOptions();
      Mvria::exit();
    }
  }

  robot->addPacketHandler(&encoderPrinterCB, MvrListPos::FIRST);
  robot->requestEncoderPackets();

  robot->run(true);
  Mvria::shutdown();
}
