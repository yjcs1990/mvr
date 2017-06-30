/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

#ifndef ARIA_wrapper_Functors_h
#define ARIA_wrapper_Functors_h

/* For Python, define MvrFunctor subclasses to hold Python C library
 * callable function objects.  These are used internally by the 
 * wrapper library, and typemaps convert target-language function 
 * objects to these MvrFunctor subclasses-- you only need to pass
 * the function to Mvria when in the C++ API you would pass a Functor.
 *
 * For Java, define subclasses of MvrFunctor for various argument types,
 * since you can't access template classes in Java.  Then you can 
 * further subclass within Java and pass that object to Mvria.
 */

#include "MvrFunctor.h"

/* Functors for Python: */

#ifdef SWIGPYTHON

class MvrPyFunctor : public virtual MvrFunctor 
{
protected:
  PyObject* pyFunction;
public:
  MvrPyFunctor(PyObject* _m) : pyFunction(_m) {
    Py_INCREF(pyFunction);
  }

  virtual void invoke() { 
    PyObject* r = PyObject_CallObject(pyFunction, NULL);
    if(!r) {
      fputs("** MvrPyFunctor: Error calling Python function: ", stderr);
      PyErr_Print();
    }
  }

  virtual ~ArPyFunctor() {
    Py_DECREF(pyFunction);
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};


/* Return type could be generalized if we find a way to convert any Python type
 * returned by the Python function to whatever the templatized return type is 
 * required by the C++ functor.  This _Bool version just checks boolean value of
 * return from python function.
 */
class MvrPyRetFunctor_Bool : 
  public virtual MvrRetFunctor<bool>,
  public virtual MvrPyFunctor
{
public:
  MvrPyRetFunctor_Bool(PyObject* _m) : MvrRetFunctor<bool>(), MvrPyFunctor(_m) {
  }

  virtual bool invokeR() {
    PyObject* r = PyObject_CallObject(pyFunction, NULL);  
    if(!r) {
      fputs("** MvrPyRetFunctor_Bool: Error calling Python function: ", stderr);
      PyErr_Print();
    }
    return(r == Py_True);
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};



class MvrPyFunctor1_String : 
  public virtual MvrFunctor1<const char*>,
  public virtual MvrPyFunctor
{
public:
  MvrPyFunctor1_String(PyObject* _m) : MvrFunctor1<const char*>(), MvrPyFunctor(_m) {
  }

  virtual void invoke(const char* arg) {
    if(!arg) {
      Py_FatalError("MvrPyFunctor1_String invoked with null argument!");
      // TODO invoke with "None" value
      return;
    }
    //printf("MvrPyFunctor1_String invoked with \"%s\"\n", arg);
    PyObject *s = PyString_FromString(arg);
    if(!s) {
      PyErr_Print();
      Py_FatalError("MvrPyFunctor1_String: Error converting argument to Python string value");
      return;
    }
    PyObject* r = PyObject_CallFunctionObjArgs(pyFunction, s, NULL);  
    if(!r) {
      fputs("** MvrPyFunctor1_String: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
    Py_DECREF(s);
  }

  virtual void invoke() {
    fputs("** MvrPyFunctor1_String: invoke: No argument supplied?", stderr);
    Py_FatalError("MvrPyFunctor1_String invoked with no arguments!");
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};

#ifdef ARIA_WRAPPER
class MvrPyPacketHandlerFunctor : 
  public virtual MvrRetFunctor1<bool, MvrRobotPacket*>,
  public virtual MvrPyFunctor
{
public:
  MvrPyPacketHandlerFunctor(PyObject* _m) : MvrRetFunctor1<bool, MvrRobotPacket*>(), MvrPyFunctor(_m) {}
  virtual bool invokeR(MvrRobotPacket* pkt) {
    if(!pkt) { 
      Py_FatalError("MvrPyPacketHandlerFunctor invoked with null argument!");
      return false;
    }
    PyObject *po = SWIG_NewPointerObj((void*)pkt, SWIGTYPE_p_ArRobotPacket, 0); //PyObject_FromPointer(arg);
    PyObject *r = PyObject_CallFunctionObjArgs(this->pyFunction, po, NULL);
    if(!r) {
      fputs("** MvrPyPacketHandlerFunctor: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
    Py_DECREF(po);
    return (r == Py_True);
  }
  virtual bool invokeR() {
    fputs("** MvrPyPacketHandlerFunctor: invokeR: No argument supplied", stderr);
    Py_FatalError("MvrPyPacketHandlerFunctor invoked with no arguments!");
    return false;
  }
  virtual void invoke() {
    fputs("** MvrPyPacketHandlerFunctor: invoke: No argument supplied?", stderr);
    Py_FatalError("MvrPyPacketHandlerFunctor invoked with no arguments!");
  }
  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};
#endif

// XXX TODO supply reference/pointer in constructor to Python library conversion function to convert to Python
// type (e.g. ...FromInt, FromLong, FromInt, etc.)
template <typename T1>
class MvrPyFunctor1 : 
  public virtual MvrFunctor1<T1>,
  public virtual MvrPyFunctor
{
public:
  MvrPyFunctor1(PyObject* pyfunc) : MvrFunctor1<T1>(), MvrPyFunctor(pyfunc) {
  }

  virtual void invoke(T1 arg) {
    puts("MvrPyFunctor1<> invoked");
    fflush(stdout);
    PyObject* r = PyObject_CallFunctionObjArgs(pyFunction, arg, NULL);  
    if(!r) {
      fputs("** MvrPyFunctor1: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
  }

  virtual void invoke() {
    fputs("** MvrPyFunctor1: invoke: No argument supplied?", stderr);
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};

template <typename T1, typename T2>
class MvrPyFunctor2 : 
  public virtual MvrFunctor2<T1, T2>,
  public virtual MvrPyFunctor
{
public:
  MvrPyFunctor2(PyObject* _m) : MvrFunctor2<T1, T2>(), MvrPyFunctor(_m) {
  }

  virtual void invoke(T1 arg1, T2 arg2) {
    PyObject* r = PyObject_CallFunctionObjArgs(pyFunction, arg1, arg2, NULL);  
    if(!r) {
      fputs("** MvrPyFunctor2: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
  }

  virtual void invoke() {
    fputs("** MvrPyFunctor2: invoke: No argument supplied?", stderr);
    Py_FatalError("MvrPyFunctor2 invoked with no arguments!");
  }

  virtual void invoke(T1 arg1) {
    fputs("** MvrPyFunctor2: invoke: No argument supplied?", stderr);
    Py_FatalError("MvrPyFunctor2 invoked with not enough arguments!");
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};

#endif // PYTHON




#endif // wrapperFunctors.h

