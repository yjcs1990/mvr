#ifndef MVRURG_H
#define MVRURG_H

#include "mvriaTypedefs.h"
#include "MvrLaser.h"
#include "MvrDeviceConnection.h"

/** Hokuyo Urg laser range device.
 *  Connects using the Urg's serial port connector (not USB).
 *  Supports URG-04LX using SCIP 1.1 protocol only. See MvrLaserConnector for instructions on 
 *  using lasers in a program.
    @sa MvrUrg_2_0
    @sa MvrLaserConnector
    @sa MvrLaser
 *  @since 2.7.0
 */
class MvrUrg : public MvrLaser
{
public:
  /// Constructor
  MVREXPORT MvrUrg(int laserNumber,
		 const char *name = "urg");
  /// Destructor
  MVREXPORT ~MvrUrg();
  MVREXPORT virtual bool blockingConnect(void);
  MVREXPORT virtual bool asyncConnect(void);
  MVREXPORT virtual bool disconnect(void);
  MVREXPORT virtual bool isConnected(void) { return myIsConnected; }
  MVREXPORT virtual bool isTryingToConnect(void) 
    { 
      if (myStartConnect)
	return true;
      else if (myTryingToConnect)
	return true; 
      else
	return false;
    }  

  /// Logs the information about the sensor
  MVREXPORT void log(void);
protected:
  /// Sets the parameters that control what data you get from the urg
  MVREXPORT bool setParams(
	  double startingDegrees = -135, double endingDegrees = 135,
	  double incrementDegrees = 1, bool flipped = false);
  /// Sets the parameters that control what data you get from the urg
  MVREXPORT bool setParamsBySteps(
	  int startingStep = 0, int endingStep = 768, int clusterCount = 3,
	  bool flipped = false);
  MVREXPORT virtual void * runThread(void *arg);
  /// internal call to write a string to the urg
  bool writeLine(const char *str);
  /// internal call to read a string from the urg
  bool readLine(char *buf, unsigned int size, unsigned int msWait);

  /// internal call to write a command and get the response back into the buf
  bool sendCommandAndRecvStatus(
	  const char *command, const char *commandDesc, 
	  char *status, unsigned int size, unsigned int msWait);
  
  void sensorInterp(void);
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  MVREXPORT virtual bool laserCheckParams(void);
  MVREXPORT virtual void laserSetName(const char *name);
  
  void failedToConnect(void);
  MvrMutex myReadingMutex;
  MvrMutex myDataMutex;

  MvrTime myReadingRequested;
  std::string myReading;

  int myStartingStep;
  int myEndingStep;
  int myClusterCount;
  bool myFlipped;
  char myRequestString[1024];
  double myClusterMiddleAngle;

  bool internalConnect(void);

  bool internalGetReading(void);

  void clear(void);
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;
  
  std::string myVendor;
  std::string myProduct;
  std::string myFirmwareVersion;
  std::string myProtocolVersion;
  std::string mySerialNumber;
  std::string myStat;
  
  bool myLogMore;
  
  MvrFunctorC<MvrUrg> mySensorInterpTask;
  MvrRetFunctorC<bool, MvrUrg> myMvrExitCB;
};

#endif // ARURG_H
