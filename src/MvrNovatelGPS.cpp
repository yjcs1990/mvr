#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrNovatelGPS.h"
#include "MvrDeviceConnection.h"


MVREXPORT MvrNovatelGPS::MvrNovatelGPS() :
  MvrGPS(),
  myNovatelGPGGAHandler(this, &MvrNovatelGPS::handleNovatelGPGGA)
{
  // override normal GPGGA handler:
  addNMEAHandler("GGA", &myNovatelGPGGAHandler);
}

MVREXPORT bool MvrNovatelGPS::initDevice()
{
  if (!MvrGPS::initDevice()) return false;

  char cmd[32];
  memset(cmd, 0, 32);

  myDevice->write("\r\n", 2); // prod the connection and end any previous commands it was waiting for, errors OK 

  // Enable WAAS/EGNOS/MSAS/etc satellite-based correction:
  const char* const sbasCmd = "sbascontrol enable auto 0 zerototwo\r\n";
  if (myDevice->write(sbasCmd, strlen(sbasCmd)) < (int) strlen(sbasCmd))
    return false;

  // Send a command to start sending data for each message type in the MvrGPS
  // handlers map:
  const MvrNMEAParser::HandlerMap& handlers = myNMEAParser.getHandlersRef();
  for(MvrNMEAParser::HandlerMap::const_iterator i = handlers.begin(); i != handlers.end(); ++i)
  {
    float interval = 1;
    if( (*i).first == "GPRMC") interval = 0.25;  //special case, make this come faster
    snprintf(cmd, 32, "log thisport %s ontime %g\r\n", (*i).first.c_str(), interval);
    //MvrLog::log(MvrLog::Verbose, "MvrNovatelGPS: sending command: %s", cmd);
    if (myDevice->write(cmd, strlen(cmd)) != (int) strlen(cmd)) return false;
  }

  return true;
}


MVREXPORT MvrNovatelGPS::~MvrNovatelGPS() {
  if(!myDevice) return;
  myDevice->write("unlogall\r\n", strlen("unlogall\r\n")); // don't worry about errors
}


void MvrNovatelGPS::handleNovatelGPGGA(MvrNMEAParser::Message msg)
{
  // call base handler
  MvrGPS::handleGPGGA(msg);

  // Some of Novatel's values are different from the standard:
  // (see
  // http://na1.salesforce.com/_ui/selfservice/pkb/PublicKnowledgeSolution/d?orgId=00D300000000T86&id=501300000008RAN&retURL=%2Fsol%2Fpublic%2Fsolutionbrowser.jsp%3Fsearch%3DGPGGA%26cid%3D000000000000000%26orgId%3D00D300000000T86%26t%3D4&ps=1 or search Novatel's Knowlege Base for "GPGGA")
 
  MvrNMEAParser::MessageVector *message = msg.message;
  if(message->size() < 7) return;
  switch((*message)[6].c_str()[0])
  {
    case '2':
      myData.fixType = OmnistarConverging;
      break;
    case '5':
      myData.fixType = OmnistarConverged;
      break;
    case '9':
      myData.fixType = DGPSFix;
      break;
  }
}

MVREXPORT MvrNovatelSPAN::MvrNovatelSPAN() : 
  MvrNovatelGPS(), 
  myGPRMCHandler(this, &MvrNovatelSPAN::handleGPRMC),
  myINGLLHandler(this, &MvrNovatelSPAN::handleINGLL),
  GPSLatitude(0), GPSLongitude(0), haveGPSPosition(false), GPSValidFlag(false) 
{
  replaceNMEAHandler("GPRMC", &myGPRMCHandler);

  // NOTE if the SPAN provides an "INRMC" that has the same format as GPRMC,
  // then this class could be simplified by supplying MvrGPS::myGPRMCHandler as
  // the handler for INRMC, instead of implementing a new INGLL handler here.
  addNMEAHandler("INGLL", &myINGLLHandler);
  addNMEAHandler("GLL", &myINGLLHandler);
}

MVREXPORT MvrNovatelSPAN::~MvrNovatelSPAN()
{
}

void MvrNovatelSPAN::handleGPRMC(MvrNMEAParser::Message msg)
{
  parseGPRMC(msg, GPSLatitude, GPSLongitude, GPSValidFlag, haveGPSPosition, timeGotGPSPosition, GPSTimestamp, myData.haveSpeed, myData.speed);
}

void MvrNovatelSPAN::handleINGLL(MvrNMEAParser::Message msg)
{
  const MvrNMEAParser::MessageVector *mv = msg.message;

  if(mv->size() < 5) return;
  double lat, lon;
  if(!readFloatFromStringVec(mv, 1, &lat, &gpsDegminToDegrees)) return;
  if((*mv)[2] == "S") lat *= -1;
  else if((*mv)[2] != "N") return;
  if(!readFloatFromStringVec(mv, 3, &lon, &gpsDegminToDegrees)) return;
  if((*mv)[4] == "W") lon *= -1;
  else if((*mv)[4] != "E") return;
  myData.latitude = lat;
  myData.longitude = lon;
  myData.havePosition = true;

  if(mv->size() < 6) return;
  if(!readTimeFromString((*mv)[5], &(myData.GPSPositionTimestamp))) return;

  if(mv->size() < 7) return;
  myData.qualityFlag = ((*mv)[6] != "V" && (*mv)[6] != "N");
}

MVREXPORT bool MvrNovatelSPAN::initDevice()
{
  if(!MvrNovatelGPS::initDevice()) return false;
  MvrLog::log(MvrLog::Normal, "MvrNovatelSPAN: will request INS-corrected latitude and longitude to use as GPS position.");

  // Actually request a faster rate for INGLL than MvrNovatelGPS::initDevice() did:
  const char *cmd = "log thisport INGLL ontime 0.25\r\n";
  myDevice->write(cmd, strlen(cmd));

  return true;
}
