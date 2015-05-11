# -*- coding: utf-8 -*-
#
# topo_mpi_test.py
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
Topology MPI Test.

Run this script as

    python topo_mpi_test.py convergent|divergent

This test builds a network using distance-dependent mask and weight function
and writes node and connection information to file, either as convergent or
divergent connections.

When run with 1, 2, or 4 MPI processes, identical network structures must result.

Create one subdir per number of MPI processes, then move into each subdir, run there.
Afterwards, diff subdirs. Diff should output nothing.

Hans Ekkehard Plesser, 2010-11-03, 2012-11-23
"""

import nest 
import nest.topology as topo
import os
import sys

assert len(sys.argv) == 2, "Usage: topo_mpi_test.py convergent|divergent"

direction = sys.argv[1]

nest.sli_run('M_ERROR setverbosity')
nest.SetKernelStatus({'total_num_virtual_procs': 4})

l1 = topo.CreateLayer({'rows': 10,
                       'columns': 20,
                       'elements': ['iaf_neuron', 2],
                       'edge_wrap': True})

l2 = topo.CreateLayer({'rows': 10,
                       'columns': 20,
                       'elements': ['iaf_neuron', 2],
                       'edge_wrap': True})

topo.ConnectLayers(l1, l2, {'connection_type': direction,
                            'mask': {'circular': {'radius': 0.4}},
                            'weights': {'linear': {'c': 1., 'a': -5.}}})

topo.DumpLayerNodes(l1+l2, 'topo_mpi_test.lyr_tmp' )
topo.DumpLayerConnections(l1, 'static_synapse', 'topo_mpi_test.cnn_tmp')

# combine all layer and connection files into one sorted file, respectively
nest.sli_run('SyncProcesses') # make sure all are done dumping
if nest.Rank() == 0:
    for filetype in ['cnn', 'lyr']:
        os.system('cat *.{0}_tmp | sort > all_sorted.{0}'.format(filetype))
        os.system('rm *.{0}_tmp'.format(filetype))

# directories for any number of MPI processes should now be diff-able
