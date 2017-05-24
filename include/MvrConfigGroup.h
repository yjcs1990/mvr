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
  MVREXPORT MvrConfigGroup(const char *baseDirectory = NULL);
  /// Destructor
  MVREXPORT ~MvrConfigGroup(void);
  /// Adds a config to the group
  MVREXPORT void addConfig(MvrConfig *config);
  /// Removes a config from the group
  MVREXPORT void remConfig(MvrConfig *config);
  /// Parses the given file (starting from the base directory)
  MVREXPORT bool parseFile(const char *fileName, bool continueOnError = false);
  /// Reloads the last file parsed
  MVREXPORT bool reloadFile(bool continueOnError = true);
  /// Writes a file out (overwrites any existing file)
  MVREXPORT bool writeFile(const char *fileName);
  /// Sets the base directory on all configs this contains
  MVREXPORT void setBaseDirectory(const char *baseDirectory);
  /// Gets the baes directory of this group (not the configs it contains)
  MVREXPORT const char *getBaseDirectory(void) const;
protected:
  std::string myBaseDirectory;
  std::string myLastFile;
  std::list<MvrConfig *> myConfigs;
};
#endif  // MVRCONFIGGROUP_H