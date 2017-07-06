/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#ifndef ARTRANSFORM_H
#define ARTRANSFORM_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"

/// Perform transforms between different coordinates
/** @ingroup UtilityClasses
*/
class MvrTransform
{
public:
  /// Constructor
  MvrTransform() 
    { 
      myX = 0;
      myY = 0; 
      myTh = 0;
      myCos = MvrMath::cos(myTh);
      mySin = MvrMath::sin(myTh);
    }
  /// Constructor, Sets the transform so points in this coord system
  /// transform to abs world coords

  MvrTransform(MvrPose pose) 
    { 
      setTransform(pose);
    }
  /// Constructor, sets the transform so that pose1 will be
  /// transformed to pose2
  MvrTransform(MvrPose pose1, MvrPose pose2)
    {
      setTransform(pose1, pose2);
    }
  /// Destructor
  virtual ~MvrTransform() {}

  /// Take the source pose and run the transform on it to put it into abs 
  /// coordinates
  /** 
      @param source the parameter to transform
      @return the source transformed into absolute coordinates
  */
  MvrPose doTransform(MvrPose source)
    {
      MvrPose ret;
      ret.setX(myX + myCos * source.getX() + mySin * source.getY());
      ret.setY(myY + myCos * source.getY() - mySin * source.getX());
      ret.setTh(MvrMath::addAngle(source.getTh(),myTh));      
      return ret;
    }
  /// Take the source pose and run the transform on it to put it into abs 
  /// coordinates
  /** 
      @param source the parameter to transform
      @return the source transformed into absolute coordinates
  */
  MvrPoseWithTime doTransform(MvrPoseWithTime source)
    {
      MvrPoseWithTime ret;
      ret.setX(myX + myCos * source.getX() + mySin * source.getY());
      ret.setY(myY + myCos * source.getY() - mySin * source.getX());
      ret.setTh(MvrMath::addAngle(source.getTh(),myTh));      
      ret.setTime(source.getTime());
      return ret;
    }

  /// Take the source pose and run the inverse transform on it, taking it from
  /// abs coords to local
  /** 
      The source and result can be the same
      @param source the parameter to transform
      @return the source transformed from absolute into local coords
  */
  MvrPose doInvTransform(MvrPose source)
    {
      MvrPose ret;
      double tx = source.getX() - myX;
      double ty = source.getY() - myY;
      ret.setX(myCos * tx - mySin * ty);
      ret.setY(myCos * ty + mySin * tx);
      ret.setTh(MvrMath::subAngle(source.getTh(),myTh));      
      return ret;
    }

  /// Take the source pose and run the inverse transform on it, taking it from
  /// abs coords to local
  /** 
      The source and result can be the same
      @param source the parameter to transform
      @return the source transformed from absolute into local coords
  */
  MvrPoseWithTime doInvTransform(MvrPoseWithTime source)
    {
      MvrPoseWithTime ret;
      double tx = source.getX() - myX;
      double ty = source.getY() - myY;
      ret.setX(myCos * tx - mySin * ty);
      ret.setY(myCos * ty + mySin * tx);
      ret.setTh(MvrMath::subAngle(source.getTh(),myTh));      
      ret.setTime(source.getTime());
      return ret;
    }


  /// Take a std::list of sensor readings and do the transform on it
  MVREXPORT void doTransform(std::list<MvrPose *> *poseList);
  /// Take a std::list of sensor readings and do the transform on it
  MVREXPORT void doTransform(std::list<MvrPoseWithTime *> *poseList);
  /// Sets the transform so points in this coord system transform to abs world coords
  MVREXPORT void setTransform(MvrPose pose);
  /// Sets the transform so that pose1 will be transformed to pose2
  MVREXPORT void setTransform(MvrPose pose1, MvrPose pose2);
  /// Gets the transform x value (mm)
  double getX() { return myX; }
  /// Gets the transform y value (mm)
  double getY() { return myY; }
  /// Gets the transform angle value (degrees)
  double getTh() { return myTh; }
  /// Internal function for setting the transform from low level data not poses
  MVREXPORT void setTransformLowLevel(double x, double y, double th);
protected:
  double myX;
  double myY;
  double myTh;
  double myCos;
  double mySin;
};


#endif // ARTRANSFORM_H
