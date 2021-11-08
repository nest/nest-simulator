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
    spike = None

    def __init__(self, *args, **kwargs):
        super(TestSplit, self).__init__(*args, **kwargs)

    def setup_and_extract_data(f):
        def _decorator(self):
            nest.ResetKernel()
            n1 = nest.Create("iaf_psc_alpha", params={"I_e": 376.0})
            self.spike = nest.Create('spike_recorder')
            nest.Connect(n1, self.spike)
            f(self)
            events = self.spike.events
            return list(zip(events['senders'], events['times']))

        return _decorator

    @setup_and_extract_data
    def runs(self):
        with nest.RunManager():
            for _ in range(self.steps):
                nest.Run(self.time)

    @setup_and_extract_data
    def simulate(self):
        nest.Simulate(self.time * self.steps)

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
