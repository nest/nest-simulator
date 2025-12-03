# -*- coding: utf-8 -*-
#
# test_inh_stdp_synapse.py
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
Basic test of stdp_synapse using test_inh_stdp_synapse.

A parrot_neuron that repeats the spikes from a poisson generator is connected to
an iaf_psc_alpha that is driven by inh. and exc. poisson input.  The synapse is
an stdp_synapse. After the simulation, we go through the pre- and postsyn.
spike-trains spike by spike and try to reproduce the STDP results. The final
weight obtained after simulation is compared to the final weight obtained from
the test.

Test ported from SLI unittest.

Author: Weidel Mar 2016
Based on: Kunkel, Nov 2010
"""

import nest
import numpy as np
import pytest


def test_inh_stdp_synapse():
    """
    Test STDP synapse by reproducing weight changes from spike trains.
    """
    nest.ResetKernel()

    resolution = 0.1
    nest.set(resolution=resolution)

    # Input parameters
    K_exc = 8000.0  # number of exc. inputs
    K_inh = 2000.0  # number of inh. inputs
    nu = 10.0  # equil. firing rate
    nu_x = 1.7  # external rate
    w_exc = 45.0  # strength of exc. connections
    w_neuron = -45.0  # strength of connections between neuron and parrot
    w_inh = w_exc * -5.0  # strength of inh. connections
    delay = 1.0  # synaptic transmission delay

    axonal_delay = 0.0
    backpr_delay = delay - axonal_delay

    # STDP parameters
    alpha = 1.1
    lambda_val = 0.01
    tau_plus = 20.0
    tau_minus = 30.0
    mu_plus = 1.0  # multiplicative
    mu_minus = 1.0  # multiplicative
    w_max = w_neuron * 2.0

    # Create poisson generators, neurons and spike recorder
    pg_exc = nest.Create("poisson_generator", params={"rate": K_exc * (nu + nu_x)})
    pg_inh = nest.Create("poisson_generator", params={"rate": K_inh * nu})
    pg_pre = nest.Create("poisson_generator", params={"rate": nu})

    parrot = nest.Create("parrot_neuron")
    neuron = nest.Create("iaf_psc_alpha", params={"tau_minus": tau_minus})

    sr_pre = nest.Create("spike_recorder")
    sr_post = nest.Create("spike_recorder")

    # Set defaults for stdp_synapse
    # Note: "lambda" is a Python keyword, but can be used as a dict key
    nest.SetDefaults(
        "stdp_synapse",
        {
            "weight": w_neuron,
            "alpha": alpha,
            "lambda": lambda_val,  # noqa: A001
            "tau_plus": tau_plus,
            "mu_plus": mu_plus,
            "mu_minus": mu_minus,
            "Wmax": w_max,
        },
    )

    # Connect
    nest.Connect(pg_exc, neuron, syn_spec={"weight": w_exc, "delay": delay})
    nest.Connect(pg_inh, neuron, syn_spec={"weight": w_inh, "delay": delay})
    nest.Connect(pg_pre, parrot, syn_spec={"weight": w_exc, "delay": delay})

    nest.Connect(parrot, neuron, syn_spec={"synapse_model": "stdp_synapse", "weight": w_neuron, "delay": delay})

    nest.Connect(parrot, sr_pre)
    nest.Connect(neuron, sr_post)

    # Simulate and get data
    nest.Simulate(T)

    pre_spikes = np.array(sr_pre.get("events", "times")) + axonal_delay
    post_spikes = np.array(sr_post.get("events", "times")) + backpr_delay

    # Get final weight
    conns = nest.GetConnections(source=parrot, target=neuron)
    assert len(conns) == 1, "Should have exactly one connection"
    final_weight = conns[0].get("weight")

    # Check final weight by reproducing STDP calculations
    K_plus = 0.0
    K_minus = 0.0
    last_pre = 0.0
    last_post = 0.0
    j = 0
    i = 0

    if len(post_spikes) == 0 or len(pre_spikes) == 0:
        pytest.skip("No spikes recorded, cannot test STDP")

    post_spike = post_spikes[i] if i < len(post_spikes) else float("inf")
    pre_spike = pre_spikes[j] if j < len(pre_spikes) else float("inf")
    w = w_neuron / w_max  # normalized weight

    def update_K_plus():
        nonlocal K_plus
        K_plus = K_plus * np.exp((last_pre - pre_spike) / tau_plus) + 1.0

    def update_K_minus():
        nonlocal K_minus
        K_minus = K_minus * np.exp((last_post - post_spike) / tau_minus) + 1.0

    def next_pre_spike():
        nonlocal j, last_pre, pre_spike
        j += 1
        last_pre = pre_spike
        if j < len(pre_spikes):
            pre_spike = pre_spikes[j]
        else:
            pre_spike = float("inf")

    def next_post_spike():
        nonlocal i, last_post, post_spike
        i += 1
        last_post = post_spike
        if i < len(post_spikes):
            post_spike = post_spikes[i]
        else:
            post_spike = float("inf")

    def facilitate():
        nonlocal w
        w = w + lambda_val * (1.0 - w) ** mu_plus * K_plus * np.exp((last_pre - post_spike) / tau_plus)
        w = min(w, 1.0)

    def depress():
        nonlocal w
        w = w - lambda_val * alpha * w ** mu_minus * K_minus * np.exp((last_post - pre_spike) / tau_minus)
        w = max(w, 0.0)

    # Process spikes
    while j < len(pre_spikes) or i < len(post_spikes):
        if pre_spike == post_spike:
            # pre- and post-syn. spike at the same time
            if last_post != post_spike:
                facilitate()
            if last_pre != pre_spike:
                depress()
            if j + 1 < len(pre_spikes):
                update_K_plus()
                next_pre_spike()
                if i + 1 < len(post_spikes):
                    update_K_minus()
                    next_post_spike()
            else:
                break
        elif pre_spike < post_spike:
            # next spike is a pre-syn. spike
            depress()
            update_K_plus()
            if j + 1 < len(pre_spikes):
                next_pre_spike()
            else:
                # we don't consider the post-syn. spikes after the last pre-syn. spike
                break
        else:
            # next spike is a post-syn. spike
            facilitate()
            update_K_minus()
            if i + 1 < len(post_spikes):
                next_post_spike()
            else:
                # we DO consider the pre-syn. spikes after the last post-syn. spike
                last_post = post_spike
                # Set post_spike to a large value to make sure we don't come here again
                post_spike = pre_spikes[-1] + resolution if len(pre_spikes) > 0 else float("inf")

    # Compare final weight
    w_final = w * w_max
    w_final_rounded = round(w_final, 13)
    final_weight_rounded = round(final_weight, 13)

    assert w_final_rounded == pytest.approx(
        final_weight_rounded, abs=1e-12
    ), f"Final weight mismatch: calculated={w_final_rounded}, simulated={final_weight_rounded}"
