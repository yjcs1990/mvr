/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrVersalogicIO.h
 > Description  : Interface to integrated digital and analog I/O interfaces on Versalogic 
                  VSBC8 and EBX-12 Cobra computers (digital IO on 2nd * gen.  PatrolBot)
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRVERSALOGICIO_H
#define MVRVERSALOGICIO_H

//#ifndef SWIG

#include "mvriaTypedefs.h"
#include "MvrRobot.h"

/*This class is a basic set of calls to use the Linux device driver, <code>amrio</code>,
  which reads and writes data to VersaLogic's Opto22 and analog interfaces.
  The <code>amrio</code> driver must be built into the Linux kernel or its
  module loaded. Contact MobileRobots for information about obtaining this
  driver.
  It currently supports the Versalogic VSBC-8d, VSBC-8k, and EBX12 (Cobra) motherboards.

  The digital portion of the Opto22 consists of two banks of 8 pins on the VSBC8
  and four banks of 8 pins on the EBX12.  Each bank can be set as either inputs
  or outputs.  The banks are zero-indexed, so bank0 is the first one.

  The analog inputs require a separate chip.  There are 8 inputs, only
  one of which can be read at a time.  It currently returns a value between
  0 and 4096 or a decimal value in the range of 0-5V.  The constructor will attempt
  an analog conversion, and if it fails will assume that the chip is not present
  and will disable the analog function.

  The SPECIAL_CONTROL_REGISTER contains a few bits of information, the one of
  importance at the moment is the CPU_OVERTEMPERATURE bit, which will be set
  high if the CPU temp is over the warning temp as set in the BIOS.  Bitwise
  AND the special_control_register output with 0x20 to find the temperature
  bit.

  The destructor closes the device, so just delete the MvrVersalogicIO instance
  to close the device.

  @ingroup OptionalClasses
   @ingroup DeviceClasses
*/
 
class MvrVersalogicIO
{
public:

  enum Direction
  {
    DIGITAL_INPUT,
    DIGITAL_OUTPUT
  };

  /// Constructor
  MVREXPORT MvrVersalogicIO(const char * dev = "/dev/amrio");
  /// Destructor
  MVREXPORT virtual ~MvrVersalogicIO(void);

  /// tries to close the device.  Returns false if operation failed
  MVREXPORT bool closeIO(void);

  /// returns true if the device is opened and operational
  MVREXPORT bool isEnabled(void) { return myEnabled; }

  /// returns true if analog values are supported
  MVREXPORT bool isAnalogSupported(void) { return myAnalogEnabled; }

  /// Take an analog reading from a port number from 0-7.
  /// This returns a conversion of the bits to a decimal value,
  /// currently assumed to be in the 0-5V range
  MVREXPORT bool getAnalogValue(int port, double *val);

  /// Take an analog reading from a port number from 0-7.
  /// This returns the actual reading from the chip, which is 12-bits
  MVREXPORT bool getAnalogValueRaw(int port, int *val);

  /// returns the direction (input or output) for the given bank
  MVREXPORT Direction getDigitalBankDirection(int bank);

  /// set direction for a particular digital I/O bank
  MVREXPORT bool setDigitalBankDirection(int bank, Direction dir);

  /// get the current value of the digital inputs on a certain bank
  MVREXPORT bool getDigitalBankInputs(int bank, unsigned char *val);

  /// get the current value of the digital outputs bits on a certain bank
  MVREXPORT bool getDigitalBankOutputs(int bank, unsigned char *val);

  /// set the value of the digital outputs bits
  MVREXPORT bool setDigitalBankOutputs(int bank, unsigned char val);

  /// gets the special register of the motherboard.
  MVREXPORT bool getSpecialControlRegister(unsigned char *val);

  /// lock the amrio device instance
  MVREXPORT int lock(void){ return(myMutex.lock()); }
  /// unlock the amrio device instance
  MVREXPORT int unlock(void){ return(myMutex.unlock()); }

  /// Try to lock the device instance without blocking
  MVREXPORT int tryLock() {return(myMutex.tryLock());}

protected:

  static MvrMutex myMutex;
  int myFD;

  bool myEnabled;
  bool myAnalogEnabled;

  int myNumBanks;

  unsigned char myDigitalBank0;
  unsigned char myDigitalBank1;
  unsigned char myDigitalBank2;
  unsigned char myDigitalBank3;

  MvrRetFunctorC<bool, MvrVersalogicIO> myDisconnectCB;
};

//#endif // SWIG

#endif // MVRVERSALOGICIO_H