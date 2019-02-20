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

__all__ = [
    'ConnectionRules',
    'CopyModel',
    'GetDefaults',
    'Models',
    'SetDefaults',
]


@check_stack
def Models(mtype="all", sel=None):
    """Return a tuple of model names, sorted by name.

    All available models are neurons, devices and synapses.

    Parameters
    ----------
    mtype : str, optional
        Use ``'mtype='nodes'`` to only see neuron and device models,
        or ``'type='synapses'`` to only see synapse models.
    sel : str, optional
        String used to filter the result list and only return models
        containing it.

    Returns
    -------
    tuple
        Available model names

    Raises
    ------
    ValueError
        Description

    Notes
    -----
    - Synapse model names ending with ``'_hpc'`` provide minimal memory
      requirements by using thread-local target neuron IDs and fixing
      the ``'rport'`` to 0.
    - Synapse model names ending with ``'_lbl'`` allow to assign an individual
      integer label (``'synapse_label'``) to created synapses at the cost
      of increased memory requirements.

    KEYWORDS: models
    """

    if mtype not in ("all", "nodes", "synapses"):
        raise ValueError("type has to be one of 'all', 'nodes' or 'synapses'")

    models = []

    if mtype in ("all", "nodes"):
        sr("modeldict")
        models += spp().keys()

    if mtype in ("all", "synapses"):
        sr("synapsedict")
        models += spp().keys()

    if sel is not None:
        models = [x for x in models if x.find(sel) >= 0]

    models.sort()

    return tuple(models)


@check_stack
def ConnectionRules():
    """Return a typle of all available connection rules, sorted by name.

    Returns
    -------
    tuple
        Available connection rules

    KEYWORDS: models
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

    KEYWORDS: models
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
        If `output` is `json`, returns parameters in JSON format.

    Raises
    ------
    TypeError

    Notes
    -----
    **Example**

    .. code_block:: python

        >>> nest.GetDefaults('iaf_psc_alpha', 'V_m')
        -70.0

        >>> nest.GetDefaults('iaf_psc_alpha', ['V_m', 'V_th'])
        (-70.0, -55.0)

    KEYWORDS: models
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

    KEYWORDS: models
    """

    model_deprecation_warning(existing)

    if params is not None:
        sps(params)
        sr("/%s /%s 3 2 roll CopyModel" % (existing, new))
    else:
        sr("/%s /%s CopyModel" % (existing, new))
