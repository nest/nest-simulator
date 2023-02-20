# -*- coding: utf-8 -*-
#
# test_astrocyte.py
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

import os
import sys
import unittest

import numpy as np
from scipy.interpolate import interp1d

# from collections import defaultdict

import nest

"""
Comparing the astrocyte model to the reference solution
obrained using the LSODAR solver (see
``doc/htmldoc/model_details/astrocyte_model_implementation.ipynb``).

The reference solution is stored in ``test_astrocyte.dat`` and was
generated using the same dictionary of parameters, the data is then downsampled
to keep one value every 0.01 ms and compare with the NEST simulation.

Details:
  We assess that the difference between the
  recorded variables and the reference is smaller than a given tolerance.
"""

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")
path = os.path.abspath(os.path.dirname(__file__))

# --------------------------------------------------------------------------- #
#  Tolerances to compare LSODAR and NEST implementations
# -------------------------
#

# higher for the potential because of the divergence at spike times
di_tolerances_lsodar = {
    "astrocyte": {"IP3_astro": 1e-4, "Ca_astro": 1e-4, "f_IP3R_astro": 1e-4},
}


# --------------------------------------------------------------------------- #
#  Individual dynamics
# -------------------------
#

models = ["astrocyte"]

num_models = len(models)

# parameters with which the LSODAR reference solution was generated

astrocyte_default = nest.GetDefaults('astrocyte')
astrocyte_param = {
    'IP3_astro': 1.0,
    'Ca_astro': 1.0,
    'f_IP3R_astro': 1.0,
    'Ca_tot_astro': astrocyte_default['Ca_tot_astro'],
    'IP3_0_astro': astrocyte_default['IP3_0_astro'],
    'K_act_astro': astrocyte_default['K_act_astro'],
    'K_inh_astro': astrocyte_default['K_inh_astro'],
    'K_IP3_1_astro': astrocyte_default['K_IP3_1_astro'],
    'K_IP3_2_astro': astrocyte_default['K_IP3_2_astro'],
    'K_SERCA_astro': astrocyte_default['K_SERCA_astro'],
    'r_ER_cyt_astro': astrocyte_default['r_ER_cyt_astro'],
    'r_IP3_astro': astrocyte_default['r_IP3_astro'],
    'r_IP3R_astro': astrocyte_default['r_IP3R_astro'],
    'r_L_astro': astrocyte_default['r_L_astro'],
    'tau_IP3_astro': astrocyte_default['tau_IP3_astro'],
    'v_IP3R_astro': astrocyte_default['v_IP3R_astro'],
    'v_SERCA_astro': astrocyte_default['v_SERCA_astro'],
}


# --------------------------------------------------------------------------- #
#  Test class
# -------------------------
#

class AstrocyteTestCase(unittest.TestCase):
    """
    Check the coherence between reference solution and NEST implementation.
    """

    def setUp(self):
        '''
        Clean up and initialize NEST before each test.
        '''
        nest.ResetKernel()
        nest.resolution = 0.01
        nest.rng_seed = 123456

    def compute_difference(self, multimeters, params, reference, recordables):
        '''
        Compute the relative differences between the values recorded by the
        multimeter and those of the reference (recorded at same times).

        Parameters
        ----------
        multimeters : dict of tuples
            Dictionary containing the model name as key and the node ID of the
            associated multimeter as value.
        params : dict
            Parameters used for the models.
        reference : dict
            Reference arrays (one per entry in `recordables`).
        recordables : list of strings
            List of recordables that will be compared.

        Returns
        -------
        rel_diff : dict of dict of doubles
            Relative differences between recorded data and reference (one dict
            per model, containing one value per entry in `recordables`).
        '''
        rel_diff = {model: {} for model in multimeters.keys()}
        # V_lim = (params["V_th"] + params["V_peak"]) / 2.

        for model, mm in iter(multimeters.items()):
            dmm = nest.GetStatus(mm, "events")[0]
            for record in recordables:
                print(dmm[record])
                # ignore places where a divide by zero would occur
                rds = np.abs(reference[record] - dmm[record])
                nonzero = np.where(~np.isclose(reference[record], 0.))[0]
                if np.any(nonzero):
                    rds = rds[nonzero] / np.abs(reference[record][nonzero])
                # # ignore events around spike times for V if it diverges
                # if record == "V_m" and params["Delta_T"] > 0.:
                #     spiking = (dmm[record] > V_lim)
                #     rds = rds[~spiking]
                rel_diff[model][record] = np.average(rds)
        return rel_diff

    def assert_pass_tolerance(self, rel_diff, di_tol):
        '''
        Test that relative differences are indeed smaller than the tolerance.
        '''
        for model, di_rel_diff in iter(rel_diff.items()):
            for var, diff in iter(di_rel_diff.items()):
                self.assertLess(diff, di_tol[model][var],
                                "{} failed test for {}: {} > {}.".format(
                                    model, var, diff, di_tol[model][var]))

    @unittest.skipIf(not HAVE_GSL, 'GSL is not available')
    def test_closeness_nest_lsodar(self):
        # Compare models to the LSODAR implementation.

        simtime = 100.

        # get lsodar reference
        lsodar = np.loadtxt(os.path.join(path, 'test_astrocyte.dat')).T
        IP3_astro_interp = interp1d(lsodar[0, :], lsodar[1, :])
        Ca_astro_interp = interp1d(lsodar[0, :], lsodar[2, :])
        f_IP3R_astro_interp = interp1d(lsodar[0, :], lsodar[3, :])

        # create the neurons and devices
        cells = {model: nest.Create(model, params=astrocyte_param)
                   for model in models}
        multimeters = {model: nest.Create("multimeter") for model in models}
        # connect them and simulate
        for model, mm in iter(multimeters.items()):
            nest.SetStatus(mm, {"interval": nest.resolution,
                                "record_from": ["IP3_astro", "Ca_astro", "f_IP3R_astro"]})
            nest.Connect(mm, cells[model])
        nest.Simulate(simtime)

        # relative differences: interpolate LSODAR to match NEST times
        mm0 = next(iter(multimeters.values()))
        nest_times = nest.GetStatus(mm0, "events")[0]["times"]
        reference = {
            'IP3_astro': IP3_astro_interp(nest_times),
            'Ca_astro': Ca_astro_interp(nest_times),
            'f_IP3R_astro': f_IP3R_astro_interp(nest_times)}

        rel_diff = self.compute_difference(multimeters, astrocyte_param, reference,
                                           ['IP3_astro', 'Ca_astro', 'f_IP3R_astro'])
        self.assert_pass_tolerance(rel_diff, di_tolerances_lsodar)


# --------------------------------------------------------------------------- #
#  Run the comparisons
# ------------------------
#

@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
def suite():
    return unittest.makeSuite(AstrocyteTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
