# -*- coding: utf-8 -*-
#
# test_ticket_737.py
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

SKIP_LIST = ["step_rate_generator"]

# Get all stimulator models
stimulators = [
    m for m in nest.node_models if m not in SKIP_LIST and nest.GetDefaults(m).get("element_type") == "stimulator"
]


@pytest.mark.parametrize("stim_model", stimulators)
def test_multiple_static_synapse_connections(stim_model):
    """
    Regression test for Ticket #737.

    Ensure that stimulation devices can only be connected with a single synapse type.
    """
    # First test: multiple connections with same type (static_synapse)
    nest.ResetKernel()
    stim = nest.Create(stim_model)
    n = nest.Create("iaf_psc_alpha")
    nest.Connect(stim, n, "all_to_all", syn_spec="static_synapse")
    nest.Connect(stim, n, "all_to_all", syn_spec="static_synapse")

    # Second test: multiple connections of user-defined type
    nest.ResetKernel()
    stim = nest.Create(stim_model)
    n = nest.Create("iaf_psc_alpha")
    synmodel = f"{stim_model}_syn"
    nest.CopyModel("static_synapse", synmodel)
    nest.Connect(stim, n, "all_to_all", syn_spec=synmodel)
    nest.Connect(stim, n, "all_to_all", syn_spec=synmodel)

    # Third test: no multiple connections with different types
    nest.ResetKernel()
    stim = nest.Create(stim_model)
    n = nest.Create("iaf_psc_alpha")
    synmodel = f"{stim_model}_syn"
    nest.CopyModel("static_synapse", synmodel)
    nest.Connect(stim, n, "all_to_all", syn_spec="static_synapse")
    with pytest.raises(nest.NESTError):
        nest.Connect(stim, n, "all_to_all", syn_spec=synmodel)
