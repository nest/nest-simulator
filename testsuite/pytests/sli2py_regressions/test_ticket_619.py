# -*- coding: utf-8 -*-
#
# test_ticket_619.py
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

"""
Regression test for Ticket #619.

Test ported from SLI regression test.
Ensure SetKernelStatus accepts zero biological time alongside other kernel parameters.

Author: Hans Ekkehard Plesser, 2012-11-29
"""


def test_ticket_619_set_kernel_status_with_zero_biological_time():
    """
    Ensure SetKernelStatus with biological_time 0.0 and rng_seed 1 succeeds.
    """

    nest.ResetKernel()
    nest.SetKernelStatus({"biological_time": 0.0, "rng_seed": 1})
