/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrConfig.cpp
 > Description  : Stores configuration information which may be read to and from files or other sources
 > Author       : Yu Jie
 > Create Time  : 2017年04月20日
 > Modify Time  : 2017年06月02日
***************************************************************************************************/
#include "MvrExport.h"
#include "mvriaOSDef.h"
#include "MvrConfig.h"
#include "MvrArgumentBuilder.h"
#include "MvrLog.h"

#include <string>

#if (defined(MVRDEBUG_CONFIG))
// #if (defined(_DEBUG) && defined(MVRDEBUG_CONFIG))
#define IFDEBUG(code) {code;}
#else
#define IFDBUG(code)
#endif  // MVRDEBUG_CONFIG

const char *MvrConfig::CURRENT_CONFIG_VERSION = "2.0";
const char *MvrConfig::CONFIG_VERSION_TAG = "ConfigVersion";

const char *MvrConfig::CURRENT_RESOURCE_VERSION = "1.1";
const char *MvrConfig::RESOURCE_VERSION_TAG = "ConfigResourceVersion";

MVREXPORT const char *MvrConfig::CATEGORY_ROBOT_INTERFACE = "Robot Interface";
MVREXPORT const char *MvrConfig::CATEGORY_ROBOT_OPERATION = "Robot Operation";
MVREXPORT const char *MvrConfig::CATEGORY_ROBOT_PHYSICAL = "Robot Physical";
MVREXPORT const char *MvrConfig::CATEGORY_FLEET = "Enterprise";
MVREXPORT const char *MvrConfig::CATEGORY_SECURITY = "Security";
MVREXPORT const char *MvrConfig::CATEGORY_DEBUG = "Debug";

MVREXPORT const char *MvrConfig::toCategoryName(const char *categoryName)
{
  if (categoryName == NULL)
    return NULL;
  if (MvrUtil::strcasecmp(categoryName, CATEGORY_ROBOT_INTERFACE) == 0){
    return CATEGORY_ROBOT_INTERFACE;
  }
  else if (MvrUtil::strcasecmp(categoryName, CATEGORY_ROBOT_OPERATION) == 0){
    return CATEGORY_ROBOT_OPERATION;
  }
  else if (MvrUtil::strcasecmp(categoryName, CATEGORY_ROBOT_PHYSICAL) == 0){
    return CATEGORY_ROBOT_PHYSICAL;
  }
  else if (MvrUtil::strcasecmp(categoryName, CATEGORY_FLEET) == 0){
    return CATEGORY_FLEET;
  }
  else if (MvrUtil::strcasecmp(categoryName, CATEGORY_DEBUG) == 0){
    return CATEGORY_DEBUG;
  }
  return NULL;
}

/*
 * @param baseDirectory a directory to search for files in
 * @param noBlanksBetweenParams if there should not be blank lines between params in output
 * @param ignoreBounds if this is true bounds checking will be ignored when the file is read in. 
 *        this should ONLY be used by developers debugging
 * @param failOnBadSection if this is true and there is a bad section, then parseFile will fail
 * @param saveUnknown if this is true and there are unknown parameters or sections then they will be saved,
 * if false then they will be ignored (can also be set with setSaveUnknown())
 */
MVREXPORT MvrConfig::MvrConfig(const char *baseDirectory, 
                               bool noBlanksBetweenParams,
                               bool ignoreBounds, 
                               bool failOnBadSection, 
                               bool saveUnknown) : myRobotName(""),
                                                   myConfigName(""),
                                                   myLogPrefix("MvrConfig: "),
                                                   myArgumentParser(NULL),
                                                   myProcessFileCBList(),
                                                   myNoBlanksBetweenParams(noBlanksBetweenParams),
                                                   myConfigVersion(),
                                                   myTranslator(NULL),
                                                   myCsvSeparatorChar('\t'),
                                                   mySection(),
                                                   mySectionToParse(NULL),
                                                   mySectionsNotToParse(),
                                                   myHighestPriorityToParse(MvrPriority::FIRST_PRIORITY),
                                                   myLowestPriorityToParse(MvrPriority::LAST_PRIORITY),

                                                   myRestartLevelNeeded(MvrConfigArg::NO_RESTART);
                                                   myCheckingForRestartLevel(true);

                                                   mySectionBroken(false);
                                                   mySectionIgnored(false);
                                                   myUsingSections(false);

                                                   myParsingListNames(),
                                                   myIsParsingListBroken(false),

                                                   myFileName(),
                                                   myBaseDirectory(),

                                                   myParser(NULL),

                                                   myIgnoreBounds(ignoreBounds);
                                                   myFailOnBadSection(failOnBadSection),

                                                   myDuplicateParams(false),

                                                   mySaveUnknown(saveUnknown),
                                                   myIsQuiet(false),

                                                   myPermissionAllowFactory(true),
                                                   myPermissionSaveUnknown(saveUnknown),
                                                   myProcessFileCallbacksLogLevel(MvrLog::Verbose),

                                                   myCategoryToSectionsMap(),
                                                   mySections(),

                                                   myParserCB(this, &MvrConfig::parseArgument),
                                                   myVersionCB(this, &MvrConfig::parseVersion),
                                                   mySectionCB(this, &MvrConfig::parseSection),
                                                   myListBeginCB(this, &MvrConfig::parseListBegin),
                                                   myListEndCB(this, &MvrConfig::parseListEnd),
                                                   myUnknownCB(this, &MvrConfig::parseUnknown)
{
  /* taking this part out and just calling 'addParsrHandler' so all the codes in one spot
  if (!myParser.addHandlerWithError("section", &mySectionCB))
  {
    MvrLog::log(MvrLog::Normal, "Could not add section to file parser, horrific failure");
  }
  */
  addparserHandlers();

  myPermissionAllowFactory = true;
  myPermissionSaveUnknown = saveUnknown;

  myArgumentParser = NULL;
  mySaveUnknown = saveUnknown;
  myIsQuiet = false;
  setBaseDirectory(baseDirectory);
  myIgnoreBounds = ignoreBounds;
  myFailOnBadSection = failOnBadSection;
  myProcessFileCallbacksLogLevel = MvrLog::Verbose;
  myUsingSections = false;
  mySectionBroken = false;
  mySectionIgnored = false;
  myDuplicateParams = false;

  myParserCB.setName("MvrConfig::parseArgument");
  myVersionCB.setName("MvrConfig::parseVersion");
  mySectionCB.setName("MvrConfig::parseSection");
  myListBeginCB.setName("MvrConfig::parseListBegin");
  myListEndCB.setName("MvrConfig::parseListEnd");
  myUnknownCB.setName("MvrConfig::parseUnknown");

  mySection = "";
}

MVREXPORT MvrConfig::~MvrConfig()
{
  clearAll();
}

MVREXPORT MvrConfig::MvrConfig(const MvrConfig &config) :
          myRobotName(""),
          myConfigName(""),
          myLogPrefix("MvrConfig: "),

          myArgumentParser(NULL),
          myProcessFileCBList(),
          myNoBlanksBetweenParams(config.noBlanksBetweenParams),

          myConfigVersion(config.myConfigVersion),
          myTranslator(config.myTranslator),
          myCsvSeparatorChar(config.myCsvSeparatorChar),

          mySection(config.mySection),
          mySectionToParse(NULL),
          mySectionsNotToParse(config.mySectionsNotToParse),
          myHighestPriorityToParse(config.myHighestPriorityToParse),
          myLowestPriorityToParse(config.myLowestPriorityToParse),

          myRestartLevelNeeded(MvrConfigArg::NO_RESTART);
          myCheckingForRestartLevel(true);

          mySectionBroken(config.mySectionBroken);
          mySectionIgnored(config.mySectionIgnored);
          myUsingSections(config.myUsingSections);

          myParsingListNames(),
          myIsParsingListBroken(false),

          myFileName(),
          myBaseDirectory(),

          myParser(NULL),

          myIgnoreBounds(config.myIgnoreBounds);
          myFailOnBadSection(config.myFailOnBadSection),

          myDuplicateParams(config.myDuplicateParams),

          mySaveUnknown(config.mySaveUnknown),
          myIsQuiet(config.myIsQuiet),

          myPermissionAllowFactory(config.myPermissionAllowFactory),
          myPermissionSaveUnknown(config.myPermissionSaveUnknown),
          myProcessFileCallbacksLogLevel(config.myProcessFileCallbacksLogLevel),

          myCategoryToSectionsMap(config.myCategoryToSectionsMap),
          mySections(),

          myParserCB(this, &MvrConfig::parseArgument),
          myVersionCB(this, &MvrConfig::parseVersion),
          mySectionCB(this, &MvrConfig::parseSection),
          myListBeginCB(this, &MvrConfig::parseListBegin),
          myListEndCB(this, &MvrConfig::parseListEnd),
          myUnknownCB(this, &MvrConfig::parseUnknown)
{
  myArgumentParser = NULL;
  setBaseDirectory(config.getBaseDirectory());

  std::list<MvrConfigSection *>::const_iterator it;
  for (it=config.mySections.begin(); it != config.mySections.end(); it++)
  {
    mySections.push_back(new MvrConfigSection(*(*it)));
  }
  copySectionsToParse(config.mySectionsToParse);

  myParserCB.setName("MvrConfig::parseArgument");
  myVersionCB.setName("MvrConfig::parseVersion");
  mySectionCB.setName("MvrConfig::parseSection");
  myListBeginCB.setName("MvrConfig::parseListBegin");
  myListEndCB.setName("MvrConfig::parseListEnd");
  myUnknownCB.setName("MvrConfig::parseUnknown");

  myParser.setQuiet(myisQuiet);
  remParserHandlers();
  addParserHandlers();
}

MVREXPORT MvrConfig &MvrConfig::Operator=(const MvrConfig &config)
{
  if (this != &config){
    // Note that the name is not copied ...
    IFDEBUG(MvrLog::log(MvrLog::Verbose,
                        "%soperator=(from %s) begins",
                        myLogPrefix.c_str(),
                        config.myLogPrefix.c_str()));

  myArgumentParser = NULL;
  setBaseDirectory(config.getBaseDirectory());
  myNoBlanksBetweenParams = config.myNoBlanksBetweenParams;
  myConfigVersion = config.myConfigVersion;
  myIgnoreBounds = config.myIgnoreBounds;    
  myFailOnBadSection = config.myFailOnBadSection;
  mySection = config.mySection;
  mySectionBroken = config.mySectionBroken;
  mySectionIgnored = config.mySectionIgnored;
  myUsingSections = config.myUsingSections;
  myDuplicateParams = config.myDuplicateParams;

  myIsParsingListBroken = config.myIsParsingListBroken;

  myPermissionAllowFactory = config.myPermissionAllowFactory;
  myPermissionSaveUnknown = config.myPermissionSaveUnknown;

  myCategoryToSectionsMap.clear();
  myCategoryToSectionsMap = config.myCategoryToSectionsMap;

  clearSections();

  stds::list<MvrConfigSection *>::const_iterator it;
  for (it=config.mySections.begin(); it!=config.mySecitons.end(); it++)
  {
    mySections.push_back(new MvrConfigSection(*(*it)));
  }

  copySectionsToParse(config.mySectionToParse);

  mySectionsNotToParse = config.mySectionsNotToParse;

  myHighestPriorityToParse = config.myHighestPriorityToParse;
  myLowestPriorityToParse  = config.myLowestPriorityToParse;
  myRestartLevelNeeded     = config.myRestartLevelNeeded;
  myCheckingForRestartLevel= config.myCheckingForRestartLevel;

  remParserHandlers();
  addParserHandlers();

  IFDEBUG(MvrLog::log(MvrLog::Verbose,
                      "%soperator=(from %s) ends",
                      myLogPrefix.c_str(),
                      config.myLogPrefix.c_str()));
  }
  return *this;
}

MVREXPORT void MvrConfig::copyAndDetach(const MvrConfig &config)
{
  if (this != &config){
    // Note than the name is not copied ...
    IFDEBUG(MvrLog::log(MvrLog::Verbose,
                        "%soperator=(from %s) begins",
                        myLogPrefix.c_str(),
                        config.myLogPrefix.c_str()));

  myArgumentParser = NULL;
  setBaseDirectory(config.getBaseDirectory());
  myNoBlanksBetweenParams = config.myNoBlanksBetweenParams;
  myConfigVersion = config.myConfigVersion;
  myIgnoreBounds = config.myIgnoreBounds;    
  myFailOnBadSection = config.myFailOnBadSection;
  mySection = config.mySection;
  mySectionBroken = config.mySectionBroken;
  mySectionIgnored = config.mySectionIgnored;
  myUsingSections = config.myUsingSections;
  myDuplicateParams = config.myDuplicateParams;

  myIsParsingListBroken = config.myIsParsingListBroken;

  myPermissionAllowFactory = config.myPermissionAllowFactory;
  myPermissionSaveUnknown = config.myPermissionSaveUnknown;

  myCategoryToSectionsMap.clear();
  myCategoryToSectionsMap = config.myCategoryToSectionsMap;

  clearSections();

  stds::list<MvrConfigSection *>::const_iterator it;
  for (it=config.mySections.begin(); it!=config.mySecitons.end(); it++)
  {
    MvrConfigSection *sectionCopy = new MvrConfigSection();
    sectionCopy->setQuiet(myIsQuiet);
    sectionCopy->copyAndDetach(*(*it));
    // mySections.push_back(new MvrConfigSection(*(*it)));
  }

  copySectionsToParse(config.mySectionToParse);

  myHighestPriorityToParse = config.myHighestPriorityToParse;
  myLowestPriorityToParse  = config.myLowestPriorityToParse;
  myRestartLevelNeeded     = config.myRestartLevelNeeded;
  myCheckingForRestartLevel= config.myCheckingForRestartLevel;
  mySectionsNotToParse     = config.mySectionsNotToParse;

  remParserHandlers();
  addParserHandlers();

  IFDEBUG(MvrLog::log(MvrLog::Verbose,
                      "%soperator=(from %s) ends",
                      myLogPrefix.c_str(),
                      config.myLogPrefix.c_str()));
  }
  // return *this;
  }
}
/*
 * These names are used solely for log messages.
 * 
 * @param configName a char * descriptive name of the MvrConfig instance
 * @param robotName an optional char * identifier of the robot that has 
 * the config
 */
MVREXPORT void MvrConfig::setConfigName(const char *configName, const char *robotName)
{
  myConfigName = ((configName != NULL) ? configName : "");
  myRobotName  = ((robotName != NULL) ? robotName : "");
  myLogPrefix  = "";

  if (!myRobotName.empty()){
    myLogPrefix = myRobotName + ":";
  }
  myLogPrefix += "MvrConfig";
  
  if (!myConfigName.empyt()){
    myLogPrefix += "("+myConfigName + ")";
  }
  myLogPrefix += ": ";
}

MVREXPORT void MvrConfig::setQuiet(bool isQuiet)
{
  myIsQuiet = isQuiet;
  myParser.setQuiet(isQuiet);
}

MVREXPORT void MvrConfig::setTranslator(MvrConfig *xlatorConfig)
{
  myTranslator = xlatorConfig;

  if (myTranslator != NULL){
    if (!mySections.empty()){
      MvrLog::log(MvrLog::Normal,
                  "%sMvrConfig::setTranslator() sections already created",
                  myLogPrefix.c_str());
    for (std::list<MvrConfigSection *>::iterator iter = mySections.begin(); iter != mySections.end(); iter++){
      translateSection(*iter);
      }
    }
  }
}

MVREXPORT MvrConfig *MvrConfig::getTranslator() const
{
  return myTranslator;
}

/* 
 * @param isSummary a bool set to true if only the section names and parameter counts
 * should be logged; false to log detailed parameter information
 * @param sectionNameList a list of the string section names to be logged; if NULL,
 * then the data for all sections are logged
 */
MVREXPORT void MvrConfig::log(bool isSummary,
                              std::list<std::string> *sectionNameList,
                              const char *logPrefix)
{
  std::string origLogPrefix = myLogPrefix;

  if (!MvrUtil::isStrEmpty(logPrefix)){
    myLogPrefix = logPrefix;
  }

  std::list<MvrConfigArg> *params = NULL;

  MvrLog::log(MvrLog::Normal, "%slog", myLogPrefix.c_str());

  for (std::list<MvrConfigSection *>::const_iterator it = mySections.begin(); it != mySections.end(); it++)
  {
    params = (*it)->getParams();

    // If the section names were specified and the current section isn't in the
    // list, then skip the section...
    if (sectionNameList != NULL){
      if (!MvrUtil::isStrInList((*it)->getName(), *setctionNameList)){
        continue;
      }
    }
  
    if (params == NULL){
      MvrLog::log(mvrLog::Normal, "    Section %s has NULL params", (*it)->getName());
      continue;
    }

    if (isSummary){
      MvrLog::log(MvrLog::Normal,"    Section %s has %i params",
                  (*it)->getName(), params->size());
      continue;
    }

    MvrLog::log(MvrLog::Normal, "    Section %s:",
                (*it)->getName());
    for (std::list<MvrConfigArg>::iterator pit = params->begin(); pit != params->end(); pit++)
    {
      (*pit).log(false, 1, myLogPrefix.c_str());
    }
  } // end for loop

  MvrLog::log(MvrLog::Normal, "%send",myLogPrefix.c_str());
}                          

MVREXPORT void MvrConfig::clearSections(void)
{
  IFDEBUG(MvrLog::log(MvrLog::Verbose, "%saclearSections() begin", myLogPrefix.c_str()));

  while (mySections.begin() != mySections.end())
  {
    delete mySections.front();
    mySections.pop_front();
  }
  // Clear this just in case ...
  if(mySectionsToParse != NULL)
  {
    delete mySectionsToParse;
    mySectionsToParse = NULL;
  }

  IFDEBUG(MvrLog::log(MvrLog::Verbose, "%sclearSections() end", myLogPrefix.c_str()));
} 

MVREXPORT void MvrConfig::clearAll(void)
{
  clearSections();
  MvrUtil::deleteSetPairs(myProcessFileCBList.begin(), myProcessFileCBList.end());
  myProcessFileCBList.clear();
}   

void MvrConfig::addParseHandlers(void)
{
  std::list<MvrConfigSection *>::const_iterator it;
  std::list<MvrConfigArg> *param;
  std::list<MvrConfigArg>::iterator it;

  if (!myParser.addHandlerWithError(CONFIG_VERSION_TAG, &myVersionCB)){
    if (!myIsQuiet){
      MvrLog::log(MvrLog::Verbose, "%sCould not add ConfigVersion parser (probably unimportant)",
                  myLogPrefix.c_str());
    }
  }
  if (!myParser.addHandlerWithError("section", &mySectionCB)){
    if (!myIsQuiet){
      MvrLog::log(MvrLog::Verbose,
                  "%sCould not add section parser (probably unimportant)",
                  myLogPrefix.c_str());
    }
  }
  if (!myParser.addHandlerWithError(MvrConfigArg::LIST_END_TAG, &myListEndCB)){
    if (!myIsQuiet){
      MvrLog::log(MvrLog::Verbose,
                  "%sCould not add _listEnd parser",
                  myLogPrefix.c_str());
    }
  }

  for (it=mySections.begin(); it!=mySections.end(); it++)
  {
    params = (*it)->getParams();
    if (params == NULL)
      continue;
    for (pit=params->begin(); pit!=params->end(); pit++)
    {
      (*pit).addToFileParser(&myParser, &myParserCB, myLogPrefix.c_str(), myIsQuiet);
    }
  }

  if (!myParser.addHandlerWithError(NULL, &myUnknownCB)){
    if (!myIsQuiet){
      MvrLog::log(MvrLog::Verbose, "%sCould not add unknown param parser (probably unimportant)",
                  myLogPrefix.c_str());
    }
  }
}

void MvrConfig::remParserHandlers(void)
{
  myParser.remHander(&myParserCB);
  myParser.remHander(&myVersionCB);
  myParser.remHander(&mySectionCB);
  myParser.remHander(&myListBeginCB);
  myParser.remHander(&myListEndCB);
  myParser.remHander(&myUnknownCB);
}

MVREXPORT bool MvrConfig::addSection(const char *categoryName, const char *sectionName, const char *sectionDescription)
{
  if ((categoryName == NULL) || MvrUtil::isStrEmpty(sectionName)){
    MvrLog::log(MvrLog::Normal, 
                "%sFailed to add section '%s' to category '%s'",
                myLogPrefix.c_str(),
                ((sectionName != NULL) ? sectionName : "NULL"),
                ((categoryName != NULL) ? categoryName : "NULL"));
  return false;
  }
  MvrConfigSection *section = findSection(sectionName);
  std::string origCategoryName;

  if ((section != NULL) && (!MvrUtil::isStrEmpty(section->getCategoryName())))
  {
    translateSection(section);

    MvrLog::log(MvrLog::Normal,
                "%sFailed to add section '%s' to category '%s', already defined in category '%s'",
                myLogPrefix.c_str(),
                ((sectionName != NULL) ? sectionName : "NULL"),
                ((categoryName != NULL) ? categoryName : "NULL"),
                section->getCategoryName());
    return false;
  }

  if (section == NULL){
    MvrLog::log(MvrLog::Verbose, "%sMaking new section '%s' in category '%s'",
                myLogPrefix.c_str(), sectionName, categoryName);

    section = new MvrConfigSection(sectionName, sectionDescription, myIsQuiet, categoryName);
    mySections.push_back(section);
  }
  else{
    MvrLog::log(MvrLog::Verbose, "%sAssigning existing section '%s' to category '%s'",
                myLogPrefix.c_str(), sectionName, categoryName);
    origCategoryName = (section->getCategoryName() != NULL ? section->getCategoryName() : "");
    section->setCategoryName(categoryName);
    section->setComment(sectionDescription);     
  }
  translateSection(section);

  // If the section was in a different category, then remove it
  if (!origCategoryName.empty() && (MvrUtil::strcasecmp(origCategoryName, categoryName) != 0))
  {
    std::map<std::string, std::list<std::string> >::iterator origCatIter = myCategoryToSectionsMap.find(origCategoryName);
    if (origCatIter != myCategoryToSectionsMap.end()){
      std::list<std::string> *origSectionList = &(origCatIter->second);
      if (origSectionList != NULL){
        for (std::list<std::string>::iterator remIter=origSectionList->begin(); remIter!=origSectionList->end(); remIter++){
          if (MvrUtil::strcasecmp(sectionName, *remIter) == 0){
            origSectionList->erase(remIter);
            break;
          }
        }
      }
    }
  }

  // Make sure that section is not already stored in the category map
  std::list<std::string> *sectionList = NULL;

  std::map<std::string, std::list<std::string> >::iterator catIter=myCategoryToSectionsMap;
  if (catIter != myCategoryToSectionsMap.end()){
    sectionList = &(catIter->second);
  }

  bool isSectionFound = false;
  if (sectionList != NULL){
    isSectionFound = MvrUtil::isStrInList(sectionName, *sectionList);
  }
  // If the section is not already in the map, then add it
  if (!isSectionFound){
    myCategoryToSectionsMap[categoryName].push_back(sectionName);
  }
  return true;
}

/*
 * Set the comment string associated with a section. If the section doesn't 
 * exist then it is created.
 * @deprecated use addSection instead
 */
MVREXPORT void MvrConfig::setSectionComment(const char *sectionName, const char *comment)
{
  MvrConfigSection *section = findSection(sectionName);

  if (section == NULL)
  {
    MvrLog::log(MvrLog::Verbose, "%sMaking new section '%s' (for comment)",
                myLogPrefix.c_str(), sectionName);
    section = new MvrConfigSection(sectionName, comment, myIsQuiet);
  
    mySections.push_back(section);
  }
  else{
    section->setComment(comment);
  }
  translateSection(section);
}

/*
 * Add a flag to a section. If the section doesn't 
 * exist then it is created.
 * @warning The section name and flags must not contain any characters with
 *          special meaning when saved and loaded from a config file, such as '#', ';',
 *          tab, or newline.  
 */
MVREXPORT bool MvrConfig::addSectionFlags(const char *sectionName, const char *flags)
{
  MvrConfigSection *section = findSection(sectionName);

  if (section == NULL)
  {
    MvrLog::log(MvrLog::Verbose, "%sMaking new section '%s' (flags)", myLogPrefix.c_str(), sectionName);
    section = new MvrConfigSection(sectionName, NULL, myIsQuiet);
    section->addFlags(flags, myIsQuiet);

    translateSection(section);
    mySections.push_back(section);
  }
  else
    section->addFlags(flags, myIsQuiet);
  return true;
}

/*
 * Add a flag to a section. If the section doesn't 
 *  exist then it is created.
 */
MVREXPORT bool MvrConfig::remSectionFlag(const char *sectionName, const char *flag)
{
  MvrConfigSection *section = findSection(sectionName);

  if (section == NULL)
    return false;
  section->remFlag(flag);
  return true;
}

/// Add a parameter.  
/*   
 * @param arg Object containing key, description and value type of this parameter. This object will be copied....
 *            it must already have the priority, display hint, restart level, and other things like that already set.
 * @param sectionName Name of the section to put this parameter in.
 */
 MVREXPORT bool MvrConfig::addParamAsIs(const MvrConfigArg &arg, const char *sectionName)
 {
   return addParam(arg, sectionName, arg.getConfigPriority(), arg.getDisplayHint(), arg.getRestartLevel());
 }

/// Add a parameter
/*  
 * @param arg Object containing key, description and value type of this parameter. This object will be copied.
 * @param sectionName Name of the section to put this parameter in.
 * @param priority Priority or importance of this parameter to a user.
 * @param displayHint Suggest an appropriate UI control for editing this parameter's value. See MvrConfigArg::setDisplayHint() for description of display hint format.
 * @param restart restart level
 */
MVREXPORT bool MvrConfig::addParam(const MvrConfigArg &arg, 
                                   const char *sectionName,
                                   MvrPriority::Priority priority,
                                   const char *displayHint,
                                   MvrConfigArg::RestartLevel restart)
{
  MvrConfigSection *section = findSection(sectionName);

  MvrConfigSection *xltrSection = NULL;
  MvrConfigArg *xltrArg = NULL;
  if ((myTranslator != NULL) && (arg.getType() != MvrConfigArg::SEPARATOR))
  {
    xltrSection = myTranslator->findSection(sectionName);

    if (xltrSection != NULL){
      xltrArg = xltrSection->findParam(arg.getName(), true); // allow string holders
      
      if (xltrArg != NULL){
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfig::addParam() found translation arg for %s, %s",
                            sectionName, arg.getName()));
      }
      else{
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfig::addParam() cannot find translation arg for %s, %s",
                            sectionName, arg.getName()));
      }
    }
    else{
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfig::addParam() cannot find translation section for %s",
                            sectionName));
    }
  }
  //printf("SECTION '%s' name '%s' desc '%s'\n", sectionName, arg.getName(), arg.getDescription());
  if (section == NULL)
  {
    MvrLog::log(MvrLog::Verbose, "MvrConfigArg %s: Making new section '%s' (for param)",
                myLogPrefix.c_str(), sectionName);
    section = new MvrConfigSection(sectionName, NULL, myIsQuiet);

    translateSection(section);

    mySections.push_back(section);
  }

  std::list<MvrConfigArg> *params = section->getParams();

  if (params == NULL)
  {
    MvrLog::log(MvrLog::Terse, "%sSomething has gone hideously wrong in MvrConfig::addParam()", myLogPrefix.c_str());
    return false;
  }

  // Dont add consecutive separators
  if (arg.getType() == MvrConfigArg::SEPARATOR && !params->empty() && params->back().getType() == MvrConfigArg::SEPARATOR)
  {
    // MvrLog::log(MvrLog::Verbose, "Last parameter a sep, so is this one, ignoring it");
    return true;
  }

  std::list<MvrConfigSection *>::iterator sectionIt;

  for (sectionIt = mySections.begin(); sectionIt!=mySections.end(); sectionIt++)
  {
    MvrConfigSection *curSection = *sectionIt;

    MvrConfig *existingParam = NULL;
    if (!MvrUtil::isStrEmpty(arg.getName())){
      existingParam = curSection->findParam(arg.getName());
    }

    // If we have an argument of this name but we don't have see if
    // this section is our own, if its not then note we have
    // duplicates
    if (existingParam != NULL)
    {
       if (strcasecmp(curSection->getName(), section->getName) != 0){
         MvrLog::log(MvrLog::Verbose,
                     "%sParameter %s (type %s) name duplicated in section %s and %s",
                     myLogPrefix.c_str(),
                     arg.getName(),
                     MvrConfigArg::toString(arg.getType()),
                     curSection->getName(),
                     section->getName());
       myDuplicateParams = true;
      }
      else{
        MvrLog::log(((!existingParam->isPlaceholder()) ? MvrLog::Normal : MvrLog::Verbose),
                    "%sParameter %s (type %s) already exists in section %s (placeholder = %i)",
                    myLogPrefix.c_str(),
                    arg.getName(),
                    MvrConfigArg::toString(arg.getType()),
                    section->getName(),
                    existingParam->isPlaceholder());  
      }
    }
  }

  // now make sure we can add it to the file parser (with the section stuff its okay if we can't)
  if (!myParser.addHandlerWithError(arg.getName(), &myParserCB))
  {
    if (!myIsQuiet){
      MvrLog::log(MvrLog::Verbose, "%sCould not add parameter '%s' to file parser, probably already there.",
                  myLogPrefix.c_str(), arg.getName());
    }
  }

  // now we add all the list names to the parser too (the function makes sure it's a list)
  addListNamesToParser(arg);

  // remove any string and list holders for this param
  section->remStringHolder(arg->getName());

  params->push_back(arg);

  if (xltrArg != NULL){
    params->back().copyTranslation(*xltrArg);
  }

  params->back().setConfigPriority(priority);
  params->back().setDisplayHint(displayHint);

  params->back().setIgnoreBounds(myIgnoreBounds);
  params->back().replaceSpacesInName();

  IFDEBUG(MvrLog::log(MvrLog::Verbose, "%sAdded parameter '%s' to section '%s'",
                      myLogPrefix.c_str(), arg.getName(), section->getName()));
  return true;
}   

/// Add a comment to a section
/*
 * @param comment Text of the comment.
 * @param sectionName Name of the section to add the comment to. If the section does not exist, it will be created.
 * @param priority Priority or importance.
 */
MVREXPORT bool MvrConfig::addComment(const char *comment, const char *sectionName, MvrPriority::Priority priority)
{
  return addParam(MvrConfigArg(comment), sectionName, priority);
}

MVREXPORT bool MvrConfig::parseVersion(MvrArgumentBuilder *arg, char *errorBuffer, size_t errorBufferLen)
{
  if ((errorBuffer != NULL) && (errorBufferLen > 0)){
    errorBuffer[0] = '\0';
  }
  if ((arg->getArgc() < 0) || (arg->getArg(0) == NULL)){
    if ((errorBuffer != NULL) && (errorBufferLen > 0)){
      snprintf(errorBuffer, errorBufferLen, "Configuration version error (blank");
      errorBuffer[errorBufferLen - 1] = '\0';
    }
    return false;
  }

  myConfigVersion = arg->getArg(0);
  MvrLog::log(MvrLog::Normal, "%sConfig version: %s", myLogPrefix.c_str(), myConfigVersion.c_str());

  return true;
} 

/*
 * The extra string of the parser should be set to the 'Section'
 * command while the rest of the arg should be the arguments to the
 * section command. Its case insensitive.
 * @param arg Should contain the 'Section' keyword as its "extra" string, and section name as argument(s).
 * @param errorBuffer if this is NULL it is ignored, otherwise the
 * string for the error is put into the buffer, the first word should
 * be the parameter that has trouble
 * @param errorBufferLen the length of the error buffer
 */
MVREXPORT bool MvrConfig::parseSection(MvrArgumentBuilder *arg, char *errorBuffer, size_t errorBufferLen)
{
  if (myFailOnBadSection && errorBuffer != NULL)
    errorBuffer[0] = '\0';
  std::list<MvrConfigSection *>::iterator sectionIt;
  MvrConfigSection *section = NULL;

  if (myFailOnBadSection && errorBuffer != NULL)
    errorBuffer[0] = '\0';
  for (sectionIt=mySections.begin(); sectionIt!=mySections.end(); sectionIt++){
    section = (*sectionIt);
    if (MvrUtil::strcasecmp(section->getName(), arg->getFullString()) == 0)
    {
      bool isParseSection = true;
      if (mySectionToParse != NULL){
        isParseSection = false;
        for (std::list<std::string>::iterator sIter = mySectionsToParse->begin(); sIter!=mySectionsToParse->end(); sIter++)
        {
          std::string sp = *sIter;
          if (MvrUtil::strcasecmp(section->getName(), sp.c_str()) == 0){
            isParseSection = true;
            break;
          }
        }
      }
      if (mySectionsNotToParse.find(section->getName()) != mySectionsNotToParse.end())
      {
        isParseSection = false;
      }

      if (isParseSection){
        MvrLog::log(MvrLog::Verbose, "%sConfig switching to section '%s'", myLogPrefix.c_str(), arg->getFullString());

        mySection = arg->getFullString();
        mySectionBroken = false;
        mySectionIgnored = false;
        myUsingSections = true;
        return true;
      }
      else{
        MvrLog::log(MvrLog::Verbose, "%signoring section '%s'",
                    myLogPrefix.c_str(), arg->getFullString());

        mySection = arg->getFullString();
        mySectionBroken = false;
        mySectionIgnored = true;
        myUsingSections = true;
        return true;                    
      }
    }
  }
  if (myFailOnBadSection)
  {
    mySection = "";
    mySectionBroken = true;
    mySectionIgnored = false;
    snprintf(errorBuffer, errorBufferLeem, "MvrConfig: Could not find section '%s'",
             arg->getFullString());
    MvrLog::log(MvrLog::Terse, "%sCould not find section '%s', failing", myLogPrefix.c_str(), arg->getFullString());
    return false;
  }
  else{
    if (mySaveUnknown && mySectionsToParse == NULL && myPermissionSaveUnknown){
      MvrLog::log(MvrLog::Verbose, "%smaking new section '%s' to save unknown", 
                  myLogPrefix.c_str(), arg->getFullString());
      mySection = arg->getFullString();
      mySectionBroken = false;
      mySectionIgnored = false;
      section = new MvrConfigSection(arg->getFullString(), NULL, myIsQuiet);

      translateSection(section);

      mySections.push_back(section);
    }
    else{
      mySection = "";
      mySectionBroken = false;
      mySectionIgnored = true;
      MvrLog::log(MvrLog::Normal, "%sIgnoring unknown section '%s'", myLogPrefix.c_str(), arg->getFullString());
    }
    return true;
  }
}

MVREXPORT bool MvrConfig::parseListBegin(MvrArgumentBuilder *arg, char *errorBuffer, size_t errorBufferLen)
{
  if ((arg == NULL) || (arg->getArgc() < 1)){
    MvrLog::log(MvrLog::Normal, "MvrConfig::parseListBegin() invalid arg input");
    return false;
  }
  if (mySectionIgnored){
    return false;
  }
  if (mySectionBroken){
    if (myFailOnBadSection){
      return false;
    }
    else {
      return true;
    }
  }

  if (myIsParsingListBroken){
    return true;
  }

  if ((errorBuffer != NULL) && (errorBufferLen > 0)){
    errorBuffer[0] = '\0';
  }

  MvrConfigArg *param = NULL;

  MvrConfigSection *section = findSection(mySection.c_str());

  if (section == NULL){
    MvrLog::log(MvrLog::Normal,
                "MvrConfig::parseListBegin() cannot find section %s for list %s",
                mySection.c_str(),
                arg->getFullString());
    return true;
  }

  MvrConfigArg *parentParam = section->findParam(myParsingListNames, true);

  myParsingListNames.push_back(arg->getFullString());
  param = section->findParam(myParsingListNames, true);

  if (param == NULL){
    MvrLog::log(MvrLog::Normal,
                "MvrConfig::parseListBegin() cannot find param for list %s, creating LIST_HOLDER",
                arg->getFullString());
    param = new MvrConfigArg(MvrConfigArg::LIST_HOLDER,
                             arg->getFullString(),
                             "");
    if (parentParam == NULL){
      addParam(*param, mySection.c_str());
    }
    else{
      parentParam->addArg(*param);
    }

    delete param;
    param = NULL;

    return true;
  }
  return true;
}

MVREXPORT bool MvrConfig::parseListEnd(MvrArgumentBuilder *arg, char *errorBuffer, size_t errorBufferLen)
{
  if ((arg == NULL) || (arg->getArgc() < 1)){
    MvrLog::log(MvrLog::Normal, "MvrConfig::parseListEnd() invalid arg input");
    return false;
  }
  if (mySectionIgnored) {
    return true;
  }
  if (mySectionBroken) {
    return false;
  }

  if (myIsParsingListBroken){
    return true;
  }

  if ((myParsingListNames.empty()) || (MvrUtil::strcasequotecmp(myParsingListNames.back(), arg->getFullString()) != 0)) 
  {
    // If previous list was broken, this really isn't an error
    MvrLog::log(MvrLog::Normal, "MvrConfig::parseListEnd() found end of different list %s",
                arg->getFullString());
    if (myIsParsingListBroken) {
      return true;
    }
    else {
      return false;
    }
  }
  // Found the end of the current list
  if (!myIsParsingListBroken) {
  
    MvrConfigArg *param = NULL;
    MvrConfigSection *section = findSection(mySection.c_str());

    if (section != NULL) {
      param = section->findParam(myParsingListNames, true);   
    }
    if (param != NULL) {
      param->setValueSet();
    }
  } // end if list is fine

  myIsParsingListBroken = false;
  // KMC 11/7/12 Note test for empty() above.
  myParsingListNames.pop_back();

  return true;
}


/// The extra string of the parser should be set to the command wanted,
/// while the rest of the arg should be the arguments to the command.
/// Its case insensitive.

/*
 * @param arg Obtain parameter name from this object's "extra string"
 *         and value(s) from its argument(s).
 * @param errorBuffer If this is NULL it is ignored, otherwise the
 *        string for the error is put into the buffer, the first word should
 *        be the parameter that has trouble
 * @param errorBufferLen the length of @a errorBuffer
 */
MVREXPORT bool MvrConfig::parseArgument(MvrArgumentBuilder *arg, char *errorBuffer, size_t errorBufferLen)
{
  bool ret = true;

  if (mySectionBroken)
  {
    MvrLog::log(MvrLog::Verbose,"%sSkipping parameter %s because section broken",
                myLogPrefix.c_str(),
                arg->getExtraString());
    if (myFailOnBadSection)
    {
      snprintf(errorBuffer, errorBufferLen,
               "Failed because broken config section");
      return false;
    }
    else{
      return true;
    }
  }
  else if (mySectionIgnored){
    return true;
  }

  if (myIsParsingListBroken){
    MvrLog::log(MvrLog::Normal,"%sSkipping parameter %s because list broken",
                myLogPrefix.c_str(),
                arg->getExtraString());
    return true;
  }
  // if we have duplicate params and don't have sections don't trash anything
  if (myDuplicateParams && myUsingSections && mySection.size() <= 0)
  {
    snprintf(errorBuffer, errorBufferLen,
             "%s not in section, client needs an upgrade",
             arg->getExtraString());
    MvrLog::log(MvrLog::Normal,
                "%s%s not in a section, client needs an upgrade",
                myLogPrefix.c_str(),
                arg->getExtraString());
    return false;
  }

  // see if we found this parameter
  bool found = false;

  if (errorBuffer != NULL)
    errorBuffer[0] = '\0';
  
  MvrConfigSection *section = findSection(mySection.c_str());

  if (section != NULL){
    std::list<MvrConfigArg *> parseParamList;

    std::list<MvrConfigArg> *paramList = section->getParams();

    if (paramList != NULL){
      for (std::list<MvrConfigArg>::iterator pIter = paramList->begin();
           pIter != paramList->end();
           pIter++){
        MvrConfigArg *param = &(*pIter);
        MvrConfigArg *parseParam = NULL;

        if(myParsingListNames.empty()){
          if (MvrUtil::strcasecmp(param->getName(), arg->getExtraString()) != 0){
            continue;
          }
          parseParamList.push_back(param);
        }
        else{
          if (MvrUtil::strcasecmp(param->getName(), myParsingListNames.front()) != 0){
            continue;
          }
          std::list<std::string>::iterator listIter = myParseListNames.begin();
          listIter++;

          std::list<MvrConfigArg*> matchParamList;
          matchParamList.push_back(param);

          std::list<MvrConfigArg*> tempParamList;

          while (listIter != myParsingListNames.end()) {

            for (std::list<MvrConfigArg*>::iterator mIter = matchParamList.begin();
                 mIter != matchParamList.end();
                 mIter++) {
              MvrConfigArg *matchParam = *mIter;
              if (matchParam == NULL) {
                continue;
              }
              for (int i = 0; i < matchParam->getArgCount(); i++) {
            
                MvrConfigArg *childArg = matchParam->getArg(i);
                if (childArg == NULL) {
                  continue;
                }
                if (MvrUtil::strcasecmp(childArg->getName(), *listIter) != 0) {
                  continue;
                }
                tempParamList.push_back(childArg);
              }
            } 

            matchParamList.clear();
            matchParamList = tempParamList;
            tempParamList.clear();

            listIter++;

          } // end for each list level
          
          for (std::list<MvrConfigArg*>::iterator matchIter = matchParamList.begin();
               matchIter != matchParamList.end();
               matchIter++) {

             MvrConfigArg *matchParam = *matchIter;
             if (matchParam == NULL) {
               continue;
             }

             for (int i = 0; i < matchParam->getArgCount(); i++) {
            
                MvrConfigArg *childArg = matchParam->getArg(i);
                if (childArg == NULL) {
                  continue;
                }
                if (MvrUtil::strcasecmp(childArg->getName(), arg->getExtraString()) != 0) {
                  continue;
                }
                parseParamList.push_back(childArg);
              }
          }
        }


        for (std::list<MvrConfigArg*>::iterator parseIter = parseParamList.begin();
             parseIter != parseParamList.end();
             parseIter++) {

          MrConfigArg *parseParam = *parseIter;
          if (parseParam == NULL) {
            continue;
          }

          found = true;

          if ((parseParam->getConfigPriority() < myHighestPriorityToParse) ||
              (parseParam->getConfigPriority() > myLowestPriorityToParse)) {
            return true;
          }

          if ((myPermissionAllowFactory) || (parseParam->getConfigPriority() < MvrPriority::FACTORY))
           {
      
	          bool changed = false;
            if (!parseParam->parseArgument(arg, errorBuffer, errorBufferLen, myLogPrefix.c_str(), myIsQuiet, &changed)) {

              MvrLog::log(MvrLog::Normal,
                         "MvrConfig::parseArgument() error parsing %s",
                         arg->getFullString());
              ret = false;
            }
            if (changed)
            {
              if (parseParam->getRestartLevel() > myRestartLevelNeeded)
              {
                myRestartLevelNeeded = parseParam->getRestartLevel();
                // don't print out the warning if nothings checking for it
                if (myCheckingForRestartLevel)
                  MvrLog::log(MvrLog::Normal, 
                "%sParameter '%s' in section '%s' changed, bumping restart level needed to %s (%d)", 
                myLogPrefix.c_str(), parseParam->getName(),
                section->getName(),
                MvrConfigArg::toString(myRestartLevelNeeded),
                myRestartLevelNeeded);
              }
            }
          }
          else {}
        } 
      } 
    } 
  } 
  // if we didn't find this param its because its a parameter in another section, 
  // so pass this off to the parser for unknown things
  if (!found)
  {
    MvrArgumentBuilder unknown;
    unknown.setQuiet(myIsQuiet);
    unknown.addPlain(arg->getExtraString());
    unknown.addPlain(arg->getFullString());
    return parseUnknown(&unknown, errorBuffer, errorBufferLen);
  }
  return ret;
}


/// The extra string of the parser should be set to the command wanted,
/// while the rest of the arg should be the arguments to the command.
/// Its case insensitive.

/*
 * @param arg Obtain parameter name from this argument builder's "exra" string and value(s) from its argument(s). 
 * @param errorBuffer if this is NULL it is ignored, otherwise the
 * string for the error is put into the buffer, the first word should
 * be the parameter that has trouble
   
 * @param errorBufferLen the length of the error buffer
 **/
MVREXPORT bool MvrConfig::parseUnknown(MvrArgumentBuilder *arg, char *errorBuffer, size_t errorBufferLen)
{
  if (arg->getArgc() < 1)
  {
    if (!myIsQuiet) {
      MvrLog::log(MvrLog::Verbose, "%sEmpty arg in section '%s', ignoring it", 
                 myLogPrefix.c_str(), mySection.c_str());
    }
    return true;
  }

  if (mySaveUnknown && mySectionsToParse == NULL && myPermissionSaveUnknown)
  {
    if (arg->getArgc() < 2)
    {
      if (!myIsQuiet) {
        MvrLog::log(MvrLog::Verbose, "%sNo arg for param '%s' in section '%s', saving it anyways", 
                  myLogPrefix.c_str(), arg->getArg(0), mySection.c_str());
      }
      
      if (myParsingListNames.empty() || myIsParsingListBroken) 
      {
        addParam(MvrConfigArg(arg->getArg(0), ""), mySection.c_str());
      }
      else 
      { 
        MvrConfigSection *section = findSection(mySection.c_str());
        if (section == NULL) {
          MvrLog::log(MvrLog::Normal,
                      "%sMvrConfigArg::parseUnknown() cannot find section %s",
                      myLogPrefix.c_str(),
                      mySection.c_str());
          return false;
        }  
        MvrConfigArg *parentParam = section->findParam(myParsingListNames, true);
        if (parentParam == NULL) {
          MvrLog::log(MvrLog::Normal,
                     "%sMvrConfigArg::parseUnknown() cannot find parent param %s for %s",
                     myLogPrefix.c_str(),
                     myParsingListNames.front().c_str(),
                     arg->getArg(0));
          return false;
        }
        parentParam->addArg(MvrConfigArg(arg->getArg(0), ""));
      }
    }
    else
    {
      std::string str;
      // int i;
      for (unsigned int i = 1; i < arg->getArgc(); i++)
      {
        if (i != 1)
	        str += ' ';
	      str += arg->getArg(i);
      }

      if (!myIsQuiet) {
        MvrLog::log(MvrLog::Verbose, "%sUnknown '%s %s' in section '%s', saving it", 
                    myLogPrefix.c_str(), arg->getArg(0), str.c_str(), 
                    mySection.c_str());
      }
      if (myParsingListNames.empty() || myIsParsingListBroken) {
  
        addParam(MvrConfigArg(arg->getArg(0), str.c_str()),mySection.c_str());
      }
      else 
      {
        MvrConfigSection *section = findSection(mySection.c_str());
        if (section == NULL)
        {
          MvrLog::log(MvrLog::Normal,
                      "%sMvrConfigArg::parseUnknown() cannot find section %s",
                      myLogPrefix.c_str(),
                      mySection.c_str());
          return false;
        }  
        MvrConfigArg *parentParam = section->findParam(myParsingListNames, true);
        if (parentParam == NULL) 
        {
          MvrLog::log(MvrLog::Normal,
                      "%sMvrConfigArg::parseUnknown() cannot find parent param '%s' for %s",
                      myLogPrefix.c_str(),
                      myParsingListNames.front().c_str(),
                      arg->getArg(0));
          return false;
        }
       
        parentParam->addArg(MvrConfigArg(arg->getArg(0), str.c_str()));

      } // end else list member
    }
  }
  else
  {
    IFDEBUG(MvrLog::log(MvrLog::Normal,
                        "MvrConfig::parseUnknown() mySaveUnknown = %i, myPermissionSaveUnknown = %i mySectionsToParse %s NULL",
                        mySaveUnknown,
                        myPermissionSaveUnknown,
                        ((mySectionsToParse == NULL) ? "==" : "!=")));

    MvrLog::log(MvrLog::Verbose, "%sUnknown '%s' in section '%s', ignoring it", 
	               myLogPrefix.c_str(), arg->getFullString(), mySection.c_str());
  }
  return true;
} 

/**
   @param fileName the file to load
   @param continueOnErrors whether to continue parsing if we get
          errors (or just bail)
   @param noFileNotFoundMessage if the file isn't found and this param
          is true it won't complain, otherwise it will
   @param errorBuffer If an error occurs and this is not NULL, copy a description of the error into this buffer
   @param errorBufferLen the length of @a errorBuffer
   @param sectionsToParse if NULL, then parse all sections; otherwise,
          a list of the section names that should be parsed (sections not in 
          the list are ignored)
   @param highestPriority Any configuration settings with a priority before 
          this are ignored
   @param lowestPriority Any configuration settings with a priority after
          this are ignored.
   @param restartLevelNeeded for internal use, leave as NULL
 **/
MVREXPORT bool MvrConfig::parseFile(const char *fileName, 
                                    bool continueOnErrors,
                                    bool noFileNotFoundMessage, 
                                    char *errorBuffer,
                                    size_t errorBufferLen,
                                    std::list<std::string> *sectionsToParse,
                                    MvrPriority::Priority highestPriority,
                                    MvrPriority::Priority lowestPriority,
                                    MvrConfigArg::RestartLevel *restartLevelNeeded)
{
  bool ret = true;

  if(fileName)
    myFileName = fileName;
  else
    myFileName = "";

  MvrLog::log(myProcessFileCallbacksLogLevel,
	     "Starting parsing file %s (%s)", myFileName.c_str(), fileName);


  if (errorBuffer != NULL)
    errorBuffer[0] = '\0';

  // Should be null, but just in case...
  if (mySectionsToParse != NULL)
  {
    delete mySectionsToParse; 
    mySectionsToParse = NULL;
  }

  copySectionsToParse(sectionsToParse);
  myHighestPriorityToParse = highestPriority;
  myLowestPriorityToParse  = lowestPriority;
  myRestartLevelNeeded = MvrConfigArg::NO_RESTART;
  if (restartLevelNeeded != NULL)
    myCheckingForRestartLevel = true;
  else
    myCheckingForRestartLevel = false;
  

  // parse the file (errors will go into myErrorBuffer from the functors)
  ret = myParser.parseFile(fileName, continueOnErrors, noFileNotFoundMessage);

  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }

  // if we have a parser and haven't failed (or we continue on errors)
  // then parse the arguments from the parser
  if (myArgumentParser != NULL && (ret || continueOnErrors))
    ret = parseArgumentParser(myArgumentParser, continueOnErrors, errorBuffer, errorBufferLen) && ret;

  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }
  //printf("parser %s\n", MvrUtil::convertBool(ret));
  
  // if we haven't failed (or we continue on errors) then call the
  // process file callbacks
  if (ret || continueOnErrors)
    ret = callProcessFileCallBacks(continueOnErrors, errorBuffer, errorBufferLen) && ret;
  
  // copy our error if we have one and haven't copied in yet
  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }

  if (restartLevelNeeded != NULL)
    *restartLevelNeeded = myRestartLevelNeeded;

  // Done with the temp parsing info, so delete it...
  delete mySectionsToParse; 
  mySectionsToParse = NULL;

  myHighestPriorityToParse = MvrPriority::FIRST_PRIORITY;
  myLowestPriorityToParse  = MvrPriority::LAST_PRIORITY;
  myRestartLevelNeeded = MvrConfigArg::NO_RESTART;
  myCheckingForRestartLevel = true;

  MvrLog::log(myProcessFileCallbacksLogLevel,
              "Done parsing file %s (ret %s)", fileName,
              MvrUtil::convertBool(ret));
  return ret;
}

/**
   @param fileName the name of the file to write out
   @param append if true then text will be appended to the file if it exists, otherwise any existing file will be overwritten.
   @param alreadyWritten if non-NULL, a list of strings that have already been
          written out, don't write it again if it's in this list; when 
          something is written by this function, then it is put it into this list 
   @param writeExtras if this is true then the priority, display hint, restart level, and other later extras of each parameter will 
          also be written to the file, if it is false they will not be
   @param sectionsToWrite if NULL, then write all sections; otherwise,
          a list of the section names that should be written 
   @param highestPriority the MvrPriority::Priority that specifies the minimum (numerical)
          priority that a parameter must have in order to be written; not related to the 
          writeExtras flag
   @param lowestPriority the MvrPriority::Priority that specifies the maximum (numerical)
          priority that a parameter must have in order to be written; not related to the 
          writeExtras flag
 **/
MVREXPORT bool MvrConfig::writeFile(const char *fileName, 
                                    bool append, 
                                    std::set<std::string> *alreadyWritten,
                                    bool writeExtras,
                                    std::list<std::string> *sectionsToWrite,
                                    MvrPriority::Priority highestPriority,
                                    MvrPriority::Priority lowestPriority)
{
  FILE *file;

  std::set<std::string> writtenSet;
  std::set<std::string> *written;
  if (alreadyWritten != NULL)
    written = alreadyWritten;
  else
    written = &writtenSet;
  //std::list<MvrConfigArg>::iterator it;

  // later this'll have a prefix
  std::string realFileName;
  if (fileName[0] == '/' || fileName[0] == '\\')
  {
    realFileName = fileName;
  }
  else
  {
    realFileName = myBaseDirectory;
    realFileName += fileName;
  }

  std::string mode;

  if (append)
    mode = "a";
  else
    mode = "w";

  if ((file = MvrUtil::fopen(realFileName.c_str(), mode.c_str())) == NULL)
  {
    MvrLog::log(MvrLog::Terse, "%sCannot open file '%s' for writing",
	              myLogPrefix.c_str(), realFileName.c_str());
    return false;
  }

  // Start the config file with version information
  fprintf(file, "%s %s\n", CONFIG_VERSION_TAG, CURRENT_CONFIG_VERSION);

  bool firstSection = true;
  std::list<MvrConfigSection *>::iterator sectionIt;
  MvrConfigSection *section = NULL;
  
  // first write out the generic section (ie sectionless stuff, mostly
  // for backwards compatibility)
  if ( ((section = findSection("")) != NULL) && (sectionsToWrite == NULL) )
  {
    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;
    writeSection(section, file, written, writeExtras, highestPriority, lowestPriority);
  }

  // then write out the rest (skip the generic section if we have one)
  for (sectionIt = mySections.begin(); sectionIt != mySections.end(); sectionIt++)
  {
    section = (*sectionIt);
    if (strcmp(section->getName(), "") == 0)
      continue;

    if (sectionsToWrite != NULL) 
    {
      bool isSectionFound = false;
      for (std::list<std::string>::iterator swIter = sectionsToWrite->begin(); swIter != sectionsToWrite->end(); swIter++) 
      {
        std::string sp = *swIter;
        if (MvrUtil::strcasecmp(section->getName(), sp.c_str()) == 0) 
        {
          isSectionFound = true;
          break;
        }
      } 

      if (!isSectionFound) {
        continue;
      }
    
    } // end if sections specified

    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;

    writeSection(section, file, written, writeExtras, highestPriority, lowestPriority);
  }
  fclose(file);
  return true;
}
  

MVREXPORT bool MvrConfig::parseText(const std::list<std::string> &configLines,
                                    bool continueOnErrors,
                                    bool *parseOk,
                                    bool *processOk,
                                    char *errorBuffer,
                                    size_t errorBufferLen,
                                    std::list<std::string> *sectionsToParse,
                                    MvrPriority::Priority highestPriority,
                                    MvrPriority::Priority lowestPriority,
                                    MvrConfigArg::RestartLevel *restartLevelNeeded)
{
  if ((errorBuffer != NULL) && (errorBufferLen > 0)) {
    errorBuffer[0] = '\0';
  }
 
  if (parseOk != NULL) {
    *parseOk = true;
  }
  if (processOk != NULL) {
    *processOk = true;
  }


  bool ret = true;
  
  if (mySectionsToParse != NULL)
  {
    delete mySectionsToParse; 
    mySectionsToParse = NULL;
  }
  

  copySectionsToParse(sectionsToParse);
  myHighestPriorityToParse = highestPriority;
  myLowestPriorityToParse  = lowestPriority;
  myRestartLevelNeeded = MvrConfigArg::NO_RESTART;
  

  if (restartLevelNeeded != NULL)
    myCheckingForRestartLevel = true;
  else
    myCheckingForRestartLevel = false;

  char lineBuf[10000];
  size_t lineBufLen = sizeof(lineBuf);
  
  for (std::list<std::string>::const_iterator iter = configLines.begin(); iter != configLines.end(); iter++) 
  {
    const std::string &curLine = *iter;

    if (curLine.empty()) {
      continue;
    }
    snprintf(lineBuf, lineBufLen, curLine.c_str());

    if (!myParser.parseLine(lineBuf, errorBuffer, errorBufferLen))
    {
      ret = false;
      if (parseOk != NULL) {
        *parseOk = false;
      }
      if (!continueOnErrors)
        break;
    }
  } // end for each text line
  
  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }
  
  // if we haven't failed (or we continue on errors) then call the
  // process file callbacks
  if (ret || continueOnErrors) 
  {
    if (!callProcessFileCallBacks(continueOnErrors, errorBuffer, errorBufferLen)) 
    {
      ret = false;
      if (processOk != NULL) 
      {
        *processOk = false;
      }
    } 
  }
 
  // copy our error if we have one and haven't copied in yet
  // set our pointers so we don't copy anymore into/over it
  if (errorBuffer != NULL && errorBuffer[0] != '\0')
  {
    errorBuffer = NULL;
    errorBufferLen = 0;
  }

  if (restartLevelNeeded != NULL)
    *restartLevelNeeded = myRestartLevelNeeded;

  // Done with the temp parsing info, so delete it...
  delete mySectionsToParse; 
  mySectionsToParse = NULL;

  myHighestPriorityToParse = MvrPriority::FIRST_PRIORITY;
  myLowestPriorityToParse  = MvrPriority::LAST_PRIORITY;
  myRestartLevelNeeded = MvrConfigArg::NO_RESTART;
  myCheckingForRestartLevel = true;

  MvrLog::log(myProcessFileCallbacksLogLevel,
              "Done parsing text list (ret %s)", 
              MvrUtil::convertBool(ret));

  return ret;

} // end method parseText 

/// Parse a config resource file, for translation.
MVREXPORT bool MvrConfig::parseResourceFile(const char *fileName, 
                                            bool continueOnError,
                                            char *errorBuffer,
                                            size_t errorBufferLen,
                                            std::list<std::string> *sectionsToParse)
{
  bool ret = true;

  if(fileName)
    myFileName = fileName;
  else
    myFileName = "";

  if (errorBuffer != NULL)
    errorBuffer[0] = '\0';
 
  // stat(fileName, &myReadFileStat);
  FILE *file = NULL;
  char line[10000];

  MvrLog::log(MvrLog::Normal, "%sMvrConfig::parseResourceFile() Opening file %s", myLogPrefix.c_str(), fileName);

  // Open file in binary mode to avoid conversion of CRLF in windows. 
  // This is necessary so that a consistent checksum value is obtained.
  if ((file = MvrUtil::fopen(fileName, "rb")) == NULL)
  {
    MvrLog::log(MvrLog::Terse, "Cannot open file '%s'", fileName);

    if (errorBuffer != NULL) {
      snprintf(errorBuffer, errorBufferLen, "Resource file invalid: cannot open file '%s'", fileName);
    }
    return false;
  }

  bool isSuccess = true;

  char *localErrorBuffer = NULL;
  size_t localErrorBufferLen = 0;

  if ((errorBuffer != NULL) && (errorBufferLen > 0)) 
  {
    localErrorBufferLen = errorBufferLen;
    localErrorBuffer = new char[localErrorBufferLen];
    localErrorBuffer[0] = '\0';
  }

  fgets(line, sizeof(line), file);

  for (int h = 0; h < 2; h++) {
    if (fgets(line, sizeof(line), file) == NULL) {
      MvrLog::log(MvrLog::Terse, "%sMvrConfig::parseResourceFile() Header missing in '%s'", 
                  myLogPrefix.c_str(),
                  fileName);
      return false;
    }
  }

  bool ignoreNormalSpaces = true;
   
  while ((fgets(line, sizeof(line), file) != NULL)) 
  {
    if (MvrUtil::isStrEmpty(line)) 
    {
      continue;
    }
     
    MvrArgumentBuilder builder(512, myCsvSeparatorChar, ignoreNormalSpaces); 
 
    builder.addPlain(line);

    if ((builder.getArgc() == 0) || (MvrUtil::isStrEmpty(builder.getArg(0)))) 
    {
      continue;
    }
    if (builder.getArgc() < MvrConfigArg::RESOURCE_INDEX_OF_DESCRIPTION) 
    {
      IFDEBUG(MvrLog::log(MvrLog::Normal,
                          "MvrConfig::parseResourceFile() no section for %s",
                          builder.getFullString()));
      continue;
    }
    std::string sectionName = MvrConfigArg::parseResourceSectionName(&builder, myLogPrefix.c_str());  
    MvrConfigSection *section = findSection(sectionName.c_str());
    if (section == NULL) {
      IFDEBUG(MvrLog::log(MvrLog::Normal,
                          "MvrConfig::parseResourceFile() adding section %s",
                          sectionName.c_str()));        
      if (!addSection(CATEGORY_DEBUG, sectionName.c_str(), "")) 
      {
        MvrLog::log(MvrLog::Normal,
                    "MvrConfig::parseResourceFile() error adding section %s",
                    sectionName.c_str());    
        continue;
      }

      section = findSection(sectionName.c_str());
      if (section == NULL) {
        MvrLog::log(MvrLog::Normal,
                    "MvrConfig::parseResourceFile() error finding new section %s",
                    sectionName.c_str());    
        continue;
      }
    }

    std::string argName = MvrConfigArg::parseResourceArgName(&builder, myLogPrefix.c_str());

    // An empty argument name means that the section comment should be set.
    if (argName.empty()) 
    {
      std::string desc = MvrConfigArg::parseResourceDescription(&builder, myLogPrefix.c_str());
   
      if (!desc.empty()) 
      {
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfig::parseResourceFile() setting comment for section %s",
                            section->getName()));
        section->setComment(desc.c_str());
      }
      continue;
    } // end if no parameter name

    MvrConfigArg::Type argType = MvrConfigArg::parseResourceType(&builder, myLogPrefix.c_str());

    bool isTopLevel = MvrConfigArg::isResourceTopLevel(&builder, myLogPrefix.c_str());
 
    MvrConfigArg *curArg = NULL;
    MvrConfigArg *parentArg = NULL;

    if (isTopLevel) 
    { 
      curArg = section->findParam(argName.c_str());
    }
    else 
    {
      std::list<std::string> parentPath = MvrConfigArg::parseResourceParentPath(&builder, 
                                                                                '|', 
                                                                                myLogPrefix.c_str());
      parentArg = section->findParam(parentPath, true); // allow holders
      if (parentArg != NULL) {
        curArg = parentArg->findArg(argName.c_str());
      }
    }

    if (curArg == NULL) 
    {
      IFDEBUG(MvrLog::log(MvrLog::Normal, "MvrConfig::parseResourceFile() creating arg for %s", argName.c_str()));
      if ((argType != MvrConfigArg::LIST) && (argType != MvrConfigArg::LIST_HOLDER)) 
      {
        curArg = new MvrConfigArg(MvrConfigArg::STRING_HOLDER, argName.c_str(), "");
      }
      else {
        curArg = new MvrConfigArg(MvrConfigArg::LIST_HOLDER, argName.c_str(), "");
      }
        
      if (curArg->parseResource(&builder, NULL, 0, myLogPrefix.c_str())) 
      {  
        if (isTopLevel) 
        {
            addParamAsIs(*curArg, sectionName.c_str());
        }
        else if (parentArg != NULL) 
        {
          parentArg->addArg(*curArg);
        }
      }
    }
    else 
    {
      IFDEBUG(MvrLog::log(MvrLog::Normal,
                          "MvrConfig::parseResourceFile() found arg for %s",
                          argName.c_str()));
      curArg->parseResource(&builder, NULL, 0, myLogPrefix.c_str());
    }
  } // end while more lines to read
  fclose(file);
  return ret;
}
  
/// Parse a config resource file with parameters suitable for custom commands.
MVREXPORT void MvrConfig::parseResourceFile(MvrArgumentBuilder *builder)
{
  if ((builder == NULL) || (builder->getArgc() <= 0) || (builder->getArg(0) == NULL)) 
  {
    MvrLog::log(MvrLog::Normal, "MvrConfig::parseResourceFile() invalid input");
    return;
  }
  parseResourceFile(builder->getArg(0));
} // end method parseResourceFile

/// Write a config resource file, for translation.
MVREXPORT bool MvrConfig::writeResourceFile(const char *fileName, 
                                            bool append,
                                            std::set<std::string> *alreadyWritten,
                                            std::list<std::string> *sectionsToWrite)
{
  FILE *file = NULL;
  char headerBuf[1024];

  std::set<std::string> writtenSet;
  std::set<std::string> *written = NULL;
  if (alreadyWritten != NULL)
    written = alreadyWritten;
  else
    written = &writtenSet;

  // later this'll have a prefix
  std::string realFileName;
  if (fileName[0] == '/' || fileName[0] == '\\')
  {
    realFileName = fileName;
  }
  else
  {
    realFileName = myBaseDirectory;
    realFileName += fileName;
  }

  std::string mode = ((!append) ? "wb" : "a");

  if ((file = MvrUtil::fopen(realFileName.c_str(), mode.c_str())) == NULL)
  {
    MvrLog::log(MvLog::Terse, "%sCannot open file '%s' for writing",
	              myLogPrefix.c_str(), realFileName.c_str());
    return false;
  }
  // Start the resource file with version information
  fprintf(file, "%s %s\n", RESOURCE_VERSION_TAG, CURRENT_RESOURCE_VERSION);

  int headerLineCount = MvrConfigArg::writeResourceHeader(file, 
                                                          headerBuf, 
                                                          sizeof(headerBuf), 
                                                          myCsvSeparatorChar, 
                                                          "SECTION",
                                                          myLogPrefix.c_str());

  MvrLog::log(MvrLog::Normal,
              "%sMvrConfig::writeResourceFile() file %s, %s %s, header contains %i lines",
              myLogPrefix.c_str(), 
              realFileName.c_str(),
              RESOURCE_VERSION_TAG,
              CURRENT_RESOURCE_VERSION,
              headerLineCount);
  
  bool firstSection = true;
  MvrConfigSection *section = NULL;
  
  if (((section = findSection("")) != NULL) && (sectionsToWrite == NULL))
  {
    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;
    writeSectionResource(section, file, written);
  }

  // then write out the rest (skip the generic section if we have one)
  for (std::list<MvrConfigSection *>::iterator sectionIt = mySections.begin(); sectionIt != mySections.end(); sectionIt++)
  {
    section = (*sectionIt);
    if (strcmp(section->getName(), "") == 0)
      continue;

    if (sectionsToWrite != NULL) 
    {
      bool isSectionFound = false;
      for (std::list<std::string>::iterator swIter = sectionsToWrite->begin(); swIter != sectionsToWrite->end(); swIter++) 
      {
        std::string sp = *swIter;
        if (MvrUtil::strcasecmp(section->getName(), sp.c_str()) == 0) 
        {
          isSectionFound = true;
          break;
        }
      } // end for each section to write

      if (!isSectionFound) {
        continue;
      }
    
    } // end if sections specified

    if (!firstSection)
      fprintf(file, "\n");
    firstSection = false;

    writeSectionResource(section, file, written);
  }
  fclose(file);
  return true;
} // end method writeResourceFile

/// Write a config resource file with parameters suitable for custom commands.
MVREXPORT void MvrConfig::writeResourceFile(MvrArgumentBuilder *builder)
{
  if ((builder == NULL) || (builder->getArgc() <= 0) || (builder->getArg(0) == NULL)) 
  {
    MvrLog::log(MvrLog::Normal, "MvrConfig::writeResourceFile() invalid input");
    return;
  }
  writeResourceFile(builder->getArg(0));
} // end method writeResourceFile


MVREXPORT void MvrConfig::writeSection(MvrConfigSection *section, 
                                       FILE *file,
                                       std::set<std::string> *alreadyWritten,
                                       bool writeExtras,
                                       MvrPriority::Priority highestPriority,
                                       MvrPriority::Priority lowestPriority)
{
  // holds each line
  char line[1024];
  //// holds the fprintf
  //char startLine[128];
  bool commented = false;
  unsigned int startCommentColumn = 25;

  /// clear out our written ones between sections
  alreadyWritten->clear();
  
  if (!MvrUtil::isStrEmpty(section->getName())) 
  {
    //fprintf(file, "; %s\n", section->getName());
    fprintf(file, "Section %s\n", section->getName());
  }

  sprintf(line, "; ");

  if (!MvrUtil::isStrEmpty(section->getComment())) 
  {
     MvrConfigArg::writeMultiLineComment(section->getComment(),
                                         file,
                                         line,
                                         sizeof(line),
                                         "; ");
  }
  
  fprintf(file, ";SectionFlags for %s: %s\n", section->getName(), section->getFlags());
  
  std::list<MvrConfigArg> *params = section->getParams();
  
  if (params == NULL) {
    return;
  }

  for (std::list<MvrConfigArg>::iterator paramIt = params->begin(); paramIt != params->end(); paramIt++)
  {
    commented = false;
    
    MvrConfigArg *param = &(*paramIt);
    
    if (param->getType() == MvrConfigArg::SEPARATOR) {
      continue;
    }
    if (alreadyWritten != NULL && param->getType() != MvrConfigArg::DESCRIPTION_HOLDER &&
	      alreadyWritten->find(param->getName()) != alreadyWritten->end()) {
      continue;
    }
    else if (alreadyWritten != NULL && param->getType() != MvrConfigArg::DESCRIPTION_HOLDER)
    {
      alreadyWritten->insert(param->getName());
    }

    // If the parameter's priority does not fall in the specified bounds,
    // then do not write it.
    if ((param->getConfigPriority() < highestPriority) || (param->getConfigPriority() > lowestPriority)) 
    {
      continue;
    }

    // Write the parameter to the file.
    param->writeArguments(file,
                          line,
                          sizeof(line),
                          startCommentColumn,
                          writeExtras,
                          myLogPrefix.c_str());
    // now put a blank line between params if we should
    if (!myNoBlanksBetweenParams)
      fprintf(file, "\n");
  }
}

MVREXPORT void MvrConfig::writeSectionResource(MvrConfigSection *section, 
                                               FILE *file,
                                               std::set<std::string> *alreadyWritten)
{
  if ((section == NULL) || (file == NULL)) 
  {
    MvrLog::log(MvrLog::Normal, "MvrConfig::writeSectionResource() error, invalid input");
    return;
  }

  // holds each line
  char line[10000]; 
  const size_t lineSize = sizeof(line);
 
  /// clear out our written ones between sections
  if (alreadyWritten != NULL) {
    alreadyWritten->clear();
  }

  MvrConfigArg::writeResourceSectionHeader(file,
                                           line,
                                           lineSize,
                                           myCsvSeparatorChar,
                                           section->getName(),
                                           section->getComment(),
                                           "",   
                                           "",   
                                           true, 
                                           myLogPrefix.c_str());
 
  std::list<MvrConfigArg> *params = section->getParams();
  
  if (params == NULL) {
    return;
  }

  for (std::list<MvrConfigArg>::iterator paramIt = params->begin(); 
       paramIt != params->end(); 
       paramIt++)
  {
    
    MvrConfigArg *param = &(*paramIt);
    
    if (param->getType() == MvrConfigArg::SEPARATOR) {
      continue;
    }
    if (MvrUtil::isStrEmpty(param->getName())) {
      continue;
    }
    if (alreadyWritten != NULL && 
	      param->getType() != MvrConfigArg::DESCRIPTION_HOLDER &&
	      alreadyWritten->find(param->getName()) != alreadyWritten->end())
      continue;

    else if (alreadyWritten != NULL && 
	           param->getType() != MvrConfigArg::DESCRIPTION_HOLDER)
    {
      alreadyWritten->insert(param->getName());
    }
   
    // Write the parameter to the file.
    param->writeResource(file,
                         line,
                         lineSize,
                         myCsvSeparatorChar,
                         section->getName(),
                         myLogPrefix.c_str());
    
  }
} // end method writeSectionResource

MVEXPORT void MvrConfig::translateSection(MvrConfigSection *section)
{
  if ((section == NULL) || (MvrUtil::isStrEmpty(section->getName()))) {
    return;
  }

  IFDEBUG(MvrLog::log(MvrLog::Normal,
                      "%sMvrConfig::translateSection() %stranslator for %s",
                      myLogPrefix.c_str(),
                      ((myTranslator != NULL) ? "" : "NULL "),
                      section->getName()));
  
  if (myTranslator != NULL) 
  {
    MvrConfigSection *xltrSection = myTranslator->findSection(section->getName());

    if (xltrSection != NULL) {
      if (!MvrUtil::isStrEmpty(xltrSection->getComment())) {
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfig::addSection() translator section for %s",
                            section->getName()));
        section->setComment(xltrSection->getComment());
      }
      else { 
        IFDEBUG(MvrLog::log(MvrLog::Normal,
                            "MvrConfig::addSection() no comment in translator section for %s",
                            section->getName()));
      }
    }
    else {
      MvrLog::log(MvrLog::Normal,
                  "MvrConfig::addSection() did not find translator section for %s",
                  section->getName());
    }
  }
}


MVREXPORT const char *MvrConfig::getBaseDirectory(void) const
{
  return myBaseDirectory.c_str();
}

MVREXPORT void MvrConfig::setBaseDirectory(const char *baseDirectory)
{
  if (baseDirectory != NULL && strlen(baseDirectory) > 0)
    myBaseDirectory = baseDirectory;
  else
    myBaseDirectory = "";
   
  myParser.setBaseDirectory(baseDirectory);
}

MVREXPORT const char *MvrConfig::getFileName(void) const
{
  return myFileName.c_str();
}

/**
   After a file has been read all the way these processFileCBs are
   called in the priority (higher numbers first)... these are only
   called if there were no errors parsing the file or
   continueOnError was set to false when parseFile was called

   @param functor the functor to call 
   @param priority the functors are called in descending order, if two
   things have the same number the first one added is the first one
   called
**/
MVREXPORT void MvrConfig::addProcessFileCB(MvrRetFunctor<bool> *functor, int priority)
{
  myProcessFileCBList.insert(std::pair<int, ProcessFileCBType *>(-priority, new ProcessFileCBType(functor)));
}

/** 
    Removes a processFileCB, see addProcessFileCB for details
 **/
MVREXPORT void MvrConfig::remProcessFileCB(MvrRetFunctor<bool> *functor)
{
  std::multimap<int, ProcessFileCBType *>::iterator it;
  ProcessFileCBType *cb;

  for (it = myProcessFileCBList.begin(); it != myProcessFileCBList.end(); ++it)
  {
    if ((*it).second->haveFunctor(functor))
    {
      cb = (*it).second;
      myProcessFileCBList.erase(it);
      delete cb;
      remProcessFileCB(functor);
      return;
    }
  }
}

/**
   After a file has been read all the way these processFileCBs are
   called in the priority (higher numbers first)... these are only
   called if there were no errors parsing the file or
   continueOnError was set to false when parseFile was called

   @param functor the functor to call 
   @param priority the functors are called in descending order, if two
          things have the same number the first one added is the first one
          called
**/
MVREXPORT void MvrConfig::addProcessFileCB(MvrRetFunctor2<bool, char *, size_t> *functor, int priority)
{
  if (functor->getName() != NULL && functor->getName()[0] != '\0')
    MvrLog::log(MvrLog::Normal, "MvrConfig::addProcessFileCB: Adding error handler callback %s that should have used addProcessFileWithErrorCB", functor->getName());
  else
    MvrLog::log(MvrLog::Normal, "MvrConfig::addProcessFileCB: Adding anonymous error handler callback that should have used addProcessFileWithErrorCB");
  myProcessFileCBList.insert(std::pair<int, ProcessFileCBType *>(-priority, new ProcessFileCBType(functor)));
}

/**
   This function has a different name than addProcessFileCB just so
   that if you mean to get this function but have the wrong functor
   you'll get an error.  The rem's are the same though since that
   shouldn't matter.

   @param functor the functor to call (the char * and unsigned int
   should be used by the functor to put an error message into that
   buffer)

   @param priority the functors are called in descending order, if two
   things have the same number the first one added is the first one
   called
**/
MVREXPORT void MvrConfig::addProcessFileWithErrorCB(MvrRetFunctor2<bool, char *, size_t> *functor, int priority)
{
  myProcessFileCBList.insert( std::pair<int, ProcessFileCBType *>(-priority, new ProcessFileCBType(functor)));
}

/** 
    Removes a processFileCB, see addProcessFileCB for details
 **/
MVREXPORT void MvrConfig::remProcessFileCB(MvrRetFunctor2<bool, char *, size_t> *functor)
{
  std::multimap<int, ProcessFileCBType *>::iterator it;
  ProcessFileCBType *cb;

  for (it = myProcessFileCBList.begin(); it != myProcessFileCBList.end(); ++it)
  {
    if ((*it).second->haveFunctor(functor))
    {
      cb = (*it).second;
      myProcessFileCBList.erase(it);
      delete cb;
      remProcessFileCB(functor);
    }
  }
}

MVREXPORT bool MvrConfig::callProcessFileCallBacks(bool continueOnErrors,
                                                   char *errorBuffer,
                                                   size_t errorBufferLen)
{
  bool ret = true;
  std::multimap<int, ProcessFileCBType *>::iterator it;
  ProcessFileCBType *callback;
  MvrLog::LogLevel level = myProcessFileCallbacksLogLevel;

  // reset our section to nothing again
  mySection = "";

  // in windows, can't simply declare an array of errorBufferLen -- so
  // allocate one.
  
  // empty the buffer, we're only going to put the first error in it
  if (errorBuffer != NULL && errorBufferLen > 0)
    errorBuffer[0] = '\0';

  MvrLog::log(level, "%sProcessing file", myLogPrefix.c_str());
  for (it = myProcessFileCBList.begin(); it != myProcessFileCBList.end(); ++it)
  {
    callback = (*it).second;
    if (callback->getName() != NULL && callback->getName()[0] != '\0')
      MvrLog::log(level, "%sProcessing functor '%s' (%d)", 
                  myLogPrefix.c_str(),
		              callback->getName(), -(*it).first);
    else
      MvrLog::log(level, "%sProcessing unnamed functor (%d)", myLogPrefix.c_str(), -(*it).first);
    if (!(*it).second->call(errorBuffer, errorBufferLen))
    {
      //printf("# %s\n", scratchBuffer); 

      // if there is an error buffer and it got filled get rid of our
      // pointer to it
      if (errorBuffer != NULL && errorBuffer[0] != '\0')
      {
        errorBuffer = NULL;
        errorBufferLen = 0;
      }
      ret = false;
      if (!continueOnErrors)
      {
	      if (callback->getName() != NULL && callback->getName()[0] != '\0')
	        MvrLog::log(MvrLog::Normal, "MvrConfig: Failed, stopping because the '%s' process file callback failed", 
		                  callback->getName());
	      else
	        MvrLog::log(MvrLog::Normal, "MvrConfig: Failed, stopping because unnamed process file callback failed");
	      break;
      }
      else
      {
	      if (callback->getName() != NULL && callback->getName()[0] != '\0')
	        MvrLog::log(MvrLog::Normal, "MvrConfig: Failed but continuing, the '%s' process file callback failed", 
		                 callback->getName());
	      else
	        MvrLog::log(MvrLog::Normal, "MvrConfig: Failed but continuing, an unnamed process file callback failed");
      }
    }
  }
  if (ret || continueOnErrors)
  {
    MvrLog::log(level, "%sProcessing with own processFile", myLogPrefix.c_str());
    if (!processFile())
      ret = false;
  }
  MvrLog::log(level, "%sDone processing file, ret is %s", myLogPrefix.c_str(),
	            MvrUtil::convertBool(ret));

  return ret;
}

  
MVREXPORT std::list<std::string> MvrConfig::getCategoryNames() const
{
  std::list<std::string> retList;

  for (std::map<std::string, std::list<std::string> >::const_iterator iter = myCategoryToSectionsMap.begin();
       iter != myCategoryToSectionsMap.end();
       iter++) 
  {
    retList.push_back(iter->first);
  }
  return retList;

} // end method getCategoryNames


/**
 * @param categoryName the char * name of the category for which to find the 
 * section names; if empty/null, then the method returns the names of all
 * sections that have not been included in any category
**/
MVREXPORT std::list<std::string> MvrConfig::getSectionNamesInCategory(const char *categoryName) const
{
  std::list<std::string> retList;
  
  if (!MvrUtil::isStrEmpty(categoryName)) 
  {
    std::map<std::string, std::list<std::string> >::const_iterator iter = myCategoryToSectionsMap.find(categoryName);
    if (iter != myCategoryToSectionsMap.end()) 
    {
      retList = iter->second;
    }
  }
  else { // find sections that have no category
   
    for (std::list<MvrConfigSection *>::const_iterator sIter = mySections.begin(); 
         sIter != mySections.end();
         sIter++) 
    {                       
      const MvrConfigSection *section = *sIter;
      if ((section != NULL) &&
          (!MvrUtil::isStrEmpty(section->getName())) &&
          (MvrUtil::isStrEmpty(section->getCategoryName()))) {
        retList.push_back(section->getName());
      }
    } // end for each section
  } // end else find sections that have no category
  return retList;
} // end method getSectionNamesInCategory
  

MVREXPORT std::list<std::string> MvrConfig::getSectionNames() const
{
   std::list<std::string> retList;

   for (std::list<MvrConfigSection *>::const_iterator sIter = mySections.begin(); 
        sIter != mySections.end();
        sIter++) {
                           
    const MvrConfigSection *section = *sIter;
    if ((section != NULL) &&(!MvrUtil::isStrEmpty(section->getName()))) 
    {
      retList.push_back(section->getName());
    }
  } // end for each section
   
  return retList;

} // end method getSectionNames


MVREXPORT std::list<MvrConfigSection *> *MvrConfig::getSections(void)
{
  return &mySections;
}


MVREXPORT void MvrConfig::setNoBlanksBetweenParams(bool noBlanksBetweenParams)
{
  myNoBlanksBetweenParams = noBlanksBetweenParams;
}

MVREXPORT bool MvrConfig::getNoBlanksBetweenParams(void)
{
  return myNoBlanksBetweenParams;
}

/**
   This argument parser the arguments in to check for parameters of
   this name, note that ONLY the first parameter of this name will be
   used, so if you have duplicates only the first one will be set.
 **/
MVREXPORT void MvrConfig::useArgumentParser(MvrArgumentParser *parser)
{
  myArgumentParser = parser;
}

MVREXPORT bool MvrConfig::parseArgumentParser(MvrArgumentParser *parser, 	
                                              bool continueOnError,
                                              char *errorBuffer,
                                              size_t errorBufferLen)
{
  std::list<MvrConfigSection *>::iterator sectionIt;
  std::list<MvrConfigArg>::iterator paramIt;
  MvrConfigSection *section = NULL;
  MvrConfigArg *param = NULL;
  std::list<MvrConfigArg> *params = NULL;

  bool ret;
  size_t i;
  std::string strArg;
  std::string strUndashArg;
  MvrArgumentBuilder builder;
  builder.setQuiet(myIsQuiet);

  bool plainMatch;

  for (i = 0; i < parser->getArgc(); i++)
  {
    if (parser->getArg(i) == NULL)
    {
      MvrLog::log(MvrLog::Terse, "%sset up wrong (parseArgumentParser broken).",
                  myLogPrefix.c_str());
      if (errorBuffer != NULL)
        strncpy(errorBuffer, "MvrConfig set up wrong (parseArgumentParser broken).", errorBufferLen);
      return false;
    }
    strArg = parser->getArg(i); 
    if (parser->getArg(i)[0] == '-')
      strUndashArg += &parser->getArg(i)[1]; 
    else
      strUndashArg = "";
    //printf("normal %s undash %s\n", strArg.c_str(), strUndashArg.c_str());
    for (sectionIt = mySections.begin(); sectionIt != mySections.end(); sectionIt++)
    {
      section = (*sectionIt);
      params = section->getParams();

      for (paramIt = params->begin(); paramIt != params->end(); paramIt++)
      {
        param = &(*paramIt);

        if (strlen(param->getName()) > 0 &&
          ((plainMatch = MvrUtil::strcasecmp(param->getName(),strArg)) == 0 ||
          MvrUtil::strcasecmp(param->getName(), strUndashArg) == 0))
        {
          if (plainMatch == 0)
            builder.setExtraString(strArg.c_str());
          else
            builder.setExtraString(strUndashArg.c_str());
          if (i+1 < parser->getArgc())
          {
            builder.addPlain(parser->getArg(i+1));
            parser->removeArg(i+1);
          }
          parser->removeArg(i);

          // set us to use the section this is in and then parse the argument
          std::string oldSection = mySection;
          bool oldSectionBroken = mySectionBroken;
          bool oldSectionIgnored = mySectionIgnored;
          bool oldUsingSections = myUsingSections;
          bool oldDuplicateParams = myDuplicateParams;
          mySection = section->getName();
          mySectionBroken = false;
          mySectionIgnored = false; // ??
          myUsingSections = true;
          myDuplicateParams = false;
          ret = parseArgument(&builder, errorBuffer, errorBufferLen);
          mySection = oldSection;
          mySectionBroken = oldSectionBroken;
          mySectionIgnored = oldSectionIgnored;
          myUsingSections = oldUsingSections;
          myDuplicateParams = oldDuplicateParams;

          // if we parsed the argument right or are continuing on
          // errors call ourselves again (so we don't hose iterators up above)
          if (ret || continueOnError)
          {
            //printf("Ret %s\n", MvrUtil::convertBool(ret));
            return ret && parseArgumentParser(parser, continueOnError, errorBuffer, errorBufferLen);
          }
          else
            return false;
        }
      }
    }
  }

  return true;
}

MVREXPORT MvrConfigSection *MvrConfig::findSection(const char *sectionName) const
{
  if (sectionName == NULL) 
  {
    MvrLog::log(MvrLog::Normal,"MvrConfig::findSection() cannot find NULL section name");
    return NULL;
  }

  MvrConfigSection *section = NULL;
  MvrConfigSection *tempSection = NULL;

  for (std::list<MvrConfigSection *>::const_iterator sectionIt = mySections.begin(); sectionIt != mySections.end(); sectionIt++)
  {
    tempSection = (*sectionIt);
    if (tempSection == NULL) {
      MvrLog::log(MvrLog::Normal,
                  "MvrConfig::findSection(%s) unexpected null section in config",
                  sectionName);
      continue;
    }

   
    if (MvrUtil::strcasecmp(tempSection->getName(), sectionName) == 0)
    {
      section = tempSection;
      break;
    }
  }
  return section;

} // end method findSection


void MvrConfig::copySectionsToParse(std::list<std::string> *from)
{
  if (mySectionsToParse != NULL)
    delete mySectionsToParse;
  std::string sections;
  mySectionsToParse = NULL;
  if (from != NULL) {
    mySectionsToParse = new std::list<std::string>();
    for (std::list<std::string>::const_iterator spIter = from->begin(); spIter != from->end(); spIter++) 
    {
      sections += "'";
      sections += (*spIter);
      sections += "' ";
      mySectionsToParse->push_back(*spIter);
    } // end for each section to parse
    MvrLog::log(MvrLog::Verbose, "Will parse section(s) %s", sections.c_str());
  } // end if copy sections to parse
  else
  {
    MvrLog::log(MvrLog::Verbose, "Will parse all sections");
  }
} // end method copySectionsToParse

MVREXPORT void MvrConfig::addSectionNotToParse(const char *section)
{
  MvrLog::log(MvrLog::Normal, "%sWill not parse section %s", 
	            myLogPrefix.c_str(), section);
  mySectionsNotToParse.insert(section);
} // end method copySectionsToParse



MVREXPORT void MvrConfig::remSectionNotToParse(const char *section)
{
  MvrLog::log(MvrLog::Normal, "%sWill not not parse section %s", myLogPrefix.c_str(), section);
  mySectionsNotToParse.erase(section);
} // end method copySectionsToParse



MVREXPORT void MvrConfig::clearAllValueSet(void)
{
  std::list<<MvrConfigSection *> *sections;
  MvrConfigSection *section;
  std::list<MvrConfigSection *>::iterator sectionIt;
  
  std::list<MvrConfigArg> *params;
  MvrConfigArg *param;
  std::list<MvrConfigArg>::iterator paramIt;

  sections = getSections();
  for (sectionIt = sections->begin(); sectionIt != sections->end(); sectionIt++)
  {
    section = (*sectionIt);
    params = section->getParams();
    for (paramIt = params->begin(); paramIt != params->end(); paramIt++)
    {
      param = &(*paramIt);
      param->clearValueSet();
    }
  }
}


MVREXPORT void MvrConfig::removeAllUnsetValues(void)
{
  removeAllUnsetValues(false);
}

MVREXPORT void MvrConfig::removeAllUnsetSections(void)
{
  removeAllUnsetValues(true);
}


MVREXPORT void MvrConfig::removeAllUnsetValues(bool isRemovingUnsetSectionsOnly)
{
  std::list<MvrConfigSection *> *sections;
  MvrConfigSection *section;
  std::list<MvrConfigSection *>::iterator sectionIt;
  
  std::list<MvrConfigArg> *params;
  MvrConfigArg *param;
  std::list<MvrConfigArg>::iterator paramIt;
  std::list<std::list<MvrConfigArg>::iterator> removeParams;
  std::list<std::list<MvrConfigArg>::iterator>::iterator removeParamsIt;

  sections = getSections();
  for (sectionIt = sections->begin(); sectionIt != sections->end(); sectionIt++)
  {
    section = (*sectionIt);
    params = section->getParams();
    for (paramIt = params->begin(); paramIt != params->end(); paramIt++)
    {
      param = &(*paramIt);
      if (param->getType() != MvrConfigArg::SEPARATOR && 
	        param->getType() != MvrConfigArg::STRING_HOLDER && 
	        param->getType() != MvrConfigArg::LIST_HOLDER && 
	        param->getType() != MvrConfigArg::DESCRIPTION_HOLDER)
      {
        if (!param->isValueSet()) {
	        removeParams.push_back(paramIt);
        }
        else if (isRemovingUnsetSectionsOnly) {
          removeParams.clear();
          break;
        }
      }
    }
    while ((removeParamsIt = removeParams.begin()) != removeParams.end())
    {
      MvrLog::log(MvrLog::Verbose, 
		              "%s:removeAllUnsetValues: Removing %s:%s", 
                  myLogPrefix.c_str(),
		              section->getName(), (*(*removeParamsIt)).getName());
                  section->getParams()->erase((*removeParamsIt));
                  removeParams.pop_front();      
    }
  }
}



/**
   This is more temporary than some of the others like 'setSaveUknown'
   and such, this should be called by anything that's going to parse
   the config...
**/
MVREXPORT void MvrConfig::setPermissions(bool allowFactory, bool saveUnknown)
{
  myPermissionAllowFactory = allowFactory;
  myPermissionSaveUnknown = saveUnknown;
  
  MvrLog::log(MvrLog::Normal, "%s: Setting permissions of allowFactory to %s and saveUnknown to %s",
	            myLogPrefix.c_str(), 
              MvrUtil::convertBool(myPermissionAllowFactory),
              MvrUtil::convertBool(myPermissionSaveUnknown));
}

MVREXPORT MvrConfigArg::RestartLevel MvrConfig::getRestartLevelNeeded(void) const
{
  return myRestartLevelNeeded;
}

MVREXPORT void MvrConfig::resetRestartLevelNeeded(void) 
{
  myRestartLevelNeeded = MvrConfigArg::NO_RESTART;
}


MVREXPORT void MvrConfig::addListNamesToParser(const MvrConfigArg &parent)
{
  if (parent.getType() != MvrConfigArg::LIST)
    return;
  
  std::list<MvrConfigArg> children = parent.getArgs();
  std::list<MvrConfigArg>::iterator it;
  
  for (it = children.begin(); it != children.end(); it++)
  {
    MvrConfigArg child = (*it);
    //MvrLog::log(MvrLog::Normal, "@@@ Adding %s", child.getName());
    if (!myParser.addHandlerWithError(child.getName(), &myParserCB))
    {
      if (!myIsQuiet) 
      {
	      MvrLog::log(MvrLog::Verbose, "%sCould not add parameter '%s' to file parser, probably already there.", 
		    myLogPrefix.c_str(), child.getName());
      }
    }
    addListNamesToParser(child);
  }
}

MVREXPORT MvrConfigSection::MvrConfigSection(const char *name, 
                                             const char *comment,
                                             bool isQuiet,
                                             const char *categoryName) :
  myName((name != NULL) ? name : ""),
  myComment((comment != NULL) ? comment : ""),
  myCategoryName((categoryName != NULL) ? categoryName : ""),
  myDisplayName(""),
  myFlags(NULL),
  myParams(),
  myIsQuiet(isQuiet)
{
  myFlags = new MvrArgumentBuilder(512, '|');
  myFlags->setQuiet(myIsQuiet);
}

MVREXPORT MvrConfigSection::~MvrConfigSection()
{
  delete myFlags;
}


MVREXPORT MvrConfigSection::MvrConfigSection(const MvrConfigSection &section) 
{
  myName = section.myName;
  myComment = section.myComment;
  myCategoryName = section.myCategoryName;
  myDisplayName = section.myDisplayName;

  myFlags = new MvrArgumentBuilder(512, '|');
  // Since any messages were logged when the first section was created,
  // it doesn't seem necessary to log them again.
  myFlags->setQuiet(true);
  myFlags->addPlain(section.myFlags->getFullString());

  for (std::list<MvrConfigArg>::const_iterator it = section.myParams.begin(); 
       it != section.myParams.end(); 
       it++) 
  {
    myParams.push_back(*it);
  }
  myIsQuiet = section.myIsQuiet;
}

MVREXPORT MvrConfigSection &MvrConfigSection::operator=(const MvrConfigSection &section) 
{
  if (this != &section) 
  {
    
    myName = section.getName();
    myComment = section.getComment();
    myCategoryName = section.getCategoryName();
    myDisplayName = section.myDisplayName;
  
    delete myFlags;
    myFlags = new MvrArgumentBuilder(512, '|');
    //myFlags->setQuiet(myIsQuiet);
    myFlags->addPlain(section.myFlags->getFullString());
    
    for (std::list<MvrConfigArg>::const_iterator it = section.myParams.begin(); 
         it != section.myParams.end(); 
         it++) 
    {
      myParams.push_back(*it);
    }
    myIsQuiet = section.myIsQuiet;
  }
  return *this;
}


MVREXPORT void MvrConfigSection::copyAndDetach(const MvrConfigSection &section) 
{
  if (this != &section) 
  {
    
    myName = section.getName();
    myComment = section.getComment();
    myCategoryName = section.getCategoryName();
    myDisplayName = section.myDisplayName;

    delete myFlags;
    myFlags = new MvrArgumentBuilder(512, '|');
    myFlags->setQuiet(myIsQuiet);

    myFlags->addPlain(section.myFlags->getFullString());
    
    for (std::list<MvrConfigArg>::const_iterator it = section.myParams.begin(); 
	       it != section.myParams.end(); 
	       it++) 
    {
      MvrConfigArg paramCopy;
      paramCopy.copyAndDetach(*it);
      myParams.push_back(paramCopy);
    }

    myIsQuiet = section.myIsQuiet;
  }
  //return *this;
}

MVREXPORT MvrConfigArg *MvrConfigSection::findParam(const char *paramName,
                                                    bool isAllowStringHolders)
{
  MvrConfigArg *param = NULL;
  MvrConfigArg *tempParam = NULL;

  for (std::list<MvrConfigArg>::iterator pIter = myParams.begin(); 
       pIter != myParams.end(); 
       pIter++)
  {
    tempParam = &(*pIter);
    // ignore string holders 
    if (!isAllowStringHolders &&
        ((tempParam->getType() == MvrConfigArg::STRING_HOLDER) || 
        (tempParam->getType() == MvrConfigArg::LIST_HOLDER)))
      continue;
    if (MvrUtil::strcasecmp(tempParam->getName(), paramName) == 0)
    {
      param = tempParam;
    }
  }
  return param;

} // end method findParam

/**
 * This method provides a shortcut for looking up child parameters 
 * in a list type parameter that is contained in the section.
 * @param paramNamePath a list of strings that specifies the sequence of parameter
 * names, from "top" to "bottom"
 * @return MvrConfigArg *, a pointer to the requested parameter, or NULL
 * if not found
**/ 
MVREXPORT MvrConfigArg *MvrConfigSection::findParam(const std::list<std::string> &paramNamePath,
                                                    bool isAllowHolders)
{
  if (paramNamePath.empty()) 
  {
    return NULL;
  }

  MvrConfigArg *topParam = findParam(paramNamePath.front().c_str(), isAllowHolders);
  if (topParam == NULL) {
    return NULL;
  }

  // Handle the degenerate case where this method is used in a non-list lookup
  if (paramNamePath.size() == 1) {
    return topParam;
  }

  MvrConfigArg *curParam = topParam;

  std::list<std::string>::const_iterator iter = paramNamePath.begin();
  iter++; // skipping the front one from above
  
  for (iter; iter != paramNamePath.end(); iter++) 
  {
    // If curParam is not a LIST type, then findArg will return NULL.
    curParam = curParam->findArg((*iter).c_str());
  }
  return curParam;
}


MVREXPORT MvrConfigArg *MvrConfigSection::findParam(const char **paramNamePath, 
                                                    int pathLength,
                                                    bool isAllowHolders)
{
  if ((paramNamePath == NULL) || (pathLength <= 0)) 
  {
    MvrLog::log(MvrLog::Normal,
                "MvrConfigSection::findParam() invalid input, paramNamePath = %p pathLength = %i",
                paramNamePath,
                pathLength);
    return NULL;
  }

  std::list<std::string> nameList;
  for (int i = 0; i < pathLength; i++) 
  {
    nameList.push_back(paramNamePath[i]);
  }
  return findParam(nameList, isAllowHolders);
} 

MVREXPORT bool MvrConfigSection::containsParamsOfPriority(MvrPriority::Priority highestPriority,
                                                          MvrPriority::Priority lowestPriority)
{
  if (highestPriority > lowestPriority) {
    return false;
  }
  for (std::list<MvrConfigArg>::iterator iter = myParams.begin();
       iter != myParams.end();
       iter++) {
    const MvrConfigArg &arg = *iter;

    if (arg.getType() == MvrConfigArg::SEPARATOR) {
      continue;
    }
    if ((arg.getConfigPriority() >= highestPriority) &&
        (arg.getConfigPriority() <= lowestPriority)) {
      return true;
    }
  }
  return false;
}

// Remove list holders
MVREXPORT bool MvrConfigSection::remStringHolder(const char *paramName)
{
  MvrConfigArg *tempParam = NULL;
  
  for (std::list<MvrConfigArg>::iterator pIter = myParams.begin(); 
       pIter != myParams.end(); 
       pIter++)
  {
    tempParam = &(*pIter);
    
     // pay attention to only string holders
    if (MvrUtil::isStrEmpty(paramName)) {
      continue;
    }
    if ((tempParam->getType() != MvrConfigArg::STRING_HOLDER) &&
        (tempParam->getType() != MvrConfigArg::LIST_HOLDER)) { 
      continue;
    }

    if (MvrUtil::strcasecmp(tempParam->getName(), paramName) == 0)
    {
      myParams.erase(pIter);
      // Recurse to ensure that all occurrences of the string holder are removed.
      remStringHolder(paramName);
      return true;
    }
  }
  return false;
}

MVREXPORT bool MvrConfigSection::hasFlag(const char *flag) const
{
  size_t i;
  for (i = 0; i < myFlags->getArgc(); i++)
  {
    if (strcmp(myFlags->getArg(i), flag) == 0)
    {
      return true;
    }
  }
  return false;
}

MVREXPORT bool MvrConfigSection::addFlags(const char *flags, bool isQuiet)
{
  if (myFlags == NULL) {
    return false;
  }
  myFlags->setQuiet(isQuiet);

  //myFlags->addPlain(flags); 

  size_t i;
  MvrArgumentBuilder theseFlags(512, '|');
  theseFlags.setQuiet(isQuiet);

  theseFlags.addPlain(flags);
  
  for (i = 0; i < theseFlags.getArgc(); i++)
  {
    if (!hasFlag(theseFlags.getArg(i)))
      myFlags->addPlain(theseFlags.getArg(i));
  }
  return true; 
}

MVREXPORT bool MvrConfigSection::remFlag(const char *flag)
{
  size_t i;
  
  for (i = 0; i < myFlags->getArgc(); i++)
  {
    if (strcmp(myFlags->getArg(i), flag) == 0)
    {
      myFlags->removeArg(i, true);
      return remFlag(flag);
    }
  }

  return true;
}
  
MVREXPORT void MvrConfigSection::setQuiet(bool isQuiet)
{
  myIsQuiet = isQuiet;
}


MVREXPORT void MvrConfigSection::setName(const char *name) 
{ 
  myName = ((name != NULL) ? name : "");
}
 
MVREXPORT void MvrConfigSection::setComment(const char *comment) 
{ 
  myComment = ((comment != NULL) ? comment : ""); 
}


MVREXPORT const char *MvrConfigSection::getCategoryName() const
{
  return myCategoryName.c_str();
}

void MvrConfigSection::setCategoryName(const char *categoryName)
{
  myCategoryName = ((categoryName != NULL) ? categoryName : "");
}


