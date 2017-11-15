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

    next = __next__  # Python2.x


class GIDCollection(object):
    """
    Class for GIDCollection.

    GIDCollection represents the nodes of a network. The class supports
    iteration, concatination, indexing, slicing, membership, convertion to and
    from lists, test for membership, and test for equality.

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

            # Concatination
            Enrns = nest.Create('aeif_cond_alpha', 600)
            Inrns = nest.Create('iaf_psc_alpha', 400)
            nrns = Enrns + Inrns

            # Indexing, slicing and membership
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
            gid = nest.sli_func('get', self._datum, key)
            try:
                gid + 1
            except TypeError:
                raise TypeError("Slicing of a GIDCollection should return a \
                                gid")
            return gid

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
        return ''.format(nest.sli_func('==', self._datum))


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
