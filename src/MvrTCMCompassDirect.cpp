#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrTCMCompassDirect.h"
#include "MvrDeviceConnection.h"
#include "MvrSerialConnection.h"
#include "mvriaUtil.h"

//#define DEBUG_MVRTCMCOMPASSDIRECT 1
//#define DEBUG_MVRTCMCOMPASSDIRECT_STATS 1

#if defined(DEBUG_MVRTCMCOMPASSDIRECT) || defined(DEBUG_MVRTCMCOMPASSDIRECT_STATS)
void MvrTCMCompassDirect_printTransUnprintable( const char *data, int size){  for(int i = 0; i < size; ++i)  {    if(data[i] < ' ' || data[i] > '~')    {      printf("[0x%X]", data[i] & 0xff);    }    else    {      putchar(data[i]);    }  }}
#endif

MVREXPORT MvrTCMCompassDirect::MvrTCMCompassDirect(MvrDeviceConnection *devCon) :
  myDeviceConnection(devCon), myCreatedOwnDeviceConnection(false),
  myNMEAParser("MvrTCMCompassDirect"),
  myHCHDMHandler(this, &MvrTCMCompassDirect::handleHCHDM)
{
  myNMEAParser.addHandler("HCHDM", &myHCHDMHandler);
}

MVREXPORT MvrTCMCompassDirect::MvrTCMCompassDirect(const char *serialPortName) :
  myCreatedOwnDeviceConnection(true),
  myNMEAParser("MvrTCMCompassDirect"),
  myHCHDMHandler(this, &MvrTCMCompassDirect::handleHCHDM)
{
  MvrSerialConnection *newSerialCon = new MvrSerialConnection();
  newSerialCon->setPort(serialPortName);
  newSerialCon->setBaud(9600);
  myDeviceConnection = newSerialCon;
  myNMEAParser.addHandler("HCHDM", &myHCHDMHandler);
}
  

MVREXPORT MvrTCMCompassDirect::~MvrTCMCompassDirect() {
  if(myCreatedOwnDeviceConnection && myDeviceConnection)
    delete myDeviceConnection;
}

bool MvrTCMCompassDirect::sendTCMCommand(const char *fmt, ...)
{
  if(!myDeviceConnection) return false;
  if(myDeviceConnection->getStatus() != MvrDeviceConnection::STATUS_OPEN) return false;
  va_list args;
  va_start(args, fmt);
  char buf[32];
  vsnprintf(buf, sizeof(buf)-1, fmt, args);
  buf[sizeof(buf)-1] = 0;
  return myDeviceConnection->write(buf, strlen(buf));
}

MVREXPORT bool MvrTCMCompassDirect::blockingConnect(unsigned long connectTimeout)
{
  MvrTime start;
  start.setToNow();
  if(!connect()) return false;
  MvrLog::log(MvrLog::Normal, "MvrTCMCompassDirect: Opened connection, waiting for initial data...");
  while((unsigned long)start.mSecSince() <= connectTimeout)
  {
    if(read(0) > 0)
      return true;
    MvrUtil::sleep(100);
  }
  MvrLog::log(MvrLog::Terse, "MvrTCMCompassDirect: Error: No response from compass after %dms.", connectTimeout);
  return false;
}


MVREXPORT bool MvrTCMCompassDirect::connect()
{
  if(!myDeviceConnection) return false;
  if(myDeviceConnection->getStatus() != MvrDeviceConnection::STATUS_OPEN)
  {
    if(!myDeviceConnection->openSimple()) return false;
  }

  if(!sendTCMCommand("h\r"))
    return false;


  // sp= is sampling/send rate, maximum of 8hz. sdo is output format (t for cm
  // protocol or n for nmea).
  if(!sendTCMCommand("h\rsp=8\rsn=m\rsdo=n\rgo\r"))  
    return false;

#ifdef DEBUG_MVRTCMCOMPASSDIRECT
  char buf[640];
  memset(buf, 0, 640);
  myDeviceConnection->read(buf, 640, 2000);
  printf("Compass responded to init commands with: %s\n", buf);
#endif

  return true;
}



MVREXPORT void MvrTCMCompassDirect::commandUserCalibration()
{
  sendTCMCommand("cc\rmpcal=e\rgo\r");
}

MVREXPORT void MvrTCMCompassDirect::commandStopCalibration()
{
  sendTCMCommand("h\rmpcal=d\rautocal=d\r");
}

MVREXPORT void MvrTCMCompassDirect::commandContinuousPackets()
{
  sendTCMCommand("go\r");
}

MVREXPORT void MvrTCMCompassDirect::commandOff()
{
  sendTCMCommand("h\r");
}

MVREXPORT void MvrTCMCompassDirect::commandOnePacket()
{
  sendTCMCommand("c?\r");
}

MVREXPORT void MvrTCMCompassDirect::commandAutoCalibration()
{
  sendTCMCommand("h\rcc\rautocal=e\r");
}

MVREXPORT int MvrTCMCompassDirect::read(unsigned int msWait)
{
  return myNMEAParser.parse(myDeviceConnection);
}

void MvrTCMCompassDirect::handleHCHDM(MvrNMEAParser::Message m)
{
  myHeading = MvrMath::fixAngle(atof((*m.message)[1].c_str()));
#ifdef DEBUG_MVRTCMCOMPASSDIRECT 
  printf("XXX MvrTCMCompassDirect: recieved HCHDM message with compass heading %f.\n", myHeading);
#endif
  myHaveHeading = true;
  incrementPacketCount();
  invokeHeadingDataCallbacks(myHeading);
}

