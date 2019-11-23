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


from ..ll_api import *
from .. import pynestkernel as kernel
from .hl_api_helper import *
from .hl_api_simulation import GetKernelStatus

import numpy

try:
    import pandas
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False

__all__ = [
    'SynapseCollection',
    'CreateParameter',
    'NodeCollection',
    'Mask',
    'Parameter',
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
    `Parameter`:
        Object representing the parameter

    Notes
    -----
    - Instead of using `CreateParameter` you can also use the various parametrizations embedded in NEST. See for
    instance :py:func:`.random.uniform`.

    **Parameter types**

    Some available parameter types (`parametertype` parameter), their function and
    acceptable keys for their corresponding specification dictionaries

    - Constant
        ::
            'constant' :
                {'value' : float} # constant value
    - Randomization
        ::
            # random parameter with uniform distribution in [min,max)
            'uniform' :
                {'min' : float, # minimum value, default: 0.0
                 'max' : float} # maximum value, default: 1.0

            # random parameter with normal distribution, optionally truncated
            # to [min,max)
            'normal':
                {'mean' : float, # mean value, default: 0.0
                 'sigma': float, # standard deviation, default: 1.0
                 'min'  : float, # minimum value, default: -inf
                 'max'  : float} # maximum value, default: +inf

            # random parameter with lognormal distribution,
            # optionally truncated to [min,max)
            'lognormal' :
                {'mu'   : float, # mean value of logarithm, default: 0.0
                 'sigma': float, # standard deviation of log, default: 1.0
                 'min'  : float, # minimum value, default: -inf
                 'max'  : float} # maximum value, default: +inf
    """
    return sli_func('CreateParameter', {parametertype: specs})


class NodeCollectionIterator(object):
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

        val = sli_func('Take', self._nc._datum, [self._increment + (self._increment >= 0)])
        self._increment += 1
        return val

    next = __next__  # Python2.x


class NodeCollection(object):
    """
    Class for `NodeCollection`.

    `NodeCollection` represents the nodes of a network. The class supports
    iteration, concatination, indexing, slicing, membership, length, convertion to and
    from lists, test for membership, and test for equality. By using the
    membership functions ``get()`` and ``set()``, you can get and set desired
    parameters.

    A `NodeCollection` is created by the :py:func:`.Create` function, or by converting a
    list of nodes to a `NodeCollection` with ``nest.NodeCollection(list)``.

    If your nodes have spatial extent, use the member parameter ``spatial`` to get the spatial information.

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

    def __init__(self, data):
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
                start = 1
            else:
                start = key.start + 1 if key.start >= 0 else key.start
            if key.stop is None:
                stop = self.__len__()
            else:
                stop = key.stop if key.stop >= 0 else key.stop
            step = 1 if key.step is None else key.step

            return sli_func('Take', self._datum, [start, stop, step])
        elif isinstance(key, (int, numpy.integer)):
            return sli_func('Take', self._datum, [key + (key >= 0)])
        else:
            raise IndexError('only integers and slices are valid indices')

    def __contains__(self, node_id):
        return sli_func('MemberQ', self._datum, node_id)

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
             Whether the returned data should be in a Pandas DataFrame or in a
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
        set
        """
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
            Dictionary of parameters or list of dictionaries of parameters of
            same length as the `NodeCollection`.
        kwargs : keyword argument pairs
            Named arguments of parameters of the elements in the `NodeCollection`.

        Raises
        ------
        TypeError
            If the input params are of the wrong form.
        KeyError
            If the specified parameter does not exist for the nodes.
        """

        if kwargs and params is None:
            params = kwargs
        elif kwargs and params:
            raise TypeError("must either provide params or kwargs, but not both.")

        if isinstance(params, dict) and self[0].get('local'):

            contains_list = [is_iterable(vals) and not is_iterable(self[0].get(key)) for key, vals in params.items()]

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
            raise TypeError(
                "status dict must be a dict, or a list of dicts of length len(nodes)")

        sli_func('SetStatus', self._datum, params)

    def tolist(self):
        """
        Convert `NodeCollection` to list.
        """
        if self.__len__() == 0:
            return []
        return list(self.get('global_id')) if self.__len__() > 1 else [self.get('global_id')]

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

    def __getattr__(self, attr):
        if attr == 'spatial':
            metadata = sli_func('GetMetadata', self._datum)
            val = metadata if metadata else None
            super().__setattr__(attr, val)
            return self.spatial
        return self.get(attr)

    def __setattr__(self, attr, value):
        # `_datum` is the only property of NodeCollection that should not be
        # interpreted as a property of the model
        if attr == '_datum':
            super().__setattr__(attr, value)
        else:
            self.set({attr: value})


class SynapseCollectionIterator(object):
    """
    Iterator class for SynapseCollection.
    """

    def __init__(self, synapse_collection):
        self._iter = iter(synapse_collection._datum)

    def __iter__(self):
        return self

    def __next__(self):
        return SynapseCollection(next(self._iter))

    next = __next__  # Python2.x


class SynapseCollection(object):
    """
    Class for Connections.

    SynapseCollection represents the connections of a network. The class supports indexing, iteration, length and
    equality. You can get and set connection parameters by using the membership functions ``get()`` and
    ``set()``, respectively. By using the membership function ``sources()`` you get an iterator over source
    nodes, while ``targets()`` returns an interator over the target nodes of the connections.

    A SynapseCollection is created by the :py:func`.GetConnections` function.
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
            *--------*-------------*
            | source | 1, 1, 2, 2, |
            *--------*-------------*
            | target | 1, 2, 1, 2, |
            *--------*-------------*
        """
        srcs = self.get('source')
        trgt = self.get('target')

        if isinstance(srcs, int):
            srcs = [srcs]
        if isinstance(trgt, int):
            trgt = [trgt]

        # 35 is arbitrarily chosen.
        if len(srcs) < 35:
            source = '| source | ' + ''.join(str(e)+', ' for e in srcs) + '|'
            target = '| target | ' + ''.join(str(e)+', ' for e in trgt) + '|'
        else:
            source = ('| source | ' + ''.join(str(e)+', ' for e in srcs[:15]) +
                      '... ' + ''.join(str(e)+', ' for e in srcs[-15:]) + '|')
            target = ('| target | ' + ''.join(str(e)+', ' for e in trgt[:15]) +
                      '... ' + ''.join(str(e)+', ' for e in trgt[-15:]) + '|')

        borderline_s = '*--------*' + '-'*(len(source) - 12) + '-*'
        borderline_t = '*--------*' + '-'*(len(target) - 12) + '-*'
        borderline_m = max(borderline_s, borderline_t)

        result = (borderline_s + '\n' + source + '\n' + borderline_m + '\n' +
                  target + '\n' + borderline_t)
        return result

    def __getattr__(self, attr):
        return self.get(attr)

    def __setattr__(self, attr, value):
        # `_datum` is the only property of SynapseCollection that should not be
        # interpreted as a property of the model
        if attr == '_datum':
            super().__setattr__(attr, value)
        else:
            self.set({attr: value})

    def sources(self):
        """Return iterator containing the source node IDs of the `SynapseCollection`."""
        sources = self.get('source')
        if not isinstance(sources, (list, tuple)):
            sources = (sources,)
        return iter(sources)

    def targets(self):
        """Return iterator containing the target node IDs of the `SynapseCollection`."""
        targets = self.get('target')
        if not isinstance(targets, (list, tuple)):
            targets = (targets,)
        return iter(targets)

    def get(self, keys=None, output=''):
        """
        Return a parameter dictionary of the connections.

        If keys is a string, a list of values is returned, unless we have a
        single connection, in which case the single value is returned.
        keys may also be a list, in which case a dictionary with a list of
        values is returned.

        Parameters
        ----------
        keys : str or list, optional
            String or a list of strings naming model properties. get
            then returns a single value or a dictionary with lists of values
            belonging to the keys given.
        output : str, ['pandas','json'], optional
            Whether the returned data should be in a Pandas DataFrame or in a
            JSON serializable format.

        Returns
        -------
        dict:
            All parameters, or, if keys is a list of strings, a dictionary with
            lists of corresponding parameters
        type:
            If keys is a string, the corrsponding parameter(s) is returned


        Raises
        ------
        TypeError
            If input params are of the wrong form.
        KeyError
            If the specified parameter does not exist for the connections.
        """
        pandas_output = output == 'pandas'
        if pandas_output and not HAVE_PANDAS:
            raise ImportError('Pandas could not be imported')

        # Return empty tuple if we have no connections or if we have done a
        # nest.ResetKernel()
        num_conn = GetKernelStatus('num_connections')
        if self.__len__() == 0 or num_conn == 0:
            return ()

        if keys is None:
            cmd = 'GetStatus'
        elif is_literal(keys):
            cmd = 'GetStatus {{ /{0} get }} Map'.format(keys)
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
            Dictionary of parameters or list of dictionaries of parameters of
            same length as the `SynapseCollection`.
        kwargs : keyword argument pairs
            Named arguments of parameters of the elements in the `SynapseCollection`.

        Raises
        ------
        TypeError
            If input params are of the wrong form.
        KeyError
            If the specified parameter does not exist for the connections.
        """

        # This was added to ensure that the function is a nop (instead of,
        # for instance, raising an exception) when applied to an empty
        # SynapseCollection, or after having done a nest.ResetKernel().
        if self.__len__() == 0 or GetKernelStatus()['network_size'] == 0:
            return

        if (isinstance(params, (list, tuple)) and
                self.__len__() != len(params)):
            raise TypeError(
                "status dict must be a dict, or a list of dicts of length "
                "len(nodes)")

        if kwargs and params is None:
            params = kwargs
        elif kwargs and params:
            raise TypeError("must either provide params or kwargs, but not both.")

        if isinstance(params, dict):
            contains_list = [is_iterable(vals) and not is_iterable(self[0].get(key)) for key, vals in params.items()]

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


class Mask(object):
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


class Parameter(object):
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
                P = nest.CreateParameter('normal', {'mean': 0.0, 'sigma': 1.0})

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
