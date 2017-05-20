/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrConfigGroup.h
 > Description  : Container for holding a group of MvrConfigs
 > Author       : Yu Jie
 > Create Time  : 2017年05月20日
 > Modify Time  : 2017年05月20日
***************************************************************************************************/
#ifndef MVRCONFIGGROUP_H
#define MVRCONFIGGROUP_H

#include "MvrConfig.h"
class MvrConfigGroup
{
public:
  /// Constructor
  AREXPORT MvrConfigGroup(const char *baseDirectory = NULL);
  /// Destructor
  AREXPORT ~MvrConfigGroup(void);
  /// Adds a config to the group
  AREXPORT void addConfig(MvrConfig *config);
  /// Removes a config from the group
  AREXPORT void remConfig(MvrConfig *config);
  /// Parses the given file (starting from the base directory)
  AREXPORT bool parseFile(const char *fileName, bool continueOnError = false);
  /// Reloads the last file parsed
  AREXPORT bool reloadFile(bool continueOnError = true);
  /// Writes a file out (overwrites any existing file)
  AREXPORT bool writeFile(const char *fileName);
  /// Sets the base directory on all configs this contains
  AREXPORT void setBaseDirectory(const char *baseDirectory);
  /// Gets the baes directory of this group (not the configs it contains)
  AREXPORT const char *getBaseDirectory(void) const;
protected:
  std::string myBaseDirectory;
  std::string myLastFile;
  std::list<MvrConfig *> myConfigs;
};
#endif  // MVRCONFIGGROUP_H