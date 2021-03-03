# -*- coding: utf-8 -*-
#
# sinusoidal_gamma_generator.py
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
Sinusoidal gamma generator example
----------------------------------

This script demonstrates the use of the ``sinusoidal_gamma_generator`` and its
different parameters and modes. The source code of the model can be found in
``models/sinusoidal_gamma_generator.h``.

The script is structured into two parts, each of which generates its own
figure. In part 1A, two generators are created with different orders of the
underlying gamma process and their resulting PST (Peristiumulus time) and ISI
(Inter-spike interval) histograms are plotted. Part 1B illustrates the effect
of the ``individual_spike_trains`` switch. In Part 2, the effects of
different settings for rate, phase and frequency are demonstrated.

"""


###############################################################################
# First, we import all necessary modules to simulate, analyze and
# plot this example.


import nest
import matplotlib.pyplot as plt
import numpy as np

nest.ResetKernel()   # in case we run the script multiple times from iPython


###############################################################################
# We first create a figure for the plot and set the resolution of NEST.


plt.figure()
nest.SetKernelStatus({'resolution': 0.01})


###############################################################################
# Then we create two instances of the ``sinusoidal_gamma_generator`` with two
# different orders of the underlying gamma process using ``Create``. Moreover,
# we create devices to record firing rates (``multimeter``) and spikes
# (``spike_recorder``) and connect them to the generators using ``Connect``.


g = nest.Create('sinusoidal_gamma_generator', n=2,
                params=[{'rate': 10000.0, 'amplitude': 5000.0,
                         'frequency': 10.0, 'phase': 0.0, 'order': 2.0},
                        {'rate': 10000.0, 'amplitude': 5000.0,
                         'frequency': 10.0, 'phase': 0.0, 'order': 10.0}])

m = nest.Create('multimeter', 2, {'interval': 0.1, 'record_from': ['rate']})
s = nest.Create('spike_recorder', 2)

nest.Connect(m, g, 'one_to_one')
nest.Connect(g, s, 'one_to_one')

nest.Simulate(200)


###############################################################################
# After simulating, the spikes are extracted from the ``spike_recorder`` using
# ``GetStatus`` and plots are created with panels for the PST and ISI histograms.

colors = ['b', 'g']

for j in range(2):

    ev = m[j].events
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
    plt.hist(np.diff(sp), bins=np.arange(0., 0.505, 0.01),
             histtype='step', color=colors[j])
    plt.title('ISI histogram')


###############################################################################
# The kernel is reset and the number of threads set to 4.


nest.ResetKernel()
nest.SetKernelStatus({'local_num_threads': 4})


###############################################################################
# First, a ``sinusoidal_gamma_generator`` with ``individual_spike_trains`` set to
# `True` is created and connected to 20 parrot neurons whose spikes are
# recorded by a spike recorder. After simulating, a raster plot of the spikes
# is created.

g = nest.Create('sinusoidal_gamma_generator',
                params={'rate': 100.0, 'amplitude': 50.0,
                        'frequency': 10.0, 'phase': 0.0, 'order': 3.,
                        'individual_spike_trains': True})
p = nest.Create('parrot_neuron', 20)
s = nest.Create('spike_recorder')

nest.Connect(g, p)
nest.Connect(p, s)

nest.Simulate(200)
ev = s.events
plt.subplot(222)
plt.plot(ev['times'], ev['senders'] - min(ev['senders']), 'o')
plt.ylim([-0.5, 19.5])
plt.yticks([])
plt.title('Individual spike trains for each target')


#################################################################################
# The kernel is reset again and the whole procedure is repeated for a
# ``sinusoidal_gamma_generator`` with ``individual_spike_trains`` set to `False`.
# The plot shows that in this case, all neurons receive the same spike train
# from the ``sinusoidal_gamma_generator``.


nest.ResetKernel()
nest.SetKernelStatus({'local_num_threads': 4})

g = nest.Create('sinusoidal_gamma_generator',
                params={'rate': 100.0, 'amplitude': 50.0,
                        'frequency': 10.0, 'phase': 0.0, 'order': 3.,
                        'individual_spike_trains': False})
p = nest.Create('parrot_neuron', 20)
s = nest.Create('spike_recorder')

nest.Connect(g, p)
nest.Connect(p, s)

nest.Simulate(200)
ev = s[0].events
plt.subplot(224)
plt.plot(ev['times'], ev['senders'] - min(ev['senders']), 'o')
plt.ylim([-0.5, 19.5])
plt.yticks([])
plt.title('One spike train for all targets')


###############################################################################
# In part 2, multiple generators are created with different settings for rate,
# phase and frequency. First, we define an auxiliary function, which simulates
# `n` generators for `t` ms. After `t/2`, the parameter dictionary of the
# generators is changed from initial to after.

def step(t, n, initial, after, seed=1, dt=0.05):

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": dt, "grng_seed": 256 * seed + 1,
                          "rng_seeds": [256 * seed + 2]})

    g = nest.Create('sinusoidal_gamma_generator', n, params=initial)
    sr = nest.Create('spike_recorder')
    nest.Connect(g, sr)
    nest.Simulate(t / 2)
    g.set(after)
    nest.Simulate(t / 2)

    return sr.events


###############################################################################
# This function serves to plot a histogram of the emitted spikes.

def plot_hist(spikes):
    plt.hist(spikes['times'],
             bins=np.arange(0., max(spikes['times']) + 1.5, 1.),
             histtype='step')


t = 1000
n = 1000
dt = 1.0
steps = int(t / dt)
offset = t / 1000. * 2 * np.pi


# We create a figure with a 2x3 grid.


grid = (2, 3)
fig = plt.figure(figsize=(15, 10))


###############################################################################
# We simulate a ``sinusoidal_gamma_generator`` with default parameter values,
# i.e. ``ac=0`` and the DC value being changed from 20 to 50 after `t/2` and
# plot the number of spikes per second over time.


plt.subplot(grid[0], grid[1], 1)
spikes = step(t, n,
              {'rate': 20.0},
              {'rate': 50.0, },
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.ones(int(steps))
exp[:int(steps / 2)] *= 20
exp[int(steps / 2):] *= 50
plt.plot(exp, 'r')
plt.title('DC rate: 20 -> 50')
plt.ylabel('Spikes per second')


###############################################################################
# We simulate a ``sinusoidal_gamma_generator`` with the DC value being changed
# from 80 to 40 after `t/2` and plot the number of spikes per second over
# time.


plt.subplot(grid[0], grid[1], 2)
spikes = step(t, n,
              {'order': 6.0, 'rate': 80.0, 'amplitude': 0.,
               'frequency': 0., 'phase': 0.},
              {'order': 6.0, 'rate': 40.0, 'amplitude': 0.,
               'frequency': 0., 'phase': 0.},
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.ones(int(steps))
exp[:int(steps / 2)] *= 80
exp[int(steps / 2):] *= 40
plt.plot(exp, 'r')
plt.title('DC rate: 80 -> 40')


###############################################################################
# Next, we simulate a ``sinusoidal_gamma_generator`` with the AC value being
# changed from 40 to 20 after `t/2` and plot the number of spikes per
# second over time.


plt.subplot(grid[0], grid[1], 3)
spikes = step(t, n,
              {'order': 3.0, 'rate': 40.0, 'amplitude': 40.,
               'frequency': 10., 'phase': 0.},
              {'order': 3.0, 'rate': 40.0, 'amplitude': 20.,
               'frequency': 10., 'phase': 0.},
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.zeros(int(steps))
exp[:int(steps / 2)] = (40. +
                        40. * np.sin(np.arange(0, t / 1000. * np.pi * 10,
                                               t / 1000. * np.pi * 10. /
                                               (steps / 2))))
exp[int(steps / 2):] = (40. + 20. * np.sin(np.arange(0, t / 1000. * np.pi * 10,
                                                     t / 1000. * np.pi * 10. /
                                                     (steps / 2)) + offset))
plt.plot(exp, 'r')
plt.title('Rate Modulation: 40 -> 20')


##################################################################################
# Finally, we simulate a ``sinusoidal_gamma_generator`` with a non-zero AC value
# and the DC value being changed from 80 to 40 after `t/2` and plot the
# number of spikes per second over time.


plt.subplot(grid[0], grid[1], 4)
spikes = step(t, n,
              {'order': 6.0, 'rate': 20.0, 'amplitude': 20.,
               'frequency': 10., 'phase': 0.},
              {'order': 6.0, 'rate': 50.0, 'amplitude': 50.,
               'frequency': 10., 'phase': 0.},
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.zeros(int(steps))
exp[:int(steps / 2)] = (20. + 20. * np.sin(np.arange(0, t / 1000. * np.pi * 10,
                                                     t / 1000. * np.pi * 10. /
                                                     (steps / 2))))
exp[int(steps / 2):] = (50. + 50. * np.sin(np.arange(0, t / 1000. * np.pi * 10,
                                                     t / 1000. * np.pi * 10. /
                                                     (steps / 2)) + offset))
plt.plot(exp, 'r')
plt.title('DC Rate and Rate Modulation: 20 -> 50')
plt.ylabel('Spikes per second')
plt.xlabel('Time [ms]')


###############################################################################
# Simulate a ``sinusoidal_gamma_generator`` with the AC value being
# changed from 0 to 40 after `t/2` and plot the number of spikes per
# second over time.


plt.subplot(grid[0], grid[1], 5)
spikes = step(t, n,
              {'rate': 40.0, },
              {'amplitude': 40.0, 'frequency': 20.},
              seed=123, dt=1.)
plot_hist(spikes)
exp = np.zeros(int(steps))
exp[:int(steps / 2)] = 40. * np.ones(int(steps / 2))
exp[int(steps / 2):] = (40. + 40. * np.sin(np.arange(0, t / 1000. * np.pi * 20,
                                                     t / 1000. * np.pi * 20. /
                                                     (steps / 2))))
plt.plot(exp, 'r')
plt.title('Rate Modulation: 0 -> 40')
plt.xlabel('Time [ms]')


###############################################################################
# Simulate a ``sinusoidal_gamma_generator`` with a phase shift at
# `t/2` and plot the number of spikes per second over time.


# Phase shift
plt.subplot(grid[0], grid[1], 6)
spikes = step(t, n,
              {'order': 6.0, 'rate': 60.0, 'amplitude': 60.,
               'frequency': 10., 'phase': 0.},
              {'order': 6.0, 'rate': 60.0, 'amplitude': 60.,
               'frequency': 10., 'phase': 180.},
              seed=123, dt=1.)
plot_hist(spikes)
exp = np.zeros(int(steps))

exp[:int(steps / 2)] = (60. + 60. * np.sin(np.arange(0, t / 1000. * np.pi * 10,
                                                     t / 1000. * np.pi * 10. /
                                                     (steps / 2))))
exp[int(steps / 2):] = (60. + 60. * np.sin(np.arange(0, t / 1000. * np.pi * 10,
                                                     t / 1000. * np.pi * 10. /
                                                     (steps / 2)) +
                                           offset + np.pi))
plt.plot(exp, 'r')
plt.title('Modulation Phase: 0 -> Pi')
plt.xlabel('Time [ms]')
plt.show()
