# -*- coding: utf-8 -*-
#
# test_getconnections.py
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
GetConnections
"""

import unittest
import nest

nest.set_verbosity('M_ERROR')


@nest.ll_api.check_stack
class GetConnectionsTestCase(unittest.TestCase):
    """Find connections and test if values can be set."""

    def test_GetConnections(self):
        """GetConnections"""

        nest.ResetKernel()

        a = nest.Create("iaf_psc_alpha", 3)
        nest.Connect(a, a)
        c1 = nest.GetConnections(a)
        c2 = nest.GetConnections(a, synapse_model="static_synapse")
        self.assertEqual(c1, c2)

        weights = (2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0)
        d1 = tuple({"weight": w} for w in weights)

        c3 = nest.GetConnections(a, a)
        nest.SetStatus(c3, d1)
        s1 = nest.GetStatus(c3, "weight")
        self.assertEqual(s1, weights)

        c4 = nest.GetConnections()
        self.assertEqual(c1, c4)

        weights = (11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0)
        d1 = tuple({"weight": w} for w in weights)

        c5 = nest.GetConnections(a, a)
        c5.set(d1)
        s2 = c5.get('weight')
        self.assertEqual(s2, list(weights))

        c6 = nest.GetConnections()
        self.assertEqual(c1, c6)

    def test_GetConnectionsTargetModels(self):
        """GetConnections iterating models for target"""
        for model in nest.Models():
            nest.ResetKernel()
            alpha = nest.Create('iaf_psc_alpha')
            try:
                other = nest.Create(model)
                nest.Connect(alpha, other)
            except nest.kernel.NESTError:
                # If we can't create a node with this model, or connect
                # to a node of this model, we ignore it.
                continue
            for get_conn_args in [{'source': alpha, 'target': other},
                                  {'source': alpha},
                                  {'target': other}]:
                conns = nest.GetConnections(**get_conn_args)
                self.assertEqual(
                    len(conns), 1,
                    'Failed to get connection with target model {} (specifying {})'.format(
                        model, ', '.join(get_conn_args.keys())))

    def test_GetConnectionsSourceModels(self):
        """GetConnections iterating models for source"""
        for model in nest.Models():
            nest.ResetKernel()
            alpha = nest.Create('iaf_psc_alpha')
            try:
                other = nest.Create(model)
                nest.Connect(other, alpha)
            except nest.kernel.NESTError:
                # If we can't create a node with this model, or connect
                # to a node of this model, we ignore it.
                continue
            for get_conn_args in [{'source': other, 'target': alpha},
                                  {'source': other},
                                  {'target': alpha}]:
                conns = nest.GetConnections(**get_conn_args)
                self.assertEqual(
                    len(conns), 1,
                    'Failed to get connection with source model {} (specifying {})'.format(
                        model, ', '.join(get_conn_args.keys())))

    def test_GetConnectionsSynapseModel(self):
        """GetConnections using synapse_model as argument"""

        num_src = 3
        num_tgt = 5

        for synapse_model in nest.Models('synapses'):
            nest.ResetKernel()

            src = nest.Create('iaf_psc_alpha', num_src)
            tgt = nest.Create('iaf_psc_alpha', num_tgt)

            # First create one connection with static_synapse
            nest.Connect(src[0], tgt[0])

            try:
                # Connect with specified synapse
                nest.Connect(src, tgt, syn_spec={'synapse_model': synapse_model})
            except nest.kernel.NESTError:
                # If we can't connect iaf_psc_alpha with the given synapse_model, we ignore it.
                continue

            reference_list = [synapse_model] * num_src * num_tgt
            if synapse_model == 'static_synapse':
                reference_list += ['static_synapse']

            conns = nest.GetConnections(synapse_model=synapse_model)
            self.assertEqual(reference_list, conns.synapse_model)

            # Also test that it works if we specify source/target and synapse_model
            conns = nest.GetConnections(source=src, target=tgt, synapse_model=synapse_model)
            self.assertEqual(reference_list, conns.synapse_model)

            conns = nest.GetConnections(source=src, synapse_model=synapse_model)
            self.assertEqual(reference_list, conns.synapse_model)

            conns = nest.GetConnections(target=tgt, synapse_model=synapse_model)
            self.assertEqual(reference_list, conns.synapse_model)

    def test_GetConnectionsSynapseLabel(self):
        """GetConnections using synapse_label as argument"""

        labeled_synapse_models = [s for s in nest.Models(mtype='synapses') if s.endswith("_lbl")]

        label = 123
        num_src = 3
        num_tgt = 5

        for synapse_model in labeled_synapse_models:
            nest.ResetKernel()

            src = nest.Create('iaf_psc_alpha', num_src)
            tgt = nest.Create('iaf_psc_alpha', num_tgt)

            # First create one connection with static_synapse
            nest.Connect(src[0], tgt[0])

            try:
                # Connect with specified synapse
                nest.Connect(src, tgt, syn_spec={'synapse_model': synapse_model, "synapse_label": label})
            except nest.kernel.NESTError:
                # If we can't connect iaf_psc_alpha with the given synapse_model, we ignore it.
                continue

            reference_list = [synapse_model] * num_src * num_tgt
            label_list = [label] * num_src * num_tgt

            # Call GetConnections with specified synapse_label and test that connections with
            # corresponding model are returned
            conns = nest.GetConnections(synapse_label=label)

            self.assertEqual(reference_list, conns.synapse_model)
            self.assertEqual(label_list, conns.synapse_label)

            # Also test that it works if we specify source/target and synapse_label
            conns = nest.GetConnections(source=src, target=tgt, synapse_label=label)
            self.assertEqual(reference_list, conns.synapse_model)

            conns = nest.GetConnections(source=src, synapse_label=label)
            self.assertEqual(reference_list, conns.synapse_model)

            conns = nest.GetConnections(target=tgt, synapse_label=label)
            self.assertEqual(reference_list, conns.synapse_model)


def suite():

    suite = unittest.makeSuite(GetConnectionsTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
