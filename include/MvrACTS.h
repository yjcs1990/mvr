#ifndef MVRACTS_H
#define MVRACTS_H

#include "mvriaTypedefs.h"
#include "MvrTcpConnection.h"
#include "mvriaUtil.h"
#include "MvrRobot.h"


/// A class for the acts blob
class MvrACTSBlob
{
public:
  /// Constructor
  MvrACTSBlob() {}
  /// Destructor
  virtual ~MvrACTSBlob() {}
  /// Gets the number of pixels (area) covered by the blob
  int getArea(void) { return myArea; }
  /// Gets the X Center of Gravity of the blob
  int getXCG(void) { return myXCG; }
  /// Gets the Y Center of Gravity of the blob
  int getYCG(void) { return myYCG; }
  /// Gets the left border of the blob
  int getLeft(void) { return myLeft; }
  /// Gets the right border of the blob
  int getRight(void) { return myRight; }
  /// Gets the top border of the blob
  int getTop(void) { return myTop; }
  /// Gets the bottom border of the blob
  int getBottom(void) { return myBottom; }
  /// Sets the number of pixels (area) covered by the blob
  void setArea(int area) { myArea = area; }
  /// Sets the X Center of Gravity of the blob
  void setXCG(int xcg) { myXCG = xcg; }
  /// Sets the Y Center of Gravity of the blob
  void setYCG(int ycg) { myYCG = ycg; }
  /// Sets the left border of the blob
  void setLeft(int left) { myLeft = left; }
  /// Sets the right border fo the blob
  void setRight(int right) { myRight = right; }
  /// Sets the top border of the blob
  void setTop(int top) { myTop = top; }
  /// Sets the bottom border of the blob
  void setBottom(int bottom) { myBottom = bottom; }
  /// Prints the stats of the blob
  void log(void)
    {
      MvrLog::log(MvrLog::Terse, "Area: %3d X: %3d Y: %3d l: %3d r: %3d t: %3d: b: %3d", 
		 myArea, myXCG, myYCG, myLeft, myRight, myTop, myBottom);
    }
protected:
  int myArea; 
  int myXCG;
  int myYCG;
  int myLeft;
  int myRight;
  int myTop;
  int myBottom;
};

/// Communicate with ACTS
/// @ingroup OptionalClasses
class MvrACTS_1_2
{
public:
  /// Constructor
  MVREXPORT MvrACTS_1_2();
  /// Destructor
  MVREXPORT virtual ~MvrACTS_1_2();

  /// Opens the connection to ACTS
  MVREXPORT bool openPort(MvrRobot *robot, const char *host = "localhost", int port = 5001);
  /// Closes the connection
  MVREXPORT bool closePort(void);

  /// Finds out whether there is connection
  MVREXPORT bool isConnected(void);

  /// Gets the robot this class is connected to
  MVREXPORT MvrRobot *getRobot(void);
  /// Sets the robot this class is connected to
  MVREXPORT void setRobot(MvrRobot *robot);

  /// Requests another packet
  MVREXPORT bool requestPacket(void);
  /// Requests that ACTS quits
  MVREXPORT bool requestQuit(void);
  /// Gets the blob information from the connection to acts
  MVREXPORT bool receiveBlobInfo(void);
  
  /// Gets the number of blobs for the given chanel
  MVREXPORT int getNumBlobs(int channel);
  
  /// Gets the given blob from the given channel
  MVREXPORT bool getBlob(int channel, int blobNumber, MvrACTSBlob *blob);

  /// A function that reads information from acts and requests packets
  MVREXPORT void actsHandler(void);
  
  enum ActsConstants
  {
    NUM_CHANNELS = 32, ///< Number of channels there are
    MAX_BLOBS = 10, ///< Number of blobs per channel
    ACTS_BLOB_DATA_SIZE = 16, ///< Size of the blob data
    DATA_HEADER = NUM_CHANNELS * 4, ///< Size of the data header
    MAX_DATA = 5300 ///< Maximum amount of data
  };
  /// This will make the image stats inverted (for use with an inverted camera)
  MVREXPORT void invert(int width = 160, int height = 120);
protected:
  int invertX(int before);
  int invertY(int before);
  /// an iternal function to strip out the information from some bytes
  int getData(char *rawData);
  MvrFunctorC<MvrACTS_1_2> mySensorTaskCB;
  MvrRobot *myRobot;
  MvrTcpConnection myConn;
  int myBlobNum[NUM_CHANNELS];
  int myBlobIndex[NUM_CHANNELS];
  char myData[MAX_DATA];
  bool myBlobsBad;
  bool myRequested;
  // these are for flipping the image
  bool myInverted;
  int myHeight;
  int myWidth;
};

#endif
