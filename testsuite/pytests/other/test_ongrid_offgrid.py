# -*- coding: utf-8 -*-
#
# test_ongrid_offgrid.py
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


class TestOngridOffgrid:
    """
    Test that plain and precise neurons can exchange spikes also with multiplicity.

    This test arose from a regression where only ongrid or only offgrid spikes were
    transmitted when at least two spikes of each size were sent.
    """

    @pytest.fixture(autouse=True)
    def build_base(self):
        nest.ResetKernel()
        self.g = nest.Create("spike_generator", params={"spike_times": [1.0, 1.0]})
        self.pre = {"plain": nest.Create("parrot_neuron"), "precise": nest.Create("parrot_neuron_ps")}
        self.post = {"plain": nest.Create("parrot_neuron"), "precise": nest.Create("parrot_neuron_ps")}
        for nrn in self.pre.values():
            nest.Connect(self.g, nrn)

    @pytest.mark.parametrize(
        "pre_type, post_type", [["plain", "plain"], ["plain", "precise"], ["precise", "plain"], ["precise", "precise"]]
    )
    def test_ongrid_offgrid(self, pre_type, post_type):
        """
        Connect pre-type to post-type neuron and check post-type neuron receives spike.
        """

        pre = self.pre[pre_type]
        post = self.post[post_type]
        nest.Connect(pre, post)

        nest.Simulate(5)

        # confirm postsynaptic neuron has received and thus emitted a spike
        assert post.t_spike > 0

    def test_ongrid_offgrid_both(self):
        """
        Connect plain-plain and precise-precise and check both streams.
        """

        for ntype in self.pre:
            nest.Connect(self.pre[ntype], self.post[ntype], syn_spec={"synapse_model": "stdp_synapse"})

        nest.Simulate(5)

        # confirm postsynaptic neuron has received and thus emitted a spike
        assert self.post["plain"].t_spike > 0 and self.post["precise"].t_spike > 0
