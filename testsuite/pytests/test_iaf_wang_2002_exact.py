# -*- coding: utf-8 -*-
#
# test_iaf_wang_2002.py
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
import numpy as np


class IafWang2002TestCase(unittest.TestCase):
    """Tests for iaf_wang_2002"""

    def setup(self):
        nest.ResetKernel()

    def test_multiple_NMDA_ports(self):
        """
        Check that setting multiple NMDA receptors works
        """
        # Create the new model, noise and detectors
        neuron = nest.Create('iaf_wang_2002')
        poiss = nest.Create('poisson_generator')
        poiss.rate = 6400.

        voltmeter = nest.Create('voltmeter')
        voltmeter.set(record_from=['V_m', 'g_AMPA', 'g_GABA', 'NMDA_sum'])

        # Connect to NMDA receptor several times to check that we create new ports every time.
        receptors = neuron.get('receptor_types')

        nest.Connect(poiss, neuron, syn_spec={'receptor_type': receptors['AMPA']})
        nest.Connect(poiss, neuron, syn_spec={'receptor_type': receptors['GABA']})
        nest.Connect(poiss, neuron, syn_spec={'receptor_type': receptors['NMDA']})
        nest.Connect(poiss, neuron, syn_spec={'receptor_type': receptors['NMDA']})
        nest.Connect(poiss, neuron, syn_spec={'receptor_type': receptors['NMDA']})
        nest.Connect(poiss, neuron, syn_spec={'receptor_type': receptors['NMDA']})

        nest.Connect(voltmeter, neuron)

        # Check if NMDA sum is 0 before simulating
        self.assertEqual(neuron.NMDA_sum, 0.)

        # Simulate
        nest.Simulate(1000.)

        # Check sum NMDA after simulating
        self.assertTrue(neuron.NMDA_sum > 0.)

        # Check g_AMPA after simulating
        self.assertTrue(voltmeter.get('events', 'g_AMPA').any() > 0.)

def suite():
    suite = unittest.makeSuite(IafWang2002TestCase, 'test')
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
