# -*- coding: utf-8 -*-
#
# test_ticket_459.py
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
Test that changing E_L in any neuron with this parameter leaves all other parameters unchanged.
"""

import nest
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


# Collect all models with E_L parameter.
# Skip gif_cond_exp_multisynapse because it has numpy array parameter making check below difficult.
models_with_EL = [model for model in nest.node_models if "E_L" in nest.GetDefaults(model)]
models_with_EL.remove("gif_cond_exp_multisynapse")


@pytest.mark.parametrize("model", models_with_EL)
def test_clean_EL_change(model):
    nrn = nest.Create(model)
    orig_params = nrn.get()

    EL_orig = orig_params["E_L"]
    EL_new = EL_orig + 0.7
    nrn.E_L = EL_new

    # Confirm E_L has been changed.
    assert nrn.E_L == EL_new

    # Confirm all other parameters are equal to original values.
    new_params = nrn.get()
    del orig_params["E_L"]
    del new_params["E_L"]
    nptest.assert_equal(new_params, orig_params)
