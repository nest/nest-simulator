# -*- coding: utf-8 -*-
#
# test_multiarea_model_siegert_rates.py
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
import unittest
import numpy as np

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


@nest.ll_api.check_stack
@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
class MultiareaModelSiegertRatesTestCase(unittest.TestCase):

    '''
    Test whether we get the same self-consistent rates as in the
    original multiarea model.
    '''

    def test_multiarea_model_siegert_rates(self):

        # multiarea model parameters
        rate_ext = 10.0
        rate_meanfield = np.load(
            'test_multiarea_model_siegert_rates/mf_rates.npy')
        K = np.load('test_multiarea_model_siegert_rates/K.npy')
        J = np.load('test_multiarea_model_siegert_rates/J.npy')
        n_populations = np.shape(K)[0]

        # siegert neuron parameters
        neuron_params = {'theta': 15.0, 'V_reset': 0.0, 'tau_m': 10.0,
                         'tau_syn': 0.5, 't_ref': 2.0, 'tau': 1.0}
        tau = neuron_params['tau_m'] * 1e-3

        nest.hl_api.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus(
            {'resolution': 0.1, 'use_wfr': False, 'print_time': False})

        # create siegert neurons representing populations
        neurons = nest.Create('siegert_neuron', n_populations,
                              params=neuron_params)

        # create & connect siegert neuron for external drive
        drive = nest.Create('siegert_neuron', 1, params={
            'rate': rate_ext, 'mean': rate_ext, 'theta': neuron_params['theta']
        })
        syn_dict = {
            'drift_factor': tau * np.array([K[:, -1] * J[:, -1]]).T,
            'diffusion_factor': tau * np.array([K[:, -1] * J[:, -1]**2]).T,
            'model': 'diffusion_connection',
            'receptor_type': 0
        }
        nest.Connect(drive, neurons, 'all_to_all', syn_dict)

        # make recurrent connections
        syn_dict = {
            'drift_factor': tau * K[:, :-1] * J[:, :-1],
            'diffusion_factor': tau * K[:, :-1] * J[:, :-1]**2,
            'model': 'diffusion_connection',
            'receptor_type': 0
        }
        nest.Connect(neurons, neurons, 'all_to_all', syn_dict)

        # set initial rates of neurons:
        nest.SetStatus(neurons, {'rate': 0.})

        # create recording device
        multimeter = nest.Create('multimeter', params={
            'record_from': ['rate'], 'interval': 1., 'to_screen': False,
            'to_file': False, 'to_memory': True
        })
        nest.Connect(multimeter, neurons)

        # simulate
        nest.Simulate(50.0)

        # extract rates from multimeter
        data = nest.GetStatus(multimeter)[0]['events']
        r = np.array([
            np.insert(
                data['rate'][np.where(data['senders'] == n)],
                0,
                0.)
            for ii, n in enumerate(neurons)
        ])

        # make sure that we converged to the desired rates
        assert(np.allclose(r[:, -1], rate_meanfield[:, -1]))


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        MultiareaModelSiegertRatesTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
