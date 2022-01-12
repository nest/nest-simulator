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

    def _assert_ticbase_change_raises_and_reset(self, after_call):
        """Assert that changing tic-base raises a NESTError, and reset the kernel"""
        with self.assertRaises(nest.kernel.NESTError, msg=f'after calling "{after_call}"'):
            # For co-dependent properties, we use `set()` instead of kernel attributes
            nest.set(resolution=0.5, tics_per_ms=1500.0)
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
