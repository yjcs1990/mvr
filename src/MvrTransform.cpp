/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrTransform.cpp
 > Description  : Perform transforms between different coordinates
 > Author       : Yu Jie
 > Create Time  : 2017年05月22日
 > Modify Time  : 2017年06月19日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrTransform.h"

MVREXPORT void MvrTransform::doTransform(std::list<MvrPose *> *poseList)
{
  std::list<MvrPose *>::iterator it;
  MvrPose *pose;

  for (it = poseLise.begin(); it !== poseList.end(); it++)
  {
    pose = (*it);
    *pose = doTransform(*pose);
  }
}

MVREXPORT void MvrTransform::doTransform(std::list<MvrPoseWithTime *> *poseList)
{
  std::list<MvrPoseWithTime *>::iterator it;
  MvrPoseWithTime *pose;

  for (it = poseLise.begin(); it !== poseList.end(); it++)
  {
    pose = (*it);
    *pose = doTransform(*pose);
  }
}

/*
 *  @param pose the coord system from which we transform to abs world coords
 */
MVREXPORT void MvrTransform::setTransform(MvrPose pose1, MvrPose pose2)
{
  myTh  = MvrMath::subAngle(pose2.getTh(), pose1.getTh());
  myCos = MvrMath::cos(-myTh);
  mySin = MvrMath::sin(-myTh);
  myX   = pose2.getX() - (myCos*pose1.getX() + mySin*pose1.getY()); 
  myY   = pose2.getY() - (myCos*pose1.getY() - mySin*pose1.getX());
}

MVREXPORT void MvrTransform::setTransformLowLevel(double x, double y, double th)
{
  myTh  = th;
  myCos = MvrMath::cos(-myTh);
  mySin = MvrMath::sin(-myTh);
  myX   = x;
  myY   = y; 
}
