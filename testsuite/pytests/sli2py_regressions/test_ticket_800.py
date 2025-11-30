# -*- coding: utf-8 -*-
#
# test_ticket_800.py
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

"""
ticket 800: Ensure that spatial parameters check their parameters

Test ported from SLI regression test

This ticket ensures that spatial parameters check their parameters for
validity, e.g., strictly positive sigma for a Gaussian.

Author: Hans Ekkehard Plesser, 2014-12-13
"""

# --- Good and bad parameter sets ---
good_random_parameters = [
    ("uniform", {"min": 0.0, "max": 1.0}),
    ("normal", {"mean": 0.0, "std": 1.0}),
    ("lognormal", {"mean": 0.0, "std": 1.0}),
    ("exponential", {"beta": 1.0}),
]

good_distance_parameters = [
    ("distance", {}),
    ("distance", {"dimension": 1}),
    ("distance", {"dimension": 2}),
]

good_parameters = good_random_parameters + good_distance_parameters

bad_random_parameters = [
    ("uniform", {"min": 0.0, "max": 0.0}),
    ("normal", {"mean": 0.0, "sigma": 1.0}),  # bad param name
    ("normal", {"mean": 0.0, "std": 0.0}),
    ("normal", {"mean": 0.0, "std": -1.0}),
    ("lognormal", {"mu": 0.0, "std": 0.0}),
    ("lognormal", {"mu": 0.0, "std": -1.0}),
    ("lognormal", {"mu": 0.0, "std": 1.0}),
]

bad_distance_parameters = [
    ("distance", {"dim": 1}),
    ("distance", {"dimension": 1, "min": 0.0}),
]

bad_parameters = bad_random_parameters + bad_distance_parameters

# --- Good and bad masks ---
good_masks_2d = [
    ("circular", {"radius": 1.0}),
    ("circular", {"radius": 1.0, "anchor": [0.5, 0.5]}),
    ("rectangular", {"lower_left": [-0.1, -0.1], "upper_right": [0.1, 0.1]}),
    ("doughnut", {"inner_radius": 0.1, "outer_radius": 0.2}),
]

good_masks_3d = [
    ("spherical", {"radius": 1.0}),
    ("box", {"lower_left": [-0.1, -0.1, -0.1], "upper_right": [0.1, 0.1, 0.1]}),
]
good_masks = good_masks_2d + good_masks_3d

bad_masks_2d = [
    ("circular", {"r": 0.0}),
    ("circular", {"radius": 1.0, "foo": 1}),
    ("circular", {"radius": 0.0}),
    ("circular", {"radius": -1.0}),
    ("rectangular", {"lower_left": [-0.1, -0.1], "upper_right": [-0.1, 0.1]}),
    ("rectangular", {"lower_left": [-0.1, -0.1], "upper_right": [0.1, -0.1]}),
    ("doughnut", {"inner_radius": 0.1, "outer_radius": 0.1}),
]

bad_masks_3d = [
    ("spherical", {"radius": 0.0}),
    ("spherical", {"radius": -1.0}),
    ("box", {"lower_left": [-0.1, -0.1, -0.1], "upper_right": [-0.1, 0.1, 0.1]}),
    ("box", {"lower_left": [-0.1, -0.1, -0.1], "upper_right": [0.1, -0.1, 0.1]}),
    ("box", {"lower_left": [-0.1, -0.1, -0.1], "upper_right": [0.1, 0.1, -0.1]}),
]
bad_masks = bad_masks_2d + bad_masks_3d

# --- Good and bad position specs ---
good_positions = [
    nest.spatial.grid(shape=[2, 2]),
    nest.spatial.grid(shape=[2, 2], extent=[2.0, 0.5], center=[0.2, 0.3], edge_wrap=True),
    nest.spatial.grid(shape=[2, 2], extent=[2.0, 0.5], center=[0.2, 0.3], edge_wrap=False),
    nest.spatial.free(pos=[[0.1, 0.1], [0.2, 0.2]], extent=[2.0, 0.5], edge_wrap=True),
    nest.spatial.free(pos=[[0.1, 0.1, 0.1], [0.2, 0.2, 0.2]], extent=[2.0, 0.5, 3.0], edge_wrap=True),
]

bad_positions = [
    nest.spatial.grid(shape=[1]),
    nest.spatial.grid(shape=[2, 2], extent=[2.0, 0.5], center=[0.2, 0.3], edge_wrap=3),  # bad datatype
    nest.spatial.free(pos=[[0.1, 0.1], [0.2, 0.2, 0.2]], extent=[2.0, 0.5], edge_wrap=True),  # dimension mix-up
    nest.spatial.free(pos=[[0.1, 0.1, 0.1], [0.2, 0.2, 0.2]], extent=[2.0, 0.5], edge_wrap=True),  # dimension mix-up
]

# --- Good and bad conn specs ---
good_connspecs = [
    {"rule": "pairwise_bernoulli", "use_on_source": True},
    {"rule": "pairwise_bernoulli", "use_on_source": False},
    {"rule": "pairwise_bernoulli", "use_on_source": True, "allow_oversized_mask": True},
    {"rule": "pairwise_bernoulli", "use_on_source": False, "allow_oversized_mask": True},
    {"rule": "pairwise_bernoulli", "use_on_source": True, "allow_multapses": False},
    {"rule": "pairwise_bernoulli", "use_on_source": True, "allow_autapses": False},
    {"rule": "fixed_outdegree", "outdegree": 2},
    {"rule": "fixed_indegree", "indegree": 2},
]

bad_connspecs = [
    {"rule": "population"},  # undefined rule
]


@pytest.fixture
def layer_2d():
    yield nest.Create("parrot_neuron", positions=nest.spatial.grid(shape=[1, 1]))


@pytest.fixture
def layer_3d():
    yield nest.Create("parrot_neuron", positions=nest.spatial.free(pos=[[0.0, 0.0, 0.0]], extent=[1.0, 1.0, 1.0]))


# Test 1
@pytest.mark.parametrize("kind, params", good_parameters)
def test_good_parameters(kind, params):
    nest.CreateParameter(kind, params)


# Test 2
@pytest.mark.parametrize("kind, params", bad_parameters)
def test_bad_parameters(kind, params):
    with pytest.raises((nest.NESTErrors.BadProperty, nest.NESTErrors.UnaccessedDictionaryEntry)):
        nest.CreateParameter(kind, params)


# Test 3
@pytest.mark.parametrize("mask_type, maskdict", good_masks)
def test_good_masks(mask_type, maskdict):
    nest.CreateMask(mask_type, maskdict)


# Test 4
@pytest.mark.parametrize("mask_type, maskdict", bad_masks)
def test_bad_masks(mask_type, maskdict):
    with pytest.raises((nest.NESTErrors.BadProperty, nest.NESTErrors.UnaccessedDictionaryEntry, IndexError)):
        nest.CreateMask(mask_type, maskdict)


# Test 5
@pytest.mark.parametrize("positions", good_positions)
def test_good_positions(positions):
    # Use grid or free depending on positions
    nest.Create("parrot_neuron", positions=positions)


# Test 6
@pytest.mark.parametrize("positions", bad_positions)
def test_bad_positions(positions):
    # Use grid or free depending on positions
    with pytest.raises((nest.NESTErrors.BadProperty, nest.NESTErrors.TypeMismatch)):
        nest.Create("parrot_neuron", positions=positions)


# Test 7
@pytest.mark.parametrize("p_name, p_spec", good_parameters)
def test_good_kernels(p_name, p_spec, layer_2d):
    nest.Connect(layer_2d, layer_2d, {"rule": "pairwise_bernoulli", "p": nest.CreateParameter(p_name, p_spec)})


# Test 8
@pytest.mark.parametrize("p_name, p_spec", bad_parameters)
def test_bad_kernels(p_name, p_spec, layer_2d):
    with pytest.raises((nest.NESTErrors.BadProperty, nest.NESTErrors.UnaccessedDictionaryEntry)):
        nest.Connect(layer_2d, layer_2d, {"rule": "pairwise_bernoulli", "p": nest.CreateParameter(p_name, p_spec)})


# Test 9
@pytest.mark.parametrize("p_name, p_spec", good_parameters)
def test_good_weights(p_name, p_spec, layer_2d):
    nest.Connect(
        layer_2d, layer_2d, {"rule": "pairwise_bernoulli", "p": 1}, {"weight": nest.CreateParameter(p_name, p_spec)}
    )


# Test 10
@pytest.mark.parametrize("p_name, p_spec", bad_parameters)
def test_bad_weights(p_name, p_spec, layer_2d):
    with pytest.raises((nest.NESTErrors.BadProperty, nest.NESTErrors.UnaccessedDictionaryEntry)):
        nest.Connect(
            layer_2d, layer_2d, {"rule": "pairwise_bernoulli", "p": 1}, {"weight": nest.CreateParameter(p_name, p_spec)}
        )


# Test 11
@pytest.mark.parametrize("mask_type, mask_dict", good_masks_2d)
def test_good_masks_2d(mask_type, mask_dict, layer_2d):
    nest.Connect(layer_2d, layer_2d, {"rule": "pairwise_bernoulli", "mask": {mask_type: mask_dict}})


# Test 12
@pytest.mark.parametrize("mask_type, mask_dict", bad_masks_2d)
def test_bad_masks_2d(mask_type, mask_dict, layer_2d):
    with pytest.raises((nest.NESTErrors.BadProperty, nest.NESTErrors.UnaccessedDictionaryEntry, IndexError)):
        nest.Connect(layer_2d, layer_2d, {"rule": "pairwise_bernoulli", "mask": {mask_type: mask_dict}})


# Test 13
@pytest.mark.parametrize("mask_type, mask_dict", good_masks_3d)
def test_good_masks_3d(mask_type, mask_dict, layer_3d):
    nest.Connect(layer_3d, layer_3d, {"rule": "pairwise_bernoulli", "mask": {mask_type: mask_dict}})


# Test 14
@pytest.mark.parametrize("mask_type, mask_dict", bad_masks_3d)
def test_bad_masks_3d(mask_type, mask_dict, layer_3d):
    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(layer_3d, layer_3d, {"rule": "pairwise_bernoulli", "mask": {mask_type: mask_dict}})


# Test 15
@pytest.mark.parametrize("connspec", good_connspecs)
def test_good_connspecs(connspec, layer_2d):
    nest.Connect(layer_2d, layer_2d, connspec)


# Test 16
@pytest.mark.parametrize("connspec", bad_connspecs)
def test_bad_connspecs(connspec, layer_2d):
    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(layer_2d, layer_2d, connspec)
