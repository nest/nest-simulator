# -*- coding: utf-8 -*-
#
# test_set_vm.py
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
This test goes through all registered node models for which V_m can be
set, and attempts to set V_m via SetModelStatus and node.set(). It then
compares results. If both ways of setting V_m give different results,
something is wtong with state initialization.

Remarks:
The tests exploits that almost all neuron models have a state variable
V_m. It ignores all other models (the test returns true for them). The
new membrane potential that is set it the potential found in the
neuron +0.1mV. This should not conflict with any limitation requiring
the potential to be set to subthreshold values, but in pathological
cases it may lead to the exclusion of a model that should be tested.
"""

import random

import nest
import pytest


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def get_models():
    all_models = nest.node_models
    return [model for model in all_models if "V_m" in nest.GetDefaults(model)]


def create_set(model, new_vm_value):
    n = nest.Create(model)
    n.set(V_m=new_vm_value)
    return n


def set_defaults_create(model, new_vm_value):
    nest.SetDefaults(model, {"V_m": new_vm_value})
    n = nest.Create(model)
    return n


@pytest.mark.parametrize("model", get_models())
def test_set_vm(model):
    new_vm_value = random.uniform(0, 1)

    set_defaults_create_instance = set_defaults_create(model, new_vm_value)
    create_set_status_instance = create_set(model, new_vm_value)

    assert set_defaults_create_instance.V_m == create_set_status_instance.V_m
