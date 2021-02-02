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

try:
    from mpi4py import MPI
    HAVE_MPI4PY = True
except ImportError:
    HAVE_MPI4PY = False

HAVE_MPI = nest.ll_api.sli_func("statusdict/have_mpi ::")
MULTIPLE_PROCESSES = nest.NumProcesses() > 1


@nest.ll_api.check_stack
class StatusTestCase(unittest.TestCase):
    """Tests of Set/GetStatus"""

    def test_GetKernelStatus(self):
        """GetKernelStatus"""

        nest.ResetKernel()

        kernel_status = nest.GetKernelStatus()
        self.assertIsInstance(kernel_status, dict)
        self.assertGreater(len(kernel_status), 1)

        self.assertRaises(KeyError, nest.GetKernelStatus,
                          "nonexistent_status_key")

        test_keys = ("resolution", ) * 3
        kernel_status = nest.GetKernelStatus(test_keys)
        self.assertEqual(len(kernel_status), len(test_keys))

        self.assertRaises(TypeError, nest.GetKernelStatus, 42)

    def test_SetKernelStatus(self):
        """SetKernelStatus"""

        nest.ResetKernel()
        nest.SetKernelStatus({})
        nest.SetKernelStatus({'resolution': 0.2})

        self.assertRaisesRegex(
            nest.kernel.NESTError, "DictError",
            nest.SetKernelStatus, {'nonexistent_status_key': 0})

    def test_GetDefaults(self):
        """GetDefaults"""

        nest.ResetKernel()

        for model in nest.Models():

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

        for m in nest.Models():
            if 'V_m' in nest.GetDefaults(m):
                v_m = nest.GetDefaults(m)['V_m']

                nest.SetDefaults(m, {'V_m': -1.})
                self.assertEqual(nest.GetDefaults(m, 'V_m'), -1.)

                nest.SetDefaults(m, 'V_m', v_m)
                self.assertEqual(nest.GetDefaults(m, 'V_m'), v_m)

                self.assertRaisesRegex(
                    nest.kernel.NESTError, "DictError",
                    nest.SetDefaults, m, 'nonexistent_status_key', 0)

    def test_GetStatus(self):
        """GetStatus"""

        for m in nest.Models():
            if 'V_m' in nest.GetDefaults(m):
                nest.ResetKernel()

                n = nest.Create(m)

                d = nest.GetStatus(n)
                self.assertIsInstance(d, tuple)
                self.assertIsInstance(d[0], dict)
                self.assertGreater(len(d[0]), 1)

                v1 = nest.GetStatus(n)[0]['V_m']
                v2 = nest.GetStatus(n, 'V_m')[0]
                self.assertEqual(v1, v2)

                n = nest.Create(m, 10)
                d = nest.GetStatus(n, 'V_m')
                self.assertEqual(len(d), len(n))
                self.assertIsInstance(d[0], float)

                test_keys = ("V_m", ) * 3
                d = nest.GetStatus(n, test_keys)
                self.assertEqual(len(d), len(n))
                self.assertEqual(len(d[0]), len(test_keys))

    def test_SetStatus(self):
        """SetStatus with dict"""

        for m in nest.Models():
            if 'V_m' in nest.GetDefaults(m):
                nest.ResetKernel()
                n = nest.Create(m)
                nest.SetStatus(n, {'V_m': 1.})
                self.assertEqual(nest.GetStatus(n, 'V_m')[0], 1.)

    def test_SetStatusList(self):
        """SetStatus with list"""

        for m in nest.Models():
            if 'V_m' in nest.GetDefaults(m):
                nest.ResetKernel()
                n = nest.Create(m)
                nest.SetStatus(n, [{'V_m': 2.}])
                self.assertEqual(nest.GetStatus(n, 'V_m')[0], 2.)

    def test_SetStatusParam(self):
        """SetStatus with parameter"""

        for m in nest.Models():
            if 'V_m' in nest.GetDefaults(m):
                nest.ResetKernel()
                n = nest.Create(m)
                nest.SetStatus(n, 'V_m', 3.)
                self.assertEqual(nest.GetStatus(n, 'V_m')[0], 3.)

    def test_SetStatusVth_E_L(self):
        """SetStatus of reversal and threshold potential """

        excluded = ['a2eif_cond_exp_HW', 'mat2_psc_exp', 'amat2_psc_exp']
        models = [m for m in nest.Models() if m not in excluded]

        for m in models:
            if all(key in nest.GetDefaults(m) for key in ('V_th', 'E_L')):
                nest.ResetKernel()

                neuron1 = nest.Create(m)
                neuron2 = nest.Create(m)

                # must not depend on the order
                new_EL = -90.
                new_Vth = -10.

                if 'V_reset' in nest.GetDefaults(m):
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

        for m in nest.Models():
            if all(key in nest.GetDefaults(m) for key in ('V_th', 'V_reset')):
                nest.ResetKernel()

                neuron = nest.Create(m)

                # should raise exception
                self.assertRaisesRegex(
                    nest.kernel.NESTError, "BadProperty",
                    nest.SetStatus, neuron,
                    {'V_reset': 10., 'V_th': 0.}
                )

@unittest.skipIf(not HAVE_MPI4PY, 'mpi4py is not available')
@nest.ll_api.check_stack
class LocalVPsTestCase(unittest.TestCase):
    """
    Test local_vps field of kernel status.
    
    This test ensure that the PyNEST-generated local_vps information
    agrees with the thread-VP mappings in the kernel.
    """
    
    # The test class is instantiated by the unittest framework regardless of the value of
    # HAVE_MPI4PY, even though all tests will be skipped in case it is False. In this
    # situation, we have to manually prevent calls to MPI in order to avoid errors during
    # the execution.
    if HAVE_MPI4PY:
        comm = MPI.COMM_WORLD.Clone()

    # With pytest or nosetests, only run these tests if using multiple processes
    __test__ = MULTIPLE_PROCESSES
    
    def setUp(self):
        nest.ResetKernel()
        
    def test_local_vps(self):
        local_vps = list(nest.GetKernelStatus('local_vps'))

        # Use thread-vp mapping of neurons to check mapping in kernel
        nrns = nest.GetLocalNodeCollection(nest.Create('iaf_psc_delta'), 
                                           2 * nest.NumProcesses())
        
        for n in nrns:
            thrd, vp = n.get(('thread', 'vp'))
            self.assertEqual(vp, local_vps[thrd])
        
def suite():
    suite = unittest.makeSuite(StatusTestCase, 'test')
    suite.addTest(LocalVPsTestCase)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
