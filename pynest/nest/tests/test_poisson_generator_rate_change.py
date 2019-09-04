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

    def test_statistical_rate_change(self):
        """Statistical test of poisson_generator_ps rate change"""

        # Lower limit of acceptable p_value
        p_value_lim = 0.3

        def kstest_first_spiketimes(sd, start_t):
            events = nest.GetStatus(sd)[0]['events']
            senders = events['senders']
            times = events['times']
            min_times = [np.min(times[np.where(senders == s)])
                         for s in np.unique(senders)]
            d, p_val = scipy.stats.kstest(
                min_times, 'expon', args=(start_t + 0.1, 10.))
            print('p_value =', p_val)
            self.assertGreater(p_val, p_value_lim)

        nest.ResetKernel()
        pg = nest.Create('poisson_generator_ps', params={'rate': 100.})
        parrots = nest.Create('parrot_neuron_ps', 1000)
        sd = nest.Create('spike_detector', params={'start': 0., 'stop': 100.})
        nest.Connect(pg, parrots, syn_spec={'delay': 0.1})
        nest.Connect(parrots, sd, syn_spec={'delay': 0.1})

        # First simulation
        nest.Simulate(100)
        kstest_first_spiketimes(sd, 0.0)

        # Second simulation, with rate = 0
        nest.SetStatus(pg, {'rate': 0.})
        # We need to skip a timestep to not receive the spikes from the
        # previous simulation run that were sent, but not received.
        nest.SetStatus(sd, {'n_events': 0, 'start': 100.1, 'stop': 200.})
        nest.Simulate(100)
        self.assertEqual(nest.GetStatus(sd)[0]['n_events'], 0)

        # Third simulation, with rate increased back up to 100
        nest.SetStatus(pg, {'rate': 100.})
        nest.SetStatus(sd, {'n_events': 0, 'start': 200., 'stop': 300.})
        nest.Simulate(100)
        kstest_first_spiketimes(sd, 200.)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPgRateChange)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
