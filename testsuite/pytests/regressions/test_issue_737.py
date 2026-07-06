# -*- coding: utf-8 -*-
#
# test_issue_737.py
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
Regression test for Issue #737 (GitHub).

This test ensures that NEST raises an error if the user tries to set synapse
parameters that cannot be set with Connect calls.
"""

import nest
import pytest


def test_static_synapse_hom_w_weight():
    """
    Test that setting weight in syn_spec for static_synapse_hom_w model raises an error.
    """
    nest.ResetKernel()
    n = nest.Create("iaf_psc_alpha")

    with pytest.raises(Exception):
        nest.Connect(
            n, n, conn_spec={"rule": "one_to_one"}, syn_spec={"synapse_model": "static_synapse_hom_w", "weight": 2.0}
        )


def test_copymodel_static_synapse_hom_w_weight():
    """
    Test that setting weight in syn_spec for CopyModel of static_synapse_hom_w model raises an error.
    """
    nest.ResetKernel()
    n = nest.Create("iaf_psc_alpha")
    nest.CopyModel("static_synapse_hom_w", "mysyn")

    with pytest.raises(Exception):
        nest.Connect(n, n, conn_spec={"rule": "one_to_one"}, syn_spec={"synapse_model": "mysyn", "weight": 2.0})


def test_stdp_dopamine_synapse_vt():
    """
    Test that setting vt in syn_spec for stdp_dopamine_synapse model raises an error.
    """
    nest.ResetKernel()
    n = nest.Create("iaf_psc_alpha")

    with pytest.raises(Exception):
        nest.Connect(
            n, n, conn_spec={"rule": "one_to_one"}, syn_spec={"synapse_model": "stdp_dopamine_synapse", "vt": 2.0}
        )


def test_copymodel_stdp_dopamine_synapse_vt():
    """
    Test that setting vt in syn_spec for CopyModel of stdp_dopamine_synapse model raises an error.
    """
    nest.ResetKernel()
    n = nest.Create("iaf_psc_alpha")
    nest.CopyModel("stdp_dopamine_synapse", "mysyn")

    with pytest.raises(Exception):
        nest.Connect(n, n, conn_spec={"rule": "one_to_one"}, syn_spec={"synapse_model": "mysyn", "vt": 2.0})
