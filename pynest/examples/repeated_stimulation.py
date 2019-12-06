# -*- coding: utf-8 -*-
#
# repeated_stimulation.py
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
Repeated Stimulation
--------------------

Simple example for how to repeat a stimulation protocol
using the ``origin`` property of devices.

In this example, a ``poisson_generator`` generates a spike train that is
recorded directly by a ``spike_detector``, using the following paradigm:

1. A single trial last for 1000 ms.
2. Within each trial, the ``poisson_generator`` is active from 100 ms to 500 ms.

We achieve this by defining the `start` and `stop` properties of the
generator to 100 ms and 500 ms, respectively, and setting the ``origin`` to the
simulation time at the beginning of each trial. Start and stop are interpreted
relative to the ``origin``.

"""


###############################################################################
# First, the modules needed for simulation and analyis are imported.


import nest
import nest.raster_plot


###############################################################################
# Second, we set the parameters so the ``poisson_generator`` generates 1000
# spikes per second and is active from 100 to 500 ms


rate = 1000.0  # generator rate in spikes/s
start = 100.0  # start of simulation relative to trial start, in ms
stop = 500.0  # end of simulation relative to trial start, in ms


###############################################################################
# The simulation is supposed to take 1s (1000 ms) and is repeated 5 times


trial_duration = 1000.0  # trial duration, in ms
num_trials = 5      # number of trials to perform


###############################################################################
# Third, the network is set up.  We reset the kernel and create a
# ``poisson_generator``, in which the handle is stored in `pg`.
#
# The parameters for rate and start and stop of activity are given as optional
# parameters in the form of a dictionary.


nest.ResetKernel()
pg = nest.Create('poisson_generator',
                 params={'rate': rate,
                         'start': start,
                         'stop': stop}
                 )


###############################################################################
# The ``spike_detector`` is created and the handle stored in `sd`.


sd = nest.Create('spike_detector')


###############################################################################
# The ``Connect`` function connects the nodes so spikes from pg are collected by
# the ``spike_detector`` `sd`


nest.Connect(pg, sd)


###############################################################################
# Before each trial, we set the ``origin`` of the ``poisson_generator`` to the
# current simulation time. This automatically sets the start and stop time of
# the ``poisson_generator`` to the specified times with respect to the origin.
# The simulation is then carried out for the specified time in trial_duration.


for n in range(num_trials):
    nest.SetStatus(pg, {'origin': nest.GetKernelStatus()['time']})
    nest.Simulate(trial_duration)


###############################################################################
# Now we plot the result, including a histogram using the ``nest.raster_plot``
# function. Note: The histogram will show spikes seemingly located before
# 100 ms into each trial. This is due to sub-optimal automatic placement of
# histogram bin borders.

nest.raster_plot.from_device(sd, hist=True, hist_binwidth=100.,
                             title='Repeated stimulation by Poisson generator')
