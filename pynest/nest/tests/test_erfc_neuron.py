# -*- coding: utf-8 -*-
#
# test_erfc_neuron.py
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
Test implementation of erfc-neuron.
"""

import unittest
import nest
import numpy as np
from scipy.special import erfc


def get_mean_activity(detector, T):
    """
    returns the mean activity of a single binary neuron connected to a spin
    detector.
    """
    states = nest.GetStatus(detector)[0]['events']['state']
    times = nest.GetStatus(detector)[0]['events']['times']
    # add total duration at the end, since we need to take into account
    # the time between the last state change and end of simulation
    times = np.hstack((times, T))
    if len(times) > 1:
        assert(states[0] == 1)
        # since neuron is starting in 0 state, summing every second period
        # will give us the total time in the up state
        activity = np.sum(np.diff(times)[::2]) / (T - times[0])
        # if we have more than one update, we calculate a more accurate value
        # for the mean activity, taking into account that our measurements are
        # biased (we only record /given/ that a state change happened)
        if len(times) > 2:
            # biased average starting at down state, p(m(t)=1|m(t-1)=0)
            M0 = 1. - (np.sum(np.diff(times)[1::2]) / (T - times[1]))
            # biased average starting at up state, p(m(t)=1|m(t-1)=1)
            M1 = activity
            # unbiased estimate,
            # p(m(t)=1)=\sum_{s \in {0,1}} p(m(t)=1|m(t-1)=s)p(m(t-1)=s),
            # assuming stationary state: p(m(t)) = p(m(t-1)), solved for p(m=1)
            activity = M0 / (1. + M0 - M1)
        return activity
    else:
        return 0.


def activation_function_theory(sigma, theta):
    """
    returns the probability for a binary neuron to be in the up state, given
    the parameters sigma and theta.
    """
    return 0.5 * erfc(theta / (np.sqrt(2.) * sigma))


class ErfcNeuronTheoryTestCase(unittest.TestCase):

    """Compare results to theoretical predictions"""

    def setUp(self):
        """defines parameters of simulation"""
        self.sigma = np.logspace(-1, 1.1, 4)
        self.theta = np.linspace(-6, 6, 15)
        self.neuron = None
        self.detector = None
        self.T = 30000.

    def build_and_connect_nodes(self, sigma, theta):
        """ sets up an erfc neuron and spin detector. """
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        self.neuron = nest.Create('erfc_neuron', 1,
                                  {'sigma': sigma, 'theta': theta})
        self.detector = nest.Create('spin_detector', 1)
        nest.Connect(self.neuron, self.detector)

    def test_activation_function(self):
        """
        simulates erfc neuron for different parameter sets and compares
        activity to theoretical value.
        """
        for sigma in self.sigma:
            for theta in self.theta:
                self.build_and_connect_nodes(sigma, theta)
                nest.Simulate(self.T)
                mean_activity = get_mean_activity(self.detector, self.T)
                mean_activity_theory = activation_function_theory(sigma, theta)
                delta = np.max([2e-1 * mean_activity_theory *
                                (1. -
                                 mean_activity_theory),
                                1e-2])
                self.assertAlmostEqual(
                    mean_activity,
                    mean_activity_theory,
                    delta=delta)


def suite():
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        ErfcNeuronTheoryTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
