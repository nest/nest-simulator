# -*- coding: utf-8 -*-
#
# test_iaf_psc_exp_ps_lossless.py
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
Synopsis: (test_iaf_psc_exp_ps_lossless) run -> compares response to current step with analytical solution and tests lossless spike detection

Description:
test_iaf_psc_exp_ps_lossless.py is an overall test of the iaf_psc_exp_ps_lossless model connected
to some useful devices.

The first part of this test is exactly the same as test_iaf_psc_exp_ps,
demonstrating the numerical equivalency of both models in usual conditions.
The only difference between the models, which is tested in the second part,
is the detection of double threshold crossings during a simulation step
(so the membrane potential is again below V_th at the end of the step)
by the lossless model.

The second part tests whether the lossless spike detection algorithm [1] is
working correctly.

The algorithm checks whether a spike is emitted on the basis of the neurons position
in state space. There are 4 regions in state space (see [1]): NS1, NS2, S1 and S2.
S1 corresponds to threshold crossings that would also be detected by the lossy 
implementation /iaf_psc_exp_ps. S2 corresponds to crossings that would be missed.
The lossless model detects both.

The test brings 3 neurons into the state space regions NS2, S1 and S2,
which is done by keeping their membrane potential close to threshold and then
sending a single spike to them, which, received via different synaptic weights,
sets the synaptic current such that the neurons are in the respective region.
The existence and precise times of the resulting spikes are compared to reference data.

If you need to reproduce the reference data, ask the authors of [1] for the script
regions_algorithm.py which they used to generate Fig. 6. Here you can adjust the
parameters as wished and obtain the respective regions.


References:
[1] Krishnan J, Porta Mana P, Helias M, Diesmann M and Di Napoli E
    (2018) Perfect Detection of Spikes in the Linear Sub-threshold
    Dynamics of Point Neurons. Front. Neuroinform. 11:75.
    doi: 10.3389/fninf.2017.00075

Author:  Jeyashree Krishnan, 2017, and Christian Keup, 2018
SeeAlso: test_iaf_psc_exp, test_iaf_psc_exp_ps
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest


def test_precise_spiking_dc_input():
    dt = 0.1
    dc_amp = 1000.0

    nest.ResetKernel()
    nest.set(resolution=dt, local_num_threads=1)

    dc_gen = nest.Create("dc_generator", {"amplitude": dc_amp})
    nrn = nest.Create("iaf_psc_exp_ps_lossless", 1)
    vm = nest.Create("voltmeter", {"interval": 0.1})

    syn_spec = {"synapse_model": "static_synapse", "weight": 1.0, "delay": dt}
    nest.Connect(dc_gen, nrn, syn_spec=syn_spec)
    nest.Connect(vm, nrn, syn_spec=syn_spec)

    nest.Simulate(8.0)

    times = vm.get("events", "times")
    times -= dt  # account for delay to multimeter

    tau_m = nrn.get("tau_m")
    R = tau_m / nrn.get("C_m")
    theta = nrn.get("V_th")
    E_L = nrn.get("E_L")

    # array for analytical solution
    V_m_analytical = np.empty_like(times)
    V_m_analytical[:] = nrn.get("E_L")

    # first index for which the DC current is received by neuron.
    # DC current will be integrated from this time step
    start_index = 1

    # analytical solution without delay and threshold on a grid
    vm_soln = E_L + (1 - np.exp(-times / tau_m)) * R * dc_amp

    # exact time at which the neuron will spike
    exact_spiketime = -tau_m * np.log(1 - (theta - E_L) / (R * dc_amp))

    # offset from grid point
    time_offset = exact_spiketime - (exact_spiketime // dt) * dt

    # solution calculated on the grid, with t0 being the exact spike time
    vm_soln_offset = E_L + (1 - np.exp(-(times - time_offset + dt) / tau_m)) * R * dc_amp

    # rise until threshold
    V_m_analytical[start_index:] = vm_soln[:-start_index]

    # set refractory potential
    # first index after spike, offset by time at which DC current arrives
    crossing_ind = int(exact_spiketime // dt + 1) + start_index
    num_ref = int(nrn.get("t_ref") / dt)
    V_m_analytical[crossing_ind : crossing_ind + num_ref] = nrn.get("V_reset")

    # rise after refractory period
    num_inds = len(times) - crossing_ind - num_ref
    V_m_analytical[crossing_ind + num_ref :] = vm_soln_offset[:num_inds]

    nptest.assert_array_almost_equal(V_m_analytical, vm.get("events", "V_m"))


def test_lossless_spike_detection():
    nest.ResetKernel()
    nest.set(local_num_threads=1, resolution=1.0)  # low resolution is crucial for the test

    params = {"tau_m": 100.0, "tau_syn_ex": 1.0, "tau_syn_in": 1.0, "C_m": 250.0, "V_th": -49.0}

    nest.SetDefaults("iaf_psc_exp_ps_lossless", params)

    # 3 neurons that will test the detection of different types of threshold crossing
    nrn_nospike = nest.Create("iaf_psc_exp_ps_lossless")
    nrn_missingspike = nest.Create("iaf_psc_exp_ps_lossless")
    nrn_spike = nest.Create("iaf_psc_exp_ps_lossless")

    # syn weights of trigger spike that will put the nrn in the different state space regions
    w_nospike = 55.0
    w_missingspike = 70.0
    w_spike = 90.0

    # send one trigger spike to the nrns at specified time:
    sp_gen = nest.Create("spike_generator", {"precise_times": True, "spike_times": [3.0]})

    nest.Connect(sp_gen, nrn_nospike)
    nest.Connect(sp_gen, nrn_missingspike)
    nest.Connect(sp_gen, nrn_spike)

    # external current to keep nrns close to threshold:
    dc_gen = nest.Create("dc_generator", {"amplitude": 52.5})

    nest.Connect(dc_gen, nrn_nospike, syn_spec={"delay": 1.0, "weight": 1})
    nest.Connect(dc_gen, nrn_missingspike, syn_spec={"delay": 1.0, "weight": 1})
    nest.Connect(dc_gen, nrn_spike, syn_spec={"delay": 1.0, "weight": 1})

    # read out spike response of nrns:
    sr_nospike = nest.Create("spike_recorder")
    nest.Connect(nrn_nospike, sr_nospike)

    sr_missingspike = nest.Create("spike_recorder")
    nest.Connect(nrn_missingspike, sr_missingspike)

    sr_spike = nest.Create("spike_recorder")
    nest.Connect(nrn_spike, sr_spike)

    nest.Simulate(2.0)

    # set nrns close to threshold
    nrn_nospike.V_m = -49.001
    nrn_missingspike.V_m = -49.001
    nrn_spike.V_m = -49.001

    # swich off ext. current. This effect will reach the nrns at 3.0 due to syn delay,
    # so that the external current will be zero when the trigger spike arrives at 4.0 .
    dc_gen.amplitude = 0

    nest.Simulate(10.0)

    # get spike times
    time_nospike = sr_nospike.events["times"]
    time_missingspike = sr_missingspike.events["times"]
    time_spike = sr_spike.events["times"]
    print(time_nospike)
    assert len(time_nospike) == 0
    assert time_missingspike == pytest.approx(4.01442)
    assert time_spike == pytest.approx(4.00659)
