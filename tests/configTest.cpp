#include "Mvria.h"

class ConfigTester : public MvrConfig
{
public:
  ConfigTester();
  virtual ~ConfigTester();
  virtual bool processFile(void);
  int myInt;
  double myDouble;
  bool myBool;
  MvrPose myPose;
  char myString[512];

  // stuff for testing the functor ones
  std::list<std::string> myList;
  std::list<MvrArgumentBuilder *> myArgList;
  bool listAdder(MvrArgumentBuilder *builder) 
  {
    myList.push_front(builder->getFullString()); 
    printf("Added %s\n", builder->getFullString());
    return true; 
  }
  const std::list<MvrArgumentBuilder *> *getList(void) 
  {
    std::list<MvrArgumentBuilder *>::iterator argIt;
    std::list<std::string>::iterator listIt;
    MvrArgumentBuilder *builder;

    if (myArgList.size() != 0)
    {
	    while ((argIt = myArgList.begin()) != myArgList.end())
	    {
	      delete (*argIt);
	      myArgList.pop_front();
	    }
    }
    for (listIt = myList.begin(); listIt != myList.end(); listIt++)
    {
      builder = new MvrArgumentBuilder;
      builder->add((*listIt).c_str());
      myArgList.push_front(builder);
    }
    return &myArgList;
  }
  MvrRetFunctor1C<bool, ConfigTester, MvrArgumentBuilder *> mySetFunctor;
  MvrRetFunctorC<const std::list<MvrArgumentBuilder *> *, ConfigTester> myGetFunctor;

};

ConfigTester::ConfigTester() : 
  mySetFunctor(this, &ConfigTester::listAdder),
  myGetFunctor(this, &ConfigTester::getList)
{
  myInt = 32;
  myDouble = 239.394;
  myBool = true;
  myPose.setPose(42, -42.3, 21.21);
  strcpy(myString, "happy fun string will begin to smoke");

  addParam(MvrConfigArg("int", &myInt, "fun things!"), "fuah");//, 0, 300));
  addParam(MvrConfigArg("double", &myDouble, "fun things double!"));//, 0, 2300));
  addParam(MvrConfigArg("bool", &myBool, "fun things bool!"));
  addParam(MvrConfigArg("string", myString, "fun things string!", sizeof(myString)));
  addParam(MvrConfigArg("functor", &mySetFunctor, &myGetFunctor, "fun functor thing!"));
}

ConfigTester::~ConfigTester()
{
  std::list<MvrArgumentBuilder *>::iterator argIt;
  if (myArgList.size() != 0)
  {
    while ((argIt = myArgList.begin()) != myArgList.end())
    {
      delete (*argIt);
      myArgList.pop_front();
    }
  }
}

bool ConfigTester::processFile(void)
{
  printf("Processed \n");
  return true;
}

bool func100(void)
{
  printf("100\n");
  return true;
}

bool func90a(void)
{
  printf("90a\n");
  return true;
}

bool func90b(void)
{
  printf("90b\n");
  return true;
}

bool func50(void)
{
  printf("50\n");
  return true;
}

int main(int argc, char **argv)
{
  Mvria::init();
  MvrLog::init(MvrLog::StdOut, MvrLog::Verbose, "test.log", true);
  
  MvrArgumentParser parser(&argc, argv);
  ConfigTester tester;
  bool ret;
  tester.writeFile("configBefore.txt");
  char errorBuffer[512];
  errorBuffer[0] = '\0';


  MvrGlobalRetFunctor<bool> func100cb(&func100);
  MvrGlobalRetFunctor<bool> func90acb(&func90a);
  MvrGlobalRetFunctor<bool> func90bcb(&func90b);
  MvrGlobalRetFunctor<bool> func50cb(&func50);

  func100cb.setName("100cb");
  func90bcb.setName("bcb");
  func50cb.setName("50cb");

  tester.addProcessFileCB(&func100cb, 100);
  tester.addProcessFileCB(&func90acb, 90);
  tester.addProcessFileCB(&func90bcb, 90);
  tester.addProcessFileCB(&func50cb, 50);

  tester.useArgumentParser(&parser);
  ret = tester.parseFile("configTest.txt", false, true, errorBuffer, sizeof(errorBuffer));
  if (ret)
  {
    printf("int %d double %g bool %s string '%s'\n", 
	   tester.myInt, tester.myDouble,
	   MvrUtil::convertBool(tester.myBool),
	   tester.myString);
  }
  else
  {
    printf("\nFailed config test because '%s'\n\n", errorBuffer);
  }

  tester.writeFile("configAfter.txt");
  exit(0);
}



