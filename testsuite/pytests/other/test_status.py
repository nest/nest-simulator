# -*- coding: utf-8 -*-
#
# test_status.py
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


class StatusTestCase(unittest.TestCase):
    """Tests of Get/SetDefaults, and Get/SetKernelStatus via get/set"""

    def test_kernel_attributes(self):
        """Test nest attribute access of kernel attributes"""

        nest.ResetKernel()

        # Remove entry containing numpy arrays from status dicts since they do not compare well
        gks_result = nest.GetKernelStatus()
        ks_result = nest.kernel_status
        del gks_result["spike_buffer_resize_log"]
        del ks_result["spike_buffer_resize_log"]
        self.assertEqual(gks_result, ks_result)

        self.assertEqual(nest.GetKernelStatus("resolution"), nest.resolution)

        nest.resolution = 0.4
        self.assertEqual(0.4, nest.resolution)
        self.assertRaises(AttributeError, setattr, nest, "network_size", 120)

    def test_GetKernelStatus(self):
        """GetKernelStatus"""

        nest.ResetKernel()

        kernel_status = nest.GetKernelStatus()
        self.assertIsInstance(kernel_status, dict)
        self.assertGreater(len(kernel_status), 1)

        self.assertRaises(KeyError, nest.GetKernelStatus, "nonexistent_status_key")

        test_keys = ("resolution",) * 3
        kernel_status = nest.GetKernelStatus(test_keys)
        self.assertEqual(len(kernel_status), len(test_keys))

        self.assertRaises(TypeError, nest.GetKernelStatus, 42)

    def test_SetKernelStatus(self):
        """SetKernelStatus"""

        nest.ResetKernel()
        nest.SetKernelStatus({})
        nest.SetKernelStatus({"resolution": 0.2})

        self.assertRaises(ValueError, nest.SetKernelStatus, {"nonexistent_status_key": 0})
        # Readonly check
        self.assertRaises(ValueError, nest.SetKernelStatus, {"network_size": 120})

    def test_GetDefaults(self):
        """GetDefaults"""

        nest.ResetKernel()

        for model in nest.node_models + nest.synapse_models:
            model_status = nest.GetDefaults(model)
            self.assertIsInstance(model_status, dict)
            self.assertGreater(len(model_status), 1)

            self.assertRaises(KeyError, nest.GetDefaults, model, 42)

            if "V_m" in model_status:
                test_value = nest.GetDefaults(model, "V_m")
                self.assertIsInstance(test_value, float)

                test_keys = ("V_m",) * 3
                model_status = nest.GetDefaults(model, test_keys)
                self.assertEqual(len(model_status), len(test_keys))

    def test_SetDefaults(self):
        """SetDefaults"""

        nest.ResetKernel()

        for model in nest.node_models:
            if "V_m" in nest.GetDefaults(model):
                v_m = nest.GetDefaults(model)["V_m"]

                nest.SetDefaults(model, {"V_m": -1.0})
                self.assertEqual(nest.GetDefaults(model, "V_m"), -1.0)

                nest.SetDefaults(model, "V_m", v_m)
                self.assertEqual(nest.GetDefaults(model, "V_m"), v_m)

                self.assertRaisesRegex(
                    nest.NESTError, "Unaccessed", nest.SetDefaults, model, "nonexistent_status_key", 0
                )


def suite():
    suite = unittest.makeSuite(StatusTestCase, "test")
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
