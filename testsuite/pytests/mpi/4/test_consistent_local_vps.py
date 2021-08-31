# -*- coding: utf-8 -*-
#
# test_consistent_local_vps.py
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
import pytest


def test_consistent_local_vps():
    """
    Test local_vps field of kernel status.
    
    This test ensures that the PyNEST-generated local_vps information
    agrees with the thread-VP mappings in the kernel.
    """
    num_procs = nest.NumProcesses()
    n_vp = 3 * num_procs
    nest.SetKernelStatus({'total_num_virtual_procs': n_vp})

    local_vps = list(nest.GetLocalVPs())

    # Use thread-vp mapping of neurons to check mapping in kernel
    nrns = nest.GetLocalNodeCollection(nest.Create('iaf_psc_delta', 2 * n_vp))

    vp_direct = [n.get('vp') for n in nrns]
    vp_indirect = [local_vps[n.get('thread')] for n in nrns]
    assert vp_direct == vp_indirect
