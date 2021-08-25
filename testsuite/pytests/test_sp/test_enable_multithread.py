# -*- coding: utf-8 -*-
#
# test_enable_multithread.py
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

import nest
import unittest


__author__ = 'sdiaz'

# Structural plasticity currently does not work with multiple threads.
# An exception should be rised if structural plasticity is enabled
# and multiple threads are set, or if multiple threads are set and
# the enable_structural_plasticity function is called.

HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


@unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
class TestEnableMultithread(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity('M_ERROR')

    def test_enable_multithread(self):

        nest.ResetKernel()
        nest.EnableStructuralPlasticity()
        # Setting multiple threads when structural plasticity is enabled should
        # throw an exception
        with self.assertRaises(nest.kernel.NESTError):
            nest.SetKernelStatus(
                {
                    'local_num_threads': 2
                }
            )

    def test_multithread_enable(self):
        nest.ResetKernel()
        nest.SetKernelStatus(
            {
                'local_num_threads': 2
            }
        )
        # Setting multiple threads when structural plasticity is enabled should
        # throw an exception
        with self.assertRaises(nest.kernel.NESTError):
            nest.EnableStructuralPlasticity()


def suite():
    test_suite = unittest.makeSuite(TestEnableMultithread, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main()
