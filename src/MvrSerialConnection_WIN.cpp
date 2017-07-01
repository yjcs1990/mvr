#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSerialConnection.h"
#include "MvrLog.h"
#include "mvriaUtil.h"


// PS 7/11/11 - nothing is done different for windows for 422
MVREXPORT MvrSerialConnection::MvrSerialConnection(bool is422)
{
  myPort = INVALID_HANDLE_VALUE;
  myBaudRate = 9600;
  myStatus = STATUS_NEVER_OPENED;
  myHardwareControl = false;
  buildStrMap();
  if (is422)
    setPortType("serial422");
  else
    setPortType("serial");
}

MVREXPORT MvrSerialConnection::~MvrSerialConnection()
{
  if (myPort != INVALID_HANDLE_VALUE)
    close();
}

void MvrSerialConnection::buildStrMap(void)
{
  myStrMap[OPEN_COULD_NOT_OPEN_PORT] = "Could not open serial port.";
  myStrMap[OPEN_COULD_NOT_SET_UP_PORT] = "Could not set up serial port.";
  myStrMap[OPEN_INVALID_BAUD_RATE] = "Baud rate invalid, could not set baud on serial port.";
  myStrMap[OPEN_COULD_NOT_SET_BAUD] = "Could not set baud rate on serial port.";
  myStrMap[OPEN_ALREADY_OPEN] = "Serial port already open.";
}

MVREXPORT const char * MvrSerialConnection::getOpenMessage(int messageNumber)
{
  return myStrMap[messageNumber].c_str();
}

MVREXPORT bool MvrSerialConnection::openSimple(void)
{
  if (internalOpen() == 0)
    return true;
  else
    return false;
}

/**
   @param port The serial port to connect to, or NULL which defaults to 
   COM1 for windows and /dev/ttyS0 for linux
   @return 0 for success, otherwise one of the open enums
   @see getOpenMessage
*/
MVREXPORT void MvrSerialConnection::setPort(const char *port)
{
  if (port == NULL)
    myPortName = "COM1";
  else
    myPortName = port;
  setPortName(myPortName.c_str());
}

/**
   @return The seiral port to connect to
**/
MVREXPORT const char * MvrSerialConnection::getPort(void)
{
  return myPortName.c_str();
}

/**
   @param port The serial port to connect to, or NULL which defaults to 
   COM1 for windows and /dev/ttyS0 for linux
   @return 0 for success, otherwise one of the open enums
   @see getOpenMessage
*/
MVREXPORT int MvrSerialConnection::open(const char *port)
{
  setPort(port);
  return internalOpen();
}



MVREXPORT int MvrSerialConnection::internalOpen(void)
{
  DCB dcb;


  if (myStatus == STATUS_OPEN) 
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrSerialConnection::open: Serial port already open");
    return OPEN_ALREADY_OPEN;
  }

  MvrLog::log(MvrLog::Verbose, "MvrSerialConnection::internalOpen: Connecting to serial port '%s'", myPortName.c_str());


  myPort = CreateFile(myPortName.c_str(),
		      GENERIC_READ | GENERIC_WRITE,
		      0,	/* exclusive access  */
		      NULL,	/* no security attrs */
		      OPEN_EXISTING,
		      0,
		      NULL );

  if (myPort == INVALID_HANDLE_VALUE) {
    MvrLog::logErrorFromOS(MvrLog::Terse, 
	       "MvrSerialConnection::open: Could not open serial port '%s'",
	       myPortName.c_str());
    return OPEN_COULD_NOT_OPEN_PORT;
  }
             
  if ( !GetCommState(myPort, &dcb) )
  {  
    MvrLog::logErrorFromOS(MvrLog::Terse, 
			  "MvrSerialConnection::open: Could not get port data to set up port");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_UP_PORT;
  }
  
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = 0;
  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fNull = FALSE;
  dcb.fOutX = FALSE;
  dcb.fInX =  FALSE;

  // set these explicitly and here now, instead of before when 
  // we used to set these after we did the first SetCommState...
  // This is this way since a Japanese windows xp user had some problems with 
  // changing baud rates after this first set
  if (myBaudRate > 0)
    dcb.BaudRate = myBaudRate;
  if (myHardwareControl == 0)
  {
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
  }
  else
  {
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
  }

  if ( !SetCommState(myPort, &dcb) )
  {  
    MvrLog::logErrorFromOS(MvrLog::Terse, 
	       "MvrSerialConnection::open: Could not set up port");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_UP_PORT;
  }

  myStatus = STATUS_OPEN;

  /* these are now set above, see the comments there for why
  if (!setBaud(myBaudRate)) 
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrSerialConnection::open: Could not set baud rate.");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_BAUD;
  }
  
  if (!setHardwareControl(myHardwareControl)) 
  {
    MvrLog::log(MvrLog::Terse, 
	       "MvrSerialConnection::open: Could not set hardware control.");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_UP_PORT;
  }
*/
  MvrLog::log(MvrLog::Verbose,
	     "MvrSerialConnection::open: Successfully opened and configured serial port.");
  return 0;
}



MVREXPORT bool MvrSerialConnection::close(void)
{
  bool ret;

  if (myPort == INVALID_HANDLE_VALUE)
    return true;

  /* disable event notification  */
  SetCommMask( myPort, 0 ) ;
  /* drop DTR	*/
  EscapeCommFunction( myPort, CLRDTR ) ;
  /* purge any outstanding reads/writes and close device handle  */
  PurgeComm( myPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

  myStatus = STATUS_CLOSED_NORMALLY;

  ret = CloseHandle( myPort ) ;
  if (ret)
    MvrLog::log(MvrLog::Verbose,
	       "MvrSerialConnection::close: Successfully closed serial port.");
  else
    MvrLog::logErrorFromOS(MvrLog::Verbose, 
	       "MvrSerialConnection::close: Unsuccessfully closed serial port.");
  myPort = (HANDLE) INVALID_HANDLE_VALUE;
  return ret;
}

MVREXPORT int MvrSerialConnection::getBaud(void)
{
   return myBaudRate;
}

MVREXPORT bool MvrSerialConnection::setBaud(int baud)
{
  DCB dcb;
  
  myBaudRate = baud;

  if (getStatus() != STATUS_OPEN)
    return true;

  if (baud == 0)
    return true;

  if ( !GetCommState(myPort, &dcb) )
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setBaud: Could not get port data.");
    return false;
  }

  dcb.BaudRate = myBaudRate;

  if ( !SetCommState(myPort, &dcb) )
  {  
    MvrLog::logErrorFromOS(MvrLog::Terse,
	       "MvrSerialConnection::setBaud: Could not set port data (trying baud %d).", myBaudRate);
    return false;
  }  

  return true;
}

MVREXPORT bool MvrSerialConnection::getHardwareControl(void)
{
  return myHardwareControl;
}

MVREXPORT bool MvrSerialConnection::setHardwareControl(bool hardwareControl)
{
  DCB dcb;

  myHardwareControl = hardwareControl;

  if (getStatus() != STATUS_OPEN)
    return true;
 
  if ( !GetCommState(myPort, &dcb) )
  {
    MvrLog::logErrorFromOS(MvrLog::Terse,
	       "MvrSerialConnection::setBaud: Could not get port Data.");
    return false;
  }
  
  if (myHardwareControl == 0) /* set control lines */
  {
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
  }
  else
  {
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
  }

  if ( !SetCommState(myPort, &dcb) )
  {  
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setBaud: Could not set port Data.");
    return false;
  }  
  
  return true;
}

MVREXPORT int MvrSerialConnection::write(const char *data, unsigned int size) 
{
  unsigned long ret;

  if (myPort != INVALID_HANDLE_VALUE && myStatus == STATUS_OPEN) 
  {
    if (!WriteFile(myPort, data, size, &ret, NULL)) 
    {
      MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::write: Error on writing.");
      return -1;
    }
    return ret;
  }
  MvrLog::log(MvrLog::Terse, "MvrSerialConnection::write: Connection invalid.");
  return -1;
}

MVREXPORT int MvrSerialConnection::read(const char *data, unsigned int size, 
				      unsigned int msWait) 
{
  COMSTAT stat;
  unsigned long ret;
  unsigned int numToRead;
  MvrTime timeDone;

  if (myPort != INVALID_HANDLE_VALUE && myStatus == STATUS_OPEN)
  {
    if (msWait > 0)
    {
      timeDone.setToNow();
      if (!timeDone.addMSec(msWait)) {
        MvrLog::log(MvrLog::Normal,
                   "MvrSerialConnection::read() error adding msecs (%i)",
                   msWait);
      }
      while (timeDone.mSecTo() >= 0) 
      {
	if (!ClearCommError(myPort, &ret, &stat))
	  return -1;
	if (stat.cbInQue < size)
	  MvrUtil::sleep(2);
	else
	  break;
      }
    }
    if (!ClearCommError(myPort, &ret, &stat))
      return -1;
    if (stat.cbInQue == 0)
      return 0;
    if (stat.cbInQue > size)
      numToRead = size;
    else
      numToRead = stat.cbInQue;
    if (ReadFile( myPort, (void *)data, numToRead, &ret, NULL))
    {
      return (int)ret;
    }
    else 
    {
      MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::read:  Read failed.");
      return -1;
    }
  }
  MvrLog::log(MvrLog::Terse, "MvrSerialConnection::read: Connection invalid.");
  return -1;
}


MVREXPORT int MvrSerialConnection::getStatus(void)
{
  return myStatus;
}

MVREXPORT bool MvrSerialConnection::isTimeStamping(void)
{
  return false;
}

MVREXPORT MvrTime MvrSerialConnection::getTimeRead(int index)
{
  MvrTime now;
  now.setToNow();
  return now;
}

MVREXPORT bool MvrSerialConnection::getCTS(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_CTS_ON);
  }
  else
  {
    fprintf(stderr, "problem with GetCommModemStatus\n");
    return false;
  }
} 

MVREXPORT bool MvrSerialConnection::getDSR(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_DSR_ON);
  }
  else
  {
    fprintf(stderr, "problem with GetCommModemStatus\n");
    return false;
  }
} 

MVREXPORT bool MvrSerialConnection::getDCD(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_RLSD_ON);
  }
  else
  {
    fprintf(stderr, "problem with GetCommModemStatus\n");
    return false;
  }
}

MVREXPORT bool MvrSerialConnection::getRing(void)
{
  DWORD modemStat;
  if (GetCommModemStatus(myPort, &modemStat))
  {
    return (bool) (modemStat & MS_RING_ON);
  }
  else
  {
    fprintf(stderr, "problem with GetCommModemStatus\n");
    return false;
  }
}

