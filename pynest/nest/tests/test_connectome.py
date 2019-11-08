# -*- coding: utf-8 -*-
#
# test_connectome.py
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
Tests for the Connectome class
"""

import unittest
import nest

try:
    import pandas
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False


@nest.ll_api.check_stack
class TestConnectome(unittest.TestCase):
    """Connectome tests"""

    def setUp(self):
        nest.ResetKernel()

    def test_basic(self):
        """
        Test simple Connectome.
        """
        nrns = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(nrns, nrns)

        get_conns = nest.GetConnections()

        self.assertTrue(isinstance(get_conns, nest.Connectome))
        self.assertEqual(len(get_conns), 4)

        sources = get_conns.get('source')
        targets = get_conns.get('target')

        self.assertEqual(sources, [1, 1, 2, 2])
        self.assertEqual(targets, [1, 2, 1, 2])

    def test_get_set(self):
        """
        Test get() and set() on Connectome.
        """
        nrns = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(nrns, nrns)

        get_conns = nest.GetConnections()

        get_conns.set(delay=2.0)
        get_conns.set(({'weight': 1.5},
                       {'weight': 2.5},
                       {'weight': 3.5},
                       {'weight': 4.5}))

        delay = get_conns.get('delay')
        weight = get_conns.get('weight')

        self.assertEqual(delay, [2.0, 2.0, 2.0, 2.0])
        self.assertEqual(weight, [1.5, 2.5, 3.5, 4.5])

        get_conns.set({'weight': 6., 'delay': 11.})

        delay = get_conns.get('delay')
        weight = get_conns.get('weight')

        self.assertEqual(delay, [11.0, 11.0, 11.0, 11.0])
        self.assertEqual(weight, [6.0, 6.0, 6.0, 6.0])

        with self.assertRaises(nest.kernel.NESTError):
            get_conns.set(source=2)

        nest.ResetKernel()
        nrns = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(nrns, nrns)

        get_conns = nest.GetConnections()

        get_conns.set(weight=[2.0, 3.0, 4.0, 5.0])
        weight = get_conns.get('weight')
        self.assertEqual(weight, [2.0, 3.0, 4.0, 5.0])

    def test_get(self):
        """
        Test get() on Connectome
        """
        nrns = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(nrns, nrns)
        conns = nest.GetConnections()

        # Key is a string
        target = conns.get('target')
        # Key is a list of strings
        dpw = conns.get(['delay', 'port', 'weight'])
        # Key is None
        all_values = conns.get()

        target_ref = [1, 2, 1, 2]
        dpw_ref = {'delay': [1., 1., 1., 1.],
                   'port': [0, 1, 2, 3],
                   'weight': [1., 1., 1., 1.]}
        all_ref = {'delay': [1.0, 1.0, 1.0, 1.0],
                   'port': [0, 1, 2, 3],
                   'receptor': [0, 0, 0, 0],
                   'sizeof': [32, 32, 32, 32],
                   'source': [1, 1, 2, 2],
                   'synapse_id': [0, 0, 0, 0],
                   'synapse_model': ['static_synapse',
                                     'static_synapse',
                                     'static_synapse',
                                     'static_synapse'],
                   'target': [1, 2, 1, 2],
                   'target_thread': [0, 0, 0, 0],
                   'weight': [1.0, 1.0, 1.0, 1.0]}

        self.assertEqual(target, target_ref)
        self.assertEqual(dpw, dpw_ref)
        self.assertEqual(all_values, all_ref)

        # Now try the same with a single connection
        nest.ResetKernel()

        nrns = nest.Create('iaf_psc_alpha')
        nest.Connect(nrns, nrns)
        conns = nest.GetConnections()

        self.assertEqual(len(conns), 1)

        # Key is a string
        target = conns.get('target')
        # Key is a list of strings
        dpw = conns.get(['delay', 'port', 'weight'])
        # Key is None
        all_values = conns.get()

        target_ref = 1
        dpw_ref = {'delay': 1., 'port': 0, 'weight': 1.}
        all_ref = {'delay': 1.0,
                   'port': 0,
                   'receptor': 0,
                   'sizeof': 32,
                   'source': 1,
                   'synapse_id': 0,
                   'synapse_model': 'static_synapse',
                   'target': 1,
                   'target_thread': 0,
                   'weight': 1.0}

        self.assertEqual(target, target_ref)
        self.assertEqual(dpw, dpw_ref)
        self.assertEqual(all_values, all_ref)

    def test_GetConnectionsOnSubset(self):
        """
        Test GetConnections on sliced GIDCollection
        """

        nrns = nest.Create('iaf_psc_alpha', 10)
        nest.Connect(nrns, nrns)

        get_conns = nest.GetConnections(nrns[3:6], nrns[2:8:2])

        self.assertEqual(len(get_conns), 9)

        sources = get_conns.get('source')
        targets = get_conns.get('target')

        self.assertEqual(sources, [4, 4, 4, 5, 5, 5, 6, 6, 6])
        self.assertEqual(targets.sort(), [3, 5, 7, 3, 5, 7, 3, 5, 7].sort())

        nest.ResetKernel()

        nrns = nest.Create('iaf_psc_alpha', 10)
        nest.Connect(nrns, nrns, {'rule': 'one_to_one'})

        get_conns = nest.GetConnections(nrns[3:6], nrns[2:8:2])

        self.assertEqual(len(get_conns), 1)

        sources = get_conns.get('source')
        targets = get_conns.get('target')

        self.assertEqual(sources, 5)
        self.assertEqual(targets, 5)

        nest.ResetKernel()

        nrns = nest.Create('iaf_psc_alpha', 10)
        nest.Connect(nrns[3:6], nrns[2:8:2], {'rule': 'one_to_one'})

        get_conns = nest.GetConnections(nrns[3:6], nrns[2:8:2])

        self.assertEqual(len(get_conns), 3)

        sources = get_conns.get('source')
        targets = get_conns.get('target')

        self.assertEqual(sources, [4, 5, 6])
        self.assertEqual(targets, [3, 5, 7])

    def test_GetConnectionsSynapse(self):
        """
        Test GetConnections with synapse_model
        """
        nrns = nest.Create('iaf_psc_alpha', 10)
        nest.Connect(nrns[:4], nrns[2:6],
                     syn_spec={'synapse_model': 'stdp_synapse'})
        nest.Connect(nrns[5:7], nrns[8:],
                     conn_spec={'rule': 'one_to_one'},
                     syn_spec={'synapse_model': 'tsodyks_synapse'})
        nest.Connect(nrns[7:], nrns[:5],
                     syn_spec={'synapse_model': 'stdp_triplet_synapse',
                               'weight': 5.})

        get_conn_1 = nest.GetConnections(nrns, nrns,
                                         synapse_model='stdp_synapse')
        get_conn_2 = nest.GetConnections(nrns, nrns,
                                         synapse_model='tsodyks_synapse')
        get_conn_3 = nest.GetConnections(nrns, nrns,
                                         synapse_model='stdp_triplet_synapse')

        sources_1 = get_conn_1.get('source')
        sources_2 = get_conn_2.get('source')
        sources_3 = get_conn_3.get('source')
        targets_1 = get_conn_1.get('target')
        targets_2 = get_conn_2.get('target')
        targets_3 = get_conn_3.get('target')

        self.assertEqual(sources_1,
                         [1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4])
        self.assertEqual(sources_2, [6, 7])
        self.assertEqual(sources_3,
                         [8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10])
        self.assertEqual(targets_1.sort(),
                         [3, 4, 5, 6, 3, 4, 5, 6,
                          3, 4, 5, 6, 3, 4, 5, 6].sort())
        self.assertEqual(targets_2, [9, 10])
        self.assertEqual(targets_3.sort(),
                         [1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5].sort())

        weight = get_conn_3.get('weight')
        self.assertEqual(weight,
                         [5., 5., 5., 5., 5., 5., 5.,
                          5., 5., 5., 5., 5., 5., 5., 5.])

        get_conns = nest.GetConnections()
        self.assertEqual(len(get_conns), 33)

    @unittest.skipIf(not HAVE_PANDAS, 'Pandas package is not available')
    def test_getWithPandasOutput(self):
        """
        Test get on Connectome with pandas output
        """
        nrn = nest.Create('iaf_psc_alpha')
        nest.Connect(nrn, nrn)
        conns = nest.GetConnections()

        conns_val = conns.get(output='pandas')
        pnds_ref = pandas.DataFrame({'delay': 1.,
                                     'port': 0,
                                     'receptor': 0,
                                     'sizeof': 32,
                                     'source': 1,
                                     'synapse_id': 0,
                                     'synapse_model': 'static_synapse',
                                     'target': 1,
                                     'target_thread': 0,
                                     'weight': 1.},
                                    index=(conns.get('source'),))
        self.assertTrue(conns_val.equals(pnds_ref))

        conns_delay = conns.get('delay', output='pandas')
        conns_sizeof = conns.get(['sizeof'], output='pandas')

        self.assertTrue(conns_delay.equals(
            pandas.DataFrame({'delay': 1.}, index=(conns.get('source'),))))
        self.assertTrue(conns_sizeof.equals(
            pandas.DataFrame({'sizeof': 32}, index=(conns.get('source'),))))

        nest.ResetKernel()

        nrns = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(nrns, nrns)
        conns = nest.GetConnections()

        conns_val = conns.get(output='pandas')
        pnds_ref = pandas.DataFrame({'delay': [1., 1., 1., 1.],
                                     'port': [0, 1, 2, 3],
                                     'receptor': [0, 0, 0, 0],
                                     'sizeof': [32, 32, 32, 32],
                                     'source': [1, 1, 2, 2],
                                     'synapse_id': [0, 0, 0, 0],
                                     'synapse_model': ['static_synapse',
                                                       'static_synapse',
                                                       'static_synapse',
                                                       'static_synapse'],
                                     'target': [1, 2, 1, 2],
                                     'target_thread': [0, 0, 0, 0],
                                     'weight': [1., 1., 1., 1.]},
                                    index=conns.get('source'))
        self.assertTrue(conns_val.equals(pnds_ref))

        conns_target = conns.get('target', output='pandas')
        conns_sizeof_port = conns.get(['sizeof', 'port'], output='pandas')

        self.assertTrue(conns_target.equals(
            pandas.DataFrame({'target': [1, 2, 1, 2]},
                             index=conns.get('source'))))
        self.assertTrue(conns_sizeof_port.equals(
            pandas.DataFrame({'sizeof': [32, 32, 32, 32],
                              'port': [0, 1, 2, 3]},
                             index=conns.get('source'))))

    def test_empty(self):
        """
        Test get on empty Connectome and after a ResetKernel
        """
        conns = nest.GetConnections()
        self.assertEqual(len(conns), 0)
        self.assertEqual(conns.get(), ())

        nrns = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(nrns, nrns, 'one_to_one')
        conns = nest.GetConnections()
        self.assertEqual(len(conns), 2)

        nest.ResetKernel()
        self.assertEqual(conns.get(), ())
        conns.set(weight=10.)
        self.assertEqual(conns.get('weight'), ())


def suite():
    suite = unittest.makeSuite(TestConnectome, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
