# -*- coding: utf-8 -*-
#
# test_events.py
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
Test of events
"""

import unittest
import nest


@nest.check_stack
class EventsTestCase(unittest.TestCase):
    """Tests of the Connect API"""

    def test_Events_1(self):
        """Recorder Events"""

        nest.ResetKernel()

        sd = nest.Create('spike_detector', 1, {'withtime': True})
        d = nest.GetStatus(sd, 'events')[0]

        senders = d['senders']
        times = d['times']

        vm = nest.Create('voltmeter', 1, {'withtime': True})
        d = nest.GetStatus(vm, 'events')[0]

        senders = d['V_m']
        times = d['times']

    def test_EventsVoltage(self):
        """Voltage Events"""

        nest.ResetKernel()

        nest.sr('20 setverbosity')
        n = nest.Create('iaf_psc_alpha')
        vm = nest.Create('voltmeter', 1, {'withtime': True, 'interval': 1.})

        nest.Connect(vm, n)
        nest.SetKernelStatus({'print_time': False})
        nest.Simulate(10)

        d = nest.GetStatus(vm, 'events')[0]

        self.assertEqual(len(d['V_m']), 9)

    def test_EventsSpikes(self):
        """Spike Events"""

        nest.ResetKernel()

        nest.sr('20 setverbosity')

        n = nest.Create('iaf_psc_alpha', 1, {'I_e': 1000.})
        sd = nest.Create('spike_detector', 1, {'withtime': True})

        nest.Connect(n, sd)
        nest.SetKernelStatus({'print_time': False})
        nest.Simulate(1000)

        d = nest.GetStatus(sd, 'events')[0]

        self.assert_(len(d['times']) > 0)


def suite():

    suite = unittest.makeSuite(EventsTestCase, 'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
