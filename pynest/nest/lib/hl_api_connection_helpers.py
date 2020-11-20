# -*- coding: utf-8 -*-
#
# hl_api_connection_helpers.py
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
These are helper functions to ease the definition of the
Connect function.
"""

import numpy as np

from ..ll_api import *
from .. import pynestkernel as kernel
from .hl_api_types import CollocatedSynapses, Mask, NodeCollection, Parameter
from .hl_api_exceptions import NESTErrors

__all__ = [
    '_connect_layers_needed',
    '_connect_spatial',
    '_process_conn_spec',
    '_process_spatial_projections',
    '_process_syn_spec',
]


def _process_conn_spec(conn_spec):
    """Processes the connectivity specifications from None, string or dictionary to a dictionary."""
    if conn_spec is None:
        # Get default conn_spec
        sr('/Connect /conn_spec GetOption')
        return spp()
    elif isinstance(conn_spec, str):
        processed_conn_spec = {'rule': conn_spec}
        return processed_conn_spec
    elif isinstance(conn_spec, dict):
        return conn_spec
    else:
        raise TypeError("conn_spec must be a string or dict")


def _process_syn_spec(syn_spec, conn_spec, prelength, postlength, use_connect_arrays):
    """Processes the synapse specifications from None, string or dictionary to a dictionary."""
    if syn_spec is None:
        # for use_connect_arrays, return "static_synapse" by default
        if use_connect_arrays:
            return {"synapse_model": "static_synapse"}

        return syn_spec

    rule = conn_spec['rule']

    if isinstance(syn_spec, str):
        return {"synapse_model": syn_spec}
    elif isinstance(syn_spec, dict):
        for key, value in syn_spec.items():
            # if value is a list, it is converted to a numpy array
            if isinstance(value, (list, tuple)):
                value = np.asarray(value)

            if isinstance(value, (np.ndarray, np.generic)):
                if len(value.shape) == 1:
                    if rule == 'one_to_one':
                        if value.shape[0] != prelength:
                            if use_connect_arrays:
                                raise kernel.NESTError(
                                    "'{}' has to be an array of dimension {}.".format(key, prelength))
                            else:
                                raise kernel.NESTError(
                                    "'{}' has to be an array of dimension {}, a scalar or a dictionary.".format(
                                        key, prelength))
                        else:
                            syn_spec[key] = value
                    elif rule == 'fixed_total_number':
                        if ('N' in conn_spec and value.shape[0] != conn_spec['N']):
                            raise kernel.NESTError(
                                "'{}' has to be an array of dimension {}, a scalar or a dictionary".format(
                                    key, conn_spec['N']))
                        else:
                            syn_spec[key] = value
                    else:
                        raise kernel.NESTError(
                            "'{}' has the wrong type. One-dimensional parameter arrays can only be used in "
                            "conjunction with rule 'one_to_one' or 'fixed_total_number'.".format(key))

                elif len(value.shape) == 2:
                    if rule == 'all_to_all':
                        if value.shape[0] != postlength or value.shape[1] != prelength:
                            raise kernel.NESTError(
                                "'{}' has to be an array of dimension {}x{} (n_target x n_sources), a scalar "
                                "or a dictionary.".format(key, postlength, prelength))
                        else:
                            syn_spec[key] = value.flatten()
                    elif rule == 'fixed_indegree':
                        indegree = conn_spec['indegree']
                        if value.shape[0] != postlength or \
                                value.shape[1] != indegree:
                            raise kernel.NESTError(
                                "'{}' has to be an array of dimension {}x{} (n_target x indegree), a scalar "
                                "or a dictionary.".format(key, postlength, indegree))
                        else:
                            syn_spec[key] = value.flatten()
                    elif rule == 'fixed_outdegree':
                        outdegree = conn_spec['outdegree']
                        if value.shape[0] != prelength or \
                                value.shape[1] != outdegree:
                            raise kernel.NESTError(
                                "'{}' has to be an array of dimension {}x{} (n_sources x outdegree), a scalar "
                                "or a dictionary.".format(key, prelength, outdegree))
                        else:
                            syn_spec[key] = value.flatten()
                    else:
                        raise kernel.NESTError(
                            "'{}' has the wrong type. Two-dimensional parameter arrays can only be used in "
                            "conjunction with rules 'all_to_all', 'fixed_indegree' or fixed_outdegree'.".format(key))

        # check that "synapse_model" is there for use_connect_arrays
        if use_connect_arrays and "synapse_model" not in syn_spec:
            syn_spec["synapse_model"] = "static_synapse"

        return syn_spec
    elif isinstance(syn_spec, CollocatedSynapses):
        return syn_spec

    raise TypeError("syn_spec must be a string or dict")


def _process_spatial_projections(conn_spec, syn_spec):
    """
    Processes the connection and synapse specifications to a single dictionary
    for the SLI function `ConnectLayers`.
    """
    allowed_conn_spec_keys = ['mask', 'allow_multapses', 'allow_autapses', 'rule',
                              'indegree', 'outdegree', 'p', 'use_on_source', 'allow_oversized_mask']
    allowed_syn_spec_keys = ['weight', 'delay', 'synapse_model']
    for key in conn_spec.keys():
        if key not in allowed_conn_spec_keys:
            raise ValueError("'{}' is not allowed in conn_spec when connecting with mask or kernel".format(key))

    projections = {}
    projections.update(conn_spec)
    if 'p' in conn_spec:
        projections['kernel'] = projections.pop('p')
    if syn_spec is not None:
        if isinstance(syn_spec, CollocatedSynapses):
            for syn_list in syn_spec.syn_specs:
                for key in syn_list.keys():
                    if key not in allowed_syn_spec_keys:
                        raise ValueError(
                            "'{}' is not allowed in syn_spec when connecting with mask or kernel".format(key))
            projections.update({'synapse_parameters': syn_spec.syn_specs})
        else:
            for key in syn_spec.keys():
                if key not in allowed_syn_spec_keys:
                    raise ValueError("'{}' is not allowed in syn_spec when connecting with mask or kernel".format(key))
            projections.update(syn_spec)
    if conn_spec['rule'] == 'fixed_indegree':
        if 'use_on_source' in conn_spec:
            raise ValueError("'use_on_source' can only be set when using pairwise_bernoulli")
        projections['connection_type'] = 'pairwise_bernoulli_on_source'
        projections['number_of_connections'] = projections.pop('indegree')
    elif conn_spec['rule'] == 'fixed_outdegree':
        if 'use_on_source' in conn_spec:
            raise ValueError("'use_on_source' can only be set when using pairwise_bernoulli")
        projections['connection_type'] = 'pairwise_bernoulli_on_target'
        projections['number_of_connections'] = projections.pop('outdegree')
    elif conn_spec['rule'] == 'pairwise_bernoulli':
        if ('use_on_source' in conn_spec and
                conn_spec['use_on_source']):
            projections['connection_type'] = 'pairwise_bernoulli_on_source'
            projections.pop('use_on_source')
        else:
            projections['connection_type'] = 'pairwise_bernoulli_on_target'
            if 'use_on_source' in projections:
                projections.pop('use_on_source')
    else:
        raise kernel.NESTError("When using kernel or mask, the only possible connection rules are "
                               "'pairwise_bernoulli', 'fixed_indegree', or 'fixed_outdegree'")
    projections.pop('rule')
    return projections


def _connect_layers_needed(conn_spec, syn_spec):
    """Determins if connection has to be made with the SLI function `ConnectLayers`."""
    if isinstance(conn_spec, dict):
        # If a conn_spec entry is based on spatial properties, we must use ConnectLayers.
        for key, item in conn_spec.items():
            if isinstance(item, Parameter) and item.is_spatial():
                return True
        # We must use ConnectLayers in some additional cases.
        rule_is_bernoulli = 'pairwise_bernoulli' in str(conn_spec['rule'])
        if ('mask' in conn_spec or
                ('p' in conn_spec and not rule_is_bernoulli) or
                'use_on_source' in conn_spec):
            return True
    # If a syn_spec entry is based on spatial properties, we must use ConnectLayers.
    if isinstance(syn_spec, dict):
        for key, item in syn_spec.items():
            if isinstance(item, Parameter) and item.is_spatial():
                return True
    elif isinstance(syn_spec, CollocatedSynapses):
        return any([_connect_layers_needed(conn_spec, syn_param) for syn_param in syn_spec.syn_specs])
    # If we get here, there is not need to use ConnectLayers.
    return False


def _connect_spatial(pre, post, projections):
    """Connect `pre` to `post` using the specifications in `projections` with the SLI function `ConnectLayers`."""
    # Replace python classes with SLI datums
    def fixdict(d):
        d = d.copy()
        for k, v in d.items():
            if isinstance(v, dict):
                d[k] = fixdict(v)
            elif isinstance(v, Mask) or isinstance(v, Parameter):
                d[k] = v._datum
        return d

    projections = fixdict(projections)
    sps(projections)
    sr('ConnectLayers')


def _process_input_nodes(pre, post, conn_spec):
    """
    Check the properties of `pre` and `post` nodes:

    * If `conn_spec` is 'one_to_one', no uniqueness check is performed; the
      "regular" one-to-one connect is used if both inputs are NodeCollection,
      "connect_arrays" is used otherwise.
    * If both `pre` and `post` are NodeCollections or can be converted to
      NodeCollections (i.e. contain unique IDs), then proceed to "regular"
      connect (potentially after conversion to NodeCollection).
    * If both `pre` and `post` are arrays and contain non-unique items, then
      we proceed to "connect_arrays".
    * If at least one of them has non-unique items and they have different
      sizes, then raise an error.
    """
    use_connect_arrays = False

    # check for 'one_to_one' conn_spec
    one_to_one_cspec = (conn_spec if not isinstance(conn_spec, dict)
                        else conn_spec.get('rule', 'all_to_all') == 'one_to_one')

    # check and convert input types
    pre_is_nc, post_is_nc = True, True

    if not isinstance(pre, NodeCollection):
        # skip uniqueness check for connect_arrays compatible `conn_spec`
        if not one_to_one_cspec and len(set(pre)) == len(pre):
            pre = NodeCollection(pre)
        else:
            pre_is_nc = False

    if not isinstance(post, NodeCollection):
        # skip uniqueness check for connect_arrays compatible `conn_spec`
        if not one_to_one_cspec and len(set(post)) == len(post):
            post = NodeCollection(post)
        else:
            post_is_nc = False

    if not pre_is_nc or not post_is_nc:
        if len(pre) != len(post):
            raise NESTErrors.ArgumentType(
                "Connect",
                "If `pre` or `post` contain non-unique IDs, then they must have the same length.")

        # convert to arrays
        pre = np.asarray(pre)
        post = np.asarray(post)

        # check array type
        if not issubclass(pre.dtype.type, (int, np.integer)):
            raise NESTErrors.ArgumentType("Connect", " `pre` IDs should be integers.")

        if not issubclass(post.dtype.type, (int, np.integer)):
            raise NESTErrors.ArgumentType("Connect", " `post` IDs should be integers.")

        # check dimension
        if not (pre.ndim == 1 and post.ndim == 1):
            raise ValueError("Sources and targets must be 1-dimensional arrays")

        use_connect_arrays = True

    if use_connect_arrays and not one_to_one_cspec:
        raise ValueError("When connecting two arrays with non-unique IDs, `conn_spec` must be 'one_to_one'.")

    return use_connect_arrays, pre, post
