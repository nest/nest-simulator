# -*- coding: utf-8 -*-
#
# mpitest_issue_578_sp.py
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
This test is called from test_mpitests.py
"""

import nest
import unittest
import sys

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


@unittest.skipIf(not HAVE_GSL, "GSL is not available")
class TestIssue578(unittest.TestCase):

    def test_targets(self):
        nest.ResetKernel()
        nest.hl_api.set_verbosity('M_ALL')
        # Testing with 2 MPI processes
        nest.SetKernelStatus(
            {
                'resolution': 0.1,
                'total_num_virtual_procs': 2
            }
        )
        # Update the SP interval
        nest.EnableStructuralPlasticity()
        nest.SetStructuralPlasticityStatus({
            'structural_plasticity_update_interval':
            100,
        })

        growth_curve = {
            'growth_curve': "gaussian",
            'growth_rate': 0.0001,  # Beta (elements/ms)
            'continuous': False,
            'eta': 0.1,
            'eps': 0.7,
        }
        structural_p_elements_E = {
            'Den_ex': growth_curve,
            'Den_in': growth_curve,
            'Axon_ex': growth_curve
        }
        neuronDict = {'V_m': -60.,
                      't_ref': 5.0, 'V_reset': -60.,
                      'V_th': -50., 'C_m': 200.,
                      'E_L': -60., 'g_L': 10.,
                      'E_ex': 0., 'E_in': -80.,
                      'tau_syn_ex': 5., 'tau_syn_in': 10.,
                      'I_e': 220.}

        nest.SetDefaults("iaf_cond_exp", neuronDict)
        neuronsE = nest.Create('iaf_cond_exp', 1, {
            'synaptic_elements': structural_p_elements_E})

        # synapses
        synDictE = {'model': 'static_synapse',
                    'weight': 3.,
                    'pre_synaptic_element': 'Axon_ex',
                    'post_synaptic_element': 'Den_ex'}

        nest.SetStructuralPlasticityStatus({
            'structural_plasticity_synapses': {
                'synapseEE': synDictE,
            }
        })

        try:
            nest.Simulate(200 * 1000)
        except:
            print(sys.exc_info()[0])
            self.fail("Exception during simulation")


def suite():
    test_suite = unittest.makeSuite(TestIssue578, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main()
