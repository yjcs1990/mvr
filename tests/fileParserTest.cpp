#include "Mvria.h"

bool boolPrinter(MvrArgumentBuilder *builder)
{
  if (!builder->isArgBool(0))
  {
    printf("Bad boolean value\n");
    return false;
  }
  if (builder->getArgBool(0))
    printf("bool(%s): true\n", builder->getExtraString());
  else 
    printf("bool(%s): false\n", builder->getExtraString());

  return true;
}

bool intPrinter(MvrArgumentBuilder *builder)
{
  if (!builder->isArgInt(0))
  {
    printf("Bad integer value\n");
    return false;
  }
  printf("int: %d\n", builder->getArgInt(0));
  return true;
}

bool doublePrinter(MvrArgumentBuilder *builder)
{
  if (!builder->isArgDouble(0))
  {
    printf("Bad double value\n");
    return false;
  }
  printf("double: %g\n", builder->getArgDouble(0));
  return true;
}

bool stringPrinter(MvrArgumentBuilder *builder)
{
  printf("string: %s\n", builder->getFullString());
  return true;
}

bool argPrinter(MvrArgumentBuilder *builder)
{
  printf("Logging builder:\n");
  builder->log();
  return true;
}

int main(int argc, char **argv)
{
  MvrGlobalRetFunctor1<bool, MvrArgumentBuilder *> boolFunctor(&boolPrinter);
  MvrGlobalRetFunctor1<bool, MvrArgumentBuilder *> intFunctor(&intPrinter);
  MvrGlobalRetFunctor1<bool, MvrArgumentBuilder *> doubleFunctor(&doublePrinter);
  MvrGlobalRetFunctor1<bool, MvrArgumentBuilder *> stringFunctor(&stringPrinter);
  MvrGlobalRetFunctor1<bool, MvrArgumentBuilder *> argFunctor(&argPrinter);

  MvrFileParser parser(Mvria::getDirectory());

  MvrLog::init(MvrLog::StdOut, MvrLog::Verbose);

  parser.addHandler("keywordBool", &boolFunctor);
  // make sure it won't let us add another on
  parser.addHandler("keywordbool", &boolFunctor);
  parser.addHandler("keywordint", &intFunctor);
  // testing the remHandler
  // parser.remHandler("keywordint");
  // parser.remHandler(&intFunctor);
  parser.addHandler("keyworddouble", &doubleFunctor);
  parser.addHandler("keywordString", &stringFunctor);
  parser.addHandler(NULL, &stringFunctor);
  parser.addHandler("keywordbuilder", &argFunctor);
  // either parse the bad file 
  // parser.parseFile("tests/fileParserTestBad.txt");
  // or parse the good file 
  parser.parseFile("../tests/fileParserTestGood.txt", false);
}