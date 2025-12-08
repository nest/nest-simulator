# -*- coding: utf-8 -*-
#
# test_issue_737_threads.py
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
Regression test for Issue #737 (threads variant).

Test ported from SLI regression test.
Ensure stdp_dopamine_synapse rejects multithreaded Connect calls that set c or n in syn_spec.

Author: Stine B. Vennemo, May 2017
"""


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("parameter_name", ("c", "n"))
@pytest.mark.parametrize("use_copy_model", (False, True))
def test_issue_737_threads_rejects_multithreaded_parameters(parameter_name, use_copy_model):
    """
    Ensure setting c or n in syn_spec raises an error when using multiple threads.
    """

    nest.ResetKernel()
    nest.local_num_threads = 4

    neuron = nest.Create("iaf_psc_alpha")
    vt = nest.Create("volume_transmitter")

    if use_copy_model:
        syn_model = "mysyn"
        nest.CopyModel("stdp_dopamine_synapse", syn_model, {"volume_transmitter": vt})
    else:
        syn_model = "stdp_dopamine_synapse"
        nest.SetDefaults(syn_model, {"volume_transmitter": vt})

    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(
            neuron,
            neuron,
            conn_spec={"rule": "one_to_one"},
            syn_spec={"synapse_model": syn_model, parameter_name: 2.0},
        )
