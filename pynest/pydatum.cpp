/*
 *  pydatum.cpp
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

#include "pydatum.h"
#include <sstream>

PyObject* PyDatum_FromDatum(Datum &d)
{
  PyDatum *py_datum = PyObject_New(PyDatum, &PyDatumType);
  new(&py_datum->token) Token(d);
  return reinterpret_cast<PyObject*>(py_datum);
}

static void PyDatum_dealloc(PyDatum* self)
{
  self->token.~Token();
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject* PyDatum_gettype(PyDatum *self, void *)
{
  return PyString_FromString(self->token->gettypename().toString().c_str());
}

static PyObject* PyDatum_str(PyDatum *self)
{
  std::stringstream s;
  self->token->print(s);
  return PyString_FromString(s.str().c_str());
}

static PyObject* PyDatum_repr(PyDatum *self)
{
  std::stringstream s;
  self->token->pprint(s);
  return PyString_FromString(s.str().c_str());
}

static PyGetSetDef PyDatum_getseters[] = {
  {"type", (getter)PyDatum_gettype, NULL, "type", NULL},
  {NULL, NULL, NULL, NULL, NULL}  /* Sentinel */
};

PyTypeObject PyDatumType = {
  PyObject_HEAD_INIT(NULL)
  0,                                    /* ob_size */
  "nest.Datum",                         /* tp_name */
  sizeof(PyDatum),                      /* tp_basicsize */
  0,                                    /* tp_itemsize */
  (destructor)PyDatum_dealloc,          /* tp_dealloc */
  0,                                    /* tp_print */
  0,                                    /* tp_getattr */
  0,                                    /* tp_setattr */
  0,                                    /* tp_compare */
  (reprfunc)PyDatum_repr,               /* tp_repr */
  0,                                    /* tp_as_number */
  0,                                    /* tp_as_sequence */
  0,                                    /* tp_as_mapping */
  0,                                    /* tp_hash  */
  0,                                    /* tp_call */
  (reprfunc)PyDatum_str,                /* tp_str */
  0,                                    /* tp_getattro */
  0,                                    /* tp_setattro */
  0,                                    /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,                   /* tp_flags */
  "Python encapsulation of SLI Datums", /* tp_doc */
  0,                                    /* tp_traverse */
  0,                                    /* tp_clear */
  0,                                    /* tp_richcompare */
  0,                                    /* tp_weaklistoffset */
  0,                                    /* tp_iter */
  0,                                    /* tp_iternext */
  0,                                    /* tp_methods */
  0,                                    /* tp_members */
  PyDatum_getseters,                    /* tp_getset */
  0,                                    /* tp_base */
  0,                                    /* tp_dict */
  0,                                    /* tp_descr_get */
  0,                                    /* tp_descr_set */
  0,                                    /* tp_dictoffset */
  0,                                    /* tp_init */
  0,                                    /* tp_alloc */
  0,                                    /* tp_new */
  0,                                    /* tp_free */
  0,                                    /* tp_is_gc */
  0,                                    /* tp_bases */
  0,                                    /* tp_mro */
  0,                                    /* tp_cache */
  0,                                    /* tp_subclasses */
  0,                                    /* tp_weaklist */
  0,                                    /* tp_del */
};

