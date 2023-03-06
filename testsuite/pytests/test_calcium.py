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


@nest.ll_api.check_stack
class TestCalcium:
    r"""
    Check that we can set and get the calcium concentration from all nodes that inherit from the structural plasticity
    node class in C++.
    """
    @pytest.fixture(autouse=True)
    def reset_kernel(self):
        nest.ResetKernel()

    def test_calcium(self):
        """Test all models in :python:`nest.node_models`"""
        n_models_tested = 0
        for model in nest.node_models:
            n = nest.Create(model)
            if "Ca" not in n.get():
                continue

            n_models_tested += 1
            n = nest.Create(model, params={"Ca": 123.})
            np.testing.assert_almost_equal(n.Ca, 123.)
            n.Ca = 42.
            np.testing.assert_almost_equal(n.Ca, 42.)

        assert n_models_tested > 63
