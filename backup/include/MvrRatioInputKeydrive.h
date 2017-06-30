/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrRatioInputKeydrive.h
 > Description  : This will use the keyboard arrow keys and the MvrActionRatioInput to drive the robot
 > Author       : Yu Jie
 > Create Time  : 2017年05月25日
 > Modify Time  : 2017年05月25日
***************************************************************************************************/
#ifndef MVRRATIOINPUTKEYDRIVE_H
#define MVRRATIOINPUTKEYDRIVE_H

#include "mvriaTypedefs.h"
#include "MvrAction.h"
#include "MvrFunctor.h"
#include "MvrActionRatioInput.h"

/*
   You have to make an MvrActionRatioInput and add it to the robot like
   a normal action for this to work.
*/
class MvrRatioInputKeydrive 
{
public:
  /// Constructor
  MVREXPORT MvrRatioInputKeydrive(MvrRobot *robot, MvrActionRatioInput *input, 
			                          	int priority = 25, double velIncrement = 5);
  /// Destructor
  MVREXPORT virtual ~MvrRatioInputKeydrive();
  /// Takes the keys this action wants to use to drive
  MVREXPORT void takeKeys(void);
  /// Gives up the keys this action wants to use to drive
  MVREXPORT void giveUpKeys(void);
  /// Internal, callback for up arrow
  MVREXPORT void up(void);
  /// Internal, callback for down arrow
  MVREXPORT void down(void);
  /// Internal, callback for z
  MVREXPORT void z(void);
  /// Internal, callback for x 
  MVREXPORT void x(void);
  /// Internal, callback for left arrow
  MVREXPORT void left(void);
  /// Internal, callback for right arrow
  MVREXPORT void right(void);
  /// Internal, callback for space key
  MVREXPORT void space(void);
  /// Internal, gets our firecb
  MVREXPORT MvrFunctor *getFireCB(void) { return &myFireCB; }
protected:
  MVREXPORT void activate(void);
  MVREXPORT void deactivate(void);
  MVREXPORT void fireCallback(void);
  MvrFunctorC<MvrRatioInputKeydrive> myUpCB;
  MvrFunctorC<MvrRatioInputKeydrive> myDownCB;
  MvrFunctorC<MvrRatioInputKeydrive> myLeftCB;
  MvrFunctorC<MvrRatioInputKeydrive> myRightCB;
  MvrFunctorC<MvrRatioInputKeydrive> myZCB;
  MvrFunctorC<MvrRatioInputKeydrive> myXCB;
  MvrFunctorC<MvrRatioInputKeydrive> mySpaceCB;

  double myPrinting;
  double myTransRatio;
  double myRotRatio;
  double myThrottle;
  double myLatRatio;

  MvrRobot *myRobot;
  bool myHaveKeys;
  double myVelIncrement;
  double myLatVelIncrement;
  MvrActionRatioInput *myInput;
  MvrFunctorC<MvrRatioInputKeydrive> myFireCB;
  MvrFunctorC<MvrRatioInputKeydrive> myActivateCB;
  MvrFunctorC<MvrRatioInputKeydrive> myDeactivateCB;
};


#endif // MVRRATIOINPUTKEYDRIVE_H
