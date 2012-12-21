/*
 *  pynestkernel.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Interface between Python and the NEST simulation tool. 
 *  www.nest-initiative.org
 *
 *  Developed in the context of the EU FACETS project.
 *  facets.kip.uni-heidelberg.de
 *
 *  Authors:
 *  Marc-Oliver Gewaltig
 *  Eilif Muller
 *  Moritz Helias
 *  Jochen Martin Eppler
 *
 */
#include <Python.h>

// If we're not using python 2.5
#if (PY_VERSION_HEX < 0x02050000)
typedef int Py_ssize_t;
#endif

// Needed for Python C extensions using NumPy, see comment in
// datumtopythonconverter.h
#define PY_ARRAY_UNIQUE_SYMBOL _pynest_arrayu

#ifdef HAVE_NUMPY
#include <numpy/arrayobject.h>
#endif


#include "interpret.h"
#include "network.h"
#include "communicator.h"
#include "random_numbers.h"
#include "slistartup.h"
#include "sliarray.h"
#include "processes.h"
#include "nestmodule.h"
#include "dynamicloader.h"
#include "oosupport.h"
#include "processes.h"
#include "sliregexp.h"
#include "specialfunctionsmodule.h"
#include "sligraphics.h"
#include "compose.hpp"
#include "filesystem.h"

#include "pynestpycsa.h"
#include "../conngen/conngenmodule.h"

#ifdef HAVE_CSA
#include "csa_glue.h"
#endif

#include "doubledatum.h"
#include "integerdatum.h"
#include "dictdatum.h"
#include "dictutils.h"
#include "arraydatum.h"
#include "booldatum.h"
#include "stringdatum.h"
#include "connectiondatum.h"
#include "datumtopythonconverter.h"
#include "pydatum.h"

#include <algorithm>

#include "spikecounter.h"

#include "static_modules.h"

/*
The following instance of nest::spikecounter needs to be defined
under MacOS X to prevent the linker from throwing away the
constructor of spikecounter during linking.
See also bub #301.
MH, 2009/01/07
*/
#ifdef __APPLE__
//http://developer.apple.com/technotes/tn2002/tn2071.html#Section10
nest::spikecounter pseudo_spikecounter_instance(0.0,0.0);
#endif

// global definitions
static SLIInterpreter *pEngine = NULL;
static nest::Network *pNet = NULL;
static PyObject *NESTError = NULL;

/**
 * This function converts PyObjects to nest::Datums. It can be
 * called recursively to covert PyObjs of PyObjs of PyObjs...
 * to Datums of Datums of Datums...
 */
extern "C"
{
Datum* PyObj_ToDatum(PyObject *pObj)
{
  if (PyInt_Check(pObj)) { // object is integer or bool
    if (pObj==Py_True)
      return new BoolDatum(true);
    else if (pObj==Py_False)
      return new BoolDatum(false);
    else
      return new IntegerDatum(PyInt_AsLong(pObj));
  }

  if (PyFloat_Check(pObj)) // object is float
    return new DoubleDatum(PyFloat_AsDouble(pObj));

  if (PyString_Check(pObj)) // object is string
    return new StringDatum(PyString_AsString(pObj));

#ifdef HAVE_NUMPY
  if (PyArray_CheckScalar(pObj)) { // handle numpy array scalars

    PyArray_Descr *typecode;
    typecode = PyArray_DescrFromScalar(pObj);

    switch (typecode->type_num)
    {
      case NPY_INT:
      {
        int val;
        PyArray_ScalarAsCtype(pObj, &val);
        return new IntegerDatum(val);
      }
      case NPY_LONG:
      {
        long val;
        PyArray_ScalarAsCtype(pObj, &val);
        return new IntegerDatum(val);
      }
      case NPY_DOUBLE:
      {
        double val;
        PyArray_ScalarAsCtype(pObj, &val);
        return new DoubleDatum(val);
      }
      case NPY_OBJECT: // handle 0-dim numpy array, which are treated as scalars
      {
        PyArrayObject *array = 0;
        array = (PyArrayObject*) pObj;
        assert(array != 0);
        return PyObj_ToDatum(PyArray_ToScalar(array->data, pObj));
      }
      default:
      {
        std::string error = String::compose("Unsupported Numpy array scalar type: '%1'.\n"
                                            "If you think this is an error, tell us at nest_user@nest-initiative.org",
                                            typecode->type_num);
        PyErr_SetString(NESTError, error.c_str());
        return NULL;
      }
    }
  }

  if (PyArray_Check(pObj)) { // handle numpy arrays by sending as VectorDatum
    
    PyArrayObject *array = 0;
    array = (PyArrayObject*) pObj;
    assert(array != 0);

    Datum* vd = 0;
    int size = PyArray_Size(pObj);

    // raise an exception if array's dimensionality is not 1
    if (array->nd != 1)
    {
      std::string error = String::compose("The given Numpy array has an unsupported dimensionality of %1.\n"
                                          "Only one-dimensional arrays are supported. "
                                          "If you think this is an error,\ntell us at nest_user@nest-initiative.org",
                                          array->nd);
      PyErr_SetString(NESTError, error.c_str());
      return NULL;
    }

    switch (array->descr->type_num)
    {
      case NPY_INT :
      case NPY_LONG:
      {
        long *begin = reinterpret_cast<long*>(array->data);
        std::vector<long>* datavec;

        // Check if we really have a pure 1-dim array instead of a selection like obtained with a[:,1]
        // The latter needs to be treated different because we have to move in steps of array->strides[0]
        if (array->strides[0] == sizeof(long))
          datavec = new std::vector<long>(begin, begin + size);
        else
        {
          datavec = new std::vector<long>(size);
          for (int i = 0; i < size; i++)
            (*datavec)[i] = *(long*)(array->data + i*array->strides[0]);
        }

        vd = new IntVectorDatum(datavec);
        break;
      }
      case NPY_DOUBLE:
      {
        double *begin = reinterpret_cast<double*>(array->data);
        std::vector<double>* datavec;
        
        // Check if we really have a pure 1-dim array instead of a selection like obtained with a[:,1]
        // The latter needs to be treated different because we have to move in steps of array->strides[0]
        if (array->strides[0] == sizeof(double))
          datavec = new std::vector<double>(begin, begin + size);
        else
        {
          datavec = new std::vector<double>(size);
          for (int i = 0; i < size; i++)
            (*datavec)[i] = *(double*)(array->data + i*array->strides[0]);
        }

        vd = new DoubleVectorDatum(datavec);
        break;
      }
      default:
      {
        std::string error = String::compose("Unsupported Numpy array type: '%1'.\n"
                                            "If you think this is an error, tell us at nest_user@nest-initiative.org",
                                            array->descr->type_num);
        PyErr_SetString(NESTError, error.c_str());
        return NULL;
      }
    }
    
    return vd;
  }
#endif

  if (PyList_Check(pObj) || PyTuple_Check(pObj)) { // object is a list or a tuple
    ArrayDatum *d = new ArrayDatum();
    PyObject* subPyObj;

    size_t size = (PyList_Check(pObj)) ? PyList_Size(pObj) : PyTuple_Size(pObj);
    d->reserve(size);
 
    for (size_t i = 0; i < size; ++i) {
      subPyObj = (PyList_Check(pObj)) ? PyList_GetItem(pObj, i) : PyTuple_GetItem(pObj, i);
      d->push_back(PyObj_ToDatum(subPyObj));
      if (PyErr_Occurred()) {
        delete d;
        return NULL;
      }
    }
    return d;
  }

  if (PyDict_Check(pObj)) { // object is a dictionary
    DictionaryDatum *d = new DictionaryDatum(new Dictionary);
    PyObject* subPyObj;
    PyObject *key;
    Py_ssize_t pos = 0;

    while (PyDict_Next(pObj, &pos, &key, &subPyObj)) {
      Token t(PyObj_ToDatum(subPyObj));
      if (PyErr_Occurred()) {
        delete d;
        return NULL;
      }
      if (!PyString_Check(key)) { // insert with a bogus key
        PyErr_Warn(PyExc_Warning, "Non-string key in Python dictionary. Using bogus key in  dictionary.");
        PyObject* keystr = PyObject_Str(key);
        if (keystr == NULL)
          (*d)->insert("BOGUS_KEY", t);
        else
          (*d)->insert(PyString_AsString(pObj), t);
      }
      else
	(*d)->insert(PyString_AsString(key), t);
    }
    return d;
  }

  if (PyDatum_Check(pObj)) { // Object is encapsulated Datum
    Datum* d = PyDatum_GetDatum(reinterpret_cast<PyDatum*>(pObj));
    d->addReference();
    return d;
  }

  if (PyPyCSA_Check(pObj)) { // object is a PyCSA object
    ConnectionGenerator *cg = new PyCSAGenerator(pObj);
    nest::ConnectionGeneratorDatum *d = new nest::ConnectionGeneratorDatum(cg);
    return d;
  }

#ifdef HAVE_CSA
  if (PyCSA_Check(pObj)) { // object is a CSA object
    ConnectionGenerator *cg = PyCSA_GetObject(pObj);
    nest::ConnectionGeneratorDatum *d = new nest::ConnectionGeneratorDatum(cg);
    return d;
  }
#endif

  std::string error = String::compose("Python object of type '%1' cannot be converted to SLI.\n"
                                      "If you think this is an error, tell us at nest_user@nest-initiative.org",
                                      pObj->ob_type->tp_name);
  PyErr_SetString(NESTError, error.c_str());
  return 0;

}

}
/**
 * Execute a SLI command, given as string. 
 */
static PyObject *runsli(PyObject *, PyObject *args)
{
  char *pSliCommand; // we don't need to delete this. Python does it. (mog, 13.7.07)

  if (pEngine==NULL) {
    PyErr_SetString(NESTError, "runsli(): PyNEST engine not initialized properly or finalized already.");
    return NULL;
  }

  if (!PyArg_ParseTuple(args,"s",&pSliCommand)) {
    PyErr_SetString(NESTError, "runsli(): Error parsing args.");
    return NULL;
  }

  // copy python string into sliCommand here
  std::string sliCommand;
  sliCommand.assign(pSliCommand);

  // send string to sli interpreter
  Py_BEGIN_ALLOW_THREADS;
  pEngine->execute(sliCommand);
  Py_END_ALLOW_THREADS;

  if (PyErr_Occurred ())
    return NULL;

  return Py_BuildValue("");
}


/**
 * Return the top object of SLI's stack as PyObject. 
 */
static PyObject *popsli(PyObject *, PyObject *)
{
  if (pEngine==NULL) {
    PyErr_SetString(NESTError, "popsli(): PyNEST engine not initialized properly or finalized already.");
    return NULL;
  }

  if (pEngine->OStack.empty()) {
    PyErr_SetString(NESTError, "popsli(): SLI stack is empty.");
    return NULL;
  }

  Token &t = pEngine->OStack.top();
  DatumToPythonConverter DatumToPyObj;
  PyObject *pObj = 0;

  try {
    pObj = DatumToPyObj.convert(*t); // operator* returns reference to Datum
  }
  catch(TypeMismatch e)
  {
    PyErr_SetString(NESTError, "NEST object cannot be converted to python object.");
  }
  pEngine->OStack.pop();

  // N because the object is new (DatumToPyObj returns a new reference)
  return Py_BuildValue("N",pObj);
}


/**
 * Push a PyObject onto SLI's stack. 
 */
static PyObject *pushsli(PyObject *, PyObject *args)
{
  PyObject *pObj;

  if (pEngine==NULL) {
    PyErr_SetString(NESTError, "pushsli(): PyNEST engine not initialized properly or finalized already.");
    return NULL;
  }

  if (!PyArg_ParseTuple(args,"O", &pObj)) {
    PyErr_SetString(NESTError, "pushsli(): Error parsing args.");
    return NULL;
  }
  
  Datum *pdat = PyObj_ToDatum(pObj);
  if (pdat != 0) {
    pEngine->OStack.push(pdat);
    return Py_BuildValue("");
  }
  else
    return NULL;
}


/**
 * Redirect std::cout to a file. The name of the file is given as argument
 */
static PyObject *logstdout(PyObject *, PyObject *args)
{
  PyObject *pObj;
  if (!PyArg_ParseTuple(args, "O", &pObj))
  {
    PyErr_SetString(NESTError, "logstdout(): Error parsing args.");
    return NULL;
  }

  if (!PyString_Check(pObj))
  {
    PyErr_SetString(NESTError, "logstdout(): Error parsing args.");
    return NULL;
  }
  
  const char* filename = PyString_AsString(pObj);
  std::ofstream* os = new std::ofstream(filename);
  std::cout.rdbuf(os->rdbuf());

  return Py_BuildValue("");
}

/**
 * Push a list of dictionaries to the SLI stack as ConnectionDatum objects.
 * This is only a helper function for GetStatus and SetStatus.
 */
static PyObject *push_connection_datums(PyObject *, PyObject *args)
{
    PyObject *pObj;
    if (!PyArg_ParseTuple(args, "O", &pObj))
    {
	PyErr_SetString(NESTError, "push_connection_datums(): Error parsing args.");
	return NULL;
    }
    
    if (!PyList_Check(pObj) && !PyTuple_Check(pObj))
    {
	PyErr_SetString(NESTError, "push_connection_datums(): Argument must be a list of dictionaries or a list of lists/arrays with 5 elements.");
	return NULL;
    }
    
    ArrayDatum connectome;
    size_t size = (PyList_Check(pObj)) ? PyList_Size(pObj) : PyTuple_Size(pObj);
    connectome.reserve(size);
    
    for (size_t i = 0; i < size; ++i)
    {
	PyObject* subPyObj = (PyList_Check(pObj)) ? PyList_GetItem(pObj, i) : PyTuple_GetItem(pObj, i);
	
	if (PyDict_Check(subPyObj))
	{
	    PyObject* subsubPyObj;
	    long source;
	    long target_thread;
	    long synapse_modelid;
	    long port;
	    
	    subsubPyObj = PyDict_GetItemString(subPyObj, nest::names::source.toString().c_str());
	    if (subsubPyObj != NULL && PyInt_Check(subsubPyObj))
		source = PyInt_AsLong(subsubPyObj);
	    else
	    {
		PyErr_SetString(NESTError, 
				"push_connection_datums(): No source entry in dictionary.");
		return NULL;
	    }
	    
	    subsubPyObj = PyDict_GetItemString(subPyObj, 
					       nest::names::target_thread.toString().c_str());
	    if (subsubPyObj != NULL && PyInt_Check(subsubPyObj))
		target_thread = PyInt_AsLong(subsubPyObj);
	    else
	    {
		PyErr_SetString(NESTError, 
				"push_connection_datums(): No target_thread entry in dictionary.");
		return NULL;
	    }
	    
	    subsubPyObj = PyDict_GetItemString(subPyObj, 
					       nest::names::synapse_modelid.toString().c_str());
	    if (subsubPyObj != NULL && PyInt_Check(subsubPyObj))
		synapse_modelid = PyInt_AsLong(subsubPyObj);
	    else
	    {
		PyErr_SetString(NESTError, 
				"push_connection_datums(): No synapse_modelid entry in dictionary.");
		return NULL;
	    }
	    
	    subsubPyObj = PyDict_GetItemString(subPyObj, nest::names::port.toString().c_str());
	    if (subsubPyObj != NULL && PyInt_Check(subsubPyObj))
		port = PyInt_AsLong(subsubPyObj);
	    else
	    {
		PyErr_SetString(NESTError, 
				"push_connection_datums(): No port entry in dictionary.");
		return NULL;
	    }
	    
	    ConnectionDatum cd = ConnectionDatum(nest::ConnectionID(source, target_thread, 
								    synapse_modelid, port));
	    connectome.push_back(cd);
	    continue;
	} 
#ifdef HAVE_NUMPY
	else if (PyArray_Check(subPyObj) )
	{
	    size_t array_size = PyArray_Size(subPyObj);
	    if(array_size!=5)
	    {
		std::string error = String::compose("push_connection_arrays(): At position %1 in connection ID list.",i)+
		    "\n Connection ID must have exactly five entries.";
		PyErr_SetString(NESTError, error.c_str());
		return 0;
	    }
	    PyArrayObject *array = (PyArrayObject*) subPyObj;
	    assert(array != 0);
	    switch (array->descr->type_num)
	    {
	    case NPY_INT :
	    {
		PyArrayIterObject *iter= (PyArrayIterObject *)PyArray_IterNew(subPyObj);
		assert(iter != 0);

		int con[5];
		while(iter->index < iter->size)
		{
		    con[iter->index]= *(int*)(iter->dataptr);
		    PyArray_ITER_NEXT(iter);
		}
		delete iter ;
		connectome.push_back(new ConnectionDatum(nest::ConnectionID(con[0],con[1],con[2], 
									    con[3], con[4])));
		continue;
	    }
	    case NPY_LONG:
	    {
		PyArrayIterObject *iter= (PyArrayIterObject *)PyArray_IterNew(subPyObj);
		assert(iter != 0);

		long con[5];
		while(iter->index < iter->size)
		{
		    con[iter->index]= *(long*)(iter->dataptr);
		    PyArray_ITER_NEXT(iter);
		}
		delete iter ;
		connectome.push_back(new ConnectionDatum(nest::ConnectionID(con[0],con[1],con[2], 
									    con[3], con[4])));
		continue;
	    }
	    default:
		std::string error = String::compose("push_connection_arrays(): At position %1 in connection ID list.",i)+
		    "\n Connection ID must be a list or numpy array of five integers.";
		PyErr_SetString(NESTError, error.c_str());
		return 0;
	    }
	}
#endif
	else if (PyList_Check(subPyObj))
	{
	    size_t tuple_size = PyList_Size(subPyObj);
	    //std::cerr << tuple_size << std::endl;
	    if (tuple_size !=5)
	    {
		std::string error = String::compose("push_connection_arrays(): At position %1 in connection ID list.",i)+
		    "\n Connection ID must have exactly five entries.";
		PyErr_SetString(NESTError, error.c_str());
		continue;
	    }
	    long con[5];
	    for (long j=0; j<5; ++j)
	    {
		PyObject* itemPyObj = PyList_GetItem(subPyObj, j);
		if(PyInt_Check(itemPyObj))
		{
		    con[j]=PyInt_AsLong(itemPyObj);
		}
#ifdef HAVE_NUMPY
		else if (PyArray_CheckScalar(itemPyObj)) // handle numpy array scalars
		{
		    PyArray_Descr *typecode;
		    typecode = PyArray_DescrFromScalar(itemPyObj);
		    switch (typecode->type_num)
		    {
		    case NPY_INT:
		    case NPY_LONG:
			long val;
			PyArray_ScalarAsCtype(itemPyObj, &val);
			con[j]=val;
			break;
		    default:
			std::string error = String::compose("push_connection_arrays(): At position %1: Unsupported Numpy array scalar type: '%2'.\n",
							    i,typecode->type_num);
			PyErr_SetString(NESTError, error.c_str());
			return NULL;
		    }
		}
#endif
		else
		{
		    std::string error = String::compose("push_connection_arrays(): At position %1, %2 in connection ID list."
							"\n Connection ID must be a list, tuple, or and array of five integers.",i,j);
		    PyErr_SetString(NESTError, error.c_str());
		    return 0;
		}
	    }
	    connectome.push_back(new ConnectionDatum(nest::ConnectionID(con[0],con[1],con[2], con[3], con[4])));
	    continue;
	}
	else {
	    std::string error = String::compose("push_connection_arrays(): At position %1 in connection ID list.",i)+
		"\n Connection ID must be a list, tuple, or array of five integers.";
	    PyErr_SetString(NESTError, error.c_str());
	    return 0;
	}
    }
    
    pEngine->OStack.push(connectome);
    return Py_BuildValue("");
}


/**
 * Converts a python list of strings into the plain old C style array
 * of null terminated strings. The string pointers MUST NOT be freed
 * afterwards, since they point to existing python objects.
 * \returns 0 on error, pointer to char** buffer on success bool
 */
bool stringlist_py2c(PyObject *stringlist, int &argc, char** &argv)
{
  argv = 0;

  if (PyList_Check(stringlist)) { // object is a list
    argc = PyList_Size(stringlist);      
    argv = new char* [argc + 1];
    assert (argv != 0);
    static char* arg0 = "pynest\0";
    argv[0] = arg0;

    for (int i = 1; i < argc + 1; i++)
    {
      PyObject* subPyObj = PyList_GetItem(stringlist, i - 1);
      if (PyString_Check(subPyObj)) // a string
        argv[i] = PyString_AsString(subPyObj); // returns pointer to internal string representation
                                               // so DO NOT DEALLOCATE OR MODIFY
      else // not a string
      {
        PyErr_SetString(NESTError, "stringlist_py2c(): List doesn't contain strings.");
        return false;
      }
    }
  }
  else // not a list
  {
    PyErr_SetString(NESTError, "stringlist_py2c(): Stringlist expected to be list of strings.");
    return false;
  }
  
  return true;
}


/**
 * Startup function
 */
int pyneststartup(int argc, char**argv, SLIInterpreter &engine, nest::Network* &pNet, std::string path)
{
  addmodule<SLIArrayModule>(engine);
  addmodule<OOSupportModule>(engine);
  addmodule<RandomNumbers>(engine);
  addmodule<SpecialFunctionsModule>(engine);   // safe without GSL
  addmodule<SLIgraphics>(engine);
  engine.addmodule(new SLIStartup(argc,argv));
  addmodule<Processes>(engine);
  addmodule<RegexpModule>(engine);
  addmodule<FilesystemModule>(engine);

  // create the network and register with NestModule class
  pNet = new nest::Network(engine);
  assert(pNet != 0);
  nest::NestModule::register_network(*pNet);
  addmodule<nest::NestModule>(engine);

  // now add static modules providing models
  add_static_modules(engine, *pNet);

#ifdef HAVE_LIBLTDL  // no dynamic loading without LIBLTDL
  //dynamic loader module for managing linked and dynamically loaded extension modules
  nest::DynamicLoaderModule *pDynLoader = new nest::DynamicLoaderModule(pNet, engine);

  // initialize all modules that were linked into at compile time
  // these modules have registered via calling DynamicLoader::registerLinkedModule
  // from their constructor
  pDynLoader->initLinkedModules(engine);

  // interpreter will delete module on destruction
  engine.addmodule(pDynLoader);
#endif

  // add the init-script to the list of module initializers
  ArrayDatum *ad = dynamic_cast<ArrayDatum *>(engine.baselookup(engine.commandstring_name).datum());
  assert(ad != NULL);
  ad->push_back(new StringDatum("(" + path + "/pynest-init.sli) run"));

  return engine.startup();
}


static PyObject *initialize(PyObject *, PyObject *args)
{
  PyObject *pObj;
  char* modulepath;
  if (!PyArg_ParseTuple(args,"Os",&pObj, &modulepath))
  {
    PyErr_SetString(NESTError, "Unable to parse arguments.");
    return NULL;
  }

  char **argv = 0;
  int argc = 0;
  if (!stringlist_py2c(pObj, argc, argv) || argc == 0)
  {
    PyErr_SetString(NESTError, "argv is expected to be a non-empty list of strings.");
    return NULL;
  }

  if (pEngine != NULL)
  {
    PyErr_SetString(NESTError, "Already initialized.");
    return NULL;
  }

  pEngine = new SLIInterpreter;

  if (pEngine==NULL)
  {
    PyErr_SetString(NESTError, "Cannot create interpreter instance.");
    return NULL;
  }

#ifdef HAVE_MPI
  nest::Communicator::init(&argc, &argv);
#endif
  
  pyneststartup(argc, argv, *pEngine, pNet, modulepath);

  // clean up argv memory
  delete [] argv;
  Py_INCREF(Py_True);
  return Py_True;
}


/**
 * Finalize NEST by deleting the Network. This also finalizes
 * the MPI links.  
 */
static PyObject *finalize(PyObject *, PyObject *)
{
#ifdef HAVE_MPI
  nest::Communicator::finalize();
#endif

  // delete the Network before deleting modules in the interpreter's destructor.
  // Otherwise, there may still be references to models defined in the modules
  if (pNet != NULL)
  {
    delete pNet;
    pNet = NULL;
  }
    
  if (pEngine != NULL)
  {
    delete pEngine;
    pEngine = NULL;
  }

  return Py_BuildValue("");
}


/**********************************/
/* Method table for python module */
/**********************************/

static PyMethodDef MethodTable[] = {
  {"initialize",initialize, METH_VARARGS, "initialize(STRING argv) initializes pynest. argv: runtime arguments"},
  {"finalize",finalize, METH_VARARGS, "finalize SLI interpreter"},
  {"runsli",runsli, METH_VARARGS, "runsli(STRING command)"},
  {"pushsli",pushsli, METH_VARARGS, "Push an object onto the Sli stack"},
  {"popsli",popsli, METH_VARARGS, "Pop and object off the SLI stack and return it"},
  {"logstdout", logstdout, METH_VARARGS, "log stdout to a file instead of writing it to screen"},
  {"push_connection_datums", push_connection_datums, METH_VARARGS, "push a list of dictionaries as connectiontype objects"},
  {NULL, NULL, 0, NULL}
};

extern void PyCSA_init ();

PyMODINIT_FUNC initpynestkernel(void)
{
  NESTError = Py_BuildValue("s", "NESTError");

  if (PyType_Ready(&PyDatumType) < 0) {
    PyErr_SetString(NESTError, "Failed to initialize nest.Datum type.");
    return;
  }

  PyObject *m;
  m = Py_InitModule3("pynestkernel", MethodTable, "Basic Python Module to access NEST SLI kernel");

  if (m == NULL) {
    PyErr_SetString(NESTError, "Failure on Py_InitModule3.");
    return;
  }

#ifdef HAVE_NUMPY
  import_array(); // we need to set up the numeric array type
#endif

  Py_INCREF(&PyDatumType);
  PyModule_AddObject(m, "Datum", (PyObject *)&PyDatumType);

  PyCSA_init ();
}
