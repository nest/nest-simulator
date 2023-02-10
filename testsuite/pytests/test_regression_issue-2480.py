# -*- coding: utf-8 -*-
#
# test_regression_issue-2480.py
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
import warnings


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.mark.parametrize("model", [m for m in nest.node_models if 'V_m' in nest.GetDefaults(m)])
def test_set_vm(model):
    nest.set_verbosity('M_FATAL')
    warnings.simplefilter('ignore')  # Suppress warnings
    n = nest.Create(model)

    try:
        n.V_m = 0.5
    except nest.NESTError as e:
        assert False, f'Setting a V_m with a constant raises {e}'

    try:
        n.V_m = nest.random.uniform(0., 1.)
    except nest.NESTError as e:
        assert False, f'Setting a V_m with a parameter raises {e}'
