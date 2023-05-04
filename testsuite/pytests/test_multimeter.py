# -*- coding: utf-8 -*-
#
# test_multimeter.py
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

import pytest

import nest


@pytest.fixture
def set_resolution():
    """
    Set resolution to power of two to avoid rounding issues.
    """

    nest.ResetKernel()
    nest.resolution = 2**-3


# Obtain all models with non-empty recordables list
models = [
    model for model in nest.node_models if nest.GetDefaults(model).get("recordables")
]


@pytest.mark.parametrize("model", models)
def test_recordables_are_recorded(set_resolution, model):
    """
    Test that recordables are recorded.

    For each model with recordables, set up minimal simulation
    recording from all recordables and test that data is provided.
    I also checks that the recording interval can be set.

    .. note::
       This test does not check if the data is meaningful.
    """

    recording_interval = 2
    simtime = 10
    num_data_expected = simtime / recording_interval - 1

    nrn = nest.Create(model)
    recordables = nrn.recordables
    mm = nest.Create(
        "multimeter", {"interval": recording_interval, "record_from": recordables}
    )
    nest.Connect(mm, nrn)
    nest.Simulate(simtime)

    result = mm.events

    for r in recordables + ("times", "senders"):
        assert r in result
        assert len(result[r]) == num_data_expected


@pytest.mark.parametrize("model", models)
def test_correct_amount_data_collected(model):
    """
    Test that the correct of amount of data is collected for each recordable.

    For all models providing a 'recordables' entry in their status dict, this
    test checks whether the correct amount of data is collected for each
    recordable quantity.

    .. note::
       This test does not check the content of the data collected.
    """

    nest.ResetKernel()

    nrn = nest.Create(model)

    # if the model is compartmental, we need to add at least a root compartment
    if "compartments" in nest.GetDefaults(model):
        nrn.compartments = {"parent_idx": -1}

    recordables = nrn.recordables
    mm = nest.Create("multimeter", {"record_from": recordables})

    nest.Connect(mm, nrn)
    nest.Simulate(10.0)

    for recordable in recordables:
        assert len(mm.events[recordable]) == 9


def test_multimeter_freeze():
    """
    Ensure that frozen parameter can be set to False but not True on multimeter.
    """

    nest.Create("multimeter", params={"frozen": False})
    with pytest.raises(Exception):
        nest.Create("multimeter", params={"frozen": True})
