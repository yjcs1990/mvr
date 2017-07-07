#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrVersalogicIO.h"
#include "mvriaInternal.h"

#include <sys/ioctl.h>
#include <fcntl.h>

/* These are taken from the amrio_dev.h file, which is part of the amrio module itself */

#define AMRIO_MAJOR_NUM       (250)
#define AMRIO_MODULE_NAME     ("amrio")

#define ANALOG_GET_VALUE                _IOWR('v', 1, unsigned int)
#define ANALOG_SET_PORT                 _IOWR('v', 2, unsigned char)
#define DIGITAL_GET_NUM_BANKS           _IOWR('v', 3, int)
#define DIGITAL_SET_BANK0               _IOWR('v', 4, unsigned char)
#define DIGITAL_SET_BANK1               _IOWR('v', 5, unsigned char)
#define DIGITAL_SET_BANK2               _IOWR('v', 6, unsigned char)
#define DIGITAL_SET_BANK3               _IOWR('v', 7, unsigned char)
#define DIGITAL_SET_BANK_DIR_IN         _IOWR('v', 8, int)
#define DIGITAL_SET_BANK_DIR_OUT        _IOWR('v', 9, int)
#define DIGITAL_GET_BANK0               _IOWR('v', 10, unsigned char)
#define DIGITAL_GET_BANK1               _IOWR('v', 11, unsigned char)
#define DIGITAL_GET_BANK2               _IOWR('v', 12, unsigned char)
#define DIGITAL_GET_BANK3               _IOWR('v', 13, unsigned char)
#define GET_SPECIAL_CONTROL_REGISTER    _IOWR('v', 14, unsigned char)

MvrMutex MvrVersalogicIO::myMutex;

/** Constructor for the MvrVersalogicIO class.  This will open the device
 * named by @a dev (the default is "/dev/amrio" if the argument is omitted).
 It will find the number of digital banks and set the to inputs.  It will also
 attempt to take an analog reading, which will fail if there is not analog chip
 present.  If the conversion fails it will disable the analog portion of the
 code.

 Check isEnabled() to see if the device was properly opened during construction.
*/
MVREXPORT MvrVersalogicIO::MvrVersalogicIO(const char * dev) :
  myDisconnectCB(this, &MvrVersalogicIO::closeIO)
{
  myMutex.setLogName("MvrVersalogicIO::myMutex");
  myNumBanks = 0;

  myDigitalBank0 = 0;
  myDigitalBank1 = 0;
  myDigitalBank2 = 0;
  myDigitalBank3 = 0;

  MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::MvrVersalogicIO: opening device %s", dev);
  myFD = MvrUtil::open(dev, O_RDWR);

  if (myFD == -1)
  {
    MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::MvrVersalogicIO: open %s failed.  Disabling class", dev);
    myEnabled = false;
  }
  else
  {
    if(ioctl(myFD, DIGITAL_GET_NUM_BANKS, &myNumBanks))
      MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::MvrVersalogicIO: failed to get the number of digital IO banks");
    // set the digital banks to inputs so as to not drive anything
    int i;
    for(i=0;i<myNumBanks;i++)
      setDigitalBankDirection(i, DIGITAL_INPUT);
    myEnabled = true;

    // check to see if the analog works
    double val;
    MvrLog::log(MvrLog::Verbose, "MvrVersalogicIO::MvrVersalogicIO: testing analog functionality");
    if ((ioctl(myFD, ANALOG_SET_PORT, 0) != 0) || (ioctl(myFD,ANALOG_GET_VALUE, &val) != 0))
    {
      MvrLog::log(MvrLog::Verbose, "MvrVersalogicIO::MvrVersalogicIO: analog conversion failed.  Disabling analog functionality");
      myAnalogEnabled = false;
    }
    else
    {
      MvrLog::log(MvrLog::Verbose, "MvrVersalogicIO::MvrVersalogicIO: analog conversion succeeded.");
      myAnalogEnabled = true;
    }

    Mvria::addExitCallback(&myDisconnectCB);
    MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::MvrVersalogicIO: device opened");
  }
}

/** Destructor.  Attempt to close the device if it was opened
 **/
MVREXPORT MvrVersalogicIO::~MvrVersalogicIO(void)
{
  if (myEnabled)
    closeIO();
}

/** Close the device when Mvr exits
 **/
MVREXPORT bool MvrVersalogicIO::closeIO(void)
{
  myEnabled = false;

  if (close(myFD) == -1)
  {
    MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::~MvrVersalogicIO: close failed on file descriptor!");
    return false;
  }
  else
  {
    MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::~MvrVersalogicIO: closed device");
    return true;
  }
}

/** Returns the raw bit value as read by the chip.

 @param port the port number, between 0 and 7
 @param val the address of the integer to store the reading in
 @return true if a reading was acquired.  false otherwise
 **/
MVREXPORT bool MvrVersalogicIO::getAnalogValueRaw(int port, int *val)
{
  if (!myEnabled || !myAnalogEnabled)
    return false;

  unsigned int tmp;

  if (ioctl(myFD, ANALOG_SET_PORT, port) != 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::getAnalogValueRaw: failed to set analog port %d", port);
    return false;
  }

  if (ioctl(myFD, ANALOG_GET_VALUE, &tmp) != 0)
  {
    MvrLog::log(MvrLog::Terse, "MvrVersalogicIO::getAnalogValueRaw: failed to get analog port %d", port);
    return false;
  }

  *val = (int) tmp;

  return true;
}

/** Returns an analog value converted from the raw reading to a scale
 if 0-5V.

 @param port the port number, between 0 and 7
 @param val the address of the double to store the reading in
 @return true if a reading was acquired.  false otherwise
 **/
MVREXPORT bool MvrVersalogicIO::getAnalogValue(int port, double *val)
{
  int tmp;

  // the reading is 4096 units, for a range of 0-5V
  if (getAnalogValueRaw(port, &tmp)) {
    *val = 5 * (tmp/4096.0);
    return true;
  }
  else {
    return false;
  }
}

// this doesn't work, yet
MVREXPORT MvrVersalogicIO::Direction MvrVersalogicIO::getDigitalBankDirection(int bank)
{
  return DIGITAL_OUTPUT;
}

MVREXPORT bool MvrVersalogicIO::setDigitalBankDirection(int bank, Direction dir)
{
  if (!myEnabled)
    return false;

  if (dir == DIGITAL_INPUT)
  {
    if (ioctl(myFD, DIGITAL_SET_BANK_DIR_IN, bank) != 0)
      return false;
  }
  else if (dir == DIGITAL_OUTPUT)
  {
    if (ioctl(myFD, DIGITAL_SET_BANK_DIR_OUT, bank) != 0)
      return false;
  }
  else
  {
    MvrLog::log(MvrLog::Verbose, "MvrVersalogicIO::setDigitalBankDirection: invalid argument for direction");
    return false;
  }

  return true;
}

/** Returns the bits of the digital input bank.  A zero is 'on' for the hardware
 side of things, so this function negates it to make it more intuitive.
 
  @return true if the ioctl call was succcessfull, false otherwise
  **/
MVREXPORT bool MvrVersalogicIO::getDigitalBankInputs(int bank, unsigned char *val)
{
  if (!myEnabled)
    return false;

  if (bank < 0 || bank > myNumBanks - 1)
    return false;

  unsigned char tmp;
  switch (bank)
  {
    case 0:
      if (ioctl(myFD, DIGITAL_GET_BANK0, &tmp) != 0)
	return false;
      break;
    case 1:
      if (ioctl(myFD, DIGITAL_GET_BANK1, &tmp) != 0)
	return false;
      break;
    case 2:
      if (ioctl(myFD, DIGITAL_GET_BANK2, &tmp) != 0)
	return false;
      break;
    case 3:
      if (ioctl(myFD, DIGITAL_GET_BANK3, &tmp) != 0)
	return false;
      break;
    default:
      return false;
  }

  // bitwise negate it because the hardware calls a 'high' off
  *val = ~tmp;
  return true;
}


/** Returns the state of the bits as they are currently set in the outputs.  It
 doesn't reconfirm with the hardware, but instead keeps track of how it last
 set them.
 @param bank which bank to check
 @param val the byte to write the values into
 @return true if the request was satisfied, false otherwise
 **/
MVREXPORT bool MvrVersalogicIO::getDigitalBankOutputs(int bank, unsigned char *val)
{
  if (!myEnabled)
    return false;

  if (bank < 0 || bank > myNumBanks - 1)
    return false;

  // should check for input vs. output
  switch (bank)
  {
    case 0:
      *val = myDigitalBank0;
      break;
    case 1:
      *val = myDigitalBank1;
      break;
    case 2:
      *val = myDigitalBank2;
      break;
    case 3:
      *val = myDigitalBank3;
      break;
  }

  return true;
}

/** The bits on the hardware side of the digital I/O ports are inverse-logic.
  The bit must be set high in the register for the output to be off, and be
  set low to be turned on.  This function negates that so that it is more
  intuitive.

  @param bank the bank number to use.  0 is the lowest bank
  @param val the status of the 8-bits in a single byte.  
 **/
MVREXPORT bool MvrVersalogicIO::setDigitalBankOutputs(int bank, unsigned char val)
{
  if (!myEnabled)
    return false;

  if (bank < 0 || bank > myNumBanks - 1)
    return false;

  // bitwise negate val because the hardware calls a 'high' off
  switch (bank)
  {
    case 0:
      if (ioctl(myFD, DIGITAL_SET_BANK0, ~val) != 0)
	return false;
      myDigitalBank0 = val;
      break;
    case 1:
      if (ioctl(myFD, DIGITAL_SET_BANK1, ~val) != 0)
	return false;
      myDigitalBank1 = val;
      break;
    case 2:
      if (ioctl(myFD, DIGITAL_SET_BANK2, ~val) != 0)
	return false;
      myDigitalBank2 = val;
      break;
    case 3:
      if (ioctl(myFD, DIGITAL_SET_BANK3, ~val) != 0)
	return false;
      myDigitalBank3 = val;
      break;
    default:
      return false;
  }

  return true;
}

/** The special_control_register contains various status bits, of which can
  be found in the manuals for the motherboards.  One interesting bit is the
  temperature bit, which gets set high if the CPU is over the specified threshold
  as set in the BIOS

 **/
MVREXPORT bool MvrVersalogicIO::getSpecialControlRegister(unsigned char *val)
{
  if (!myEnabled)
    return false;

  unsigned char tmp;
  if (ioctl(myFD, GET_SPECIAL_CONTROL_REGISTER, &tmp) != 0)
    return false;

  *val = tmp;
  return true;
}
