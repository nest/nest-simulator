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

# --- Good and bad layer specs ---
good_layers = [
    {"shape": [2, 2], "elements": "iaf_psc_alpha"},
    {"shape": [2, 2], "elements": "iaf_psc_alpha", "extent": [2.0, 0.5], "center": [0.2, 0.3], "edge_wrap": True},
    {"shape": [2, 2], "elements": "iaf_psc_alpha", "extent": [2.0, 0.5], "center": [0.2, 0.3], "edge_wrap": False},
    {"positions": [[0.1, 0.1], [0.2, 0.2]], "elements": "iaf_psc_alpha", "extent": [2.0, 0.5], "edge_wrap": True},
    {
        "positions": [[0.1, 0.1, 0.1], [0.2, 0.2, 0.2]],
        "elements": "iaf_psc_alpha",
        "extent": [2.0, 0.5, 3.0],
        "edge_wrap": True,
    },
]
bad_layers = [
    {"shape": [1], "elements": "iaf_psc_alpha"},
    {"shape": [2, 2], "elementsss": "iaf_psc_alpha"},
    {"shape": [2, 2], "elements": "iaf_psc_alpha", "extent": [2.0, 0.5], "center": [0.2, 0.3], "edge_wrap": 3},
    {"positions": [[0.1, 0.1], [0.2, 0.2, 0.2]], "elements": "iaf_psc_alpha", "extent": [2.0, 0.5], "edge_wrap": True},
    {
        "positions": [[0.1, 0.1, 0.1], [0.2, 0.2, 0.2]],
        "elements": "iaf_psc_alpha",
        "extent": [2.0, 0.5],
        "edge_wrap": True,
    },
]


@pytest.mark.parametrize("kind, params", good_parameters)
def test_good_parameters(kind, params):
    nest.CreateParameter(kind, params)


@pytest.mark.parametrize("kind, params", bad_parameters)
def test_bad_parameters(kind, params):
    with pytest.raises(Exception):
        nest.CreateParameter(kind, params)


@pytest.mark.parametrize("mask_type, maskdict", good_masks)
def test_good_masks(mask_type, maskdict):
    nest.CreateMask(mask_type, maskdict)


@pytest.mark.parametrize("mask_type, maskdict", bad_masks)
def test_bad_masks(mask_type, maskdict):
    with pytest.raises(Exception):
        nest.CreateMask(mask_type, maskdict)


@pytest.mark.parametrize("layer_spec", good_layers)
def test_good_layers(layer_spec):
    # Use grid or free depending on positions
    nest.ResetKernel()
    if "positions" in layer_spec:
        positions = layer_spec["positions"]
        nest.Create(
            layer_spec.get("elements", "iaf_psc_alpha"),
            positions=positions,
            extent=layer_spec.get("extent"),
            edge_wrap=layer_spec.get("edge_wrap"),
        )
    else:
        shape = layer_spec["shape"]
        nest.Create(
            layer_spec.get("elements", "iaf_psc_alpha"),
            positions=nest.spatial.grid(
                shape,
                extent=layer_spec.get("extent"),
                center=layer_spec.get("center"),
                edge_wrap=layer_spec.get("edge_wrap"),
            ),
        )


@pytest.mark.parametrize("layer_spec", bad_layers)
def test_bad_layers(layer_spec):
    nest.ResetKernel()
    with pytest.raises(Exception):
        if "positions" in layer_spec:
            positions = layer_spec["positions"]
            nest.Create(
                layer_spec.get("elements", "iaf_psc_alpha"),
                positions=positions,
                extent=layer_spec.get("extent"),
                edge_wrap=layer_spec.get("edge_wrap"),
            )
        else:
            shape = layer_spec["shape"]
            nest.Create(
                layer_spec.get("elements", "iaf_psc_alpha"),
                positions=nest.spatial.grid(
                    shape,
                    extent=layer_spec.get("extent"),
                    center=layer_spec.get("center"),
                    edge_wrap=layer_spec.get("edge_wrap"),
                ),
            )
