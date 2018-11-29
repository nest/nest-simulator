# -*- coding: utf-8 -*-
#
# test_siegert_neuron.py
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

# This script tests the siegert_neuron in NEST.

import nest
import unittest
import numpy as np

HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")


@nest.check_stack
@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
class SiegertNeuronTestCase(unittest.TestCase):
    """
    Test siegert_neuron

    Details
    -------
    Compares the rate of a Poisson-driven iaf_psc_delta neuron
    with the prediction from the siegert neuron.
    """

    def setUp(self):
        # test parameter to compare analytic solution to simulation
        self.rtol = 1.0

        # test parameters
        self.N = 100
        self.rate_ex = 1.5 * 1e4
        self.J = 0.1

        # simulation parameters
        self.simtime = 500.
        self.dt = 0.1
        self.start = 200.

        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus(
            {'resolution': self.dt, 'use_wfr': False, 'print_time': True})

        # set up driven integrate-and-fire neuron

        self.iaf_psc_delta = nest.Create(
            'iaf_psc_delta', self.N)  # , params={"C_m": 1.0})

        self.poisson_generator = nest.Create(
            'poisson_generator', params={'rate': self.rate_ex})
        nest.Connect(self.poisson_generator, self.iaf_psc_delta,
                     syn_spec={'weight': self.J, 'delay': self.dt})

        self.spike_detector = nest.Create(
            "spike_detector", params={'start': self.start})
        nest.Connect(
            self.iaf_psc_delta, self.spike_detector)

        # set up driven siegert neuron

        neuron_status = nest.GetStatus(self.iaf_psc_delta)[0]
        siegert_params = {'tau_m': neuron_status['tau_m'],
                          't_ref': neuron_status['t_ref'],
                          'theta': neuron_status['V_th'] -
                          neuron_status['E_L'],
                          'V_reset': neuron_status['V_reset'] -
                          neuron_status['E_L']}
        self.siegert_neuron = nest.Create(
            'siegert_neuron', params=siegert_params)

        self.siegert_drive = nest.Create(
            'siegert_neuron', 1,
            params={'mean': self.rate_ex, 'theta': siegert_params['theta']})
        J_mu_ex = neuron_status['tau_m'] * 1e-3 * self.J
        J_sigma_ex = neuron_status['tau_m'] * 1e-3 * self.J ** 2
        syn_dict = {'drift_factor': J_mu_ex, 'diffusion_factor':
                    J_sigma_ex, 'model': 'diffusion_connection'}
        nest.Connect(
            self.siegert_drive, self.siegert_neuron, syn_spec=syn_dict)

        self.multimeter = nest.Create(
            "multimeter", params={'record_from': ['rate'],
                                  'interval': self.dt})
        nest.Connect(
            self.multimeter, self.siegert_neuron)

    def test_RatePrediction(self):
        """Check the rate prediction of the siegert neuron"""

        # simulate
        nest.Simulate(self.simtime)

        # get rate prediction from siegert neuron
        events = nest.GetStatus(self.multimeter)[0]["events"]
        senders = events['senders']
        rate = events['rate'][np.where(senders == self.siegert_neuron)]
        rate_prediction = rate[-1]

        # get simulated rate of integrate-and-fire neuron
        rate_iaf = nest.GetStatus(self.spike_detector)[0][
            "n_events"] / ((self.simtime - self.start) * 1e-3) / self.N

        # test rate prediction against simulated rate of
        # integrate-and-fire neuron
        self.assertTrue(np.isclose(rate_iaf, rate_prediction, rtol=self.rtol))
        # test rate prediction against hard coded result
        rate_prediction_test = 27.1095934379
        self.assertTrue(np.isclose(rate_prediction_test, rate_prediction))


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        SiegertNeuronTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
