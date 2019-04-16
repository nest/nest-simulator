# -*- coding: utf-8 -*-
#
# test_split_simulation.py
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

import unittest
import nest


class TestSplit(unittest.TestCase):
    steps = 100
    time = 100

    def __init__(self, *args, **kwargs):
        super(TestSplit, self).__init__(*args, **kwargs)
        self.spike = None

    def setup(self):
        nest.ResetKernel()
        nest.SetDefaults('spike_detector', {'withtime': True})

        n1 = nest.Create("iaf_psc_alpha")
        nest.SetStatus(n1, {"I_e": 376.0})

        self.spike = spike = nest.Create('spike_detector')
        nest.Connect(n1, spike)

    def runner(self, time, f):
        spike = self.spike
        nest.SetStatus(spike, [{'n_events': 0}])

        f(time)
        spikes = nest.GetStatus(spike, 'events')[0]
        senders, times = spikes['senders'], spikes['times']

        return zip(senders, times)

    def runs(self):
        self.setup()
        steps, time = self.steps, self.time

        with nest.RunManager():
            return [
              (s, t)
              for _ in range(steps)
              for s, t in self.runner(time, nest.Run)
            ]

    def simulate(self):
        self.setup()
        steps, time = self.steps, self.time

        return [
            (s, t)
            for s, t in self.runner(time * steps, nest.Simulate)
        ]

    def test_split_match(self):
        r0 = self.runs()
        r1 = self.simulate()
        self.assertEqual(r0, r1)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSplit)
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
