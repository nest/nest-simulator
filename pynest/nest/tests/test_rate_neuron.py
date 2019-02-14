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

# This script tests the rate_neuron in NEST. For input noise we test
# the mean and variance of the rate as well as the standard deviation of
# the input noise. For output noise we test the mean of the rate and the
# standard deviation of the output noise, which already determines the variance
# of the rate.

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
        self.neuron_params = {'mu': 1.5, 'sigma': 0.5, 'tau': 5.}

        # simulation parameters
        self.simtime = 10000.
        self.dt = 0.1
        self.tstart = 10. * self.neuron_params['tau']

        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus(
            {'resolution': self.dt, 'use_wfr': False, 'print_time': True})

        # set up rate neuron and devices
        self.rate_neuron_ipn = nest.Create(
            'lin_rate_ipn', params=self.neuron_params)
        self.rate_neuron_opn = nest.Create(
            'lin_rate_opn', params=self.neuron_params)
        self.multimeter = nest.Create(
            "multimeter", params={'record_from': ['rate', 'noise'],
                                  'interval': self.dt, 'start': self.tstart})

        # record rates and noise
        nest.Connect(
            self.multimeter, self.rate_neuron_ipn + self.rate_neuron_opn)

    def test_RateNeuronMean(self):
        """Check the mean value of the rate_neurons"""

        # simulate
        nest.Simulate(self.simtime)

        # get noise from rate neurons
        events = nest.GetStatus(self.multimeter)[0]["events"]
        senders = events['senders']
        senders_ipn = np.where(senders == self.rate_neuron_ipn)[0]
        senders_opn = np.where(senders == self.rate_neuron_opn)[0]

        mean_rate_ipn = np.mean(events['rate'][senders_ipn])
        mean_rate_opn = np.mean(events['rate'][senders_opn])

        self.assertTrue(
            np.isclose(mean_rate_ipn, self.neuron_params['mu'],
                       rtol=self.rtol))
        self.assertTrue(
            np.isclose(mean_rate_opn, self.neuron_params['mu'],
                       rtol=self.rtol))

    def test_RateNeuronNoise(self):
        """Check noise of the rate_neurons"""

        # simulate
        nest.Simulate(self.simtime)

        # get noise from rate neurons
        events = nest.GetStatus(self.multimeter)[0]["events"]
        senders = events['senders']
        senders_ipn = np.where(senders == self.rate_neuron_ipn)[0]
        senders_opn = np.where(senders == self.rate_neuron_opn)[0]

        noise_ipn = events['noise'][senders_ipn]
        std_noise_ipn = np.std(noise_ipn)
        noise_opn = events['noise'][senders_opn]
        std_noise_opn = np.std(noise_opn)

        self.assertTrue(
            np.isclose(std_noise_ipn, self.neuron_params['sigma'],
                       rtol=self.rtol))
        self.assertTrue(
            np.isclose(std_noise_opn, self.neuron_params['sigma'],
                       rtol=self.rtol))

    def test_RateNeuronVariance(self):
        """Check the variance of the rate of the rate_neuron for input noise"""

        # simulate
        nest.Simulate(self.simtime)

        # get variance of the rate
        events = nest.GetStatus(self.multimeter)[0]["events"]
        senders = events['senders']
        senders_ipn = np.where(senders == self.rate_neuron_ipn)[0]

        rate = events['rate'][senders_ipn]
        var_rate = np.var(rate)

        # expected variance
        var_test = self.neuron_params[
            'sigma']**2 / 2.

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
