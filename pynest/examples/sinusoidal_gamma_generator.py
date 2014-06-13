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
Short demonstration of the sinusoidal_gamma_generator for AC Gamma trains.
"""

# import NEST
import nest
nest.ResetKernel()   # in case we run the script multiple times from iPython

# import plotting tools
import matplotlib.pyplot as plt
import numpy as np

plt.figure()  # figure window for Part 1

# Part 1A: Illustrate two generators -------------------------------------------------------

nest.SetKernelStatus({'resolution': 0.01})

# create two generators with different orders
g = nest.Create('sinusoidal_gamma_generator', n=2,
                params=[{'dc': 10000.0, 'ac': 5000.0, 'freq': 10.0, 'phi': 0.0, 'order': 2.0},
                        {'dc': 10000.0, 'ac': 5000.0, 'freq': 10.0, 'phi': 0.0, 'order': 10.0}])

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
    plt.hist(np.diff(sp), bins=np.arange(0., 0.505, 0.01),
             histtype='step', color=colors[j])
    plt.title('ISI histogram')

# Part 1B: Illustrate /individual_spike_trains switch -----------------------------------------

# individual spike trains first
nest.ResetKernel()
nest.SetKernelStatus({'local_num_threads': 4})  # show this work for multiple threads

g = nest.Create('sinusoidal_gamma_generator',
                params={'dc': 100.0, 'ac': 50.0, 'freq': 10.0, 'phi': 0.0, 'order': 3.,
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

g = nest.Create('sinusoidal_gamma_generator',
                params={'dc': 100.0, 'ac': 50.0, 'freq': 10.0, 'phi': 0.0, 'order': 3.,
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



# Part 2: Illustrate change of rate/phase/frequency --------------------------------------

def step(t, n, initial, after, seed=1, dt=0.05):
    """Simulates for n generators for t ms. Step at T/2."""

    ## prepare/reset nest
    nest.ResetKernel()
    ## initialize simulation
    #np.random.seed(256 * seed)
    nest.SetStatus([0],[{"resolution": dt}])
    nest.SetStatus([0],[{"grng_seed": 256 * seed + 1}])
    nest.SetStatus([0],[{"rng_seeds": [256 * seed + 2]}])    

    g = nest.Create('sinusoidal_gamma_generator', n, params=initial)
    sd = nest.Create('spike_detector')
    nest.ConvergentConnect(g, sd)
    nest.Simulate(t/2)
    nest.SetStatus(g, after)
    nest.Simulate(t/2)

    return nest.GetStatus(sd, 'events')[0]


def plot_hist(spikes):
    plt.hist(spikes['times'],
             bins=np.arange(0.,max(spikes['times'])+1.5,1.),
             histtype='step')

t = 1000
n = 1000
dt = 1.0
steps = t/dt
offset = t/1000.*2*np.pi


grid = (2,3)
fig = plt.figure(figsize=(15,10))

## Defaults for everything but dc. Step up
plt.subplot(grid[0], grid[1], 1)
spikes = step(t, n,
              {'dc': 20.0},
              {'dc': 50.0,},
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.ones(steps)   
exp[:steps/2] *= 20
exp[steps/2:] *= 50
plt.plot(exp, 'r')
plt.title('DC rate: 20 -> 50')
plt.ylabel('Spikes per second')

## Set all parameters. Step down
plt.subplot(grid[0], grid[1], 2)
spikes = step(t, n,
              {'order': 6.0, 'dc': 80.0, 'ac': 0., 'freq': 0., 'phi': 0.},
              {'order': 6.0, 'dc': 40.0, 'ac': 0., 'freq': 0., 'phi': 0.},
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.ones(steps)
exp[:steps/2] *= 80
exp[steps/2:] *= 40
plt.plot(exp, 'r')
plt.title('DC rate: 80 -> 40')

## Set all parameters. ac change
plt.subplot(grid[0], grid[1], 3)
spikes = step(t, n,
              {'order': 3.0, 'dc': 40.0, 'ac': 40., 'freq': 10., 'phi': 0.},
              {'order': 3.0, 'dc': 40.0, 'ac': 20., 'freq': 10., 'phi': 0.},
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.zeros(steps)
exp[:steps/2] = 40. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2))) + 40.
exp[steps/2:] = 20. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2)) + offset) + 40.
plt.plot(exp, 'r')
plt.title('Rate Modulation: 40 -> 20')

## dc change with non-zero ac
plt.subplot(grid[0], grid[1], 4)
spikes = step(t, n,
              {'order': 6.0, 'dc': 20.0, 'ac': 20., 'freq': 10., 'phi': 0.},
              {'order': 6.0, 'dc': 50.0, 'ac': 50., 'freq': 10., 'phi': 0.},
              seed=123, dt=dt)
plot_hist(spikes)
exp = np.zeros(steps)
exp[:steps/2] = 20. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2))) + 20.
exp[steps/2:] = 50. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2)) + offset) + 50.
plt.plot(exp, 'r')
plt.title('DC Rate and Rate Modulation: 20 -> 50')
plt.ylabel('Spikes per second')
plt.xlabel('Time [ms]')


## Mostly defaults. ac up
plt.subplot(grid[0], grid[1], 5)
spikes = step(t, n,
              {'dc': 40.0,},
              {'ac': 40.0, 'freq': 20.},
              seed=123, dt=1.)
plot_hist(spikes)
exp = np.zeros(steps)
exp[:steps/2] = np.ones(steps/2) * 40.
exp[steps/2:] = 40. * np.sin(np.arange(0, t/1000.*np.pi*20, t/1000.*np.pi*20./(steps/2))) + 40.
plt.plot(exp, 'r')
plt.title('Rate Modulation: 0 -> 40')
plt.xlabel('Time [ms]')


#Phase shift    
plt.subplot(grid[0], grid[1], 6)
spikes = step(t, n,
              {'order': 6.0, 'dc': 60.0, 'ac': 60., 'freq': 10., 'phi': 0.},
              {'order': 6.0, 'dc': 60.0, 'ac': 60., 'freq': 10., 'phi': np.pi},
              seed=123, dt=1.)
plot_hist(spikes)
exp = np.zeros(steps)
exp[:steps/2] = 60. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2))) + 60.
exp[steps/2:] = 60. * np.sin(np.arange(0, t/1000.*np.pi*10, t/1000.*np.pi*10./(steps/2)) + offset + np.pi)  + 60.
plt.plot(exp, 'r')
plt.title('Modulation Phase: 0 -> Pi')
plt.xlabel('Time [ms]')
