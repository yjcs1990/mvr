/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : MvrFunctor.h
 > Description  : An object which allows storing a generalized reference 
                  to a method with an object instance to call later (used for callback functions)
 > Author       : Yu Jie
 > Create Time  : 2017年05月10日
 > Modify Time  : 2017年05月10日
***************************************************************************************************/

#ifndef MVRFUNCTOR_H
#define MVRFUNCTOR_H

class MvrFunctor
{
public:
  /// Destructor
  virtual ~MvrFunctor() {}

  /// Invokes the functor
  virtual void invoke(void) = 0;

  /// Gets the name of the functor
  virtual const char *getName(void) { return myName.c_str();  }

  /// Sets the name of the functor
  virtual void setName(const char *name) { myName = name; }

#ifndef SWIG
  /// Sets the name of the functor with formatting
  /** @swigomit use setName() */
  virtual void setNameVar(const char *name, ...) 
    { 
      char arg[2048];
      va_list ptr;
      va_start(ptr, name);
      vsnprintf(arg, sizeof(arg), name, ptr);
      arg[sizeof(arg) - 1] = '\0';
      va_end(ptr);
      return setName(arg);
    }
#endif

protected:
  std::string myName;
};

/// Base class for functors with 1 parameter 
/**
   This is the base class for functors with 1 parameter. Code that has a
   reference to a functor that takes 1 parameter should use this class
   name. This allows the code to know how to invoke the functor without
   knowing which class the member function is in.

   For an overall description of functors, see MvrFunctor.
*/
template<class P1>
class MvrFunctor1 : public MvrFunctor
{
public:
  /// Destructor
  virtual ~MvrFunctor1() {}

  /// Invokes the functor
  virtual void invoke(void) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) = 0;
};

/// Base class for functors with 2 parameters
/**
   This is the base class for functors with 2 parameters. Code that has a
   reference to a functor that takes 2 parameters should use this class
   name. This allows the code to know how to invoke the functor without
   knowing which class the member function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1, class P2>
class MvrFunctor2 : public MvrFunctor1<P1>
{
public:
  /// Destructor
  virtual ~MvrFunctor2() {}

  /// Invokes the functor
  virtual void invoke(void) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) = 0;
};

/// Base class for functors with 3 parameters
/**
   This is the base class for functors with 3 parameters. Code that has a
   reference to a functor that takes 3 parameters should use this class
   name. This allows the code to know how to invoke the functor without
   knowing which class the member function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1, class P2, class P3>
class MvrFunctor3 : public MvrFunctor2<P1, P2>
{
public:
  /// Destructor
  virtual ~MvrFunctor3() {}

  /// Invokes the functor
  virtual void invoke(void) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) = 0;
};

/// Base class for functors with 4 parameters
/**
   This is the base class for functors with 4 parameters. Code that has a
   reference to a functor that takes 4 parameters should use this class
   name. This allows the code to know how to invoke the functor without
   knowing which class the member function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1, class P2, class P3, class P4, class P5>
class MvrFunctor5 : public MvrFunctor4<P1, P2, P3, P4>
{
public:
  /// Destructor
  virtual ~MvrFunctor5() {}

  /// Invokes the functor
  virtual void invoke(void) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) = 0;

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) = 0;
};

/// Base class for functors with a return value
/**
   This is the base class for functors with a return value. Code that has a
   reference to a functor that returns a value should use this class
   name. This allows the code to know how to invoke the functor without
   knowing which class the member function is in.
   
   For an overall description of functors, see MvrFunctor.     
*/
template<class Ret>
class MvrRetFunctor : public MvrFunctor
{
public:
  /// Destructor
  virtual ~MvrRetFunctor() {}

  /// Invokes the functor
  virtual void invoke(void) {invokeR();}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) = 0;
};

/// Base class for functors with a return value with 1 parameter
/**
   This is the base class for functors with a return value and take 1
   parameter. Code that has a reference to a functor that returns a value
   and takes 1 parameter should use this class name. This allows the code
   to know how to invoke the functor without knowing which class the member
   function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1>
class MvrRetFunctor1 : public MvrRetFunctor<Ret>
{
public:
  /// Destructor
  virtual ~MvrRetFunctor1() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) = 0;
};

/// Base class for functors with a return value with 2 parameters
/**
   This is the base class for functors with a return value and take 2
   parameters. Code that has a reference to a functor that returns a value
   and takes 2 parameters should use this class name. This allows the code
   to know how to invoke the functor without knowing which class the member
   function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2>
class MvrRetFunctor2 : public MvrRetFunctor1<Ret, P1>
{
public:
  /// Destructor
  virtual ~MvrRetFunctor2() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) = 0;
};

/// Base class for functors with a return value with 3 parameters
/**
   This is the base class for functors with a return value and take 3
   parameters. Code that has a reference to a functor that returns a value
   and takes 3 parameters should use this class name. This allows the code
   to know how to invoke the functor without knowing which class the member
   function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2, class P3>
class MvrRetFunctor3 : public MvrRetFunctor2<Ret, P1, P2>
{
public:
  /// Destructor
  virtual ~MvrRetFunctor3() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) = 0;
};

/// Base class for functors with a return value with 4 parameters
/**
   This is the base class for functors with a return value and take 4
   parameters. Code that has a reference to a functor that returns a value
   and takes 4 parameters should use this class name. This allows the code
   to know how to invoke the functor without knowing which class the member
   function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2, class P3, class P4>
class MvrRetFunctor4 : public MvrRetFunctor3<Ret, P1, P2, P3>
{
public:
  /// Destructor
  virtual ~MvrRetFunctor4() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) = 0;
};

/// Base class for functors with a return value with 5 parameters
/**
   This is the base class for functors with a return value and take 5
   parameters. Code that has a reference to a functor that returns a value
   and takes 5 parameters should use this class name. This allows the code
   to know how to invoke the functor without knowing which class the member
   function is in.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2, class P3, class P4, class P5>
class MvrRetFunctor5 : public MvrRetFunctor4<Ret, P1, P2, P3, P4>
{
public:
  /// Destructor
  virtual ~MvrRetFunctor5() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) = 0;

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) = 0;
};

//
//
//
// MvrFunctors for global functions. C style function pointers.
//
//
//

#ifndef SWIG
/// Functor for a global function with no parameters
/**
   This is a class for global functions. This ties a C style function
   pointer into the functor class hierarchy as a convience. Code that
   has a reference to this class and treat it as an MvrFunctor can use
   it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
class MvrGlobalFunctor : public MvrFunctor
{
public:
  /// Constructor
  MvrGlobalFunctor() {}
  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalFunctor(void (*func)(void)) : myFunc(func) {}
  /// Destructor
  virtual ~MvrGlobalFunctor() {}

  /// Invokes the functor
  virtual void invoke(void) {(*myFunc)();}
protected:
  void (*myFunc)(void);
};

/// Functor for a global function with 1 parameter
/**
   This is a class for global functions which take 1 parameter. This ties
   a C style function pointer into the functor class hierarchy as a
   convience. Code that has a reference to this class and treat it as
   an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1>
class MvrGlobalFunctor1 : public MvrFunctor1<P1>
{
public:
  /// Constructor
  MvrGlobalFunctor1() {}
  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalFunctor1(void (*func)(P1)) : myFunc(func), myP1() {}
  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor1(void (*func)(P1), P1 p1) : myFunc(func), myP1(p1) {}

  /// Destructor
  virtual ~MvrGlobalFunctor1() {}

  /// Invokes the functor
  virtual void invoke(void) {(*myFunc)(myP1);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(*myFunc)(p1);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

protected:
  void (*myFunc)(P1);
  P1 myP1;
};

/// Functor for a global function with 2 parameters
/**
   This is a class for global functions which take 2 parameters. This ties
   a C style function pointer into the functor class hierarchy as a
   convience. Code that has a reference to this class and treat it as
   an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1, class P2>
class MvrGlobalFunctor2 : public MvrFunctor2<P1, P2>
{
public:
  /// Constructor
  MvrGlobalFunctor2() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalFunctor2(void (*func)(P1, P2)) : myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor2(void (*func)(P1, P2), P1 p1) : myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalFunctor2(void (*func)(P1, P2), P1 p1, P2 p2) : myFunc(func), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~MvrGlobalFunctor2() {}

  /// Invokes the functor
  virtual void invoke(void) {(*myFunc)(myP1, myP2);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(*myFunc)(p1, myP2);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(*myFunc)(p1, p2);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  virtual void setP2(P2 p2) {myP2=p2;}

protected:
  void (*myFunc)(P1, P2);
  P1 myP1;
  P2 myP2;
};


/// Functor for a global function with 3 parameters
/**
   This is a class for global functions which take 3 parameters. This ties
   a C style function pointer into the functor class hierarchy as a
   convience. Code that has a reference to this class and treat it as
   an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1, class P2, class P3>
class MvrGlobalFunctor3 : public MvrFunctor3<P1, P2, P3>
{
public:
  /// Constructor
  MvrGlobalFunctor3() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalFunctor3(void (*func)(P1, P2, P3)) : myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1) : myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1, P2 p2) : myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) : myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~MvrGlobalFunctor3() {}

  /// Invokes the functor
  virtual void invoke(void) {(*myFunc)(myP1, myP2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(*myFunc)(p1, myP2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(*myFunc)(p1, p2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(*myFunc)(p1, p2, p3);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  virtual void setP3(P3 p3) {myP3=p3;}

protected:

  void (*myFunc)(P1, P2, P3);
  P1 myP1;
  P2 myP2;
  P3 myP3;
};

/// Functor for a global function with 4 parameters
/**
   This is a class for global functions which take 4 parameters. This ties
   a C style function pointer into the functor class hierarchy as a
   convience. Code that has a reference to this class and treat it as
   an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1, class P2, class P3, class P4>
class MvrGlobalFunctor4 : public MvrFunctor4<P1, P2, P3, P4>
{
public:
  /// Constructor
  MvrGlobalFunctor4() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4)) : myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1) : myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2) : myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) : myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

   /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) : myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~MvrGlobalFunctor4() {}

  /// Invokes the functor
  virtual void invoke(void) {(*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) {(*myFunc)(p1, p2, p3, p4);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  virtual void setP4(P4 p4) {myP4=p4;}

protected:
  void (*myFunc)(P1, P2, P3, P4);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};

/// Functor for a global function with 5 parameters
/**
   This is a class for global functions which take 5 parameters. This ties
   a C style function pointer into the functor class hierarchy as a
   convience. Code that has a reference to this class and treat it as
   an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class P1, class P2, class P3, class P4, class P5>
class MvrGlobalFunctor5 : public MvrFunctor5<P1, P2, P3, P4, P5>
{
public:
  /// Constructor
  MvrGlobalFunctor5() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

   /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}

   /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  MvrGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Destructor
  virtual ~MvrGlobalFunctor5() {}

  /// Invokes the functor
  virtual void invoke(void) {(*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) {(*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {(*myFunc)(p1, p2, p3, p4, p5);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  virtual void setP4(P4 p4) {myP4=p4;}

  /// Set the default fifth parameter
  /**
     @param p5 default fifth parameter
  */
  virtual void setP5(P5 p5) {myP5=p5;}

protected:

  void (*myFunc)(P1, P2, P3, P4, P5);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};

#endif // Omitting MvrGlobalFunctor from Swig


#endif  // MVRFUNCTOR_H