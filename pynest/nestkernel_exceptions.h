/*
 *  nestkernel_exceptions.h
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

#include <Python.h>
#include <ios>
#include <stdexcept>

#include "exceptions.h"
//#include "exception_names.h // TODO: PyNEST-NG

static struct PyModuleDef
  nest_module = { PyModuleDef_HEAD_INIT, "NESTError", nullptr, -1, nullptr, nullptr, nullptr, nullptr, nullptr };
PyObject* nest_error_module = PyModule_Create( &nest_module );

std::map< std::string, PyObject* > nest_exceptions;

void
create_exceptions()
{

  std::vector< std::string > exception_names = { "UnknownModelName" };
  for ( auto name : exception_names )
  {
    PyObject* exception = PyErr_NewException( ( "NESTError." + name ).c_str(), nullptr, nullptr );
    nest_exceptions[ name ] = exception;
    Py_INCREF( exception );
    PyModule_AddObject( nest_error_module, name.c_str(), exception );
  }
}

void
custom_exception_handler()
{
  try
  {
    if ( PyErr_Occurred() )
    {
      ; // let the latest Python exn pass through and ignore the current one
    }
    else
    {
      throw;
    }
  }
  catch ( nest::KernelException& exn )
  {
    PyErr_SetString( nest_exceptions[ exn.exception_name() ], exn.what() );
  }
  catch ( ... )
  {
    PyErr_SetString( PyExc_RuntimeError, "Unexpected C++ exception" );
  }
}
