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

from ..ll_api import check_stack, spp, sps, sr
from .hl_api_helper import (
    deprecated,
    is_iterable,
    is_literal,
    model_deprecation_warning,
)
from .hl_api_simulation import GetKernelStatus
from .hl_api_types import to_json

__all__ = [
    "ConnectionRules",
    "CopyModel",
    "GetDefaults",
    "Models",
    "SetDefaults",
]


@check_stack
@deprecated("nest.node_models or nest.synapse_models")
def Models(mtype="all", sel=None):
    r"""Return a tuple of neuron, device, or synapse model names.

    Parameters
    ----------
    mtype : str, optional
        Use ``mtype='nodes'`` to only get neuron and device models,
        or ``mtype='synapses'`` to only get synapse models.
    sel : str, optional
        Filter results and only return models containing ``sel``.

    Returns
    -------
    tuple
        Available model names, sorted by name

    Raises
    ------
    ValueError
        Description

    Notes
    -----
    - Synapse model names ending in ``_hpc`` require less memory because of
      thread-local indices for target neuron IDs and fixed ``rport``\s of 0.
    - Synapse model names ending in ``_lbl`` allow to assign an integer label
      (``synapse_label``) to each individual synapse, at the cost of increased
      memory requirements.

    """

    if mtype not in ("all", "nodes", "synapses"):
        raise ValueError("mtype has to be one of 'all', 'nodes', or 'synapses'")

    models = []

    if mtype in ("all", "nodes"):
        models += GetKernelStatus("node_models")

    if mtype in ("all", "synapses"):
        models += GetKernelStatus("synapse_models")

    if sel is not None:
        models = [x for x in models if sel in x]

    models.sort()

    return tuple(models)


@check_stack
@deprecated("nest.connection_rules")
def ConnectionRules():
    """Return a tuple of all available connection rules, sorted by name.

    Returns
    -------
    tuple
        Available connection rules, sorted by name

    """

    return tuple(sorted(GetKernelStatus("connection_rules")))


@check_stack
def SetDefaults(model, params, val=None):
    """Set defaults for the given model or recording backend.

    New default values are used for all subsequently created instances
    of the model.

    Parameters
    ----------
    model : str
        Name of the model or recording backend
    params : str or dict
        Dictionary of new default parameter values
    val : str, optional
        If given, ``params`` has to be the name of a parameter.

    """

    if val is not None:
        if is_literal(params):
            params = {params: val}

    sps(params)
    sr("/{0} exch SetDefaults".format(model))


@check_stack
def GetDefaults(model, keys=None, output=""):
    """Return defaults of the given model or recording backend.

    Parameters
    ----------
    model : str
        Name of the model or recording backend
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
        cmd = "/{0} GetDefaults /{1} get".format(model, keys)
    elif is_iterable(keys):
        keys_str = " ".join("/{0}".format(x) for x in keys)
        cmd = "/{0} GetDefaults  [ {1} ] {{ 1 index exch get }}".format(model, keys_str) + " Map exch pop"
    else:
        raise TypeError("keys should be either a string or an iterable")

    sr(cmd)
    result = spp()

    if output == "json":
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
