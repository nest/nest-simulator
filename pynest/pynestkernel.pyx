# -*- coding: utf-8 -*-
#
# pynestkernel.pyx
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.
#
# distutils: language = c++
#

import cython

from libc.stdlib cimport malloc, free
from libc.string cimport memcpy

from libcpp.string cimport string
from libcpp.vector cimport vector

from cython.operator cimport dereference as deref
from cython.operator cimport preincrement as inc

from cpython cimport array

from cpython.ref cimport PyObject
from cpython.object cimport Py_LT, Py_LE, Py_EQ, Py_NE, Py_GT, Py_GE


cdef string SLI_TYPE_BOOL = "booltype"
cdef string SLI_TYPE_INTEGER = "integertype"
cdef string SLI_TYPE_DOUBLE = "doubletype"
cdef string SLI_TYPE_STRING = "stringtype"
cdef string SLI_TYPE_LITERAL = "literaltype"
cdef string SLI_TYPE_ARRAY = "arraytype"
cdef string SLI_TYPE_DICTIONARY = "dictionarytype"
cdef string SLI_TYPE_CONNECTION = "connectiontype"
cdef string SLI_TYPE_VECTOR_INT = "intvectortype"
cdef string SLI_TYPE_VECTOR_DOUBLE = "doublevectortype"
cdef string SLI_TYPE_MASK = "masktype"
cdef string SLI_TYPE_PARAMETER = "parametertype"


DEF CONN_ELMS = 5

cdef unicode CONN_NAME_SRC = u"source"
cdef unicode CONN_NAME_SYN = u"synapse_modelid"
cdef unicode CONN_NAME_PRT = u"port"
cdef unicode CONN_NAME_THREAD = u"target_thread"

CONN_LEN = CONN_ELMS


cdef ARRAY_LONG = array.array('l')
cdef ARRAY_DOUBLE = array.array('d')


cdef bint HAVE_NUMPY = False

try:
    import numpy
    HAVE_NUMPY = True
except ImportError:
    pass


class NESTError(Exception):
    pass


cdef class SLIDatum(object):

    cdef Datum* thisptr
    cdef readonly unicode dtype

    def __cinit__(self):

        self.dtype = u""
        self.thisptr = NULL

    def __dealloc__(self):

        if self.thisptr is not NULL:
            del self.thisptr

    def __repr__(self):

        if self.thisptr is not NULL:
            return "<SLIDatum: {0}>".format(self.dtype)
        else:
            return "<SLIDatum: unassociated>"

    cdef _set_datum(self, Datum* dat, unicode dtype):

        self.dtype = dtype
        self.thisptr = dat


cdef class SLILiteral(object):

    cdef readonly object name
    cdef object _hash

    def __init__(self, name):
        self.name = str(name)
        self._hash = None

    def __hash__(self):

        if self._hash is None:
            self._hash = hash(self.name)

        return self._hash

    def __repr__(self):
        return "<SLILiteral: {0}>".format(self.name)

    def __str__(self):
        return "{0}".format(self.name)

    def __richcmp__(self, other, int op):

        if isinstance(other, SLILiteral):
            obj = other.name
        else:
            obj = other

        if op == Py_LT:
            return self.name < obj
        elif op == Py_EQ:
            return self.name == obj
        elif op == Py_GT:
            return self.name > obj
        elif op == Py_LE:
            return self.name <= obj
        elif op == Py_NE:
            return self.name != obj
        elif op == Py_GE:
            return self.name >= obj


cdef class NESTEngine(object):

    cdef SLIInterpreter* pEngine
    cdef Network* pNet

    def __cinit__(self):

        self.pNet = NULL
        self.pEngine = NULL

    def __dealloc__(self):

        nestshutdown()

        del self.pNet
        del self.pEngine

        self.pNet = NULL
        self.pEngine = NULL

    def init(self, argv, modulepath):

        if self.pEngine is not NULL:
            raise NESTError("engine already initialized")

        cdef int argc = len(argv)

        if argc <= 0:
            raise NESTError("argv can't be empty")

        cdef string modulepath_str = modulepath.encode()

        cdef char* arg0 = "pynest\0"
        cdef char** argv_bytes = <char**> malloc((argc+1) * sizeof(char*))

        if argv_bytes is NULL:
            raise NESTError("couldn't allocate argv_bytes")

        argv_bytes[0] = arg0        
        argv_bytes[argc] = NULL

        try:
            for i in range(1, argc):
                arg_byte = argv[i].encode()
                argv_bytes[i] = arg_byte

            self.pEngine = new SLIInterpreter()

            neststartup(cython.address (argc),
                        cython.address (argv_bytes),
                        deref(self.pEngine),
                        self.pNet,
                        modulepath_str)
            # If using MPI, argv might now have changed, so rebuild it
            del argv[:]
            for i in range(argc):
                # str(...) will convert to ordinary string in Python2.7
                argv.append(str(argv_bytes[i].decode()))
        finally:
            free(argv_bytes)

        return True

    def run(self, cmd):

        if self.pEngine is NULL:
            raise NESTError("engine uninitialized")

        cdef string cmd_bytes = cmd.encode()

        self.pEngine.execute(cmd_bytes)

    def push(self, obj):

        if self.pEngine is NULL:
            raise NESTError("engine uninitialized")

        self.pEngine.OStack.push(python_object_to_datum(obj))

    def pop(self):

        if self.pEngine is NULL:
            raise NESTError("engine uninitialized")

        if self.pEngine.OStack.empty():
            raise NESTError("interpreter stack is empty")

        cdef Datum* dat = (addr_tok(self.pEngine.OStack.top())).datum()

        ret = sli_datum_to_object(dat)

        self.pEngine.OStack.pop()

        return ret

    def push_connection_datums(self, conns):

        cdef ConnectionDatum* cdt = NULL
        cdef ArrayDatum* connectome = new ArrayDatum()

        try:
            connectome.reserve(len(conns))

            for cnn in conns:
                if isinstance(cnn, dict):
                    cdt = new ConnectionDatum(ConnectionID(cnn[CONN_NAME_SRC], cnn[CONN_NAME_THREAD], cnn[CONN_NAME_SYN], cnn[CONN_NAME_PRT]))
                else:
                    cdt = new ConnectionDatum(ConnectionID(cnn[0], cnn[1], cnn[2], cnn[3], cnn[4]))

                connectome.push_back(<Datum*> cdt)

            self.pEngine.OStack.push(<Datum*> connectome)

        except:
            del connectome
            raise

cdef inline Datum* python_object_to_datum(obj) except NULL:

    cdef Datum* ret = NULL

    cdef ArrayDatum* ad = NULL
    cdef DictionaryDatum* dd = NULL

    cdef string obj_str

    if isinstance(obj, bool):
        ret = <Datum*> new BoolDatum(obj)
    elif isinstance(obj, (int, long)):
        ret = <Datum*> new IntegerDatum(obj)
    elif isinstance(obj, float):
        ret = <Datum*> new DoubleDatum(obj)
    elif isinstance(obj, bytes):
        obj_str = obj
        ret = <Datum*> new StringDatum(obj_str)
    elif isinstance(obj, unicode):
        obj_str = obj.encode()
        ret = <Datum*> new StringDatum(obj_str)
    elif isinstance(obj, SLILiteral):
        obj_str = obj.name.encode()
        ret = <Datum*> new LiteralDatum(obj_str)
    elif isinstance(obj, (tuple, list, xrange)):
        ad = new ArrayDatum()
        ad.reserve(len(obj))
        for x in obj:
            ad.push_back(python_object_to_datum(x))
        ret = <Datum*> ad
    elif isinstance(obj, dict):
        dd = new DictionaryDatum(new Dictionary())
        for k, v in obj.items():
            obj_str = str(k).encode()
            deref_dict(dd).insert(obj_str, python_object_to_datum(v))
        ret = <Datum*> dd
    elif HAVE_NUMPY and (
        isinstance(obj, numpy.integer) or                   # integral scalars
        isinstance(obj, numpy.floating) or                  # floating point scalars
        (isinstance(obj, numpy.ndarray) and obj.ndim == 0)  # zero-rank arrays
    ):
        ret = python_object_to_datum(obj.item())
    elif isinstance(obj, SLIDatum):
        if (<SLIDatum> obj).dtype == SLI_TYPE_MASK.decode():
            ret = <Datum*> new MaskDatum(deref(<MaskDatum*> (<SLIDatum> obj).thisptr))
        elif (<SLIDatum> obj).dtype == SLI_TYPE_PARAMETER.decode():
            ret = <Datum*> new ParameterDatum(deref(<ParameterDatum*> (<SLIDatum> obj).thisptr))
        else:
            raise NESTError("unknown SLI datum type: {0}".format((<SLIDatum> obj).dtype))
    elif isConnectionGenerator(<PyObject*> obj):
        ret = unpackConnectionGeneratorDatum(<PyObject*> obj)
        if ret is NULL:
            raise NESTError("failed to unpack passed connection generator object")
    else:

        try:
            ret = python_buffer_to_datum[buffer_int_1d_t, long](obj)
        except (ValueError, TypeError):
            pass

        try:
            ret = python_buffer_to_datum[buffer_long_1d_t, long](obj)
        except (ValueError, TypeError):
            pass

        try:
            ret = python_buffer_to_datum[buffer_float_1d_t, double](obj)
        except (ValueError, TypeError):
            pass

        try:
            ret = python_buffer_to_datum[buffer_double_1d_t, double](obj)
        except (ValueError, TypeError):
            pass

        # NumPy < 1.5.0 doesn't support PEP-3118 buffer interface
        #
        if ret is NULL and HAVE_NUMPY and isinstance(obj, numpy.ndarray) and obj.ndim == 1:
            if numpy.issubdtype(obj.dtype, numpy.integer):
                ret = python_buffer_to_datum[object, long](obj)
            elif numpy.issubdtype(obj.dtype, numpy.floating):
                ret = python_buffer_to_datum[object, double](obj)
            else:
                raise NESTError("only vectors of integers or floats are supported")

        if ret is not NULL:
            return ret
        else:
            raise NESTError("unknown Python type: {0}".format(type(obj)))

    if ret is NULL:
        raise NESTError("conversion resulted in a null pointer")

    return ret

@cython.boundscheck(False)
cdef inline Datum* python_buffer_to_datum(numeric_buffer_t buff, vector_value_t _ = 0) except NULL:

    cdef size_t i, n

    cdef Datum* dat = NULL
    cdef vector[vector_value_t]* vector_ptr = new vector[vector_value_t]()

    if vector_value_t is long:
        dat = <Datum*> new IntVectorDatum(vector_ptr)
    elif vector_value_t is double:
        dat = <Datum*> new DoubleVectorDatum(vector_ptr)
    else:
        raise NESTError("unsupported specialization: {0}".format(vector_value_t))

    n = len(buff)

    vector_ptr.reserve(n)

    for i in range(n):
        vector_ptr.push_back(<vector_value_t> buff[i])

    return <Datum*> dat


cdef inline object sli_datum_to_object(Datum* dat):

    if dat is NULL:
        raise NESTError("datum is a null pointer")

    cdef string obj_str
    cdef object ret = None

    cdef string datum_type = dat.gettypename().toString()

    if datum_type == SLI_TYPE_BOOL:
        ret = (<BoolDatum*> dat).get()
    elif datum_type == SLI_TYPE_INTEGER:
        ret = (<IntegerDatum*> dat).get()
    elif datum_type == SLI_TYPE_DOUBLE:
        ret = (<DoubleDatum*> dat).get()
    elif datum_type == SLI_TYPE_STRING:
         ret = (<string> deref_str(<StringDatum*> dat)).decode()
    elif datum_type == SLI_TYPE_LITERAL:
        obj_str = (<LiteralDatum*> dat).toString()
        ret = SLILiteral(obj_str.decode())
    elif datum_type == SLI_TYPE_ARRAY:
        ret = sli_array_to_object(<ArrayDatum*> dat)
    elif datum_type == SLI_TYPE_DICTIONARY:
        ret = sli_dict_to_object(<DictionaryDatum*> dat)
    elif datum_type == SLI_TYPE_CONNECTION:
        ret = sli_connection_to_object(<ConnectionDatum*> dat)
    elif datum_type == SLI_TYPE_VECTOR_INT:
        ret = sli_vector_to_object[sli_vector_int_ptr_t, long](<IntVectorDatum*> dat)
    elif datum_type == SLI_TYPE_VECTOR_DOUBLE:
        ret = sli_vector_to_object[sli_vector_double_ptr_t, double](<DoubleVectorDatum*> dat)
    elif datum_type == SLI_TYPE_MASK:
        ret = SLIDatum()
        (<SLIDatum> ret)._set_datum(<Datum*> new MaskDatum(deref(<MaskDatum*> dat)), SLI_TYPE_MASK.decode())
    elif datum_type == SLI_TYPE_PARAMETER:
        ret = SLIDatum()
        (<SLIDatum> ret)._set_datum(<Datum*> new ParameterDatum(deref(<ParameterDatum*> dat)), SLI_TYPE_PARAMETER.decode())
    else:
        raise NESTError("unknown SLI type: {0}".format(datum_type.decode()))

    if ret is None:
        raise NESTError("conversion resulted in a None object")

    return ret

cdef inline object sli_array_to_object(ArrayDatum* dat):

    cdef tmp = [None] * dat.size()

    cdef size_t i
    cdef Token* tok = dat.begin()

    for i in range(len(tmp)):
        tmp[i] = sli_datum_to_object(tok.datum())
        inc(tok)

    return tuple(tmp)

cdef inline object sli_dict_to_object(DictionaryDatum* dat):

    cdef tmp = {}

    cdef string key_str
    cdef const Token* tok = NULL

    cdef TokenMap.const_iterator dt = deref_dict(dat).begin()

    while dt != deref_dict(dat).end():
        key_str = deref_tmap(dt).first.toString()
        tok = &deref_tmap(dt).second
        tmp[key_str.decode()] = sli_datum_to_object(tok.datum())
        inc(dt)

    return tmp

cdef inline object sli_connection_to_object(ConnectionDatum* dat):

    cdef array.array arr
    cdef long[CONN_ELMS] CONN_ARR

    arr = array.clone(ARRAY_LONG, CONN_ELMS, False)
    CONN_ARR[0] = dat.get_source_gid()
    CONN_ARR[1] = dat.get_target_gid()
    CONN_ARR[2] = dat.get_target_thread()
    CONN_ARR[3] = dat.get_synapse_model_id()
    CONN_ARR[4] = dat.get_port()
    memcpy(arr.data.as_longs, &CONN_ARR, CONN_ELMS * sizeof(long))

    return arr

cdef inline object sli_vector_to_object(sli_vector_ptr_t dat, vector_value_t _ = 0):

    cdef vector_value_t* array_data = NULL
    cdef vector[vector_value_t]* vector_ptr = NULL

    if sli_vector_ptr_t is sli_vector_int_ptr_t and vector_value_t is long:
        vector_ptr = deref_ivector(dat)
        arr = array.clone(ARRAY_LONG, vector_ptr.size(), False)
        array_data = arr.data.as_longs
        if HAVE_NUMPY:
            ret_dtype = numpy.int_
    elif sli_vector_ptr_t is sli_vector_double_ptr_t and vector_value_t is double:
        vector_ptr = deref_dvector(dat)
        arr = array.clone(ARRAY_DOUBLE, vector_ptr.size(), False)
        array_data = arr.data.as_doubles
        if HAVE_NUMPY:
            ret_dtype = numpy.float_
    else:
        raise NESTError("unsupported specialization")

    memcpy(array_data, &vector_ptr.front(), vector_ptr.size() * sizeof(vector_value_t))

    if HAVE_NUMPY:
        if vector_ptr.size() > 0:
            return numpy.frombuffer(arr, dtype=ret_dtype)
        else:
            # Compatibility with NumPy < 1.7.0
            return numpy.array([], dtype=ret_dtype)
    else:
        return arr
