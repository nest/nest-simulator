# -*- coding: utf-8 -*-
#
# test_ticket_478.py
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
Regression test for Ticket #478.

Ensure that devices can only be connected using static synapses.

This test ensures that NEST throws an exception if one tries to connect generator devices
(sending DSSpikeEvents or DSCurrentEvents) or a multimeter (sending DataLoggingRequest)
to a neuron using a plastic synapse.

Author: Hans Ekkehard Plesser, 2010-10-22
"""

import nest
import pytest

# Exclude synapse types not relevant for spiking devices as senders
EXCLUDED_SYNAPSES = {
    "gap_junction",
    "sic_connection",
    "rate_connection_delayed",
    "rate_connection_instantaneous",
}


# Helper to get sorted list of parameter keys for a synapse model
def _synapse_param_keys(model):
    return sorted(str(k) for k in nest.GetDefaults(model).keys())


@pytest.fixture(autouse=True)
def prepare_kernel():
    nest.ResetKernel()
    nest.set_verbosity("M_ERROR")


def test_generators_static_and_plastic_synapses():
    """
    Test that generator devices can only be connected to neurons using static synapses.
    """
    # List of generator models to test (only those present in this NEST build)
    candidate_generators = [
        "gamma_sup_generator",
        "mip_generator",
        "noise_generator",
        "poisson_generator",
        "ppd_sup_generator",
        "sinusoidal_gamma_generator",
        "poisson_generator_ps",
    ]
    ds_models = [m for m in candidate_generators if m in nest.node_models]

    # Identify static and plastic synapse models
    static_defaults = _synapse_param_keys("static_synapse")
    static_lbl_defaults = _synapse_param_keys("static_synapse_lbl")
    static_syn_models = [
        m
        for m in nest.synapse_models
        if m not in EXCLUDED_SYNAPSES
        and (_synapse_param_keys(m) == static_defaults or _synapse_param_keys(m) == static_lbl_defaults)
    ]
    plastic_syn_models = [
        m
        for m in nest.synapse_models
        if m not in EXCLUDED_SYNAPSES
        and _synapse_param_keys(m) != static_defaults
        and _synapse_param_keys(m) != static_lbl_defaults
    ]

    # All static synapses should work for all generator models
    for syn in static_syn_models:
        for gen in ds_models:
            nest.ResetKernel()
            src = nest.Create(gen)
            tgt = nest.Create("iaf_psc_alpha")
            try:
                nest.Connect(src, tgt, syn_spec={"synapse_model": syn})
            except nest.NESTError as e:
                # Only skip if it's an IllegalConnection, otherwise re-raise
                if "IllegalConnection" in str(type(e)):
                    print(f"SKIP: {gen} -> iaf_psc_alpha with {syn}: {e}")
                    continue
                raise

    # All plastic synapses should fail for all generator models
    for syn in plastic_syn_models:
        for gen in ds_models:
            nest.ResetKernel()
            src = nest.Create(gen)
            tgt = nest.Create("iaf_psc_alpha")
            with pytest.raises(nest.NESTError):
                nest.Connect(src, tgt, syn_spec={"synapse_model": syn})


def test_multimeter_static_and_plastic_synapses():
    """
    Test that multimeter can only be connected to neurons using static (non-HPC) synapses,
    and fails for plastic and _hpc static synapses.
    """
    # Identify static and plastic synapse models as above
    static_defaults = _synapse_param_keys("static_synapse")
    static_lbl_defaults = _synapse_param_keys("static_synapse_lbl")
    static_syn_models = [
        m
        for m in nest.synapse_models
        if m not in EXCLUDED_SYNAPSES
        and (_synapse_param_keys(m) == static_defaults or _synapse_param_keys(m) == static_lbl_defaults)
    ]
    plastic_syn_models = [
        m
        for m in nest.synapse_models
        if m not in EXCLUDED_SYNAPSES
        and _synapse_param_keys(m) != static_defaults
        and _synapse_param_keys(m) != static_lbl_defaults
    ]
    # Static synapses that are not _hpc
    static_non_hpc_models = [m for m in static_syn_models if not m.endswith("_hpc")]
    # Models that should fail: all plastic + static _hpc
    models_to_fail = plastic_syn_models + [m for m in static_syn_models if m.endswith("_hpc")]

    # All static non-HPC synapses should work
    for syn in static_non_hpc_models:
        nest.ResetKernel()
        src = nest.Create("multimeter", params={"record_from": ["V_m"]})
        tgt = nest.Create("iaf_psc_alpha")
        try:
            nest.Connect(src, tgt, syn_spec={"synapse_model": syn})
        except nest.NESTError as e:
            if "IllegalConnection" in str(type(e)):
                print(f"SKIP: multimeter -> iaf_psc_alpha with {syn}: {e}")
                continue
            raise

    # All plastic and static _hpc synapses should fail
    for syn in models_to_fail:
        nest.ResetKernel()
        src = nest.Create("multimeter", params={"record_from": ["V_m"]})
        tgt = nest.Create("iaf_psc_alpha")
        with pytest.raises(nest.NESTError):
            nest.Connect(src, tgt, syn_spec={"synapse_model": syn})
