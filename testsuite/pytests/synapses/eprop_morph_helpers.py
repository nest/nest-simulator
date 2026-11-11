# -*- coding: utf-8 -*-
#
# eprop_morph_helpers.py
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
Run the e-prop tests with structure-aware communication (morph) active.

The fixture ``morph_connect`` replaces ``nest.Connect`` for the duration of a test: the first call sets
``threshold_delay`` to the resolution, and every connection between neurons by a spike-transmitting synapse
model with a delay of at least the threshold is created with ``long_range``. Spikes of these connections, including
the flush events of e-prop, then travel on the long path of morph. The reference values of the e-prop tests must
hold unchanged.
"""

import numbers

import nest
import numpy as np
import pytest

# synapse models that transmit secondary events; they cannot be long_range
SECONDARY_SYNAPSE_MODELS = {
    "diffusion_connection",
    "eprop_learning_signal_connection",
    "eprop_learning_signal_connection_bsshslm_2020",
    "gap_junction",
    "rate_connection_delayed",
    "rate_connection_instantaneous",
    "sic_connection",
}


def _all_neurons(nc):
    """Return True if the NodeCollection contains only neurons."""

    if not isinstance(nc, nest.NodeCollection):
        return False
    element_types = nc.get("element_type")
    if isinstance(element_types, str):
        element_types = [element_types]
    return all(et == "neuron" for et in element_types)


def _connect_long_range(original_connect, statistics):
    """Return a replacement of ``nest.Connect`` that creates eligible connections as long_range."""

    def connect(pre, post, conn_spec=None, syn_spec=None, **kwargs):
        if np.isinf(nest.threshold_delay):
            nest.threshold_delay = nest.resolution
        spec = syn_spec if isinstance(syn_spec, dict) else {"synapse_model": syn_spec or "static_synapse"}
        model = spec.get("synapse_model", "static_synapse")
        delay = spec.get("delay", nest.GetDefaults(model, "delay"))
        if (
            model not in SECONDARY_SYNAPSE_MODELS
            and isinstance(delay, numbers.Number)
            and delay >= nest.threshold_delay
            and _all_neurons(pre)
            and _all_neurons(post)
        ):
            if conn_spec is None:
                conn_spec = {"rule": "all_to_all"}
            elif isinstance(conn_spec, str):
                conn_spec = {"rule": conn_spec}
            conn_spec = dict(conn_spec, long_range=True)
            statistics["long_range"] += 1
        return original_connect(pre, post, conn_spec, syn_spec, **kwargs)

    return connect


@pytest.fixture
def morph_connect(monkeypatch):
    """Create connections between neurons as long_range while a test runs; yields the number of such calls."""

    statistics = {"long_range": 0}
    # nest freezes its module attributes; the public API lives in the instance dictionary
    monkeypatch.setitem(nest.__dict__, "Connect", _connect_long_range(nest.Connect, statistics))
    yield statistics
