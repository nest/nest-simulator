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

import cython

# from libc.stdlib cimport malloc, free
# from libc.string cimport memcpy

from libcpp.string cimport string
from libcpp.vector cimport vector

from cython.operator cimport dereference as deref
from cython.operator cimport preincrement as inc

# from cpython cimport array

# from cpython.ref cimport PyObject
# from cpython.object cimport Py_LT, Py_LE, Py_EQ, Py_NE, Py_GT, Py_GE

import nest
from nest.lib.hl_api_exceptions import NESTMappedException, NESTErrors, NESTError


cdef class NodeCollectionObject(object):

    cdef NodeCollectionPTR thisptr

    def __repr__(self):
        return "<NodeCollectionObject>"

    cdef _set_nc(self, NodeCollectionPTR nc):
        self.thisptr = nc

    # cdef _get_ptr(self):
    #     return self.thisptr


cdef object any_vector_to_list(vector[any] cvec):
    cdef tmp = []
    cdef vector[any].iterator it = cvec.begin()
    while it != cvec.end():
        tmp.append(any_to_pyobj(deref(it)))
        inc(it)
    return tmp


cdef object any_to_pyobj(any operand):
    if is_int(operand):
        return any_cast[int](operand)
    if is_long(operand):
        return any_cast[long](operand)
    if is_size_t(operand):
        return any_cast[size_t](operand)
    if is_double(operand):
        return any_cast[double](operand)
    if is_bool(operand):
        return any_cast[cbool](operand)
    if is_string(operand):
        return any_cast[string](operand)
    if is_int_vector(operand):
        return any_cast[vector[int]](operand)
    if is_double_vector(operand):
        return any_cast[vector[double]](operand)
    if is_string_vector(operand):
        return any_cast[vector[string]](operand)
    if is_any_vector(operand):
        return tuple(any_vector_to_list(any_cast[vector[any]](operand)))
    if is_dict(operand):
        return dictionary_to_pydict(any_cast[dictionary](operand))

cdef object dictionary_to_pydict(dictionary cdict):
    cdef tmp = {}

    cdef dictionary.const_iterator it = cdict.begin()
    while it != cdict.end():
        tmp[deref(it).first.decode('utf8')] = any_to_pyobj(deref(it).second)
        if tmp[deref(it).first.decode('utf8')] is None:
            print('Could not convert: ' + deref(it).first.decode('utf8') + ' of type ' + debug_type(deref(it).second).decode('utf8'))
        inc(it)
    return tmp

cdef dictionary pydict_to_dictionary(object py_dict):
    cdef dictionary cdict = dictionary()
    for key, value in py_dict.items():
        if isinstance(value, int):
            cdict[key.encode('utf-8')] = <long>value
        elif isinstance(value, float):
            cdict[key.encode('utf-8')] = <double>value
        elif isinstance(value, str):
            cdict[key.encode('utf-8')] = <string>value.encode('utf-8')
        else:
            raise AttributeError(f'value of key ({key}) is not a known type, got {type(value)}')
    return cdict

# cdef object vec_of_dict_to_list(vector[dictionary] cvec):
#     cdef tmp = []
#     cdef vector[dictionary].iterator it = cvec.begin()
#     while it != cvec.end():
#         tmp.append(dictionary_to_pydict(deref(it)))
#         inc(it)
#     return tmp

def llapi_reset_kernel():
    reset_kernel()

def llapi_create(string model, long n):
    cdef NodeCollectionPTR gids
    try:
        gids = create(model, n)
    except RuntimeError as e:
        exceptionCls = getattr(NESTErrors, str(e))
        raise exceptionCls('llapi_create', '') from None
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)

def llapi_make_nodecollection(object node_ids):
    cdef NodeCollectionPTR gids
    try:
        gids = make_nodecollection(node_ids)
    except RuntimeError as e:
        exceptionCls = getattr(NESTErrors, str(e))
        raise exceptionCls('llapi_make_nodecollection', '') from None
    obj = NodeCollectionObject()
    obj._set_nc(gids)
    return nest.NodeCollection(obj)

def llapi_connect(NodeCollectionObject pre, NodeCollectionObject post, object conn_params, object synapse_params):
    cdef vector[dictionary] syn_param_vec
    if synapse_params is not None:
        syn_param_vec.push_back(pydict_to_dictionary(synapse_params))

    connect(pre.thisptr, post.thisptr,
            pydict_to_dictionary(conn_params),
            syn_param_vec)

def llapi_slice(NodeCollectionObject nc, long start, long stop, long step):
    cdef NodeCollectionPTR nc_ptr
    try:
        nc_ptr = slice_nc(nc.thisptr, start, stop, step)
    except RuntimeError as e:
        exceptionCls = getattr(NESTErrors, str(e))
        raise exceptionCls('llapi_slice', '') from None
    obj = NodeCollectionObject()
    obj._set_nc(nc_ptr)
    return nest.NodeCollection(obj)

def llapi_nc_size(NodeCollectionObject nc):
    return nc_size(nc.thisptr)

def llapi_to_string(NodeCollectionObject nc):
    return pprint_to_string(nc.thisptr)

def llapi_get_kernel_status():
    cdef dictionary cdict = get_kernel_status()
    return dictionary_to_pydict(cdict)

def llapi_set_kernel_status(object params):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    try:
        set_kernel_status(params_dict)
    except RuntimeError as e:
        exceptionCls = getattr(NESTErrors, str(e))
        raise exceptionCls('llapi_set_kernel_status', '') from None

def llapi_simulate(float t):
    simulate(t)

def llapi_get_nc_status(NodeCollectionObject nc):
    cdef dictionary statuses = get_nc_status(nc.thisptr)
    # return vec_of_dict_to_list(statuses)
    return dictionary_to_pydict(statuses)

def llapi_set_nc_status(NodeCollectionObject nc, object params):
    cdef dictionary params_dict = pydict_to_dictionary(params)
    try:
        set_nc_status(nc.thisptr, params_dict)
    except RuntimeError as e:
        exceptionCls = getattr(NESTErrors, str(e))
        raise exceptionCls('llapi_set_nc_status', '') from None
