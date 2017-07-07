#ifndef MVRRESOLVER_H
#define MVRRESOLVER_H

#include "mvriaTypedefs.h"
#include "MvrActionDesired.h"
#include <string>

class MvrAction;
class MvrRobot;

/// Resolves a list of actions and returns what to do
/**
  MvrResolver::resolve() is the function that MvrRobot
  calls with the action list in order
  to produce a combined MvrActionDesired object from them, according to
  the subclass's particular algorithm or policy.
*/
class MvrResolver
{
public:
  /// Constructor
  typedef std::multimap<int, MvrAction *> ActionMap;
  MvrResolver(const char *name, const char * description = "")
    { myName = name; myDescription = description; }
  /// Desturctor
  virtual ~MvrResolver() {};
  /// Figure out a single MvrActionDesired from a list of MvrAction s
  virtual MvrActionDesired *resolve(ActionMap *actions, MvrRobot *robot,
				   bool logActions = false) = 0;
  /// Gets the name of the resolver
  virtual const char *getName(void) const { return myName.c_str(); }
  /// Gets the long description fo the resolver
  virtual const char *getDescription(void) const { return myDescription.c_str(); }
  
protected:
  std::string myName;
  std::string myDescription;
};

#endif
