# -*- coding: utf-8 -*-
#
# test_poisson_generator_rate_change.py
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


import nest
import unittest
import scipy.stats
import numpy as np


class TestPgRateChange(unittest.TestCase):

    def _kstest_first_spiketimes(self, sr, start_t, rate, n_parrots, resolution, p_value_lim):
        scale_parameter = n_parrots / rate
        events = nest.GetStatus(sr)[0]['events']
        senders = events['senders']
        times = events['times']
        min_times = [np.min(times[np.where(senders == s)])
                     for s in np.unique(senders)]
        d, p_val = scipy.stats.kstest(
            min_times, 'expon', args=(start_t + resolution, scale_parameter))
        print('p_value =', p_val)
        self.assertGreater(p_val, p_value_lim)

    def test_statistical_rate_change(self):
        """Statistical test of poisson_generator_ps rate change"""

        p_value_lim = 0.05  # Lower limit of acceptable p_value
        resolution = 0.25  # Simulation resolution
        n_parrots = 1000  # Number of parrot neurons
        sim_time = 100  # Time to simulate

        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': resolution})
        rate = 100.
        pg = nest.Create('poisson_generator_ps', params={'rate': rate})
        parrots = nest.Create('parrot_neuron_ps', n_parrots)
        sr = nest.Create('spike_recorder',
                         params={'start': 0., 'stop': float(sim_time)})
        nest.Connect(pg, parrots, syn_spec={'delay': resolution})
        nest.Connect(parrots, sr, syn_spec={'delay': resolution})

        # First simulation
        nest.Simulate(sim_time)
        self._kstest_first_spiketimes(sr, 0., rate, n_parrots, resolution, p_value_lim)

        # Second simulation, with rate = 0
        rate = 0.
        nest.SetStatus(pg, {'rate': rate})
        # We need to skip a timestep to not receive the spikes from the
        # previous simulation run that were sent, but not received.
        nest.SetStatus(sr, {'n_events': 0,
                            'start': float(sim_time) + resolution,
                            'stop': 2. * sim_time})
        nest.Simulate(sim_time)
        self.assertEqual(nest.GetStatus(sr)[0]['n_events'], 0)

        # Third simulation, with rate increased back up to 100
        rate = 100.
        nest.SetStatus(pg, {'rate': rate})
        nest.SetStatus(sr, {'n_events': 0,
                            'start': 2. * sim_time,
                            'stop': 3. * sim_time})
        nest.Simulate(sim_time)
        self._kstest_first_spiketimes(sr, 2. * sim_time, rate, n_parrots, resolution, p_value_lim)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPgRateChange)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
