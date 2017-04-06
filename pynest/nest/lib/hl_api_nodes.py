# -*- coding: utf-8 -*-
#
# hl_api_nodes.py
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
Classes and functions for node handling
"""

import nest
from .hl_api_helper import *
from .hl_api_info import SetStatus


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
            self._datum = nest.sli_func('cvgidcollection', data)

    def __iter__(self):
        return GIDCollectionIterator(self)

    def items(self):
        return GIDCollectionIterator(self, True)

    def __add__(self, other):
        if not isinstance(other, GIDCollection):
            return NotImplemented
        return GIDCollection(nest.sli_func('join', self._datum, other._datum))

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

            return (GIDCollection(nest.sli_func('Take',
                                                self._datum,
                                                [start, stop, step])))
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


@check_stack
def Create(model, n=1, params=None):
    """Create n instances of type model.

    Parameters
    ----------
    model : str
        Name of the model to create
    n : int, optional
        Number of instances to create
    params : TYPE, optional
        Parameters for the new nodes. A single dictionary or a list of
        dictionaries with size n. If omitted, the model's defaults are used.

    Returns
    -------
    GIDCollection:
        Object representing global IDs of created nodes
    """

    model_deprecation_warning(model)

    if isinstance(params, dict):
        cmd = "/%s 3 1 roll exch Create" % model
        sps(params)
    else:
        cmd = "/%s exch Create" % model

    sps(n)
    sr(cmd)

    gids = GIDCollection(spp())

    if params is not None and not isinstance(params, dict):
        try:
            SetStatus(gids, params)
        except:
            warnings.warn(
                "SetStatus() call failed, but nodes have already been " +
                "created! The GIDs of the new nodes are: {0}.".format(gids))
            raise

    return gids


@check_stack
def PrintNodes():
    """Print the GID ranges and model names of the nodes in the network."""

    sr("PrintNodesToStream")
    print(spp())
