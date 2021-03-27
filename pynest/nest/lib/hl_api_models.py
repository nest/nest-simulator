# -*- coding: utf-8 -*-
#
# hl_api_models.py
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
Functions for model handling
"""

from ..ll_api import *
from .hl_api_helper import *
from .hl_api_types import to_json

__all__ = [
    'ConnectionRules',
    'CopyModel',
    'GetDefaults',
    'SetDefaults',
]


@check_stack
def ConnectionRules():
    """Return a typle of all available connection rules, sorted by name.

    Returns
    -------
    tuple
        Available connection rules

    """

    sr('connruledict')
    return tuple(sorted(spp().keys()))


@check_stack
def SetDefaults(model, params, val=None):
    """Set the default parameter values of the given model.

    New default values are used for all subsequently created instances
    of the model.

    Parameters
    ----------
    model : str
        Name of the model
    params : str or dict
        Dictionary of new default parameter values
    val : str, optional
        If given, `params` has to be the name of a model property.

    """

    if val is not None:
        if is_literal(params):
            params = {params: val}

    sps(params)
    sr('/{0} exch SetDefaults'.format(model))


@check_stack
def GetDefaults(model, keys=None, output=''):
    """Return default parameters of the given model, specified by a string.

    Parameters
    ----------
    model : str
        Name of the model
    keys : str or list, optional
        String or a list of strings naming model properties. `GetDefaults` then
        returns a single value or a list of values belonging to the keys
        given.
    output : str, optional
        Whether the returned data should be in a format
        (``output='json'``). Default is ''.

    Returns
    -------
    dict
        A dictionary of default parameters.
    type
        If keys is a string, the corrsponding default parameter is returned.
    list
        If keys is a list of strings, a list of corrsponding default parameters
        is returned.
    str :
        If `output` is ``json``, returns parameters in JSON format.

    Raises
    ------
    TypeError

    """

    if keys is None:
        cmd = "/{0} GetDefaults".format(model)
    elif is_literal(keys):
        cmd = '/{0} GetDefaults /{1} get'.format(model, keys)
    elif is_iterable(keys):
        keys_str = " ".join("/{0}".format(x) for x in keys)
        cmd = "/{0} GetDefaults  [ {1} ] {{ 1 index exch get }}"\
              .format(model, keys_str) + " Map exch pop"
    else:
        raise TypeError("keys should be either a string or an iterable")

    sr(cmd)
    result = spp()

    if output == 'json':
        result = to_json(result)

    return result


@check_stack
def CopyModel(existing, new, params=None):
    """Create a new model by copying an existing one.

    Parameters
    ----------
    existing : str
        Name of existing model
    new : str
        Name of the copied model
    params : dict, optional
        Default parameters assigned to the copy. Not provided parameters are
        taken from the existing model.

    """

    model_deprecation_warning(existing)

    if params is not None:
        sps(params)
        sr("/%s /%s 3 2 roll CopyModel" % (existing, new))
    else:
        sr("/%s /%s CopyModel" % (existing, new))
