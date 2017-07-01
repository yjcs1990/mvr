#ifndef MVRMTXIO_H
#define MVRMTXIO_H


#include "mvriaTypedefs.h"
#include "MvrRobot.h"

/** @brief Interface to digital and analog I/O and switched power outputs on MTX
 * core (used in Pioneer LX and other MTX-based robots).
 
  On Linux this class uses the <code>mtx</code> driver to interface with the 
  MTX digital and analog IO, and control switched power outputs. 

  The <code>mtx</code> driver module must be loaded for this interface to work.

  The <code>mtx</code> interface (<code>/dev/mtx</code>) is opened automatically
  in the MvrMTXIO constructor.  If successful, isEnabled() will then return true.

  The MTX IO is organized as two sets of 8-bit "banks" of inputs, and two 8-bit
  "banks" of outputs.

  To read the state input, use getDigitalIOInputMon1() and
  getDigitalIOInputMon2(), or getDigitalBankInputs() with bank index 0 or 1.

  To set state of output, use setDigitalOutputControl1() and
  setDigitalOutputControl2(), or setDigitalBankOutputs() with bank index 2 or
  3.

  Multile MvrMTXIO objects may be instatiated; a shared may be
  locked and unlocked by calling lock() and unlock().

   @ingroup OptionalClasses
   @ingroup DeviceClasses
   @ingroup MTX

 @linuxonly

*/
 
class MvrMTXIO
{
public:

  enum Direction
  {
    DIGITAL_INPUT,
    DIGITAL_OUTPUT,
    INVALID
  };

  /// Constructor
  MVREXPORT MvrMTXIO(const char * dev = "/dev/mtx");
  /// Destructor
  MVREXPORT virtual ~MvrMTXIO(void);

  /// tries to close the device.  Returns false if operation failed
  MVREXPORT bool closeIO(void);

  /// returns true if the device is opened and operational
  bool isEnabled(void) { return myEnabled; }

  /// returns true if analog values are supported
  MVREXPORT bool isAnalogSupported(void) { return myAnalogEnabled; }

  /// @note not yet implemented.
  bool getAnalogValue(int port, double *val) { return true; }
  /// @note not yet implemented
  bool getAnalogValueRaw(int port, int *val) { return true; }

  /// Get/set value of digital IO banks (see class description above)
  /// Banks are 0-indexed.
  /// @{
  MVREXPORT Direction getDigitalBankDirection(int bank);
  MVREXPORT bool setDigitalBankOutputs(int bank, unsigned char val);
  MVREXPORT bool getDigitalBankInputs(int bank, unsigned char *val);
  MVREXPORT bool getDigitalBankOutputs(int bank, unsigned char *val);
  /// @}

  /// Set one bit of an output bank. @a bank and @a bit are 0-indexed.
  bool setDigitalOutputBit(int bank, int bit) {
    unsigned char val;
    if(!getDigitalBankOutputs(bank, &val))
      return false;
    return setDigitalBankOutputs( bank, val | (1 << bit) );
  }

  /// Get one bit of an input bank. @a bank and @a bit are 0-indexed.
  bool getDigitalInputBit(int bank, int bit) {
    unsigned char val = 0;
    getDigitalBankInputs(bank, &val);
    return val & (1 << bit);
  }

  /// get/set value of power output controls (see robot manual for information
  /// on what components and user outputs are controlled). Banks are 0-indexed.
  /// @{
  MVREXPORT bool setPeripheralPowerBankOutputs(int bank, unsigned char val);
  MVREXPORT bool getPeripheralPowerBankOutputs(int bank, unsigned char *val);
  /// @}

  /// Set one power output. @a bank and @a bit are 0-indexed.
  bool setPowerOutput(int bank, int bit, bool on) {
    unsigned char val = 0;
    if(!getPeripheralPowerBankOutputs(bank, &val))
      return false;
    if (on)
      return setPeripheralPowerBankOutputs(bank, val & (1 << bit));
    else
      return setPeripheralPowerBankOutputs(bank, val ^ (1 << bit));
  }

  /// Lock global (shared) mutex for all MvrMTXIO instances.
  /// This allows multiple access to MTX IO (through multiple MvrMTXIO objects).
  MVREXPORT int lock(void){ return(ourMutex.lock()); }
  /// Unlock global (shared) mutex for all MvrMTXIO instances.
  /// This allows multiple access to MTX IO (through multiple MvrMTXIO objects).
  MVREXPORT int unlock(void){ return(ourMutex.unlock()); }

  /// Try to lock without blocking (see MvrMutex::tryLock())
  MVREXPORT int tryLock() {return(ourMutex.tryLock());}

  /// gets the Firmware Revision 
  MVREXPORT unsigned char getFirmwareRevision()
	{ return myFirmwareRevision; }

  /// gets the Firmware Version 
  MVREXPORT unsigned char getFirmwareVersion()
	{ return myFirmwareVersion; }

  /// gets the Compatibility Code 
  MVREXPORT unsigned char getCompatibilityCode()
	{ return myCompatibilityCode; }

  /// gets the MTX FPGA Type
  MVREXPORT unsigned char getFPGAType()
	{ return myFPGAType; }

  /// gets the values of digital input/output monitoring registers 1 & 2
  /// @{
  MVREXPORT bool getDigitalIOInputMon1(unsigned char *val);
  MVREXPORT bool getDigitalIOInputMon2(unsigned char *val);
  MVREXPORT bool getDigitalIOOutputMon1(unsigned char *val);
  MVREXPORT bool getDigitalIOOutputMon2(unsigned char *val);
  /// @}

  /// get the Light Pole IO Output Control Register
  MVREXPORT bool getLightPole(unsigned char *val);
  /// sets the Light Pole IO Output Control Register
  MVREXPORT bool setLightPole(unsigned char *val);

  /// @internal
  MVREXPORT bool getLPCTimeUSec(MvrTypes::UByte4 *timeUSec);

  /// @internal
  MVREXPORT MvrRetFunctor1<bool, MvrTypes::UByte4 *> *getLPCTimeUSecCB(void)
    { return &myLPCTimeUSecCB; }

  /// gets/sets the Semaphore Registers
  /// @internal
  //@{
	MVREXPORT bool setSemaphore1(unsigned char *val);
  MVREXPORT bool getSemaphore1(unsigned char *val);
	MVREXPORT bool setSemaphore2(unsigned char *val);
  MVREXPORT bool getSemaphore2(unsigned char *val);
	MVREXPORT bool setSemaphore3(unsigned char *val);
  MVREXPORT bool getSemaphore3(unsigned char *val);
	MVREXPORT bool setSemaphore4(unsigned char *val);
  MVREXPORT bool getSemaphore4(unsigned char *val);
  //@}

  /// gets the bumper Input Monitoring Reg
  /// @internal
  MVREXPORT bool getBumperInput(unsigned char *val);

  /// gets the Power Status Register 
  /// @internal
  MVREXPORT bool getPowerStatus1(unsigned char *val);
  /// gets the Power Status Register 2
  /// @internal
  MVREXPORT bool getPowerStatus2(unsigned char *val);

  /// gets the LIDAR Safety Status Register
  MVREXPORT bool getLIDARSafety(unsigned char *val);

  /// gets the ESTOP status Registers
  /// @internal
  //@{
  MVREXPORT bool getESTOPStatus1(unsigned char *val);
  MVREXPORT bool getESTOPStatus2(unsigned char *val);
  MVREXPORT bool getESTOPStatus3(unsigned char *val);
  MVREXPORT bool getESTOPStatus4(unsigned char *val);
  /// Compares the high nibble of this byte against the passed in val, returns true if it matches
  MVREXPORT bool compareESTOPStatus4HighNibbleAgainst(int val);
  //@}

  /// gets/sets Digital IO Output Control Registers 1 &amp; 2
  //@{
  MVREXPORT bool getDigitalOutputControl1(unsigned char *val);
  MVREXPORT bool setDigitalOutputControl1(unsigned char *val);
  MVREXPORT bool getDigitalOutputControl2(unsigned char *val);
  MVREXPORT bool setDigitalOutputControl2(unsigned char *val);
  //@}

  /// gets/sets the Peripheral Power Control Regs 1 &amp; 2
  /// These control power to core and robot components, and to user/auxilliary
  /// power outputs.  Refer to robot manual for information on which components
  /// and outputs are controlled by which bits in the peripheral power bitmasks.
  //@{
  MVREXPORT bool getPeripheralPower1(unsigned char *val); ///< bank 0
  MVREXPORT bool setPeripheralPower1(unsigned char *val); ///< bank 0
  MVREXPORT bool getPeripheralPower2(unsigned char *val); ///< bank 1
  MVREXPORT bool setPeripheralPower2(unsigned char *val); ///< bank 1
  MVREXPORT bool getPeripheralPower3(unsigned char *val); ///< bank 2
  MVREXPORT bool setPeripheralPower3(unsigned char *val); ///< bank 2
  //@}

  /// gets the motion power status
  MVREXPORT bool getMotionPowerStatus(unsigned char *val);

  /// gets/sets the LIDAR Control Reg
  /// @internal
  //@{
  MVREXPORT bool getLIDARControl(unsigned char *val);
  MVREXPORT bool setLIDARControl(unsigned char *val);
  //@}


  /// gets analog Block 1 & 2
  //@{
  MVREXPORT bool getAnalogIOBlock1(int analog, unsigned short *val);
  MVREXPORT bool getAnalogIOBlock2(int analog, unsigned short *val);

  MVREXPORT bool setAnalogIOBlock2(int analog, unsigned short *val);
  //@}

  /// This returns a conversion of the bits to a decimal value,
  /// currently assumed to be in the 0-5V range
  /// @internal
  MVREXPORT bool getAnalogValue(double *val);

  /// @internal
  MVREXPORT bool getAnalogValueRaw(int *val);

protected:

  bool getLPCTimer0(unsigned char *val);
  bool getLPCTimer1(unsigned char *val);
  bool getLPCTimer2(unsigned char *val);
  bool getLPCTimer3(unsigned char *val);
	

  static MvrMutex ourMutex;
  int myFD;

  bool myEnabled;
  bool myAnalogEnabled;

	unsigned char myFirmwareRevision;
	unsigned char myFirmwareVersion;
	unsigned char myCompatibilityCode;
	unsigned char myFPGAType;

	struct MTX_IOREQ{
    unsigned short	myReg;
    unsigned short	mySize;
    union {
        unsigned int	myVal;
        unsigned int	myVal32;
        unsigned short myVal16;
        unsigned char	myVal8;
				//unsigned char myVal128[16];
    } myData;
	};

  int myNumBanks;

  unsigned char myDigitalBank1;
  unsigned char myDigitalBank2;
  unsigned char myDigitalBank3;
  unsigned char myDigitalBank4;

  MvrRetFunctorC<bool, MvrMTXIO> myDisconnectCB;
  MvrRetFunctor1C<bool, MvrMTXIO, MvrTypes::UByte4 *> myLPCTimeUSecCB;
};

//#endif // SWIG

#endif // MVRMTXIO_H
