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
using the 'origin' property of devices.

In this example, a poisson_generator generates a spike train that is
recorded directly by a spike_detector, using the following paradigm:

1. A single trial last for 1000ms.
2. Within each trial, the poisson_generator is active from 100ms to 500ms.

We achieve this by defining the 'start' and 'stop' properties of the
generator to 100ms and 500ms, respectively, and setting the 'origin' to the
simulation time at the beginning of each trial. Start and stop are interpreted
relative to the origin.

KEYWORDS:examples plot repeated stimulatioy 
"""



import nest
import nest.raster_plot



rate = 1000.0  # generator rate in spikes/s
start = 100.0  # start of simulation relative to trial start, in ms
stop = 500.0  # end of simulation relative to trial start, in ms



trial_duration = 1000.0  # trial duration, in ms
num_trials = 5      # number of trials to perform



nest.ResetKernel()
pg = nest.Create('poisson_generator',
                 params={'rate': rate,
                         'start': start,
                         'stop': stop}
                 )



sd = nest.Create('spike_detector')



nest.Connect(pg, sd)



for n in range(num_trials):
    nest.SetStatus(pg, {'origin': nest.GetKernelStatus()['time']})
    nest.Simulate(trial_duration)


nest.raster_plot.from_device(sd, hist=True, hist_binwidth=100.,
                             title='Repeated stimulation by Poisson generator')
