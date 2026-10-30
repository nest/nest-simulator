# -*- coding: utf-8 -*-
#
# test_iaf_psc_exp_hpc.py
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
Test that ``iaf_psc_exp_hpc`` gives exactly the same results as ``iaf_psc_exp`` and lacks its additional functionality.
"""

import nest
import numpy as np
import pytest

# parameters of the microcircuit model of Potjans and Diesmann (2014), different from the defaults
PD14_PARAMS = {
    "C_m": 250.0,
    "tau_m": 10.0,
    "tau_syn_ex": 0.5,
    "tau_syn_in": 0.5,
    "t_ref": 2.0,
    "E_L": -65.0,
    "V_th": -50.0,
    "V_reset": -65.0,
}


def build_and_run(model, threads, params):
    """Random network of excitatory and inhibitory neurons with Poisson and DC input; returns spikes and traces."""

    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.rng_seed = 1234

    n_ex, n_in = 80, 20
    neurons = nest.Create(model, n_ex + n_in, params=dict(params, I_e=200.0))
    neurons.V_m = nest.random.uniform(-70.0, -55.0)
    noise = nest.Create("poisson_generator", params={"rate": 8000.0})
    dc = nest.Create("dc_generator", params={"amplitude": 150.0, "start": 50.0, "stop": 150.0})
    rec = nest.Create("spike_recorder")
    mm = nest.Create("multimeter", params={"record_from": ["V_m", "I_syn_ex", "I_syn_in"], "interval": 0.1})

    nest.Connect(noise, neurons, syn_spec={"weight": 15.0, "delay": 1.0})
    nest.Connect(dc, neurons[::3], syn_spec={"delay": 1.0})
    nest.Connect(
        neurons[:n_ex],
        neurons,
        {"rule": "fixed_indegree", "indegree": 20},
        {"weight": nest.random.normal(20.0, 4.0), "delay": nest.random.uniform(0.5, 3.0)},
    )
    nest.Connect(
        neurons[n_ex:],
        neurons,
        {"rule": "fixed_indegree", "indegree": 10},
        {"weight": -80.0, "delay": nest.random.uniform(0.5, 3.0)},
    )
    nest.Connect(neurons, rec)
    nest.Connect(mm, neurons[:10])

    # two calls to Simulate, so that state is carried across them
    nest.Simulate(100.0)
    nest.Simulate(100.0)

    sp = rec.get("events")
    tr = mm.get("events")
    sp_order = np.lexsort((sp["senders"], sp["times"]))
    order = np.lexsort((tr["senders"], tr["times"]))
    return (
        sp["senders"][sp_order],
        sp["times"][sp_order],
        tr["senders"][order],
        tr["V_m"][order],
        tr["I_syn_ex"][order],
        tr["I_syn_in"][order],
    )


@pytest.mark.parametrize(
    "threads",
    [
        1,
        pytest.param(2, marks=pytest.mark.skipif_missing_threads),
        pytest.param(3, marks=pytest.mark.skipif_missing_threads),
    ],
)
@pytest.mark.parametrize("params", [{}, PD14_PARAMS], ids=["defaults", "pd14"])
def test_identical_to_iaf_psc_exp(threads, params):
    """Spikes, membrane potentials and synaptic currents are bit-identical to ``iaf_psc_exp`` with ``delta = 0``."""

    ref = build_and_run("iaf_psc_exp", threads, params)
    assert len(ref[0]) > 0, "network must spike"
    for x, y in zip(ref, build_and_run("iaf_psc_exp_hpc", threads, params)):
        np.testing.assert_array_equal(x, y)


def test_parameters():
    """The parameters of the stochastic spiking do not exist; the other parameters have the defaults of iaf_psc_exp."""

    nest.ResetKernel()
    ref = nest.Create("iaf_psc_exp")
    nrn = nest.Create("iaf_psc_exp_hpc")
    status = nrn.get()
    assert "delta" not in status and "rho" not in status
    for key in ["E_L", "C_m", "tau_m", "t_ref", "V_th", "V_reset", "tau_syn_ex", "tau_syn_in", "I_e", "V_m"]:
        assert status[key] == ref.get(key)
    with pytest.raises(nest.NESTErrors.UnaccessedDictionaryEntry):
        nrn.set(delta=1.0)
    with pytest.raises(nest.NESTErrors.UnaccessedDictionaryEntry):
        nrn.set(rho=1.0)


def test_current_input_only_on_receptor_0():
    """Current input is accepted on receptor type 0 only; receptor type 1 of iaf_psc_exp does not exist."""

    nest.ResetKernel()
    nrn = nest.Create("iaf_psc_exp_hpc")
    dc = nest.Create("dc_generator")
    nest.Connect(dc, nrn, syn_spec={"receptor_type": 0})
    with pytest.raises(nest.NESTErrors.UnknownReceptorType):
        nest.Connect(dc, nrn, syn_spec={"receptor_type": 1})
