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
import sys
import traceback

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


class TestIssue578():

    def test_targets(self):
        nest.ResetKernel()
        nest.set_verbosity('M_ALL')
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
            'structural_plasticity_update_interval': 1000.,
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
        synDictE = {'synapse_model': 'static_synapse',
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


# We can not define the regular suite() and runner() functions here, because
# it will not show up as failed in the testsuite if it fails. This is
# because the test is called from test_mpitests, and the unittest system in
# test_mpitests will only register the failing test if we call this test
# directly.
if HAVE_GSL:
    mpitest = TestIssue578()
    mpitest.test_targets()
else:
    print("Skipping because GSL is not available")
