/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrActionStallRecover.h
 > Description  : Action to recover from a stall
 > Author       : Yu Jie
 > Create Time  : 2017年04月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRACTIONSTALLRECOVER_H
#define MVRACTIONSTALLRECOVER_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"

class MvrResolver;

class MvrActionStallRecover : public public MvrAction
{
public:
  /// Constructor
  MVREXPORT MvrActionStallRecover(const char * name = "stall recover", 
                                  double obstacleDistance = 225, int cyclesToMove = 50, 
                                  double speed = 150, double degreesToTurn = 45,
                                  bool enabled = true);
  /// Destructor
  MVREXPORT virtual ~MvrActionStallRecover();
  MVREXPORT virtual MvrActionDesired *fire(MvrActionDesired currentDesired);
  MVREXPORT virtual MvrActionDesired *getDesired(void) 
  { return &myActionDesired; }
#ifndef SWIG
  MVREXPORT virtual const MvrActionDesired *getDesired(void) const 
  { return &myActionDesired; }
#endif
  MVREXPORT void addToConfig(MvrConfig* config, const char* sectionName, MvrPriority::Priority priority = MvrPriority::NORMAL);
  MVREXPORT virtual void activate(void);
protected:
  // these are internal things, don't touch unless you know what you are doing
  void doit(void); // does whatever should be done
  void addSequence(int sequence);
  int myDoing; // what we're doing, uses the stuff from the enum What
  int myState; // holds the state
  int myCount; // count down variable, -1 if first time in this state
  int mySideStalled; // 1 for left, 2 for right, 3 for both

  enum State 
  { 
    STATE_NOTHING = 0, // waiting
    STATE_GOING // do something
  };

  enum What
  {
    BACK=0x1, // back up
    FORWARD=0x2, // go forward
    TURN=0x4, // turn away from obstacles
    TURN_LEFT=0x8, // turn left
    TURN_RIGHT=0x10, // turn right
    MOVEMASK = BACK | FORWARD,
    TURNMASK = TURN | TURN_LEFT | TURN_RIGHT
  };

  std::map<int, int> mySequence; // list of things to do as stall continues
  int mySequenceNum;
  int mySequencePos;
  time_t myLastFired;
  double myObstacleDistance;
  int myCyclesToMove;
  bool myEnabled;
  double mySpeed;
  int myCyclesToTurn;
  double myDegreesToTurn;
  double myDesiredHeading;
  MvrActionDesired myActionDesired;
  
  MvrResolver *myResolver;
};

#endif  // MVRACTIONSTALLRECOVER_H