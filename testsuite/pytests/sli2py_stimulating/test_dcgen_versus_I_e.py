# -*- coding: utf-8 -*-
#
# test_dcgen_versus_I_e.py
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
This testscript checks consistency between currents coming from dc_generator and internal variable I_e;
The approach is to simulate both situations for some time and compare membrane potentials at the end;
Any model which supports current events and setting I_e is checked, other models are skipped.
"""

import nest
import pytest

models = [
    model for model in nest.get("node_models") if "V_m" in nest.GetDefaults(model) and "I_e" in nest.GetDefaults(model)
]


@pytest.mark.parametrize("model", models)
def test_dcgen_vs_I_e(model):
    amp = 123.456
    nest.ResetKernel()

    # Models requiring special parameters
    if model in ["gif_psc_exp", "gif_cond_exp", "gif_psc_exp_multisynapse", "gif_cond_exp_multisynapse"]:
        nest.SetDefaults(model, params={"lambda_0": 0.0})
    elif model == "pp_psc_delta":
        nest.SetDefaults(model, params={"c_2": 0.0})

    # Create two neurons
    n1 = nest.Create(model)
    n2 = nest.Create(model)

    # Connect dc generator to one neuron (n1)
    dc_gen = nest.Create("dc_generator", params={"start": 99.0})
    nest.Connect(dc_gen, n1)
    dc_gen.set(amplitude=amp)

    # Simulate
    nest.Simulate(100.0)

    # Set the "I_e" with a constant current equal to the
    # amplitude of the dc current to another neuron (n2)
    n2.set(I_e=amp)

    # Simulate
    nest.Simulate(300.0)

    # Compare membrane potentials of the two neurons
    v1 = n1.get("V_m")
    v2 = n2.get("V_m")

    assert v1 == pytest.approx(v2)
