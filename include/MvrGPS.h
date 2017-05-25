/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrGPS.h
 > Description  : GPS Device Interface 
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRGPS_H
#define MVRGPS_H

#include "mvriaTypedefs.h"
#include "MvrFunctor.h"
#include "mvriaUtil.h"
#include "MvrMutex.h"
#include "MvrNMEAParser.h"
#include "MvrGPSCoords.h"
#include <math.h>
#include <string>
#include <vector>

class MvrDeviceConnection; // for pointer in MvrGPS

 /*
 *  Connects to GPS device over a serial port or other device connection and reads data.
 *  Supports GPS devices sending standard NMEA format data 
 */
class MvrGPS {
public:
    MVREXPORT MvrGPS();

    virtual ~MvrGPS() { }

    /** @brief Set device connection to use */
    void setDeviceConnection(MvrDeviceConnection* deviceConn) { myDevice = deviceConn; }

    /** @brief Return device connection in use (or NULL if none) */
    MvrDeviceConnection* getDeviceConnection() const { return myDevice; }


    /** @brief Check that the device connection (e.g. serial port) is open, and
     *  that data is being received from GPS.
     *  Subclasses may override this method so that  device-specific
     *  initialization commands may be sent.
     *  @return false if there is no device connection or the device connection
     *  is not open, or if there is an error sending device initialization
     *  commands, or if no data is received after calling read() every 100 ms
     *  for @a connectTimeout ms. Otherwise, return true.
     */
    MVREXPORT virtual bool connect(unsigned long connectTimeout = 20000);

    /** Same as connect(). See connect().  */
    bool blockingConnect(unsigned long connectTimeout = 20000) { return connect(connectTimeout); }

public:
    /// @brief Flags to indicates what the read() method did. 
    enum {
      ReadFinished = MvrNMEAParser::ParseFinished,
      ReadError = MvrNMEAParser::ParseError, 
      ReadData = MvrNMEAParser::ParseData,
      ReadUpdated = MvrNMEAParser::ParseUpdated
    } ReadFlags;

    /** @brief Read some data from the device connection, and update stored data as complete messages are received. 
     * @param maxTime If nonzero, return when this time limit is reached, even if there is still data available to read. If zero, then don't return until all available data has been exhausted or an error occurs. 
     * @return A mask of ReadFlags codes, combined with bitwise or (|), or 0 if no attempt to read from the device occured (for example because the @a maxTime timeout was reached before the first attempt to read occured).  The flags will include
     */
    MVREXPORT virtual int read(unsigned long maxTime = 0);

    /** Calls lock(), calls read(maxTime), then calls unlock(). Note, this could
     * end up keeping MvrGPS locked until @a maxTime is reached, or for any amount
     * of time if @a maxTime is 0, so watch out for that. */
    int readWithLock(unsigned int maxTime) { lock(); int r = read(maxTime); unlock(); return r; }

    /** Locks a mutex object contained by this class.
     *  No other method (except readWithLock()) in MvrGPS locks or unlocks this
     *  mutex, it is provided for you to use when accessing MvrGPS from multiple
     *  threads.
     */
    void lock() { myMutex.lock(); }

    /** Unlocks a mutex object contained by this class.
     *  No other method (except readWithLock()) in MvrGPS locks or unlocks this
     *  mutex, it is provided for you to use when accessing MvrGPS from multiple
     *  threads.
     */
    void unlock() { myMutex.unlock(); }
    

    /** @brief Set whether checksum sent with NMEA messages is ignored */
    void setIgnoreChecksum(bool ignore) { myNMEAParser.setIgnoreChecksum(ignore); }

    /** @brief Log last received data using MvrLog. */
    MVREXPORT void logData() const;

    /** Print basic navigation data on one line to standard output, with no newline at end. */
    MVREXPORT void printData(bool labels = true) const;

    MVREXPORT void printDataLabelsHeader() const;

    /** Data accessors
     * @brief Access the last received data from the GPS */
    // @{
 
    typedef enum {  
        NoFix, BadFix, GPSFix, DGPSFix, PPSFix, 
        RTKinFix, FloatRTKinFix, DeadReckFix, 
        ManualFix, SimulatedFix, UnknownFixType,
        OmnistarConverging = FloatRTKinFix, 
        OmnistarConverged = RTKinFix
     } FixType;

    class Data {
    public:
        MVREXPORT Data();
        double latitude; ///< (from NMEA GPRMC)
        double longitude; ///< (from NMEA GPRMC)
        bool havePosition; ///< (from NMEA GPRMC)
        MvrTime timeGotPosition;   ///< Local computer time when MvrGPS class received the position message from the GPS. (From NMEA GPRMC)
        double speed; ///< (From NMEA GPRMC, if provided)
        bool haveSpeed; ///< (From NMEA GPRMC)
        MvrTime GPSPositionTimestamp;   ///< Timestamp provided by GPS device along with latitude and longitude. (from NMEA GPRMC)
        MvrGPS::FixType fixType; ///< (from NMEA GPGGA)
        unsigned short numSatellitesTracked;
        double altitude;    ///< receiver provides this based on GPS data.  meters above sea level. (from NMEA GPGGA)
        bool haveAltitude; //< (from NMEA GPGGA)
        double altimeter;   ///< from seperate altimeter (if receiver provides PGRMZ message). meters above sea level.
        bool haveAltimeter;
        unsigned short DGPSStationID; ///< (from NMEA GPGGA)
        bool haveDGPSStation; ///< (from NMEA GPGGA)
        double garminPositionError; ///< Error in meters, only some GPS devices provide this
        bool haveGarminPositionError; ///< Error in meters, only some GPS devices provide this (PGRME)
        double garminVerticalPositionError; ///< Error in meters, only some GPS devices provide this (PGRME)
        bool haveGarminVerticalPositionError; ///< Error in meters, only some GPS devices provide this (PGRME)
        double compassHeadingMag; ///< (from HCDHM message, if device provides it)
        double compassHeadingTrue; ///< (from HCHDT, if device provides it)
        bool haveCompassHeadingMag; ///< (from HCDHM message, if device provides it)
        bool haveCompassHeadingTrue; ///< (from HCHDT message, if device provides it)
        unsigned long compassMagCounter;  ///< Incremented whenever @a compassHeadingMag is updated with new data
        unsigned long compassTrueCounter;  ///< Incremented whenever @a compassHeadingMag is updated with new data
        bool haveHDOP; ///< Horizontal dilution of precision (from NMEA GPGGA)
        double HDOP; ///< Horizontal dilution of precision (from NMEA GPGGA)
        bool haveVDOP; ///< Vertical dilution of precision (from NMEA GPGGA)
        double VDOP; ///< Vertical dilution of precision (from NMEA GPGGA)
        bool havePDOP; ///< Combined dilution of precision (from NMEA GPGGA)
        double PDOP; ///< Combined dilution of precision (from NMEA GPGGA)
        bool qualityFlag;   ///< Some GPS devices set this to false if data quality is below some thresholds.
        double meanSNR;   ///< Mean of satellite signal-noise ratios (dB)
        bool haveSNR; ///< (from NMEA GPGSV)
        double beaconSignalStrength;  ///< dB (from NMEA GPMSS)
        double beaconSNR; ///< dB  (from NMEA GPMSS)
        double beaconFreq; ///< kHz (from NMEA GPMSS)
        unsigned short beaconBPS; ///< Bits/sec (from NMEA GPMSS)
        unsigned short beaconChannel; ///< (from NMEA GPMSS)
        bool haveBeaconInfo; ///< (from NMEA GPMSS)
        double inputsRMS; ///< (from NMEA GPGST)
        bool haveInputsRMS; ///< (from NMEA GPGST)
        MvrPose errorEllipse; ///< Ellipse shows standard deviation, in meters. Orientation is degrees from true north. (from NMEA GPGST)
        bool haveErrorEllipse; ///< (from NMEA GPGST)
        MvrPose latLonError; ///< Std.deviation, meters. Theta is unused. May only be provided by the GPS in certain fix modes. Note, values could be inf or nan (GPS sends these in some situations). Use isinf() and isnan() to check.
        bool haveLatLonError;
        double altitudeError; ///< Std. deviation, meters. Note, value could be inf or nan (GPS sends these in some situations). use isinf() and isnan() to check.
        bool haveAltitudeError;
    };

    /** Access all of the internally stored data directly. @see MvrGPS::Data  */
    const MvrGPS::Data& getCurrentDataRef() const { return myData; } 

    /** (from NMEA GPGGA) */
    FixType getFixType() const { return myData.fixType; }
    /** (from NMEA GPGGA) */
    MVREXPORT const char* getFixTypeName() const;
    static MVREXPORT const char* getFixTypeName(FixType type);

    /** (from NMEA GPRMC) */
    MVREXPORT bool havePosition() const { return myData.havePosition; }
    /** (from NMEA GPRMC) */
    MVREXPORT bool haveLatitude() const { return myData.havePosition; }
    /** (from NMEA GPRMC) */
    MVREXPORT bool haveLongitude() const { return myData.havePosition; }

    /** @return latitude in decimal degrees. 
        (from NMEA GPRMC) */
    double getLatitude() const { return myData.latitude; }

    /** @return longitude in decimal degrees. 
        (from NMEA GPRMC) */
    double getLongitude() const { return myData.longitude; }

    /// @return latitude, longitude and altitude
    MvrLLACoords getLLA() const { return MvrLLACoords(myData.latitude, myData.longitude, myData.altitude); }

    /// @return latitude and longitude in an MvrPose object. Theta will be 0.
    MvrPose getPose() const { return MvrPose(myData.latitude, myData.longitude); }

    /// @return copy of an MvrTime object set to the time that MvrGPS read and received latitude and longitude data from the GPS. 
    MvrTime getTimeReceivedPosition() const { return myData.timeGotPosition; }

    bool haveSpeed() const { return myData.haveSpeed; }

    double getSpeed() const { return myData.speed; }

    /** Timestamp provided by GPS device along with position. (from NMEA GPRMC) */
    MvrTime getGPSPositionTimestamp() const { return myData.GPSPositionTimestamp; }

    int getNumSatellitesTracked() const { return (int) myData.numSatellitesTracked; }
    /** (from NMEA GPGGA) */
    bool haveDGPSStation() const { return myData.haveDGPSStation; }
    /** (from NMEA GPGGA) */
    unsigned short getDGPSStationID() const { return myData.DGPSStationID; }

    /** @return whether GPS provided a distance error estimation (from a
     * Garmin-specific message PGRME, most GPS receivers will not provide this) */
    bool haveGarminPositionError() const { return myData.haveGarminPositionError; }
    /// GPS device's error estimation in meters (from a Garmin-specific message PGRME,
      
    double getGarminPositionError() const { return myData.garminPositionError; }
    /** @return whether GPS provided an altitude error estimation (from a
     * Garmin-specific message PGRME, most GPS receivers will not provide this) */
    bool haveGarminVerticalPositionError() const { return myData.haveGarminVerticalPositionError; }
    /** @return An altitude error estimation (from a Garmin-specific message PGRME,
       * most GPS receivers will not provide this) */
    double getGarminVerticalPositionError() const { return myData.garminVerticalPositionError; }

    /** Have a compass heading value relative to magnetic north.
        @note The GPS or compass device must be configured to send HCHDM messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    bool haveCompassHeadingMag() const { return myData.haveCompassHeadingMag; }
    /** Have a compass heading value relative to true north (using GPS/compass
        device's configured declination).
        @note The GPS or compass device must be configured to send HCHDT messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    bool haveCompassHeadingTrue() const { return myData.haveCompassHeadingTrue; }
    /** Heading from magnetic north
        @note The GPS or compass device must be configured to send HCHDM messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    double getCompassHeadingMag() const { return myData.compassHeadingMag; }
    /** Heading from true north
        @note The GPS or compass device must be configured to send HCHDT messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    double getCompassHeadingTrue() const { return myData.compassHeadingTrue; }


    /** Manually set compass value. */
    void setCompassHeadingMag(double val)
    { 
      myData.haveCompassHeadingMag = true;
      myData.compassHeadingMag = val; 
      myData.compassMagCounter++; 
    }

    /** Manually set compass value. */
    void setCompassHeadingTrue(double val)
    { 
      myData.haveCompassHeadingTrue = true;
      myData.compassHeadingTrue = val; 
      myData.compassMagCounter++; 
    }

    /** Manually set compass value. */
    void setCompassHeadingMagWithLock(double val) { lock(); setCompassHeadingMag(val); unlock(); }
    /** Manually set compass value. */
    void setCompassHeadingTrueWithLock(double val) { lock(); setCompassHeadingTrue(val); unlock(); }

    /// Altitude above sea level calculated from satellite positions (see also haveAltimiter()) (from NMEA GPGGA, if provided)
    bool haveAltitude() const { return myData.haveAltitude; }
    /// Altitude above sea level (meters), calculated from satellite positions (see also getAltimiter()) (from NMEA GPGGA, if provided)
    double getAltitude() const { return myData.altitude; }

    /// Some receivers may have an additional altitude from an altimiter (meters above sea level) (from PGRMZ, if receiver provides it)
    bool haveAltimeter() const { return myData.haveAltimeter; }
    /// Some receivers may have an additional altitude from an altimiter (meters above sea level) (from PGRMZ, if receiver provides it)
    double getAltimeter() const { return myData.altimeter; }

    /** (from NMEA GPGGA) */
    bool haveHDOP() const { return myData.haveHDOP; }
    /** (from NMEA GPGGA) */
    double getHDOP() const { return myData.HDOP; }
    /** (from NMEA GPGGA) */
    bool haveVDOP() const { return myData.haveVDOP; }
    /** (from NMEA GPGGA) */
    double getVDOP() const { return myData.VDOP; }
    /** (from NMEA GPGGA) */
    bool havePDOP() const { return myData.havePDOP; }
    /** (from NMEA GPGGA) */
    double getPDOP() const { return myData.PDOP; }

    /** (from NMEA GPGSV) */
    bool haveSNR() const { return myData.haveSNR; }
    /// dB (from NMEA GPGSV)
    double getMeanSNR() const { return myData.meanSNR; }

    /** Whether we have any DGPS stationary beacon info  (from NMEA GPMSS) */
    bool haveBeaconInfo() const { return myData.haveBeaconInfo; }
    /** DGPS stationary beacon signal strength (dB) (from NMEA GPMSS) */
    double getBeaconSignalStrength() const { return myData.beaconSignalStrength; }  
    /** DGPS stationary beacon signal to noise (dB) (from NMEA GPMSS) */
    double getBeaconSNR() const { return myData.beaconSNR; }  
    /** DGPS stationary beacon frequency (kHz) (from NMEA GPMSS) */
    double getBeaconFreq() const { return myData.beaconFreq; }
    /** DGPS stationary beacon bitrate (bits per second) (from NMEA GPMSS) */
    unsigned short getBecaonBPS() const { return myData.beaconBPS; }
    /** DGPS stationary beacon channel (from NMEA GPMSS) */
    unsigned short getBeaconChannel() const { return myData.beaconChannel; }

    /** Whether we have a position error estimate (as standard deviations in latitude and longitude) (from NMEA GPGST) */
    bool haveErrorEllipse() const { return myData.haveErrorEllipse; }
    /** Standard deviation of position error (latitude and longitude), meters. Theta in MvrPose is orientation of ellipse from true north, Y is the length of the major axis on that orientation, X the minor.
        (from NMEA GPGST)
        @note Values may be inf or NaN (if GPS supplies "Inf" or "NAN")
    */
    MvrPose getErrorEllipse() const {return myData.errorEllipse; }
    
    /** Whether we have latitude or longitude error estimates  (from NMEA GPGST) */
    bool haveLatLonError() const { return myData.haveLatLonError; }
    /** Standard deviation of latitude and longitude error, meters. 
        Theta value in MvrPose is unused. 
        @note May only be provided by GPS in certain fix modes
        @note Values may be inf or NaN (if GPS supplies "Inf" or "NAN")
    */
    MvrPose getLatLonError() const { return myData.latLonError; }
    /** @copydoc getLatLonError() */
    double getLatitudeError() const { return myData.latLonError.getX(); }
    /** @copydoc getLatLonError() */
    double getLongitudeError() const { return myData.latLonError.getY(); }

    bool haveAltitudeError() const { return myData.haveAltitudeError; }
    /// Standard deviation of altitude error, meters. (from NMEA GPGST, if provided)
    double getAltitudeError() const { return myData.altitudeError; }
    
    /// (from NMEA GPGST)
    bool haveInputsRMS() const { return myData.haveInputsRMS; }
    /// (from NMEA GPGST)
    double getInputsRMS() const { return myData.inputsRMS; }

    

    /** Set a handler for an NMEA message. Mostly for internal use or to be used
     * by related classes, but you could use for ususual or custom messages
     * emitted by a device that you wish to be handled outside of the MvrGPS
     * class. 
     */
    void addNMEAHandler(const char *message, MvrNMEAParser::Handler *handler) { myNMEAParser.addHandler(message, handler); }
    void removeNMEAHandler(const char *message) { myNMEAParser.removeHandler(message); }
    void replaceNMEAHandler(const char *message, MvrNMEAParser::Handler *handler)
    { 
      myNMEAParser.removeHandler(message);
      myNMEAParser.addHandler(message, handler); 
    }

protected:
    /** Block until data is read from GPS.
        Waits by calling read() every 100 ms for @a timeout ms.
     */
    MVREXPORT bool waitForData(unsigned long timeout);

    /** Subclasses may override to send device initialization/configuration
     * commands and set up device-specific message handlers. (Default behavior
     * is to do nothing and return true.)
     */
    virtual bool initDevice() { return true; }

     
    /* Most recent data values received, to return to user */
    Data myData;

    /* Utility to read a double floating point number out of a std::string, if possible.
     * @return true if the string was nonempty and @a target was modified.
     */
    bool readFloatFromString(const std::string& str, double* target, double(*convf)(double) = NULL) const;

    /* Utility to read an unsigned short integer out of a std::string, if possible.
     * @return true if the string was nonempty and @a target was modified.
     */
    bool readUShortFromString(const std::string& str, unsigned short* target, unsigned short (*convf)(unsigned short) = NULL) const;


    /* Utility to read a double from a member of a vector of strings, if it exists. */
    bool readFloatFromStringVec(const std::vector<std::string>* vec, size_t i, double* target, double (*convf)(double) = NULL) const;

    /* Utility to read a double from a member of a vector of strings, if it exists. */
    bool readUShortFromStringVec(const std::vector<std::string>* vec, size_t i, unsigned short* target, unsigned short (*convf)(unsigned short) = NULL) const;

    /* Utility to convert DDDMM.MMMM to decimal degrees */
    static double gpsDegminToDegrees(double degmin);

    /* Utility to convert US nautical knots to meters/sec */
    static double knotsToMPS(double knots);
 
    /** Convert meters per second to miles per hour */
    static double mpsToMph(const double mps) { return mps * 2.23693629; }

    /* Utility to convert meters to US feet */
    static double metersToFeet(double m) { return m * 3.2808399; }

    /* Utility to convert US feet  to meters */
    static double feetToMeters(double f) { return f / 3.2808399; }

    /* Mutex */
    MvrMutex myMutex;

    /* Connection info */
    MvrDeviceConnection *myDevice;
    bool myCreatedOwnDeviceCon;
    MvrRetFunctorC<bool, MvrGPS> myParseArgsCallback; 
    MvrArgumentParser* myArgParser;
    
    /* NMEA Parser */
    MvrNMEAParser myNMEAParser;

    /* GPS message handlers */

    void handleGPRMC(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myGPRMCHandler;

    void handleGPGGA(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myGPGGAHandler;

    void handlePGRME(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myPGRMEHandler;

    void handlePGRMZ(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myPGRMZHandler;

    void handleHCHDx(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myHCHDxHandler;

    void handleGPGSA(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myGPGSAHandler;

    void handleGPGSV(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myGPGSVHandler;

    /* For calculating SNR averages based on multiple GPGSV messages. */
    unsigned int mySNRSum;
    unsigned short mySNRNum;

    void handleGPMSS(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myGPMSSHandler;

    void handleGPGST(MvrNMEAParser::Message msg);
    MvrFunctor1C<MvrGPS, MvrNMEAParser::Message> myGPGSTHandler;

    /* Set an MvrTime object using a time read from a string as decimal seconds (SSS.SS) */
    bool readTimeFromString(const std::string& s, MvrTime* time) const;

    /** Parse a GPRMC message (in @a msg) and place results in provided
     * variables. (Can be used by subclasses to store results of GPRMC differently
     * than normal.)*/
    void parseGPRMC(const MvrNMEAParser::Message &msg, double &latitudeResult, double &longitudeResult, 
                    bool &qualityFlagResult, bool &gotPosition, MvrTime &timeGotPositionResult, 
                    MvrTime &gpsTimestampResult, bool &gotSpeedResult, double &speedResult);

};


class MvrRobotPacket;
class MvrRobot;

class MvrSimulatedGPS : public virtual MvrGPS
{
  bool myHaveDummyPosition;
  MvrRetFunctor1C<bool, MvrSimulatedGPS, MvrRobotPacket*> mySimStatHandlerCB;
  MvrRobot *myRobot;
public:
  MVREXPORT MvrSimulatedGPS(MvrRobot *robot = NULL);
  MVREXPORT virtual ~MvrSimulatedGPS();
  void setDummyPosition(double latitude, double longitude) {
    myData.latitude = latitude;
    myData.havePosition = true;
    myData.longitude = longitude;
    if(!myData.haveHDOP) myData.HDOP = 1.0;
    myData.haveHDOP = true;
    if(!myData.haveVDOP) myData.VDOP = 1.0;
    myData.haveVDOP = true;
    if(!myData.havePDOP) myData.PDOP = 1.0;
    myData.havePDOP = true;
    myData.fixType = SimulatedFix;
    myHaveDummyPosition = true;
  }
  void clearDummyPosition() {
    clearPosition();
    myHaveDummyPosition = false;
  }
  void clearPosition() {
    myData.havePosition = false;
    myData.latitude = 0;
    myData.longitude = 0;
    myData.altitude = 0;
    myData.HDOP = 0;
    myData.VDOP = 0;
    myData.PDOP = 0;
    myData.fixType = NoFix;
  }
  void setDummyPosition(double latitude, double longitude, double altitude) {
    myData.altitude = altitude;
    setDummyPosition(latitude, longitude);
  }
  MVREXPORT void setDummyPosition(MvrArgumentBuilder *args); 
  void setDummyPositionFromArgs(MvrArgumentBuilder *args) { setDummyPosition(args); } // non-overloaded function can be used in functors
  MVREXPORT virtual bool connect(unsigned long connectTimeout = 10000);
  virtual bool initDevice() { return true; }
  virtual int read(unsigned long maxTime = 0) {
    if(myHaveDummyPosition)
    {
      myData.timeGotPosition.setToNow();
    }
    return ReadUpdated | ReadFinished;
  }
private:
#ifndef SWIG
  bool handleSimStatPacket(MvrRobotPacket *pkt); 
#endif
};

#endif // MVRGPS_H
