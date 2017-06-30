/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionTriangleDriveTo.h
 > Description  : Action to drive up to a triangle target found from an MvrLineFinder
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONTRIANGLEDRIVETO_H
#define MVRACTIONTRIANGLEDRIVETO_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrLineFinder.h"

class MvrActionTriangleDriveTo : public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionTriangleDriveTo(const char *name = "triangleDriveTo", 
            double finalDistFromVertex = 400,
            double approachDistFromVertex = 1000,
            double speed = 200, 
            double closeDist = 100,
            double acquireTurnSpeed = 30);
  /// Destructor
  MVREXPORT virtual ~MvrActionTriangleDriveTo();
  /// Sets if we will turn to see if we can acquire the triangle
  void setAcquire(bool acquire = false) { myAcquire = acquire; }
  /// Gets if we will turn to see if we can acquire the triangle
  bool getAcquire(void) { return myAcquire; }
  /// Sets the final distance from vertex
  void setFinalDistFromVertex(double dist) { myFinalDistFromVertex = dist; }
  /// Sets the final distance from vertex
  double getFinalDistFromVertex(void) { return myFinalDistFromVertex; }
  /// Sets the parameter of the triangle we're looking for
  void setTriangleParams(double line1Length = 254, 
                         double angleBetween = 135,
                         double line2Length = 254)
  { myLine1Length = line1Length; myAngleBetween = angleBetween; myLine2Length = line2Length; }
  MVREXPORT void setParameters(double finalDistFromVertex = 400,
                               double approachDistFromVertex = 1000,
                               double speed = 200, 
                               double closeDist = 100,
                               double acquireTurnSpeed = 30);

  /// Gets whether we adjust the vertex or not
  bool getAdjustVertex(void) { return myAdjustVertex; }
  /// Sets whether we adjust the vertex or not
  void setAdjustVertex(bool adjustVertex) { myAdjustVertex = adjustVertex; }
  /// Sets the vertex offset
  void setVertexOffset(int localXOffset, int localYOffset, double thOffset) 
  {
    myLocalXOffset = localXOffset; myLocalYOffset = localYOffset; 
    myThOffset = thOffset; 
  } 
  /// Sets if we should use the legacy vertex mode or not
  void setUseLegacyVertexOffset(bool useLegacyVertexOffset)
  { myUseLegacyVertexOffset = useLegacyVertexOffset; }
  /// Gets if we are use the legacy vertex mode or not
  bool getUseLegacyVertexOffset(void) { return myUseLegacyVertexOffset; }
  /// Gets whether it always goto the vertex and not the point in front
  bool getGotoVertex(void) { return myGotoVertex; }
  /// Sets whether it always goto the vertex and not the point in front
  void setGotoVertex(bool gotoVertex) { myGotoVertex = gotoVertex; }
  /// Sets the distance to the triangle at which we start ignoring it
  void setIgnoreTriangleDist(double dist = 250, bool useIgnoreInGotoVertexMode = false) 
  {
    myIgnoreTriangleDist = dist; 
    myUseIgnoreInGoto = useIgnoreInGotoVertexMode; 
  }
  /// Gets the distance to the triangle at which we start ignoring it
  double getIgnoreTriangleDist(void) { return myIgnoreTriangleDist; }
  /// Gets if we're ignoring the triangle in goto vertex mode
  bool getUseIgnoreInGotoVertexMode(void) { return myUseIgnoreInGoto; }
  /// How long to keep going without having seen the vertex (0 is no timeout)
  void setVertexUnseenStopMSecs(int vertexUnseenStopMSecs = 4000)
  { myVertexUnseenStopMSecs = vertexUnseenStopMSecs; }
  /// How long to keep going without having seen the vertex (0 is no timeout)
  int getVertexUnseenStopMSecs(void) { return myVertexUnseenStopMSecs; }
  /// Sets the maximum distance between points in a line
  void setMaxDistBetweenLinePoints(int maxDistBetweenLinePoints = 0) 
  { myMaxDistBetweenLinePoints = maxDistBetweenLinePoints; }
  /// Sets the maximum lateral distance from the robot to the triangle line
  void setMaxLateralDist(int maxLateralDist = 0)
  { myMaxLateralDist = maxLateralDist; }
  /// Sets the maximum angle misalignment from the robot to the triangle line
  void setMaxAngleMisalignment(int maxAngleMisalignment = 0)
  { myMaxAngleMisalignment = maxAngleMisalignment; }
  enum State { 
    STATE_INACTIVE,       ///< we aren't trying
    STATE_ACQUIRE,        ///< finding the target
    STATE_SEARCHING,      ///< Turning trying to find the target
    STATE_GOTO_APPROACH,  ///< driving to the approach point
    STATE_ALIGN_APPROACH, ///< aligning to the target the first time
    STATE_GOTO_VERTEX,    ///< driving to the vertex
    STATE_GOTO_FINAL,     ///< driving to the final point
    STATE_ALIGN_FINAL,    ///< aligning to the target the final time
    STATE_SUCCEEDED,      ///< pointing at the target
    STATE_FAILED          ///< if we're not acquiring and we lost the vertex we fail
  };
  /// Gets the state
  State getState(void) { return myState; }
  /// Gets if we've seen the vertex ever for this attempted drive (it gets reset in searching, but that's the only time, so will only be set once an activation by default (unless setAcquire is called)
  bool getVertexSeen(void) { return myVertexSeen; }
  /// Sets the line finder to use
  MVREXPORT void setLineFinder(MvrLineFinder *lineFinder);
  /// Sets the line finder used
  MVREXPORT MvrLineFinder *getLineFinder(void) { return myLineFinder; }
  
  /// Sets whether we're logging the driving or not
  void setLogging(bool logging) { myPrinting = logging; }
  /// Gets whether we're logging the driving or not
  bool setLogging(void) { return myPrinting; }
  MVREXPORT virtual void activate(void);
  MVREXPORT virtual void deactivate(void);
  MVREXPORT virtual void setRobot(MvrRobot *robot);
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
  { return &myDesired; }
#endif

#ifndef SWIG

  class Data 
  {
    public:
    Data() 
    { myFinalSet = false; myApproachSet = false; myLinesAndVertexSet = false; 
    myVertexSeenThisCycle = false; myVertexSeen = false; }
    ~Data() {}
    void setLinesAndVertex(std::map<int, MvrLineFinderSegment *> *lines, MvrPose vertex)
    {
      myLines.clear();
      std::map<int, MvrLineFinderSegment *>::iterator it;
      for (it = lines->begin(); it != lines->end(); it++)
      {
	      myLines.push_front(*(*it).second);
      }
      myVertex = vertex;
      myLinesAndVertexSet = true;
    }
    void setFinal(MvrPose final) { myFinalSet = true; myFinal = final; }
    void setApproach(MvrPose approach) 
    { myApproachSet = true; myApproach = approach; }
    void setVertexSeenThisCycle(bool vertexSeenThisCycle)
    { myVertexSeenThisCycle = vertexSeenThisCycle; }
    void setVertexSeen(bool vertexSeen)
    { myVertexSeen = vertexSeen; }


    bool myLinesAndVertexSet;
    std::list<MvrLineSegment> myLines;
    MvrPose myVertex;
    bool myFinalSet;
    MvrPose myFinal;
    bool myApproachSet;
    MvrPose myApproach;
    bool myVertexSeenThisCycle;
    bool myVertexSeen;
  };
  void setSaveData(bool saveData) { mySaveData = saveData; }
  bool getSaveData(void) { return mySaveData; }
  Data *getData(void);
#endif // SWIG
protected:
  // Find our triangle, Internal function
  MVREXPORT void findTriangle(bool initial, bool goStraight = false);
  // Finds a pose this dist from the vertex along the line from the vertex
  MVREXPORT MvrPose findPoseFromVertex(double distFromVertex);
  // where the vertex was in encoder coordinates
  MvrPose myVertex;

  MvrRangeDevice *myLaser;
  //MvrLaser *myLaser;
  MvrMutex myDataMutex;
  bool mySaveData;
  Data *myData;
  std::map<int, MvrLineFinderSegment *> *myLines;
  unsigned int myGotLinesCounter;
  bool myVertexSeen;
  bool myPrinting;
  MvrTime myVertexSeenLast;
  State myState;
  double myOriginalAngle;
  bool myAcquire;
  double myFinalDistFromVertex;
  double myApproachDistFromVertex;
  double mySpeed;
  double myCloseDist;
  double myAcquireTurnSpeed;
  bool myTwoStageApproach;
  double myLine1Length;
  double myAngleBetween;
  double myLine2Length;
  int myVertexUnseenStopMSecs;
  MvrActionDesired myDesired;
  MvrLineFinder *myLineFinder;
  bool myOwnLineFinder;
  bool myAdjustVertex;
  bool myGotoVertex;
  int myLocalXOffset;
  int myLocalYOffset;
  double myThOffset;
  bool myUseLegacyVertexOffset;
  double myIgnoreTriangleDist;
  bool myUseIgnoreInGoto;
  int myMaxDistBetweenLinePoints;
  int myMaxLateralDist;
  int myMaxAngleMisalignment;
};

#endif  // MVRACTIONTRIANGLEDRIVETO_H
