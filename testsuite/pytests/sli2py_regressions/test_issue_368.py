# -*- coding: utf-8 -*-
#
# test_issue_368.py
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
Regression test for Issue #368 (GitHub).

This test ensures that models with precise timing handle input spikes
arriving at exactly the same times correctly.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.mark.parametrize("model", ["iaf_psc_alpha_ps", "iaf_psc_delta_ps", "iaf_psc_exp_ps"])
def test_linear_summation_of_input_ps_models(model):
    """
    Test linear summation of input for precise spiking neuron models.

    The test creates four neurons n1, ..., n4 and connects each to a
    `spike_generator` such that:

    * n1 receives single spike with weight 2 * w1
    * n2 receives two spikes at same time with weight w1 each
    * Both n1 and n2 must have same V_m after simulation
    * n3 receives single spike with weight w1 + w2
    * n4 receives one spike with weight w1, one at same time with weight w2
    * Both n3 and n4 must have same V_m after simulation
    """

    nest.set(resolution=0.25, tics_per_ms=1024.0)

    if model == "iaf_psc_exp_ps":
        nest.SetDefaults("iaf_psc_exp_ps", {"tau_syn_ex": 2.0, "tau_syn_in": 2.0})

    t_spike1 = 2.375  # 2.5 - 0.125 offset
    w1 = 5.0  # sufficiently small to avoid psc_delta spikes
    w2 = -10.0

    sg = nest.Create("spike_generator", {"precise_times": True, "spike_times": [t_spike1]})
    n1 = nest.Create(model)
    n2 = nest.Create(model)
    n3 = nest.Create(model)
    n4 = nest.Create(model)

    # single connection, double weight
    nest.Connect(sg, n1, "one_to_one", {"weight": w1 * 2})

    # two connections, double weight
    nest.Connect(sg, n2, "one_to_one", {"weight": w1})
    nest.Connect(sg, n2, "one_to_one", {"weight": w1})

    # single connection, combined weight
    nest.Connect(sg, n3, "one_to_one", {"weight": w1 + w2})

    # two connections, different weights
    nest.Connect(sg, n4, "one_to_one", {"weight": w1})
    nest.Connect(sg, n4, "one_to_one", {"weight": w2})

    nest.Simulate(4.0)

    assert n1.V_m == pytest.approx(n2.V_m)
    assert n3.V_m == pytest.approx(n4.V_m)


@pytest.mark.parametrize("model", ["iaf_psc_exp_ps"])
def test_linear_summation_of_input_ps_models_with_two_time_constants(model):
    """
    Test linear summation of input for ps models with two time constants.

    The test creates three neurons n1, ..., n3 and connects each to a
    `spike_generator` such that:

    * n1 receives one excitatory spike
    * n2 receives one inhibitory spike at the same time
    * n3 receives both spikes

    We let :math:`V_0 = E_L` be the membrane potential at :math:`t_0`
    (same for all) and :math:`V_1`, :math:`V_2`, :math:`V_3` be the potentials
    of n1, n2, n3 after simulation. Then, we expect due to linearity

    .. math::

        ( V_1 - V_0 ) + ( V_2 - V_0 ) = V_3 - V_0
    """

    nest.set(resolution=0.25, tics_per_ms=1024.0)

    nest.SetDefaults(model, {"tau_syn_ex": 5.0, "tau_syn_in": 1.0})

    t_spike1 = 2.375  # 2.5 - 0.125 offset
    w1 = 1000.0

    sg = nest.Create("spike_generator", {"precise_times": True, "spike_times": [t_spike1]})
    n1 = nest.Create(model)
    n2 = nest.Create(model)
    n3 = nest.Create(model)

    # excitatory connection only
    nest.Connect(sg, n1, "one_to_one", {"weight": w1})

    # inhibitory connection only
    nest.Connect(sg, n2, "one_to_one", {"weight": -w1})

    # excitatory and inhibitory connection
    nest.Connect(sg, n3, "one_to_one", {"weight": w1})
    nest.Connect(sg, n3, "one_to_one", {"weight": -w1})

    # assume that neurons are initialized at equilibrium
    V0 = n1.V_m

    # sanity check for test
    assert V0 == n1.E_L

    nest.Simulate(4.0)

    V1 = n1.V_m
    V2 = n2.V_m
    V3 = n3.V_m

    assert (V1 - V0) + (V2 - V0) == V3 - V0
