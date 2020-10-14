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

import nest
from nest.lib.hl_api_exceptions import NESTMappedException, NESTErrors, NESTError


cdef string SLI_TYPE_BOOL = b"booltype"
cdef string SLI_TYPE_INTEGER = b"integertype"
cdef string SLI_TYPE_DOUBLE = b"doubletype"
cdef string SLI_TYPE_STRING = b"stringtype"
cdef string SLI_TYPE_LITERAL = b"literaltype"
cdef string SLI_TYPE_ARRAY = b"arraytype"
cdef string SLI_TYPE_DICTIONARY = b"dictionarytype"
cdef string SLI_TYPE_CONNECTION = b"connectiontype"
cdef string SLI_TYPE_VECTOR_INT = b"intvectortype"
cdef string SLI_TYPE_VECTOR_DOUBLE = b"doublevectortype"
cdef string SLI_TYPE_MASK = b"masktype"
cdef string SLI_TYPE_PARAMETER = b"parametertype"
cdef string SLI_TYPE_NODECOLLECTION = b"nodecollectiontype"
cdef string SLI_TYPE_NODECOLLECTIONITERATOR = b"nodecollectioniteratortype"


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

    def __cinit__(self):

        self.pEngine = NULL

    def __dealloc__(self):

        nestshutdown( 0 )

        del self.pEngine

        self.pEngine = NULL

    def set_communicator(self, comm):
        # extract mpi_comm from mpi4py
        if nest_has_mpi4py():
            c_set_communicator(comm)
        else:
            raise NESTError("set_communicator: "
                            "NEST not compiled with MPI4PY")

    def init(self, argv, modulepath):
        if self.pEngine is not NULL:
            raise NESTErrors.PyNESTError("engine already initialized")

        cdef int argc = <int> len(argv)
        if argc <= 0:
            raise NESTErrors.PyNESTError("argv can't be empty")

        # Create c-style argv arguments from sys.argv
        cdef char** argv_chars = <char**> malloc((argc+1) * sizeof(char*))
        if argv_chars is NULL:
            raise NESTErrors.PyNESTError("couldn't allocate argv_char")
        try:
            # argv must be null terminated. openmpi depends on this
            argv_chars[argc] = NULL

            # Need to keep a reference to encoded bytes issue #377
            # argv_bytes = [byte...] which internally holds a reference
            # to the c string in argv_char = [c-string... NULL]
            # the `byte` is the utf-8 encoding of sys.argv[...]
            argv_bytes = [argvi.encode() for argvi in argv]
            for i, argvi in enumerate(argv_bytes):
                argv_chars[i] = argvi # c-string ref extracted

            self.pEngine = new SLIInterpreter()
            modulepath_bytes = modulepath.encode()

            neststartup(&argc,
                        &argv_chars,
                        deref(self.pEngine),
                        modulepath_bytes)

            # If using MPI, argv might now have changed, so rebuild it
            del argv[:]
            # Convert back from utf8 char* to utf8 str
            argv.extend(str(argvi.decode()) for argvi in argv_chars[:argc])
        finally:
            free(argv_chars)

        return True

    def run(self, cmd):

        if self.pEngine is NULL:
            raise NESTErrors.PyNESTError("engine uninitialized")
        cdef string cmd_bytes
        cmd_bytes = cmd.encode('utf-8')
        self.pEngine.execute(cmd_bytes)

    def push(self, obj):

        if self.pEngine is NULL:
            raise NESTErrors.PyNESTError("engine uninitialized")
        self.pEngine.OStack.push(python_object_to_datum(obj))

    def pop(self):

        if self.pEngine is NULL:
            raise NESTErrors.PyNESTError("engine uninitialized")

        if self.pEngine.OStack.empty():
            raise NESTErrors.PyNESTError("interpreter stack is empty")

        cdef Datum* dat = (addr_tok(self.pEngine.OStack.top())).datum()

        ret = sli_datum_to_object(dat)

        self.pEngine.OStack.pop()

        return ret

    def take_array_index(self, node_collection, array):
        if self.pEngine is NULL:
            raise NESTErrors.PyNESTError("engine uninitialized")

        if not (isinstance(node_collection, SLIDatum) and (<SLIDatum> node_collection).dtype == SLI_TYPE_NODECOLLECTION.decode()):
            raise TypeError('node_collection must be a NodeCollection, got {}'.format(type(node_collection)))
        if not isinstance(array, numpy.ndarray):
            raise TypeError('array must be a 1-dimensional NumPy array of ints or bools, got {}'.format(type(array)))
        if not array.ndim == 1:
            raise TypeError('array must be a 1-dimensional NumPy array, got {}-dimensional NumPy array'.format(array.ndim))

        # Get pointers to the first element in the Numpy array
        cdef long[:] array_long_mv
        cdef long* array_long_ptr

        cdef cbool[:] array_bool_mv
        cdef cbool* array_bool_ptr

        cdef Datum* nc_datum = python_object_to_datum(node_collection)

        try:
            if array.dtype == numpy.bool:
                # Boolean C-type arrays are not supported in NumPy, so we use an 8-bit integer array
                array_bool_mv = numpy.ascontiguousarray(array, dtype=numpy.uint8)
                array_bool_ptr = &array_bool_mv[0]
                new_nc_datum = node_collection_array_index(nc_datum, array_bool_ptr, len(array))
                return sli_datum_to_object(new_nc_datum)
            elif numpy.issubdtype(array.dtype, numpy.integer):
                array_long_mv = numpy.ascontiguousarray(array, dtype=numpy.long)
                array_long_ptr = &array_long_mv[0]
                new_nc_datum = node_collection_array_index(nc_datum, array_long_ptr, len(array))
                return sli_datum_to_object(new_nc_datum)
            else:
                raise TypeError('array must be a NumPy array of ints or bools, got {}'.format(array.dtype))
        except RuntimeError as e:
            exceptionCls = getattr(NESTErrors, str(e))
            raise exceptionCls('take_array_index', '') from None

    def connect_arrays(self, sources, targets, weights, delays, synapse_model, syn_param_keys, syn_param_values):
        """Calls connect_arrays function, bypassing SLI to expose pointers to the NumPy arrays"""
        if self.pEngine is NULL:
            raise NESTErrors.PyNESTError("engine uninitialized")
        if not HAVE_NUMPY:
            raise NESTErrors.PyNESTError("NumPy is not available")

        if not (isinstance(sources, numpy.ndarray) and sources.ndim == 1) or not numpy.issubdtype(sources.dtype, numpy.integer):
            raise TypeError('sources must be a 1-dimensional NumPy array of integers')
        if not (isinstance(targets, numpy.ndarray) and targets.ndim == 1) or not numpy.issubdtype(targets.dtype, numpy.integer):
            raise TypeError('targets must be a 1-dimensional NumPy array of integers')
        if weights is not None and not (isinstance(weights, numpy.ndarray) and weights.ndim == 1):
            raise TypeError('weights must be a 1-dimensional NumPy array')
        if delays is not None and  not (isinstance(delays, numpy.ndarray) and delays.ndim == 1):
            raise TypeError('delays must be a 1-dimensional NumPy array')
        if syn_param_keys is not None and not ((isinstance(syn_param_keys, numpy.ndarray) and syn_param_keys.ndim == 1) and
                                              numpy.issubdtype(syn_param_keys.dtype, numpy.string_)):
            raise TypeError('syn_param_keys must be a 1-dimensional NumPy array of strings')
        if syn_param_values is not None and not ((isinstance(syn_param_values, numpy.ndarray) and syn_param_values.ndim == 2)):
            raise TypeError('syn_param_values must be a 2-dimensional NumPy array')

        if not len(sources) == len(targets):
            raise ValueError('Sources and targets must be arrays of the same length.')
        if weights is not None:
            if not len(sources) == len(weights):
                raise ValueError('weights must be an array of the same length as sources and targets.')
        if delays is not None:
            if not len(sources) == len(delays):
                raise ValueError('delays must be an array of the same length as sources and targets.')
        if syn_param_values is not None:
            if not len(syn_param_keys) == syn_param_values.shape[0]:
                raise ValueError('syn_param_values must be a matrix with one array per key in syn_param_keys.')
            if not len(sources) == syn_param_values.shape[1]:
                raise ValueError('syn_param_values must be a matrix with arrays of the same length as sources and targets.')

        # Get pointers to the first element in each NumPy array
        cdef long[::1] sources_mv = numpy.ascontiguousarray(sources, dtype=numpy.long)
        cdef long* sources_ptr = &sources_mv[0]

        cdef long[::1] targets_mv = numpy.ascontiguousarray(targets, dtype=numpy.long)
        cdef long* targets_ptr = &targets_mv[0]

        cdef double[::1] weights_mv
        cdef double* weights_ptr = NULL
        if weights is not None:
            weights_mv = numpy.ascontiguousarray(weights, dtype=numpy.double)
            weights_ptr = &weights_mv[0]

        cdef double[::1] delays_mv
        cdef double* delays_ptr = NULL
        if delays is not None:
            delays_mv = numpy.ascontiguousarray(delays, dtype=numpy.double)
            delays_ptr = &delays_mv[0]

        # Storing parameter keys in a vector of strings
        cdef vector[string] param_keys_ptr
        if syn_param_keys is not None:
            for i, key in enumerate(syn_param_keys):
                param_keys_ptr.push_back(key)

        cdef double[:, ::1] param_values_mv
        cdef double* param_values_ptr = NULL
        if syn_param_values is not None:
            param_values_mv = numpy.ascontiguousarray(syn_param_values, dtype=numpy.double)
            param_values_ptr = &param_values_mv[0][0]

        cdef string syn_model_string = synapse_model.encode('UTF-8')

        try:
            connect_arrays( sources_ptr, targets_ptr, weights_ptr, delays_ptr, param_keys_ptr, param_values_ptr, len(sources), syn_model_string )
        except RuntimeError as e:
            exceptionCls = getattr(NESTErrors, str(e))
            raise exceptionCls('connect_arrays', '') from None

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
        elif (<SLIDatum> obj).dtype == SLI_TYPE_NODECOLLECTION.decode():
            ret = <Datum*> new NodeCollectionDatum(deref(<NodeCollectionDatum*> (<SLIDatum> obj).thisptr))
        elif (<SLIDatum> obj).dtype == SLI_TYPE_NODECOLLECTIONITERATOR.decode():
            ret = <Datum*> new NodeCollectionIteratorDatum(deref(<NodeCollectionIteratorDatum*> (<SLIDatum> obj).thisptr))
        elif (<SLIDatum> obj).dtype == SLI_TYPE_CONNECTION.decode():
            ret = <Datum*> new ConnectionDatum(deref(<ConnectionDatum*> (<SLIDatum> obj).thisptr))
        else:
            raise NESTErrors.PyNESTError("unknown SLI datum type: {0}".format((<SLIDatum> obj).dtype))
    elif isConnectionGenerator(<PyObject*> obj):
        ret = unpackConnectionGeneratorDatum(<PyObject*> obj)
        if ret is NULL:
            raise NESTErrors.PyNESTError("failed to unpack passed connection generator object")
    elif isinstance(obj, nest.CollocatedSynapses):
        ret = python_object_to_datum(obj.syn_specs)
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
                raise NESTError.PyNESTError("only vectors of integers or floats are supported")

        if ret is NULL:
            try:
                if isinstance( obj._datum, SLIDatum ) or isinstance( obj._datum[0], SLIDatum):
                    ret = python_object_to_datum( obj._datum )
            except:
                pass

        if ret is not NULL:
            return ret
        else:
            raise NESTErrors.PyNESTError("unknown Python type: {0}".format(type(obj)))

    if ret is NULL:
        raise NESTErrors.PyNESTError("conversion resulted in a null pointer")

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
        raise NESTErrors.PyNESTError("unsupported specialization: {0}".format(vector_value_t))

    n = len(buff)

    vector_ptr.reserve(n)

    for i in range(n):
        vector_ptr.push_back(<vector_value_t> buff[i])

    return <Datum*> dat


cdef inline object sli_datum_to_object(Datum* dat):

    if dat is NULL:
        raise NESTErrors.PyNESTError("datum is a null pointer")

    cdef string obj_str
    cdef object ret = None
    cdef ignore_none = False

    cdef string datum_type = dat.gettypename().toString()

    if datum_type == SLI_TYPE_BOOL:
        ret = (<BoolDatum*> dat).get()
    elif datum_type == SLI_TYPE_INTEGER:
        ret = (<IntegerDatum*> dat).get()
    elif datum_type == SLI_TYPE_DOUBLE:
        ret = (<DoubleDatum*> dat).get()
    elif datum_type == SLI_TYPE_STRING:
        ret = (<string> deref_str(<StringDatum*> dat)).decode('utf-8')
    elif datum_type == SLI_TYPE_LITERAL:
        obj_str = (<LiteralDatum*> dat).toString()
        ret = obj_str.decode()
        if ret == 'None':
            ret = None
            ignore_none = True
    elif datum_type == SLI_TYPE_ARRAY:
        ret = sli_array_to_object(<ArrayDatum*> dat)
    elif datum_type == SLI_TYPE_DICTIONARY:
        ret = sli_dict_to_object(<DictionaryDatum*> dat)
    elif datum_type == SLI_TYPE_CONNECTION:
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new ConnectionDatum(deref(<ConnectionDatum*> dat)), SLI_TYPE_CONNECTION.decode())
        ret = nest.SynapseCollection(datum)
    elif datum_type == SLI_TYPE_VECTOR_INT:
        ret = sli_vector_to_object[sli_vector_int_ptr_t, long](<IntVectorDatum*> dat)
    elif datum_type == SLI_TYPE_VECTOR_DOUBLE:
        ret = sli_vector_to_object[sli_vector_double_ptr_t, double](<DoubleVectorDatum*> dat)
    elif datum_type == SLI_TYPE_MASK:
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new MaskDatum(deref(<MaskDatum*> dat)), SLI_TYPE_MASK.decode())
        ret = nest.Mask(datum)
    elif datum_type == SLI_TYPE_PARAMETER:
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new ParameterDatum(deref(<ParameterDatum*> dat)), SLI_TYPE_PARAMETER.decode())
        ret = nest.Parameter(datum)
    elif datum_type == SLI_TYPE_NODECOLLECTION:
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new NodeCollectionDatum(deref(<NodeCollectionDatum*> dat)), SLI_TYPE_NODECOLLECTION.decode())
        ret = nest.NodeCollection(datum)
    elif datum_type == SLI_TYPE_NODECOLLECTIONITERATOR:
        ret = SLIDatum()
        (<SLIDatum> ret)._set_datum(<Datum*> new NodeCollectionIteratorDatum(deref(<NodeCollectionIteratorDatum*> dat)), SLI_TYPE_NODECOLLECTIONITERATOR.decode())
    else:
        raise NESTErrors.PyNESTError("unknown SLI type: {0}".format(datum_type.decode()))

    if ret is None and not ignore_none:
        raise NESTErrors.PyNESTError("conversion resulted in a None object")

    return ret

cdef inline object sli_array_to_object(ArrayDatum* dat):

    # the size of dat has to be explicitly cast to int to avoid
    # compiler warnings (#1318) during cythonization
    cdef tmp = [None] * int(dat.size())

    # i and n have to be cast to size_t (unsigned long int) to avoid
    # compiler warnings (#1318) in the for loop below
    cdef size_t i, n
    cdef Token* tok = dat.begin()

    n = len(tmp)
    if not n:
        return ()

    if tok.datum().gettypename().toString() == SLI_TYPE_CONNECTION:
        for i in range(n):
            datum = SLIDatum()
            (<SLIDatum> datum)._set_datum(<Datum*> new ConnectionDatum(deref(<ConnectionDatum*> tok.datum())), SLI_TYPE_CONNECTION.decode())
            tmp[i] = datum
            # Increment
            inc(tok)
        return nest.SynapseCollection(tmp)
    else:
        for i in range(n):
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
        raise NESTErrors.PyNESTError("unsupported specialization")

    memcpy(array_data, &vector_ptr.front(), vector_ptr.size() * sizeof(vector_value_t))

    if HAVE_NUMPY:
        if vector_ptr.size() > 0:
            return numpy.frombuffer(arr, dtype=ret_dtype)
        else:
            # Compatibility with NumPy < 1.7.0
            return numpy.array([], dtype=ret_dtype)
    else:
        return arr
