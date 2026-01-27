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

import nest
import numpy.testing as nptest
import pytest

# Obtain all models with non-empty recordables list
all_models_with_rec = [model for model in nest.node_models if nest.GetDefaults(model).get("recordables")]


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


def test_connect_multimeter_twice():
    """
    Ensure one multimeter can only be connected once to one neuron.

    First, we check that a multimeter can be connected to a neuron once. Then,
    we check that that we cannot connect the multimeter more than once.
    """

    nrn = nest.Create("iaf_psc_alpha")
    mm = nest.Create("multimeter")
    nest.Connect(mm, nrn)

    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(mm, nrn)


@pytest.mark.parametrize("model", all_models_with_rec)
def test_receptors_with_multiple_multimeters(model):
    """
    Test receptors when connecting to multiple multimeters.

    This test is to ensure that connections from two multimeters get
    receptors 1 and 2 for all models with recordables.
    """

    nrn = nest.Create(model)
    mm1 = nest.Create("multimeter", {"record_from": nrn.recordables})
    mm2 = nest.Create("multimeter", {"record_from": nrn.recordables})
    nest.Connect(mm1, nrn)
    nest.Connect(mm2, nrn)

    mm1_receptor = nest.GetConnections(mm1).get("receptor")
    mm2_receptor = nest.GetConnections(mm2).get("receptor")

    assert mm1_receptor == 1
    assert mm2_receptor == 2


@pytest.mark.parametrize("model", all_models_with_rec)
def test_recordables_are_recorded(model):
    """
    Test that recordables are recorded.

    For each model with recordables, set up minimal simulation recording
    from all recordables and test that data is provided. The test checks
    that the correct of amount of data is collected for each recordable.
    It also checks that the recording interval can be set.

    .. note::
       This test does not check if the data is meaningful.
    """

    recording_interval = 2
    simtime = 10
    num_data_expected = simtime / recording_interval - 1

    nrn = nest.Create(model)
    recordables = nrn.recordables
    mm = nest.Create("multimeter", {"interval": recording_interval, "record_from": recordables})
    nest.Connect(mm, nrn)
    nest.Simulate(simtime)

    result = mm.events

    for r in recordables + ("times", "senders"):
        assert r in result
        assert len(result[r]) == num_data_expected


@pytest.mark.parametrize("model", all_models_with_rec)
def test_identical_recording_from_multiple_multimeters(model):
    """
    Test identical recordings from multimeters with same configurations.

    In this test two identical multimeters are connected to the same neuron.
    They should record identical data.
    """

    nrn = nest.Create(model)
    # if the model is compartmental, we need to add at least a root compartment
    if "compartments" in nest.GetDefaults(model):
        nrn.compartments = {"parent_idx": -1}

    recordables = nrn.recordables
    mm1 = nest.Create("multimeter", {"record_from": recordables})
    mm2 = nest.Create("multimeter", {"record_from": recordables})

    nest.Connect(mm1, nrn)
    nest.Connect(mm2, nrn)

    nest.Simulate(10.0)

    for recordable in recordables:
        nptest.assert_array_equal(mm1.events[recordable], mm2.events[recordable])


@pytest.mark.parametrize("interval", [0, 0.05, 0.15])
def test_bad_intervals_detected(interval):
    """
    Test that NEST raises BadProperty if interval cannot be represented as multiple of resolution.
    """

    nest.resolution = 0.1

    with pytest.raises(nest.kernel.NESTError, match="BadProperty"):
        nest.Create("multimeter", params={"interval": interval})
