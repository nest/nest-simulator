# -*- coding: utf-8 -*-
#
# hl_api_types.py
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

"""
Classes defining the different PyNEST types
"""

from ..ll_api import check_stack, sli_func, sps, sr, spp, take_array_index
from .. import pynestkernel as kernel
from .hl_api_helper import (
    broadcast,
    get_parameters,
    get_parameters_hierarchical_addressing,
    is_iterable,
    is_literal,
    restructure_data,
)
from .hl_api_simulation import GetKernelStatus

import numpy
import json
from math import floor, log

try:
    import pandas
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False

__all__ = [
    'CollocatedSynapses',
    'Compartments',
    'CreateParameter',
    'Mask',
    'NodeCollection',
    'Parameter',
    'Receptors',
    'serializable',
    'SynapseCollection',
    'to_json',
]


def CreateParameter(parametertype, specs):
    """
    Create a parameter.

    Parameters
    ----------
    parametertype : string
        Parameter type with or without distance dependency.
        Can be one of the following: 'constant', 'linear', 'exponential', 'gaussian', 'gaussian2D',
        'uniform', 'normal', 'lognormal', 'distance', 'position'
    specs : dict
        Dictionary specifying the parameters of the provided
        `parametertype`, see **Parameter types**.


    Returns
    -------
    ``Parameter``:
        Object representing the parameter

    Notes
    -----
    - Instead of using `CreateParameter` you can also use the various parametrizations embedded in NEST. See for
    instance :py:func:`.uniform`.

    **Parameter types**

    Examples of available parameter types (`parametertype` parameter), with their function and
    acceptable keys for their corresponding specification dictionaries:

    * Constant
        ::

            'constant' :
                {'value' : float} # constant value
    * Randomization
        ::

            # random parameter with uniform distribution in [min,max)
            'uniform' :
                {'min' : float, # minimum value, default: 0.0
                 'max' : float} # maximum value, default: 1.0

            # random parameter with normal distribution
            'normal':
                {'mean' : float, # mean value, default: 0.0
                 'std'  : float} # standard deviation, default: 1.0

            # random parameter with lognormal distribution
            'lognormal' :
                {'mean' : float, # mean value of logarithm, default: 0.0
                 'std'  : float} # standard deviation of log, default: 1.0
    """
    return sli_func('CreateParameter', {parametertype: specs})


class NodeCollectionIterator:
    """
    Iterator class for `NodeCollection`.

    Returns
    -------
    `NodeCollection`:
        Single node ID `NodeCollection` of respective iteration.
    """

    def __init__(self, nc):
        self._nc = nc
        self._increment = 0

    def __iter__(self):
        return self

    def __next__(self):
        if self._increment > len(self._nc) - 1:
            raise StopIteration

        val = sli_func('Take_g_a', self._nc._datum, [self._increment, self._increment + 1, 1])
        self._increment += 1
        return val


class NodeCollection:
    """
    Class for `NodeCollection`.

    `NodeCollection` represents the nodes of a network. The class supports
    iteration, concatenation, indexing, slicing, membership, length, conversion to and
    from lists, test for membership, and test for equality. By using the
    membership functions :py:func:`get()` and :py:func:`set()`, you can get and set desired
    parameters.

    A `NodeCollection` is created by the :py:func:`.Create` function, or by converting a
    list of nodes to a `NodeCollection` with ``nest.NodeCollection(list)``.

    If your nodes have spatial extent, use the member parameter ``spatial`` to get the spatial information.

    Slicing a NodeCollection follows standard Python slicing syntax: nc[start:stop:step], where start and stop
    gives the zero-indexed right-open range of nodes, and step gives the step length between nodes. The step must
    be strictly positive.

    Example
    -------
        ::

            import nest

            nest.ResetKernel()

            # Create NodeCollection representing nodes
            nc = nest.Create('iaf_psc_alpha', 10)

            # Convert from list
            node_ids_in = [2, 4, 6, 8]
            new_nc = nest.NodeCollection(node_ids_in)

            # Convert to list
            nc_list =  nc.tolist()

            # Concatenation
            Enrns = nest.Create('aeif_cond_alpha', 600)
            Inrns = nest.Create('iaf_psc_alpha', 400)
            nrns = Enrns + Inrns

            # Slicing and membership
            print(new_nc[2])
            print(new_nc[1:2])
            6 in new_nc
    """

    _datum = None

    def __init__(self, data=None):
        if data is None:
            data = []
        if isinstance(data, kernel.SLIDatum):
            if data.dtype != "nodecollectiontype":
                raise TypeError("Need NodeCollection Datum.")
            self._datum = data
        else:
            # Data from user, must be converted to datum
            # Data can be anything that can be converted to a NodeCollection,
            # such as list, tuple, etc.
            nc = sli_func('cvnodecollection', data)
            self._datum = nc._datum

    def __iter__(self):
        return NodeCollectionIterator(self)

    def __add__(self, other):
        if not isinstance(other, NodeCollection):
            raise NotImplementedError()

        return sli_func('join', self._datum, other._datum)

    def __getitem__(self, key):
        if isinstance(key, slice):
            if key.start is None:
                start = 0
            else:
                start = key.start
                if abs(start) > self.__len__():
                    raise IndexError('slice start value outside of the NodeCollection')
            if key.stop is None:
                stop = self.__len__()
            else:
                stop = key.stop
                if abs(stop) > self.__len__():
                    raise IndexError('slice stop value outside of the NodeCollection')
            step = 1 if key.step is None else key.step
            if step < 1:
                raise IndexError('slicing step for NodeCollection must be strictly positive')

            return sli_func('Take_g_a', self._datum, [start, stop, step])
        elif isinstance(key, (int, numpy.integer)):
            if key >= self.__len__() or key + self.__len__() < 0:
                raise IndexError('index value outside of the NodeCollection')
            return sli_func('Take_g_a', self._datum, [key, key + 1, 1])
        elif isinstance(key, (list, tuple)):
            if len(key) == 0:
                return NodeCollection([])
            # Must check if elements are bool first, because bool inherits from int
            if all(isinstance(x, bool) for x in key):
                if len(key) != len(self):
                    raise IndexError('Bool index array must be the same length as NodeCollection')
                np_key = numpy.array(key, dtype=bool)
            # Checking that elements are not instances of bool too, because bool inherits from int
            elif all(isinstance(x, int) and not isinstance(x, bool) for x in key):
                np_key = numpy.array(key, dtype=numpy.uint64)
                if len(numpy.unique(np_key)) != len(np_key):
                    raise ValueError('All node IDs in a NodeCollection have to be unique')
            else:
                raise TypeError('Indices must be integers or bools')
            return take_array_index(self._datum, np_key)
        elif isinstance(key, numpy.ndarray):
            if len(key) == 0:
                return NodeCollection([])
            if len(key.shape) != 1:
                raise TypeError('NumPy indices must one-dimensional')
            is_booltype = numpy.issubdtype(key.dtype, numpy.dtype(bool).type)
            if not (is_booltype or numpy.issubdtype(key.dtype, numpy.integer)):
                raise TypeError('NumPy indices must be an array of integers or bools')
            if is_booltype and len(key) != len(self):
                raise IndexError('Bool index array must be the same length as NodeCollection')
            if not is_booltype and len(numpy.unique(key)) != len(key):
                raise ValueError('All node IDs in a NodeCollection have to be unique')
            return take_array_index(self._datum, key)
        else:
            raise IndexError('only integers, slices, lists, tuples, and numpy arrays are valid indices')

    def __contains__(self, node_id):
        return sli_func('InCollection', self._datum, node_id)

    def __eq__(self, other):
        if not isinstance(other, NodeCollection):
            raise NotImplementedError('Cannot compare NodeCollection to {}'.format(type(other).__name__))

        if self.__len__() != other.__len__():
            return False

        return sli_func('eq', self, other)

    def __neq__(self, other):
        if not isinstance(other, NodeCollection):
            raise NotImplementedError()

        return not self == other

    def __len__(self):
        return sli_func('size', self._datum)

    def __str__(self):
        return sli_func('pcvs', self._datum)

    def __repr__(self):
        return sli_func('pcvs', self._datum)

    def get(self, *params, **kwargs):
        """
        Get parameters from nodes.

        Parameters
        ----------
        params : str or list, optional
            Parameters to get from the nodes. It must be one of the following:

            - A single string.
            - A list of strings.
            - One or more strings, followed by a string or list of strings.
              This is for hierarchical addressing.
        output : str, ['pandas','json'], optional
             If the returned data should be in a Pandas DataFrame or in a
             JSON serializable format.

        Returns
        -------
        int or float:
            If there is a single node in the `NodeCollection`, and a single
            parameter in params.
        array_like:
            If there are multiple nodes in the `NodeCollection`, and a single
            parameter in params.
        dict:
            If there are multiple parameters in params. Or, if no parameters
            are specified, a dictionary containing aggregated parameter-values
            for all nodes is returned.
        DataFrame:
            Pandas Data frame if output should be in pandas format.

        Raises
        ------
        TypeError
            If the input params are of the wrong form.
        KeyError
            If the specified parameter does not exist for the nodes.

        See Also
        --------
        :py:func:`set`,
        :py:func:`GetStatus()<nest.lib.hl_api_info.GetStatus>`,
        :py:func:`SetStatus()<nest.lib.hl_api_info.SetStatus>`

        Examples
        --------

        >>>    nodes.get()
               {'archiver_length': (0, 0, 0),
               'beta_Ca': (0.001, 0.001, 0.001),
               'C_m': (250.0, 250.0, 250.0),
               ...
               'V_th': (-55.0, -55.0, -55.0),
               'vp': (0, 0, 0)}

        >>>    nodes.get('V_m')
               (-70.0, -70.0, -70.0)

        >>>    nodes[0].get('V_m')
               -70.0

        >>>    nodes.get('V_m', 'C_m')
               {'V_m': (-70.0, -70.0, -70.0), 'C_m': (250.0, 250.0, 250.0)}

        >>>    voltmeter.get('events', 'senders')
               array([...], dtype=int64)
        """

        if not self:
            raise ValueError('Cannot get parameter of empty NodeCollection')

        # ------------------------- #
        #      Checks of input      #
        # ------------------------- #
        if not kwargs:
            output = ''
        elif 'output' in kwargs:
            output = kwargs['output']
            if output == 'pandas' and not HAVE_PANDAS:
                raise ImportError('Pandas could not be imported')
        else:
            raise TypeError('Got unexpected keyword argument')

        pandas_output = output == 'pandas'

        if len(params) == 0:
            # get() is called without arguments
            result = sli_func('get', self._datum)
        elif len(params) == 1:
            # params is a tuple with a string or list of strings
            result = get_parameters(self, params[0])
            if params[0] == 'compartments':
                result = Compartments(self, result)
            elif params[0] == 'receptors':
                result = Receptors(self, result)
        else:
            # Hierarchical addressing
            result = get_parameters_hierarchical_addressing(self, params)

        if pandas_output:
            index = self.get('global_id')
            if len(params) == 1 and is_literal(params[0]):
                # params is a string
                result = {params[0]: result}
            elif len(params) > 1 and is_literal(params[1]):
                # hierarchical, single string
                result = {params[1]: result}
            if len(self) == 1:
                index = [index]
                result = {key: [val] for key, val in result.items()}
            result = pandas.DataFrame(result, index=index)
        elif output == 'json':
            result = to_json(result)

        return result

    def set(self, params=None, **kwargs):
        """
        Set the parameters of nodes to params.

        NB! This is almost the same implementation as `SetStatus`.

        If `kwargs` is given, it has to be names and values of an attribute as keyword argument pairs. The values
        can be single values or list of the same size as the `NodeCollection`.

        Parameters
        ----------
        params : str or dict or list
            Dictionary of parameters (either lists or single values) or list of dictionaries of parameters
            of same length as the `NodeCollection`.
        kwargs : keyword argument pairs
            Named arguments of parameters of the elements in the `NodeCollection`.

        Raises
        ------
        TypeError
            If the input params are of the wrong form.
        KeyError
            If the specified parameter does not exist for the nodes.

        See Also
        --------
        :py:func:`get`,
        :py:func:`SetStatus()<nest.lib.hl_api_info.SetStatus>`,
        :py:func:`GetStatus()<nest.lib.hl_api_info.GetStatus>`
        """

        if not self:
            return
        if kwargs and params is None:
            params = kwargs
        elif kwargs and params:
            raise TypeError("must either provide params or kwargs, but not both.")

        local_nodes = [self.local] if len(self) == 1 else self.local

        if isinstance(params, dict) and 'compartments' in params:
            if isinstance(params['compartments'], Compartments):
                params['compartments'] = params['compartments'].get_tuple()
            elif params['compartments'] is None:
                # Adding compartments has been handled by the += operator, so we can remove the entry.
                params.pop('compartments')

        if isinstance(params, dict) and 'receptors' in params:
            if isinstance(params['receptors'], Receptors):
                params['receptors'] = params['receptors'].get_tuple()
            elif params['receptors'] is None:
                # Adding receptors has been handled by the += operator, so we can remove the entry.
                params.pop('receptors')

        if isinstance(params, dict) and all(local_nodes):

            node_params = self[0].get()
            contains_list = [is_iterable(vals) and key in node_params and not is_iterable(node_params[key]) for
                             key, vals in params.items()]

            if any(contains_list):
                temp_param = [{} for _ in range(self.__len__())]

                for key, vals in params.items():
                    if not is_iterable(vals):
                        for temp_dict in temp_param:
                            temp_dict[key] = vals
                    else:
                        for i, temp_dict in enumerate(temp_param):
                            temp_dict[key] = vals[i]
                params = temp_param

        if (isinstance(params, (list, tuple)) and self.__len__() != len(params)):
            raise TypeError("status dict must be a dict, or a list of dicts of length {} ".format(self.__len__()))

        sli_func('SetStatus', self._datum, params)

    def tolist(self):
        """
        Convert `NodeCollection` to list.
        """
        if self.__len__() == 0:
            return []

        return (list(self.get('global_id')) if len(self) > 1
                else [self.get('global_id')])

    def index(self, node_id):
        """
        Find the index of a node ID in the `NodeCollection`.

        Parameters
        ----------
        node_id : int
            Global ID to be found.

        Raises
        ------
        ValueError
            If the node ID is not in the `NodeCollection`.
        """
        index = sli_func('Find', self._datum, node_id)

        if index == -1:
            raise ValueError('{} is not in NodeCollection'.format(node_id))

        return index

    def __bool__(self):
        """Converts the NodeCollection to a bool. False if it is empty, True otherwise."""
        return len(self) > 0

    def __array__(self, dtype=None):
        """Convert the NodeCollection to a NumPy array."""
        return numpy.array(self.tolist(), dtype=dtype)

    def __getattr__(self, attr):
        if not self:
            raise AttributeError('Cannot get attribute of empty NodeCollection')

        if attr == 'spatial':
            metadata = sli_func('GetMetadata', self._datum)
            val = metadata if metadata else None
            super().__setattr__(attr, val)
            return self.spatial

        # NumPy compatibility check:
        # raises AttributeError to tell NumPy that interfaces other than
        # __array__ are not available (otherwise get_parameters would be
        # queried, KeyError would be raised, and all would crash)
        if attr.startswith('__array_'):
            raise AttributeError

        return self.get(attr)

    def __setattr__(self, attr, value):
        # `_datum` is the only property of NodeCollection that should not be
        # interpreted as a property of the model
        if attr == '_datum':
            super().__setattr__(attr, value)
        else:
            self.set({attr: value})


class SynapseCollectionIterator:
    """
    Iterator class for SynapseCollection.
    """

    def __init__(self, synapse_collection):
        self._iter = iter(synapse_collection._datum)

    def __iter__(self):
        return self

    def __next__(self):
        return SynapseCollection(next(self._iter))


class SynapseCollection:
    """
    Class for Connections.

    `SynapseCollection` represents the connections of a network. The class supports indexing, iteration, length and
    equality. You can get and set connection parameters by using the membership functions :py:func:`get()` and
    :py:func:`set()`. By using the membership function :py:func:`sources()` you get an iterator over
    source nodes, while :py:func:`targets()` returns an interator over the target nodes of the connections.

    A SynapseCollection is created by the :py:func:`.GetConnections` function.
    """

    _datum = None

    def __init__(self, data):

        if isinstance(data, list):
            for datum in data:
                if (not isinstance(datum, kernel.SLIDatum) or
                        datum.dtype != "connectiontype"):
                    raise TypeError("Expected Connection Datum.")
            self._datum = data
        elif data is None:
            # We can have an empty SynapseCollection if there are no connections.
            self._datum = data
        else:
            if (not isinstance(data, kernel.SLIDatum) or
                    data.dtype != "connectiontype"):
                raise TypeError("Expected Connection Datum.")
            # self._datum needs to be a list of Connection datums.
            self._datum = [data]

        self.print_full = False

    def __iter__(self):
        return SynapseCollectionIterator(self)

    def __len__(self):
        if self._datum is None:
            return 0
        return len(self._datum)

    def __eq__(self, other):
        if not isinstance(other, SynapseCollection):
            raise NotImplementedError()

        if self.__len__() != other.__len__():
            return False
        self_get = self.get(['source', 'target', 'target_thread',
                             'synapse_id', 'port'])
        other_get = other.get(['source', 'target', 'target_thread',
                               'synapse_id', 'port'])
        if self_get != other_get:
            return False
        return True

    def __neq__(self, other):
        if not isinstance(other, SynapseCollection):
            raise NotImplementedError()
        return not self == other

    def __getitem__(self, key):
        if isinstance(key, slice):
            return SynapseCollection(self._datum[key])
        else:
            return SynapseCollection([self._datum[key]])

    def __str__(self):
        """
        Printing a `SynapseCollection` returns something of the form:

             source   target   synapse model   weight   delay
            -------- -------- --------------- -------- -------
                  1        4  static_synapse    1.000   1.000
                  2        4  static_synapse    2.000   1.000
                  1        3    stdp_synapse    4.000   1.000
                  1        4    stdp_synapse    3.000   1.000
                  2        3    stdp_synapse    3.000   1.000
                  2        4    stdp_synapse    2.000   1.000

        If your SynapseCollection has more than 36 elements, only the first and last 15 connections are printed. To
        display all, first set `print_full = True`.

        ::

            conns = nest.GetConnections()
            conns.print_full = True
            print(conns)
        """

        def format_row_(s, t, sm, w, dly):
            try:
                return f'{s:>{src_len-1}d} {t:>{trg_len}d} {sm:>{sm_len}s} {w:>#{w_len}.{4}g} {dly:>#{d_len}.{4}g}'
            except ValueError:
                # Used when we have many connections and print_full=False
                return f'{s:>{src_len-1}} {t:>{trg_len}} {sm:>{sm_len}} {w:>{w_len}} {dly:>{d_len}}'

        MAX_SIZE_FULL_PRINT = 35  # 35 is arbitrarily chosen.

        params = self.get()

        if len(params) == 0:
            return 'The synapse collection does not contain any connections.'

        srcs = params['source']
        trgt = params['target']
        wght = params['weight']
        dlay = params['delay']
        s_model = params['synapse_model']

        if isinstance(srcs, int):
            srcs = [srcs]
            trgt = [trgt]
            wght = [wght]
            dlay = [dlay]
            s_model = [s_model]

        src_h = 'source'
        trg_h = 'target'
        sm_h = 'synapse model'
        w_h = 'weight'
        d_h = 'delay'

        # Find maximum number of characters for each column, used to determine width of column
        src_len = max(len(src_h) + 2, floor(log(max(srcs), 10)))
        trg_len = max(len(trg_h) + 2, floor(log(max(trgt), 10)))
        sm_len = max(len(sm_h) + 2, len(max(s_model, key=len)))
        w_len = len(w_h) + 2
        d_len = len(d_h) + 2

        # 35 is arbitrarily chosen.
        if len(srcs) >= MAX_SIZE_FULL_PRINT and not self.print_full:
            # u'\u22EE ' is the unicode for vertical ellipsis, used when we have many connections
            srcs = srcs[:15] + [u'\u22EE '] + srcs[-15:]
            trgt = trgt[:15] + [u'\u22EE '] + trgt[-15:]
            wght = wght[:15] + [u'\u22EE '] + wght[-15:]
            dlay = dlay[:15] + [u'\u22EE '] + dlay[-15:]
            s_model = s_model[:15] + [u'\u22EE '] + s_model[-15:]

        headers = f'{src_h:^{src_len}} {trg_h:^{trg_len}} {sm_h:^{sm_len}} {w_h:^{w_len}} {d_h:^{d_len}}' + '\n'
        borders = '-'*src_len + ' ' + '-'*trg_len + ' ' + '-'*sm_len + ' ' + '-'*w_len + ' ' + '-'*d_len + '\n'
        output = '\n'.join(format_row_(s, t, sm, w, d) for s, t, sm, w, d in zip(srcs, trgt, s_model, wght, dlay))
        result = headers + borders + output

        return result

    def __getattr__(self, attr):
        if attr == 'distance':
            dist = sli_func('Distance', self._datum)
            super().__setattr__(attr, dist)
            return self.distance

        return self.get(attr)

    def __setattr__(self, attr, value):
        # `_datum` is the only property of SynapseCollection that should not be
        # interpreted as a property of the model
        if attr == '_datum' or attr == 'print_full':
            super().__setattr__(attr, value)
        else:
            self.set({attr: value})

    def sources(self):
        """Returns iterator containing the source node IDs of the `SynapseCollection`."""
        sources = self.get('source')
        if not isinstance(sources, (list, tuple)):
            sources = (sources,)
        return iter(sources)

    def targets(self):
        """Returns iterator containing the target node IDs of the `SynapseCollection`."""
        targets = self.get('target')
        if not isinstance(targets, (list, tuple)):
            targets = (targets,)
        return iter(targets)

    def get(self, keys=None, output=''):
        """
        Return a parameter dictionary of the connections.

        If `keys` is a string, a list of values is returned, unless we have a
        single connection, in which case the single value is returned.
        `keys` may also be a list, in which case a dictionary with a list of
        values is returned.

        Parameters
        ----------
        keys : str or list, optional
            String or a list of strings naming model properties. get
            then returns a single value or a dictionary with lists of values
            belonging to the given `keys`.
        output : str, ['pandas','json'], optional
            If the returned data should be in a Pandas DataFrame or in a
            JSON serializable format.

        Returns
        -------
        dict:
            All parameters, or, if keys is a list of strings, a dictionary with
            lists of corresponding parameters
        type:
            If keys is a string, the corresponding parameter(s) is returned


        Raises
        ------
        TypeError
            If input params are of the wrong form.
        KeyError
            If the specified parameter does not exist for the connections.

        See Also
        --------
        set

        Examples
        --------

        >>>    conns.get()
               {'delay': [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
                ...
                'weight': [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]}

        >>>    conns.get('weight')
               [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]

        >>>    conns[0].get('weight')
               1.0

        >>>    nodes.get(['source', 'weight'])
               {'source': [1, 1, 1, 2, 2, 2, 3, 3, 3],
                'weight': [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]}
        """
        pandas_output = output == 'pandas'
        if pandas_output and not HAVE_PANDAS:
            raise ImportError('Pandas could not be imported')

        # Return empty dictionary if we have no connections or if we have done a nest.ResetKernel()
        num_conns = GetKernelStatus('num_connections')  # Has to be called first because it involves MPI communication.
        if self.__len__() == 0 or num_conns == 0:
            # Return empty tuple if get is called with an argument
            return {} if keys is None else ()

        if keys is None:
            cmd = 'GetStatus'
        elif is_literal(keys):
            #  Extracting the correct values will be done in restructure_data below
            cmd = 'GetStatus'
        elif is_iterable(keys):
            keys_str = " ".join("/{0}".format(x) for x in keys)
            cmd = 'GetStatus {{ [ [ {0} ] ] get }} Map'.format(keys_str)
        else:
            raise TypeError("keys should be either a string or an iterable")

        sps(self._datum)
        sr(cmd)
        result = spp()

        # Need to restructure the data.
        final_result = restructure_data(result, keys)

        if pandas_output:
            index = (self.get('source') if self.__len__() > 1 else
                     (self.get('source'),))
            if is_literal(keys):
                final_result = {keys: final_result}
            final_result = pandas.DataFrame(final_result, index=index)
        elif output == 'json':
            final_result = to_json(final_result)

        return final_result

    def set(self, params=None, **kwargs):
        """
        Set the parameters of the connections to `params`.

        NB! This is almost the same implementation as SetStatus

        If `kwargs` is given, it has to be names and values of an attribute as keyword argument pairs. The values
        can be single values or list of the same size as the `SynapseCollection`.

        Parameters
        ----------
        params : str or dict or list
            Dictionary of parameters (either lists or single values) or list of dictionaries of parameters
            of same length as `SynapseCollection`.
        kwargs : keyword argument pairs
            Named arguments of parameters of the elements in the `SynapseCollection`.

        Raises
        ------
        TypeError
            If input params are of the wrong form.
        KeyError
            If the specified parameter does not exist for the connections.

        See Also
        --------
        get
        """

        # This was added to ensure that the function is a nop (instead of,
        # for instance, raising an exception) when applied to an empty
        # SynapseCollection, or after having done a nest.ResetKernel().
        if self.__len__() == 0 or GetKernelStatus('network_size') == 0:
            return

        if (isinstance(params, (list, tuple)) and
                self.__len__() != len(params)):
            raise TypeError("status dict must be a dict, or a list of dicts of length {}".format(self.__len__()))

        if kwargs and params is None:
            params = kwargs
        elif kwargs and params:
            raise TypeError("must either provide params or kwargs, but not both.")

        if isinstance(params, dict):
            node_params = self[0].get()
            contains_list = [is_iterable(vals) and key in node_params and not is_iterable(node_params[key]) for
                             key, vals in params.items()]

            if any(contains_list):
                temp_param = [{} for _ in range(self.__len__())]

                for key, vals in params.items():
                    if not is_iterable(vals):
                        for temp_dict in temp_param:
                            temp_dict[key] = vals
                    else:
                        for i, temp_dict in enumerate(temp_param):
                            temp_dict[key] = vals[i]
                params = temp_param

        params = broadcast(params, self.__len__(), (dict,), "params")

        sps(self._datum)
        sps(params)

        sr('2 arraystore')
        sr('Transpose { arrayload pop SetStatus } forall')

    def disconnect(self):
        """
        Disconnect the connections in the `SynapseCollection`.
        """
        sps(self._datum)
        sr('Disconnect_a')


class CollocatedSynapses:
    """
    Class for collocated synapse specifications.

    Wrapper around a list of specifications, used when calling :py:func:`.Connect`.

    Example
    -------

        ::

            nodes = nest.Create('iaf_psc_alpha', 3)
            syn_spec = nest.CollocatedSynapses({'weight': 4., 'delay': 1.5},
                                               {'synapse_model': 'stdp_synapse'},
                                               {'synapse_model': 'stdp_synapse', 'alpha': 3.})
            nest.Connect(nodes, nodes, conn_spec='one_to_one', syn_spec=syn_spec)

            conns = nest.GetConnections()

            print(conns.alpha)
            print(len(syn_spec))
    """

    def __init__(self, *args):
        self.syn_specs = args

    def __len__(self):
        return len(self.syn_specs)


class Mask:
    """
    Class for spatial masks.

    Masks are used when creating connections when nodes have spatial extent. A mask
    describes the area of the pool population that shall be searched to find nodes to
    connect to for any given node in the driver population. Masks are created using
    the :py:func:`.CreateMask` command.
    """

    _datum = None

    # The constructor should not be called by the user
    def __init__(self, datum):
        """Masks must be created using the CreateMask command."""
        if not isinstance(datum, kernel.SLIDatum) or datum.dtype != "masktype":
            raise TypeError("expected mask Datum")
        self._datum = datum

    # Generic binary operation
    def _binop(self, op, other):
        if not isinstance(other, Mask):
            raise NotImplementedError()
        return sli_func(op, self._datum, other._datum)

    def __or__(self, other):
        return self._binop("or", other)

    def __and__(self, other):
        return self._binop("and", other)

    def __sub__(self, other):
        return self._binop("sub", other)

    def Inside(self, point):
        """
        Test if a point is inside a mask.

        Parameters
        ----------
        point : tuple/list of float values
            Coordinate of point

        Returns
        -------
        out : bool
            True if the point is inside the mask, False otherwise
        """
        return sli_func("Inside", point, self._datum)


class Parameter:
    """
    Class for parameters

    A parameter may be used as a probability kernel when creating
    connections and nodes or as synaptic parameters (such as weight and delay).
    Parameters are created using the :py:func:`.CreateParameter` command.
    """

    _datum = None

    # The constructor should not be called by the user
    def __init__(self, datum):
        """Parameters must be created using the CreateParameter command."""
        if not isinstance(datum,
                          kernel.SLIDatum) or datum.dtype != "parametertype":
            raise TypeError("expected parameter datum")
        self._datum = datum

    # Generic binary operation
    def _binop(self, op, other, params=None):
        if isinstance(other, (int, float)):
            other = CreateParameter('constant', {'value': float(other)})
        if not isinstance(other, Parameter):
            raise NotImplementedError()

        if params is None:
            return sli_func(op, self._datum, other._datum)
        else:
            return sli_func(op, self._datum, other._datum, params)

    def __add__(self, other):
        return self._binop("add", other)

    def __radd__(self, other):
        return self + other

    def __sub__(self, other):
        return self._binop("sub", other)

    def __rsub__(self, other):
        return self * (-1) + other

    def __neg__(self):
        return self * (-1)

    def __mul__(self, other):
        return self._binop("mul", other)

    def __rmul__(self, other):
        return self * other

    def __div__(self, other):
        return self._binop("div", other)

    def __truediv__(self, other):
        return self._binop("div", other)

    def __pow__(self, exponent):
        return sli_func("pow", self._datum, float(exponent))

    def __lt__(self, other):
        return self._binop("compare", other, {'comparator': 0})

    def __le__(self, other):
        return self._binop("compare", other, {'comparator': 1})

    def __eq__(self, other):
        return self._binop("compare", other, {'comparator': 2})

    def __ne__(self, other):
        return self._binop("compare", other, {'comparator': 3})

    def __ge__(self, other):
        return self._binop("compare", other, {'comparator': 4})

    def __gt__(self, other):
        return self._binop("compare", other, {'comparator': 5})

    def GetValue(self):
        """
        Compute value of parameter.

        Returns
        -------
        out : value
            The value of the parameter

        See also
        --------
        CreateParameter

        Example
        -------
            ::

                import nest

                # normal distribution parameter
                P = nest.CreateParameter('normal', {'mean': 0.0, 'std': 1.0})

                # get out value
                P.GetValue()
        """
        return sli_func("GetValue", self._datum)

    def is_spatial(self):
        return sli_func('ParameterIsSpatial', self._datum)

    def apply(self, spatial_nc, positions=None):
        if positions is None:
            return sli_func('Apply', self._datum, spatial_nc)
        else:
            if len(spatial_nc) != 1:
                raise ValueError('The NodeCollection must contain a single node ID only')
            if not isinstance(positions, (list, tuple)):
                raise TypeError('Positions must be a list or tuple of positions')
            for pos in positions:
                if not isinstance(pos, (list, tuple, numpy.ndarray)):
                    raise TypeError('Each position must be a list or tuple')
                if len(pos) != len(positions[0]):
                    raise ValueError('All positions must have the same number of dimensions')
            return sli_func('Apply', self._datum, {'source': spatial_nc, 'targets': positions})


class CmBase:

    def __init__(self, node_collection, elements):
        if not isinstance(node_collection, NodeCollection):
            raise TypeError(f'node_collection must be a NodeCollection, got {type(node_collection)}')
        if not isinstance(elements, tuple):
            raise TypeError(f'elements must be a tuple of dicts, got {type(elements)}')
        self._elements = elements
        self._node_collection = node_collection

    def __add__(self, other):
        new_elements = list(self._elements)
        if isinstance(other, dict):
            new_elements += [other]
        elif isinstance(other, (tuple, list)):
            if not all(isinstance(d, dict) for d in other):
                raise TypeError(
                    f'{self.__class__.__name__} can only be added with dicts, lists of dicts, '
                    f'or other {self.__class__.__name__}')
            new_elements += list(other)
        elif isinstance(other, self.__class__):
            new_elements += list(other._elements)
        else:
            raise NotImplementedError(f'{self.__class__.__name__} can only be added with dicts, lists of dicts,'
                                      f' or other {self.__class__.__name__}, got {type(other)}')

        return self.__class__(self._node_collection, tuple(new_elements))

    def __iadd__(self, other):
        if isinstance(other, dict):
            new_elements = [other]
        elif isinstance(other, (tuple, list)):
            if not all(isinstance(d, dict) for d in other):
                raise TypeError(f'{self.__class__.__name__} can only be added with dicts, lists of dicts, '
                                f'or other {self.__class__.__name__}')
            new_elements = list(other)
        elif isinstance(other, self.__class__):
            new_elements = list(other._elements)
        else:
            raise NotImplementedError(f'{self.__class__.__name__} can only be added with dicts, lists of dicts,'
                                      f' or other {self.__class__.__name__}, got {type(other)}')
        self._node_collection.set({f'add_{self.__class__.__name__.lower()}': new_elements})
        return None  # Flagging elements as added by returning None

    def __getitem__(self, key):
        return self._elements[key]

    def __str__(self):
        return str(self._elements)

    def get_tuple(self):
        return self._elements


class Compartments(CmBase):
    # No specialization here because all is done in the base class based on the class name.
    pass


class Receptors(CmBase):
    # No specialization here because all is done in the base class based on the class name.
    pass


def serializable(data):
    """Make data serializable for JSON.

    Parameters
    ----------
    data : any

    Returns
    -------
    data_serialized : str, int, float, list, dict
        Data can be encoded to JSON
    """

    if isinstance(data, (numpy.ndarray, NodeCollection)):
        return data.tolist()
    if isinstance(data, SynapseCollection):
        # Get full information from SynapseCollection
        return serializable(data.get())
    if isinstance(data, kernel.SLILiteral):
        # Get name of SLILiteral.
        return data.name
    if isinstance(data, (list, tuple)):
        return [serializable(d) for d in data]
    if isinstance(data, dict):
        return dict([(key, serializable(value)) for key, value in data.items()])
    return data


def to_json(data, **kwargs):
    """Serialize data to JSON.

    Parameters
    ----------
    data : any
    kwargs : keyword argument pairs
        Named arguments of parameters for `json.dumps` function.

    Returns
    -------
    data_json : str
        JSON format of the data
    """

    data_serialized = serializable(data)
    data_json = json.dumps(data_serialized, **kwargs)
    return data_json
