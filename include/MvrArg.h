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
    POSE,               ///< MvrPose argument
    FUNCTOR,            ///< Argument that handles things with functors
    DESCRIPTION_HOLDER, ///< Argument that just holds a description
    
    LAST_TYPE = DESCRIPTION_HOLDER  ///< Last value in the enumeration
  } Type;

  enum{
    TYPE_COUNT = LAST_TYPE + 1  ///< Number of argument types
  };

  /// Default empty constructor
  MVREXPORT MvrArg();
  /// Constructor for making a integer argument
  MVREXPORT MvrArg(const char *name, int *pointer,
                   const char *description="",
                   int minInt = INT_MIN,
                   int maxInt = INT_MAX);
  /// Constructor for making a double argument
  MVREXPORT MvrArg(const char *name, double *pointer,
                   const char *description="",
                   double minDouble = -HUGE_VAL,
                   double maxDouble = HUGE_VAL);
  /// Constructor for making a boolean argument
  MVREXPORT MvrArg(const char *name, bool *pointer,
                   const char *description="");
  /// Constructor for making a position argument
  MVREXPORT MvrArg(const char *name, MvrPose *pointer,
                   const char *description="");
  /// Constructor for making an argument of a string
  MVREXPORT MvrArg(const char *name, char *pointer,
                   const char *description, size_t maxStrLen);
  /// Constructor for making an argument that has functors to handle things
  MVREXPORT MvrArg(const char *name, 
            MvrRetFunctor1<bool, MvrArgumentBuilder *> *setFunctor,
            MvrRetFunctor<const std::list<MvrArgumentBuilder *> *> *getFunctor,
            const char *description);
  /// Constructor for just holding a description
  MVREXPORT MvrArg(const char *description);
  /// Copy constructor
  MVREXPORT MvrArg(const MvrArg &arg);
  /// Assignment operator
  MVREXPORT MvrArg &operator=(const MvrArg &arg);
  /// Destructor
  MVREXPORT virtual ~MvrArg();

  /// Gets the type of the argument
  MVREXPORT Type getType(void) const;
  /// Gets the name of the argument
  MVREXPORT const char *getName(void) const;
  /// Gets the long description of the argument
  MVREXPORT const char *getDescription(void) const;
  /// Sets the argument value, for int arguments
  MVREXPORT bool setInt(int val);
  /// Sets the argument value, for double arguments
  MVREXPORT bool setDouble(double val);
  /// Sets the argument value, for bool arguments
  MVREXPORT bool setBool(bool val);
  /// Sets the argument value, for MvrPose arguments
  MVREXPORT bool setPose(MvrPose pose);
  /// Sets the argument value for MvrArgumentBuilder arguments
  MVREXPORT bool setString(const char *str);
  /// Sets the argument by calling the setFunctor callback
  MVREXPORT bool setArgWithFunctor(MvrArgumentBuilder *argument);
  /// Gets the argument value, for int arguments
  MVREXPORT int getInt(void) const; 
  /// Gets the argument value, for double arguments
  MVREXPORT double getDouble(void) const;
  /// Gets the argument value, for bool arguments
  MVREXPORT bool getBool(void) const;
  /// Gets the argument value, for pose arguments
  MVREXPORT MvrPose getPose(void) const;
  /// Gets the argument value, for string arguments
  MVREXPORT const char *getString(void) const;
  /// Gets the argument value, which is a list of argument builders here
  MVREXPORT const std::list<MvrArgumentBuilder *> *getArgsWithFunctor(void) const;
  /// Logs the type, name, and value of this argument
  MVREXPORT void log(void) const;
  /// Gets the minimum int value
  MVREXPORT int getMinInt(void) const;
  /// Gets the maximum int value
  MVREXPORT int getMaxInt(void) const;
  /// Gets the minimum double value
  MVREXPORT double getMinDouble(void) const;
  /// Gets the maximum double value
  MVREXPORT double getMaxDouble(void) const;
  /// Gets if the config priority is set
  MVREXPORT bool getConfigPrioritySet(void) const;
  /// Gets the priority (only used by MvrConfig)
  MVREXPORT MvrPriority::Priority getConfigPriority(void) const;
  /// Sets the priority (only used by MvrConfig)
  MVREXPORT void setConfigPriority(MvrPriority::Priority priority);

private:
  /// Internal helper function
  MVREXPORT void clear(void);
protected:
  MvrArg::Type myType;
  std::string myName;
  std::string myDescription;
  int *myIntPointer;
  int myMinInt, myMaxInt;
  double *myDoublePointer;
  double myMinDouble, myMaxDouble;
  bool *myBoolPointer;
  MvrPose *myPosePointer;
  char *myStringPointer;
  size_t myMaxString;
  bool myConfigPrioritySet;
  MvrPriority::Priority myConfigPriority;
  MvrRetFunctor1<bool, MvrArgumentBuilder *> *mySetFunctor;
  MvrRetFunctor<const std::list<MvrArgumentBuilder *> *> *myGetFunctor;
};

#endif  // MVRARG_H