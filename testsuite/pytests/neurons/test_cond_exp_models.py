# -*- coding: utf-8 -*-
#
# test_cond_exp_models.py
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

skip_list = ["pp_cond_exp_mc_urbanczik"]  # cannot read V_m directly

# List of models to be checked
models = [
    node for node in nest.node_models if node not in skip_list and "cond_exp" in node and "multisynapse" not in node
]


@pytest.mark.parametrize("model", models)
class TestCondExpModels:
    """
    Test for cond_exp models
    """

    # Some models don't have a known resting potential, and thus get a drift in
    # some tests. In these cases, instead of checking that the potential is
    # unchanged, we check that the drift is not too large.
    inaccurate_rest_pot_diff_limit = {"hh_cond_exp_traub": 6.0e-2, "aeif_cond_exp": 5.4e-7}

    SIM_TIME = 5.0

    @pytest.fixture
    def reference_vm(self, model):
        """
        Get the reference v_m of the given model.
        """
        n = nest.Create(model)
        nest.Simulate(self.SIM_TIME)
        vm_ref = n.get("V_m")
        return vm_ref

    @pytest.fixture
    def get_vm(self, model, request):
        """
        Get the membrane potential value for the neuron model
        """
        # Create the neuron model and set the excitatory and inhibitory reversal potential
        nest.ResetKernel()
        n = nest.Create(model)
        e_ex = n.get(request.param["E_ex"])
        e_in = n.get(request.param["E_in"])
        n.set({"E_ex": e_ex, "E_in": e_in})

        # Spike generator
        sg = nest.Create("spike_generator", params={"spike_times": [1.0]})
        nest.Connect(sg, n, syn_spec={"weight": request.param["sg_weight"], "delay": 1.0})

        # Simulate
        nest.Simulate(self.SIM_TIME)

        # Get V_m from neuron
        v_m = n.get("V_m")
        return v_m

    @pytest.mark.parametrize("get_vm", [{"E_ex": "E_ex", "E_in": "E_in", "sg_weight": 5.0}], indirect=True)
    def test_with_excitatory_input(self, reference_vm, get_vm):
        assert reference_vm < get_vm

    @pytest.mark.parametrize("get_vm", [{"E_ex": "E_ex", "E_in": "E_in", "sg_weight": -5.0}], indirect=True)
    def test_with_inhibitory_input(self, reference_vm, get_vm):
        assert reference_vm > get_vm

    @pytest.mark.parametrize("get_vm", [{"E_ex": "E_in", "E_in": "E_ex", "sg_weight": 5.0}], indirect=True)
    def test_excitatory_input_with_flipped_params(self, reference_vm, get_vm):
        assert reference_vm > get_vm

    @pytest.mark.parametrize("get_vm", [{"E_ex": "E_L", "E_in": "E_L", "sg_weight": 5.0}], indirect=True)
    def test_with_excitatory_input_and_resting_potential(self, model, reference_vm, get_vm):
        if model in self.inaccurate_rest_pot_diff_limit.keys():
            assert abs(reference_vm - get_vm) < self.inaccurate_rest_pot_diff_limit[model]
        else:
            assert reference_vm == get_vm

    @pytest.mark.parametrize("get_vm", [{"E_ex": "E_L", "E_in": "E_L", "sg_weight": -5.0}], indirect=True)
    def test_with_inhibitory_input_and_resting_potential(self, model, reference_vm, get_vm):
        if model in self.inaccurate_rest_pot_diff_limit.keys():
            assert abs(reference_vm - get_vm) < self.inaccurate_rest_pot_diff_limit[model]
        else:
            assert reference_vm == get_vm
