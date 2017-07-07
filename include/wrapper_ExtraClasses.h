#ifndef MVRIA_wrapper_ExtraClasses_h
#define MVRIA_wrapper_ExtraClasses_h

/** @cond INCLUDE_SWIG_ONLY_CLASSES */

/* MvrConfigArg subclasses for specific types, since for some target languages
 * (Python) Swig can't differentiate booleans, integers, short integers,
 * unsigned integers, etc.  Furthermore, MvrConfig can't change program vmvriables
 * via pointers in most languages, so you need to only use the constructors that take
 * an initial value for an internally held vmvriable instead of pointers anyway.
 */

class MvrConfigArg_Bool : public MvrConfigArg
{
public:
  MvrConfigArg_Bool(const char *name, bool b, const char *desc = "") :
    MvrConfigArg(name, b, desc)
  { }
};

class MvrConfigArg_Int : public MvrConfigArg
{
public:
  MvrConfigArg_Int(const char *name, int i, const char *desc = "", int min = INT_MIN, int max = INT_MAX) :
    MvrConfigArg(name, i, desc, min, max)
  { }
};

class MvrConfigArg_Double : public MvrConfigArg
{
public:
  MvrConfigArg_Double(const char *name, double d, const char *desc = "", double min = -HUGE_VAL, double max = HUGE_VAL) :
    MvrConfigArg(name, d, desc, min, max)
  { }
};

class MvrConfigArg_String : public MvrConfigArg
{
public:
  MvrConfigArg_String(const char *name, char *str, const char *desc) :
    MvrConfigArg(name, str, desc, 0)
  { 
  }
};

/** @endcond INCLUDE_SWIG_ONLY_CLASSES */

#endif // wrapperExtraClasses.h
