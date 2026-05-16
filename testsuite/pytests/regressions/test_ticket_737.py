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

"""
Regression test for Ticket #737.

Test ported from SLI regression test
Ensure that stimulation devices can only be connected with a single synapse type.
"""


# Get all viable stimulator models
SKIP_LIST = ["step_rate_generator"]
stimulators = [
    m for m in nest.node_models if m not in SKIP_LIST and nest.GetDefaults(m).get("element_type") == "stimulator"
]


@pytest.fixture(params=stimulators)
def stimulator_and_neuron(request):
    stim_model = request.param

    nest.ResetKernel()
    stim = nest.Create(stim_model)
    n = nest.Create("iaf_psc_alpha")

    yield stim, n


def test_ticket_737_all_static(stimulator_and_neuron):
    """Confirm we can connect multiple times with static_synapse."""

    stim, n = stimulator_and_neuron

    nest.Connect(stim, n, "all_to_all", syn_spec="static_synapse")
    nest.Connect(stim, n, "all_to_all", syn_spec="static_synapse")


def test_ticket_737_user_defined(stimulator_and_neuron):
    """Confirm we can connect multiple times with user-defined synapse model."""

    stim, n = stimulator_and_neuron

    synmodel = f"{stim.model}_syn"
    nest.CopyModel("static_synapse", synmodel)
    nest.Connect(stim, n, "all_to_all", syn_spec=synmodel)
    nest.Connect(stim, n, "all_to_all", syn_spec=synmodel)


def test_ticket_737_break_on_different_types(stimulator_and_neuron):
    """Confirm NEST raises error if trying to connect one generator with different synpase models."""

    stim, n = stimulator_and_neuron

    synmodel = f"{stim.model}_syn"
    nest.CopyModel("static_synapse", synmodel)
    nest.Connect(stim, n, "all_to_all", syn_spec="static_synapse")

    with pytest.raises(nest.NESTErrors.IllegalConnection):
        nest.Connect(stim, n, "all_to_all", syn_spec=synmodel)
