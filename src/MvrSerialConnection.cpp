#include "MvrExport.h"
#include "mvriaOSDef.h"
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/serial.h>

#include "MvrSerialConnection.h"
#include "MvrLog.h"
#include "mvriaUtil.h"


#define TIOGETTIMESTAMP         0x5480
#define TIOSTARTTIMESTAMP       0x5481

MVREXPORT MvrSerialConnection::MvrSerialConnection(bool is422)
{
  myPort = -1;
  myPortName = "none";
  myBaudRate = 9600;
  myHardwareControl = false;
  myStatus = STATUS_NEVER_OPENED;
  myTakingTimeStamps = false;
  myIs422 = is422;
  if (myIs422)
    setPortType("serial422");
  else
    setPortType("serial");
  buildStrMap();
}

MVREXPORT MvrSerialConnection::~MvrSerialConnection()
{
  if (myPort != -1)
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

MVREXPORT int MvrSerialConnection::internalOpen(void)
{
  struct termios tio;

  if (myStatus == STATUS_OPEN) 
  {
    MvrLog::log(MvrLog::Terse, "MvrSerialConnection::internalOpen: Serial port already open");
    return OPEN_ALREADY_OPEN;
  }

  if (myIs422)
    MvrLog::log(MvrLog::Verbose, "MvrSerialConnection::internalOpen: Connecting to serial422 port '%s'", myPortName.c_str());
  else
    MvrLog::log(MvrLog::Verbose, "MvrSerialConnection::internalOpen: Connecting to serial port '%s'", myPortName.c_str());

  /* open the port */
  if (!myIs422)
  {
	  if ((myPort = MvrUtil::open(myPortName.c_str(),O_RDWR | O_NDELAY)) < 0)
	  {
		  MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::open: Could not open serial port '%s'", myPortName.c_str());
		  return OPEN_COULD_NOT_OPEN_PORT;
	  }
  }
  else
  {
	  // PS 9/9/11 - the RDONLY worked for the s3series as it did not
	  // do any writes, but the sZseries needs to do writes, so changed the
	  // flag to RDWR
	  //	 if ((myPort = MvrUtil::open(myPortName.c_str(),O_RDONLY | O_NOCTTY)) < 0)
	  if ((myPort = MvrUtil::open(myPortName.c_str(),O_RDWR | O_NOCTTY)) < 0)
	  {
		  MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::open: Could not open serial port '%s'", myPortName.c_str());
		  return OPEN_COULD_NOT_OPEN_PORT;
	  }
  }

  
  /* set the tty baud, buffering and modes */
  if (tcgetattr(myPort, &tio) != 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::open: Could not get port data to set up port");
    close();
    myStatus = STATUS_OPEN_FAILED;
    return OPEN_COULD_NOT_SET_UP_PORT;
  }    

  /* turn off echo, canonical mode, extended processing, signals */
  tio.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  /* turn off break sig, cr->nl, parity off, 8 bit strip, flow control */
  tio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  /* clear size, turn off parity bit */
  tio.c_cflag &= ~(CSIZE | PARENB);

  /* set size to 8 bits */
  tio.c_cflag |= CS8;

  /* turn output processing off */
  tio.c_oflag &= ~(OPOST);

  /* Set time and bytes to read at once */
  tio.c_cc[VTIME] = 0;
  tio.c_cc[VMIN] = 0;

  // PS 7/3/11 - check if dev is RS422, if so then the cflags need
  // to be set different, for now test for dev/ttyS3, later we need to input that
  // the device is RS422

  // PS 9/9/11 - taking out the B38500, we now set the baud below
  if (myIs422)
	  tio.c_cflag = CS8 | CLOCAL | CREAD |IGNPAR;
      //tio.c_cflag = B57600 | CS8 | CLOCAL | CREAD |IGNPAR;

  if (tcflush(myPort,TCIFLUSH) == -1)
  {
	  MvrLog::logErrorFromOS(MvrLog::Terse,
			  "MvrSerialConnection::open: Could not set up port tcflush failed");
	  close();
	  myStatus = STATUS_OPEN_FAILED;
	  return OPEN_COULD_NOT_SET_UP_PORT;
  }

  if (tcsetattr(myPort,TCSAFLUSH,&tio) == -1)
  {
	  MvrLog::logErrorFromOS(MvrLog::Terse,
			  "MvrSerialConnection::open: Could not set up port");
	  close();
	  myStatus = STATUS_OPEN_FAILED;
	  return OPEN_COULD_NOT_SET_UP_PORT;
  }

  myStatus = STATUS_OPEN;

  // PS 7/3/11 - only set the baud and hw control if RS232
  // for now test for dev/ttyS3, later we need to input that
  // the device is RS422

  if (!myIs422)
  {
	  if (myBaudRate != 0 && rateToBaud(myBaudRate) == -1)
	  {
		  MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::open: Invalid baud rate.");
		  close();
		  myStatus = STATUS_OPEN_FAILED;
		  return OPEN_INVALID_BAUD_RATE;
	  }

	  //printf("my baud rate = %d\n",myBaudRate);
	  if (myBaudRate != 0 && !setBaud(myBaudRate))
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
  }
  // PS 9/9/11 - added else for SZ
  else
  {
	  if (myBaudRate != 0 && rateToBaud(myBaudRate) == -1)
	  {
		  MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::open: Invalid baud rate.");
		  close();
		  myStatus = STATUS_OPEN_FAILED;
		  return OPEN_INVALID_BAUD_RATE;
	  }

	  //printf("my baud rate = %d\n",myBaudRate);
	  if (myBaudRate != 0 && !setBaud(myBaudRate))
	  {
		  MvrLog::log(MvrLog::Terse,
				  "MvrSerialConnection::open: Could not set baud rate.");
		  close();
		  myStatus = STATUS_OPEN_FAILED;
		  return OPEN_COULD_NOT_SET_BAUD;
	  }
  }

  MvrLog::log(MvrLog::Verbose, "MvrSerialConnection::open: Successfully opened and configured serial port '%s'.", myPortName.c_str());
  return 0;
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
   @see getOpenMessage
*/
MVREXPORT void MvrSerialConnection::setPort(const char *port)
{
  if (port == NULL)
    myPortName = "/dev/ttyS0";
  else
    myPortName = port;
  setPortName(myPortName.c_str());
}

/**
   @return The serial port to connect to
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

MVREXPORT bool MvrSerialConnection::close(void)
{
  int ret;

  myStatus = STATUS_CLOSED_NORMALLY;
  if (myPort == -1)
    return true;
  

  ret = ::close(myPort);

  if (ret == 0)
    MvrLog::log(MvrLog::Verbose,
	       "MvrSerialConnection::close: Successfully closed serial port.");
  else
    MvrLog::logErrorFromOS(MvrLog::Verbose, 
	       "MvrSerialConnection::close: Unsuccessfully closed serial port.");

  myPort = -1;
  if (ret == 0)
    return true;
  else
    return false;
}

MVREXPORT bool MvrSerialConnection::setBaud(int rate)
{
  struct termios tio;  
  int baud;

  myBaudRate = rate;
  
  if (getStatus() != STATUS_OPEN)
    return true;

  if (myBaudRate == 0)
    return true;

  if ((baud = rateToBaud(myBaudRate)) == -1)
    return false;
  
  if (tcgetattr(myPort, &tio) != 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setBaud: Could not get port data.");
    return false;
  }
  
  if (cfsetospeed(&tio, baud)) 
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setBaud: Could not set output baud rate on termios struct.");
    return false;
  }
       
  if (cfsetispeed(&tio, baud)) 
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setBaud: Could not set input baud rate on termios struct.");
    return false;
  }

  if(tcsetattr(myPort,TCSAFLUSH,&tio) < 0) 
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setBaud: Could not set baud rate.");
    return false;
  }

  startTimeStamping();
  
  return true;
}

MVREXPORT void MvrSerialConnection::startTimeStamping(void)
{
  long baud;
  baud = myBaudRate;
  if (ioctl(myPort, TIOSTARTTIMESTAMP, &baud) != 0)
    myTakingTimeStamps = false;
  else
    myTakingTimeStamps = true;
}

/**
   @return the current baud rate of the connection
*/

MVREXPORT int MvrSerialConnection::getBaud(void)
{
  return myBaudRate;
}

int MvrSerialConnection::rateToBaud(int rate)
{
  switch (rate) {
  case 300: return B300;
  case 1200: return B1200;
  case 1800: return B1800;
  case 2400: return B2400;
  case 4800: return B4800;
  case 9600: return B9600;
  case 19200: return B19200;
  case 38400: return B38400;
  case 57600: return B57600;
  case 115200: return B115200;
  case 230400: return B230400;
  case 460800: return B460800;
  default: 
    MvrLog::log(MvrLog::Terse, "MvrSerialConnection::rateToBaud: Did not know baud for rate %d.", rate);
    return -1;
  }
}

int MvrSerialConnection::baudToRate(int baud)
{
  switch (baud) {
  case B300: return 300;
  case B1200: return 1200;
  case B1800: return 1800;
  case B2400: return 2400;
  case B4800: return 4800;
  case B9600: return 9600;
  case B19200: return 19200;
  case B38400: return 38400;
  case B57600: return 57600;
  case B115200: return 115200;
  case B230400: return 230400;
  case B460800: return 460800;
  default: 
    MvrLog::log(MvrLog::Terse, "MvrSerialConnection:baudToRate: Did not know rate for baud.");
    return -1;
  }
  
}

/**
   @param hardwareControl true to enable hardware control of lines
   @return true if the set succeeded
*/

MVREXPORT bool MvrSerialConnection::setHardwareControl(bool hardwareControl)
{
  struct termios tio;

  myHardwareControl = hardwareControl;

  if (getStatus() != STATUS_OPEN) 
    return true;

  //tcgetattr(myPort, &tio);

  if (tcgetattr(myPort, &tio) != 0)
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setHardwareControl: Could not get port data.");
    return false;
  }


  /* check for hardware flow control */
  if (myHardwareControl)
    tio.c_cflag |= CRTSCTS;
  else
    tio.c_cflag &= ~CRTSCTS;
      
  if(tcsetattr(myPort,TCSAFLUSH,&tio) < 0) {
    MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::setHardwareControl: Could not set hardware control.");
    return false;
  }
  
  return true;
}

/**
   @return true if hardware control of lines is enabled, false otherwise
*/
MVREXPORT bool MvrSerialConnection::getHardwareControl(void)
{
  return myHardwareControl;
}

MVREXPORT int MvrSerialConnection::write(const char *data, unsigned int size) 
{
  int n;
  /*
  printf("SERIAL_WRITE(%3d): ", size);
  for (int i = 0; i < size; i++)
    printf("0x%x %c", data[i], data[i]);
  printf("\n");
  */
  
  /* 
  char buf[10000];
  sprintf(buf, "SERIAL_WRITE(%3d bytes %d): ", size, myPort);
  for (int i = 0; i < size; i++)
    sprintf(buf, "%s %02x", buf, (unsigned char)data[i]);
  MvrLog::log(MvrLog::Normal, buf);
  */

  if (myPort >= 0) 
  {
    n = ::write(myPort, data, size);
    if (n == -1) 
    {
#if 0
      if (errno == EAGAIN)   /* try it again, for USB/serial */
	{
	  usleep(10);
	  n = ::write(myPort, data, size);
	  if (n >= 0)
	    return n;
	}
#endif 
      MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::write: Error on writing.");
    }
    return n;
  }
  MvrLog::log(MvrLog::Terse, "MvrSerialConnection::write: Connection invalid.");
  return -1;
}

MVREXPORT int MvrSerialConnection::read(const char *data, unsigned int size,
				      unsigned int msWait) 
{
  struct timeval tp;		/* time interval structure for timeout */
  fd_set fdset;			/* fd set ??? */
  int n;
  long timeLeft;
  unsigned int bytesRead = 0;
  MvrTime timeDone;

  if (myPort >= 0)
  {
    if (msWait >= 0)
    {
      timeDone.setToNow();
      if (!timeDone.addMSec(msWait)) {
        MvrLog::log(MvrLog::Normal,
                   "MvrSerialConnection::read() error adding msecs (%i)",
                   msWait);
      }
      while ((timeLeft = timeDone.mSecTo()) >= 0) 
      {
	tp.tv_sec = (timeLeft) / 1000;	/* we're polling */
	tp.tv_usec = (timeLeft % 1000) * 1000;
	FD_ZERO(&fdset);
	FD_SET(myPort,&fdset);
	if (select(myPort+1,&fdset,NULL,NULL,&tp) <= 0) 
	  return bytesRead;
	if ((n = ::read(myPort, const_cast<char *>(data)+bytesRead, 
			size-bytesRead)) == -1)
	{
	  MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::read:  Blocking read failed.");
	  return bytesRead;
	}
	bytesRead += n;
	if (bytesRead >= size)
	  return bytesRead;
      }
      return bytesRead;
    }
    else 
    {
      n = ::read(myPort, const_cast<char *>(data), size);
      if (n == -1)
	MvrLog::logErrorFromOS(MvrLog::Terse, "MvrSerialConnection::read:  Non-Blocking read failed.");
      return n;
    }
  }
  MvrLog::log(MvrLog::Normal, "MvrSerialConnection::read:  Connection invalid.");
  return -1;
}

MVREXPORT int MvrSerialConnection::getStatus(void)
{
  return myStatus;
}

MVREXPORT MvrTime MvrSerialConnection::getTimeRead(int index)
{
  MvrTime ret;
  struct timeval timeStamp;
  if (myPort <= 0)
  {
    ret.setToNow();
    return ret;
  }

  if (myTakingTimeStamps)
  {
    timeStamp.tv_sec = index;
    if (ioctl(myPort, TIOGETTIMESTAMP, &timeStamp) == 0)
    {
      ret.setSec(timeStamp.tv_sec);
      ret.setMSec(timeStamp.tv_usec / 1000);
    }
    else
      ret.setToNow();
  }
  else
    ret.setToNow();

  return ret;
}

MVREXPORT bool MvrSerialConnection::isTimeStamping(void)
{
  return myTakingTimeStamps;
}


MVREXPORT bool MvrSerialConnection::getCTS(void)
{
  unsigned int value;
  if (ioctl(myPort, TIOCMGET, &value) == 0)
  {
    return (bool) (value & TIOCM_CTS);
  }
  else
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSerialConnection::getCTS: ioctl(TIOCMGET)");
    return false;
  }
}

MVREXPORT bool MvrSerialConnection::getDSR(void)
{
  unsigned int value;
  if (ioctl(myPort, TIOCMGET, &value) == 0)
  {
    return (bool) (value & TIOCM_DSR);
  }
  else
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSerialConnection::getDSR: ioctl(TIOCMGET)");
    return false;
  }
}

MVREXPORT bool MvrSerialConnection::getDCD(void)
{
  unsigned int value;
  if (ioctl(myPort, TIOCMGET, &value) == 0)
  {
    return (bool) (value & TIOCM_CAR);
  }
  else
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSerialConnection::getDCD: ioctl(TIOCMGET)");
    return false;
  }
}

MVREXPORT bool MvrSerialConnection::getRing(void)
{
  unsigned int value;
  if (ioctl(myPort, TIOCMGET, &value) == 0)
  {
    return (bool) (value & TIOCM_RI);
  }
  else
  {
    MvrLog::logErrorFromOS(MvrLog::Normal, "MvrSerialConnection::getRing: ioctl(TIOCMGET)");
    return false;
  }
}
