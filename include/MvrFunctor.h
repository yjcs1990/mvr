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
template<class P1, class P2, class P3, class P4>
class MvrFunctor4 : public MvrFunctor3<P1, P2, P3>
{
public:

    /// Destructor
    virtual ~MvrFunctor4() {}

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
  MvrGlobalFunctor(void (*func)(void)) :
   myFunc(func) {}
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
  MvrGlobalFunctor1(void (*func)(P1)) :
   myFunc(func), myP1() {}
  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor1(void (*func)(P1), P1 p1) :
   myFunc(func), myP1(p1) {}

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
  MvrGlobalFunctor2(void (*func)(P1, P2)) :
   myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor2(void (*func)(P1, P2), P1 p1) :
   myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalFunctor2(void (*func)(P1, P2), P1 p1, P2 p2) :
   myFunc(func), myP1(p1), myP2(p2) {}

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
  MvrGlobalFunctor3(void (*func)(P1, P2, P3)) :
   myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1) :
   myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1, P2 p2) :
   myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
   myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

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
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4)) :
   myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1) :
   myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
   myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
   myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

   /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) : 
   myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

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

#endif // Omitting MvrGlobalFunctor from SWIG

//
//
//
// MvrFunctors for global functions, C style function pointers with return
// return values.
//
//
//

#ifndef SWIG

/// Functor for a global function with return value
/**
   This is a class for global functions which return a value. This ties
   a C style function pointer into the functor class hierarchy as a
   convience. Code that has a reference to this class and treat it as
   an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret>
class MvrGlobalRetFunctor : public MvrRetFunctor<Ret>
{
public:
  /// Constructor
  MvrGlobalRetFunctor() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalRetFunctor(Ret (*func)(void)) : myFunc(func) {}

  /// Destructor
  virtual ~MvrGlobalRetFunctor() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (*myFunc)();}

protected:
  Ret (*myFunc)(void);
};

/// Functor for a global function with 1 parameter and return value
/**
   This is a class for global functions which take 1 parameter and return
   a value. This ties a C style function pointer into the functor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1>
class MvrGlobalRetFunctor1 : public MvrRetFunctor1<Ret, P1>
{
public:
  /// Constructor
  MvrGlobalRetFunctor1() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalRetFunctor1(Ret (*func)(P1)) : myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalRetFunctor1(Ret (*func)(P1), P1 p1) : myFunc(func), myP1(p1) {}

  /// Destructor
  virtual ~MvrGlobalRetFunctor1() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (*myFunc)(myP1);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (*myFunc)(p1);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

protected:
  Ret (*myFunc)(P1);
  P1 myP1;
};

/// Functor for a global function with 2 parameters and return value
/**
   This is a class for global functions which take 2 parameters and return
   a value. This ties a C style function pointer into the functor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2>
class MvrGlobalRetFunctor2 : public MvrRetFunctor2<Ret, P1, P2>
{
public:
  /// Constructor
  MvrGlobalRetFunctor2() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalRetFunctor2(Ret (*func)(P1, P2)) :
   myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalRetFunctor2(Ret (*func)(P1, P2), P1 p1) :
   myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalRetFunctor2(Ret (*func)(P1, P2), P1 p1, P2 p2) :
   myFunc(func), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~MvrGlobalRetFunctor2() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (*myFunc)(myP1, myP2);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (*myFunc)(p1, myP2);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (*myFunc)(p1, p2);}

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
  Ret (*myFunc)(P1, P2);
  P1 myP1;
  P2 myP2;
};

/// Functor for a global function with 2 parameters and return value
/**
   This is a class for global functions which take 2 parameters and return
   a value. This ties a C style function pointer into the functor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2, class P3>
class MvrGlobalRetFunctor3 : public MvrRetFunctor3<Ret, P1, P2, P3>
{
public:
  /// Constructor
  MvrGlobalRetFunctor3() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalRetFunctor3(Ret (*func)(P1, P2, P3)) :
   myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalRetFunctor3(Ret (*func)(P1, P2, P3), P1 p1) :
   myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalRetFunctor3(Ret (*func)(P1, P2, P3), P1 p1, P2 p2) :
   myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrGlobalRetFunctor3(Ret (*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
   myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~MvrGlobalRetFunctor3() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (*myFunc)(myP1, myP2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (*myFunc)(p1, myP2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (*myFunc)(p1, p2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) {return (*myFunc)(p1, p2, p3);}

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
  Ret (*myFunc)(P1, P2, P3);
  P1 myP1;
  P2 myP2;
  P3 myP3;
};

/// Functor for a global function with 4 parameters and return value
/**
   This is a class for global functions which take 4 parameters and return
   a value. This ties a C style function pointer into the functor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2, class P3, class P4>
class MvrGlobalRetFunctor4 : public MvrRetFunctor4<Ret, P1, P2, P3, P4>
{
public:
  /// Constructor
  MvrGlobalRetFunctor4() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4)) :
   myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1) :
   myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
   myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
 */
  MvrGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
   myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

	/// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
   myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~MvrGlobalRetFunctor4() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) {return (*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) {return (*myFunc)(p1, p2, p3, p4);}

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

  Ret (*myFunc)(P1, P2, P3, P4);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};

/// Functor for a global function with 4 parameters and return value
/**
   This is a class for global functions which take 4 parameters and return
   a value. This ties a C style function pointer into the functor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an MvrFunctor can use it like any other functor.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class P1, class P2, class P3, class P4, class P5>
class MvrGlobalRetFunctor5 : public MvrRetFunctor5<Ret, P1, P2, P3, P4, P5>
{
public:
  /// Constructor
  MvrGlobalRetFunctor5() {}

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  MvrGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  MvrGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
 */
  MvrGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

	/// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}

  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  MvrGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Destructor
  virtual ~MvrGlobalRetFunctor5() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) {return (*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) {return (*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {return (*myFunc)(p1, p2, p3, p4, p5);}

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
  Ret (*myFunc)(P1, P2, P3, P4, P5);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};

#endif // omitting MvrGlobalRetFunctor from SWIG


//
//
//
// MvrFunctors for member functions
//
//
//
/// Functor for a member function
/**
   This is a class for member functions. This class contains the knowledge
   on how to call a member function on a particular instance of a class.
   This class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class T>
class MvrFunctorC : public MvrFunctor
{
public:
  /// Constructor
  MvrFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctorC(T &obj, void (T::*func)(void)) : myObj(&obj), myFunc(func) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctorC(T *obj, void (T::*func)(void)) : myObj(obj), myFunc(func) {}

  /// Destructor
  virtual ~MvrFunctorC() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:
  T *myObj;
  void (T::*myFunc)(void);
};


/// Functor for a member function with 1 parameter
/**
   This is a class for member functions which take 1 parameter. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class T, class P1>
class MvrFunctor1C : public MvrFunctor1<P1>
{
public:

  /// Constructor
  MvrFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor1C(T &obj, void (T::*func)(P1)) :
   myObj(&obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor1C(T &obj, void (T::*func)(P1), P1 p1) :
   myObj(&obj), myFunc(func), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor1C(T *obj, void (T::*func)(P1)) :
   myObj(obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor1C(T *obj, void (T::*func)(P1), P1 p1) :
   myObj(obj), myFunc(func), myP1(p1) {}

  /// Destructor
  virtual ~MvrFunctor1C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

protected:
  T *myObj;
  void (T::*myFunc)(P1);
  P1 myP1;
};


/// Functor for a member function with 2 parameters
/**
   This is a class for member functions which take 2 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class T, class P1, class P2>
class MvrFunctor2C : public MvrFunctor2<P1, P2>
{
public:
  /// Constructor
  MvrFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor2C(T &obj, void (T::*func)(P1, P2)) :
   myObj(&obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor2C(T &obj, void (T::*func)(P1, P2), P1 p1) :
   myObj(&obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor2C(T &obj, void (T::*func)(P1, P2), P1 p1, P2 p2) :
   myObj(&obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor2C(T *obj, void (T::*func)(P1, P2)) :
   myObj(obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor2C(T *obj, void (T::*func)(P1, P2), P1 p1) :
   myObj(obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor2C(T *obj, void (T::*func)(P1, P2), P1 p1, P2 p2) :
   myObj(obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~MvrFunctor2C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  void (T::*myFunc)(P1, P2);
  P1 myP1;
  P2 myP2;
};

/// Functor for a member function with 3 parameters
/**
   This is a class for member functions which take 3 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class T, class P1, class P2, class P3>
class MvrFunctor3C : public MvrFunctor3<P1, P2, P3>
{
public:
  /// Constructor
  MvrFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor3C(T &obj, void (T::*func)(P1, P2, P3)) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor3C(T &obj, void (T::*func)(P1, P2, P3), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor3C(T &obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrFunctor3C(T &obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor3C(T *obj, void (T::*func)(P1, P2, P3)) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor3C(T *obj, void (T::*func)(P1, P2, P3), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor3C(T *obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3() {} 

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrFunctor3C(T *obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~MvrFunctor3C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  void (T::*myFunc)(P1, P2, P3);
  P1 myP1;
  P2 myP2;
  P3 myP3;
};

/// Functor for a member function with 4 parameters
/**
   This is a class for member functions which take 4 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class T, class P1, class P2, class P3, class P4>
class MvrFunctor4C : public MvrFunctor4<P1, P2, P3, P4>
{
public:
  /// Constructor
  MvrFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4)) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
      @param p4 default fourth parameter
 */
  MvrFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4)) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~MvrFunctor4C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) {(myObj->*myFunc)(p1, p2, p3, p4);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  void (T::*myFunc)(P1, P2, P3, P4);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};

/// Functor for a member function with 5 parameters
/**
   This is a class for member functions which take 5 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class T, class P1, class P2, class P3, class P4, class P5>
class MvrFunctor5C : public MvrFunctor5<P1, P2, P3, P4, P5>
{
public:
  /// Constructor
  MvrFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5)) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}
  
  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
MvrFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
  myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5)) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  MvrFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}
	
  /// Destructor
  virtual ~MvrFunctor5C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) {(myObj->*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {(myObj->*myFunc)(p1, p2, p3, p4, p5);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  void (T::*myFunc)(P1, P2, P3, P4, P5);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};

//
//
//
// MvrFunctors for member functions with return values
//
//
//
/// Functor for a member function with return value
/**
   This is a class for member functions which return a value. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class T>
class MvrRetFunctorC : public MvrRetFunctor<Ret>
{
public:
  /// Constructor
  MvrRetFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctorC(T &obj, Ret (T::*func)(void)) : myObj(&obj), myFunc(func) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctorC(T *obj, Ret (T::*func)(void)) : myObj(obj), myFunc(func) {}

  /// Destructor - supply function pointer
  virtual ~MvrRetFunctorC() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:
  T *myObj;
  Ret (T::*myFunc)(void);
};

/// Functor for a member function with return value and 1 parameter
/**
   This is a class for member functions which take 1 parameter and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a functor to another
   piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class T, class P1>
class MvrRetFunctor1C : public MvrRetFunctor1<Ret, P1>
{
public:
  /// Constructor
  MvrRetFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor1C(T &obj, Ret (T::*func)(P1)) :
   myObj(&obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor1C(T &obj, Ret (T::*func)(P1), P1 p1) :
   myObj(&obj), myFunc(func), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor1C(T *obj, Ret (T::*func)(P1)) :
   myObj(obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor1C(T *obj, Ret (T::*func)(P1), P1 p1) :
   myObj(obj), myFunc(func), myP1(p1) {}

  /// Destructor
  virtual ~MvrRetFunctor1C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

protected:
  T *myObj;
  Ret (T::*myFunc)(P1);
  P1 myP1;
};

/// Functor for a member function with return value and 2 parameters
/**
   This is a class for member functions which take 2 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a functor to another
   piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class T, class P1, class P2>
class MvrRetFunctor2C : public MvrRetFunctor2<Ret, P1, P2>
{
public:
  /// Constructor
  MvrRetFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor2C(T &obj, Ret (T::*func)(P1, P2)) :
    myObj(&obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor2C(T &obj, Ret (T::*func)(P1, P2), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor2C(T &obj, Ret (T::*func)(P1, P2), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor2C(T *obj, Ret (T::*func)(P1, P2)) :
    myObj(obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor2C(T *obj, Ret (T::*func)(P1, P2), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor2C(T *obj, Ret (T::*func)(P1, P2), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~MvrRetFunctor2C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  Ret (T::*myFunc)(P1, P2);
  P1 myP1;
  P2 myP2;
};

/// Functor for a member function with return value and 3 parameters
/**
   This is a class for member functions which take 3 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a functor to another
   piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class T, class P1, class P2, class P3>
class MvrRetFunctor3C : public MvrRetFunctor3<Ret, P1, P2, P3>
{
public:
  /// Constructor
  MvrRetFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3)) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3)) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~MvrRetFunctor3C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) 
    {return (myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj object to call function on
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj object to call function on
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  Ret (T::*myFunc)(P1, P2, P3);
  P1 myP1;
  P2 myP2;
  P3 myP3;
};

/// Functor for a member function with return value and 4 parameters
/**
   This is a class for member functions which take 4 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a functor to another
   piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class T, class P1, class P2, class P3, class P4>
class MvrRetFunctor4C : public MvrRetFunctor4<Ret, P1, P2, P3, P4>
{
public:

  /// Constructor
  MvrRetFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4)) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4)) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~MvrRetFunctor4C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) 
    {return (myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) 
    {return (myObj->*myFunc)(p1, p2, p3, p4);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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

  T *myObj;
  Ret (T::*myFunc)(P1, P2, P3, P4);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};



/// Functor for a member function with return value and 4 parameters
/**
   This is a class for member functions which take 4 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a functor to another
   piece of code.
   
   For an overall description of functors, see MvrFunctor.
*/
template<class Ret, class T, class P1, class P2, class P3, class P4, class P5>
class MvrRetFunctor5C : public MvrRetFunctor5<Ret, P1, P2, P3, P4, P5>
{
public:

  /// Constructor
  MvrRetFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5)) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  MvrRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  MvrRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5)) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  MvrRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  MvrRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Destructor
  virtual ~MvrRetFunctor5C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) 
    {return (myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) 
    {return (myObj->*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) 
    {return (myObj->*myFunc)(p1, p2, p3, p4, p5);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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

  T *myObj;
  Ret (T::*myFunc)(P1, P2, P3, P4, P5);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P4 myP5;
};

/// SWIG doesn't like the const functors
#ifndef SWIG

//
//
//
// MvrFunctors for const member functions
//
//
//
/// Functor for a const member function
/**
   This is a class for const member functions. This class contains the
   knowledge on how to call a const member function on a particular
   instance of a class.  This class should be instantiated by code
   that wishes to pass off a functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  

   @swigomit
*/
template<class T>
class MvrConstFunctorC : public MvrFunctor
{
public:
  /// Constructor
  MvrConstFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctorC(T &obj, void (T::*func)(void) const) : myObj(&obj), myFunc(func) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctorC(T *obj, void (T::*func)(void) const) : myObj(obj), myFunc(func) {}

  /// Destructor
  virtual ~MvrConstFunctorC() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:
  T *myObj;
  void (T::*myFunc)(void) const;
};

/// Functor for a const member function with 1 parameter
/**
   This is a class for const member functions which take 1
   parameter. This class contains the knowledge on how to call a const
   member function on a particular instance of a class. This class
   should be instantiated by code that wishes to pass off a functor to
   another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class T, class P1>
class MvrConstFunctor1C : public MvrFunctor1<P1>
{
public:
  /// Constructor
  MvrConstFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor1C(T &obj, void (T::*func)(P1) const) :
    myObj(&obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor1C(T &obj, void (T::*func)(P1) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor1C(T *obj, void (T::*func)(P1) const) :
    myObj(obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor1C(T *obj, void (T::*func)(P1) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1) {}

  /// Destructor
  virtual ~MvrConstFunctor1C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

protected:
  T *myObj;
  void (T::*myFunc)(P1) const;
  P1 myP1;
};

/// Functor for a const member function with 2 parameters
/**
   This is a class for const member functions which take 2
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class T, class P1, class P2>
class MvrConstFunctor2C : public MvrFunctor2<P1, P2>
{
public:
  /// Constructor
  MvrConstFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor2C(T &obj, void (T::*func)(P1, P2) const) :
    myObj(&obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor2C(T &obj, void (T::*func)(P1, P2) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor2C(T &obj, void (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor2C(T *obj, void (T::*func)(P1, P2) const) :
    myObj(obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor2C(T *obj, void (T::*func)(P1, P2) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor2C(T *obj, void (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~MvrConstFunctor2C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  void (T::*myFunc)(P1, P2) const;
  P1 myP1;
  P2 myP2;
};

/// Functor for a const member function with 3 parameters
/**
   This is a class for const member functions which take 3
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class T, class P1, class P2, class P3>
class MvrConstFunctor3C : public MvrFunctor3<P1, P2, P3>
{
public:
  /// Constructor
  MvrConstFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3() {} 

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~MvrConstFunctor3C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  void (T::*myFunc)(P1, P2, P3) const;
  P1 myP1;
  P2 myP2;
  P3 myP3;
};

/// Functor for a const member function with 4 parameters
/**
   This is a class for const member functions which take 4
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class T, class P1, class P2, class P3, class P4>
class MvrConstFunctor4C : public MvrFunctor4<P1, P2, P3, P4>
{
public:

  /// Constructor
  MvrConstFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4) const) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
      @param p4 default fourth parameter
 */
  MvrConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4) const) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

	
  /// Destructor
  virtual ~MvrConstFunctor4C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) {(myObj->*myFunc)(p1, p2, p3, p4);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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

  T *myObj;
  void (T::*myFunc)(P1, P2, P3, P4) const;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};

/// Functor for a const member function with 4 parameters
/**
   This is a class for const member functions which take 4
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class T, class P1, class P2, class P3, class P4, class P5>
class MvrConstFunctor5C : public MvrFunctor5<P1, P2, P3, P4, P5>
{
public:
  /// Constructor
  MvrConstFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5) const) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
      @param p4 default fourth parameter
 */
  MvrConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  MvrConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5) const) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  MvrConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

	
  /// Destructor
  virtual ~MvrConstFunctor5C() {}

  /// Invokes the functor
  virtual void invoke(void) {(myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) {(myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) {(myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) {(myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) {(myObj->*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the functor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {(myObj->*myFunc)(p1, p2, p3, p4, p5);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  void (T::*myFunc)(P1, P2, P3, P4, P5) const;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P4 myP5;
};


//
//
//
// MvrFunctors for const member functions with return values
//
//
//

/// Functor for a const member function with return value
/**
   This is a class for const member functions which return a
   value. This class contains the knowledge on how to call a const
   member function on a particular instance of a class. This class
   should be instantiated by code that wishes to pass off a functor to
   another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class Ret, class T>
class MvrConstRetFunctorC : public MvrRetFunctor<Ret>
{
public:
  /// Constructor
  MvrConstRetFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctorC(T &obj, Ret (T::*func)(void) const) : myObj(&obj), myFunc(func) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctorC(T *obj, Ret (T::*func)(void) const) : myObj(obj), myFunc(func) {}

  /// Destructor - supply function pointer
  virtual ~MvrConstRetFunctorC() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:
  T *myObj;
  Ret (T::*myFunc)(void) const;
};


/// Functor for a const member function with return value and 1 parameter
/**
   This is a class for const member functions which take 1 parameter
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class Ret, class T, class P1>
class MvrConstRetFunctor1C : public MvrRetFunctor1<Ret, P1>
{
public:
  /// Constructor
  MvrConstRetFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor1C(T &obj, Ret (T::*func)(P1) const) :
    myObj(&obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor1C(T &obj, Ret (T::*func)(P1) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor1C(T *obj, Ret (T::*func)(P1) const) :
    myObj(obj), myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor1C(T *obj, Ret (T::*func)(P1) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1) {}

  /// Destructor
  virtual ~MvrConstRetFunctor1C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  virtual void setP1(P1 p1) {myP1=p1;}

protected:
  T *myObj;
  Ret (T::*myFunc)(P1) const;
  P1 myP1;
};

/// Functor for a const member function with return value and 2 parameters
/**
   This is a class for const member functions which take 2 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class Ret, class T, class P1, class P2>
class MvrConstRetFunctor2C : public MvrRetFunctor2<Ret, P1, P2>
{
public:
  /// Constructor
  MvrConstRetFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor2C(T &obj, Ret (T::*func)(P1, P2) const) :
    myObj(&obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor2C(T &obj, Ret (T::*func)(P1, P2) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor2C(T &obj, Ret (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor2C(T *obj, Ret (T::*func)(P1, P2) const) :
    myObj(obj), myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor2C(T *obj, Ret (T::*func)(P1, P2) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor2C(T *obj, Ret (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~MvrConstRetFunctor2C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  Ret (T::*myFunc)(P1, P2) const;
  P1 myP1;
  P2 myP2;
};

/// Functor for a const member function with return value and 3 parameters
/**
   This is a class for const member functions which take 3 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class Ret, class T, class P1, class P2, class P3>
class MvrConstRetFunctor3C : public MvrRetFunctor3<Ret, P1, P2, P3>
{
public:
  /// Constructor
  MvrConstRetFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~MvrConstRetFunctor3C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) 
    {return (myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  Ret (T::*myFunc)(P1, P2, P3) const;
  P1 myP1;
  P2 myP2;
  P3 myP3;
};




// Start 4


/// Functor for a const member function with return value and 4 parameters
/**
   This is a class for const member functions which take 4 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class Ret, class T, class P1, class P2, class P3, class P4>
class MvrConstRetFunctor4C : public MvrRetFunctor4<Ret, P1, P2, P3, P4>
{
public:
  /// Constructor
  MvrConstRetFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~MvrConstRetFunctor4C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) 
    {return (myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) 
    {return (myObj->*myFunc)(p1, p2, p3, p4);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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
  T *myObj;
  Ret (T::*myFunc)(P1, P2, P3, P4) const;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};




/// Functor for a const member function with return value and 5 parameters
/**
   This is a class for const member functions which take 5 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   functor to another piece of code.
   
   For an overall description of functors, see MvrFunctor.  */
template<class Ret, class T, class P1, class P2, class P3, class P4, class P5>
class MvrConstRetFunctor5C : public MvrRetFunctor5<Ret, P1, P2, P3, P4, P5>
{
public:

  /// Constructor
  MvrConstRetFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const) :
    myObj(&obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  MvrConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  MvrConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myObj(&obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  MvrConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const) :
    myObj(obj), myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  MvrConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1) :
    myObj(obj), myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  MvrConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  MvrConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  MvrConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  MvrConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myObj(obj), myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Destructor
  virtual ~MvrConstRetFunctor5C() {}

  /// Invokes the functor with return value
  virtual Ret invokeR(void) {return (myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
  */
  virtual Ret invokeR(P1 p1) {return (myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2) {return (myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 second parameter
  */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3) 
    {return (myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) 
    {return (myObj->*myFunc)(p1, p2, p3, p4, myP5);}


  /// Invokes the functor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) 
    {return (myObj->*myFunc)(p1, p2, p3, p4, p5);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

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

  T *myObj;
  Ret (T::*myFunc)(P1, P2, P3, P4, P5) const;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};

#endif // omitting Const functors from SWIG

#endif  // MVRFUNCTOR_H