#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrDPPTU.h"
#include "MvrCommands.h"
#include "MvrBasePacket.h"
#include "mvriaInternal.h"
#include "MvrDeviceConnection.h"

#define DO_DEBUG(code) {code;}

//#define DEBUG_CMD DO_DEBUG
//#define DEBUG_TIME DO_DEBUG
//#define DEBUG_POS DO_DEBUG

#ifndef DEBUG_CMD
#define DEBUG_CMD(code) {}
#endif

#ifndef DEBUG_TIME
#define DEBUG_TIME(code) {}
#endif

#ifndef DEBUG_POS
#define DEBUG_POS(code) {}
#endif

MVREXPORT MvrDPPTUPacket::MvrDPPTUPacket(MvrTypes::UByte2 bufferSize) :
  MvrBasePacket(bufferSize, 0)
{
}

MVREXPORT MvrDPPTUPacket::~MvrDPPTUPacket()
{

}

MVREXPORT void MvrDPPTUPacket::byte2ToBuf(int val)
{
  //MvrLog::log(MvrLog::Normal, "Putting %d in an DPPTU packet...", val);
  int i;
  char buf[8];
  if (myLength + 4 > myMaxLength)
  {
    MvrLog::log(MvrLog::Terse, "MvrDPPTUPacket::byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }

  if(val > 9999999 || val < -999999) 
	  MvrLog::log(MvrLog::Terse, "MvrDPPTUPacket::byte2ToBuf: Warning: truncating value %d to 7 digits!", val);

  snprintf(buf, 8, "%d", val);

  for (i=0;i<(int)strlen(buf);i++)
  {
      myBuf[myLength] = buf[i];
      ++myLength;
  }
}

MVREXPORT void MvrDPPTUPacket::finalizePacket(void)
{
    MvrDPPTUPacket::uByteToBuf(MvrDPPTUCommands::DELIM);
}

/** @a deviceType If a type other than PANTILT_DEFAULT is given, then manually
 * selects different parameters for different DPPTU models. It is recommended to
 * use PANTILT_DEFAULT (unless your PTU is not responding to resolution
 * queries for some reason)
        
 * @a deviceIndex if you have more than one PTU on a Seekur robot, this
 * specifies one device to power on at startup and power off on exit. If -1
 * (default value), then all possible PTU power ports are powered on.
 */
MVREXPORT MvrDPPTU::MvrDPPTU(MvrRobot *robot, DeviceType deviceType, int deviceIndex) :
  MvrPTZ(robot),
  myCanGetRealPanTilt(true),
  myInit(false),
  myQueryCB(this, &MvrDPPTU::query),
  myGotPanRes(false),
  myGotTiltRes(false)
{
  myRobot = robot;
  myDeviceType = deviceType;

  switch(myDeviceType) {
    case PANTILT_PTUD47:
	  myPanConvert = 0.0514;
      myTiltConvert = 0.0129;
	  MvrPTZ::setLimits(158, -158, 30, -46);
	  /*
      myMaxPan = 158;
      myMinPan = -158;
      myMaxTilt = 30;
      myMinTilt = -46;
	  */
      myMaxPanSlew = 149;
      myMinPanSlew = 2;
      myMaxTiltSlew = 149;
      myMinTiltSlew = 2;
      myMaxPanAccel = 102;
      myMinPanAccel = 2;
      myMaxTiltAccel = 102;
      myMinTiltAccel = 2;
      myPanSlew = 40;
      myTiltSlew = 40;
      break;
    case PANTILT_PTUD46:
    case PANTILT_DEFAULT:  
	  // if DEFAULT, then in init() we will query the PTU to get the real conversion factors and limits (but start out assuming same as D46)
	  myPanConvert = 0.0514;
      myTiltConvert = 0.0514;
	  MvrPTZ::setLimits(158, -158, 30, -46);
	  /*
      myMaxPan = 158;
      myMinPan = -158;
      myMaxTilt = 30;
      myMinTilt = -46;
	  */
      myMaxPanSlew = 149;
      myMinPanSlew = 2;
      myMaxTiltSlew = 149;
      myMinTiltSlew = 2;
      myMaxPanAccel = 102;
      myMinPanAccel = 2;
      myMaxTiltAccel = 102;
      myMinTiltAccel = 2;
      myPanSlew = 40; //Default to 1000 positions/sec
      myTiltSlew = 40; //Defaults to 1000 positions/sec
    default:
      break;
  }

  if(deviceIndex == -1)
  {
    myPowerPorts.push_back(9);
    myPowerPorts.push_back(22);
    myPowerPorts.push_back(23);
  }
  else if(deviceIndex == 0)
    myPowerPorts.push_back(9);
  else if(deviceIndex == 1)
    myPowerPorts.push_back(22);
  else if(deviceIndex == 2)
    myPowerPorts.push_back(23);
  else
    MvrLog::log(MvrLog::Terse, "MvrDPPTU: Warning: No Seekur power port assignment known for PTU device #%d, won't turn any on.");

  if(myRobot)
  {
    for(std::vector<char>::const_iterator i = myPowerPorts.begin(); i != myPowerPorts.end(); ++i)
    {
      myRobot->com2Bytes(116, *i, 1);
    }
    //myRobot->addDisconnectNormallyCallback(&myShutdownCB);
    //myRobot->addDisconnectOnErrorCallback(&myShutdownCB);
    //myRobot->addUserTask("MvrDPPTU", 65, &myQueryCB);
  }

 // Mvria::addExitCallback(&myShutdownCB);
    
}

MVREXPORT MvrDPPTU::~MvrDPPTU()
{
  shutdown();
  Mvria::remExitCallback(&myShutdownCB);
}

void MvrDPPTU::shutdown()
{
  if(!myInit) return;
  haltAll();
  offStatPower();
  lowMotPower();
  if(myRobot)
  {
    for(std::vector<char>::const_iterator i = myPowerPorts.begin(); i != myPowerPorts.end(); ++i)
    {
      myRobot->com2Bytes(116, *i, 0);
    }
    myRobot->remSensorInterpTask(&myQueryCB);
  }
  myInit = false;
}

void MvrDPPTU::preparePacket(void)
{
  myPacket.empty();
  myPacket.byteToBuf(MvrDPPTUCommands::DELIM);
}

MVREXPORT bool MvrDPPTU::init(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::INIT);
  if (!sendPacket(&myPacket))
  {
    MvrLog::log(MvrLog::Terse, "MvrDPPTU: Error sending INIT to PTU! (Write error?)");
    return false;
  }

  myPan = -1;  //myPan and myTilt set to -1 for initial positioning
  myTilt = -1;

  setMovePower(PAN, LOW);
  setMovePower(TILT, LOW);
  setHoldPower(PAN, OFF);
  setHoldPower(PAN, OFF);


  switch(myDeviceType) {
    case PANTILT_PTUD47:
      //Assuming default accel and slew rates
      myPanSlew = 40;
      myBasePanSlew = 40;
      myTiltSlew = 40;
      myBaseTiltSlew = 40;
      myPanAccel = 80;
      myTiltAccel = 80;
      break;
    case PANTILT_PTUD46:
    case PANTILT_DEFAULT:
    default:
      //Assuming default accel and slew rates
      myPanSlew = 40; // 1000 positions/sec
      myBasePanSlew = 40; // 1000 positions/sec
      myTiltSlew = 40; // 1000 positions/sec
      myBaseTiltSlew = 40; // 1000 positions/sec
      myPanAccel = 80; // 2000 positions/sec^2
      myTiltAccel = 80; // 2000 positions/sec^2
      break;
  }

  if(myDeviceType == PANTILT_DEFAULT)
  {
    // query resolution, conversion factors will be 
    // set again based on responses (replacing default value set 
    // in constructor)
    preparePacket();
    myPacket.byteToBuf('P');
    myPacket.byteToBuf('R');
    if(!sendPacket(&myPacket))
      MvrLog::log(MvrLog::Terse, "MvrDPPTU: Warning: write error sending pan resolution query");
    // We can't distinguish PR and TR responses based on their content alone, so
    // we have to query pan resolution (PR), then after receiving resolution
    // response, query TR. (see readPacket() for TR).
	///@todo query the device for pan and tilt limits, and when response is received, call MvrPTZ::setLimits() to change.
  }

  query();   // do first position query

  if (!panTilt(0,0))
    return false;

  myInit = true;

  return true;
}

/** A blank packet can be sent to exit monitor mode **/
MVREXPORT bool MvrDPPTU::blank(void)
{
  myPacket.empty();
  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::pan_i(double pdeg)
{
  //MvrLog::log(MvrLog::Normal, "MvrDPPTU::panTilt(%f, %f)", pdeg, tdeg);
  if (pdeg > getMaxPosPan_i())
    pdeg = getMaxPosPan_i();
  if (pdeg < getMaxNegPan_i())
    pdeg = getMaxNegPan_i();

  if (pdeg != myPanSent)
  {
	  DEBUG_CMD(MvrLog::log(MvrLog::Normal, 
      "MvrDPPTU::pan: sending command to pan to %f deg (maxPosPan=%f, minNegPan=%f, myPanSent=%f)", 
      pdeg, getMaxPosPan_i(), getMaxNegPan_i(), myPanSent); 
    )
    preparePacket();
    myPacket.byteToBuf('P');
    myPacket.byteToBuf('P');
    myPacket.byte2ToBuf(MvrMath::roundInt(pdeg/myPanConvert));

    myPanSent = pdeg;
    if(!myCanGetRealPanTilt) myPan = myPanSent;
    if (!sendPacket(&myPacket)) return false;
  }
return true;
}

MVREXPORT bool MvrDPPTU::tilt_i(double tdeg)
{
  if (tdeg > getMaxPosTilt_i())
    tdeg = getMaxPosTilt_i();
  if (tdeg < getMaxNegTilt_i())
    tdeg = getMaxNegTilt_i();

  if (tdeg != myTiltSent)
  {
	  DEBUG_CMD(MvrLog::log(MvrLog::Normal, 
      "MvrDPPTU::tilt: sending command to tilt to %f deg (maxPosTilt=%f, minNegTilt=%f, myTiltSent=%f)", 
      tdeg, getMaxPosTilt_i(), getMaxNegTilt_i(), myTiltSent)
    );
    preparePacket();
    myPacket.byteToBuf('T');
    myPacket.byteToBuf('P');
    myPacket.byte2ToBuf(MvrMath::roundInt(tdeg/myTiltConvert));

    myTiltSent = tdeg;
    if(!myCanGetRealPanTilt) myTilt = myTiltSent;
    if (!sendPacket(&myPacket)) return false;
  }

  return true;
}

MVREXPORT bool MvrDPPTU::panSlew(double deg)
{
  if (deg > getMaxPanSlew())
    deg = getMaxPanSlew();
  if (deg < getMinPanSlew())
    deg = getMinPanSlew();
  
  myPanSlew = deg;
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::PAN);
  myPacket.byteToBuf(MvrDPPTUCommands::SPEED);

  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::tiltSlew(double deg)
{
  if (deg > getMaxTiltSlew())
    deg = getMaxTiltSlew();
  if (deg < getMinTiltSlew())
    deg = getMinTiltSlew();
  
  myTiltSlew = deg;
  preparePacket();
  myPacket.byteToBuf('T');// MvrDPPTUCommands::TILT);
  myPacket.byteToBuf('S');// MvrDPPTUCommands::SPEED);

  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::resetCalib(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::RESET);
  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::disableReset(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::RESET);
  myPacket.byteToBuf(MvrDPPTUCommands::DISABLE);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::resetTilt(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::RESET);
  myPacket.byteToBuf(MvrDPPTUCommands::TILT);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::resetPan(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::RESET);
  myPacket.byteToBuf(MvrDPPTUCommands::PAN);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::resetAll(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::RESET);
  myPacket.byteToBuf(MvrDPPTUCommands::ENABLE);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::saveSet(void)
{
  preparePacket();
  myPacket.byteToBuf('D'); //MvrDPPTUCommands::DISABLE);
  myPacket.byteToBuf('S'); //MvrDPPTUCommands::SPEED);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::restoreSet(void)
{
  preparePacket();
  myPacket.byteToBuf('D'); //MvrDPPTUCommands::DISABLE);
  myPacket.byteToBuf('R'); //MvrDPPTUCommands::RESET);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::factorySet(void)
{
  preparePacket();
  myPacket.byteToBuf('D'); //MvrDPPTUCommands::DISABLE);
  myPacket.byteToBuf('F'); //MvrDPPTUCommands::FACTORY);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::limitEnforce(bool val)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::LIMIT);

  if (val)
    myPacket.byteToBuf(MvrDPPTUCommands::ENABLE);
  else
    myPacket.byteToBuf(MvrDPPTUCommands::DISABLE);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::immedExec(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::IMMED);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::slaveExec(void)
{
  preparePacket();
  myPacket.byteToBuf('S'); //MvrDPPTUCommands::SPEED);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::awaitExec(void)
{
  preparePacket();
  myPacket.byteToBuf('A'); //MvrDPPTUCommands::ACCEL);

  return sendPacket(&myPacket);
}


MVREXPORT bool MvrDPPTU::haltAll(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::HALT);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::haltPan(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::HALT);
  myPacket.byteToBuf(MvrDPPTUCommands::PAN);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::haltTilt(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::HALT);
  myPacket.byteToBuf(MvrDPPTUCommands::TILT);

  return sendPacket(&myPacket);
}


MVREXPORT bool MvrDPPTU::panAccel(double deg)
{
  if (deg > getMaxPanAccel())
    deg = getMaxPanAccel();
  if (deg < getMinPanAccel())
    deg = getMinPanAccel();

  if (myPanAccel != deg) {
    preparePacket();
    myPacket.byteToBuf(MvrDPPTUCommands::PAN);
    myPacket.byteToBuf(MvrDPPTUCommands::ACCEL);
    myPacket.byte2ToBuf(MvrMath::roundInt(deg/myPanConvert));

    return sendPacket(&myPacket);
  }

  return true;
}

MVREXPORT bool MvrDPPTU::tiltAccel(double deg)
{
  if (deg > getMaxPanAccel())
    deg = getMaxPanAccel();
  if (deg < getMinPanAccel())
    deg = getMinPanAccel();

  if (myTiltAccel != deg) {
    preparePacket();
    myPacket.byteToBuf(MvrDPPTUCommands::TILT);
    myPacket.byteToBuf(MvrDPPTUCommands::ACCEL);
    myPacket.byte2ToBuf(MvrMath::roundInt(deg/myTiltConvert));

    return sendPacket(&myPacket);
  }

  return true;
}

MVREXPORT bool MvrDPPTU::basePanSlew(double deg)
{
  myBasePanSlew = deg;

  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::PAN);
  myPacket.byteToBuf(MvrDPPTUCommands::BASE);
  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::baseTiltSlew(double deg)
{
  myBaseTiltSlew = deg;

  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::TILT);
  myPacket.byteToBuf(MvrDPPTUCommands::BASE);
  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::upperPanSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::PAN);
  myPacket.byteToBuf(MvrDPPTUCommands::UPPER);
  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::lowerPanSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::PAN);
  myPacket.byteToBuf('L'); //MvrDPPTUCommands::LIMIT);
  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::upperTiltSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::TILT);
  myPacket.byteToBuf(MvrDPPTUCommands::UPPER);
  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::lowerTiltSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::TILT);
  myPacket.byteToBuf('L'); //MvrDPPTUCommands::LIMIT);
  myPacket.byte2ToBuf(MvrMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::indepMove(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::CONTROL);
  myPacket.byteToBuf('I'); //MvrDPPTUCommands::IMMED);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::velMove(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::CONTROL);
  myPacket.byteToBuf(MvrDPPTUCommands::VELOCITY);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::enMon(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::MONITOR);
  myPacket.byteToBuf(MvrDPPTUCommands::ENABLE);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::disMon(void)
{
  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::MONITOR);
  myPacket.byteToBuf(MvrDPPTUCommands::DISABLE);

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::initMon(double deg1, double deg2, 
			       double deg3, double deg4)
{

  preparePacket();
  myPacket.byteToBuf(MvrDPPTUCommands::MONITOR);

  myPacket.byte2ToBuf(MvrMath::roundInt(deg1/myPanConvert));
  myPacket.byteToBuf(',');
  myPacket.byte2ToBuf(MvrMath::roundInt(deg2/myPanConvert));
  myPacket.byteToBuf(',');
  myPacket.byte2ToBuf(MvrMath::roundInt(deg3/myTiltConvert));
  myPacket.byteToBuf(',');
  myPacket.byte2ToBuf(MvrMath::roundInt(deg4/myTiltConvert));

  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::setHoldPower(Axis axis, PowerMode mode)
{
  preparePacket();
  myPacket.byteToBuf(axis);
  myPacket.byteToBuf('H');
  myPacket.byteToBuf(mode);
  return sendPacket(&myPacket);
}

MVREXPORT bool MvrDPPTU::setMovePower(Axis axis, PowerMode mode)
{
  preparePacket();
  myPacket.byteToBuf(axis);
  myPacket.byteToBuf('M');
  myPacket.byteToBuf(mode);
  return sendPacket(&myPacket);
}


MVREXPORT MvrBasePacket *MvrDPPTU::readPacket()
{
  if(!myConn)
    return NULL;

  // ask for next set of data every robot cycle
  query();

  const int databufsize = 64;
  char databuf[databufsize];
  int databufp = 0;

  // state indicates what part of the message we expect
  // to see next as we read and scan it.
  enum State {
    start,
    space,
    data,
    end
  } state;
  state = start;
  bool errormsg = false;
  bool gotpan = false;
  bool gottilt = false;

  const int maxbytes = 512;

  // bug, this doesn't handle messages broken accross our 512 byte read size.

  int i;
  for(i = 0;  i < maxbytes; ++i)
  {
    char c;
    if(myConn->read(&c, 1, 1) <= 0)
      break; // no data, abort read until next robot cycle

    //printf("MvrDPPTU::read[%d]= %c (0x%x)\n", i, c, c);

    if(c == '*')
      state = start; // jump into start of new message no matter what the previous state was. (start state will see * again and proceed to state after that)

    switch (state)
    {
      case start:
        //puts("(RE)START PARSE");
        databuf[0] = 0;
        databufp = 0;
        if(c == '*')
        {
          state = space;
          errormsg = false;
        }
        else if(c == '!')
        {
          state = space;
          errormsg = true;
        }
        else // error, restart
        {
          state = start;
        }
        break;

      case space:
        if(c == ' ')
        {
          state = data;
        }
        else // error, restart
        {
          state = start;
        }
        break;

      case data:
        if(c == '\r')
        {
          //puts("IN DATA, REACHED END");
          databuf[databufp++] = '\0';
          state = end;
        }
        else if(!isprint(c))
        {
          // error, restart
          state = start;
        }
        else
        {
          databuf[databufp++] = c;
          if(databufp > databufsize)
          {
            MvrLog::log(MvrLog::Terse, "MvrDPPTU: Internal parse error, data in incoming message is too large for buffer, aborting.");
            state = start;
          }
        }
        break;

      case end:
      default:
        // fall out of switch, end is a special flag state checked outside this switch
        // where action is taken based on what data was received from the dpptu,
        // then state is reset to start.
        break;
    }

    if(state == end)
    {

      // responses we handle are:
      const char *panPosResponse = "Current Pan position is ";
      const size_t panPosResponseLen = strlen(panPosResponse);
      const char *tiltPosResponse = "Current Tilt position is ";
      const size_t tiltPosResponseLen = strlen(tiltPosResponse);
      const char *resResponse = " seconds arc per position";

      //printf("IN PARSE END, DATABUF=%s   \n", databuf);
      if(strncmp(databuf, panPosResponse, panPosResponseLen) == 0)
      {
        char *s = databuf + panPosResponseLen;
        DEBUG_POS(printf("\npan position: %s\n", s));
        myPan = myPanRecd = atof(s) * myPanConvert;
        myCanGetRealPanTilt = true;
        gotpan = true;
      }
      else if(strncmp(databuf, tiltPosResponse, tiltPosResponseLen) == 0)
      {
        char *s = databuf + tiltPosResponseLen;
        DEBUG_POS(printf("\ntilt position: %s\n", s));
        myTilt = myTiltRecd = atof(s) * myTiltConvert;
        myCanGetRealPanTilt = true;
        gottilt = true;
      }
// XXX TODO get pan/tilt and slew limits as well
// conversion factors to ( response / (60.0*60.0) ) (ticks per arcsecond...   converting to degrees)
      else if(char *s = strstr(databuf, resResponse))
      {
         //printf("it's a resolution response. (already got pan? %d; already got tilt?  %d)", myGotPanRes, myGotTiltRes);
        *s = '\0';
        const float res = atof(databuf);
        if(!myGotPanRes)
        {
          myPanConvert = res / (60.0*60.0);  // convert from arcsecond to degr
          MvrLog::log(MvrLog::Normal, "MvrDPPTU: Received pan resolution response from PTU: %f deg / %f arcsec. Now requesting tilt resolution...", myPanConvert, res);

          // Now ask for tilt resolution (there is no way to distinguish the
          // responses from the PTU so we have to ask for them in sequence.)
          myGotPanRes = true;
          preparePacket();
          myPacket.byteToBuf('T');
          myPacket.byteToBuf('R');
          sendPacket(&myPacket);
        }
        else if(!myGotTiltRes)
        {
          myTiltConvert = res / (60.0*60.0);
          MvrLog::log(MvrLog::Normal, "MvrDPPTU: Received tilt resolution response from PTU: %f deg / %f arcsec", myTiltConvert, res);
          myGotTiltRes = true;
        }
        else
        {
          MvrLog::log(MvrLog::Normal, "MvrDPPTU: Warning: got unexpected resolution response (already received both pan and tilt resolution responses). Ignoring.");
        }
      }
      else
      {
        // unrecognized message.
        MvrLog::log(MvrLog::Normal, "MvrDPPTU: Warning: received unrecognized message from PTU: %s", databuf);
      }

      state = start; // start looking for next message


    }


  }

  if(gotpan && gottilt)
  {
      // a message was recognized and handled above.
      DEBUG_TIME(MvrLog::log(MvrLog::Normal, "MvrDPPTU recieve interval=%ld", myLastPositionMessageHandled.mSecSince()));
      myLastPositionMessageHandled.setToNow();
      myWarnedOldPositionData = false;

      // ask for next position . this times our queries to the actual     //
      // recieve rate.
      // moved to above to always query every loop query();
  }

  if(myLastPositionMessageHandled.mSecSince() > 2000 && !myWarnedOldPositionData)
  {
    MvrLog::log(MvrLog::Terse, "MvrDPPTU: Warning: Have not received pan and tilt position for more than 2 seconds! Data will be incorrect.");
    myWarnedOldPositionData = true;
  }

  if(i >= maxbytes)
  {
    MvrLog::log(MvrLog::Normal, "MvrDPPTU: Warning: parse or communications error: no valid message found in %d bytes read from PTU.", maxbytes);
  }

  // next time.
  return NULL;
}


void MvrDPPTU::query()
{
  DEBUG_TIME(MvrLog::log(MvrLog::Normal, "MvrDPPTU query interval=%ld", myLastQuerySent.mSecSince()));
  // ask for pan and tilt positions.
  if(myConn)
  {
    if(myConn->write("PP\rTP\r", 2*2+2) <= 0) return;
    myLastQuerySent.setToNow();
  }
}


MvrPTZConnector::GlobalPTZCreateFunc MvrDPPTU::ourCreateFunc(&MvrDPPTU::create);

MvrPTZ* MvrDPPTU::create(size_t index, MvrPTZParams params, MvrArgumentParser *parser, MvrRobot *robot)
{
  return new MvrDPPTU(robot);
}

void MvrDPPTU::registerPTZType()
{
  MvrPTZConnector::registerPTZType("dpptu", &ourCreateFunc);
}
