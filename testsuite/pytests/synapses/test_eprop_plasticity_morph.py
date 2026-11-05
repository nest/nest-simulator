# -*- coding: utf-8 -*-
#
# test_eprop_plasticity_morph.py
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
Run the e-prop tests with reference values of ``test_eprop_plasticity.py``
with structure-aware communication (morph).

Connections between neurons are long_range (see ``eprop_morph_helpers.py``); the tests must reproduce the
reference values of the conventional exchange.
"""

import pytest
from eprop_morph_helpers import morph_connect  # noqa: F401
from test_eprop_plasticity import fix_resolution  # noqa: F401
from test_eprop_plasticity import test_eprop_history_cleaning  # noqa: F401
from test_eprop_plasticity import test_eprop_regression  # noqa: F401
from test_eprop_plasticity import test_eprop_surrogate_gradients  # noqa: F401

pytestmark = pytest.mark.skipif_missing_morph


@pytest.fixture(autouse=True)
def long_range_eprop(morph_connect):  # noqa: F811
    """Run each test with long_range connections between neurons and check that some were created."""

    yield
    assert morph_connect["long_range"] > 0
