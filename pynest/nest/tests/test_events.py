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


@nest.ll_api.check_stack
class EventsTestCase(unittest.TestCase):
    """Tests of the Connect API"""

    def test_EventsVoltage(self):
        """Voltage Events"""

        nest.ResetKernel()

        nest.ll_api.sr('20 setverbosity')
        n = nest.Create('iaf_psc_alpha')
        vm = nest.Create('voltmeter', params={'interval': 1.})

        nest.Connect(vm, n)
        nest.Simulate(10)

        d = nest.GetStatus(vm, 'events')[0]

        self.assertEqual(len(d['V_m']), 9)

    def test_EventsSpikes(self):
        """Spike Events"""

        nest.ResetKernel()

        nest.ll_api.sr('20 setverbosity')

        n = nest.Create('iaf_psc_alpha', params={'I_e': 1000.})
        sd = nest.Create('spike_detector')

        nest.Connect(n, sd)
        nest.Simulate(1000)

        d = nest.GetStatus(sd, 'events')[0]

        self.assert_(len(d['times']) > 0)


def suite():

    suite = unittest.makeSuite(EventsTestCase, 'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
