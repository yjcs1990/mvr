#include "Mvria.h"

class TestClass
{
public:
  void function();
  void function(int arg1);
  void function(bool arg1, std::string arg2);

  bool retFunction();
  char *retFunction(int arg1);
  double retFunction(bool arg1, std::string arg2);
};

void TestClass::function()
{
  printf("TestClass::function\n");
}

void TestClass::function(int arg1)
{
  printf("TestClass::function(int arg1=%d)\n", arg1);
}

void TestClass::function(bool arg1, std::string arg2)
{
  printf("TestClass::function(bool arg1=%d, std::string arg2='%s')\n",
	       arg1, arg2.c_str());
}

bool TestClass::retFunction()
{
  printf("bool TestClass::retFunction\n");
  return(true);
}

char * TestClass::retFunction(int arg1)
{
  printf("char * TestClass::retFunction(int arg1=%d)\n", arg1);
  return("Hello");
}

double TestClass::retFunction(bool arg1, std::string arg2)
{
  printf("double TestClass::retFunction(bool arg1=%d, std::string arg2='%s')\n",
	 arg1, arg2.c_str());
  return(4.62);
}

void function()
{
  printf("function\n");
}

void function(int arg1)
{
  printf("function(int arg1=%d)\n", arg1);
}

void function(bool arg1, std::string arg2)
{
  printf("function(bool arg1=%d, std::string arg2='%s')\n",
	 arg1, arg2.c_str());
}

bool retFunction()
{
  printf("bool retFunction\n");
  return(true);
}

char * retFunction(int arg1)
{
  printf("char * retFunction(int arg1=%d)\n", arg1);
  return("Hello");
}

double retFunction(bool arg1, std::string arg2)
{
  printf("double retFunction(bool arg1=%d, std::string arg2='%s')\n",
	 arg1, arg2.c_str());
  return(4.62);
}

/*
  Test functors with class member funtions
*/

// Direct invocation of the functors with supplying parameters.
void testDirect()
{
  TestClass test;
  MvrFunctorC<TestClass> functor(test, &TestClass::function);
  MvrFunctor1C<TestClass, int> functor1(test, &TestClass::function, 1);
  MvrFunctor2C<TestClass, bool, std::string> functor2(test, &TestClass::function, false, "default arg");

  printf("\n****** Testing direct invocation using MvrFunctor::invoke(...)\n");
  puts("> Should see TestClass::function()...");
  functor.invoke();
  puts("> Should see TestClass::function(1)...");
  functor1.invoke();
  puts("> Should see TestClass::function(5)...");
  functor1.invoke(5);
  puts("> Should see TestClass::function(true, \"argument 1\")...");
  functor2.invoke(true, "argument 1");
}

// Invocation of a base MvrFunctor pointer to a functor. Because the pointer
// is of type MvrFunctor, the parameters can not be supplied. The default
// parameters, which are supplied when the functor is constructed, are used.
void testBase()
{
  TestClass test;
  MvrFunctor *fptr;
  MvrFunctorC<TestClass> functor(test, &TestClass::function);
  MvrFunctor1C<TestClass, int> functor1(test, &TestClass::function, 1);
  MvrFunctor2C<TestClass, bool, std::string> functor2(test,
						     &TestClass::function,
						     false, "default arg");

  printf("\n****** Testing base invocation\n");
  fptr=&functor;
  puts("> Should see TestClass::function()...");
  fptr->invoke();
  fptr=&functor1;
  puts("> Should see TestClass::function(1)...");
  fptr->invoke();
  fptr=&functor2;
  puts("> Should see TestClass::function(false, \"default arg\")...");
  fptr->invoke();
}

// Invocation of pointers which supply the parameter type. Full invocation
// with paramters is posesible in this fashion with out knowing the class
// that the functor refers to.
void testParams()
{
  TestClass test;
  MvrFunctorC<TestClass> functor(test, &TestClass::function);
  MvrFunctor1C<TestClass, int> functor1(test, &TestClass::function);
  MvrFunctor2C<TestClass, bool, std::string> functor2(test,
						     &TestClass::function);
  MvrFunctor *fptr;
  MvrFunctor1<int> *fptr1;
  MvrFunctor2<bool, std::string> *fptr2;

  printf("\n****** Testing pointer invocation\n");
  fptr=&functor;
  puts("> Should see TestClass::function()...");
  fptr->invoke();
  fptr1=&functor1;
  puts("> Should see TestClass::function(2)...");
  fptr1->invoke(2);
  fptr2=&functor2;
  puts("> Should see TestClass::function(true, \"argument 2\")...");
  fptr2->invoke(true, "argument 2");
}


void setFunctorPtr(MvrFunctor *f)
{
}

void setIntFunctorPtr(MvrFunctor1<int> *f)
{
}

// It is possible to supply a more specialized MvrFunctor class to a function
// that takes a plant MvrFunctor pointer, since it will be
// implicitly cast to that parent class
void testDowncast()
{
  MvrRetFunctor1C<char*, TestClass, int> f;
  MvrFunctor* y = &f;
  setFunctorPtr(&f);
  setFunctorPtr(y);
}

/*
  Test functors with return values, MvrRetFunctor
*/

// Direct invocation of the functors with return values and supplying
// parameters. It is not posesible to have the operator() for functors with
// return values. This is due to limitations of C++ and different C++
// compilers where you can not overload return values in all cases.
void testReturnDirect()
{
  TestClass test;
  MvrRetFunctorC<bool, TestClass> functor(test, &TestClass::retFunction);
  MvrRetFunctor1C<char*, TestClass, int>
    functor1(test, &TestClass::retFunction, 1);
  MvrRetFunctor2C<double, TestClass, bool, std::string>
    functor2(test, &TestClass::retFunction, false, "default arg");
  bool bret;
  char *cret;
  double dret;

  //bret=test.retFunction();
  //cret=test.retFunction(4);
  //dret=test.retFunction(true, "foof");

  printf("\n****** Testing direct invocation with return\n");
  puts("> TestClass::retFunction() should return true...");
  bret=functor.invokeR();
  printf("Returned: %d\n", bret);
  puts("> TestClass::retFunction(5) should return \"Hello\"...");
  cret=functor1.invokeR(5);
  printf("Returned: %s\n", cret);
  puts("> TestClass::retFunction(true, \"argument 1\") should return 4.62...");
  dret=functor2.invokeR(true, "argument 1");
  printf("Returned: %e\n", dret);
}

void testReturnBase()
{
  TestClass test;
  MvrRetFunctorC<bool, TestClass> functor(test, &TestClass::retFunction);
  MvrRetFunctor1C<char*, TestClass, int>
    functor1(test, &TestClass::retFunction, 1);
  MvrRetFunctor2C<double, TestClass, bool, std::string>
    functor2(test, &TestClass::retFunction, false, "default arg");
  MvrRetFunctor<bool> *fBoolPtr;
  MvrRetFunctor<char*> *fCharPtr;
  MvrRetFunctor<double> *fDoublePtr;
  bool bret;
  char *cret;
  double dret;

  printf("\n****** Testing base invocation with return\n");
  fBoolPtr=&functor;
  puts("> TestClass::retFunction() should return true");
  bret=fBoolPtr->invokeR();
  printf("Returned: %d\n", bret);
  fCharPtr=&functor1;
  puts("> TestClass::retFunction(1) should return \"Hello\"");
  cret=fCharPtr->invokeR();
  printf("Returned: %s\n", cret);
  fDoublePtr=&functor2;
  puts("> TestClass::retFunction(false, \"default arg\" should return 4.62");
  dret=fDoublePtr->invokeR();
  printf("Returned: %e\n", dret);
}

void testReturnParams()
{
  TestClass test;
  MvrRetFunctorC<bool, TestClass> functor(test, &TestClass::retFunction);
  MvrRetFunctor1C<char*, TestClass, int>
    functor1(test, &TestClass::retFunction, 1);
  MvrRetFunctor2C<double, TestClass, bool, std::string>
    functor2(test, &TestClass::retFunction, false, "default arg");
  MvrRetFunctor<bool> *fBoolPtr;
  MvrRetFunctor1<char*, int> *fCharPtr;
  MvrRetFunctor2<double, bool, std::string> *fDoublePtr;
  bool bret;
  char *cret;
  double dret;

  printf("\n****** Testing pointer invocation with return\n");
  fBoolPtr=&functor;
  puts("> TestClass::retFunction() should return true");
  bret=fBoolPtr->invokeR();
  printf("Returned: %d\n", bret);
  fCharPtr=&functor1;
  puts("> TestClass::retFunction(7) should return \"Hello\"");
  cret=fCharPtr->invokeR(7);
  printf("Returned: %s\n", cret);
  fDoublePtr=&functor2;
  puts("> TestClass::retFunction(false, \"argument 3\") should return 4.62...");
  dret=fDoublePtr->invokeR(false, "argument 3");
  printf("Returned: %e\n", dret);
}


/*
  Test global functors, MvrGlobalFunctor.
*/

// Direct invocation of the global functors with supplying parameters.
void testGlobalDirect()
{
  MvrGlobalFunctor functor(&function);
  MvrGlobalFunctor1<int> functor1(&function, 1);
  MvrGlobalFunctor2<bool, std::string> functor2(&function,
					       false, "default arg");

  printf("\n****** Testing global direct invocation\n");
  puts("> Should see function()...");
  functor.invoke();
  puts("> Should see function(5)...");
  functor1.invoke(5);
  puts("> Should see function(true, \"argument 1\")...");
  functor2.invoke(true, "argument 1");
}

// Invocation of a base MvrFunctor pointer to a global functor. Because the
// pointer is of type MvrFunctor, the parameters can not be supplied. The
// default parameters, which are supplied when the functor is constructed,
// are used.
void testGlobalBase()
{
  MvrFunctor *fptr;
  MvrGlobalFunctor functor(function);
  MvrGlobalFunctor1<int> functor1(function, 1);
  MvrGlobalFunctor2<bool, std::string> functor2(function, false,
						"default arg");

  printf("\n****** Testing global base invocation\n");
  fptr=&functor;
  puts("> Should see function()...");
  fptr->invoke();
  fptr=&functor1;
  puts("> Should see function(1)...");
  fptr->invoke();
  fptr=&functor2;
  puts("> Should see function(false, \"default arg\")...");
  fptr->invoke();
}

// Invocation of pointers which supply the parameter type. Full invocation
// with paramters is posesible in this fashion with out knowing the class
// that the functor refers to.
void testGlobalParams()
{
  MvrGlobalFunctor functor(function);
  MvrGlobalFunctor1<int> functor1(function, 1);
  MvrGlobalFunctor2<bool, std::string> functor2(function, false,
						"default arg");
  MvrFunctor *fptr;
  MvrFunctor1<int> *fptr1;
  MvrFunctor2<bool, std::string> *fptr2;

  printf("\n****** Testing global pointer invocation\n");
  fptr=&functor;
  puts("> Should see function()...");
  fptr->invoke();
  fptr1=&functor1;
  puts("> Should see function(2)...");
  fptr1->invoke(2);
  fptr2=&functor2;
  puts("> Should see function(true, \"argument 2\")...");
  fptr2->invoke(true, "argument 2");
}


/*
  Test global functors with return, MvrGlobalRetFunctor.
*/

// Direct invocation of the global functors with supplying parameters.
void testGlobalReturnDirect()
{
  MvrGlobalRetFunctor<bool> functor(&retFunction);
  MvrGlobalRetFunctor1<char*, int> functor1(&retFunction, 1);
  MvrGlobalRetFunctor2<double, bool, std::string>
    functor2(&retFunction, false, "default arg");
  bool bret;
  char *cret;
  double dret;

  printf("\n****** Testing global direct invocation with return\n");
  puts("> bool retFunction() should return true...");
  bret=functor.invokeR();
  printf("Returned: %d\n", bret);
  puts("> char* retFunction(5) should return \"Hello\"...");
  cret=functor1.invokeR(5);
  printf("Returned: %s\n", cret);
  puts("> double retFunction(true, \"argument 1\") should return 4.62...");
  dret=functor2.invokeR(true, "argument 1");
  printf("Returned: %e\n", dret);
}

// Invocation of a base MvrFunctor pointer to a global functor. Because the
// pointer is of type MvrFunctor, the parameters can not be supplied. The
// default parameters, which are supplied when the functor is constructed,
// are used.
void testGlobalReturnBase()
{
  MvrGlobalRetFunctor<bool> functor(retFunction);
  MvrGlobalRetFunctor1<char*, int> functor1(retFunction, 1);
  MvrGlobalRetFunctor2<double, bool, std::string>
    functor2(retFunction, false, "default arg");
  MvrRetFunctor<bool> *fBoolPtr;
  MvrRetFunctor<char*> *fCharPtr;
  MvrRetFunctor<double> *fDoublePtr;
  bool bret;
  char *cret;
  double dret;

  printf("\n****** Testing global base invocation with return\n");
  fBoolPtr=&functor;
  puts("> bool retFunction() should return true...");
  bret=fBoolPtr->invokeR();
  printf("Returned: %d\n", bret);
  fCharPtr=&functor1;
  puts("> char* retFunction(1) should return \"Hello\"...");
  cret=fCharPtr->invokeR();
  printf("Returned: %s\n", cret);
  fDoublePtr=&functor2;
  puts("> double retFunction(false, \"default arg\") should return 4.62...");
  dret=fDoublePtr->invokeR();
  printf("Returned: %e\n", dret);
}

// Invocation of pointers which supply the parameter type. Full invocation
// with paramters is posesible in this fashion with out knowing the class
// that the functor refers to.
void testGlobalReturnParams()
{
  MvrGlobalRetFunctor<bool> functor(retFunction);
  MvrGlobalRetFunctor1<char*, int> functor1(retFunction, 1);
  MvrGlobalRetFunctor2<double, bool, std::string>
    functor2(retFunction, false, "default arg");
  MvrRetFunctor<bool> *fBoolPtr;
  MvrRetFunctor1<char*, int> *fCharPtr;
  MvrRetFunctor2<double, bool, std::string> *fDoublePtr;
  bool bret;
  char *cret;
  double dret;

  printf("\n****** Testing global pointer invocation with return\n");
  fBoolPtr=&functor;
  puts("> bool retFunction() should return true...");
  bret=fBoolPtr->invokeR();
  printf("Returned: %d\n", bret);
  fCharPtr=&functor1;
  puts("> char* retFunction(7) should return \"Hello\"...");
  cret=fCharPtr->invokeR(7);
  printf("Returned: %s\n", cret);
  fDoublePtr=&functor2;
  puts("> double retFunction(false, \"argument 3\") should return 4.62...");
  dret=fDoublePtr->invokeR(false, "argument 3");
  printf("Returned: %e\n", dret);
}


// main(). Drives this example by creating an instance of the TestClass and
// instances of functors. Then the functors are invoked.
int main()
{
  testDirect();
  testBase();
  testParams();
  testReturnDirect();
  testReturnBase();
  testReturnParams();
  testGlobalDirect();
  testGlobalBase();
  testGlobalParams();
  testGlobalReturnDirect();
  testGlobalReturnBase();
  testGlobalReturnParams();

  MvrGlobalFunctor2<bool, std::string> f(&function);
  f.setP2("hello");
  f.invoke(true);

  return(0);
}
