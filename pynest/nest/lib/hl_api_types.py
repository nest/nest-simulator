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

import nest

try:
    import pandas
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False


class GIDCollectionIterator(object):
    """
    Iterator class for GIDCollection.

    Can either return the respective gid (this is the default), or a pair
    consisting of the respective gid and modelID.
    """

    def __init__(self, gc, iterpairs=False):
        self._gciter = nest.sli_func(':beginiterator_g', gc)
        self._deref_and_increment = 'dup {} exch :next_q pop'.format(
            ':getgidmodelid_q' if iterpairs else ':getgid_q')

    def __iter__(self):
        return self

    def __next__(self):
        try:
            val = nest.sli_func(self._deref_and_increment, self._gciter)
        except nest.NESTError:
            raise StopIteration
        return val

    def __str__(self):
        return nest.sli_func('pcvs', self._datum)

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
        if isinstance(data, nest.SLIDatum):
            if data.dtype != "gidcollectiontype":
                raise TypeError("Need GIDCollection Datum.")
            self._datum = data
        else:
            # Data from user, must be converted to datum
            # Data can be anything that can be converted to a GIDCollection,
            # such as list, tuple, etc.
            gc = nest.sli_func('cvgidcollection', data)
            self._datum = gc._datum

    def __iter__(self):
        return GIDCollectionIterator(self)

    def items(self):
        return GIDCollectionIterator(self, True)

    def __add__(self, other):
        if not isinstance(other, GIDCollection):
            return NotImplemented
        return nest.sli_func('join', self._datum, other._datum)

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

            return nest.sli_func('Take', self._datum, [start, stop, step])
        else:
            return nest.sli_func('Take', self._datum, [key + (key >= 0)])

    def __contains__(self, gid):
        return nest.sli_func('MemberQ', self._datum, gid)

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
        return nest.sli_func('size', self._datum)

    def __str__(self):
        return nest.sli_func('pcvs', self._datum)

    def get(self, *params, **kwargs):
        """
        Get parameters from nodes or layer.

        Parameters
        ----------
        params : str or list, optional
            Parameters to get from the nodes or from the layer status. It must
            be one of the following:
            - A single string.
            - A list of strings.
            - One or more strings, followed by a string or list of strings.
              This is for hierarchical addressing.
        pandas_output : bool, optional
            Whether the returned data should be in a Pandas DataFrame or not.
            Default is not.

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
            If the pandas_output parameter is True.

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
        # TODO: Needs to be cleaned up.

        #############################
        #      Checks of input      #
        #############################
        if not kwargs:
            pandas_output = False
        elif 'pandas_output' in kwargs:
            if not HAVE_PANDAS:
                raise ImportError('Pandas could not be imported')
            pandas_output = kwargs['pandas_output']
        else:
            raise TypeError('Got unexpected keyword argument')

        #############################
        #  No specified params case #
        #############################
        if len(params) == 0:
            result = nest.sli_func('get', self._datum)
            if pandas_output:
                if 'global_id' in result:
                    index = result['global_id']
                    if isinstance(index, int):
                        index = (index,)
                        result = {key: (item,) for key, item in result.items()}
                    result = pandas.DataFrame(result, index=index)
                else:  # The GIDCollection is a layer
                    result = {key: (item,) for key, item in result.items()}
                    result = pandas.DataFrame(result,
                                              index=['layer']).transpose()
        #############################
        #     Normal addressing     #
        #############################
        elif len(params) == 1:
            param = params[0]
            # Single literal case
            if nest.is_literal(param):
                cmd = '/{} get'.format(param)
                nest.sps(self._datum)
                nest.sr(cmd)
                result = nest.spp()
                if pandas_output:
                    try:
                        index = self.get('global_id')
                        if type(index) is int:
                            index = [index]
                        if type(result) is dict:
                            # Problematic if result[key] isn't array-like
                            result = pandas.DataFrame(
                                {(param, key): result[key]
                                 if len(result[key]) != 0
                                 else [None]
                                 for key in result.keys()},
                                index=index)
                        else:
                            result = pandas.DataFrame({param: result},
                                                      index=index)
                    except nest.NESTError:  # It is (probably) a layer
                        result = pandas.DataFrame({param: (result,)},
                                                  columns=['layer'])
            # Array param case
            elif nest.is_iterable(param):
                result = {param_name: self.get(param_name)
                          for param_name in param}
                if pandas_output:
                    try:
                        index = self.get('global_id')
                        if type(index) is int:
                            index = [index]
                        p_dict = {}
                        for key, item in result.items():
                            if type(item) is dict:
                                for subkey, subitem in item.items():
                                    # Problematic if subitem isn't array-like
                                    p_dict.update({(key, subkey): subitem
                                                   if len(subitem) != 0
                                                   else [None]})
                            else:
                                p_dict.update({key: item})
                        result = pandas.DataFrame(p_dict,
                                                  index=index)
                    except nest.NESTError:  # It is (probably) a layer
                        result = pandas.DataFrame(result,
                                                  columns=['layer'])

            else:
                raise TypeError("Params should be either a string or an " +
                                "iterable")

        ##############################
        #   Hierarchical addressing  #
        ##############################
        else:
            first = True
            # Path parameters
            for param in params[:-1]:
                if nest.is_literal(param):
                    if first:
                        value_list = self.get(param)
                        if type(value_list) != tuple:
                            value_list = (value_list,)
                        first = False
                    else:
                        # TODO481 : This is never run as the max depth in the
                        # status dictionary is too low.
                        value_list = [nest.sli_func('/{} get'.format(param), d)
                                      for d in value_list]
                elif nest.is_iterable(param):
                    raise TypeError("Only the last argument can be an " +
                                    "iterable")
                else:
                    raise TypeError("Argument must be a string")
            # Value parameter, literal case
            if nest.is_literal(params[-1]):
                if len(self) == 1:
                    result = value_list[0][params[-1]]
                    if pandas_output:
                        index = self.get('global_id')
                        result = pandas.DataFrame({params[-1]: [result]
                                                   if len(result) != 0
                                                   else [[]]},
                                                  index=[index])
                else:
                    if pandas_output:
                        index = self.get('global_id')
                        result_list = [d[params[-1]] for d in value_list]
                        result = pandas.DataFrame({params[-1]: result_list
                                                   if len(result_list) != 0
                                                   else [[]]},
                                                  index=index)
                    else:
                        result = tuple([d[params[-1]] for d in value_list])

            # Value parameter, array case
            elif nest.is_iterable(params[-1]):
                for value in params[-1]:
                    # TODO481 : Assuming they are all of equal type, we check
                    # only the values of the first node. This must be changed
                    # if we decide something else.
                    if value not in value_list[0].keys():
                        raise KeyError("The value '{}' does not exist at" +
                                       "the given path".format(value))
                if len(self) == 1:  # If GIDCollection contains a single node
                    if pandas_output:
                        index = [self.get('global_id')]
                        result = {key: [item]
                                  if len(item) != 0 else [[]]
                                  for key, item in value_list[0].items()
                                  if key in params[-1]}
                        result = pandas.DataFrame(result,
                                                  index=index)
                    else:
                        result = {'{}'.format(key): value
                                  for key, value in value_list[0].items()
                                  if key in params[-1]}
                else:  # If GIDCollection contains multiple nodes
                    if pandas_output:
                        index = self.get('global_id')
                        result = pandas.DataFrame(
                            [{key: item
                              if len(item) != 0 else []
                              for key, item in d.items()
                              if key in params[-1]}
                             for d in value_list],
                            index=index)
                    else:
                        result = tuple([{'{}'.format(key): value
                                         for key, value in d.items()
                                         if key in params[-1]}
                                        for d in value_list])
            else:
                raise TypeError("Final argument should be either a string " +
                                "or an iterable")
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

        # This was added to ensure that the function is a nop (instead of,
        # for instance, raising an exception) when applied to an empty list,
        # which is an artifact of the API operating on lists, rather than
        # relying on language idioms, such as comprehensions.
        if self.__len__() == 0:
            return

        if val is not None and nest.is_literal(params):
            if (nest.is_iterable(val) and not
                    isinstance(val, (nest.uni_str, dict))):
                params = [{params: x} for x in val]
            else:
                params = {params: val}

        if ((isinstance(params, list) and self.__len__() != len(params)) or
                (isinstance(params, tuple) and self.__len__() != len(params))):
            raise TypeError(
                "status dict must be a dict, or a list of dicts of length "
                "len(nodes)")

        nest.sli_func('SetStatus', self._datum, params)


class Connectome(object):
    """
    Class for Connections.

    Connectome represents the connections of a network. The class supports
    get(), set(), len() and equality.

    A Connectome is created by the ``GetConnections`` function.
    """

    _datum = None

    def __init__(self, data):

        if isinstance(data, list):
            for datum in data:
                if (not isinstance(datum, nest.SLIDatum) or
                        datum.dtype != "connectiontype"):
                    raise TypeError("Expected Connection Datum.")
            self._datum = data
        elif data is None:
            # So we can have empty Connectome and not a tuple if there are no
            # connections.
            self._datum = data
        else:
            if (not isinstance(data, nest.SLIDatum) or
                    data.dtype != "connectiontype"):
                raise TypeError("Expected Connection Datum.")
            # self._datum is a list of Connection datums.
            self._datum = [data]

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

    def __str__(self):
        """
        Printing a Connectome returns something of the form:
            *--------*-------------*
            | source | 1, 1, 2, 2, |
            *--------*-------------*
            | target | 1, 2, 1, 2, |
            *--------*-------------*
        """
        src = self.get('source')
        trgt = self.get('target')

        if isinstance(src, int):
            src = [src]
        if isinstance(trgt, int):
            trgt = [trgt]

        source = '| source | ' + ''.join(str(e)+', ' for e in src) + '|'
        target = '| target | ' + ''.join(str(e)+', ' for e in trgt) + '|'

        # 35 is arbitrarily chosen.
        if len(src) < 35:
            borderline = '*--------*' + '-'*(len(source) - 12) + '-*'
        else:
            borderline = '*--------*' + '-'*3*35 + '-*'

        result = (borderline + '\n' + source + '\n' + borderline + '\n' +
                  target + '\n' + borderline)
        return result

    def get(self, keys=None, pandas_output=False):
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

        Returns
        -------
        dict:
            All parameters
        type:
            If keys is a string, the corrsponding default parameter is returned
        list:
            If keys is a list of strings, a dictionary with a list of
            corresponding parameters is returned

        Raises
        ------
        TypeError
            Description
        """
        if pandas_output and not HAVE_PANDAS:
            raise ImportError('Pandas could not be imported')

        # Return empty tuple if we have no connections or if we have done a
        # nest.ResetKernel()
        if self.__len__() == 0 or nest.GetKernelStatus()['network_size'] == 0:
            return ()

        if keys is None:
            cmd = 'GetStatus'
        elif nest.is_literal(keys):
            cmd = 'GetStatus {{ /{0} get }} Map'.format(keys)
        elif nest.is_iterable(keys):
            keys_str = " ".join("/{0}".format(x) for x in keys)
            cmd = 'GetStatus {{ [ [ {0} ] ] get }} Map'.format(keys_str)
        else:
            raise TypeError("keys should be either a string or an iterable")

        nest.sps(self._datum)
        nest.sr(cmd)
        result = nest.spp()

        # Need to restructure the data.
        if nest.is_literal(keys):
            final_result = result[0] if self.__len__() == 1 else list(result)
        elif nest.is_iterable(keys):
            final_result = {}
            if self.__len__() != 1:
                # We want a dictionary of lists if len != 1
                for key in keys:
                    final_result[key] = []
                for val in result:
                    # val is ordered the same way as keys.
                    for count, key in enumerate(keys):
                        final_result[key].append(val[count])
            else:
                # Result is a tuple with a single value, a tuple with values
                # in the same order as parameters in keys
                for count, key in enumerate(keys):
                    final_result[key] = result[0][count]
        elif keys is None:
            final_result = {}
            if self.__len__() != 1:
                # We want a dictionary of lists if len != 1
                # First set the keys:
                for key in result[0]:
                    final_result[key] = []
                # Then set the values
                for val in result:
                    # Get a dictionary
                    for key, value in val.items():
                        final_result[key].append(value)
            else:
                for key, value in result[0].items():
                    final_result[key] = value

        if pandas_output:
            index = self.get('source') if self.__len__() > 1 else (self.get('source'),)
            if nest.is_literal(keys):
                final_result = {keys: final_result}
            final_result = pandas.DataFrame(final_result, index=index)

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
        if self.__len__() == 0  or nest.GetKernelStatus()['network_size'] == 0:
            return

        if val is not None and nest.is_literal(params):
            if (nest.is_iterable(val)
                and not isinstance(val, (nest.uni_str, dict))):
                    params = [{params: x} for x in val]
            else:
                params = {params: val}

        if ((isinstance(params, list) and self.__len__() != len(params)) or
                (isinstance(params, tuple) and self.__len__() != len(params))):
            raise TypeError(
                "status dict must be a dict, or a list of dicts of length "
                "len(nodes)")

        params = nest.broadcast(params, self.__len__(), (dict,), "params")

        nest.sps(self._datum)
        nest.sps(params)

        nest.sr('2 arraystore')
        nest.sr('Transpose { arrayload pop SetStatus } forall')


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
        if not isinstance(datum, nest.SLIDatum) or datum.dtype != "masktype":
            raise TypeError("expected mask Datum")
        self._datum = datum

    # Generic binary operation
    def _binop(self, op, other):
        if not isinstance(other, Mask):
            return NotImplemented
        return nest.sli_func(op, self._datum, other._datum)

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
        return nest.sli_func("Inside", point, self._datum)


class Parameter(object):
    """
    Class for parameters for distance dependency or randomization.

    Parameters are spatial functions which are used when creating
    connections in the Topology module. A parameter may be used as a
    probability kernel when creating connections or as synaptic parameters
    (such as weight and delay). Parameters are created using the
    ``CreateParameter`` command.
    """

    _datum = None

    # The constructor should not be called by the user
    def __init__(self, datum):
        """Parameters must be created using the CreateParameter command."""
        if not isinstance(datum,
                          nest.SLIDatum) or datum.dtype != "parametertype":
            raise TypeError("expected parameter datum")
        self._datum = datum

    # Generic binary operation
    def _binop(self, op, other):
        if not isinstance(other, Parameter):
            return NotImplemented
        return nest.sli_func(op, self._datum, other._datum)

    def __add__(self, other):
        return self._binop("add", other)

    def __sub__(self, other):
        return self._binop("sub", other)

    def __mul__(self, other):
        return self._binop("mul", other)

    def __div__(self, other):
        return self._binop("div", other)

    def __truediv__(self, other):
        return self._binop("div", other)

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
        CreateParameter : create parameter for e.g., distance dependency


        Notes
        -----
        -


        **Example**
            ::

                import nest.topology as tp

                #linear dependent parameter
                P = tp.CreateParameter('linear', {'a' : 2., 'c' : 0.})

                #get out value
                P.GetValue(point=[3., 4.])

        """
        return nest.sli_func("GetValue", point, self._datum)
