# -*- coding: utf-8 -*-
#
# test_iaf_tum_2000.py
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
Test the ``iaf_tum_2000`` neuron model implementation.

The ``iaf_tum_2000`` neuron model incorporates the ``tsodyks_synapse``
computations directly in an ``iaf_psc_exp`` neuron on the presynaptic
side. This test ensures that we obtain identical dynamics from a system of
two ``iaf_tum_2000`` neurons connected by a static synapse and a system
of two ``iaf_psc_exp`` neurons connected by a Tsodyks synapse.
"""

import matplotlib.pyplot as plt
import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture(scope="module")
def simulator():
    """Build and simulate systems."""

    # Set model parameters
    sim_time = 1200.0  # simulation time [ms]
    stim_start = 50.0  # start time of DC input [ms]
    stim_end = 1050.0  # end time of DC input [ms]
    dc_amp = 500.0  # DC amplitude [pA]

    weight = 250.0  # synaptic weight [pA]
    delay = 1.0  # synaptic delay [ms]
    tau_psc = 2.0  # PSC time constant [ms]
    tau_rec = 400.0  # recovery time [ms]
    tau_fac = 1000.0  # facilitation time [ms]

    iaf_tum_params = {
        "tau_psc": tau_psc,
        "tau_rec": tau_rec,
        "tau_fac": tau_fac,
    }

    tsodyks_syn_spec = {
        "synapse_model": "tsodyks_synapse",
        "weight": weight,
        "delay": delay,
        "tau_psc": tau_psc,
        "tau_rec": tau_rec,
        "tau_fac": tau_fac,
        "U": 0.5,
        "u": 0.0,
        "x": 0.0,
        "y": 0.0,
    }

    # Create common current generator
    dc_gen = nest.Create("dc_generator", 1, params={"amplitude": dc_amp, "start": stim_start, "stop": stim_end})

    # Build iaf_tum_2000 system
    iaf_tum_pre = nest.Create("iaf_tum_2000", 1, params=iaf_tum_params)
    iaf_tum_post = nest.Create("iaf_tum_2000", 1, params=iaf_tum_params)
    mm_tum = nest.Create("multimeter", 1, params={"record_from": ["V_m", "I_syn_ex"]})
    nest.Connect(dc_gen, iaf_tum_pre)
    nest.Connect(
        iaf_tum_pre,
        iaf_tum_post,
        syn_spec={"synapse_model": "static_synapse", "weight": weight, "delay": delay, "receptor_type": 1},
    )
    nest.Connect(mm_tum, iaf_tum_post)

    # Build iaf_psc_exp system
    iaf_exp_pre = nest.Create("iaf_psc_exp", 1)
    iaf_exp_post = nest.Create("iaf_psc_exp", 1)
    mm_exp = nest.Create("multimeter", 1, params={"record_from": ["V_m", "I_syn_ex"]})
    nest.Connect(dc_gen, iaf_exp_pre)
    nest.Connect(
        iaf_exp_pre,
        iaf_exp_post,
        syn_spec=tsodyks_syn_spec,
    )
    nest.Connect(mm_exp, iaf_exp_post)

    nest.Simulate(sim_time)

    return mm_tum, mm_exp


def test_iaf_tum_2000_correct_membrane_potential(simulator):
    """
    Test that ``iaf_tum_2000`` has the expected membrane potential in simulation.
    """

    mm_tum, mm_exp = simulator

    nptest.assert_almost_equal(mm_tum.events["V_m"], mm_exp.events["V_m"])


def test_iaf_tum_2000_correct_synaptic_current(simulator):
    """
    Test that ``iaf_tum_2000`` has the expected synaptic current in simulation.
    """

    mm_tum, mm_exp = simulator

    nptest.assert_almost_equal(mm_tum.events["I_syn_ex"], mm_exp.events["I_syn_ex"])
