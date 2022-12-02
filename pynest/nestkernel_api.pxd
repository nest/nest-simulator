# -*- coding: utf-8 -*-
#
# ll_api.pxd
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
from libcpp.deque cimport deque
from libcpp.utility cimport pair
from libcpp.memory cimport shared_ptr


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

cdef extern from "connection_id.h" namespace "nest":
    cppclass ConnectionID:
        ConnectionID()

cdef extern from "node_collection.h" namespace "nest":
    cppclass NodeCollectionPTR:
        NodeCollectionPTR()

    NodeCollectionPTR operator+(NodeCollectionPTR, NodeCollectionPTR) except +

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
    ParameterPTR multiply_parameter(const ParameterPTR first, const ParameterPTR second) except +
    ParameterPTR divide_parameter(const ParameterPTR first, const ParameterPTR second) except +
    ParameterPTR add_parameter(const ParameterPTR first, const ParameterPTR second) except +
    ParameterPTR subtract_parameter(const ParameterPTR first, const ParameterPTR second) except +
    ParameterPTR compare_parameter(const ParameterPTR first, const ParameterPTR second, const dictionary& d) except +
    ParameterPTR conditional_parameter(const ParameterPTR condition, const ParameterPTR if_true, const ParameterPTR if_false) except +
    ParameterPTR min_parameter(const ParameterPTR parameter, const double other) except +
    ParameterPTR max_parameter(const ParameterPTR parameter, const double other) except +
    ParameterPTR redraw_parameter(const ParameterPTR parameter, const double min, const double max) except +
    ParameterPTR exp_parameter(const ParameterPTR parameter) except +
    ParameterPTR sin_parameter(const ParameterPTR parameter) except +
    ParameterPTR cos_parameter(const ParameterPTR parameter) except +
    ParameterPTR pow_parameter(const ParameterPTR parameter, const double exponent) except +

    ParameterPTR dimension_parameter(const ParameterPTR x, const ParameterPTR y) except +
    ParameterPTR dimension_parameter(const ParameterPTR x, const ParameterPTR y, const ParameterPTR z) except +

cdef extern from "nest.h" namespace "nest":
    void init_nest( int* argc, char** argv[] )
    void reset_kernel()
    NodeCollectionPTR create( const string model_name, const long n ) except +
    NodeCollectionPTR create_spatial( const dictionary& ) except +

    NodeCollectionPTR make_nodecollection( const vector[size_t] node_ids ) except +

    cbool equal( const NodeCollectionPTR lhs, const NodeCollectionPTR rhs ) except +
    cbool contains( const NodeCollectionPTR nc, const size_t node_id ) except +
    long find( const NodeCollectionPTR nc, size_t node_id ) except +
    dictionary get_metadata( const NodeCollectionPTR nc ) except +

    NodeCollectionPTR slice_nc( const NodeCollectionPTR nc, long start, long stop, long step ) except +
    void connect(NodeCollectionPTR sources,
                 NodeCollectionPTR targets,
                 const dictionary& connectivity,
                 const vector[dictionary]& synapse_params ) except +
    void disconnect(NodeCollectionPTR sources,
                 NodeCollectionPTR targets,
                 const dictionary& connectivity,
                 const dictionary& synapse_params) except +
    int get_rank() except +
    int get_num_mpi_processes() except +
    string print_nodes_to_string()
    string pprint_to_string( NodeCollectionPTR nc ) except +
    size_t nc_size( NodeCollectionPTR nc ) except +
    dictionary get_kernel_status() except +
    dictionary get_model_defaults( const string& ) except +
    void set_model_defaults( const string&, const dictionary& ) except +
    NodeCollectionPTR get_nodes( const dictionary& params, const cbool local_only ) except +
    deque[ConnectionID] get_connections( const dictionary& dict ) except +
    void set_kernel_status( const dictionary& ) except +
    dictionary get_nc_status( NodeCollectionPTR nc ) except +
    void set_nc_status( NodeCollectionPTR nc, vector[dictionary]& params ) except +
    vector[dictionary] get_connection_status(const deque[ConnectionID]&) except +
    void set_connection_status(const deque[ConnectionID]&, const dictionary&) except +
    void set_connection_status(const deque[ConnectionID]&, const vector[dictionary]&) except +
    void simulate( const double& t ) except +
    void prepare() except +
    void run( const double& t ) except +
    void cleanup() except +
    void copy_model( const string&, const string&, const dictionary& ) except +
    ParameterPTR create_parameter( const dictionary& param_dict ) except +
    double get_value( const ParameterPTR param ) except +
    cbool is_spatial( const ParameterPTR param ) except +
    NodeCollectionPTR node_collection_array_index(NodeCollectionPTR node_collection, const long* array, unsigned long n) except +
    NodeCollectionPTR node_collection_array_index(NodeCollectionPTR node_collection, const cbool* array, unsigned long n) except +
    void connect_arrays( long* sources, long* targets, double* weights, double* delays, vector[string]& p_keys, double* p_values, size_t n, string syn_model ) except +
    vector[double] apply( const ParameterPTR param, const NodeCollectionPTR nc ) except +
    vector[double] apply( const ParameterPTR param, const dictionary& positions ) except +


# PYNEST-NG: Move these global functions to nest.h?
cdef extern from "spatial.h" namespace "nest":
    vector[vector[double]] get_position( NodeCollectionPTR layer_nc ) except +
    vector[double] distance( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc ) except +
    vector[double] distance( NodeCollectionPTR layer_nc, const vector[vector[double]]& point ) except +
    vector[double] distance( const vector[ConnectionID]& conns ) except +
    vector[vector[double]] displacement( NodeCollectionPTR layer_to_nc, NodeCollectionPTR layer_from_nc ) except +
    vector[vector[double]] displacement( NodeCollectionPTR layer_nc, const vector[vector[double]]& point ) except +

    void connect_layers( NodeCollectionPTR source_nc, NodeCollectionPTR target_nc, const dictionary& dict ) except +
