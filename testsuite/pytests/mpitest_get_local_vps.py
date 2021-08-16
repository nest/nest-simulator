# -*- coding: utf-8 -*-
#
# mpitest_get_local_vps.py
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


class LocalVPsTestCase():
    """
    Test local_vps field of kernel status.

    This test ensures that the PyNEST-generated local_vps information
    agrees with the thread-VP mappings in the kernel.
    """

    def test_local_vps(self):
        num_procs = nest.NumProcesses()
        n_vp = 3 * num_procs
        nest.SetKernelStatus({'total_num_virtual_procs': n_vp})

        local_vps = list(nest.GetLocalVPs())

        # Use thread-vp mapping of neurons to check mapping in kernel
        nrns = nest.GetLocalNodeCollection(nest.Create('iaf_psc_delta', 2 * n_vp))

        for n in nrns:
            thrd = n.get('thread')
            vp = n.get('vp')
            assert vp == local_vps[thrd]


# We can not define the regular suite() and runner() functions here, because
# it will not show up as failed in the testsuite if it fails. This is
# because the test is called from test_mpitests, and the unittest system in
# test_mpitests will only register the failing test if we call this test
# directly.
mpitest = LocalVPsTestCase()
mpitest.test_local_vps()
