# -*- coding: utf-8 -*-
#
# cross_check_mip_corrdet.py
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

"""Auto- and crosscorrelation functions for spike trains
-----------------------------------------------------------

A time bin of size `tbin` is centered around the time difference it
represents. If the correlation function is calculated for `tau` in
`[-tau_max, tau_max]`, the pair events contributing to the left-most
bin are those for which `tau` in `[-tau_max-tbin/2, tau_max+tbin/2)` and
so on.

Correlate two spike trains with each other assumes spike times to be ordered in
time. `tau > 0` means spike2 is later than spike1

* tau_max: maximum time lag in ms correlation function
* tbin:    bin size
* spike1:  first spike train [tspike...]
* spike2:  second spike train [tspike...]

"""

import nest
import numpy as np


def corr_spikes_sorted(spike1, spike2, tbin, tau_max, h):
    tau_max_i = int(tau_max / h)
    tbin_i = int(tbin / h)

    cross = np.zeros(int(2 * tau_max_i / tbin_i + 1), 'd')

    j0 = 0

    for spki in spike1:
        j = j0
        while j < len(spike2) and spike2[j] - spki < -tau_max_i - tbin_i / 2.0:
            j += 1
        j0 = j

        while j < len(spike2) and spike2[j] - spki < tau_max_i + tbin_i / 2.0:
            cross[int(
                (spike2[j] - spki + tau_max_i + 0.5 * tbin_i) / tbin_i)] += 1.0
            j += 1

    return cross


nest.ResetKernel()

h = 0.1             # Computation step size in ms
T = 100000.0        # Total duration
delta_tau = 10.0
tau_max = 100.0
pc = 0.5
nu = 100.0

# grng_seed is 0 because test data was produced for seed = 0
nest.SetKernelStatus({'local_num_threads': 1, 'resolution': h,
                      'overwrite_files': True, 'grng_seed': 0})

# Set up network, connect and simulate
mg = nest.Create('mip_generator')
mg.set(rate=nu, p_copy=pc)

cd = nest.Create('correlation_detector')
cd.set(tau_max=tau_max, delta_tau=delta_tau)

sr = nest.Create('spike_recorder', params={'time_in_steps': True})

pn1 = nest.Create('parrot_neuron')
pn2 = nest.Create('parrot_neuron')

nest.Connect(mg, pn1)
nest.Connect(mg, pn2)
nest.Connect(pn1, sr)
nest.Connect(pn2, sr)

nest.SetDefaults('static_synapse', {'weight': 1.0, 'receptor_type': 0})
nest.Connect(pn1, cd)

nest.SetDefaults('static_synapse', {'weight': 1.0, 'receptor_type': 1})
nest.Connect(pn2, cd)

nest.Simulate(T)

n_events = cd.get('n_events')
n1 = n_events[0]
n2 = n_events[1]

lmbd1 = (n1 / (T - tau_max)) * 1000.0
lmbd2 = (n2 / (T - tau_max)) * 1000.0

h = 0.1
tau_max = 100.0  # ms correlation window
t_bin = 10.0  # ms bin size

spikes = sr.get('events', 'senders')

sp1 = spikes[spikes == 4]
sp2 = spikes[spikes == 5]

# Find crosscorrolation
cross = corr_spikes_sorted(sp1, sp2, t_bin, tau_max, h)

print("Crosscorrelation:")
print(cross)
print("Sum of crosscorrelation:")
print(sum(cross))
