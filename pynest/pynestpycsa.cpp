/*
 *  pynestpycsa.cpp
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
 */

#include "pynestpycsa.h"

#include <string>
#include <iostream>

static PyObject *NESTError = NULL;

static PyObject* pMask = 0;
static PyObject* pConnectionSet = 0;
static PyObject* pCSAClasses = 0;
static PyObject* pArity = 0;
static PyObject* pCross = 0;
static PyObject* pPartition = 0;


static void
error (std::string errstring)
{
  PYGILSTATE_ENSURE (gstate);
  PyErr_SetString (NESTError, errstring.c_str ());
  PYGILSTATE_RELEASE (gstate);
}


static bool
CSAimported ()
{
  return PyMapping_HasKeyString (PyImport_GetModuleDict (), (char*)"csa");
}


static bool
loadCSA ()
{
  PYGILSTATE_ENSURE (gstate);
  PyObject* pModule = PyMapping_GetItemString (PyImport_GetModuleDict (), (char*)"csa");

  pMask = PyObject_GetAttrString (pModule, "Mask");
  if (pMask == NULL)
    {
      Py_DECREF (pModule);
      PYGILSTATE_RELEASE (gstate);
      error ("Couldn't find the Mask class in the CSA library");
      return false;
    }

  pConnectionSet = PyObject_GetAttrString (pModule, "ConnectionSet");
  if (pConnectionSet == NULL)
    {
      Py_DECREF (pModule);
      PYGILSTATE_RELEASE (gstate);
      error ("Couldn't find the ConnectionSet class in the CSA library");
      return false;
    }

  pArity = PyObject_GetAttrString (pModule, "arity");
  pCross = PyObject_GetAttrString (pModule, "cross");
  pPartition = PyObject_GetAttrString (pModule, "partition");
  Py_DECREF (pModule);
  if (pArity == NULL)
    {
      PYGILSTATE_RELEASE (gstate);
      error ("Couldn't find the arity function in the CSA library");
      return false;
    }

  pCSAClasses = PyTuple_Pack (2, pMask, pConnectionSet);
  PYGILSTATE_RELEASE (gstate);
  return true;
}


bool PyPyCSA_Check (PyObject* obj)
{
  if (pCSAClasses == 0)
    {
      if (!CSAimported ())
	return false;

      // load CSA library
      bool status = loadCSA ();
      if (!status)
	return false;
    }

  return PyObject_IsInstance (obj, pCSAClasses);
}


PyCSAGenerator::PyCSAGenerator (PyObject* obj)
  : pCSAObject (obj), pPartitionedCSAObject (NULL), pIterator (NULL)
{
  PYGILSTATE_ENSURE (gstate);
  Py_INCREF (pCSAObject);
  PyObject* a = PyObject_CallFunctionObjArgs (pArity, pCSAObject, NULL);
  arity_ = PyInt_AsLong (a);
  Py_DECREF (a);
  PYGILSTATE_RELEASE (gstate);
}


PyCSAGenerator::~PyCSAGenerator ()
{
  PYGILSTATE_ENSURE (gstate);
  Py_XDECREF (pIterator);
  Py_XDECREF (pPartitionedCSAObject);
  Py_DECREF (pCSAObject);
  PYGILSTATE_RELEASE (gstate);
}


int
PyCSAGenerator::arity ()
{
  return arity_;
}


PyObject*
PyCSAGenerator::makeIntervals (IntervalSet& iset)
{
  PyObject* ivals = PyList_New (0);
  if (iset.skip () == 1)
    {
      for (IntervalSet::iterator i = iset.begin (); i != iset.end (); ++i)
	PyList_Append (ivals,
		       PyTuple_Pack (2,
				     PyInt_FromLong (i->first),
				     PyInt_FromLong (i->last)));
    }
  else
    {
      for (IntervalSet::iterator i = iset.begin (); i != iset.end (); ++i)
	{
	  int last = i->last;
	  for (int j = i->first; j < last; j += iset.skip ())
	    PyList_Append (ivals,
			   PyTuple_Pack (2,
					 PyInt_FromLong (j),
					 PyInt_FromLong (j)));
	}
    }
  return ivals;
}


void
PyCSAGenerator::setMask (std::vector<Mask>& masks, int local)
{
  PYGILSTATE_ENSURE (gstate);
  PyObject* pMasks = PyList_New (masks.size ());
  for (size_t i = 0; i < masks.size (); ++i)
    {
      PyObject* pMask
	= PyObject_CallFunctionObjArgs (pCross,
					makeIntervals (masks[i].sources),
					makeIntervals (masks[i].targets),
					NULL);
      PyList_SetItem (pMasks, i, pMask);
    }

  Py_XDECREF (pPartitionedCSAObject);
  pPartitionedCSAObject = PyObject_CallFunctionObjArgs (pPartition,
							pCSAObject,
							pMasks,
							PyInt_FromLong (local),
							NULL);
  if (pPartitionedCSAObject == NULL)
    {
      PYGILSTATE_RELEASE (gstate);
      std::cerr << "Failed to create masked CSA object" << std::endl;
      return;
    }
  Py_INCREF (pPartitionedCSAObject); //*fixme* check if necessary!
  PYGILSTATE_RELEASE (gstate);
}


int
PyCSAGenerator::size ()
{
  PYGILSTATE_ENSURE (gstate);
  int size = PySequence_Size (pCSAObject);
  PYGILSTATE_RELEASE (gstate);
  return size;
}


void
PyCSAGenerator::start ()
{
  if (pPartitionedCSAObject == NULL)
    {
      error ("CSA connection generator not properly initialized");
      return;
    }
  PYGILSTATE_ENSURE (gstate);
  Py_XDECREF (pIterator);
  pIterator = PyObject_GetIter (pPartitionedCSAObject);
  PYGILSTATE_RELEASE (gstate);
}


bool
PyCSAGenerator::next (int& source, int& target, double* value)
{
  if (pIterator == NULL)
    {
      error ("Must call start() before next()");
      return false;
    }

  PYGILSTATE_ENSURE (gstate);
  PyObject* tuple = PyIter_Next (pIterator);
  PyObject* err = PyErr_Occurred ();
  if (err)
    {
      PYGILSTATE_RELEASE (gstate);
      return false;
    }

  if (tuple == NULL)
    {
      Py_DECREF (pIterator);
      pIterator = NULL;
      PYGILSTATE_RELEASE (gstate);
      return false;
    }

  source = PyInt_AsLong (PyTuple_GET_ITEM (tuple, 0));
  target = PyInt_AsLong (PyTuple_GET_ITEM (tuple, 1));
  for (int i = 0; i < arity_; ++i)
    {
      PyObject* v = PyTuple_GET_ITEM (tuple, i + 2);
      if (!PyFloat_Check (v))
	{
	  Py_DECREF (tuple);
	  PYGILSTATE_RELEASE (gstate);
	  error ("NEST cannot handle non-float CSA value sets");
	  return false;
	}
      value[i] = PyFloat_AsDouble (v);
    }

  Py_DECREF (tuple);
  PYGILSTATE_RELEASE (gstate);
  return true;
}

void
PyCSA_init(void)
{
  NESTError = Py_BuildValue("s", "NESTError");
}
