#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrConfigGroup.h"
#include "MvrArgumentBuilder.h"
#include "MvrLog.h"

MVREXPORT MvrConfigGroup::MvrConfigGroup(const char *baseDirectory)
{
  if (baseDirectory != NULL)
    myBaseDirectory = baseDirectory;
  else
    myBaseDirectory = "";
}

MVREXPORT MvrConfigGroup::~MvrConfigGroup(void)
{
  
}

MVREXPORT void MvrConfigGroup::addConfig(MvrConfig *config)
{
  myConfigs.push_back(config);
}

MVREXPORT void MvrConfigGroup::remConfig(MvrConfig *config)
{
  myConfigs.remove(config);
}

MVREXPORT bool MvrConfigGroup::parseFile(const char *fileName, 
				       bool continueOnError)
{
  std::list<MvrConfig *>::iterator it;
  bool ret = true;

  myLastFile = fileName;
  // go through all the configs and set the base directory (we don't
  // do it when we're parsing just so that whether it suceeds or fails
  // its the same behavior in this base directory regard)
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    (*it)->setBaseDirectory(myBaseDirectory.c_str());
  }
  // now we go through and parse files... if we get an error we stop
  // if we're supposed to
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    if (!(*it)->parseFile(fileName, continueOnError))
    {
      // if we are continuing on errors we still want to tell them we
      // had an error
      ret = false;
      // if we aren't continuing on error then just return
      if (!continueOnError)
	return false;
    }
  }
  return ret;
}

MVREXPORT bool MvrConfigGroup::reloadFile(bool continueOnError)
{
  return parseFile(myLastFile.c_str(), continueOnError);
}

MVREXPORT bool MvrConfigGroup::writeFile(const char *fileName)
{
  std::set<std::string> alreadyWritten;
  std::list<MvrConfig *>::iterator it;
  bool ret = true;
  bool append = false;
  
  // go through all the configs and set the base directory (we don't
  // do it when we're parsing just so that whether it suceeds or fails
  // its the same behavior in this base directory regard)
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    (*it)->setBaseDirectory(myBaseDirectory.c_str());
  }
  // now we go through and parse files... if we get an error we stop
  // if we're supposed to
  for (it = myConfigs.begin(); it != myConfigs.end(); it++)
  {
    MvrLog::log(MvrLog::Verbose, "Writing config file");
    if (!(*it)->writeFile(fileName, append, &alreadyWritten))
    {
      // if we are continuing on errors we still want to tell them we
      // had an error
      ret = false;
    }
    append = true;
  }
  return ret;
  
}

MVREXPORT void MvrConfigGroup::setBaseDirectory(const char *baseDirectory)
{
  myBaseDirectory = baseDirectory;
}

MVREXPORT const char *MvrConfigGroup::getBaseDirectory(void) const
{
  return myBaseDirectory.c_str();
}
