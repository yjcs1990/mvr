#ifndef MVRANALOGGYRO_H
#define MVRANALOGGYRO_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"

class MvrRobot;
class MvrRobotPacket;

/// Use onboard gyro to improve the heading in an MvrRobot object's pose value
/**
   The gyro is an accessory connected to the robot's microcontroller.
   The gyro's purpose is to improve large errors due to wheel
   slippage, wheels coming off the ground, and other factors like this
   (or for the skid steer of an AT). The gyro also happens to provide
   temperature readings as well.

   When an MvrAnalogGyro object is created, it registers callbacks with the
   robot, and automatic heading correction will begin when new data arrives
   from the robot.
   If you delete the object, it will remove itself from the robot.  
   
   The readings come back from the gyro over the robot microcontroller's analog ports.  
   The microcontroller then sends the readings to us just before
   sending us the standard packet.  MvrAnalogGyro uses these  readings to
   calculate a better heading of the robot (integrating the velocities to
   give position).  Then when the standard packet comes in, MvrRobot calls
   an encoder correction callback in MvrAnalogGyro which does a simple 
   Kalman filter and fuses the encoder information's heading with the gyro's heading
   to compute the most probable heading which it then returns to the
   MvrRobot object to use as its heading.

   The robot's normal dead reconing angle is fairly good if you have properly
   inflated tires (if you have pneumatic tires) and if you have
   the revcount parameter sent correctly.  See the robot operation manual for
   how to change the revcount parameter.

   Gyro readings are affected by temperature (a temperature value 
   is reported along with the gyro data in the packets from the
   microcontroller).  The gyro will auto calibrate itself to the
   center of the range (which may have drifted due to temperature)
   whenever the robot is stationary for more than one second (that is, if its
   translational and rotational velocities are less than 1 
   and if the gyro readings are also within .5% of the average).  The
   scaling factor (between change of voltage and amount of turn)
   doesn't seem to change due to temperature, though it can vary among
   different gyro devices.  (This scaling factor can be accessed with
   getScalingFactor() and setScalingFactor().)
   The default value used here was within 3% of correct most of the time.  
   The default resides in the parameter file for the robot that is loaded or used when a
   connection is made.  If you want to tune this more you can, by
   finding a different scale factor.  (There is just one so there
   doesn't have to be alot of calibration in flash that is lost if
   parameters are loaded from a file.)  
   
   General notes on gyros and inertial correction: The purpose of the gyro is to
   correct for wheel slippage and large errors. It is not to totally correct
   for all errors, but to bring the errors into smaller zones that
   software can correct for more easily (such as by localization).
   We have found that if gyros or inertial measurement devices are used to attempt to
   correct for all error, the result can be not very robust, and include
   many problematic assumptions. For this reason, we have found that
   it is best to integrate this one simple gyro to simply improve robot odometry, 
   rather than a complicated and expensive IMU.

   @ingroup OptionalClasses

*/
class MvrAnalogGyro
{
public:
  /// Constructor
  MVREXPORT MvrAnalogGyro(MvrRobot *robot);
  /// Destructor
  MVREXPORT virtual ~MvrAnalogGyro();
  /// Gets if we really have a gyro or not
  MVREXPORT bool isActive(void) { return myIsActive; }
  /// Lets the gyro correct readings
  MVREXPORT void activate(void);
  /// Stops the gyro from correcting readings (still accumulates)
  MVREXPORT void deactivate(void);
  /// If we have a gyro only mode
  bool hasGyroOnlyMode(void) { return myHasGyroOnlyMode; }
  /// If we're using gyro only mode
  bool isGyroOnlyActive(void) { return myIsGyroOnlyActive; }
  /// Activates it and puts it in gyro only mode
  MVREXPORT void activateGyroOnly(void);
  /// If this class actually has data or not (if it has no data, the
  /// robot is all there is)
  bool hasNoInternalData(void) { return myHasNoData; }
  /// Returns true if any amount of gyro data has yet been received, false if no readings have yet been received from the robot.
  MVREXPORT bool haveGottenData(void) { return myHaveGottenData; }
  /// Gets a heading calculated from past gyro readings
  MVREXPORT double getHeading(void) const { return myHeading; }
  /// Gets the temperature the gyro has
  MVREXPORT int getTemperature(void) const { return myTemperature; }
  /// Set the parameters of the Kalman filter model
  /**
     @param gyroSigma the amount its off statically

     @param inertialVar the proportional amount it is off
     
     @param rotVar the amount the rotation is off proportionally
     
     @param transVar the amount the translation throws off the heading
     proportionally
  **/
  MVREXPORT void setFilterModel(double gyroSigma, double inertialVar, 
				       double rotVar, double transVar)
    { myGyroSigma = gyroSigma; myInertialVmvrianceModel = inertialVar;
      myRotVmvrianceModel = rotVar; myTransVmvrianceModel = transVar; };

  /// Returns the number of readings taken in the last second
  MVREXPORT int getPacCount(void) { return myPacCount; }

  /// Gets the most recently calculated average rotational velocity (over one
  //second)
  MVREXPORT double getAverage(void) const { return myLastAverage; }
  /// Gets the time the last average was taken 
  MVREXPORT MvrTime getAverageTaken(void) const { return myLastAverageTaken; }
  /// Gets the scaling factor used for multiplying the reading values received (default 1.626)
  MVREXPORT double getScalingFactor(void) const { return myScalingFactor; }
  /// Sets the scaling factor used for multiplying the readings
  MVREXPORT void setScalingFactor(double factor) { myScalingFactor = factor; }

  /// Internal packet handler for the gyro packets
  MVREXPORT bool handleGyroPacket(MvrRobotPacket *packet);
  /// internal function for correcting the encoder readings with the gyro data
  MVREXPORT double encoderCorrect(MvrPoseWithTime deltaPose);
  /// Internal connection callback; delays for a short amount of time to give the gyro enough time to stabilize before we try to use it
  MVREXPORT void stabilizingCallback(void);
  /// Internal user task callback
  MVREXPORT void userTaskCallback(void);
  /// Sets whether we log anomalies or not (temporary function for debugging)
  void setLogAnomalies(bool logAnomalies) { myLogAnomalies = logAnomalies; }
protected:
  // whether we're correcting readings or not
  bool myIsActive;
  // whether we're really getting readings or not
  bool myHaveGottenData;
  // our double for our scaling factor
  double myScalingFactor;
  // if we've gotten a reading this cycle
  bool myReadingThisCycle;
  // whether we're logging anomalies or not
  bool myLogAnomalies;
  // counting data
  int myPacCount;
  int myPacCurrentCount;
  time_t myTimeLastPacket;

  // data for averaging
  MvrTime myAverageStarted;
  double myLastAverage;
  MvrTime myLastAverageTaken;
  double myAverageTotal;
  int myAverageCount;


  // constants for kalman filtering
  double myGyroSigma;
  double myInertialVmvrianceModel;
  double myRotVmvrianceModel; // deg2/deg
  double myTransVmvrianceModel; // deg2/meter

  // for if our gyro packets aren't aligned with our sips how much
  // we've changed since we last got to correct one
  double myAccumulatedDelta;

  double myHeading;
  int myTemperature;
  double myLastHeading;
  MvrRobot *myRobot;
  MvrRetFunctor1C<bool, MvrAnalogGyro, MvrRobotPacket *> myHandleGyroPacketCB;
  MvrRetFunctor1C<double, MvrAnalogGyro, MvrPoseWithTime> myEncoderCorrectCB;
  MvrFunctorC<MvrAnalogGyro> myStabilizingCB;
  MvrFunctorC<MvrAnalogGyro> myUserTaskCB;

  /// Gyro type
  enum GyroType
  {
    GYRO_NONE, ///< No gyro
    GYRO_ANALOG_COMPUTER, ///< Analog gyro used by computer
    GYRO_ANALOG_CONTROLLER ///< Analog gyro used by microcontroller
  };
  GyroType myGyroType;
  bool myHasNoData;
  bool myHasGyroOnlyMode;
  bool myIsGyroOnlyActive;
  
  bool myGyroWorking;
};

#endif // ARANALOGGYRO_H


