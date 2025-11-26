# -*- coding: utf-8 -*-
#
# nestkernel_api.pyx
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

from cython.operator cimport dereference as deref
from cython.operator cimport preincrement as inc
from libc.stdint cimport int64_t, uint64_t
from libc.stdlib cimport free, malloc
from libcpp.deque cimport deque as std_deque
from libcpp.map cimport map as std_map
from libcpp.string cimport string as std_string
from libcpp.vector cimport vector as std_vector

import numbers

import nest
import numpy

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

NESTErrors = None

def init(args):
    global NESTErrors
    cdef int argc = len(args)

    # OpenMPI depends on last entry of argv being null
    cdef char** c_argv = <char**>malloc(sizeof(char*) * (argc + 1))
    if c_argv is NULL:
        raise NESTErrors.PyNESTError("NESTKernel::init : couldn't allocate c_argv")

    c_argv[argc] = NULL

    try:
        for idx, s in enumerate([pystr_to_string(x) for x in args]):
            c_argv[idx] = s
        init_nest(&argc, &c_argv)
        create_exceptions()  # requires fully initialized NEST kernel
        NESTErrors = <object> nest_error_module
    finally:
        free(c_argv)


cdef class NodeCollectionObject:

    cdef NodeCollectionPTR thisptr

    def __repr__(self):
        return "<NodeCollectionObject>"

    cdef _set_nc(self, NodeCollectionPTR nc):
        self.thisptr = nc


cdef class ConnectionObject:

    cdef ConnectionID thisobj

    def __repr__(self):
        return "<ConnectionObject>"

    cdef _set_connection_id(self, ConnectionID conn_id):
        self.thisobj = conn_id


cdef class ParameterObject:

    cdef ParameterPTR thisptr

    def __repr__(self):
        return "<ParameterObject>"

    cdef _set_parameter(self, ParameterPTR parameter_ptr):
        self.thisptr = parameter_ptr


cdef class MaskObject:

    cdef MaskPTR thisptr

    def __repr__(self):
        return "<MaskObject>"

    cdef _set_mask(self, MaskPTR mask_ptr):
        self.thisptr = mask_ptr


cdef object any_vector_to_list(vector[any] cvec):
    cdef tmp = []
    cdef vector[any].iterator it = cvec.begin()
    while it != cvec.end():
        tmp.append(any_to_pyobj(deref(it)))
        inc(it)
    return tmp


cdef object dict_vector_to_list(vector[dictionary] cvec):
    cdef tmp = []
    cdef vector[dictionary].iterator it = cvec.begin()
    while it != cvec.end():
        tmp.append(dictionary_to_pydict(deref(it)))
        inc(it)
    return tmp

def make_tuple_or_ndarray(operand):
        if len(operand) > 0 and isinstance(operand[0], numbers.Number):
            return numpy.array(operand)
        else:
            return tuple(operand)

cdef object any_to_pyobj(any operand):
    if is_type[int](operand):
        return any_cast[int](operand)
    if is_type[uint](operand):
        return any_cast[uint](operand)
    if is_type[long](operand):
        return any_cast[long](operand)
    if is_type[size_t](operand):
        return any_cast[size_t](operand)
    if is_type[uint64_t](operand):
        return any_cast[uint64_t](operand)
    if is_type[int64_t](operand):
        return any_cast[int64_t](operand)
    if is_type[double](operand):
        return any_cast[double](operand)
    if is_type[cbool](operand):
        return any_cast[cbool](operand)
    if is_type[std_string](operand):
        return string_to_pystr(any_cast[std_string](operand))
    if is_type[std_vector[int]](operand):
        return numpy.array(any_cast[std_vector[int]](operand))
    if is_type[std_vector[long]](operand):
        return numpy.array(any_cast[std_vector[long]](operand))
    if is_type[std_vector[size_t]](operand):
        return numpy.array(any_cast[std_vector[size_t]](operand))
    if is_type[std_vector[double]](operand):
        return numpy.array(any_cast[std_vector[double]](operand))
    if is_type[std_vector[std_vector[double]]](operand):
        return numpy.array(any_cast[std_vector[std_vector[double]]](operand))
    if is_type[std_vector[std_vector[std_vector[double]]]](operand):
        return numpy.array(any_cast[std_vector[std_vector[std_vector[double]]]](operand))
    if is_type[std_vector[std_vector[std_vector[long]]]](operand):
        return numpy.array(any_cast[std_vector[std_vector[std_vector[long]]]](operand))
    if is_type[std_vector[std_string]](operand):
        # PYNEST-NG: Do we want to have this or are bytestrings fine?
        # return any_cast[std_vector[std_string]](operand)
        return list(map(lambda x: x.decode("utf-8"), any_cast[std_vector[std_string]](operand)))
    if is_type[std_vector[dictionary]](operand):
        return dict_vector_to_list(any_cast[std_vector[dictionary]](operand))
    if is_type[std_vector[any]](operand):
        # PYNEST-NG: This will create a Python list first and then convert to
        # either tuple or numpy array, which will copy the data element-wise.
        return make_tuple_or_ndarray(any_vector_to_list(any_cast[std_vector[any]](operand)))
    if is_type[dictionary](operand):
        return dictionary_to_pydict(any_cast[dictionary](operand))
    if is_type[NodeCollectionPTR](operand):
        obj = NodeCollectionObject()
        obj._set_nc(any_cast[NodeCollectionPTR](operand))
        return nest.NodeCollection(obj)
    if is_type[VerbosityLevel](operand):
        return any_cast[VerbosityLevel](operand)

cdef object dictionary_to_pydict(dictionary cdict):
    cdef tmp = {}

    cdef dictionary.const_iterator it = cdict.begin()
    while it != cdict.end():
        key = string_to_pystr(deref(it).first)
        tmp[key] = any_to_pyobj(deref(it).second.item)
        if tmp[key] is None:
            # If we end up here, the value in the dictionary is of a type that any_to_pyobj() cannot handle.
            raise RuntimeError('Could not convert: ' + key + ' of type ' + string_to_pystr(debug_type(deref(it).second.item)))
        inc(it)
    return tmp


cdef is_list_tuple_ndarray_of_float(v):
    list_of_float = type(v) is list and len(v) > 0 and type(v[0]) is float
    tuple_of_float = type(v) is tuple and len(v) > 0 and type(v[0]) is float
    ndarray_of_float = isinstance(v, numpy.ndarray) and numpy.issubdtype(v.dtype, numpy.floating)
    return list_of_float or tuple_of_float or ndarray_of_float


cdef is_list_tuple_ndarray_of_int(v):
    list_of_int = type(v) is list and len(v) > 0 and type(v[0]) is int
    tuple_of_int = type(v) is tuple and len(v) > 0 and type(v[0]) is int
    ndarray_of_int = isinstance(v, numpy.ndarray) and numpy.issubdtype(v.dtype, numpy.integer)
    return list_of_int or tuple_of_int or ndarray_of_int


cdef dictionary pydict_to_dictionary(object py_dict) except *:  # Adding "except *" makes cython propagate the error if it is raised.
    cdef dictionary cdict = dictionary()
    for key, value in py_dict.items():
        if type(value) is tuple:
            value = list(value)

        if type(value) is int or isinstance(value, numpy.integer):
	    # PYTEST-NG: Should we guard against overflow given that python int has infinite range?
            cdict[pystr_to_string(key)] = <long>value
        elif type(value) is float or isinstance(value, numpy.floating):
            cdict[pystr_to_string(key)] = <double>value
        elif type(value) is bool:
            cdict[pystr_to_string(key)] = <cbool>value
        elif type(value) is str:
            cdict[pystr_to_string(key)] = <string>pystr_to_string(value)
        elif type(value) is list and len(value) == 0:
            # We cannot infer the intended element type from an empty list.
            # We therefore pass an empty vector[any]. vector[any] will always be empty
            # and an empty vector will always be vector[any] in the PyNEST interface.
            cdict[pystr_to_string(key)] = empty_any_vec()
        elif is_list_tuple_ndarray_of_float(value):
            cdict[pystr_to_string(key)] = pylist_or_ndarray_to_doublevec(value)
        elif is_list_tuple_ndarray_of_int(value):
            cdict[pystr_to_string(key)] = pylist_to_intvec(value)
        elif type(value) is list and len(value) > 0 and isinstance(value[0], (list, tuple)):
            cdict[pystr_to_string(key)] = list_of_list_to_doublevec(value)
        elif type(value) is list and len(value) > 0 and isinstance(value[0], numpy.ndarray):
            cdict[pystr_to_string(key)] = list_of_list_to_doublevec(value)
        elif type(value) is list and len(value) > 0 and type(value[0]) is str:
            cdict[pystr_to_string(key)] = pylist_to_stringvec(value)
        elif type(value) is list and len(value) > 0 and type(value[0]) is dict:
            cdict[pystr_to_string(key)] = pylist_to_dictvec(value)
        elif type(value) is dict:
            cdict[pystr_to_string(key)] = pydict_to_dictionary(value)
        elif type(value) is nest.NodeCollection:
            cdict[pystr_to_string(key)] = (<NodeCollectionObject>(value._datum)).thisptr
        elif isinstance(value, nest.Parameter):
            cdict[pystr_to_string(key)] = (<ParameterObject>(value._datum)).thisptr
        elif type(value) is ParameterObject:
            cdict[pystr_to_string(key)] = (<ParameterObject>value).thisptr
        elif type(value) is type(nest.VerbosityLevel.ALL):  # ALL will always exist
            cdict[pystr_to_string(key)] = <VerbosityLevel>(value)
        else:
            typename = type(value)
            if type(value) is list:
                assert len(value) > 0   # empty list should have been caught above
                typename = f"list of {type(value[0])}"
            raise AttributeError(f'when converting Python dictionary: value of key ({key}) is not a known type, got {typename}')

    return cdict


cdef object vec_of_dict_to_list(vector[dictionary] cvec):
    cdef tmp = []
    cdef vector[dictionary].iterator it = cvec.begin()
    while it != cvec.end():
        tmp.append(dictionary_to_pydict(deref(it)))
        inc(it)
    return tmp


cdef vector[any] empty_any_vec():
    cdef vector[any] empty_vec
    return empty_vec


cdef vector[dictionary] list_of_dict_to_vec(object pylist):
    cdef vector[dictionary] vec
    # PYNEST-NG: reserve the correct size and use index-based
    # assignments instead of pushing back
    for pydict in pylist:
        vec.push_back(pydict_to_dictionary(pydict))
    return vec


cdef vector[std_vector[double]] list_of_list_to_doublevec(object pylist):
    cdef vector[std_vector[double]] vec
    for val in pylist:
        vec.push_back(val)
    return vec


cdef vector[long] pylist_to_intvec(object pylist):
    cdef vector[long] vec
    for val in pylist:
        vec.push_back(val)
    return vec


cdef vector[double] pylist_or_ndarray_to_doublevec(object pylist):
    cdef vector[double] vec
    vec = pylist
    return vec


cdef vector[std_string] pylist_to_stringvec(object pylist):
    cdef vector[std_string] vec
    for val in pylist:
        vec.push_back(<string>pystr_to_string(val))
    return vec


cdef vector[dictionary] pylist_to_dictvec(object pylist):
    cdef vector[dictionary] vec
    for val in pylist:
        vec.push_back(pydict_to_dictionary(val))
    return vec


cdef object string_to_pystr(string s):
    return s.decode('utf-8')


cdef string pystr_to_string(object s):
    return s.encode('utf-8')


def llapi_shutdown_nest( exitcode ):
    shutdown_nest( exitcode )


def llapi_reset_kernel():
    reset_kernel()


def llapi_enable_structural_plasticity():
    enable_structural_plasticity()


def llapi_disable_structural_plasticity():
    disable_structural_plasticity()


def llapi_create(model, long n):
    cdef NodeCollectionPTR gids
    gids = create(pystr_to_string(model), n)
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)


def llapi_create_spatial(object layer_params):
    cdef NodeCollectionPTR gids
    gids = create_spatial(pydict_to_dictionary(layer_params))
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)


def llapi_get_position(NodeCollectionObject layer):
    cdef vector[std_vector[double]] result = get_position(layer.thisptr)
    if nc_size(layer.thisptr) == 1:
        return result[0]
    else:
        return result

def llapi_node_collection_to_array( NodeCollectionObject nc, selection):
    cdef vector[size_t] result = node_collection_to_array( nc.thisptr, pystr_to_string(selection) )
    return result

def llapi_spatial_distance(object from_arg, to_arg):
    cdef vector[std_vector[double]] from_vec
    if isinstance(from_arg, nest.NodeCollection):
        return distance((<NodeCollectionObject>(to_arg._datum)).thisptr, (<NodeCollectionObject>(from_arg._datum)).thisptr)
    elif isinstance(from_arg, (list, tuple)):
        from_vec = from_arg
        return distance((<NodeCollectionObject>(to_arg._datum)).thisptr, from_vec)
    else:
        raise TypeError("from_arg must be either a NodeCollection or a list/tuple of positions")


def llapi_displacement(object from_arg, to_arg):
    cdef vector[std_vector[double]] from_vec
    if isinstance(from_arg, nest.NodeCollection):
        return displacement((<NodeCollectionObject>(to_arg._datum)).thisptr, (<NodeCollectionObject>(from_arg._datum)).thisptr)
    elif isinstance(from_arg, (list, tuple)):
        from_vec = from_arg
        return displacement((<NodeCollectionObject>(to_arg._datum)).thisptr, from_vec)
    else:
        raise TypeError("from_arg must be either a NodeCollection or a list/tuple of positions")


def llapi_distance(object conn):  # PYNEST-NG: should there be a SynapseCollectionObject?
    cdef vector[ConnectionID] conn_vec
    for c in conn:
        conn_vec.push_back((<ConnectionObject>(c)).thisobj)
    cdef vector[double] result = distance(conn_vec)
    return result

# PYNEST-NG:
#
# inside
# or (aka union_mask)
# and (aka intersect_mask)
# sub (aka minus_mask)

def llapi_make_nodecollection(object node_ids):
    cdef NodeCollectionPTR gids
    # node_ids list is automatically converted to an std::vector
    gids = make_nodecollection(node_ids)
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)


def llapi_connect(NodeCollectionObject pre, NodeCollectionObject post, object conn_params, object synapse_params):
    conn_params = conn_params if conn_params is not None else {}
    synapse_params = synapse_params if synapse_params is not None else {}

    if ("rule" in conn_params and conn_params["rule"] is None) or "rule" not in conn_params:
        conn_params["rule"] = "all_to_all"

    if synapse_params is dict and "synapse_model" not in synapse_params:
        synapse_params["synapse_model"] = "static_synapse"

    cdef vector[dictionary] syn_param_vec
    if isinstance(synapse_params, nest.CollocatedSynapses):
        syn_param_vec = pylist_to_dictvec(synapse_params.syn_specs)
    elif synapse_params is not None:
        syn_param_vec.push_back(pydict_to_dictionary(synapse_params))

    connect(pre.thisptr, post.thisptr,
            pydict_to_dictionary(conn_params),
            syn_param_vec)


def llapi_connect_tripartite(NodeCollectionObject pre, NodeCollectionObject post, NodeCollectionObject third,
                             object conn_params, object third_factor_conn_params, object synapse_params):
    # PYNEST-NG: See if we should add some more checks as in llapi_connect above (rule)


    # We are guaranteed that syn_param_vec is a dict {'primary': sc_p, 'third_in': sc_i, 'third_out': sc_o}
    # where all sc_* are SynapseCollection objects

    cdef std_map[string, vector[dictionary]] syn_param_map
    for k, colloc_syns in synapse_params.items():
        syn_param_map[pystr_to_string(k)] = pylist_to_dictvec(colloc_syns.syn_specs)

    connect_tripartite(pre.thisptr, post.thisptr, third.thisptr,
    	               pydict_to_dictionary(conn_params), pydict_to_dictionary(third_factor_conn_params),
                       syn_param_map)


def llapi_disconnect(NodeCollectionObject pre, NodeCollectionObject post, object conn_params, object synapse_params):
    conn_params = conn_params if conn_params is not None else {}
    synapse_params = synapse_params if synapse_params is not None else {}

    if ("rule" in conn_params and conn_params["rule"] is None) or "rule" not in conn_params:
        conn_params["rule"] = "all_to_all"

    if synapse_params is dict and "synapse_model" not in synapse_params:
        synapse_params["synapse_model"] = "static_synapse"

    # Pass synapse specs as vector/collocated synapse for consistency with Connect().
    # This simplifies the C++ level because ConnBuilder() constructors expect vectors of synapse specs.
    cdef vector[dictionary] syn_param_vec
    if isinstance(synapse_params, nest.CollocatedSynapses):
        syn_param_vec = pylist_to_dictvec(synapse_params.syn_specs)
    elif synapse_params is not None:
        syn_param_vec.push_back(pydict_to_dictionary(synapse_params))

    disconnect(pre.thisptr, post.thisptr,
            pydict_to_dictionary(conn_params),
            syn_param_vec)


def llapi_disconnect_syncoll(object conns):
    cdef std_deque[ConnectionID] conn_deque
    cdef ConnectionObject conn_object
    for conn_object in conns:
        conn_deque.push_back(conn_object.thisobj)

    disconnect(conn_deque)


def llapi_connect_layers(NodeCollectionObject pre, NodeCollectionObject post, object projections):
    connect_layers(pre.thisptr, post.thisptr, pydict_to_dictionary(projections))


def llapi_connect_sonata(object graph_specs, long hyperslab_size):
    connect_sonata(pydict_to_dictionary(graph_specs), hyperslab_size)


def llapi_create_mask(object specs):
    cdef dictionary specs_dictionary = pydict_to_dictionary(specs)
    cdef MaskPTR mask
    mask = create_mask(specs_dictionary)
    obj = MaskObject()
    obj._set_mask(mask)
    return nest.Mask(obj)


def llapi_select_nodes_by_mask(NodeCollectionObject layer, vector[double] anchor, MaskObject mask_datum):
    nodes = select_nodes_by_mask(layer.thisptr, anchor, mask_datum.thisptr)
    obj = NodeCollectionObject()
    obj._set_nc(nodes)
    return nest.NodeCollection(obj)


def llapi_inside_mask(vector[double] point, MaskObject mask):
    return inside(point, mask.thisptr)


def llapi_dump_layer_nodes(NodeCollectionObject layer, object filename):
    dump_layer_nodes(layer.thisptr, pystr_to_string(filename))


def llapi_dump_layer_connections(NodeCollectionObject source_layer, NodeCollectionObject target_layer, synapse_model, filename):
    dump_layer_connections(source_layer.thisptr, target_layer.thisptr, pystr_to_string(synapse_model), pystr_to_string(filename))


def llapi_slice(NodeCollectionObject nc, long start, long stop, long step):
    cdef NodeCollectionPTR nc_ptr
    nc_ptr = slice_nc(nc.thisptr, start, stop, step)
    obj = NodeCollectionObject()
    obj._set_nc(nc_ptr)
    return nest.NodeCollection(obj)


def llapi_print_nodes():
    return string_to_pystr(print_nodes_to_string())


def llapi_nc_size(NodeCollectionObject nc):
    return nc_size(nc.thisptr)


def llapi_to_string(NodeCollectionObject nc):
    return string_to_pystr(pprint_to_string(nc.thisptr))


def llapi_get_kernel_status():
    cdef dictionary cdict = get_kernel_status()
    return dictionary_to_pydict(cdict)


def llapi_get_defaults(object model_name):
    return dictionary_to_pydict(get_model_defaults(pystr_to_string(model_name)))


def llapi_set_defaults(object model_name, object params):
    set_model_defaults(pystr_to_string(model_name), pydict_to_dictionary(params))


def llapi_get_nodes(object params, cbool local_only):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    cdef NodeCollectionPTR nc_ptr = get_nodes(params_dict, local_only)
    obj = NodeCollectionObject()
    obj._set_nc(nc_ptr)
    return nest.NodeCollection(obj)


def llapi_set_kernel_status(object params):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    set_kernel_status(params_dict)


def llapi_simulate(double t):
    simulate(t)


def llapi_prepare():
    prepare()


def llapi_run(double t):
    run(t)


def llapi_cleanup():
    cleanup()


def llapi_copy_model(oldmodname, newmodname, object params):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    copy_model(pystr_to_string(oldmodname), pystr_to_string(newmodname), params_dict)


def llapi_get_nc_status(NodeCollectionObject nc, object key=None):
    cdef dictionary statuses = get_nc_status(nc.thisptr)
    if key is None:
        return dictionary_to_pydict(statuses)
    elif isinstance(key, str):
        if not statuses.known(pystr_to_string(key)):
            raise KeyError(key)
        value = any_to_pyobj(statuses[pystr_to_string(key)])
        # PYNEST-NG: This is backwards-compatible, but makes it harder
        # to write scalable code. Maybe just return value as is?
        return value[0] if len(value) == 1 else value
    else:
        raise TypeError(f'key must be a string, got {type(key)}')


def llapi_set_nc_status(NodeCollectionObject nc, object params_list):
    cdef vector[dictionary] params = list_of_dict_to_vec(params_list)
    set_nc_status(nc.thisptr, params)


def llapi_join_nc(NodeCollectionObject lhs, NodeCollectionObject rhs):
    cdef NodeCollectionPTR result
    # Using operator+() directly
    result = lhs.thisptr + rhs.thisptr
    obj = NodeCollectionObject()
    obj._set_nc(result)
    return nest.NodeCollection(obj)


def llapi_eq_nc(NodeCollectionObject lhs, NodeCollectionObject rhs):
    return equal(lhs.thisptr, rhs.thisptr)


def llapi_nc_contains(NodeCollectionObject nc, long node_id):
    return contains(nc.thisptr, node_id)


def llapi_nc_find(NodeCollectionObject nc, long node_id):
    return find(nc.thisptr, node_id)


def llapi_get_nc_metadata(NodeCollectionObject nc):
    return dictionary_to_pydict(get_metadata(nc.thisptr))


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

    if array.dtype == bool:
        # Boolean C-type arrays are not supported in NumPy, so we use an 8-bit integer array
        array_bool_mv = numpy.ascontiguousarray(array, dtype=numpy.uint8)
        array_bool_ptr = &array_bool_mv[0]
        new_nc_ptr = node_collection_array_index(node_collection.thisptr, array_bool_ptr, len(array))
    elif numpy.issubdtype(array.dtype, numpy.integer):
        array_long_mv = numpy.ascontiguousarray(array, dtype=int)
        array_long_ptr = &array_long_mv[0]
        new_nc_ptr = node_collection_array_index(node_collection.thisptr, array_long_ptr, len(array))
    else:
        raise TypeError('array must be a NumPy array of ints or bools, got {}'.format(array.dtype))
    obj = NodeCollectionObject()
    obj._set_nc(new_nc_ptr)
    return nest.NodeCollection(obj)


def llapi_create_parameter(object specs):
    cdef dictionary specs_dictionary = pydict_to_dictionary(specs)
    cdef ParameterPTR parameter
    parameter = create_parameter(specs_dictionary)
    obj = ParameterObject()
    obj._set_parameter(parameter)
    return nest.Parameter(obj)


def llapi_get_param_value(ParameterObject parameter):
    return get_value(parameter.thisptr)


def llapi_param_is_spatial(ParameterObject parameter):
    return is_spatial(parameter.thisptr)


def llapi_apply_parameter(ParameterObject parameter, object pos_or_nc):
    if type(pos_or_nc) is nest.NodeCollection:
        return tuple(apply(parameter.thisptr, (<NodeCollectionObject>(pos_or_nc._datum)).thisptr))
    else:
        return tuple(apply(parameter.thisptr, pydict_to_dictionary(pos_or_nc)))


def llapi_multiply_parameter(ParameterObject first, ParameterObject second):
    cdef ParameterPTR new_parameter
    new_parameter = multiply_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_divide_parameter(ParameterObject first, ParameterObject second):
    cdef ParameterPTR new_parameter
    new_parameter = divide_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_add_parameter(ParameterObject first, ParameterObject second):
    cdef ParameterPTR new_parameter
    new_parameter = add_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_subtract_parameter(ParameterObject first, ParameterObject second):
    cdef ParameterPTR new_parameter
    new_parameter = subtract_parameter(first.thisptr, second.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_compare_parameter(ParameterObject first, ParameterObject second, object pydict):
    cdef ParameterPTR new_parameter
    cdef dictionary cdict = pydict_to_dictionary(pydict)
    new_parameter = compare_parameter(first.thisptr, second.thisptr, cdict)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_conditional_parameter(ParameterObject condition, ParameterObject if_true, ParameterObject if_false):
    cdef ParameterPTR new_parameter
    new_parameter = conditional_parameter(condition.thisptr, if_true.thisptr, if_false.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_min_parameter(ParameterObject parameter, double other_value):
    cdef ParameterPTR new_parameter
    new_parameter = min_parameter(parameter.thisptr, other_value)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_max_parameter(ParameterObject parameter, double other_value):
    cdef ParameterPTR new_parameter
    new_parameter = max_parameter(parameter.thisptr, other_value)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_redraw_parameter(ParameterObject parameter, double min_value, double max_value):
    cdef ParameterPTR new_parameter
    new_parameter = redraw_parameter(parameter.thisptr, min_value, max_value)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_exp_parameter(ParameterObject parameter):
    cdef ParameterPTR new_parameter
    new_parameter = exp_parameter(parameter.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_sin_parameter(ParameterObject parameter):
    cdef ParameterPTR new_parameter
    new_parameter = sin_parameter(parameter.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_cos_parameter(ParameterObject parameter):
    cdef ParameterPTR new_parameter
    new_parameter = cos_parameter(parameter.thisptr)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_pow_parameter(ParameterObject parameter, double exponent):
    cdef ParameterPTR new_parameter
    new_parameter = pow_parameter(parameter.thisptr, exponent)
    obj = ParameterObject()
    obj._set_parameter(new_parameter)
    return nest.Parameter(obj)


def llapi_dimension_parameter(object list_of_pos_params):
    cdef ParameterPTR dim_parameter
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


def llapi_get_connections(object params):
    cdef dictionary params_dictionary = pydict_to_dictionary(params)
    cdef std_deque[ConnectionID] connections

    connections = get_connections(params_dictionary)

    cdef connections_list = []
    cdef std_deque[ConnectionID].iterator it = connections.begin()
    while it != connections.end():
        obj = ConnectionObject()
        obj._set_connection_id(deref(it))
        connections_list.append(obj)
        inc(it)

    return nest.SynapseCollection(connections_list)

def llapi_get_connection_status(object conns):
    cdef vector[dictionary] connection_statuses
    # Convert the list of connections to a deque
    cdef std_deque[ConnectionID] conn_deque
    cdef ConnectionObject conn_object
    for conn_object in conns:
        conn_deque.push_back(conn_object.thisobj)

    connection_statuses = get_connection_status(conn_deque)

    return vec_of_dict_to_list(connection_statuses)


def llapi_set_connection_status(object conns, object params):
    # Convert the list of connections to a deque
    cdef std_deque[ConnectionID] conn_deque
    cdef ConnectionObject conn_object
    for conn_object in conns:
        conn_deque.push_back(conn_object.thisobj)

    # params can be a dictionary or a list of dictionaries
    if isinstance(params, dict):
        set_connection_status(conn_deque, pydict_to_dictionary(params))
    elif isinstance(params, (list, tuple)):
        if len(params) != len(conns):
            raise ValueError('params list length must be equal to number of connections')
        set_connection_status(conn_deque, list_of_dict_to_vec(params))
    else:
        raise TypeError('params must be a dict or a list of dicts')


def llapi_connect_arrays(sources, targets, weights, delays, synapse_model, syn_param_keys, syn_param_values):
    """Calls connect_arrays function, bypassing SLI to expose pointers to the NumPy arrays"""

    if not (isinstance(sources, numpy.ndarray) and sources.ndim == 1) or not numpy.issubdtype(sources.dtype, numpy.integer):
        raise TypeError('sources must be a 1-dimensional NumPy array of integers')
    if not (isinstance(targets, numpy.ndarray) and targets.ndim == 1) or not numpy.issubdtype(targets.dtype, numpy.integer):
        raise TypeError('targets must be a 1-dimensional NumPy array of integers')
    if weights is not None and not (isinstance(weights, numpy.ndarray) and weights.ndim == 1):
        raise TypeError('weights must be a 1-dimensional NumPy array')
    if delays is not None and  not (isinstance(delays, numpy.ndarray) and delays.ndim == 1):
        raise TypeError('delays must be a 1-dimensional NumPy array')
    if syn_param_keys is not None and not ((isinstance(syn_param_keys, numpy.ndarray) and syn_param_keys.ndim == 1) and
                                            numpy.issubdtype(syn_param_keys.dtype, numpy.str_)):
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
    cdef long[::1] sources_mv = numpy.ascontiguousarray(sources, dtype=numpy.int64)
    cdef long* sources_ptr = &sources_mv[0]

    cdef long[::1] targets_mv = numpy.ascontiguousarray(targets, dtype=numpy.int64)
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
    cdef vector[std_string] param_keys_ptr
    if syn_param_keys is not None:
        for key in syn_param_keys:
            param_keys_ptr.push_back(pystr_to_string(key))

    cdef double[:, ::1] param_values_mv
    cdef double* param_values_ptr = NULL
    if syn_param_values is not None:
        param_values_mv = numpy.ascontiguousarray(syn_param_values, dtype=numpy.double)
        param_values_ptr = &param_values_mv[0][0]

    cdef string syn_model_string = synapse_model.encode('UTF-8')

    connect_arrays( sources_ptr, targets_ptr, weights_ptr, delays_ptr, param_keys_ptr, param_values_ptr, len(sources), syn_model_string )
