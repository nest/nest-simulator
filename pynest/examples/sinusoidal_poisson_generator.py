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
Sinusoidal poisson generator example
------------------------------------

This script demonstrates the use of the ``sinusoidal_poisson_generator``
and its different parameters and modes. The source code of the model
can be found in ``models/sinusoidal_poisson_generator.h``.

The script is structured into two parts and creates one common figure.
In Part 1, two instances of the ``sinusoidal_poisson_generator`` are
created with different parameters. Part 2 illustrates the effect of
the ``individual_spike_trains`` switch.

"""


###############################################################################
# We import the modules required to simulate, analyze and plot this example.


import nest
import matplotlib.pyplot as plt
import numpy as np

nest.ResetKernel()   # in case we run the script multiple times from iPython


####################################################################################
# We create two instances of the ``sinusoidal_poisson_generator`` with two
# different parameter sets using ``Create``. Moreover, we create devices to
# record firing rates (``Multimeter``) and spikes (``spike_detector``) and connect
# them to the generators using ``Connect``.


nest.SetKernelStatus({'resolution': 0.01})

g = nest.Create('sinusoidal_poisson_generator', n=2,
                params=[{'rate': 10000.0,
                         'amplitude': 5000.0,
                         'frequency': 10.0,
                         'phase': 0.0},
                        {'rate': 0.0,
                         'amplitude': 10000.0,
                         'frequency': 5.0,
                         'phase': 90.0}])

m = nest.Create('multimeter', n=2, {'interval': 0.1, 'record_from': ['rate']})
s = nest.Create('spike_detector', n=2)

nest.Connect(m, g, 'one_to_one')
nest.Connect(g, s, 'one_to_one')
print(nest.GetStatus(m))
nest.Simulate(200)


###############################################################################
# After simulating, the spikes are extracted from the ``spike_detector`` using
# ``GetStatus`` and plots are created with panels for the PST and ISI histograms.


colors = ['b', 'g']

for j in range(2):

    ev = nest.GetStatus(m[j])[0]['events']
    t = ev['times']
    r = ev['rate']

    sp = nest.GetStatus(s[j])[0]['events']['times']
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


###############################################################################
# The kernel is reset and the number of threads set to 4.


nest.ResetKernel()
nest.SetKernelStatus({'local_num_threads': 4})


###############################################################################
# A ``sinusoidal_poisson_generator`` with  ``individual_spike_trains`` set to
# `True` is created and connected to 20 parrot neurons whose spikes are
# recorded by a ``spike_detector``. After simulating, a raster plot of the spikes
# is created.


g = nest.Create('sinusoidal_poisson_generator',
                params={'rate': 100.0, 'amplitude': 50.0,
                        'frequency': 10.0, 'phase': 0.0,
                        'individual_spike_trains': True})
p = nest.Create('parrot_neuron', 20)
s = nest.Create('spike_detector')

nest.Connect(g, p, 'all_to_all')
nest.Connect(p, s, 'all_to_all')

nest.Simulate(200)
ev = nest.GetStatus(s)[0]['events']
plt.subplot(222)
plt.plot(ev['times'], ev['senders'] - min(ev['senders']), 'o')
plt.ylim([-0.5, 19.5])
plt.yticks([])
plt.title('Individual spike trains for each target')


###############################################################################
# The kernel is reset again and the whole procedure is repeated for a
# ``sinusoidal_poisson_generator`` with `individual_spike_trains` set to
# `False`. The plot shows that in this case, all neurons receive the same
# spike train from the ``sinusoidal_poisson_generator``.


nest.ResetKernel()
nest.SetKernelStatus({'local_num_threads': 4})

g = nest.Create('sinusoidal_poisson_generator',
                params={'rate': 100.0, 'amplitude': 50.0,
                        'frequency': 10.0, 'phase': 0.0,
                        'individual_spike_trains': False})
p = nest.Create('parrot_neuron', 20)
s = nest.Create('spike_detector')

nest.Connect(g, p, 'all_to_all')
nest.Connect(p, s, 'all_to_all')

nest.Simulate(200)
ev = nest.GetStatus(s)[0]['events']
plt.subplot(224)
plt.plot(ev['times'], ev['senders'] - min(ev['senders']), 'o')
plt.ylim([-0.5, 19.5])
plt.yticks([])
plt.title('One spike train for all targets')
