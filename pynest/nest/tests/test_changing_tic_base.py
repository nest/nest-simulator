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
    def setUp(self):
        nest.ResetKernel()

    def test_models(self):
        """Time objects in models correctly updated"""
        reference = {}
        for model in nest.Models():
            try:
                reference[model] = nest.GetDefaults(model)
            except nest.kernel.NESTError:
                # If we can't get the defaults, we ignore the model.
                pass
        nest.SetKernelStatus({'tics_per_ms': 1024., 'resolution': 0.5})
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

            if model_defaults != model_reference:
                keydiff = [key for key, value in model_defaults.items() if value != model_reference[key]]
                print(model, keydiff)
                failing_models.append(model)
        self.assertEqual([], failing_models)


def suite():
    suite = unittest.makeSuite(TestChangingTicBase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
