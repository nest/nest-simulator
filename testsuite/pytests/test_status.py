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

"""
Test if Set/GetStatus work properly
"""

import unittest
import nest


@nest.ll_api.check_stack
class StatusTestCase(unittest.TestCase):
    """Tests of Get/SetStatus, Get/SetDefaults, and Get/SetKernelStatus via get/set"""

    def test_kernel_attributes(self):
        """Test nest attribute access of kernel attributes"""

        nest.ResetKernel()

        self.assertEqual(nest.GetKernelStatus(), nest.kernel_status)
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

        test_keys = ("resolution", ) * 3
        kernel_status = nest.GetKernelStatus(test_keys)
        self.assertEqual(len(kernel_status), len(test_keys))

        self.assertRaises(TypeError, nest.GetKernelStatus, 42)

    def test_SetKernelStatus(self):
        """SetKernelStatus"""

        nest.ResetKernel()
        nest.SetKernelStatus({})
        nest.SetKernelStatus({'resolution': 0.2})

        self.assertRaises(ValueError, nest.SetKernelStatus, {'nonexistent_status_key': 0})
        # Readonly check
        self.assertRaises(ValueError, nest.SetKernelStatus, {'network_size': 120})

    def test_GetDefaults(self):
        """GetDefaults"""

        nest.ResetKernel()

        for model in nest.node_models + nest.synapse_models:

            model_status = nest.GetDefaults(model)
            self.assertIsInstance(model_status, dict)
            self.assertGreater(len(model_status), 1)

            self.assertRaises(TypeError, nest.GetDefaults, model, 42)

            if "V_m" in model_status:
                test_value = nest.GetDefaults(model, "V_m")
                self.assertIsInstance(test_value, float)

                test_keys = ("V_m", ) * 3
                model_status = nest.GetDefaults(model, test_keys)
                self.assertEqual(len(model_status), len(test_keys))

    def test_SetDefaults(self):
        """SetDefaults"""

        nest.ResetKernel()

        for model in nest.node_models:
            if 'V_m' in nest.GetDefaults(model):
                v_m = nest.GetDefaults(model)['V_m']

                nest.SetDefaults(model, {'V_m': -1.})
                self.assertEqual(nest.GetDefaults(model, 'V_m'), -1.)

                nest.SetDefaults(model, 'V_m', v_m)
                self.assertEqual(nest.GetDefaults(model, 'V_m'), v_m)

                self.assertRaisesRegex(
                    nest.kernel.NESTError, "DictError",
                    nest.SetDefaults, model, 'nonexistent_status_key', 0)

    def test_GetStatus(self):
        """GetStatus"""

        for model in nest.node_models:
            if 'V_m' in nest.GetDefaults(model):
                nest.ResetKernel()

                n = nest.Create(model)

                d = nest.GetStatus(n)
                self.assertIsInstance(d, tuple)
                self.assertIsInstance(d[0], dict)
                self.assertGreater(len(d[0]), 1)

                v1 = nest.GetStatus(n)[0]['V_m']
                v2 = nest.GetStatus(n, 'V_m')[0]
                self.assertEqual(v1, v2)

                n = nest.Create(model, 10)
                d = nest.GetStatus(n, 'V_m')
                self.assertEqual(len(d), len(n))
                self.assertIsInstance(d[0], float)

                test_keys = ("V_m", ) * 3
                d = nest.GetStatus(n, test_keys)
                self.assertEqual(len(d), len(n))
                self.assertEqual(len(d[0]), len(test_keys))

    def test_SetStatus(self):
        """SetStatus with dict"""

        for model in nest.node_models:
            if 'V_m' in nest.GetDefaults(model):
                nest.ResetKernel()
                n = nest.Create(model)
                nest.SetStatus(n, {'V_m': 1.})
                self.assertEqual(nest.GetStatus(n, 'V_m')[0], 1.)

    def test_SetStatusList(self):
        """SetStatus with list"""

        for model in nest.node_models:
            if 'V_m' in nest.GetDefaults(model):
                nest.ResetKernel()
                n = nest.Create(model)
                nest.SetStatus(n, [{'V_m': 2.}])
                self.assertEqual(nest.GetStatus(n, 'V_m')[0], 2.)

    def test_SetStatusParam(self):
        """SetStatus with parameter"""

        for model in nest.node_models:
            if 'V_m' in nest.GetDefaults(model):
                nest.ResetKernel()
                n = nest.Create(model)
                nest.SetStatus(n, 'V_m', 3.)
                self.assertEqual(nest.GetStatus(n, 'V_m')[0], 3.)

    def test_SetStatusVth_E_L(self):
        """SetStatus of reversal and threshold potential """

        excluded = ['a2eif_cond_exp_HW', 'mat2_psc_exp', 'amat2_psc_exp']
        models = nest.node_models + nest.synapse_models

        for model in [m for m in models if m not in excluded]:
            if all(key in nest.GetDefaults(model) for key in ('V_th', 'E_L')):
                nest.ResetKernel()

                neuron1 = nest.Create(model)
                neuron2 = nest.Create(model)

                # must not depend on the order
                new_EL = -90.
                new_Vth = -10.

                if 'V_reset' in nest.GetDefaults(model):
                    nest.SetStatus(neuron1 + neuron2, {'V_reset': new_EL})

                nest.SetStatus(neuron1, {'E_L': new_EL})
                nest.SetStatus(neuron2, {'V_th': new_Vth})
                nest.SetStatus(neuron1, {'V_th': new_Vth})
                nest.SetStatus(neuron2, {'E_L': new_EL})
                vth1, vth2 = nest.GetStatus(neuron1 + neuron2, 'V_th')
                self.assertEqual(vth1, vth2)

    def test_SetStatusV_th_smaller_V_reset(self):
        """SetStatus of reversal and threshold potential
           check if error is raised if V_reset > V_th"""

        for model in nest.node_models + nest.synapse_models:
            if all(key in nest.GetDefaults(model) for key in ('V_th', 'V_reset')):
                nest.ResetKernel()

                neuron = nest.Create(model)

                # should raise exception
                self.assertRaisesRegex(
                    nest.kernel.NESTError, "BadProperty",
                    nest.SetStatus, neuron,
                    {'V_reset': 10., 'V_th': 0.}
                )


def suite():
    suite = unittest.makeSuite(StatusTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
