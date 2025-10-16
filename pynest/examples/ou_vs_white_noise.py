# -*- coding: utf-8 -*-
#
# ou_vs_white_noise.py
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
Comparing white and OU noise
----------------------------

This example compares two neurons: one driven by the ``noise_generator`` (white noise)
and one driven by the ``ou_noise_generator`` (temporally correlated noise).
We match the mean and variance of both inputs and report firing rate,
ISI coefficient of variation, and membrane-potential statistics.
"""

import nest
import numpy as np

###############################################################################
# First, we reset the kernel and define the simulation resolution.
# The update interval of the generators must be an
# integer multiple of the resolution.

nest.ResetKernel()
nest.resolution = 0.1
dt = nest.resolution

###############################################################################
# Second, we create two identical LIF neurons, one multimeter and one spike
# recorder per neuron.

simtime = 500_000.0

lif_params = dict(C_m=250.0, tau_m=20.0, t_ref=2.0, V_reset=-70.0, E_L=-70.0, V_th=-54.0)
n_white = nest.Create("iaf_psc_alpha", params=lif_params)
n_ou = nest.Create("iaf_psc_alpha", params=lif_params)

mm_white = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": dt})
mm_ou = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": dt})
sr_white = nest.Create("spike_recorder")
sr_ou = nest.Create("spike_recorder")

###############################################################################
# Third, we create and configure the generators. We use the same mean and
# stationary standard deviation for both inputs and set ``tau`` only for the
# OU generator.

mean_I, std_I = 250.0, 50.0
tau_ou = 50.0
ng_white = nest.Create("noise_generator", {"mean": mean_I, "std": std_I, "dt": dt})
ng_ou = nest.Create("ou_noise_generator", {"mean": mean_I, "std": std_I, "tau": tau_ou, "dt": dt})

###############################################################################
# Fourth, we connect generators to neurons and measurement devices to neurons.

nest.Connect(ng_white, n_white)
nest.Connect(n_white, sr_white)
nest.Connect(mm_white, n_white)
nest.Connect(ng_ou, n_ou)
nest.Connect(n_ou, sr_ou)
nest.Connect(mm_ou, n_ou)

###############################################################################
# Now we simulate the network.

nest.Simulate(simtime)

###############################################################################
# Finally, we compute the membrane-potential 'mean/std' (after a 1 s burn-in),
# firing rate, and ISI coefficient of variation.


def isi_cv(spike_times):
    if len(spike_times) < 3:
        return np.nan
    isi = np.diff(np.asarray(spike_times))
    m = isi.mean()
    return np.nan if m == 0 else isi.std(ddof=1) / m


def vm_stats(mm, burn_in_ms=1000.0):
    ev = mm.get("events")
    t = np.asarray(ev["times"])
    vm = np.asarray(ev["V_m"])
    keep = t >= burn_in_ms
    vm = vm[keep]
    return vm.mean(), vm.std(ddof=1)


def spike_stats(sr, simtime_ms):
    ev = sr.get("events")
    times = np.asarray(ev["times"])
    rate = 1000.0 * len(times) / simtime_ms
    return rate, isi_cv(times)


vm_mean_w, vm_std_w = vm_stats(mm_white)
vm_mean_o, vm_std_o = vm_stats(mm_ou)
rate_w, cv_w = spike_stats(sr_white, simtime)
rate_o, cv_o = spike_stats(sr_ou, simtime)

print("=== White noise (noise_generator) ===")
print(f"Rate: {rate_w:.2f} Hz, ISI CV: {cv_w:.2f}, Vm mean: {vm_mean_w:.2f} mV, Vm std: {vm_std_w:.2f} mV")
print("=== OU noise (ou_noise_generator) ===")
print(f"Rate: {rate_o:.2f} Hz, ISI CV: {cv_o:.2f}, Vm mean: {vm_mean_o:.2f} mV, Vm std: {vm_std_o:.2f} mV")
