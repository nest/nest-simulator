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
obrained using the Scipy ODEINT solver (see
``doc/htmldoc/model_details/astrocyte_model_implementation.ipynb``).

The reference solution is stored in ``test_astrocyte.dat`` and was
generated using the same dictionary of parameters.

Details:
  We assess that the difference between the
  recorded variables and the reference is smaller than a given tolerance.
"""

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")
path = os.path.abspath(os.path.dirname(__file__))

# --------------------------------------------------------------------------- #
#  Tolerances to compare ODEINT and NEST implementations
# -------------------------
#

di_tolerances_odeint = {
    "astrocyte_lr_1994": {"IP3": 1e-4, "Ca": 1e-4, "h_IP3R": 1e-4},
}


# --------------------------------------------------------------------------- #
#  Individual dynamics
# -------------------------
#

models = ["astrocyte_lr_1994"]

num_models = len(models)

# parameters with which the ODEINT reference solution was generated

astrocyte_default = nest.GetDefaults('astrocyte_lr_1994')
astrocyte_param = {
    # use different values for state variables to produce dynamics
    'IP3': 1.0,
    'Ca': 1.0,
    'h_IP3R': 1.0,
    'Ca_tot': astrocyte_default['Ca_tot'],
    'IP3_0': astrocyte_default['IP3_0'],
    'Kd_act': astrocyte_default['Kd_act'],
    'Kd_inh': astrocyte_default['Kd_inh'],
    'Kd_IP3_1': astrocyte_default['Kd_IP3_1'],
    'Kd_IP3_2': astrocyte_default['Kd_IP3_2'],
    'Km_SERCA': astrocyte_default['Km_SERCA'],
    'ratio_ER_cyt': astrocyte_default['ratio_ER_cyt'],
    'incr_IP3': astrocyte_default['incr_IP3'],
    'k_IP3R': astrocyte_default['k_IP3R'],
    'rate_L': astrocyte_default['rate_L'],
    'tau_IP3': astrocyte_default['tau_IP3'],
    'rate_IP3R': astrocyte_default['rate_IP3R'],
    'rate_SERCA': astrocyte_default['rate_SERCA'],
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

        for model, mm in iter(multimeters.items()):
            dmm = nest.GetStatus(mm, "events")[0]
            for record in recordables:
                rds = np.abs(reference[record] - dmm[record])
                nonzero = np.where(~np.isclose(reference[record], 0.))[0]
                if np.any(nonzero):
                    rds = rds[nonzero] / np.abs(reference[record][nonzero])
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
    def test_closeness_nest_odeint(self):
        # Compare models to the ODEINT implementation.

        simtime = 100.

        # get ODEINT reference
        odeint = np.loadtxt(os.path.join(path, 'test_astrocyte.dat')).T
        IP3_astro_interp = interp1d(odeint[0, :], odeint[1, :])
        Ca_astro_interp = interp1d(odeint[0, :], odeint[2, :])
        h_IP3R_astro_interp = interp1d(odeint[0, :], odeint[3, :])

        # create the neurons and devices
        cells = {model: nest.Create(model, params=astrocyte_param)
                   for model in models}
        multimeters = {model: nest.Create("multimeter") for model in models}
        # connect them and simulate
        for model, mm in iter(multimeters.items()):
            nest.SetStatus(mm, {"interval": nest.resolution,
                                "record_from": ["IP3", "Ca", "h_IP3R"]})
            nest.Connect(mm, cells[model])
        nest.Simulate(simtime)

        # relative differences: interpolate ODEINT to match NEST times
        mm0 = next(iter(multimeters.values()))
        nest_times = nest.GetStatus(mm0, "events")[0]["times"]
        reference = {
            'IP3': IP3_astro_interp(nest_times),
            'Ca': Ca_astro_interp(nest_times),
            'h_IP3R': h_IP3R_astro_interp(nest_times)}

        rel_diff = self.compute_difference(multimeters, astrocyte_param, reference,
                                           ['IP3', 'Ca', 'h_IP3R'])
        self.assert_pass_tolerance(rel_diff, di_tolerances_odeint)


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
