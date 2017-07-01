#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrSick.h"
#include "MvrRobot.h"
#include "MvrSerialConnection.h"
#include "mvriaInternal.h"
#include <time.h>


MVREXPORT MvrSick::MvrSick(
	size_t currentBufferSize, size_t cumulativeBufferSize,
	const char *name, bool addMvriaExitCB, bool isSecondLaser) : 
  MvrLMS2xx(!isSecondLaser ? 1 : 2, name, !isSecondLaser ? false : true)
{
  setCurrentBufferSize(currentBufferSize);
  setCumulativeBufferSize(cumulativeBufferSize);
}

MVREXPORT MvrSick::~MvrSick()
{

}


/**
 * Manually set laser configuration options for connection. This must be called
 * only before connecting to the laser (not while the laser is connected).
 * This configuration is automatically performed if you are using
 * MvrSimpleConnector to connect to the laser based on command line parameters,
 * so calling this function is only neccesary if you are not using
 * MvrSimpleConnector, or you wish to always override MvrSimpleConnector's
 * configuration. 
 *
 * (Don't forget, you must lock MvrLMS2xx with lockDevice() if multiple threads
 * are accessing the MvrLMS2xx, e.g. if you used runAsync().)
**/
MVREXPORT void MvrSick::configure(bool useSim, bool powerControl,
				bool laserFlipped, BaudRate baud,
				Degrees deg, Increment incr)
{
  configureShort(useSim, baud, deg, incr);

  setPowerControlled(powerControl);
  setFlipped(laserFlipped);
}

/**
 * @copydoc configure()
**/
MVREXPORT void MvrSick::configureShort(bool useSim, BaudRate baud,
				     Degrees deg, Increment incr)
{
  myUseSim = useSim;

  setPowerControlled(true);
  setFlipped(false);

  if (baud == BAUD9600)
    chooseAutoBaud("9600");
  else if (baud == BAUD19200)
    chooseAutoBaud("19200");
  else if (baud == BAUD38400)
    chooseAutoBaud("38400");
  else
    MvrLog::log(MvrLog::Normal, "%s: Bad baud choice", getName());

  if (deg == DEGREES180)
    chooseDegrees("180");
  else if (deg == DEGREES100)
    chooseDegrees("100");
  else
    MvrLog::log(MvrLog::Normal, "%s: Bad degrees choice", getName());


  if (incr == INCREMENT_ONE)
    chooseIncrement("one");
  else if (incr == INCREMENT_HALF)
    chooseIncrement("half");
  else
    MvrLog::log(MvrLog::Normal, "%s: Bad increment choice", getName());

}

/**
   Sets the range/bit information.  The old immutable combination is
   (in effect) the same as the new default.  If you look at the enums
   for these units you can see the effect this has on range.  
**/
MVREXPORT void MvrSick::setRangeInformation(Bits bits, Units units)
{
  if (bits == BITS_1REFLECTOR)
    chooseReflectorBits("1ref");
  else if (bits == BITS_2REFLECTOR)
    chooseReflectorBits("2ref");
  else if (bits == BITS_3REFLECTOR)
    chooseReflectorBits("3ref");
  else
    MvrLog::log(MvrLog::Normal, "%s: Bad reflectorBits choice", getName());

  if (units == UNITS_1MM)
    chooseUnits("1mm");
  else if (units == UNITS_1MM)
    chooseUnits("1cm");
  else if (units == UNITS_10CM)
    chooseUnits("10cm");
  else
    MvrLog::log(MvrLog::Normal, "%s: Bad units choice", getName());
}



MVREXPORT bool MvrSick::isControllingPower(void)
{
  return getPowerControlled(); 
}

MVREXPORT bool MvrSick::isLaserFlipped(void)
{
  return getFlipped();
}

MVREXPORT MvrSick::Degrees MvrSick::getDegrees(void)
{
  if (strcasecmp(getDegreesChoice(), "180") == 0)
    return DEGREES180;
  else if (strcasecmp(getDegreesChoice(), "100") == 0)
    return DEGREES100;
  else
  {
    MvrLog::log(MvrLog::Normal, "MvrSick::getDegrees: Invalid degrees %s",
	       getDegreesChoice());
    return DEGREES_INVALID;
  }
}

MVREXPORT MvrSick::Increment MvrSick::getIncrement(void)
{
  if (strcasecmp(getIncrementChoice(), "one") == 0)
    return INCREMENT_ONE;
  else if (strcasecmp(getIncrementChoice(), "half") == 0)
    return INCREMENT_HALF;
  else
  {
    MvrLog::log(MvrLog::Normal, "MvrSick::getIncrement: Invalid increment %s",
	       getIncrementChoice());
    return INCREMENT_INVALID;
  }
}

MVREXPORT MvrSick::Bits MvrSick::getBits(void)
{
  if (strcasecmp(getReflectorBitsChoice(), "1ref") == 0)
    return BITS_1REFLECTOR;
  else if (strcasecmp(getReflectorBitsChoice(), "2ref") == 0)
    return BITS_2REFLECTOR;
  else if (strcasecmp(getReflectorBitsChoice(), "3ref") == 0)
    return BITS_3REFLECTOR;
  else
  {
    MvrLog::log(MvrLog::Normal, "MvrSick::getReflectorBits: Invalid ReflectorBits %s",
	       getReflectorBitsChoice());
    return BITS_INVALID;
  }
}

MVREXPORT MvrSick::Units MvrSick::getUnits(void)
{
  if (strcasecmp(getUnitsChoice(), "1mm") == 0)
    return UNITS_1MM;
  else if (strcasecmp(getUnitsChoice(), "1cm") == 0)
    return UNITS_1CM;
  else if (strcasecmp(getUnitsChoice(), "10cm") == 0)
    return UNITS_10CM;
  else
  {
    MvrLog::log(MvrLog::Normal, "MvrSick::getUnits: Invalid units %s",
	       getUnitsChoice());
    return UNITS_INVALID;
  }
}

MVREXPORT void MvrSick::setIsControllingPower(bool controlPower)
{
  setPowerControlled(controlPower);
}

MVREXPORT void MvrSick::setIsLaserFlipped(bool laserFlipped)
{
  setFlipped(laserFlipped);
}


MVREXPORT bool MvrSick::isUsingSim(void)
{
  return sickGetIsUsingSim();
}

MVREXPORT void MvrSick::setIsUsingSim(bool usingSim)
{
  return sickSetIsUsingSim(usingSim);
}
