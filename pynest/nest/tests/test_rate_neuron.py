# -*- coding: utf-8 -*-
#
# test_rate_neuron.py
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

# This script tests the rate_neuron in NEST.

import nest
import unittest
import numpy as np


@nest.check_stack
class RateNeuronTestCase(unittest.TestCase):

    """Check rate_neuron"""

    def setUp(self):
        # test parameter
        self.rtol = 0.05

        # neuron parameters
        self.neuron_params = {'mean': 1.5, 'std': 0.5, 'tau': 5.}

        # simulation parameters
        self.simtime = 1000.
        self.dt = 0.1
        self.tstart = 10. * self.neuron_params['tau']

        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus(
            {'resolution': self.dt, 'use_wfr': False, 'print_time': True})

        # set up rate neuron and devices
        self.rate_neuron = nest.Create(
            'lin_rate_ipn', params=self.neuron_params)
        self.multimeter = nest.Create(
            "multimeter", params={'record_from': ['rate', 'noise'],
                                  'interval': self.dt, 'start': self.tstart})

        # record rates and noise
        nest.Connect(self.multimeter, self.rate_neuron)

    def test_RateNeuronMean(self):
        """Check the mean value of the rate_neuron"""

        # simulate
        nest.Simulate(self.simtime)

        # get noise from rate neuron
        events = nest.GetStatus(self.multimeter)[0]["events"]
        mean_rate = np.mean(events['rate'])

        self.assertTrue(
            np.isclose(mean_rate, self.neuron_params['mean'], rtol=self.rtol))

    def test_RateNeuronNoise(self):
        """Check the input noise of the rate_neuron"""

        # simulate
        nest.Simulate(self.simtime)

        # get noise from rate neuron
        events = nest.GetStatus(self.multimeter)[0]["events"]
        noise = events['noise']
        std_noise = np.std(noise)

        self.assertTrue(
            np.isclose(std_noise, self.neuron_params['std'], rtol=self.rtol))

    def test_RateNeuronVariance(self):
        """Check the variance of the rate of the rate_neuron"""

        # simulate
        nest.Simulate(self.simtime)

        # get variance from rate neuron output
        events = nest.GetStatus(self.multimeter)[0]["events"]
        rate = events['rate']
        var_rate = np.var(rate)

        # expected variance
        var_test = self.neuron_params[
            'std']**2 / 2.

        # assert
        self.assertTrue(
            np.isclose(var_rate, var_test, rtol=self.rtol))


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        RateNeuronTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
