/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrUrg_2_0.h
 > Description  : Hokuyo Urg laser range device.
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年05月22日
***************************************************************************************************/
#ifndef MVRURG_2_0_H
#define MVRURG_2_0_H

#include "mvriaTypedefs.h"
#include "MvrLaser.h"
#include "MvrDeviceConnection.h"

/** 
    Connects using the Urg's serial port connector or USB.  Note that
    if the max range is 4095 or less it'll use 2 bytes per range
    reading, and if the max range is more than that it'll use 3 bytes
    per range reading.
 */
class MvrUrg_2_0 : public MvrLaser
{
public:
  /// Constructor
  MVREXPORT MvrUrg_2_0(int laserNumber, const char *name = "urg2.0");
  /// Destructor
  MVREXPORT ~MvrUrg_2_0();
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
  MVREXPORT bool setParams(double startingDegrees = -135, double endingDegrees = 135,
	                         double incrementDegrees = 1, bool flipped = false);
  /// Sets the parameters that control what data you get from the urg
  MVREXPORT bool setParamsBySteps(int startingStep = 0, int endingStep = 768, 
	                                int clusterCount = 3, bool flipped = false);
  MVREXPORT virtual void * runThread(void *arg);
  /// internal call to write a string to the urg
  bool writeLine(const char *str);
  /// internal call to read a string from the urg
  bool readLine(char *buf, unsigned int size, unsigned int msWait, 
		            bool noChecksum, bool stripLastSemicolon, MvrTime *firstByte = NULL);

  /// internal call to write a command and get the response back into the buf
  bool sendCommandAndRecvStatus(const char *command, const char *commandDesc, 
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

  std::string myModel;
  int myDMin;
  int myDMax;
  int myARes;
  int myAMin;
  int myAMax;
  int myAFront;
  int myScan;

  double myStepSize;
  double myStepFirst;
  bool myUseThreeDataBytes;

  bool myLogMore;
  
  MvrFunctorC<MvrUrg_2_0> mySensorInterpTask;
  MvrRetFunctorC<bool, MvrUrg_2_0> myMvriaExitCB;
};

#endif // MVRURG_2_0_H