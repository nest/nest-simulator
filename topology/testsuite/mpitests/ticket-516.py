#! /usr/bin/env python
#
# ticket-516.py
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
Regression test for Ticket 516.

This test must be run with six MPI processes: mpirun -np 6 python ticket-516.sli

NB: If a test fails, the Python will hang, as only some processes throw and error,
    while others enter the simulation loop.

Based on an initial reproducer by Sacha van Albada.

"""

__author__ = "Hans Ekkehard Plesser"

import unittest

import nest
import nest.topology as topo

class Ticket516RegressionCase(unittest.TestCase):
    """Regression test for Ticket 516."""

    req_num_mpi_procs = 6
    master_seed = 1234567   
    sim_time = 200


    def setUp(self):
        """Ensure NEST is run with correct number of MPI procs."""
        assert nest.NumProcesses() == self.req_num_mpi_procs
        nest.set_verbosity('M_ERROR')
        nest.ResetKernel()

    def build_network(self, conn_type):
        """
        Build network with randomized weights.

        conn_type: 'convergent' or 'divergent'

        """

        n_vp = nest.GetKernelStatus()['total_num_virtual_procs']
        nest.SetKernelStatus({'grng_seed' : self.master_seed + n_vp,
                              'rng_seeds' : range(self.master_seed +     n_vp + 1,
                                                  self.master_seed + 2 * n_vp + 1)})

        layer = topo.CreateLayer({'rows': 10,'columns': 10, 'edge_wrap': False,
                                  'elements': 'iaf_psc_delta'})

        topo.ConnectLayers(layer, layer,
                           {'connection_type': conn_type,
                            'mask': {'circular': {'radius': 0.5}},
                            'kernel': 1.0,
                            'weights': {'uniform': {'min': 1.0, 'max': 1.5}},
                            'delays': 1.0})


    def test_ConvergentConn(self):
        """Test for failure in convergent connection."""

        self.build_network('convergent')
        nest.Simulate(200)

    def test_DivergentConn(self):
        """Test for failure in divergent connection."""

        self.build_network('divergent')
        nest.Simulate(200)

def suite():

    suite = unittest.makeSuite(Ticket516RegressionCase)
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

  
