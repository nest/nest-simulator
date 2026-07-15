# -*- coding: utf-8 -*-
#
# test_set_defaults.py
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
Test for `SetDefaults`, still incomplete.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.mark.parametrize(
    "model, param",
    [
        ["iaf_psc_alpha", "tau_syn_ex"],
        # ["stdp_synapse", "alpha"]
    ],
)
@pytest.mark.parametrize("as_dict", [True, False])
@pytest.mark.parametrize("bad_value", [[1.0, 2.0, 3.0], nest.random.uniform(0, 1)])
def test_set_defaults_rejects_non_definite_types(model, param, as_dict, bad_value):
    """Confirm that SetDefaults rejects arrays or Parameter objects."""

    with pytest.raises(ValueError):
        if as_dict:
            nest.SetDefaults(model, {param: bad_value})
        else:
            nest.SetDefaults(model, param, bad_value)
