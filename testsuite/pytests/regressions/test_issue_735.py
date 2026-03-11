# -*- coding: utf-8 -*-
#
# test_issue_735.py
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


@pytest.mark.parametrize("use_param_on_connect", [True, False])
@pytest.mark.parametrize("copy_model", [True, False])
@pytest.mark.parametrize("param_name", ["A_minus", "A_plus", "Wmax", "Wmin", "b", "tau_c", "tau_n", "tau_plus"])
def test_issue_735(use_param_on_connect, copy_model, param_name):
    """
    Regression test for Issue #735 (GitHub).

    This test ensures that NEST raises an error if the user tries to set stdp parameters
    in ways that are not supported for stdp_dopamine_synapse.
    """

    nest.ResetKernel()

    n = nest.Create("iaf_psc_alpha")
    vt = nest.Create("volume_transmitter")

    if copy_model:
        nest.CopyModel("stdp_dopamine_synapse", "mysyn", {"volume_transmitter": vt})
        syn_model = "mysyn"
    else:
        nest.SetDefaults("stdp_dopamine_synapse", {"volume_transmitter": vt})
        syn_model = "stdp_dopamine_synapse"

    if use_param_on_connect:
        with pytest.raises(nest.NESTErrors.NotImplemented):
            nest.Connect(
                n,
                n,
                conn_spec={"rule": "one_to_one"},
                syn_spec={"synapse_model": syn_model, "weight": 2.0, param_name: 1.0},
            )
    else:
        nest.Connect(n, n, syn_spec={"synapse_model": syn_model, "weight": 2.0})
        conns = nest.GetConnections()
        with pytest.raises(nest.NESTErrors.UnaccessedDictionaryEntry):
            conns.set({param_name: 1.0, "weight": 2.0})
