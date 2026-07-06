# -*- coding: utf-8 -*-
#
# test_calcium.py
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
Test models with calcium concentration.

This set of tests verify the behavior of the calcium concentration in models
that inherit from the strutural plasticity node class in the kernel.
"""

import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


# Obtain all models with calcium concentration
models = [model for model in nest.node_models if "Ca" in nest.GetDefaults(model)]


def test_at_least_one_model():
    """
    Verify that that at least one model contains calcium concentration.
    """

    assert len(models) > 0


@pytest.mark.parametrize("model", models)
def test_calcium_set_get(model):
    """
    Verify setters and getters for models with calcium concentration.
    """

    ca_default = nest.GetDefaults(model, "Ca")
    n = nest.Create(model, params={"Ca": ca_default + 42.0})
    np.testing.assert_allclose(n.Ca, ca_default + 42.0)
    n.Ca = n.Ca + 99999.0
    np.testing.assert_allclose(n.Ca, ca_default + 42.0 + 99999.0)
