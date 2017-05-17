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
 *  This is the base class for functors with 1 parameter. Code that has a
 *  reference to a functor that takes 1 parameter should use this class
 *  name. This allows the code to know how to invoke the functor without
 *  knowing which class the member function is in.

 *  For an overall description of functors, see MvrFunctor.
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


#endif  // MVRFUNCTOR_H