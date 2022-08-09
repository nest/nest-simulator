# -*- coding: utf-8 -*-
#
# ll_api.pyx
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

# import cython

# from libc.stdlib cimport malloc, free
# from libc.string cimport memcpy

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.deque cimport deque
from libcpp.memory cimport shared_ptr

from cython.operator cimport dereference as deref
from cython.operator cimport preincrement as inc

import nest
from nest.lib.hl_api_exceptions import NESTErrors

import numpy


cdef class NodeCollectionObject:

    cdef NodeCollectionPTR thisptr

    def __repr__(self):
        return "<NodeCollectionObject>"

    cdef _set_nc(self, NodeCollectionPTR nc):
        self.thisptr = nc


cdef class ConnectionObject:

    cdef ConnectionID thisobj

    def __repr__(self):
        return "<ConnectionIDObject>"

    cdef _set_connection_id(self, ConnectionID conn_id):
        self.thisobj = conn_id


cdef class ParameterObject:

    cdef shared_ptr[Parameter] thisptr

    def __repr__(self):
        return "<ParameterObject>"

    cdef _set_parameter(self, shared_ptr[Parameter] parameter_ptr):
        self.thisptr = parameter_ptr


cdef object any_vector_to_list(vector[any] cvec):
    cdef tmp = []
    cdef vector[any].iterator it = cvec.begin()
    while it != cvec.end():
        tmp.append(any_to_pyobj(deref(it)))
        inc(it)
    return tmp


cdef object any_to_pyobj(any operand):
    if is_type[int](operand):
        return any_cast[int](operand)
    if is_type[uint](operand):
        return any_cast[uint](operand)
    if is_type[long](operand):
        return any_cast[long](operand)
    if is_type[size_t](operand):
        return any_cast[size_t](operand)
    if is_type[double](operand):
        return any_cast[double](operand)
    if is_type[cbool](operand):
        return any_cast[cbool](operand)
    if is_type[string](operand):
        return any_cast[string](operand).decode('utf8')
    if is_type[vector[int]](operand):
        return any_cast[vector[int]](operand)
    if is_type[vector[long]](operand):
        return any_cast[vector[long]](operand)
    if is_type[vector[double]](operand):
        return any_cast[vector[double]](operand)
    if is_type[vector[vector[double]]](operand):
        return any_cast[vector[vector[double]]](operand)
    if is_type[vector[string]](operand):
        return any_cast[vector[string]](operand)
    if is_type[vector[any]](operand):
        return tuple(any_vector_to_list(any_cast[vector[any]](operand)))
    if is_type[dictionary](operand):
        return dictionary_to_pydict(any_cast[dictionary](operand))

cdef object dictionary_to_pydict(dictionary cdict):
    cdef tmp = {}

    cdef dictionary.const_iterator it = cdict.begin()
    while it != cdict.end():
        tmp[deref(it).first.decode('utf8')] = any_to_pyobj(deref(it).second)
        if tmp[deref(it).first.decode('utf8')] is None:
            # If we end up here, the value in the dictionary is of a type that any_to_pyobj() cannot handle.
            raise RuntimeError('Could not convert: ' + deref(it).first.decode('utf8') + ' of type ' + debug_type(deref(it).second).decode('utf8'))
        inc(it)
    return tmp

cdef dictionary pydict_to_dictionary(object py_dict) except *:  # Adding "except *" makes cython propagate the error if it is raised.
    cdef dictionary cdict = dictionary()
    for key, value in py_dict.items():
        if type(value) is int:
            cdict[key.encode('utf-8')] = <long>value
        elif type(value) is float:
            cdict[key.encode('utf-8')] = <double>value
        elif type(value) is bool:
            cdict[key.encode('utf-8')] = <cbool>value
        elif type(value) is str:
            cdict[key.encode('utf-8')] = <string>value.encode('utf-8')
        elif type(value) is dict:
            cdict[key.encode('utf-8')] = pydict_to_dictionary(value)
        elif type(value) is nest.NodeCollection:
            cdict[key.encode('utf-8')] = (<NodeCollectionObject>(value._datum)).thisptr
        elif type(value) is nest.Parameter:
            cdict[key.encode('utf-8')] = (<ParameterObject>(value._datum)).thisptr
        elif type(value) is ParameterObject:
            cdict[key.encode('utf-8')] = (<ParameterObject>value).thisptr
        else:
            raise AttributeError(f'when converting Python dictionary: value of key ({key}) is not a known type, got {type(value)}')
    return cdict

cdef object vec_of_dict_to_list(vector[dictionary] cvec):
    cdef tmp = []
    cdef vector[dictionary].iterator it = cvec.begin()
    while it != cvec.end():
        tmp.append(dictionary_to_pydict(deref(it)))
        inc(it)
    return tmp

cdef vector[dictionary] list_of_dict_to_vec(object pylist):
    cdef vector[dictionary] vec
    for pydict in pylist:
        vec.push_back(pydict_to_dictionary(pydict))
    return vec

def catch_cpp_error(func):
    def wrapper_catch_cpp_error(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except RuntimeError as e:
            raise NESTErrors.NESTError(f'in {func.__name__}: {e}') from None
    return wrapper_catch_cpp_error

def llapi_reset_kernel():
    reset_kernel()

@catch_cpp_error
def llapi_create(string model, long n):
    cdef NodeCollectionPTR gids
    gids = create(model, n)
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)

@catch_cpp_error
def llapi_create_spatial(object layer_params):
    cdef NodeCollectionPTR gids
    gids = create_spatial(pydict_to_dictionary(layer_params))
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)

@catch_cpp_error
def llapi_make_nodecollection(object node_ids):
    cdef NodeCollectionPTR gids
    # node_ids list is automatically converted to an std::vector
    gids = make_nodecollection(node_ids)
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)

@catch_cpp_error
def llapi_connect(NodeCollectionObject pre, NodeCollectionObject post, object conn_params, object synapse_params):
    cdef vector[dictionary] syn_param_vec
    if synapse_params is not None:
        syn_param_vec.push_back(pydict_to_dictionary(synapse_params))

    connect(pre.thisptr, post.thisptr,
            pydict_to_dictionary(conn_params),
            syn_param_vec)

@catch_cpp_error
def llapi_slice(NodeCollectionObject nc, long start, long stop, long step):
    cdef NodeCollectionPTR nc_ptr
    nc_ptr = slice_nc(nc.thisptr, start, stop, step)
    obj = NodeCollectionObject()
    obj._set_nc(nc_ptr)
    return nest.NodeCollection(obj)

@catch_cpp_error
def llapi_get_rank():
    return get_rank()

@catch_cpp_error
def llapi_get_num_mpi_processes():
    return get_num_mpi_processes()

def llapi_print_nodes():
    return print_nodes_to_string().decode('utf8')

@catch_cpp_error
def llapi_nc_size(NodeCollectionObject nc):
    return nc_size(nc.thisptr)

@catch_cpp_error
def llapi_to_string(NodeCollectionObject nc):
    return pprint_to_string(nc.thisptr).decode('utf8')

@catch_cpp_error
def llapi_get_modeldict():
    cdef dictionary cdict = get_modeldict()
    return dictionary_to_pydict(cdict)

@catch_cpp_error
def llapi_get_synapsedict():
    cdef dictionary cdict = get_synapsedict()
    return dictionary_to_pydict(cdict)

@catch_cpp_error
def llapi_get_kernel_status():
    cdef dictionary cdict = get_kernel_status()
    return dictionary_to_pydict(cdict)

@catch_cpp_error
def llapi_get_defaults(string model_name):
    return dictionary_to_pydict(get_model_defaults(model_name))

@catch_cpp_error
def llapi_get_nodes(object params, cbool local_only):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    cdef NodeCollectionPTR nc_ptr = get_nodes(params_dict, local_only)
    obj = NodeCollectionObject()
    obj._set_nc(nc_ptr)
    return nest.NodeCollection(obj)

@catch_cpp_error
def llapi_set_kernel_status(object params):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    set_kernel_status(params_dict)

@catch_cpp_error
def llapi_simulate(float t):
    simulate(t)

@catch_cpp_error
def llapi_prepare():
    prepare()

@catch_cpp_error
def llapi_run(float t):
    run(t)

@catch_cpp_error
def llapi_cleanup():
    cleanup()

@catch_cpp_error
def llapi_get_nc_status(NodeCollectionObject nc, object key=None):
    cdef dictionary statuses = get_nc_status(nc.thisptr)
    if key is None:
        return dictionary_to_pydict(statuses)
    elif isinstance(key, str):
        value = any_to_pyobj(statuses[key.encode('utf-8')])
        return value[0] if len(value) == 1 else value
    else:
        raise TypeError(f'key must be a string, got {type(key)}')

@catch_cpp_error
def llapi_set_nc_status(NodeCollectionObject nc, object params):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    set_nc_status(nc.thisptr, params_dict)

@catch_cpp_error
def llapi_join_nc(NodeCollectionObject lhs, NodeCollectionObject rhs):
    cdef NodeCollectionPTR result
    # Using operator+() directly
    result = lhs.thisptr + rhs.thisptr
    obj = NodeCollectionObject()
    obj._set_nc(result)
    return nest.NodeCollection(obj)

@catch_cpp_error
def llapi_eq_nc(NodeCollectionObject lhs, NodeCollectionObject rhs):
    return equal(lhs.thisptr, rhs.thisptr)

@catch_cpp_error
def llapi_nc_contains(NodeCollectionObject nc, long node_id):
    return contains(nc.thisptr, node_id)

@catch_cpp_error
def llapi_nc_find(NodeCollectionObject nc, long node_id):
    return find(nc.thisptr, node_id)

@catch_cpp_error
def llapi_get_nc_metadata(NodeCollectionObject nc):
    return dictionary_to_pydict(get_metadata(nc.thisptr))

@catch_cpp_error
def llapi_take_array_index(NodeCollectionObject node_collection, object array):
    if not isinstance(array, numpy.ndarray):
        raise TypeError('array must be a 1-dimensional NumPy array of ints or bools, got {}'.format(type(array)))
    if not array.ndim == 1:
        raise TypeError('array must be a 1-dimensional NumPy array, got {}-dimensional NumPy array'.format(array.ndim))

    # Get pointers to the first element in the Numpy array
    cdef long[:] array_long_mv
    cdef long* array_long_ptr

    cdef cbool[:] array_bool_mv
    cdef cbool* array_bool_ptr

    cdef NodeCollectionPTR new_nc_ptr

    if array.dtype == numpy.bool:
        # Boolean C-type arrays are not supported in NumPy, so we use an 8-bit integer array
        array_bool_mv = numpy.ascontiguousarray(array, dtype=numpy.uint8)
        array_bool_ptr = &array_bool_mv[0]
        new_nc_ptr = node_collection_array_index(node_collection.thisptr, array_bool_ptr, len(array))
    elif numpy.issubdtype(array.dtype, numpy.integer):
        array_long_mv = numpy.ascontiguousarray(array, dtype=numpy.long)
        array_long_ptr = &array_long_mv[0]
        new_nc_ptr = node_collection_array_index(node_collection.thisptr, array_long_ptr, len(array))
    else:
        raise TypeError('array must be a NumPy array of ints or bools, got {}'.format(array.dtype))
    obj = NodeCollectionObject()
    obj._set_nc(new_nc_ptr)
    return nest.NodeCollection(obj)

@catch_cpp_error
def llapi_create_parameter(object specs):
    cdef dictionary specs_dictionary = pydict_to_dictionary(specs)
    cdef shared_ptr[Parameter] parameter
    parameter = create_parameter(specs_dictionary)
    obj = ParameterObject()
    obj._set_parameter(parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_get_param_value(ParameterObject parameter):
    return get_value(parameter.thisptr)

@catch_cpp_error
def llapi_param_is_spatial(ParameterObject parameter):
    return is_spatial(parameter.thisptr)

@catch_cpp_error
def llapi_multiply_parameter(ParameterObject first, ParameterObject second):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = multiply_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_divide_parameter(ParameterObject first, ParameterObject second):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = divide_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_add_parameter(ParameterObject first, ParameterObject second):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = add_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_subtract_parameter(ParameterObject first, ParameterObject second):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = subtract_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_compare_parameter(ParameterObject first, ParameterObject second, object pydict):
    cdef shared_ptr[Parameter] new_parameter
    cdef dictionary cdict = pydict_to_dictionary(pydict)
    new_parameter = compare_parameter(first.thisptr, second.thisptr, cdict)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_conditional_parameter(ParameterObject condition, ParameterObject if_true, ParameterObject if_false):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = conditional_parameter(condition.thisptr, if_true.thisptr, if_false.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_min_parameter(ParameterObject parameter, double other_value):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = min_parameter(parameter.thisptr, other_value)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_max_parameter(ParameterObject parameter, double other_value):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = max_parameter(parameter.thisptr, other_value)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_redraw_parameter(ParameterObject parameter, double min_value, double max_value):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = redraw_parameter(parameter.thisptr, min_value, max_value)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_exp_parameter(ParameterObject parameter):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = exp_parameter(parameter.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_sin_parameter(ParameterObject parameter):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = sin_parameter(parameter.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_cos_parameter(ParameterObject parameter):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = cos_parameter(parameter.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_pow_parameter(ParameterObject parameter, double exponent):
    cdef shared_ptr[Parameter] new_parameter
    new_parameter = pow_parameter(parameter.thisptr, exponent)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_dimension_parameter(object list_of_pos_params):
    cdef shared_ptr[Parameter] dim_parameter
    cdef ParameterObject x, y, z
    if len(list_of_pos_params) == 2:
        x, y = list_of_pos_params
        dim_parameter = dimension_parameter(x.thisptr, y.thisptr)
    if len(list_of_pos_params) == 3:
        x, y, z = list_of_pos_params
        dim_parameter = dimension_parameter(x.thisptr, y.thisptr, z.thisptr)
    obj = ParameterObject()
    obj._set_parameter(dim_parameter)
    return nest.Parameter(obj)

@catch_cpp_error
def llapi_get_connections(object params):
    cdef dictionary params_dictionary = pydict_to_dictionary(params)
    cdef deque[ConnectionID] connections

    connections = get_connections(params_dictionary)

    cdef connections_list = []
    cdef deque[ConnectionID].iterator it = connections.begin()
    while it != connections.end():
        obj = ConnectionObject()
        obj._set_connection_id(deref(it))
        connections_list.append(obj)
        inc(it)

    return nest.SynapseCollection(connections_list)

@catch_cpp_error
def llapi_get_connection_status(object conns):
    cdef vector[dictionary] connection_statuses
    # Convert the list of connections to a deque
    cdef deque[ConnectionID] conn_deque
    cdef ConnectionObject conn_object
    for conn_object in conns:
        conn_deque.push_back(conn_object.thisobj)

    connection_statuses = get_connection_status(conn_deque)

    return vec_of_dict_to_list(connection_statuses)


@catch_cpp_error
def llapi_set_connection_status(object conns, object params):
    # Convert the list of connections to a deque
    cdef deque[ConnectionID] conn_deque
    cdef ConnectionObject conn_object
    for conn_object in conns:
        conn_deque.push_back(conn_object.thisobj)

    # params can be a dictionary or a list of dictionaries
    if isinstance(params, dict):
        set_connection_status(conn_deque, pydict_to_dictionary(params))
    elif isinstance(params, list):
        if len(params) != len(conns):
            raise ValueError('params list length must be equal to number of connections')
        set_connection_status(conn_deque, list_of_dict_to_vec(params))
    else:
        raise TypeError('params must be a dict or a list of dicts')
