#! /usr/bin/env python
#
# smp_generator.py
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
Short demonstration of the smp_generator for AC Poisson trains.
"""

# import nest and Hill-Tononi module
import nest
nest.ResetKernel()

# import plotting tools
import matplotlib.pyplot as plt
import numpy as np

# create two generators with different frequencies, phases, amplitudes
g = nest.Create('smp_generator', n=2, params=[{'dc': 10000.0, 'ac': 5000.0, 
                                              'freq': 10.0, 'phi': 0.0},
                                             {'dc': 0.0, 'ac': 10000.0, 
                                              'freq': 5.0, 'phi': np.pi/2.}])

# create multimeters and spike detectors
m = nest.Create('multimeter', n=2, params={'interval': 0.1, 'withgid': False,
                                           'record_from': ['Rate']})
s = nest.Create('spike_detector', n=2, params={'withgid': False})

nest.Connect(m, g)
nest.Connect(g, s)

nest.Simulate(200)

for j in xrange(2):
    ev = nest.GetStatus([m[j]])[0]['events']
    t = ev['times']
    r = ev['Rate']
    plt.subplot(211)
    plt.plot(t, r, '-')

    sp = nest.GetStatus([s[j]])[0]['events']['times']
    plt.subplot(212)
    plt.hist(sp, bins=20, range=[0, 200])

plt.show()
