/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrTrimbleGPS.h
 > Description  : GPS subclass to support the Trimble AgGPS and other Trimble GPS devices.
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRTRIMBLEGPS_H
#define MVRTRIMBLEGPS_H


#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "mvriaUtil.h"
#include "MvrGPS.h"
#include "MvrDeviceConnection.h"
#include "MvrMutex.h"

#include <deque>

class MvrTrimbleAuxDeviceConnection;

/*  This subclass extends MvrGPS to send initialization commands specific
 *  to Trimble GPS devices, and to handle the PTNLAG001 message which
 *  is specific to the Trimble GPS (this message contains data received 
 *  from an auxilliary device connected to the GPS; MvrTrimbleGPS
 *  simply parses its contents as a new NMEA message;
 */
class MvrTrimbleGPS : public virtual MvrGPS {
private:
  MvrFunctor1C<MvrTrimbleGPS, MvrNMEAParser::Message> myAuxDataHandler;
  void handlePTNLAG001(MvrNMEAParser::Message message);
public:
  MVREXPORT MvrTrimbleGPS();
  MVREXPORT virtual ~MvrTrimbleGPS();

  /** Send a TSIP command to the Trimble GPS.
   *  See the TSIP Reference guide for details.
   *  Note, the data must be 66 characters or less.
   */
  MVREXPORT bool sendTSIPCommand(char command, const char *data, size_t dataLen);

protected:
  MVREXPORT virtual bool initDevice();

};


#endif 