/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrArg.h
 > Description  : This class mostly for actions, could be used for other things
 > Author       : Yu Jie
 > Create Time  : 2017年05月24日
 > Modify Time  : 2017年05月24日
***************************************************************************************************/
#ifndef MVRARG_H
#define MVRARG_H

#include "mvriaTypedefs.h"
#include "mvriaUtil.h"
#include "MvrFunctor.h"

class MvrArgumentBuilder;

class MvrArg
{
public:
  typedef enum{
    INVALID,            ///< An invalid argument, the argument wasn't created correctly
    INT,                ///< Integer argument
    DOUBLE,             ///< Double argument
    STRING,             ///< String argument
    BOOL,               ///< Boolean argument
    POSE,               ///< ArPose argument
    FUNCTOR,            ///< Argument that handles things with functors
    DESCRIPTION_HOLDER, ///< Argument that just holds a description
    
    LAST_TYPE = DESCRIPTION_HOLDER  ///< Last value in the enumeration
  } Type;

  enum{
    TYPE_COUNT = LAST_TYPE + 1  ///< Number of argument types
  };

  /// Default empty constructor
  MVREXPORT MvrArg();
};

#endif  // MVRARG_H