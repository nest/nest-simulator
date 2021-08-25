# -*- coding: utf-8 -*-
#
# recording_demo.py
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
Recording examples
------------------

This script demonstrates how to select different recording backends
and read the result data back in. The simulated network itself is
rather boring with only a single poisson generator stimulating a
single neuron, so we get some data.

"""

import nest
import numpy as np


def setup(record_to, time_in_steps):
    """Set up the network with the given parameters."""

    nest.ResetKernel()
    nest.SetKernelStatus({'overwrite_files': True})

    pg_params = {'rate': 1000000.}
    sr_params = {'record_to': record_to, 'time_in_steps': time_in_steps}

    n = nest.Create('iaf_psc_exp')
    pg = nest.Create('poisson_generator', 1, pg_params)
    sr = nest.Create('spike_recorder', 1, sr_params)

    nest.Connect(pg, n, syn_spec={'weight': 10.})
    nest.Connect(n, sr)

    return sr


def get_data(sr):
    """Get recorded data from the spike_recorder."""

    if sr.record_to == 'ascii':
        return np.loadtxt(f'{sr.filenames[0]}', dtype=object)
    if sr.record_to == 'memory':
        return sr.get('events')


# Just loop through some recording backends and settings
for time_in_steps in (True, False):
    for record_to in ('ascii', 'memory'):
        sr = setup(record_to, time_in_steps)
        nest.Simulate(30.0)
        data = get_data(sr)
        print(f"simulation resolution in ms: {nest.GetKernelStatus('resolution')}")
        print(f"data recorded by recording backend {record_to} (time_in_steps={time_in_steps})")
        print(data)
