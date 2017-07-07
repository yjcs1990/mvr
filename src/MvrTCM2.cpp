#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "mvriaInternal.h"
#include "mvriaUtil.h"
#include "MvrTCM2.h"
#include "MvrTCMCompassDirect.h"
#include "MvrTCMCompassRobot.h"

MVREXPORT MvrTCM2::MvrTCM2() :
  myHeading(0.0),
  myPitch(0.0),
  myRoll(0.0),
  myXMag(0.0),
  myYMag(0.0),
  myZMag(0.0),
  myTemperature(0.0),
  myError(0),
  myCalibrationH(0.0),
  myCalibrationV(0.0),
  myCalibrationM(0.0),
  myHaveHeading(false), 
  myHavePitch(false),
  myHaveRoll(false),
  myHaveXMag(false),
  myHaveYMag(false),
  myHaveZMag(false),
  myHaveTemperature(false),
  myHaveCalibrationH(false),
  myHaveCalibrationV(false),
  myHaveCalibrationM(false),
  myTimeLastPacket(0),
  myPacCurrentCount(0),
  myPacCount(0)
{
}

MVREXPORT bool MvrTCM2::connect() { return true; }
MVREXPORT bool MvrTCM2::blockingConnect(unsigned long) { return true; }

MVREXPORT MvrCompassConnector::MvrCompassConnector(MvrArgumentParser *argParser) :
  myArgParser(argParser),
  myParseArgsCallback(this, &MvrCompassConnector::parseArgs),
  myLogArgsCallback(this, &MvrCompassConnector::logOptions),
  myDeviceType(None),
  mySerialPort(MVRTCM2_DEFAULT_SERIAL_PORT),
  mySerialTCMReadFunctor(NULL),
  myRobot(NULL)
{
  myParseArgsCallback.setName("MvrCompassConnector");
  myLogArgsCallback.setName("MvrCompassConnector");
  Mvria::addParseArgsCB(&myParseArgsCallback);
  Mvria::addLogOptionsCB(&myLogArgsCallback);
}

MVREXPORT MvrCompassConnector::~MvrCompassConnector() {
  if(mySerialTCMReadFunctor && myRobot) 
  {
    myRobot->lock();
    myRobot->remSensorInterpTask(mySerialTCMReadFunctor);
    delete mySerialTCMReadFunctor;
    myRobot->unlock();
  }
}

bool MvrCompassConnector::parseArgs()
{
  if(!myArgParser) return false;
  if(!myArgParser->checkParameterArgumentString("-compassPort", &mySerialPort)) return false;
  char *deviceType = myArgParser->checkParameterArgument("-compassType");
  if(deviceType)
  {
    if(strcasecmp(deviceType, "robot") == 0)
      myDeviceType = Robot;
    else if(strcasecmp(deviceType, "serialtcm") == 0)
      myDeviceType = SerialTCM;
    else if(strcasecmp(deviceType, "tcm") == 0)
      myDeviceType = SerialTCM;
    else
    {
      MvrLog::log(MvrLog::Terse, "MvrCompassConnector: Error: unrecognized -compassType option: %s. Valid values are robot and serialTCM.", deviceType);
      return false;
    }
  }
  return true;
}

void MvrCompassConnector::logOptions()
{
  MvrLog::log(MvrLog::Terse, "Compass options:");
  MvrLog::log(MvrLog::Terse, "-compassType <robot|serialTCM>\tSelect compass device type (default: robot)");
  MvrLog::log(MvrLog::Terse, "-compassPort <port>\tSerial port for \"serialTCM\" type compass. (default: %s)", MVRTCM2_DEFAULT_SERIAL_PORT);
}

MVREXPORT MvrTCM2 *MvrCompassConnector::create(MvrRobot *robot)
{
  if(myDeviceType == None)
  {
    if(robot && robot->getRobotParams())
    {
      const char *type = robot->getRobotParams()->getCompassType();
      if(type == NULL || strlen(type) == 0 || strcmp(type, "robot") == 0)
      {
        myDeviceType = Robot;
      }
      else if(strcmp(type, "serialTCM") == 0)
      {
        myDeviceType = SerialTCM;
        const char *port = robot->getRobotParams()->getCompassPort();
        if(port == NULL || strlen(port) == 0)
          mySerialPort = MVRTCM2_DEFAULT_SERIAL_PORT;
        else
          mySerialPort = port;
      }
      else
      {
        MvrLog::log(MvrLog::Terse, "MvrCompassConnector: Error: invalid compass type \"%s\" in robot parameters.", type);
        return NULL;
      }
    }
    else
    {
      myDeviceType = Robot;
    }
  }

  switch(myDeviceType)
  {
    case Robot:
      MvrLog::log(MvrLog::Verbose, "MvrCompassConnector: Using robot compass");
      return new MvrTCMCompassRobot(robot);
    case SerialTCM:
      {
        MvrLog::log(MvrLog::Verbose, "MvrCompassConnector: Using TCM2 compass on serial port %s", mySerialPort);
        MvrTCMCompassDirect *newDirectTCM = new MvrTCMCompassDirect(mySerialPort);
        mySerialTCMReadFunctor = new MvrRetFunctor1C<int, MvrTCMCompassDirect, unsigned int>(newDirectTCM, &MvrTCMCompassDirect::read, 1);
        robot->lock();
        robot->addSensorInterpTask("MvrTCMCompassDirect read", 200, mySerialTCMReadFunctor);
        myRobot = robot;
        robot->unlock();
        return newDirectTCM;
      }
    case None:
    default:
      // break out of switch and print warning there
      break;
  }
  MvrLog::log(MvrLog::Terse, "MvrCompassConnector: Error: No compass type selected.");
  return NULL;
}

MVREXPORT bool MvrCompassConnector::connect(MvrTCM2 *compass) const
{
  return compass->blockingConnect();
}

