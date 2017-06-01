/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrConfig.cpp
 > Description  : Stores configuration information which may be read to and from files or other sources
 > Author       : Yu Jie
 > Create Time  : 2017年04月20日
 > Modify Time  : 2017年05月20日
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
MVREXPORT MvrConfig::MvrCOnfig(const char *baseDirectory, 
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
  // TODO: The following attributes also need to be copied (or
  // something)
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
  // TODO: The following attributes also need to be copied (or
  // something)
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
 * @param configName a char * descriptive name of the ArConfig instance
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
      ArLog::log(ArLog::Normal, "    Section %s has NULL params",
                  (*it)->getName());
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
 * @param displayHint Suggest an appropriate UI control for editing this parameter's value. See ArConfigArg::setDisplayHint() for description of display hint format.
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
      section = new ArConfigSection(arg->getFullString(), NULL, myIsQuiet);

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

