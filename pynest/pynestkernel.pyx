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

from nest.lib.hl_api_types import GIDCollection, Connectome, Mask, Parameter


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
cdef string SLI_TYPE_GIDCOLLECTION = b"gidcollectiontype"
cdef string SLI_TYPE_GIDCOLLECTIONITERATOR = b"gidcollectioniteratortype"


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

    def __dealloc__(self):

        kernel().mpi_manager.mpi_finalize( 0 );
        kernel().destroy_kernel_manager();

    def init(self, argv):

        cdef int argc = <int> len(argv)
        if argc <= 0:
            raise NESTError("argv can't be empty")

        # Create c-style argv arguments from sys.argv
        cdef char** argv_chars = <char**> malloc((argc+1) * sizeof(char*))
        if argv_chars is NULL:
            raise NESTError("couldn't allocate argv_char")
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

            init_nest(&argc, &argv_chars)

            # PyNEST-NG
            # nest::kernel().model_manager.get_modeldict()
            # nest::kernel().model_manager.get_synapsedict()
            # nest::kernel().connection_manager.get_connruledict()
            # nest::kernel().sp_manager.get_growthcurvedict()

            # If using MPI, argv might now have changed, so rebuild it
            del argv[:]
            # Convert back from utf8 char* to utf8 str in both python2 & 3
            argv.extend(str(argvi.decode()) for argvi in argv_chars[:argc])
        finally:
            free(argv_chars)

        return True


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
        elif (<SLIDatum> obj).dtype == SLI_TYPE_GIDCOLLECTION.decode():
            ret = <Datum*> new GIDCollectionDatum(deref(<GIDCollectionDatum*> (<SLIDatum> obj).thisptr))
        elif (<SLIDatum> obj).dtype == SLI_TYPE_GIDCOLLECTIONITERATOR.decode():
            ret = <Datum*> new GIDCollectionIteratorDatum(deref(<GIDCollectionIteratorDatum*> (<SLIDatum> obj).thisptr))
        elif (<SLIDatum> obj).dtype == SLI_TYPE_CONNECTION.decode():
            ret = <Datum*> new ConnectionDatum(deref(<ConnectionDatum*> (<SLIDatum> obj).thisptr))
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

        if ret is NULL:
            try:
                if isinstance( obj._datum, SLIDatum ) or isinstance( obj._datum[0], SLIDatum):
                    ret = python_object_to_datum( obj._datum )
            except:
                pass

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
        ret = (<string> deref_str(<StringDatum*> dat)).decode('utf-8')
    elif datum_type == SLI_TYPE_LITERAL:
        obj_str = (<LiteralDatum*> dat).toString()
        ret = SLILiteral(obj_str.decode())
    elif datum_type == SLI_TYPE_ARRAY:
        ret = sli_array_to_object(<ArrayDatum*> dat)
    elif datum_type == SLI_TYPE_DICTIONARY:
        ret = sli_dict_to_object(<DictionaryDatum*> dat)
    elif datum_type == SLI_TYPE_CONNECTION:
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new ConnectionDatum(deref(<ConnectionDatum*> dat)), SLI_TYPE_CONNECTION.decode())
        ret = Connectome(datum)
    elif datum_type == SLI_TYPE_VECTOR_INT:
        ret = sli_vector_to_object[sli_vector_int_ptr_t, long](<IntVectorDatum*> dat)
    elif datum_type == SLI_TYPE_VECTOR_DOUBLE:
        ret = sli_vector_to_object[sli_vector_double_ptr_t, double](<DoubleVectorDatum*> dat)
    elif datum_type == SLI_TYPE_MASK:
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new MaskDatum(deref(<MaskDatum*> dat)), SLI_TYPE_MASK.decode())
        ret = Mask(datum)
    elif datum_type == SLI_TYPE_PARAMETER:
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new ParameterDatum(deref(<ParameterDatum*> dat)), SLI_TYPE_PARAMETER.decode())
        ret = Parameter(datum)
    elif datum_type == SLI_TYPE_GIDCOLLECTION:        
        datum = SLIDatum()
        (<SLIDatum> datum)._set_datum(<Datum*> new GIDCollectionDatum(deref(<GIDCollectionDatum*> dat)), SLI_TYPE_GIDCOLLECTION.decode())
        ret = GIDCollection(datum)
    elif datum_type == SLI_TYPE_GIDCOLLECTIONITERATOR:
        ret = SLIDatum()
        (<SLIDatum> ret)._set_datum(<Datum*> new GIDCollectionIteratorDatum(deref(<GIDCollectionIteratorDatum*> dat)), SLI_TYPE_GIDCOLLECTIONITERATOR.decode())
    else:
        raise NESTError("unknown SLI type: {0}".format(datum_type.decode()))

    if ret is None:
        raise NESTError("conversion resulted in a None object")

    return ret

cdef inline object sli_array_to_object(ArrayDatum* dat):
    cdef tmp = [None] * dat.size()

    cdef size_t i
    cdef Token* tok = dat.begin()
    
    if not len(tmp):
        return ()

    if tok.datum().gettypename().toString() == SLI_TYPE_CONNECTION:
        for i in range(len(tmp)):
            datum = SLIDatum()
            (<SLIDatum> datum)._set_datum(<Datum*> new ConnectionDatum(deref(<ConnectionDatum*> tok.datum())), SLI_TYPE_CONNECTION.decode())
            tmp[i] = datum
            # Increment
            inc(tok)
        return Connectome(tmp)
    else:
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





################################################################################
####                                                                        ####
####                              PyNEST HL API                             ####
####                                                                        ####
################################################################################

from nest.lib.hl_api_helper import model_deprecation_warning, warnings

def Create(string model, long n=1, params=None):
    """Create n instances of type model.

    Parameters
    ----------
    model : str
        Name of the model to create
    n : int, optional
        Number of instances to create
    params : TYPE, optional
        Parameters for the new nodes. A single dictionary or a list of
        dictionaries with size n. If omitted, the model's defaults are used.

    Returns
    -------
    GIDCollection:
        Object representing global IDs of created nodes
    """

    model_deprecation_warning(model)

    cdef GIDCollectionPTR gids = create(model, n)
    cdef GIDCollectionDatum* gids_ = new GIDCollectionDatum(gids)

    datum = SLIDatum()
    (<SLIDatum> datum)._set_datum(<Datum*> new GIDCollectionDatum(gids), SLI_TYPE_GIDCOLLECTION.decode())

    return datum
#
#    if isinstance(params, dict):
#        // same parameters for all nodes
#    else:
#        for i, node in enumerate(gids):
#            SetStatus(node, params[i])
#
    
    ### gids.set(params)
###    
###    if isinstance(params, dict):
###        cmd = "/%s 3 1 roll exch Create" % model
#######        sps(params)
###    else:
###        cmd = "/%s exch Create" % model
###
#######    sps(n)
#######    sr(cmd)
###
#######    gids = spp()
###
###    if params is not None and not isinstance(params, dict):
###        try:
#######            SetStatus(gids, params)
###            pass
###        except:
###            warnings.warn(
###                "SetStatus() call failed, but nodes have already been " +
###                "created! The GIDs of the new nodes are: {0}.".format(gids))
###            raise
#    datum = SLIDatum()
#    (<SLIDatum> datum)._set_datum(<Datum*> new GIDCollectionDatum(deref(<GIDCollectionPTR> gids)), SLI_TYPE_GIDCOLLECTION.decode())
#   
#    return datum
