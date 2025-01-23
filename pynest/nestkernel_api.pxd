# -*- coding: utf-8 -*-
#
# nestkernel_api.pxd
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
from libcpp.deque cimport deque
from libcpp.map cimport map as std_map
from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.utility cimport pair
from libcpp.vector cimport vector


cdef extern from "Python.h" nogil:
    ctypedef struct PyObject


cdef extern from "nestkernel_exceptions.h":
    cdef PyObject* nest_error_module
    cdef void create_exceptions()
    cdef void custom_exception_handler()


cdef extern from "dictionary.h" namespace "boost":
    cppclass any:
        any()
        any& operator=[T](T&)
    T any_cast[T](any& operand)


cdef extern from "dictionary.h":
    cppclass dictionary:
        dictionary()
        any& operator[](const string&)
        cppclass const_iterator:
            pair[string, any]& operator*()
            const_iterator operator++()
            bint operator==(const const_iterator&)
            bint operator!=(const const_iterator&)
        const_iterator begin()
        const_iterator end()
        cbool known(const string&)
    string debug_type(const any&)
    string debug_dict_types(const dictionary&)
    cbool is_type[T](const any&)


cdef extern from "logging.h" namespace "nest":
    cpdef enum severity_t:
        M_ALL,
        M_DEBUG,
        M_STATUS,
        M_INFO,
        M_PROGRESS,
        M_DEPRECATED,
        M_WARNING,
        M_ERROR,
        M_FATAL,
        M_QUIET


cdef extern from "connection_id.h" namespace "nest":
    cppclass ConnectionID:
        ConnectionID()


cdef extern from "node_collection.h" namespace "nest":
    cppclass NodeCollectionPTR:
        NodeCollectionPTR()

    NodeCollectionPTR operator+(NodeCollectionPTR, NodeCollectionPTR) except +custom_exception_handler


cdef extern from "node_collection.h":
    cppclass NodeCollectionDatum:
        NodeCollectionDatum(const NodeCollectionDatum&)

    cppclass NodeCollectionIteratorDatum:
        NodeCollectionIteratorDatum(const NodeCollectionIteratorDatum&)


cdef extern from "parameter.h" namespace "nest":
    cppclass ParameterPTR:
        ParameterPTR()
    cppclass Parameter:
        Parameter()
    ParameterPTR multiply_parameter(const ParameterPTR first, const ParameterPTR second) except +custom_exception_handler
    ParameterPTR divide_parameter(const ParameterPTR first, const ParameterPTR second) except +custom_exception_handler
    ParameterPTR add_parameter(const ParameterPTR first, const ParameterPTR second) except +custom_exception_handler
    ParameterPTR subtract_parameter(const ParameterPTR first, const ParameterPTR second) except +custom_exception_handler
    ParameterPTR compare_parameter(const ParameterPTR first, const ParameterPTR second, const dictionary& d) except +custom_exception_handler
    ParameterPTR conditional_parameter(const ParameterPTR condition, const ParameterPTR if_true, const ParameterPTR if_false) except +custom_exception_handler
    ParameterPTR min_parameter(const ParameterPTR parameter, const double other) except +custom_exception_handler
    ParameterPTR max_parameter(const ParameterPTR parameter, const double other) except +custom_exception_handler
    ParameterPTR redraw_parameter(const ParameterPTR parameter, const double min, const double max) except +custom_exception_handler
    ParameterPTR exp_parameter(const ParameterPTR parameter) except +custom_exception_handler
    ParameterPTR sin_parameter(const ParameterPTR parameter) except +custom_exception_handler
    ParameterPTR cos_parameter(const ParameterPTR parameter) except +custom_exception_handler
    ParameterPTR pow_parameter(const ParameterPTR parameter, const double exponent) except +custom_exception_handler

    ParameterPTR dimension_parameter(const ParameterPTR x, const ParameterPTR y) except +custom_exception_handler
    ParameterPTR dimension_parameter(const ParameterPTR x, const ParameterPTR y, const ParameterPTR z) except +custom_exception_handler


cdef extern from "mask.h" namespace "nest":
    cppclass MaskPTR:
        MaskPTR()


cdef extern from "nest.h" namespace "nest":
    void init_nest( int* argc, char** argv[] )
    void reset_kernel()

    severity_t get_verbosity()
    void set_verbosity( severity_t )

    void enable_structural_plasticity() except +custom_exception_handler
    void disable_structural_plasticity() except +custom_exception_handler

    NodeCollectionPTR create( const string& model_name, const long n ) except +custom_exception_handler
    NodeCollectionPTR create_spatial( const dictionary& ) except +custom_exception_handler

    NodeCollectionPTR make_nodecollection( const vector[size_t]& node_ids ) except +custom_exception_handler

    cbool equal( const NodeCollectionPTR lhs, const NodeCollectionPTR rhs ) except +custom_exception_handler
    cbool contains( const NodeCollectionPTR nc, const size_t node_id ) except +custom_exception_handler
    long find( const NodeCollectionPTR nc, size_t node_id ) except +custom_exception_handler
    dictionary get_metadata( const NodeCollectionPTR nc ) except +custom_exception_handler

    NodeCollectionPTR slice_nc( const NodeCollectionPTR nc, long start, long stop, long step ) except +custom_exception_handler
    void connect(NodeCollectionPTR sources,
                 NodeCollectionPTR targets,
                 const dictionary& connectivity,
                 const vector[dictionary]& synapse_params ) except +custom_exception_handler
    void connect_tripartite(NodeCollectionPTR sources,
                            NodeCollectionPTR targets,
			    NodeCollectionPTR third,
                            const dictionary& connectivity,
                            const dictionary& third_connectivity,
                            const std_map[string, vector[dictionary]]& synapse_params ) except +custom_exception_handler
    void connect_sonata( const dictionary& graph_specs, const long hyperslab_size ) except +custom_exception_handler
    void disconnect(NodeCollectionPTR sources,
                 NodeCollectionPTR targets,
                 const dictionary& connectivity,
                 const dictionary& synapse_params) except +custom_exception_handler
    void disconnect( const deque[ConnectionID]& conns ) except +custom_exception_handler
    string print_nodes_to_string()
    string pprint_to_string( NodeCollectionPTR nc ) except +custom_exception_handler
    size_t nc_size( NodeCollectionPTR nc ) except +custom_exception_handler
    dictionary get_kernel_status() except +custom_exception_handler
    dictionary get_model_defaults( const string& ) except +custom_exception_handler
    void set_model_defaults( const string&, const dictionary& ) except +custom_exception_handler
    NodeCollectionPTR get_nodes( const dictionary& params, const cbool local_only ) except +custom_exception_handler
    deque[ConnectionID] get_connections( const dictionary& dict ) except +custom_exception_handler
    void set_kernel_status( const dictionary& ) except +custom_exception_handler
    dictionary get_nc_status( NodeCollectionPTR nc ) except +custom_exception_handler
    void set_nc_status( NodeCollectionPTR nc, vector[dictionary]& params ) except +custom_exception_handler
    vector[dictionary] get_connection_status(const deque[ConnectionID]&) except +custom_exception_handler
    void set_connection_status(const deque[ConnectionID]&, const dictionary&) except +custom_exception_handler
    void set_connection_status(const deque[ConnectionID]&, const vector[dictionary]&) except +custom_exception_handler
    void simulate( const double& t ) except +custom_exception_handler
    void prepare() except +custom_exception_handler
    void run( const double& t ) except +custom_exception_handler
    void cleanup() except +custom_exception_handler
    void copy_model( const string&, const string&, const dictionary& ) except +custom_exception_handler
    ParameterPTR create_parameter( const dictionary& param_dict ) except +custom_exception_handler
    double get_value( const ParameterPTR param ) except +custom_exception_handler
    cbool is_spatial( const ParameterPTR param ) except +custom_exception_handler
    NodeCollectionPTR node_collection_array_index(NodeCollectionPTR node_collection, const long* array, unsigned long n) except +custom_exception_handler
    NodeCollectionPTR node_collection_array_index(NodeCollectionPTR node_collection, const cbool* array, unsigned long n) except +custom_exception_handler

    vector[size_t] node_collection_to_array( NodeCollectionPTR node_collection, const string& selection ) except +custom_exception_handler

    void connect_arrays( long* sources, long* targets, double* weights, double* delays, vector[string]& p_keys, double* p_values, size_t n, string syn_model ) except +custom_exception_handler
    vector[double] apply( const ParameterPTR param, const NodeCollectionPTR nc ) except +custom_exception_handler
    vector[double] apply( const ParameterPTR param, const dictionary& positions ) except +custom_exception_handler


# PYNEST-NG: Move these global functions to nest.h?
cdef extern from "spatial.h" namespace "nest":
    vector[vector[double]] get_position( NodeCollectionPTR layer_nc ) except +custom_exception_handler
    vector[double] distance( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc ) except +custom_exception_handler
    vector[double] distance( NodeCollectionPTR layer_nc, const vector[vector[double]]& point ) except +custom_exception_handler
    vector[double] distance( const vector[ConnectionID]& conns ) except +custom_exception_handler
    vector[vector[double]] displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc ) except +custom_exception_handler
    vector[vector[double]] displacement( NodeCollectionPTR layer_nc, const vector[vector[double]]& point ) except +custom_exception_handler

    void connect_layers( NodeCollectionPTR source_nc, NodeCollectionPTR target_nc, const dictionary& dict ) except +custom_exception_handler
    MaskPTR create_mask( const dictionary& mask_dict ) except +custom_exception_handler
    NodeCollectionPTR select_nodes_by_mask( const NodeCollectionPTR layer_nc, const vector[double]& anchor, const MaskPTR mask ) except +custom_exception_handler
    cbool inside(const vector[double]& point, const MaskPTR mask ) except +custom_exception_handler

    void dump_layer_nodes(const NodeCollectionPTR layer_nc, const string& filename)
    void dump_layer_connections(const NodeCollectionPTR source_layer,
                                const NodeCollectionPTR target_layer,
                                const string& synapse_model,
                                const string& filename) except +custom_exception_handler
