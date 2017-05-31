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
  
}