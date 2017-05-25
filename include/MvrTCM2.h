/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrTCM2.h
 > Description  :  Interface to the PNI TCM 2, TCM 2.5, and TCM 2.6  3-axis compass (magnetometer)
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/

#ifndef ARTCM2_H
#define ARTCM2_H

#include "mvriaUtil.h"
#include "MvrFunctor.h"
#include "MvrRobot.h"


#ifdef WIN32
#define ARTCM2_DEFAULT_SERIAL_PORT "COM4"
#else
#define ARTCM2_DEFAULT_SERIAL_PORT "/dev/ttyS3"
#endif

class MvrTCM2
{
public:
  
  AREXPORT MvrTCM2();
  virtual ~MvrTCM2() {}

  /** If a connection/initialization procedure is required, perform it, and
      return true on success, false on failure. Otherwise, just return true.
  */
  AREXPORT virtual bool connect();

  /** If a connection/initialization procedure is required, perform it, wait
   * until data is recieved from the compass, and
      return true on success, false on failure. Otherwise, just return true.
  */
  AREXPORT virtual bool blockingConnect(unsigned long connectTimeout = 5000);


  /// Get the compass heading (-180, 180] degrees
  double getHeading(void) const { return myHeading; }
  bool haveHeading() const { return myHaveHeading; }

  /** Get the compass heading (-180, 180] degrees
   *  @deprecated Use getHeading()
   */
  double getCompass(void) const { return getHeading(); }

  /// Get the pitch (-180,180] degrees.
  double getPitch(void) const { return myPitch; }
  bool havePitch() const { return myHavePitch; }

  /// Get the roll (-180,180] degrees.
  double getRoll(void) const { return myRoll; }
  bool haveRoll() const { return myHaveRoll; }

  /// Get the magnetic field X component (See TCM2 Manual)
  double getXMagnetic(void) const { return myXMag; }
  bool haveXMagnetic() const { return myHaveXMag; }

  /// Get the magnetic field Y component (See TCM2 Manual)
  double getYMagnetic(void) const { return myYMag; }
  bool haveYMagnetic() const { return myHaveYMag; }

  /// Get the magnetic field Z component (See TCM2 Manual)
  double getZMagnetic(void) const { return myZMag; }
  bool haveZMagnetic() const { return myHaveZMag; }

  /// Get the temperature (degrees C)
  double getTemperature(void) const { return myTemperature; }
  bool haveTemperature() const { return myHaveTemperature; }

  // Get last error code (see TCM manual) recieved. 0 means no error recieved.
  int getError(void) const { return myError; }

  /// Get the calibration H score (See TCM Manual)
  double getCalibrationH(void) const { return myCalibrationH; }
  bool haveCalibrationH() const { return myHaveCalibrationH; }

  /// Get the calibration V score (See TCM Manual)
  double getCalibrationV(void) const { return myCalibrationV; }
  bool haveCalibrationV() const { return myHaveCalibrationV; }

  /// Get the calibration M score (See TCM Manual)
  double getCalibrationM(void) const { return myCalibrationM; }
  bool haveCalibrationM() const { return myHaveCalibrationM; }

  /// Turn sending of data off (Compass remains powered on, this is not its not low power standby mode)
  virtual void commandOff(void) = 0;

  /// Get one reading from the compass
  virtual void commandOnePacket(void) = 0;

  /// Start the compass sending a continuous stream of readings at its fastest rate
  virtual void commandContinuousPackets(void) = 0;

  /// Start user calibration
  virtual void commandUserCalibration(void) = 0;

  /// Start auto calibration
  virtual void commandAutoCalibration(void) = 0;

  /// Stop calibration
  virtual void commandStopCalibration(void)  = 0;

  /// Command to do a soft reset of the compass
  virtual void commandSoftReset(void) = 0;

  /// Command to just send compass heading data
  virtual void commandJustCompass(void) = 0;

  /// Gets the number of readings recieved in the last second
  int getPacCount(void) {
    if(myTimeLastPacket == time(NULL)) return myPacCount;
    if(myTimeLastPacket == time(NULL) - 1) return myPacCurrentCount;
    return 0;
  }

  // Add a callback to be invoked when a new heading is recieved
  void addHeadingDataCallback(MvrFunctor1<double> *f) {
    myHeadingDataCallbacks.push_back(f);
  }


protected:  
  double myHeading;
  double myPitch;
  double myRoll;
  double myXMag;
  double myYMag;
  double myZMag;
  double myTemperature;
  int myError;
  double myCalibrationH;
  double myCalibrationV;
  double myCalibrationM;

  bool myHaveHeading;
  bool myHavePitch;
  bool myHaveRoll;
  bool myHaveXMag;
  bool myHaveYMag;
  bool myHaveZMag;
  bool myHaveTemperature;
  bool myHaveCalibrationH;
  bool myHaveCalibrationV;
  bool myHaveCalibrationM;

  std::list< MvrFunctor1<double>* > myHeadingDataCallbacks;

  // packet count
  time_t myTimeLastPacket;
  int myPacCurrentCount;
  int myPacCount;		

  void incrementPacketCount() {
    if (myTimeLastPacket != time(NULL)) 
    {
      myTimeLastPacket = time(NULL);
      myPacCount = myPacCurrentCount;
      myPacCurrentCount = 0;
    }
    myPacCurrentCount++;
  }


  // call the heading data callbacks
  void invokeHeadingDataCallbacks(double heading) {
    for(std::list<MvrFunctor1<double>*>::iterator i = myHeadingDataCallbacks.begin(); i != myHeadingDataCallbacks.end(); ++i)
      if(*i) (*i)->invoke(heading);
  }
  
};


/** Use this class to create an instance of a TCM2 subclass
 * and connect to the device based on program command line options. 
 * This allows the user of a program to select a different kind
 * of compass configuration (for example, if the compass is connected
 * to a computer serial port, use MvrTCMCompassDirect instead of
 * the normal MvrTCMCompassRobot.)
 */

class MvrCompassConnector
{
protected:
  MvrArgumentParser *myArgParser;
  MvrRetFunctorC<bool, MvrCompassConnector> myParseArgsCallback;
  MvrFunctorC<MvrCompassConnector> myLogArgsCallback;
  typedef enum {
    Robot,
    SerialTCM,
    None
  } DeviceType;
  DeviceType myDeviceType;
  const char *mySerialPort;
  AREXPORT bool parseArgs();
  MvrFunctor *mySerialTCMReadFunctor;
  MvrRobot *myRobot;
  AREXPORT void logOptions();
public:
  AREXPORT MvrCompassConnector(MvrArgumentParser *argParser);
  AREXPORT ~MvrCompassConnector();
  AREXPORT MvrTCM2 *create(MvrRobot *robot);
  AREXPORT bool connect(MvrTCM2*) const;
};

#endif // ARTCM2_H