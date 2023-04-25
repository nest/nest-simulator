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

import nest
import numpy as np
import pytest

from conftest import get_node_models_by_attribute


class TestCalcium:
    r"""
    Check that we can set and get the calcium concentration from all nodes that inherit from the structural plasticity
    node class in C++.
    """

    @pytest.fixture(autouse=True)
    def prepare_test(self):
        nest.ResetKernel()

    def _test_calcium_single_model(self, model):
        ca_default = nest.GetDefaults(model, "Ca")
        n = nest.Create(model, params={'Ca': ca_default + 42.})
        np.testing.assert_allclose(n.Ca, ca_default + 42.)
        n.Ca = ca_default + 99999.
        np.testing.assert_allclose(n.Ca, ca_default + 99999.)

    def test_calcium(self):
        models = get_node_models_by_attribute("Ca")
        assert len(models) > 0
        for model in models:
            self._test_calcium_single_model(model)
