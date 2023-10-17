# -*- coding: utf-8 -*-
#
# test_synapse_collection_set_get_multithreading.py
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


@pytest.mark.skipif_missing_threads
def test_syn_collection_set_get_with_threads():
    """
    Test ``SynapseCollection`` setter and getter in multithreaded context.
    """

    nest.ResetKernel()
    nest.local_num_threads = 2

    source = nest.Create("iaf_psc_alpha")
    target = nest.Create("iaf_psc_alpha")
    nest.Connect(source, target, syn_spec={"synapse_model": "stdp_synapse"})

    conn = nest.GetConnections(source=source, synapse_model="stdp_synapse")
    conn_dict = conn.get()

    assert conn_dict["weight"] == 1.0
    assert conn_dict["synapse_model"] == "stdp_synapse"

    conn.set(weight=2.0)

    assert conn.weight == 2.0
