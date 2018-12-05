# -*- coding: utf-8 -*-
#
# ll_api.py
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
Low-level API of PyNEST Topology Module
"""

import nest


def topology_func(slifunc, *args):
    """
    Execute SLI function `slifunc` with arguments `args` in Topology namespace.


    Parameters
    ----------
    slifunc : str
        SLI namespace expression


    Other parameters
    ----------------
    args : dict
        An arbitrary number of arguments


    Returns
    -------
    out :
        Values from SLI function `slifunc`


    See also
    --------
    nest.ll_api.sli_func
    """

    return nest.ll_api.sli_func(slifunc, *args)
