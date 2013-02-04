/*
 *  pynestpycsa.h
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

#ifndef PYNESTPYCSA_H
#define PYNESTPYCSA_H

extern "C" {
#include<Python.h>
}

#ifdef WITH_THREAD
/* CPython compiled with threads */
#define PYGILSTATE_ENSURE(VAR)  PyGILState_STATE VAR = PyGILState_Ensure()
#define PYGILSTATE_RELEASE(VAR) PyGILState_Release (VAR)
#else
#define PYGILSTATE_ENSURE(VAR)
#define PYGILSTATE_RELEASE(VAR)
#endif

#include "connection_generator.h"

bool PyPyCSA_Check (PyObject* obj);

class PyCSAGenerator : public ConnectionGenerator {
  PyObject* pCSAObject;
  PyObject* pPartitionedCSAObject;
  int arity_;
  PyObject* pIterator;
 private:
  PyObject* makeIntervals (IntervalSet& iset);
 public:
  PyCSAGenerator (PyObject* obj);

  ~PyCSAGenerator ();

  int arity ();

  void setMask (std::vector<Mask>& masks, int local);

  int size ();

  void start ();

  bool next (int& source, int& target, double* value);
};

#endif
