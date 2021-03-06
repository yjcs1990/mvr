#ifndef MVRRANGEDEVICELASER_H
#define MVRRANGEDEVICELASER_H

#include "mvriaTypedefs.h"
#include "MvrRangeDeviceThreaded.h"

class MvrDeviceConnection;



/** MvrRangeDevice interface specialized for laser rangefinder sensors; see
 * MvrRangeDevice for more data access methods.

   This class is a subclass of MvrRangeDeviceThreaded meant for any
   planar scanning lasers, like the SICK lasers, Hokoyo URG series
   lasers, etc.  Unlike other base classes this contains the superset
   of everything that may need to be configured on any of the sensors,
   even though some subclasses may only provide some of those parameters
   and features. (This allows the configuration interfaces and parameter
   files to work for any laser type.)

   Normally, a program does not define or create any MvrLaser objects
   directly. Instead,
   objects (device-specific subclasses of MvrLaser) are created by MvrLaserConnector::connectLasers()
   based on the robot parameter file and command line arguments, and
   the resulting MvrLaser objects are stored in the MvrRobot object
   Use MvrRobot::getLaserMap() or MvrRobot::findLaser() to access the MvrLaser
   objects after calling MvrLaserConnector::connectLasers(). 

   The
   canSetDegrees(), canChooseRange(), canSetIncrement(),
   canChooseIncrement(), canChooseUnits(), canChooseReflectorBits(),
   canSetPowerControlled(), canChooseStartBaud(), and canChooseAutoBaud()  and
other similar functions are used by MvrLaserConnector to test if a parameter
   is relevant to a specific laser type.


   @par Creating New MvrLaser Subclasses
   If you want to create your own new class to implement a sensor
   not in ARIA, create a subclass of this class.  MvrUrg
   is the best current example of this.
   Call the laserAllow* 
   functions in its constructor depending on what features
   that laser sensor has (use the laserAllowSet* functions if it is possible
   to set any value in a range, or use the laserAllow*Choices functions if it
   is only possible to set specific values).
   You may also want to use the override the laserSetName
   call so that your own mutexes will get named appropriately.  You
   can use laserSetDefaultTcpPort to set the default TCP port (which
   you should do if the laser normally is connected to over TCP).  You
   can use laserSetDefaultPortType to the type of port normally used
   (so that if a port is passed in we can make a reasonable guess as
   to what type, so that people don't always have to pass in a type).
   Before you connect you should call laserPullUnsetParams to fill in
   the parameters that weren't set explicitly with the default ones in
   the .p files.  If the maximum range of the laser changes depending
   on settings (like on the LMS2xx) you should call
   laserSetAbsoluteMaxRange (after you call
   laserPullUnsetParamsFromRobot).  Implement the blockingConnect,
   asyncConnect, disconnect, isConnected, and isTryingToConnect
   calls... while calling laserConnect, laserFailedConnect,
   laserDisconnectNormally, and laserDisconnectOnError calls to let
   the laser base class call the appropriate callbacks.  You should
   use the laserCheckLostConnection in your runThread to see if the
   laser has lost connection (see its documentation for details).
   Then after you get your readings call laserProcessReadings to fill
   them from the raw buffer into the current and cumulative buffer
   (and call the reading callbacks).  The internal* calls are all
   internal to the base class and shouldn't have to be used by
   inheriting classes.  After you have created a new laser type you
   can add it to Mvr by using the Mvria::laserAddCreator call and then
   it will work like any of the built in laser types (see the
   documentation for that function for what the creator needs to do,
   and an example is MvrLaserCreatorHelper in the mvriaUtil.h/cpp
   files).  Similiarly if you need a new connection type (ie not
   serial or tcp) you implement them and then use
   Mvria::deviceConnectionAddCreator call to add new types so they too
   will then be treated the same as the built in Mvr ones
   (MvrDeviceConnectionCreatorHelper in mvriaUtil.h/cpp file is the
   example for that one).

   @since 2.7.0
   @ingroup ImportantClasses
   @ingroup DeviceClasses
**/

class MvrLaser : public MvrRangeDeviceThreaded
{
public:
  /// Constructor
  MVREXPORT MvrLaser(int laserNumber, 
		   const char *name, 
		   unsigned int absoluteMaxRange,
		   bool locationDependent = false,
		   bool appendLaserNumberToName = true);
  /// Destructor
  MVREXPORT virtual ~MvrLaser();

  /// Connect to the laser and block for the result
  MVREXPORT virtual bool blockingConnect(void) = 0;
  /// Connect to the laser without blocking
  MVREXPORT virtual bool asyncConnect(void) = 0;
  /// Disconnect from the laser
  MVREXPORT virtual bool disconnect(void) = 0;
  /// See if the laser is connected
  MVREXPORT virtual bool isConnected(void) = 0;  
  /// See if the laser is trying to connect
  MVREXPORT virtual bool isTryingToConnect(void) = 0;  
  
  /// Sets the numter of seconds without a response until connection assumed lost
  MVREXPORT virtual void setConnectionTimeoutSeconds(double seconds);
  /// Gets the number of seconds without a response until connection assumed lost
  MVREXPORT virtual double getConnectionTimeoutSeconds(void);

  /// Gets the time data was last receieved
  MvrTime getLastReadingTime(void) { return myLastReading; }
  
  /// Gets the number of laser readings received in the last second
  MVREXPORT int getReadingCount(void);

  /// Sets the device connection
  MVREXPORT virtual void setDeviceConnection(MvrDeviceConnection *conn);
  /// Gets the device connection
  MVREXPORT virtual MvrDeviceConnection *getDeviceConnection(void);

  /// Sets the position of the sensor on the robot
  MVREXPORT void setSensorPosition(double x, double y, double th, double z = 0);
  /// Sets the position of the sensor on the robot
  MVREXPORT void setSensorPosition(MvrPose pose, double z = 0);  
  /// Gets if the sensor pose has been set 
  bool hasSensorPosition(void) { return myHaveSensorPose; }
  /// Gets the position of the sensor on the robot
  MvrPose getSensorPosition(void) { return mySensorPose; }
  /// Gets the X position of the sensor on the robot
  double getSensorPositionX(void) { return mySensorPose.getX(); }
  /// Gets the Y position of the sensor on the robot
  double getSensorPositionY(void) { return mySensorPose.getY(); }
  /// Gets the Z position of the sensor on the robot (0 is unknown)
  double getSensorPositionZ(void) { return mySensorZ; }
  /// Gets the heading of the sensor on the robot
  double getSensorPositionTh(void) { return mySensorPose.getTh(); }

  /// Gets the number of the laser this is
  int getLaserNumber(void) { return myLaserNumber; }
  
  /// Sets the log level that informational things are logged at
  void setInfoLogLevel(MvrLog::LogLevel infoLogLevel) 
    { myInfoLogLevel = infoLogLevel; }
  /// Gets the log level that informational things are logged at
  MvrLog::LogLevel getInfoLogLevel(void) 
    { return myInfoLogLevel; }

  /// Cumulative readings that are this close to current beams are discarded
  void setCumulativeCleanDist(double dist)
    {
      myCumulativeCleanDist = dist;
      myCumulativeCleanDistSquared = dist * dist;
    }
  /// Cumulative readings that are this close to current beams are discarded
  double getCumulativeCleanDist(void)
    { 
      return myCumulativeCleanDist; 
    }
  /// Cumulative readings are cleaned every this number of milliseconds
  void setCumulativeCleanInterval(int milliSeconds)
    {
      myCumulativeCleanInterval = milliSeconds;
    }
  /// Cumulative readings are cleaned every this number of milliseconds
  int getCumulativeCleanInterval(void)
    {
      return myCumulativeCleanInterval;
    }
  /// Offset for cumulative cleaning 
  void setCumulativeCleanOffset(int milliSeconds)
    {
      myCumulativeCleanOffset = milliSeconds;
    }
  /// Gets the offset for cumulative cleaning
  int getCumulativeCleanOffset(void)
    {
      return myCumulativeCleanOffset;
    }
  /// Resets when the cumulative cleaning happened (so offset can help)
  void resetLastCumulativeCleanTime(void)
    {
      myCumulativeLastClean.setToNow();
      myCumulativeLastClean.addMSec(myCumulativeCleanOffset);
    }

  /// Adds a series of degree at which to ignore readings (within 1 degree of nearest integer)
  /// @arg ignoreReadings a string containing a space- or comma-separated list of angles or angle ranges.
  ///   Angle ranges are two separated by a '-'.  Negative angles are also indicated
  ///   with '-' as well.  Ranges should be in the form <i>lower</i>-<i>higher</i> including
  ///   on the negative side.  For example, "-93--91,42,91-93"
  ///   means the range from -93 to -91, the single degree at 42, and the range
  ///   from 91 to 93 degrees.  0 is the center of the laser's forward field of view.
  /// @sa addIgnoreReading()
  /// @sa clearIgnoreReadings()
  /// @sa setStartDegrees()
  /// @sa setEndDegrees()
  MVREXPORT bool addIgnoreReadings(const char *ignoreReadings); 
  /// Adds a degree at which to ignore readings (within 1 degree of nearest integer)
  /// @sa clearIgnoreReadings()
  /// @sa setStartDegrees()
  /// @sa setEndDegrees()
  void addIgnoreReading(double ignoreReading)
    { myIgnoreReadings.insert(MvrMath::roundInt(ignoreReading)); }
  /// Clears the degrees we ignore readings at
  void clearIgnoreReadings(void) 
    { myIgnoreReadings.clear(); }
  /// Gets the list of readings that we ignore
  const std::set<int> *getIgnoreReadings(void) const
    { return &myIgnoreReadings; }
  
  /// Gets if the laser is flipped or not
  bool getFlipped(void) { return myFlipped; }
  /// Sets if the laser is flipped or not
  bool setFlipped(bool flipped) { myFlipped = flipped; myFlippedSet = true; return true; }

  /// Gets the default TCP port for the laser
  int getDefaultTcpPort(void) { return myDefaultTcpPort; }

  /// Gets the default port type for the laser
  const char *getDefaultPortType(void) { return myDefaultPortType.c_str(); }

  /// Indicates whether it is possible to set the specific start and stop angles of sensing (field of view)
  /**
     Gets if this class can set the start and end degrees with doubles.
     
     If so, you can use getStartDegreesMin and getStartDegreesMax to see
     the valid values that you can use with setStartDegrees (and see
     what was set with getStartDegrees), and getEndDegreesMin and
     getEndDegreesMax to see the valid values that you can use with
     setEndDegrees (and see what was set with getEndDegrees).

  **/
  bool canSetDegrees(void) { return myCanSetDegrees; }

  /// Gets the minimum value for the start angle
  /** @see canSetDegrees **/
  double getStartDegreesMin(void) { return myStartDegreesMin; }
  /// Gets the maximum value for the start angle
  /** @see canSetDegrees **/
  double getStartDegreesMax(void) { return myStartDegreesMax; }
  /// Gets the start angle
  /** @see canSetDegrees **/
  double getStartDegrees(void) { return myStartDegrees; }
  /// Sets the start angle, it must be between getStartDegreesMin and getStartDegreesMax
  /** @see canSetDegrees **/
  MVREXPORT bool setStartDegrees(double startDegrees);
  /// Gets the minimum value for the end angle
  /** @see canSetDegrees **/
  double getEndDegreesMin(void) { return myEndDegreesMin; }
  /// Gets the maximum value for the end angle
  /** @see canSetDegrees **/
  double getEndDegreesMax(void) { return myEndDegreesMax; }
  /// Gets the end angle
  /** @see canSetDegrees **/
  double getEndDegrees(void) { return myEndDegrees; }
  /// Sets the end angle, it must be between getEndDegreesMin and getEndDegreesMax
  /** @see canSetDegrees **/
  MVREXPORT bool setEndDegrees(double endDegrees);


  /**
     Gets if you can choose the number of degrees

     If so, you can call chooseDegrees with one of the strings from
     getDegreesChoices, and get the degrees chosen as a string with 
     getDegreesChoice or get the degrees chosen as a double with
     getDegreesChoiceDouble.
  **/
  bool canChooseDegrees(void) { return myCanChooseDegrees; }
  /// Gets the list of range choices
  /** @see canChooseDegrees **/
  std::list<std::string> getDegreesChoices(void) 
    { return myDegreesChoicesList; }
  /// Gets a string with the list of degrees choices seperated by |s 
  /** @see canChooseDegrees **/
  const char *getDegreesChoicesString(void) 
    { return myDegreesChoicesString.c_str(); }
  /// Sets the range to one of the choices from getDegreesChoices
  /** @see canChooseDegrees **/
  MVREXPORT bool chooseDegrees(const char *range);
  /// Gets the range that was chosen
  /** @see canChooseDegrees **/
  const char *getDegreesChoice(void) 
    { return myDegreesChoice.c_str(); }
  /// Gets the range that was chosen as a double
  /** @see canChooseDegrees **/
  double getDegreesChoiceDouble(void) { return myDegreesChoiceDouble; }
  /// Gets the map of degrees choices to what they mean 
  /** 
      This is mostly for the simulated laser
      @see canChooseDegrees 
      @internal 
  **/
  std::map<std::string, double> getDegreesChoicesMap(void) 
    { return myDegreesChoices; }


  /**
     Gets if you can set an increment
     
  **/
  bool canSetIncrement(void) { return myCanSetIncrement; }
  /// Gets the increment minimum
  /** @see canSetIncrement **/
  double getIncrementMin(void) { return myIncrementMin; }
  /// Gets the increment maximum
  /** @see canSetIncrement **/
  double getIncrementMax(void) { return myIncrementMax; }
  /// Gets the increment
  /** @see canSetIncrement **/
  double getIncrement(void) { return myIncrement; }
  /// Sets the increment
  /** @see canSetIncrement **/
  MVREXPORT bool setIncrement(double increment);

  /**
     Gets if you can choose an increment.

     If so, call chooseIncrement with one of the choices in
     getIncrementChoices, and get the choice as a string with
     getIncrementChoice or the choice as a string with
     getIncrementChoiceDouble.
  **/
  bool canChooseIncrement(void) { return myCanChooseIncrement; }
  /// Gets the list of increment choices 
  /** @see canChooseIncrement **/
  std::list<std::string> getIncrementChoices(void) 
    { return myIncrementChoicesList; }
  /// Gets a string with the list of increment choices seperated by |s 
  /** @see canChooseIncrement **/
  const char *getIncrementChoicesString(void) 
    { return myIncrementChoicesString.c_str(); }
  /// Sets the increment to one of the choices from getIncrementChoices
  /** @see canChooseIncrement **/
  MVREXPORT bool chooseIncrement(const char *increment);
  /// Gets the increment that was chosen
  /** @see canChooseIncrement **/
  const char *getIncrementChoice(void) { return myIncrementChoice.c_str(); }
  /// Gets the increment that was chosen as a double
  /** @see canChooseIncrement **/
  double getIncrementChoiceDouble(void) { return myIncrementChoiceDouble; }
  /// Gets the map of increment choices to what they mean
  /** 
      This is mostly for the simulated laser
      @see canChooseIncrement 
      @internal 
  **/
  std::map<std::string, double> getIncrementChoicesMap(void) 
    { return myIncrementChoices; }

  /**
     Gets if you can choose units for the laser.

     If so, call chooseUnits with one of the choices in
     getUnitsChoices, and see what the choice was with
     getUnitsChoice.
  **/
  bool canChooseUnits(void) { return myCanChooseUnits; }
  /// Gets the list of units choices 
  /** @see canChooseUnits **/
  std::list<std::string> getUnitsChoices(void) 
    { return myUnitsChoices; }
  /// Gets a string with the list of units choices seperated by |s 
  /** @see canChooseUnits **/
  const char *getUnitsChoicesString(void) 
    { return myUnitsChoicesString.c_str(); }
  /// Sets the units to one of the choices from getUnitsChoices
  /** @see canChooseUnits **/
  MVREXPORT bool chooseUnits(const char *units);
  /// Gets the units that was chosen
  /** @see canChooseUnits **/
  const char *getUnitsChoice(void) { return myUnitsChoice.c_str(); }

  /**
     Gets if you can choose reflectorBits for the laser.

     If so, call chooseReflectorBits with one of the choices in
     getReflectorBitsChoices, and see what the choice was with
     getReflectorBitsChoice.
  **/
  bool canChooseReflectorBits(void) { return myCanChooseReflectorBits; }
  /// Gets the list of reflectorBits choices 
  /** @see canChooseReflectorBits **/
  std::list<std::string> getReflectorBitsChoices(void) 
    { return myReflectorBitsChoices; }
  /// Gets a string with the list of reflectorBits choices seperated by |s 
  /** @see canChooseReflectorBits **/
  const char *getReflectorBitsChoicesString(void) 
    { return myReflectorBitsChoicesString.c_str(); }
  /// Sets the reflectorBits to one of the choices from getReflectorBitsChoices
  /** @see canChooseReflectorBits **/
  MVREXPORT bool chooseReflectorBits(const char *reflectorBits);
  /// Gets the reflectorBits that was chosen
  /** @see canChooseReflectorBits **/
  const char *getReflectorBitsChoice(void) { return myReflectorBitsChoice.c_str(); }

  /**
     Gets if you can set powerControlled for the laser.

     If so, call setPowerControlled to set if the power is being controlled
     or not, and see what the setting is with getPowerControlled.
  **/
  bool canSetPowerControlled(void) { return myCanSetPowerControlled; }
  /// Sets if the power is controlled 
  /** @see canChoosePowerControlled **/
  MVREXPORT bool setPowerControlled(bool powerControlled);
  /// Gets if the power is controlled
  /** @see canChoosePowerControlled **/
  bool getPowerControlled(void) { return myPowerControlled; }

  /**
     Gets if you can choose startingBaud for the laser.

     If so, call chooseStartingBaud with one of the choices in
     getStartingBaudChoices, and see what the choice was with
     getStartingBaudChoice.
  **/
  bool canChooseStartingBaud(void) { return myCanChooseStartingBaud; }
  /// Gets the list of reflectorBits choices 
  /** @see canChooseStartingBaud **/
  std::list<std::string> getStartingBaudChoices(void) 
    { return myStartingBaudChoices; }
  /// Gets a string with the list of reflectorBits choices seperated by |s 
  /** @see canChooseStartingBaud **/
  const char *getStartingBaudChoicesString(void) 
    { return myStartingBaudChoicesString.c_str(); }
  /// Sets the reflectorBits to one of the choices from getStartingBaudChoices
  /** @see canChooseStartingBaud **/
  MVREXPORT bool chooseStartingBaud(const char *reflectorBits);
  /// Gets the reflectorBits that was chosen
  /** @see canChooseStartingBaud **/
  const char *getStartingBaudChoice(void) { return myStartingBaudChoice.c_str(); }


  /**
     Gets if you can choose autoBaud for the laser.

     If so, call chooseAutoBaud with one of the choices in
     getAutoBaudChoices, and see what the choice was with
     getAutoBaudChoice.
  **/
  bool canChooseAutoBaud(void) { return myCanChooseAutoBaud; }
  /// Gets the list of reflectorBits choices 
  /** @see canChooseAutoBaud **/
  std::list<std::string> getAutoBaudChoices(void) 
    { return myAutoBaudChoices; }
  /// Gets a string with the list of reflectorBits choices seperated by |s 
  /** @see canChooseAutoBaud **/
  const char *getAutoBaudChoicesString(void) 
    { return myAutoBaudChoicesString.c_str(); }
  /// Sets the reflectorBits to one of the choices from getAutoBaudChoices
  /** @see canChooseAutoBaud **/
  MVREXPORT bool chooseAutoBaud(const char *reflectorBits);
  /// Gets the reflectorBits that was chosen
  /** @see canChooseAutoBaud **/
  const char *getAutoBaudChoice(void) { return myAutoBaudChoice.c_str(); }


  /// Adds a connect callback
  void addConnectCB(MvrFunctor *functor,
			     int position = 50) 
    { myConnectCBList.addCallback(functor, position); }
  /// Adds a disconnect callback
  void remConnectCB(MvrFunctor *functor)
    { myConnectCBList.remCallback(functor); }

  /// Adds a callback for when a connection to the robot is failed
  void addFailedConnectCB(MvrFunctor *functor, 
				   int position = 50) 
    { myFailedConnectCBList.addCallback(functor, position); }
  /// Removes a callback for when a connection to the robot is failed
  void remFailedConnectCB(MvrFunctor *functor)
    { myFailedConnectCBList.remCallback(functor); }

  /// Adds a callback for when disconnect is called while connected
  void addDisconnectNormallyCB(MvrFunctor *functor, 
			     int position = 50) 
    { myDisconnectNormallyCBList.addCallback(functor, position); }

  /// Removes a callback for when disconnect is called while connected
  void remDisconnectNormallyCB(MvrFunctor *functor)
    { myDisconnectNormallyCBList.remCallback(functor); }
  
  /// Adds a callback for when disconnection happens because of an error
  void addDisconnectOnErrorCB(MvrFunctor *functor, 
			     int position = 50) 
    { myDisconnectOnErrorCBList.addCallback(functor, position); }

  /// Removes a callback for when disconnection happens because of an error
  void remDisconnectOnErrorCB(MvrFunctor *functor)
    { myDisconnectOnErrorCBList.remCallback(functor); }

  /// Adds a callback that is called whenever a laser reading is processed
  void addReadingCB(MvrFunctor *functor,
			     int position = 50) 
    { myDataCBList.addCallback(functor, position); }

  /// Removes a callback that is called whenever a laser reading is processed
  void remReadingCB(MvrFunctor *functor)
    { myDataCBList.remCallback(functor); }

  /// Gets the absolute maximum range on the sensor
  unsigned int getAbsoluteMaxRange(void) { return myAbsoluteMaxRange; }

  /// Copies the reading count stuff from another laser (for the laser filter)
  MVREXPORT void copyReadingCount(const MvrLaser* laser);

  /// override the default to bound the maxrange by the absolute max range
  MVREXPORT virtual void setMaxRange(unsigned int maxRange);

  /// override the default to keep track of its been set or not
  MVREXPORT virtual void setCumulativeBufferSize(size_t size);

  /// Call the laser can implement to make sure the parameters
  /// are all okay or set the maximum range (based on the params)
  /**
     The base laser should make sure all the parameters make sense
     according to what was set up as allowed. 

     This is here for two purposes.  The first is to check for
     parameters that aren't valid because of something the base class
     can't check for. The second is to recalculate whatever the
     maximum range of the sensor is based on those settings, and call
     setAbsoluteMaxRange if the maximum range has changed based on the
     settings.

     This is strictly an internal call, mostly for the simulated laser
     so that it can more closely match the real laser on complicated
     things like the LMS2xx where the settings for the units and bits
     affect what the maximum range is.

     @internal
  **/
  MVREXPORT virtual bool laserCheckParams(void) { return true; }

  /// Applies a transform to the buffers
  MVREXPORT virtual void applyTransform(MvrTransform trans,
                                        bool doCumulative = true);

  /// Makes it so we'll apply simple naming to all the lasers
  MVREXPORT static void useSimpleNamingForAllLasers(void);
protected:
  
  /// Converts the raw readings into the buffers (needs to be called
  /// by subclasses)
  MVREXPORT void laserProcessReadings(void);

  /// Returns if the laser has lost connection so that the subclass
  /// can do something appropriate
  MVREXPORT bool laserCheckLostConnection(void);

  /// Pulls the unset params from the robot parameter file
  MVREXPORT bool laserPullUnsetParamsFromRobot(void);

  /// Allows setting the degrees the laser uses to anything in a range
  MVREXPORT void laserAllowSetDegrees(double defaultStartDegrees, double startDegreesMin, double startDegreesMax, double defaultEndDegrees, double endDegreesMin, double endDegreesMax);

  /// Allows setting the degrees the laser uses to one of a number of choices
  MVREXPORT void laserAllowDegreesChoices(const char *defaultDegreesChoice,
			   std::map<std::string, double> degreesChoices);

  /// Allows setting the increment the laser uses to anything in a range
  MVREXPORT void laserAllowSetIncrement(
	  double defaultIncrement, double incrementMin,  double incrementMax);

  /// Allows setting the increment to one of a number of choices
  MVREXPORT void laserAllowIncrementChoices(const char *defaultIncrementChoice, 
			     std::map<std::string, double> incrementChoices);

  /// Allows setting the units the laser will use to one of a number of choices
  MVREXPORT void laserAllowUnitsChoices(const char *defaultUnitsChoice, 
			 std::list<std::string> unitsChoices);

  /// Allows setting the reflector bits the laser will use to one of a
  /// number of choices
  MVREXPORT void laserAllowReflectorBitsChoices(
	  const char *defaultReflectorBitsChoice,
	  std::list<std::string> reflectorBitsChoices);

  /// Allows setting if the power is controlled or not
  MVREXPORT void laserAllowSetPowerControlled(bool defaultPowerControlled);

  /// Allows setting the starting baud to one of a number of choices
  MVREXPORT void laserAllowStartingBaudChoices(
	  const char *defaultStartingBaudChoice, 
	  std::list<std::string> startingBaudChoices);

  /// Allows setting the auto baud speed to one of a number of choices
  MVREXPORT void laserAllowAutoBaudChoices(
	  const char *defaultAutoBaudChoice, 
	  std::list<std::string> autoBaudChoices);

  /// Called when the lasers name is set
  MVREXPORT virtual void laserSetName(const char *name);
  
  /// Sets the laser's default TCP port
  MVREXPORT void laserSetDefaultTcpPort(int defaultLaserTcpPort);

  /// Sets the laser's default connection port type
  MVREXPORT void laserSetDefaultPortType(const char *defaultPortType);

  /// Sets the absolute maximum range on the sensor
  MVREXPORT void laserSetAbsoluteMaxRange(unsigned int absoluteMaxRange);

  /// Function for a laser to call when it connects
  MVREXPORT virtual void laserConnect(void);
  /// Function for a laser to call when it fails to connects
  MVREXPORT virtual void laserFailedConnect(void);
  /// Function for a laser to call when it disconnects normally
  MVREXPORT virtual void laserDisconnectNormally(void);
  /// Function for a laser to call when it loses connection
  MVREXPORT virtual void laserDisconnectOnError(void);

  // processes the individual reading, helper for base class
  MVREXPORT void internalProcessReading(double x, double y, unsigned int range,
				    bool clean, bool onlyClean);

  // internal helper function for seeing if the choice matches
  MVREXPORT bool internalCheckChoice(const char *check, const char *choice,
		   std::list<std::string> *choices, const char *choicesStr);
  // internal helper function for seeing if the choice matches
  MVREXPORT bool internalCheckChoice(const char *check, const char *choice,
		   std::map<std::string, double> *choices,
		   const char *choicesStr, double *choiceDouble);
  // internal helper function for building a string for a list of chocies
  void internalBuildChoicesString(std::list<std::string> *choices, std::string *str);
  // internal helper function for building a string for a list of chocies
  void internalBuildChoices(std::map<std::string, double> *choices, 
		    std::string *str, std::list<std::string> *choicesList);

  // Function called in laserProcessReadings to indicate that a
  // reading was received
  MVREXPORT virtual void internalGotReading(void);

  int myLaserNumber;


  MvrDeviceConnection *myConn;
  MvrMutex myConnMutex;

  double myTimeoutSeconds;
  

  MvrPose mySensorPose;
  double mySensorZ;
  bool myHaveSensorPose;

  double myCumulativeCleanDist;
  double myCumulativeCleanDistSquared;
  int myCumulativeCleanInterval;
  int myCumulativeCleanOffset;
  MvrTime myCumulativeLastClean;
  std::set<int> myIgnoreReadings;

  unsigned int myAbsoluteMaxRange;
  bool myMaxRangeSet;

  bool myCumulativeBufferSizeSet;

  bool myFlippedSet;
  bool myFlipped;
  
  bool myCanSetDegrees; 
  double myStartDegreesMin;
  double myStartDegreesMax;
  bool myStartDegreesSet;
  double myStartDegrees;
  double myEndDegreesMin;
  double myEndDegreesMax;
  bool myEndDegreesSet;
  double myEndDegrees;

  bool myCanChooseDegrees;
  std::map<std::string, double> myDegreesChoices; 
  std::list<std::string> myDegreesChoicesList; 
  bool myDegreesChoiceSet;
  std::string myDegreesChoicesString; 
  std::string myDegreesChoice;
  double myDegreesChoiceDouble;


  bool myCanSetIncrement; 
  double myIncrementMin; 
  double myIncrementMax; 
  bool myIncrementSet;
  double myIncrement;

  bool myCanChooseIncrement; 
  std::map<std::string, double> myIncrementChoices; 
  std::list<std::string> myIncrementChoicesList; 
  std::string myIncrementChoicesString; 
  bool myIncrementChoiceSet;
  std::string myIncrementChoice;
  double myIncrementChoiceDouble;

  bool myCanChooseUnits; 
  std::list<std::string> myUnitsChoices; 
  std::string myUnitsChoicesString; 
  bool myUnitsChoiceSet;
  std::string myUnitsChoice;

  bool myCanChooseReflectorBits; 
  std::list<std::string> myReflectorBitsChoices; 
  std::string myReflectorBitsChoicesString; 
  bool myReflectorBitsChoiceSet;
  std::string myReflectorBitsChoice;
  
  bool myCanSetPowerControlled;
  bool myPowerControlledSet;
  bool myPowerControlled;

  bool myCanChooseStartingBaud; 
  std::list<std::string> myStartingBaudChoices; 
  std::string myStartingBaudChoicesString; 
  bool myStartingBaudChoiceSet;
  std::string myStartingBaudChoice;

  bool myCanChooseAutoBaud; 
  std::list<std::string> myAutoBaudChoices; 
  std::string myAutoBaudChoicesString; 
  bool myAutoBaudChoiceSet;
  std::string myAutoBaudChoice;

  int myDefaultTcpPort;
  std::string myDefaultPortType;

  MvrCallbackList myConnectCBList;
  MvrCallbackList myFailedConnectCBList;
  MvrCallbackList myDisconnectOnErrorCBList;
  MvrCallbackList myDisconnectNormallyCBList;
  MvrCallbackList myDataCBList;

  MvrLog::LogLevel myInfoLogLevel;

  MvrTime myLastReading;
  // packet count
  time_t myTimeLastReading;
  int myReadingCurrentCount;
  int myReadingCount;
  bool myRobotRunningAndConnected;

  static bool ourUseSimpleNaming;
};

#endif // ARRANGEDEVICELASER_H
