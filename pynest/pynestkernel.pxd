# -*- coding: utf-8 -*-
#
# pynestkernel.pxd
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

from libcpp cimport bool as cbool

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.utility cimport pair

from cpython.ref cimport PyObject

cdef extern from "name.h":
    cppclass Name:
        string toString() except +

cdef extern from "datum.h":
    cppclass Datum:
        Name gettypename() except +

cdef extern from "token.h":
    cppclass Token:
        Datum* datum() except +

cdef extern from "namedatum.h":
    cppclass LiteralDatum:
        LiteralDatum(const string&) except +
        string toString() except +

cdef extern from "booldatum.h":
    cppclass BoolDatum:
        BoolDatum(cbool) except +
        bint get() except +

cdef extern from "integerdatum.h":
    cppclass IntegerDatum:
        IntegerDatum(long) except +
        long get() except +

cdef extern from "doubledatum.h":
    cppclass DoubleDatum:
        DoubleDatum(double) except +
        double get() except +

cdef extern from "stringdatum.h":
    cppclass StringDatum:
        StringDatum(const string&) except +

cdef extern from "mask.h" namespace "nest":
    cppclass MaskDatum:
        MaskDatum(const MaskDatum&)

cdef extern from "parameter.h":
    cppclass ParameterDatum:
        ParameterDatum(const ParameterDatum&)

cdef extern from "node_collection.h" namespace "nest":
    cppclass NodeCollectionPTR:
        NodeCollectionPTR()

cdef extern from "node_collection.h":
    cppclass NodeCollectionDatum:
        NodeCollectionDatum(const NodeCollectionDatum&)

    cppclass NodeCollectionIteratorDatum:
        NodeCollectionIteratorDatum(const NodeCollectionIteratorDatum&)

cdef extern from "connection_id.h" namespace "nest":
    cppclass ConnectionID:
        ConnectionID(long, long, long, long) except +
        ConnectionID(long, long, long, long, long) except +

cdef extern from "nest_datums.h":
    cppclass ConnectionDatum:
        ConnectionDatum(const ConnectionID&) except +
        ConnectionDatum(const ConnectionDatum&) except +
        long get_source_node_id()
        long get_target_node_id()
        long get_target_thread()
        long get_synapse_model_id()
        long get_port()

    cppclass NodeCollectionIteratorDatum:
        NodeCollectionIteratorDatum(const NodeCollectionIteratorDatum&)


cdef extern from "arraydatum.h":
    cppclass ArrayDatum:
        ArrayDatum() except +
        size_t size()
        void reserve(size_t) except +
        void push_back(Datum*) except +
        Token* begin()
        Token* end()

    cppclass IntVectorDatum:
        IntVectorDatum(vector[long]*) except +

    cppclass DoubleVectorDatum:
        DoubleVectorDatum(vector[double]*) except +

cdef extern from "dict.h":
    cppclass Dictionary:
        Dictionary() except +

cdef extern from "dictdatum.h":
    cppclass TokenMap:
        cppclass const_iterator:
            const_iterator operator++()
            bint operator!=(const_iterator)
            Name first
            Token second

    cppclass DictionaryDatum:
        DictionaryDatum(Dictionary *) except +
        void insert(const string&, Datum*) except +
        TokenMap.const_iterator begin()
        TokenMap.const_iterator end()

cdef extern from "tokenstack.h":
    cppclass TokenStack:
        void push(Datum*) except +
        void pop()
        cbool empty()

        # Supposed to be used only through the addr_tok macro
        Token* top()

cdef extern from "dictionary.h" namespace "boost":
    cppclass any:
        any()
        any& operator=[T](T&)
    T any_cast[T](any& operand)

cdef extern from "dictionary.h":
    cppclass dictionary:
        dictionary()
        # ctypedef key_type
        # ctypedef mapped_type
        any& operator[](const string&)
        cppclass const_iterator:
            pair[string, any]& operator*()
            const_iterator operator++()
            bint operator==(const const_iterator&)
            bint operator!=(const const_iterator&)
        const_iterator begin()
        const_iterator end()
    string debug_type(const any&)
    cbool is_int(const any&)
    cbool is_long(const any&)
    cbool is_size_t(const any&)
    cbool is_double(const any&)
    cbool is_bool(const any&)
    cbool is_string(const any&)
    cbool is_int_vector(const any&)
    cbool is_double_vector(const any&)
    cbool is_string_vector(const any&)
    cbool is_any_vector(const any&)
    cbool is_dict(const any&)

cdef extern from "mpi_manager.h" namespace "nest":
    cppclass MPIManager:
        void mpi_finalize( int exitcode ) except +

cdef extern from "kernel_manager.h" namespace "nest":
    KernelManager& kernel()
    cppclass KernelManager:
        KernelManager()
        void destroy_kernel_manager()
        MPIManager mpi_manager

cdef extern from "nest.h" namespace "nest":
    void init_nest( int* argc, char** argv[] )
    void reset_kernel()
    NodeCollectionPTR create( const string model_name, const long n ) except +

    NodeCollectionPTR make_nodecollection( const vector[size_t] node_ids ) except +

    NodeCollectionPTR slice_nc( const NodeCollectionPTR nc, long start, long stop, long step ) except +
    void connect(NodeCollectionPTR sources,
                 NodeCollectionPTR targets,
                 const dictionary& connectivity,
                 const vector[dictionary]& synapse_params ) except +
    string pprint_to_string( NodeCollectionPTR nc )
    size_t nc_size( NodeCollectionPTR nc )
    dictionary get_kernel_status()
    void set_kernel_status( const dictionary& ) except +
    dictionary get_nc_status( NodeCollectionPTR nc )
    void set_nc_status( NodeCollectionPTR nc, dictionary& params ) except +
    void simulate( const double& t )

cdef extern from "pynestkernel_aux.h":
    CYTHON_isConnectionGenerator( x )
    CYTHON_unpackConnectionGeneratorDatum( PyObject* obj )
    CYTHON_DEREF( x )
    CYTHON_ADDR( x )

# TODO-PYNEST-NG: Move these from neststartup to mpimanager
# cdef extern from "neststartup.h":
#     cbool nest_has_mpi4py()
#     void c_set_communicator "set_communicator" (object) with gil

cdef extern from "nest.h" namespace "nest":
    Datum* node_collection_array_index(const Datum* node_collection, const long* array, unsigned long n) except +
    Datum* node_collection_array_index(const Datum* node_collection, const cbool* array, unsigned long n) except +
    void connect_arrays( long* sources, long* targets, double* weights, double* delays, vector[string]& p_keys, double* p_values, size_t n, string syn_model ) except +

cdef extern from *:

    # Real support for CSA has to be implemented below the Cython level,
    # or else we won't be able to distribute pre-generated kernels
    #
    cbool isConnectionGenerator "CYTHON_isConnectionGenerator" (PyObject*)
    Datum* unpackConnectionGeneratorDatum "CYTHON_unpackConnectionGeneratorDatum" (PyObject*) except +

    Token* addr_tok "CYTHON_ADDR" (Token*)

    StringDatum* deref_str "CYTHON_DEREF" (StringDatum*)
    DictionaryDatum* deref_dict "CYTHON_DEREF" (DictionaryDatum*)

    TokenMap.const_iterator deref_tmap "CYTHON_DEREF" (TokenMap.const_iterator)

    vector[long]* deref_ivector "&*CYTHON_DEREF" (IntVectorDatum*)
    vector[double]* deref_dvector "&*CYTHON_DEREF" (DoubleVectorDatum*)


ctypedef fused vector_value_t:
    long
    double

ctypedef IntVectorDatum* sli_vector_int_ptr_t
ctypedef DoubleVectorDatum* sli_vector_double_ptr_t

ctypedef fused sli_vector_ptr_t:
    sli_vector_int_ptr_t
    sli_vector_double_ptr_t

ctypedef int [:] buffer_int_1d_t
ctypedef long [:] buffer_long_1d_t

ctypedef float [:] buffer_float_1d_t
ctypedef double [:] buffer_double_1d_t

ctypedef fused numeric_buffer_t:
    object

    buffer_int_1d_t
    buffer_long_1d_t

    buffer_float_1d_t
    buffer_double_1d_t
