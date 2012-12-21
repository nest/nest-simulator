/*
 *  pydatum.h
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

#ifndef PYTHONDATUM_H
#define PYTHONDATUM_H

extern "C" {
#include<Python.h>
}

#include "token.h"
#include "datum.h"

/**
 * Python class for encapsulating generic Datums which can not be converted
 * to a native python type.
 */
struct PyDatum
{
    PyObject_HEAD
    Token token;
};

extern PyTypeObject PyDatumType;

/**
 * Create a new PyDatum object initialized with the given Datum.
 * @returns new reference.
 */
extern PyObject* PyDatum_FromDatum(Datum &d);

/**
 * Get the pointer to the Datum contained in this PyDatum.
 */
inline
Datum* PyDatum_GetDatum(PyDatum *pyd)
{
  return pyd->token.datum();
}

/**
 * Check if the object is a PyDatum.
 * @returns true if the object is a PyDatum.
 */
inline
bool PyDatum_Check(PyObject *pObj)
{
  return PyObject_IsInstance(pObj,reinterpret_cast<PyObject*>(&PyDatumType));
}

#endif
