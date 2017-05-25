/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrSick.h
 > Description  : Compatability class used to access SICK LMS-200 laser rangefinder device
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRSICK_H
#define MVRSICK_H

#include "mvriaTypedefs.h"
#include "MvrRobotPacket.h"
#include "MvrLaser.h"   
#include "MvrFunctor.h"
#include "MvrCondition.h"
#include "MvrLMS2xx.h"

class MvrSick : public MvrLMS2xx
{
public:
  /// Constructor
  MVRXPORT MvrSick(size_t currentBufferSize = 361, 
		  size_t cumulativeBufferSize = 0,
		  const char *name = "laser",
		  bool addMvriaExitCB = true,
		  bool isSecondLaser = false);
  /// Destructor
  MVRXPORT virtual ~MvrSick();

  enum BaudRate {
    BAUD9600, ///< 9600 Baud
    BAUD19200, ///< 19200 Baud
    BAUD38400, ///< 38400 Baud
    BAUD_INVALID ///< Invalid baud
  };
  enum Degrees {
    DEGREES180, ///< 180 Degrees
    DEGREES100, ///< 100 Degrees
    DEGREES_INVALID ///< Invalid degrees
  };
  enum Increment {
    INCREMENT_ONE, ///< One degree increments
    INCREMENT_HALF, ///< Half a degree increments
    INCREMENT_INVALID ///< Increment invalid
  };
  enum Units {
    UNITS_1MM, ///< Uses 1 mm resolution (8/16/32 meter max range)
    UNITS_1CM, ///< Uses 1 cm resolution (80/160/320 meter max range)
    UNITS_10CM, ///< Uses 10 cm resolution (150 meter max range)
    UNITS_INVALID ///< Invalid units
  };
  enum Bits {
    BITS_1REFLECTOR, ///< Uses 1 reflector bits (means 32/320/150 meter range)
    BITS_2REFLECTOR, ///< Uses 2 reflector bits (means 16/160/150 meter range)
    BITS_3REFLECTOR, ///< Uses 3 reflector bits (means 8/80/150 meter range)
    BITS_INVALID ///< Invalid bits
  };

  /// Sees if this is trying to connect to the laser at the moment (note no other range devices work like this so you probably shouldn't use this)
  bool tryingToConnect(void) 
    { return isTryingToConnect(); }

  /// Runs the laser off of the robot
  bool runOnRobot(void) { return internalRunOnRobot(); }


  /// Use this to manually configure the laser before connecting to it
  MVRXPORT void configure(bool useSim = false, bool powerControl = true, 
                          bool laserFlipped = false, 
                          BaudRate baud = BAUD38400,
                          Degrees deg = DEGREES180, 
                          Increment incr = INCREMENT_ONE);
  /// Shorter configure for the laser (settings are overridden by the .p file)
  MVRXPORT void configureShort(bool useSim = false, 
                               BaudRate baud = BAUD38400,
                               Degrees deg = DEGREES180, 
                               Increment incr = INCREMENT_ONE);
  /// Sets the ranging/reflector information
  MVRXPORT void setRangeInformation(Bits bits = BITS_1REFLECTOR,
				                            Units units = UNITS_1MM);


  // Gets whether the laser is simulated or not
  MVRXPORT bool isUsingSim(void);
  /// Gets whether the computer is controling laser power or not
  MVRXPORT bool isControllingPower(void);
  /// Gets whether the laser is flipped over or not
  MVRXPORT bool isLaserFlipped(void);
  /// Gets the degrees the laser is scanning
  MVRXPORT Degrees getDegrees(void);
#ifndef SWIG
        // this ends up causing problems in the Java wrapper, since it has the
        // wrong return type vs. MvrLaser::getIncrement(). But we need to keep
        // this function for backwards compatability.
        /// Gets the amount each scan increments
  MVRXPORT Increment getIncrement(void);
#endif
  /// Gets the Bits the laser is using
  MVRXPORT Bits getBits(void);
  /// Gets the Units the laser is using
  MVRXPORT Units getUnits(void);

  // Sets whether the laser is simulated or not
  MVRXPORT void setIsUsingSim(bool usingSim);
  /// Sets whether the computer is controling laser power or not
  MVRXPORT void setIsControllingPower(bool controlPower);
  /// Sets whether the laser is flipped over or not
  MVRXPORT void setIsLaserFlipped(bool laserFlipped);



  /** You should use setCumulativeCleanDist for this now
     @deprecated
  */
  void setFilterCumulativeCleanDist(double dist) { setCumulativeCleanDist(dist);}
  /** You should use getCumulativeCleanDist for this now
     @deprecated
  */
  double getFilterCumulativeCleanDist(void) { return getCumulativeCleanDist(); }
  /** You should use setCumulativeCleanInterval for this now
     @deprecated
  */
  void setFilterCleanCumulativeInterval(int milliSeconds)
  { setCumulativeCleanInterval(milliSeconds); }
  /** You should use getPlanarCumulativeCleanInterval for this now
     @deprecated
  */
  int getFilterCleanCumulativeInterval(void) { return getCumulativeCleanInterval(); }

  void setFilterCumulativeNearDist(double dist) { setMinDistBetweenCumulative(dist); }
  double getFilterCumulativeNearDist(void) { return getMinDistBetweenCumulative(); }

  void setFilterNearDist(double dist) { setMinDistBetweenCurrent(dist); }
  double getFilterNearDist(void) { return getMinDistBetweenCurrent(); }

  void setFilterCumulativeInsertMaxDist(double dist)
  { setMaxInsertDistCumulative(dist); }
  /**
     You should use getMaxInsertDistCumulative for this now
     @deprecated
  */
  double getFilterCumulativeInsertMaxDist(void) { return getMaxInsertDistCumulative(); }

  unsigned int getMinRange(void) { return 0; } 
 
  void setMinRange(unsigned int minRange) { }

  void setFilterCumulativeMaxDist(double dist) 
  { setMaxDistToKeepCumulative(dist); }

  double getFilterCumulativeMaxDist(void)
  { return getMaxDistToKeepCumulative(); }

  void setFilterCumulativeMaxAge(int seconds) 
  { setMaxSecondsToKeepCumulative(seconds); }

  int getFilterCumulativeMaxAge(void) 
  { return getMaxSecondsToKeepCumulative(); }

  /// Gets the number of laser packets received in the last second
  int getSickPacCount(void)
  { return getReadingCount(); }

  /// Adds a connect callback
  void addConnectCB(MvrFunctor *functor,
			     MvrListPos::Pos position = MvrListPos::LAST)
  {
    if (position == MvrListPos::FIRST)
	    MvrLMS2xx::addConnectCB(functor, 75);
    else if (position == MvrListPos::LAST)
	    MvrLMS2xx::addConnectCB(functor, 25);
    else
	    MvrLMS2xx::addConnectCB(functor, 25);
  }
  /// Adds a disconnect callback
  void remConnectCB(MvrFunctor *functor)
  {
    MvrLMS2xx::remConnectCB(functor);
  }

  /// Adds a callback for when a connection to the robot is failed
  void addFailedConnectCB(MvrFunctor *functor, 
				   MvrListPos::Pos position = MvrListPos::LAST)
  {
    if (position == MvrListPos::FIRST)
	    MvrLMS2xx::addFailedConnectCB(functor, 75);
    else if (position == MvrListPos::LAST)
	    MvrLMS2xx::addFailedConnectCB(functor, 25);
    else
	    MvrLMS2xx::addFailedConnectCB(functor, 25);
  }
  /// Removes a callback for when a connection to the robot is failed
  void remFailedConnectCB(MvrFunctor *functor)
  {
    MvrLMS2xx::remFailedConnectCB(functor);
  }

  /// Adds a callback for when disconnect is called while connected
  void addDisconnectNormallyCB(MvrFunctor *functor, 
				MvrListPos::Pos position = MvrListPos::LAST)
  {
    if (position == MvrListPos::FIRST)
	    MvrLMS2xx::addDisconnectNormallyCB(functor, 75);
    else if (position == MvrListPos::LAST)
	    MvrLMS2xx::addDisconnectNormallyCB(functor, 25);
    else
	    MvrLMS2xx::addDisconnectNormallyCB(functor, 25);
  }
  /// Removes a callback for when disconnect is called while connected
   void remDisconnectNormallyCB(MvrFunctor *functor)
  {
    MvrLMS2xx::remDisconnectNormallyCB(functor);
  }
  
  /// Adds a callback for when disconnection happens because of an error
  void addDisconnectOnErrorCB(MvrFunctor *functor, 
			       MvrListPos::Pos position = MvrListPos::LAST)
  {
    if (position == MvrListPos::FIRST)
	    MvrLMS2xx::addDisconnectOnErrorCB(functor, 75);
    else if (position == MvrListPos::LAST)
	    MvrLMS2xx::addDisconnectOnErrorCB(functor, 25);
    else
	    MvrLMS2xx::addDisconnectOnErrorCB(functor, 25);
  }
  /// Removes a callback for when disconnection happens because of an error
  void remDisconnectOnErrorCB(MvrFunctor *functor)
  {
    MvrLMS2xx::remDisconnectOnErrorCB(functor);
  }
  /// Adds a callback that is called whenever a laser packet is processed
  void addDataCB(MvrFunctor *functor,
			  MvrListPos::Pos position = MvrListPos::LAST)
  {
    if (position == MvrListPos::FIRST)
	    MvrLMS2xx::addReadingCB(functor, 75);
    else if (position == MvrListPos::LAST)
	    MvrLMS2xx::addReadingCB(functor, 25);
    else
	    MvrLMS2xx::addReadingCB(functor, 25);
  }
  /// Removes a callback that is called whenever a laser packet is processed
  void remDataCB(MvrFunctor *functor)
  {
    MvrLMS2xx::remReadingCB(functor);
  }

  /// Sets the time without a response until connection assumed lost
  void setConnectionTimeoutTime(int secs)
  { setConnectionTimeoutSeconds(secs); }
  /// Gets the time without a response until connection assumed lost
  int getConnectionTimeoutTime(void)
  { return MvrMath::roundInt(getConnectionTimeoutSeconds()); }

};


#endif //MVRSICK_H
