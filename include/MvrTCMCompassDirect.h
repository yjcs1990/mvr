/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrTCMCompassDirect.h
 > Description  : Talk to a compass directly over a computer serial port
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRTCMCOMPASSDIRECT_H
#define MVRTCMCOMPASSDIRECT_H


#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrDeviceConnection.h"
#include "MvrTCM2.h"
#include "MvrNMEAParser.h"

/** @brief Talk to a compass directly over a computer serial port
 *  
 *  This class configures and recieves compass heading data from a TCM2 or TCM2.5 compass
 *  directly over a computer serial port, rather than via tha robot.
 *  This class cannot recieve pitch, roll or temperature data from the compass.
 *  On all Pioneer robots, the TCM compass (as originally installed by
 *  MobileRobots) is connected to the robot microcontroller, so if you 
 *  have a Pioneer with this configuration, you  should instead use the 
 *  MvrTCMCompassRobot class. Only use this class if you
 *  have connected the compass to the computer serial port.
 *
 *  You can create an instance of this class directly, or using an
 *  MvrCompassConnector object and giving the "-compassType serialtcm" program
 *  argument. 
 *
 */
class MvrTCMCompassDirect : public virtual MvrTCM2  
{
private:
  MvrDeviceConnection *myDeviceConnection;
  bool myCreatedOwnDeviceConnection;
  const char *mySerialPortName;
  MvrNMEAParser myNMEAParser;
  bool sendTCMCommand(const char *str, ...);
  MvrFunctor1C<MvrTCMCompassDirect, MvrNMEAParser::Message>  myHCHDMHandler;
  void handleHCHDM(MvrNMEAParser::Message);
public:
  MVREXPORT MvrTCMCompassDirect(MvrDeviceConnection *devCon);
  MVREXPORT MvrTCMCompassDirect(const char *serialPortName = MVRTCM2_DEFAULT_SERIAL_PORT);
  MVREXPORT ~MvrTCMCompassDirect();

  /** Open device connection if not yet open  and send commands to configure compass. */
  MVREXPORT virtual bool connect();
  MVREXPORT virtual bool blockingConnect(unsigned long connectTimeout = 5000);

  /** Send commands to begin calibrating */
  MVREXPORT virtual void commandAutoCalibration();
  MVREXPORT virtual void commandUserCalibration();
  MVREXPORT virtual void commandStopCalibration();

  /** Send commands to start/stop sending data.  */
  MVREXPORT virtual void commandContinuousPackets();
  MVREXPORT virtual void commandOnePacket();
  MVREXPORT virtual void commandOff();

  /** Not implemented yet. @todo */
  MVREXPORT virtual void commandSoftReset() { /* TODO */ }

  /** Same as commandContinuousPackets() in this implementation. */
  MVREXPORT virtual void commandJustCompass() { commandContinuousPackets(); }

  /** Read all available data, store, and call callbacks if any were added. 
   *  unsigned int msWait If 0, wait indefinately for new data. Otherwise, wait
   *  a maximum of this many milliseconds for data to arrive.
   *  @return A value > 0 if messages were recieved from the compass, 0 if no
   *  data was recieved, and a value < 0 on error reading from the compass.
   * */
  MVREXPORT int read(unsigned int msWait = 1);
 
  MVREXPORT void setDeviceConnection(MvrDeviceConnection *devCon) { myDeviceConnection = devCon; }
  MVREXPORT MvrDeviceConnection *getDeviceConnetion() { return myDeviceConnection; }
};

#endif  // MVRTCMCOMPASSDIRECT_H


