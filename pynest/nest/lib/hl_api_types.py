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
from nest.topology import CreateTopologyParameter

import numpy

try:
    import pandas
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False

__all__ = [
    '_get_hierarchical_addressing',
    '_get_params_is_strings',
    '_restructure_data',
    'Connectome',
    'CreateParameter',
    'GIDCollection',
    'GIDCollectionIterator',
    'Mask',
    'Parameter',
    'TopologyParameter',
]


def _restructure_data(result, keys):
    """
    Restructure output data for Connectome in get.

    Parameters
    ----------
    result: list
        list of status dictionaries or list (of lists) of parameter values.
    keys: string or list of strings
        name(s) of properties

    Returns
    -------
    int, list or dict
    """
    if is_literal(keys):
        final_result = result[0] if len(result) == 1 else list(result)

    elif is_iterable(keys):
        final_result = ({key: [val[i] for val in result]
                         for i, key in enumerate(keys)} if len(result) != 1
                        else {key: val[i] for val in result
                              for i, key in enumerate(keys)})

    elif keys is None:
        final_result = ({key: [result_dict[key] for result_dict in result]
                         for key in result[0]} if len(result) != 1
                        else {key: result_dict[key] for result_dict in result
                              for key in result[0]})
    return final_result


def _get_params_is_strings(gc, param):
    """
    Get parameters from nodes.

    Used by GIDCollection.get()

    Parameters
    ----------
    gc: GIDCollection
        nodes to get values from
    param: string or list of strings
        string or list of string naming model properties.

    Returns
    -------
    int, list:
        param is a string so the value(s) is returned
    dict:
        param is a list of string so a dictionary is returned
    """
    # Single literal case
    if is_literal(param):
        cmd = '/{} get'.format(param)
        sps(gc._datum)
        try:
            sr(cmd)
            result = spp()
        except kernel.NESTError:
            result = gc.get()[param]  # If the GIDCollection is a composite.

    # Array param case
    elif is_iterable(param):
        result = {param_name: gc.get(param_name) for param_name in param}

    else:
        raise TypeError("Params should be either a string or an iterable")

    return result


def _get_hierarchical_addressing(gc, params):
    """
    Get parameters from nodes, hierarchical case.

    Used by GIDCollection.get()

    Parameters
    ----------
    gc: GIDCollection
        nodes to get values from
    params: tuple
        first value in the tuple should be a string, second can be a string
        or a list of string.
        The first value corresponds to the path into the hierarchical structure
        while the second value corresponds to the name(s) of the desired
        properties.

    Returns
    -------
    int, list:
        params[-1] is a string so the value(s) is returned
    dict:
        params[-1] is a list of string so a dictionary is returned
    """

    # Right now, NEST only allows get(arg0, arg1) for hierarchical
    # addressing, where arg0 must be a string and arg1 can be string
    # or list of strings.
    if is_literal(params[0]):
        value_list = gc.get(params[0])
        if type(value_list) != tuple:
            value_list = (value_list,)
    else:
        raise TypeError('First argument must be a string, specifying' +
                        ' path into hierarchical dictionary')

    result = _restructure_data(value_list, None)

    if is_literal(params[-1]):
        result = result[params[-1]]
    else:
        result = {key: result[key] for key in params[-1]}
    return result


def CreateParameter(parametertype, specs):
    """
    Create a parameter.

    Parameters
    ----------
    parametertype : {'constant', 'linear', 'exponential', 'gaussian', \
        'gaussian2D', 'uniform', 'normal', 'lognormal'}
        Function types with or without distance dependency
    specs : dict
        Dictionary specifying the parameters of the provided
        `'parametertype'`, see **Parameter types**.


    Returns
    -------
    out : ``Parameter`` object

    Notes
    -----
    -


    **Parameter types**

    Available parameter types (`parametertype` parameter), their function and
    acceptable keys for their corresponding specification dictionaries

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
            # or
            # random parameter with normal distribution, optionally truncated
            # to [min,max)
            'normal':
                {'mean' : float, # mean value, default: 0.0
                 'sigma': float, # standard deviation, default: 1.0
                 'min'  : float, # minimum value, default: -inf

                 'max'  : float} # maximum value, default: +inf
            # or
            # random parameter with lognormal distribution,
            # optionally truncated to [min,max)
            'lognormal' :
                {'mu'   : float, # mean value of logarithm, default: 0.0
                 'sigma': float, # standard deviation of log, default: 1.0
                 'min'  : float, # minimum value, default: -inf
                 'max'  : float} # maximum value, default: +inf


    **Example**
        ::

    """
    return sli_func('CreateParameter', {parametertype: specs})


class GIDCollectionIterator(object):
    """
    Iterator class for GIDCollection.

    Can either return the respective gid (this is the default), or a pair
    consisting of the respective gid and modelID.
    """

    def __init__(self, gc, iterpairs=False):
        self._gciter = sli_func(':beginiterator_g', gc)
        self._deref_and_increment = 'dup {} exch :next_q pop'.format(
            ':getgidmodelid_q' if iterpairs else ':getgid_q')

    def __iter__(self):
        return self

    def __next__(self):
        try:
            val = sli_func(self._deref_and_increment, self._gciter)
        except kernel.NESTError:
            raise StopIteration
        return val

    next = __next__  # Python2.x


class GIDCollection(object):
    """
    Class for GIDCollection.

    GIDCollection represents the nodes of a network. The class supports
    iteration, concatination, indexing, slicing, membership, convertion to and
    from lists, test for membership, and test for equality. By using the
    membership functions ``get(.)`` and ``set(.)``, you can get and set desired
    parameters.

    A GIDCollection is created by the ``Create`` function, or by converting a
    list of nodes to a GIDCollection with ``nest.GIDCollection(list)``.

    By iterating over the GIDCollection you get the gids. If you apply the
    ``items()`` function, you can also read the modelIDs.

    **Example**
        ::
            import nest

            nest.ResetKernel()

            # Create GIDCollection representing nodes
            gc = nest.Create('iaf_psc_alpha', 10)

            # Print gids and modelID
            for gid, mid in gc.items():
                print(gid, mid)

            # Convert from list
            gids_in = [2, 4, 6, 8]
            new_gc = nest.GIDCollection(gids_in)

            # Convert to list
            gc_list =  [x for x in gc]

            # Concatenation
            Enrns = nest.Create('aeif_cond_alpha', 600)
            Inrns = nest.Create('iaf_psc_alpha', 400)
            nrns = Enrns + Inrns

            # Slicing and membership
            print(new_gc[2])
            print(new_gc[1:2])
            6 in new_gc
    """

    _datum = None

    def __init__(self, data):
        if isinstance(data, kernel.SLIDatum):
            if data.dtype != "gidcollectiontype":
                raise TypeError("Need GIDCollection Datum.")
            self._datum = data
        else:
            # Data from user, must be converted to datum
            # Data can be anything that can be converted to a GIDCollection,
            # such as list, tuple, etc.
            gc = sli_func('cvgidcollection', data)
            self._datum = gc._datum

        # Spatial values for layers.
        self.spatial = None

    def __iter__(self):
        return GIDCollectionIterator(self)

    def items(self):
        return GIDCollectionIterator(self, True)

    def __add__(self, other):
        if not isinstance(other, GIDCollection):
            return NotImplemented
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
        else:
            return sli_func('Take', self._datum, [key + (key >= 0)])

    def __contains__(self, gid):
        return sli_func('MemberQ', self._datum, gid)

    def __eq__(self, other):
        if not isinstance(other, GIDCollection):
            return NotImplemented

        if self.__len__() != other.__len__():
            return False
        for selfpair, otherpair in zip(self.items(), other.items()):
            if selfpair != otherpair:
                return False
        return True

    def __neq__(self, other):
        if not isinstance(other, GIDCollection):
            return NotImplemented
        return not self == other

    def __len__(self):
        return sli_func('size', self._datum)

    def __str__(self):
        return sli_func('pcvs', self._datum)

    def set_spatial(self):
        """
        set spatial data to self.spatial
        """
        self.spatial = sli_func('GetMetadata', self._datum)

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
             JSON serializable format.  Default is ''.

        Returns
        -------
        int or float
            If there is a single node in the GIDCollection, and a single
            parameter in params.
        array_like
            If there are multiple nodes in the GIDCollection, and a single
            parameter in params.
        dict
            If there are multiple parameters in params. Also, if no parameters
            are specified, a dictionary containing aggregated parameter-values
            for all nodes is returned.
        DataFrame
            Pandas Data frame if output should be in pandas format.

        Raises
        ------
        TypeError
            If the input params are on the wrong form.
        KeyError
            If the specified parameter does not exist for the nodes.

        See Also
        --------
        set

        Examples
        --------
        Single parameter:

        >>> neurons.get('V_m')
        (-70.0, -70.0, ..., -70.0)

        >>> neurons[3].get('V_m')
        -70.0

        Multiple parameters:

        >>> neurons.get(['V_m', 'V_th'])
        {'V_m': (-70.0, -70.0, ..., -70.0),
         'V_th': (-55.0, -55.0, ..., -55.0)}

        Hierarchical addressing:

        >>> sd.get('events', 'senders')
        array([], dtype=int64)

        >>> sd.get('events', ['senders', 'times'])
        {'senders': array([], dtype=int64),
         'times': array([], dtype=float64)}
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
            result = _get_params_is_strings(self, params[0])
        else:
            # Hierarchical addressing
            result = _get_hierarchical_addressing(self, params)

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

    def set(self, params, val=None):
        """
        NB! This is the same implementation as SetStatus

        Set the parameters of nodes or layers to params.

        If val is given, params has to be the name of an attribute, which is
        set to val on the nodes. val can be a single value or a list of the
        same size as the GIDCollection.

        Parameters
        ----------
        params : str or dict or list
            Dictionary of parameters or list of dictionaries of parameters of
            same length as the GIDCollection. If val is given, this has to be
            the name of a model property as a str.
        val : int or list, optional
            If given, params has to be the name of a model property.

        Raises
        ------
        TypeError
            Description
        """

        if isinstance(params, dict) and self[0].get('local'):

            contains_list = [is_iterable(vals) and not
                             is_iterable(self[0].get(key))
                             for key, vals in params.items()]

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

        if val is not None and is_literal(params):
            if (is_iterable(val) and not
                    isinstance(val, (uni_str, dict))):
                params = [{params: x} for x in val]
            else:
                params = {params: val}

        if (isinstance(params, (list, tuple)) and
                self.__len__() != len(params)):
            raise TypeError(
                "status dict must be a dict, or a list of dicts of length "
                "len(nodes)")

        sli_func('SetStatus', self._datum, params)

    def tolist(self):
        return list(self)


class ConnectomeIterator(object):
    """
    Iterator class for Connectome.
    """

    def __init__(self, conn):
        self._iter = iter(conn._datum)

    def __iter__(self):
        return self

    def __next__(self):
        return Connectome(next(self._iter))

    next = __next__  # Python2.x


class Connectome(object):
    """
    Class for Connections.

    Connectome represents the connections of a network. The class supports
    get(), set(), len(), indexing, iteration and equality.

    A Connectome is created by the ``GetConnections`` function.
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
            # So we can have empty Connectome and not a tuple if there are no
            # connections.
            self._datum = data
        else:
            if (not isinstance(data, kernel.SLIDatum) or
                    data.dtype != "connectiontype"):
                raise TypeError("Expected Connection Datum.")
            # self._datum is a list of Connection datums.
            self._datum = [data]

    def __iter__(self):
        return ConnectomeIterator(self)

    def __len__(self):
        if self._datum is None:
            return 0
        return len(self._datum)

    def __eq__(self, other):
        if not isinstance(other, Connectome):
            return NotImplemented

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
        if not isinstance(other, Connectome):
            return NotImplemented
        return not self == other

    def __getitem__(self, key):
        if isinstance(key, slice):
            return Connectome(self._datum[key])
        else:
            return Connectome([self._datum[key]])

    def __str__(self):
        """
        Printing a Connectome returns something of the form:
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

    def source(self):
        """
        Return iterator containing the source gids of the connectome.
        """
        sources = self.get('source')
        if not isinstance(sources, (list, tuple)):
            sources = (sources,)
        return iter(sources)

    def target(self):
        """
        Return iterator containing the target gids of the connectome.
        """
        targets = self.get('target')
        if not isinstance(targets, (list, tuple)):
            targets = (targets,)
        return iter(targets)

    def get(self, keys=None, output=''):
        """
        Return the parameter dictionary of connections.

        If keys is a string, a list of values is returned, unless we have a
        single ConnectionDatum, in which case the single value is returned.
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
            JSON serializable format.  Default is ''.

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
            Description
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
        final_result = _restructure_data(result, keys)

        if pandas_output:
            index = (self.get('source') if self.__len__() > 1 else
                     (self.get('source'),))
            if is_literal(keys):
                final_result = {keys: final_result}
            final_result = pandas.DataFrame(final_result, index=index)
        elif output == 'json':
            final_result = to_json(final_result)

        return final_result

    def set(self, params, val=None):
        """
        NB! This is the same implementation as SetStatus

        Set the parameters of nodes or connections to params.

        If val is given, params has to be the name of an attribute, which is
        set to val on the nodes. val can be a single value or a list of the
        same size as the Connections.

        Parameters
        ----------
        params : str or dict or list
            Dictionary of parameters or list of dictionaries of parameters of
            same length as the Connections. If val is given, this has to be
            the name of a model property as a str.
        val : str, optional
            If given, params has to be the name of a model property.

        Raises
        ------
        TypeError
            Description
        """

        # This was added to ensure that the function is a nop (instead of,
        # for instance, raising an exception) when applied to an empty
        # Connectome, or after having done a nest.ResetKernel().
        if self.__len__() == 0 or GetKernelStatus()['network_size'] == 0:
            return

        if val is not None and is_literal(params):
            if is_iterable(val) and not isinstance(val, (uni_str, dict)):
                params = [{params: x} for x in val]
            else:
                params = {params: val}

        if (isinstance(params, (list, tuple)) and
                self.__len__() != len(params)):
            raise TypeError(
                "status dict must be a dict, or a list of dicts of length "
                "len(nodes)")

        params = broadcast(params, self.__len__(), (dict,), "params")

        sps(self._datum)
        sps(params)

        sr('2 arraystore')
        sr('Transpose { arrayload pop SetStatus } forall')


class Mask(object):
    """
    Class for spatial masks.

    Masks are used when creating connections in the Topology module. A mask
    describes which area of the pool layer shall be searched for nodes to
    connect for any given node in the driver layer. Masks are created using
    the ``CreateMask`` command.
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
            return NotImplemented
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
    connections or as synaptic parameters (such as weight and delay).
    Parameters are created using the ``CreateParameter`` command.
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
            return NotImplemented

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


        Notes
        -----
        -


        **Example**
            ::

                import nest

                # normal distribution parameter
                P = nest.CreateParameter('normal', {'mean': 0.0, 'sigma': 1.0})

                # get out value
                P.GetValue()

        """
        return sli_func("GetValue", self._datum)


class TopologyParameter(Parameter):
    """
    Class for parameters for distance dependency or randomization.

    Parameters are spatial functions which are used when creating
    connections in the Topology module. A parameter may be used as a
    probability kernel when creating connections or as synaptic parameters
    (such as weight and delay). Parameters are created using the
    ``CreateTopologyParameter`` command.
    """
    # The constructor should not be called by the user

    def __init__(self, datum):
        """
        Parameters must be created using the CreateTopologyParameter
        command.
        """
        if not (isinstance(datum, kernel.SLIDatum)
                or datum.dtype != "topologyparametertype"):
            raise TypeError("expected parameter datum")
        self._datum = datum

    # Generic binary operation
    def _binop(self, op, other):
        if isinstance(other, (int, float)):
            other = CreateTopologyParameter(
                'constant', {'value': float(other)})
        if not isinstance(other, Parameter):
            return NotImplemented

        return sli_func(op, self._datum, other._datum)

    def GetValue(self, point):
        """
        Compute value of parameter at a point.


        Parameters
        ----------
        point : tuple/list of float values
            coordinate of point


        Returns
        -------
        out : value
            The value of the parameter at the point


        See also
        --------
        CreateTopologyParameter : create parameter for e.g.,
        distance dependency


        Notes
        -----
        -


        **Example**
            ::

                import nest.topology as tp

                #linear dependent parameter
                P = tp.CreateTopologyParameter('linear', {'a' : 2., 'c' : 0.})

                #get out value
                P.GetValue(point=[3., 4.])

        """
        return sli_func("GetValue", point, self._datum)
