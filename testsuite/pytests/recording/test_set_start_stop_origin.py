# -*- coding: utf-8 -*-
#
# test_set_start_stop_origin.py
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
Test if `start`, `stop` and `origin` are set properly.
For all models (i.e, Devices) having `start`, `stop`, and `origin` parameters, this test
checks the following:
    - Default values can be set successfully.
    - Nodes are created with correct default values.
    - Nodes can be created with correct default values from the command line.
    - Parameters can be set via SetStatus.
"""

import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def device_attributes():
    return set(["start", "stop", "origin"])


def get_devices():
    all_models = nest.node_models
    models = [m for m in all_models if len(device_attributes().intersection(nest.GetDefaults(m).keys())) > 0]
    return models


@pytest.mark.parametrize("device", get_devices())
def test_one_attr_implies_all_attr(device):
    """Check that any model that has one of the attributes `start`, ``stop` or `origin`, it must have all"""

    model_defaults = nest.GetDefaults(device)
    required_all_attr = device_attributes()

    actual = len(required_all_attr.intersection(model_defaults.keys()))
    excepted = len(required_all_attr)

    assert actual == excepted


@pytest.mark.parametrize("device", get_devices())
def test_initial_default_value(device):
    """Ensure that no default value is equal to test value."""

    model_defaults = nest.GetDefaults(device, device_attributes())

    unexpected_value = 1234.0 * 42
    assert np.all(model_defaults != unexpected_value)


@pytest.mark.parametrize("device", get_devices())
@pytest.mark.parametrize("attr", device_attributes())
def test_setting_new_default_value(device, attr):
    """Check that default value of each attribute can be set."""

    value = 1234.0 * 42
    nest.SetDefaults(device, {attr: value})

    actual = nest.GetDefaults(device, attr)

    assert actual == value


@pytest.mark.parametrize("device", get_devices())
@pytest.mark.parametrize("attr", device_attributes())
def test_inheriting_new_default_on_instance(device, attr):
    """Check that default value of each attribute are transferred to the model instance."""

    value = 1234.0 * 42

    nest.SetDefaults(device, {attr: value})

    actual = nest.Create(device).get(attr)

    assert actual == value


@pytest.mark.parametrize("device", get_devices())
@pytest.mark.parametrize("attr", device_attributes())
def test_setting_new_value_on_create(device, attr):
    """Check that values can be set on creation."""

    value = 1234.0 * 42

    device_instance = nest.Create(device, params={attr: value})

    actual = device_instance.get(attr)

    assert actual == value


@pytest.mark.parametrize("device", get_devices())
@pytest.mark.parametrize("attr", device_attributes())
def test_setting_new_value_after_create(device, attr):
    """Check that values can be set on model instances."""

    value = 1234.0 * 42

    device_instance = nest.Create(device)
    device_instance.set({attr: value})

    actual = device_instance.get(attr)

    assert actual == value
