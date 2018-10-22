# -*- coding: utf-8 -*-
#
# test_disconnect.py
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
import numpy as np

__author__ = 'naveau'

try:
    from mpi4py import MPI
except ImportError:
    # Test without MPI
    mpi_test = 0
else:
    # Test with MPI
    mpi_test = 1
mpi_test = nest.sli_func("statusdict/have_mpi ::") & mpi_test


class TestDisconnectSingle(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity('M_ERROR')
        self.num_procs = 1
        if mpi_test:
            self.comm = MPI.COMM_WORLD
            self.rank = self.comm.Get_rank()
            assert(nest.Rank() == self.rank)
            self.num_procs = 2
        self.exclude_synapse_model = [
            'stdp_dopamine_synapse',
            'stdp_dopamine_synapse_lbl',
            'stdp_dopamine_synapse_hpc',
            'stdp_dopamine_synapse_hpc_lbl',
            'rate_connection_instantaneous',
            'rate_connection_instantaneous_lbl',
            'rate_connection_delayed',
            'rate_connection_delayed_lbl',
            'gap_junction',
            'gap_junction_lbl',
            'diffusion_connection',
            'diffusion_connection_lbl',
        ]

    def test_synapse_deletion_one_to_one_no_sp(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                print(syn_model)
                nest.SetKernelStatus(
                    {
                        'resolution': 0.1,
                        'total_num_virtual_procs': self.num_procs
                    }
                )
                neurons = nest.Create('iaf_psc_alpha', 4)
                syn_dict = {'model': syn_model}

                nest.Connect([neurons[0]], [neurons[2]],
                             "one_to_one", syn_dict)
                nest.Connect([neurons[1]], [neurons[3]],
                             "one_to_one", syn_dict)
                # Delete existent connection
                conns = nest.GetConnections(
                    [neurons[0]], [neurons[2]], syn_model)
                if mpi_test:
                    conns = self.comm.allgather(conns)
                    conns = filter(None, conns)
                assert len(list(conns)) == 1

                nest.DisconnectOneToOne(neurons[0], neurons[2], syn_dict)
                conns = nest.GetConnections(
                    [neurons[0]], [neurons[2]], syn_model)
                if mpi_test:
                    conns = self.comm.allgather(conns)
                    conns = filter(None, conns)

                assert len(list(conns)) == 0

                # Assert that one can not delete a non existent connection
                conns1 = nest.GetConnections(
                    [neurons[0]], [neurons[1]], syn_model)
                if mpi_test:
                    conns1 = self.comm.allgather(conns1)
                    conns1 = filter(None, conns1)
                assert len(list(conns1)) == 0

                try:
                    nest.DisconnectOneToOne(neurons[0], neurons[1], syn_dict)
                    assert False
                except nest.NESTError:
                    print("Synapse deletion ok: " + syn_model)

    def test_simple(self):
        nodes = nest.Create('iaf_psc_alpha', 5)
        nest.Connect(nodes, nodes, 'one_to_one')

        nest.DisconnectOneToOne(nodes[0], nodes[0], 'static_synapse')

        self.assertEqual(nest.GetKernelStatus('num_connections'), 4)


def suite():
    test_suite = unittest.makeSuite(TestDisconnectSingle, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main()
