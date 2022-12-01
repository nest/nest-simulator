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

try:
    from mpi4py import MPI
    have_mpi4py = True
except ImportError:
    have_mpi4py = False

have_mpi = nest.ll_api.sli_func("statusdict/have_mpi ::")
test_with_mpi = have_mpi and have_mpi4py and nest.num_processes > 1


class TestDisconnectSingle(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity('M_ERROR')
        if test_with_mpi:
            self.comm = MPI.COMM_WORLD
            self.rank = self.comm.Get_rank()
            assert(nest.Rank() == self.rank)

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
            'clopath_synapse',
            'clopath_synapse_lbl',
            'clopath_synapse_hpc',
            'urbanczik_synapse',
            'urbanczik_synapse_lbl',
            'urbanczik_synapse_hpc'
        ]

    def test_synapse_deletion_one_to_one_no_sp(self):
        for syn_model in nest.synapse_models:
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.resolution = 0.1
                nest.total_num_virtual_procs = nest.num_processes

                neurons = nest.Create('iaf_psc_alpha', 4)
                syn_dict = {'synapse_model': syn_model}

                nest.Connect(neurons[0], neurons[2], "one_to_one", syn_dict)
                nest.Connect(neurons[1], neurons[3], "one_to_one", syn_dict)

                # Delete existent connection
                conns = nest.GetConnections(neurons[0], neurons[2], syn_model)
                if test_with_mpi:
                    conns = self.comm.allgather(conns.get('source'))
                    conns = list(filter(None, conns))
                assert len(conns) == 1

                nest.Disconnect(neurons[0], neurons[2], syn_spec=syn_dict)

                conns = nest.GetConnections(
                    neurons[0], neurons[2], syn_model)
                if test_with_mpi:
                    conns = self.comm.allgather(conns.get('source'))
                    conns = list(filter(None, conns))
                assert len(conns) == 0

                # Assert that one cannot delete a non-existing connection
                conns1 = nest.GetConnections(
                    neurons[:1], neurons[1:2], syn_model)
                if test_with_mpi:
                    conns1 = self.comm.allgather(conns1.get('source'))
                    conns1 = list(filter(None, conns1))
                assert len(conns1) == 0

                with self.assertRaises(nest.NESTErrors.NESTError):
                    nest.Disconnect(neurons[0], neurons[1], syn_spec=syn_dict)

    def test_disconnect_synapsecollection(self):
        def get_conns(pre, post, syn_model):
            conns = nest.GetConnections(pre, post, syn_model)
            if test_with_mpi:
                conns = self.comm.allgather(conns.get('source'))
                conns = list(filter(None, conns))
            return conns

        neurons = nest.Create('iaf_psc_alpha', 4)
        syn_model = 'static_synapse'
        syn_dict = {'synapse_model': syn_model}

        nest.Connect(neurons[0], neurons[2], "one_to_one", syn_dict)
        nest.Connect(neurons[1], neurons[3], "one_to_one", syn_dict)

        orig_num_conns = 2
        self.assertEqual(nest.GetKernelStatus('num_connections'), orig_num_conns)

        # Delete existent connection with nest.Disconnect()
        conns = get_conns(neurons[0], neurons[2], syn_model)
        self.assertEqual(len(conns), 1)
        nest.Disconnect(conns)

        self.assertEqual(nest.GetKernelStatus('num_connections'), orig_num_conns - 1)
        conns = get_conns(neurons[0], neurons[2], syn_model)
        self.assertEqual(len(conns), 0)

        # Delete existent connection with SynapseCollection.disconnect()
        conns = get_conns(neurons[1], neurons[3], syn_model)
        conns.disconnect()
        self.assertEqual(nest.GetKernelStatus('num_connections'), orig_num_conns - 2)

    def test_disconnect_synapsecollection_raises(self):
        pre, post = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(pre, post, 'one_to_one')
        conns = nest.GetConnections()

        # Wrong type as SynapseCollection
        with self.assertRaises(TypeError):
            nest.Disconnect([1])

        # Passing specifications with SynapseCollection
        with self.assertRaises(ValueError):
            nest.Disconnect(conns, conn_spec='one_to_one')
        with self.assertRaises(ValueError):
            nest.Disconnect(conns, syn_spec='static_synapse')

        # Too many arguments
        with self.assertRaises(TypeError):
            nest.Disconnect(pre, post, conns)


def suite():
    test_suite = unittest.makeSuite(TestDisconnectSingle, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main()
