# -*- coding: utf-8 -*-
#
# test_regression_issue-1409.py
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
import numpy as np
import unittest

HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


@unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
class MultiplePoissonGeneratorsTestCase(unittest.TestCase):

    def test_multiple_poisson_generators(self):
        """Invariable number of spikes with multiple poisson generators"""
        local_num_threads = 4
        time_simulation = 100
        num_neurons = 1
        num_pg = 50
        num_iterations = 50

        num_spikes = []
        for i in range(num_iterations):
            nest.ResetKernel()
            nest.SetKernelStatus({'local_num_threads': local_num_threads})
            nest.set_verbosity('M_WARNING')
            print('num iter {:>5d}/{}'.format(i+1, num_iterations), end='\r')

            parrots = nest.Create('parrot_neuron', num_neurons)
            poisson_generator = nest.Create('poisson_generator', num_pg)
            poisson_generator.rate = 2000.

            nest.Connect(poisson_generator, parrots, 'all_to_all')

            nest.Simulate(time_simulation)
            num_spikes.append(nest.GetKernelStatus('local_spike_counter'))

        self.assertEqual(len(np.unique(num_spikes)), 1)


def suite():
    t = unittest.TestLoader().loadTestsFromTestCase(MultiplePoissonGeneratorsTestCase)
    return unittest.TestSuite([t])


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
