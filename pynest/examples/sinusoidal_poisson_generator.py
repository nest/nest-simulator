# -*- coding: utf-8 -*-
#
# sinusoidal_poisson_generator.py
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
#
"""
Short demonstration of the sinusoidal_poisson_generator for AC Poisson trains.
"""

# import NEST
import nest
nest.ResetKernel()   # in case we run the script multiple times from iPython

# import plotting tools
import matplotlib.pyplot as plt
import numpy as np

# Part 1: Illustrate two generators -------------------------------------------------------

nest.SetKernelStatus({'resolution': 0.01})

# create two generators with different frequencies, phases, amplitudes
g = nest.Create('sinusoidal_poisson_generator', n=2, params=[{'dc': 10000.0, 'ac': 5000.0,
                                                              'freq': 10.0, 'phi': 0.0},
                                                             {'dc': 0.0, 'ac': 10000.0,
                                                              'freq': 5.0, 'phi': np.pi/2.}])

# create multimeters and spike detectors
m = nest.Create('multimeter', n=2, params={'interval': 0.1, 'withgid': False,
                                           'record_from': ['rate']})
s = nest.Create('spike_detector', n=2, params={'withgid': False})

nest.Connect(m, g)
nest.Connect(g, s)

nest.Simulate(200)

colors = ['b', 'g']

for j in range(2):

    ev = nest.GetStatus([m[j]])[0]['events']
    t = ev['times']
    r = ev['rate']

    sp = nest.GetStatus([s[j]])[0]['events']['times']
    plt.subplot(221)
    h, e = np.histogram(sp, bins=np.arange(0., 201., 5.))
    plt.plot(t, r, color=colors[j])
    plt.step(e[:-1], h * 1000 / 5., color=colors[j], where='post')
    plt.title('PST histogram and firing rates')
    plt.ylabel('Spikes per second')

    plt.subplot(223)
    plt.hist(np.diff(sp), bins=np.arange(0., 1.005, 0.02),
             histtype='step', color=colors[j])
    plt.title('ISI histogram')

# Part 2: Illustrate /individual_spike_trains switch -----------------------------------------

# individual spike trains first
nest.ResetKernel()
nest.SetKernelStatus({'local_num_threads': 4})  # show this work for multiple threads

g = nest.Create('sinusoidal_poisson_generator',
                params={'dc': 100.0, 'ac': 50.0, 'freq': 10.0, 'phi': 0.0,
                        'individual_spike_trains': True})
p = nest.Create('parrot_neuron', 20)
s = nest.Create('spike_detector')

nest.DivergentConnect(g, p)
nest.ConvergentConnect(p, s)

nest.Simulate(200)
ev = nest.GetStatus(s)[0]['events']
plt.subplot(222)
plt.plot(ev['times'], ev['senders']-min(ev['senders']), 'o')
plt.ylim([-0.5, 19.5])
plt.yticks([])
plt.title('Individual spike trains for each target')


# now one spike train for all targets
nest.ResetKernel()
nest.SetKernelStatus({'local_num_threads': 4})  # show this work for multiple threads

g = nest.Create('sinusoidal_poisson_generator',
                params={'dc': 100.0, 'ac': 50.0, 'freq': 10.0, 'phi': 0.0,
                        'individual_spike_trains': False})
p = nest.Create('parrot_neuron', 20)
s = nest.Create('spike_detector')

nest.DivergentConnect(g, p)
nest.ConvergentConnect(p, s)

nest.Simulate(200)
ev = nest.GetStatus(s)[0]['events']
plt.subplot(224)
plt.plot(ev['times'], ev['senders']-min(ev['senders']), 'o')
plt.ylim([-0.5, 19.5])
plt.yticks([])
plt.title('One spike train for all targets')
