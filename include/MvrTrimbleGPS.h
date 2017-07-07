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

/** @brief GPS subclass to support the Trimble AgGPS and other Trimble GPS devices.
 *  
 *  This subclass extends MvrGPS to send initialization commands specific
 *  to Trimble GPS devices, and to handle the PTNLAG001 message which
 *  is specific to the Trimble GPS (this message contains data received 
 *  from an auxilliary device connected to the GPS; MvrTrimbleGPS
 *  simply parses its contents as a new NMEA message; i.e. data received
 *  by the Trimble is assumed to be NMEA messages that it forwards
 *  via the PTNLAG001 message.)
 *
 *  @note You must also configure the ports using
 *  the Trimble AgRemote program
 *  (http://www.trimble.com/support_trl.asp?pt=AgRemote&Nav=Collection-1545).  
 *  Enable the following messages on whichever
 *  GPS port the computer is connected to: GPRMC, GPGGA, GPGSA, GPGSV, GPGST,
 *  GPMSS, and set input (I) protocol to TSIP 38k baud, and output
 *  protocol (O) to NMEA 38k baud. 
 *  This configuration is done by MobileRobots when shipping a Trimble AgGPS
 *  but you may need to do this if the GPS loses its configuration or after
 *  changing any other settings (Note that AgRemote resets the port settings each time
 *  it connects, so you must reset them each time before exiting AgRemote!)
 *
 *  @since 2.6.0
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


