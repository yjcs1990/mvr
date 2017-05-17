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

#endif  // MVRFUNCTOR_H