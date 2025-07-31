# -*- coding: utf-8 -*-
#
# test_noise_generator.py
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
Tests parameter setting and statistical correctness for one application.
"""

import nest
import numpy as np
import pytest
from scipy.signal import fftconvolve


@pytest.fixture
def prepare_kernel():
    nest.ResetKernel()
    nest.resolution = 0.1

def burn_in_start(dt, tau, k=10.0):
    """
    Compute the array index corresponding to k*tau of warm-up.
    dt   : simulation time step (ms)
    tau  : OU time constant (ms)
    k    : how many taus to discard (default 10)
    """
    return int(round((k * tau) / dt))




def test_ou_noise_generator_set_parameters(prepare_kernel):
    params = {"mean": 210.0, "std": 60.0, "dt": 0.1}

    oung1 = nest.Create("ou_noise_generator")
    oung1.set(params)

    nest.SetDefaults("ou_noise_generator", params)

    oung2 = nest.Create("ou_noise_generator")
    assert oung1.get(params) == oung2.get(params)


def test_ou_noise_generator_incorrect_noise_dt(prepare_kernel):
    with pytest.raises(nest.kernel.NESTError, match="StepMultipleRequired"):
        nest.Create("ou_noise_generator", {"dt": 0.25})

def test_ou_noise_generator_statistics(prepare_kernel):
    # run for resolution dt=0.1 project to iaf_psc_alpha.
    # create 100 repetitions of 1000ms simulations
    # collect membrane potential at end
    #nest.rng_seed = 202

    #oung = nest.Create("ou_noise_generator")
    oung = nest.Create('ou_noise_generator', 1, {'mean':200.0, 'std': 60.0, 'tau':10, 'dt':0.1})
    neuron = nest.Create("iaf_psc_alpha")
    # we need to connect to a neuron otherwise the generator does not generate
    nest.Connect(oung, neuron)

    mm = nest.Create('multimeter', 1, {'record_from':['I']})
    nest.Connect(mm, oung, syn_spec={'weight': 1})

    # Simulate for 100 times
    n_sims = 100
    ou_current = np.empty(n_sims)
    for i in range(n_sims):
        nest.Simulate(1000.0)

        ou_current[i] = mm.get('events')['I'][-1]


    curr_mean = np.mean(ou_current)
    curr_var = np.var(ou_current)
    expected_curr_mean = oung.mean
    expected_curr_var = oung.std**2

    # require mean within 3 std dev, std dev within 0.2 std dev
    assert np.abs(curr_mean - expected_curr_mean) < 3 * oung.std
    assert np.abs(curr_var - expected_curr_var) < 0.2 * curr_var


def test_ou_noise_generator_autocorrelation():
    nest.rng_seed = 6789

    dt = 1.0
    tau = 1.0

    oung = nest.Create('ou_noise_generator', 1, {'mean':0.0, 'std': 60.0, 'tau':tau,'dt': nest.resolution})
    neuron = nest.Create("iaf_psc_alpha")
    # we need to connect to a neuron otherwise the generator does not generate
    nest.Connect(oung, neuron)
    mm = nest.Create('multimeter', 1, {'record_from':['I']})
    nest.Connect(mm, oung, syn_spec={'weight': 1})
    nest.Simulate(10000.0)

    
    ou_current = mm.get('events')['I']

    ## drop first k*tau of data
    cutoff = burn_in_start(dt=dt, tau=tau, k=10.0)
    x = ou_current[cutoff:]
    x -= x.mean()

    # empirical lag-1 autocorrelation
    emp_ac1 = np.dot(x[:-1], x[1:]) / ((len(x)-1)*x.var())
    # theoretical lag-1: exp(-dt/tau)
    theor_ac1 = np.exp(-dt / tau)


    assert abs(emp_ac1 - theor_ac1) < 0.05, \
        f"autocorr {emp_ac1:.3f} vs theoretical {theor_ac1:.3f}"
    
def _crosscorr_between_two_neurons(V_mean, V_std, dt, tau_m, t_max):
    nest.ResetKernel(); nest.resolution = dt
    ou = nest.Create("ou_noise_generator", 1,
                     {"mean": V_mean, "std": V_std, "tau": tau_m, "dt": dt})
    neurons = nest.Create("iaf_psc_alpha", 2,
                          {"E_L": 0.0, "V_th": 1e9, "C_m": 1.0, "tau_m": tau_m})
    nest.Connect(ou, neurons)
    mm = nest.Create("multimeter", 1, {"record_from": ["V_m"], "interval": dt})
    nest.Connect(mm, neurons)
    nest.Simulate(t_max)
    ev = mm.get("events")
    senders = np.asarray(ev["senders"], int)
    times = np.asarray(ev["times"])
    vms = np.asarray(ev["V_m"])
    t_steps = int(round(t_max / dt)) + 1
    V = np.zeros((t_steps, 2))
    for t, s, v in zip(times, senders, vms):
        V[int(round(t / dt)), neurons.index(s)] = v
    start = burn_in_start(dt, tau_m)
    X1 = V[start:, 0] - V[start:, 0].mean()
    X2 = V[start:, 1] - V[start:, 1].mean()
    corr = fftconvolve(X1, X2[::-1], mode="full")
    corr /= np.sqrt(np.dot(X1, X1) * np.dot(X2, X2))
    mid = corr.size // 2
    return corr[mid:]


def test_ou_noise_generator_cross_correlation(prepare_kernel):
    nest.rng_seed = 2321
    crosscorr = _crosscorr_between_two_neurons(
        V_mean=0.0, V_std=60.0, dt=nest.resolution, tau_m=1.0, t_max=30_000.0
    )
    assert float(np.max(np.abs(crosscorr))) < 0.05