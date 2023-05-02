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
For all models having start, stop, and origin parameters, this test checks that
- default values can be set successfully
- nodes are created with correct default values
- nodes can be created with correct default values from the command line
- parameters can be set via SetStatus
"""

import nest
import pytest
import numpy as np


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def device_attributes():
    return set(["start", "stop", "origin"])


def get_devices():
    all_models = nest.node_models
    models = [m for m in all_models if len(device_attributes().intersection(nest.GetDefaults(m).keys())) > 0]
    return models


@pytest.mark.parametrize('device', get_devices())
def test_one_attr_implies_all_attr(device):
    model_defaults = nest.GetDefaults(device)
    required_all_attr = device_attributes()

    actual = len(required_all_attr.intersection(model_defaults.keys()))
    excepted = len(required_all_attr)

    assert actual == excepted


@pytest.mark.parametrize('device', get_devices())
def test_initial_default_value(device):
    model_defaults = nest.GetDefaults(device, device_attributes())

    unexpected_value = 1234.0 * 42
    assert np.all(model_defaults != unexpected_value)


@pytest.mark.parametrize('device', get_devices())
@pytest.mark.parametrize('attr', device_attributes())
def test_setting_new_default_value(device, attr):
    value = 1234.0 * 42
    nest.SetDefaults(device, {attr: value})

    actual = nest.GetDefaults(device, attr)

    assert actual == value


@pytest.mark.parametrize('device', get_devices())
@pytest.mark.parametrize('attr', device_attributes())
def test_inheriting_new_default_on_instance(device, attr):
    value = 1234.0 * 42

    nest.SetDefaults(device, {attr: value})

    actual = nest.Create(device).get(attr)

    assert actual == value


@pytest.mark.parametrize('device', get_devices())
@pytest.mark.parametrize('attr', device_attributes())
def test_setting_new_value_on_create(device, attr):
    value = 1234.0 * 42

    device_instance = nest.Create(device, params={attr: value})

    actual = device_instance.get(attr)

    assert actual == value


@pytest.mark.parametrize('device', get_devices())
@pytest.mark.parametrize('attr', device_attributes())
def test_setting_new_value_after_create(device, attr):
    value = 1234.0 * 42

    device_instance = nest.Create(device)
    device_instance.set({attr: value})

    actual = device_instance.get(attr)

    assert actual == value
