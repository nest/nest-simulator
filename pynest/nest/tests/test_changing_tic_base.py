# -*- coding: utf-8 -*-
#
# test_changing_tic_base.py
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

import unittest
import nest
import numpy as np


@nest.ll_api.check_stack
class TestChangingTicBase(unittest.TestCase):
    eps = 1e-7  # Tolerance value
    # The defaults of iaf_psc_exp_ps_lossless contains the time of the last spike, converted to ms from step=-1.
    # As the initialized step-value is negative, there is no need to account for the change in tic-base.
    # However, because the value in the defaults is converted to ms, it will differ from the reference value.
    # The model is therefore ignored.
    ignored_models = ['iaf_psc_exp_ps_lossless']

    def setUp(self):
        nest.ResetKernel()

    def test_models(self):
        """Time objects in models correctly updated"""
        # Generate a dictionary of reference values for each model.
        reference = {}
        for model in nest.Models():
            if model in self.ignored_models:
                continue
            try:
                reference[model] = nest.GetDefaults(model)
            except nest.kernel.NESTError:
                # If we can't get the defaults, we ignore the model.
                pass

        # Change the tic-base.
        nest.SetKernelStatus({'tics_per_ms': 1500., 'resolution': 0.5})
        # At this point, Time objects in models should have been updated to
        # account for the new tic-base. Values in model defaults should therefore
        # be equal (within a tolerance) to the reference values.

        failing_models = []
        for model in reference.keys():
            model_reference = reference[model]
            model_defaults = nest.GetDefaults(model)
            # Remove entries where the item contains more than one value, as this causes issues when comparing.
            array_keys = [key for key, value in model_defaults.items()
                          if isinstance(value, (list, tuple, dict, np.ndarray))]
            for key in array_keys:
                del model_defaults[key]
                del model_reference[key]

            keydiff = []
            for key, value in model_defaults.items():
                # value may not be a number, so we test for equality first.
                # If it's not equal to the reference value, we assume it is a number.
                if value != model_reference[key] and abs(value - model_reference[key]) > self.eps:
                    print(value - model_reference[key])
                    keydiff.append([key, model_reference[key], value])
            # If any keys have values different from the reference, the model fails.
            if len(keydiff) > 0:
                print(model, keydiff)
                failing_models.append(model)

        # No models should fail for the test to pass.
        self.assertEqual([], failing_models)

    def _assert_ticbase_change_raises_and_reset(self, after_call):
        """Assert that changing tic-base raises a NESTError, and reset the kernel"""
        with self.assertRaises(nest.kernel.NESTError, msg='after calling "{}"'.format(after_call)):
            nest.SetKernelStatus({'tics_per_ms': 1500., 'resolution': 0.5})
        nest.ResetKernel()

    def test_prohibit_change_tic_base(self):
        """Getting error when changing tic-base in prohibited conditions"""

        nest.CopyModel('iaf_psc_alpha', 'alpha_copy')
        self._assert_ticbase_change_raises_and_reset('CopyModel')

        nest.SetDefaults("multimeter", {"record_to": "ascii"})
        self._assert_ticbase_change_raises_and_reset('SetDefaults')

        nest.Create('multimeter')
        self._assert_ticbase_change_raises_and_reset('Create')

        nest.Simulate(10.)
        self._assert_ticbase_change_raises_and_reset('Simulate')


def suite():
    suite = unittest.makeSuite(TestChangingTicBase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
