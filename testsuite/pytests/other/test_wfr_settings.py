# -*- coding: utf-8 -*-
#
# test_wfr_settings.py
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
This set of tests checks the possible settings for the waveform relaxation method.

The waveform relaxation method is used for iterative solution when connections
without delay are present (e.g. gap junctions).
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.mark.parametrize("use_wfr", [False, True])
def test_set_wfr(use_wfr):
    """Test that ``use_wfr`` can be set."""

    nest.set(use_wfr=use_wfr)


def test_set_wfr_after_node_creation_raises():
    """Ensure that ``use_wfr`` cannot be set after nodes are created."""

    nest.Create("iaf_psc_alpha")

    with pytest.raises(nest.NESTErrors.KernelException):
        nest.set(use_wfr=True)


def test_wfr_comm_interval_lower_than_resolution_raises():
    """Ensure that ``wfr_comm_interval`` cannot be set lower than the resolution."""

    with pytest.raises(nest.NESTErrors.KernelException):
        nest.set(resolution=0.1, wfr_comm_interval=0.05)


def test_wfr_comm_interval_cannot_be_set_when_use_wfr_false():
    """Ensure that ``wfr_comm_interval`` cannot be set if ``use_wfr=False``."""

    with pytest.raises(nest.NESTErrors.KernelException):
        nest.set(use_wfr=False, wfr_comm_interval=0.5)


def test_wfr_comm_interval_set_to_resolution_after_disabling_use_wfr():
    """Ensure that ``wfr_comm_interval`` is set to and updated with resolution when disabling ``use_wfr``."""

    nest.set(use_wfr=True, wfr_comm_interval=0.5)

    # Disable use_wfr
    nest.set(resolution=0.1, use_wfr=False)

    assert nest.resolution == 0.1
    assert nest.wfr_comm_interval == nest.resolution

    # Ensure that wfr_comm_interval is updated with the resolution
    nest.set(resolution=0.2)

    assert nest.resolution == 0.2
    assert nest.wfr_comm_interval == nest.resolution


def test_wfr_comm_interval_not_updated_with_resolution_when_use_wfr_true():
    """Ensure that ``wfr_comm_interval`` is not updated with resolution if ``use_wfr=True``."""

    nest.set(use_wfr=True, wfr_comm_interval=2.0)
    nest.set(resolution=0.1)
    assert nest.wfr_comm_interval == 2.0


@pytest.mark.skipif_missing_gsl
@pytest.mark.parametrize("use_wfr", [False, True])
def test_use_wfr_set_correctly_in_created_node(use_wfr):
    """
    Test correct set of ``use_wfr`` in created node.

    The test verifies both the case with ``use_wfr`` off and on.
    """

    nest.set(use_wfr=use_wfr)

    nrn_gap = nest.Create("hh_psc_alpha_gap")
    nest.Simulate(5.0)

    assert nrn_gap.node_uses_wfr == use_wfr
