# -*- coding: utf-8 -*-
#
# mpitest_test_parameters_thread_invariance.py
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
import sys

assert len(sys.argv) == 2, 'script must be called with parameter name as argument'

parameter = getattr(nest.random, sys.argv[1])
# Special case for uniform_int because it must be called with an argument
args = 10 if sys.argv[1] == 'uniform_int' else None

nest.ResetKernel()
nest.SetKernelStatus({'total_num_virtual_procs': 4, 'rng_seed': 1234})

nodes = nest.Create('iaf_psc_alpha', 10)
nodes.V_m = parameter() if args is None else parameter(args)
print(nodes.V_m)
