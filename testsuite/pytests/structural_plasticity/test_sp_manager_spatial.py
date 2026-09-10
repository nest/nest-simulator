# -*- coding: utf-8 -*-
#
# test_sp_manager_spatial.py
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
Tests for the spatial kernel and mask of structural plasticity.

The kernel weights each candidate partner and the mask excludes candidates outside it. Both are
evaluated through the target's layer, so periodic boundary conditions are respected.
"""

import nest
import numpy as np
import pytest


def build_network(positions, axon_z, den_z, *, allow_autapses=True, edge_wrap=False, extent=None, seed=1):
    """
    Create spatially distributed neurons with fixed numbers of synaptic elements.

    ``axon_z[i]`` and ``den_z[i]`` are the pre- and postsynaptic element counts of neuron ``i``.
    The growth rate is zero, so each neuron offers exactly that many elements.
    """

    nest.ResetKernel()
    nest.rng_seed = seed
    nest.structural_plasticity_update_interval = 1.0

    nest.CopyModel("static_synapse", "sp_synapse")
    nest.SetDefaults("sp_synapse", {"weight": 1.0, "delay": 1.0})
    nest.structural_plasticity_synapses = {
        "sp_syn": {
            "synapse_model": "sp_synapse",
            "pre_synaptic_element": "Axon_ex",
            "post_synaptic_element": "Den_ex",
            "allow_autapses": allow_autapses,
        }
    }

    spatial_kwargs = {"pos": [[float(x) for x in pos] for pos in positions], "edge_wrap": edge_wrap}
    if extent is not None:
        spatial_kwargs["extent"] = [float(x) for x in extent]

    params = [
        {
            "synaptic_elements": {
                "Axon_ex": {"z": float(a), "growth_rate": 0.0},
                "Den_ex": {"z": float(d), "growth_rate": 0.0},
            }
        }
        for a, d in zip(axon_z, den_z)
    ]

    return nest.Create("iaf_psc_alpha", len(positions), params, positions=nest.spatial.free(**spatial_kwargs))


def sp_connections():
    """Return the source and target node IDs of the connections created by structural plasticity."""

    conns = nest.GetConnections(synapse_model="sp_synapse")
    if len(conns) == 0:
        return [], []
    sources, targets = conns.source, conns.target
    if not isinstance(sources, (list, tuple)):
        sources, targets = [sources], [targets]
    return list(sources), list(targets)


def gaussian_kernel(std, mean=0.0):
    return nest.spatial_distributions.gaussian(nest.spatial.distance, mean=mean, std=std)


def test_gaussian_follows_nest_convention():
    """
    The kernel is NEST's Gaussian, so K(0) = 1, K(std) = exp(-1/2) and K(2 std) = exp(-2).

    This pins ``std`` to NEST's convention, exp(-(d - mean)^2 / (2 std^2)).
    """

    std = 0.7
    nrns = build_network([(0.0, 0.0), (1.0, 0.0)], [1, 0], [0, 1])
    values = gaussian_kernel(std=std).apply(nrns[0], [[0.0, 0.0], [std, 0.0], [2 * std, 0.0]])

    assert values == pytest.approx([1.0, np.exp(-0.5), np.exp(-2.0)])


def test_selection_frequencies_match_kernel_values():
    """
    A candidate is chosen in proportion to the value of the spatial kernel.

    One presynaptic element is offered two candidates at different distances; over many seeds the
    fraction choosing the nearer one must match the ratio of the two kernel values.
    """

    std = 1.0
    near, far = 0.5, 1.5
    positions = [(0.0, 0.0), (near, 0.0), (far, 0.0)]
    num_runs = 1000

    # The expected split comes from the very Parameter that structural plasticity is given below.
    reference = build_network(positions, [1, 0, 0], [0, 1, 1])
    weights = np.array(gaussian_kernel(std=std).apply(reference[0], [[near, 0.0], [far, 0.0]]))
    expected = weights[0] / weights.sum()

    num_near = 0
    for seed in range(1, num_runs + 1):
        nrns = build_network(positions, [1, 0, 0], [0, 1, 1], seed=seed)
        near_id = nrns[1].global_id
        nest.EnableStructuralPlasticity(spatial_kernel=gaussian_kernel(std=std))
        nest.Simulate(5.0)

        _, targets = sp_connections()
        assert len(targets) == 1
        num_near += targets[0] == near_id

    # Four standard errors of the binomial estimate, so the tolerance does not depend on the seeds.
    tolerance = 4 * np.sqrt(expected * (1 - expected) / num_runs)
    assert num_near / num_runs == pytest.approx(expected, abs=tolerance)


def test_gaussian_kernel_without_mask_prefers_near_targets():
    """A kernel alone biases connectivity towards nearby partners without excluding anyone."""

    rng = np.random.default_rng(1234)
    positions = rng.uniform(0, 10, (30, 2))

    build_network(positions, [1] * 30, [1] * 30)
    nest.EnableStructuralPlasticity(spatial_kernel=gaussian_kernel(std=1.0))
    nest.Simulate(5.0)

    sources, targets = sp_connections()
    assert len(sources) == 30

    distances = np.linalg.norm(positions[np.array(sources) - 1] - positions[np.array(targets) - 1], axis=1)
    # Two uniform draws from this square average about 5.2 apart; std=1.0 must do far better.
    assert distances.mean() < 2.0


def test_mask_without_kernel_excludes_distant_targets():
    """
    A mask alone restricts the partners without weighting them.

    This also guards against a mask being ignored when no kernel is given: positions must be
    gathered whenever either is configured.
    """

    radius = 3.0
    rng = np.random.default_rng(5678)
    positions = rng.uniform(0, 10, (30, 2))

    build_network(positions, [1] * 30, [1] * 30)
    nest.EnableStructuralPlasticity(mask=nest.CreateMask("circular", {"radius": radius}))
    nest.Simulate(5.0)

    sources, targets = sp_connections()
    assert len(sources) > 0

    distances = np.linalg.norm(positions[np.array(sources) - 1] - positions[np.array(targets) - 1], axis=1)
    assert distances.max() <= radius


def test_kernel_and_mask_combined():
    """The kernel weights the candidates that the mask admits."""

    radius = 3.0
    rng = np.random.default_rng(9012)
    positions = rng.uniform(0, 10, (30, 2))

    build_network(positions, [1] * 30, [1] * 30)
    nest.EnableStructuralPlasticity(
        spatial_kernel=gaussian_kernel(std=2.0),
        mask=nest.CreateMask("circular", {"radius": radius}),
    )
    nest.Simulate(5.0)

    sources, targets = sp_connections()
    assert len(sources) > 0

    distances = np.linalg.norm(positions[np.array(sources) - 1] - positions[np.array(targets) - 1], axis=1)
    assert distances.max() <= radius


@pytest.mark.parametrize("num_dimensions", [2, 3])
def test_layers_of_two_and_three_dimensions(num_dimensions):
    """Both two- and three-dimensional layers can be used with a kernel and a mask."""

    rng = np.random.default_rng(2468)
    positions = rng.uniform(0, 10, (25, num_dimensions))
    mask_name = "circular" if num_dimensions == 2 else "spherical"
    radius = 4.0

    build_network(positions, [1] * 25, [1] * 25)
    nest.EnableStructuralPlasticity(
        spatial_kernel=gaussian_kernel(std=2.0),
        mask=nest.CreateMask(mask_name, {"radius": radius}),
    )
    nest.Simulate(5.0)

    sources, targets = sp_connections()
    assert len(sources) > 0

    distances = np.linalg.norm(positions[np.array(sources) - 1] - positions[np.array(targets) - 1], axis=1)
    assert distances.max() <= radius


def test_periodic_boundary_conditions_are_respected():
    """
    Distances are computed by the layer, so wrapped neighbours count as close.

    The wrapped candidate is 0.1 away across the boundary but 0.9 away without wrapping; the direct
    one is 0.4 away either way. A radius of 0.2 admits only the former, and neither without wrapping.
    """

    positions = [(0.05, 0.5), (0.95, 0.5), (0.45, 0.5)]

    nrns = build_network(positions, [1, 0, 0], [0, 1, 1], edge_wrap=True, extent=[1.0, 1.0])
    wrapped_id = nrns[1].global_id
    nest.EnableStructuralPlasticity(mask=nest.CreateMask("circular", {"radius": 0.2}))
    nest.Simulate(5.0)
    assert sp_connections()[1] == [wrapped_id]

    build_network(positions, [1, 0, 0], [0, 1, 1], edge_wrap=False, extent=[1.0, 1.0])
    nest.EnableStructuralPlasticity(mask=nest.CreateMask("circular", {"radius": 0.2}))
    nest.Simulate(5.0)
    assert sp_connections()[1] == []


def test_periodic_boundary_conditions_affect_kernel_values():
    """The kernel sees the wrapped distance as well, not just the mask."""

    positions = [(0.05, 0.5), (0.95, 0.5), (0.45, 0.5)]
    nrns = build_network(positions, [1, 0, 0], [0, 1, 1], edge_wrap=True, extent=[1.0, 1.0])
    wrapped_id = nrns[1].global_id

    # The wrapped candidate outweighs the direct one by exp((0.16 - 0.01) / 0.02) > 1e3.
    nest.EnableStructuralPlasticity(spatial_kernel=gaussian_kernel(std=0.1))
    nest.Simulate(5.0)
    assert sp_connections()[1] == [wrapped_id]


@pytest.mark.parametrize(
    "kernel_name",
    ["negative", "infinite", "nan"],
)
def test_invalid_kernel_values_are_rejected(kernel_name):
    """A kernel must evaluate to a finite, non-negative value for every candidate pair."""

    distance = nest.spatial.distance
    kernels = {
        "negative": distance * -1.0,
        "infinite": nest.math.exp(distance * 1e4),
        "nan": nest.math.exp(distance * 1e4) * 0.0,
    }

    build_network([(0.0, 0.0), (1.0, 0.0)], [1, 0], [0, 1])
    nest.EnableStructuralPlasticity(spatial_kernel=kernels[kernel_name])

    with pytest.raises(nest.NESTErrors.BadProperty, match="finite, non-negative"):
        nest.Simulate(5.0)


def test_all_zero_weights_leave_element_unmatched():
    """When every candidate has zero weight, the presynaptic element stays vacant."""

    nrns = build_network([(0.0, 0.0), (1.0, 0.0)], [1, 0], [0, 1])
    nest.EnableStructuralPlasticity(spatial_kernel=nest.spatial.distance * 0.0)
    nest.Simulate(5.0)

    assert sp_connections() == ([], [])
    assert nrns[0].synaptic_elements["Axon_ex"]["z_connected"] == 0


def test_mask_excluding_everything_leaves_element_unmatched():
    """The same holds when the mask, rather than the kernel, rules out every candidate."""

    nrns = build_network([(0.0, 0.0), (5.0, 0.0)], [1, 0], [0, 1])
    nest.EnableStructuralPlasticity(mask=nest.CreateMask("circular", {"radius": 1.0}))
    nest.Simulate(5.0)

    assert sp_connections() == ([], [])
    assert nrns[0].synaptic_elements["Axon_ex"]["z_connected"] == 0


@pytest.mark.parametrize("allow_autapses", [True, False])
def test_autapse_restriction_holds_with_mask(allow_autapses):
    """
    ``allow_autapses`` is honoured among the candidates the mask admits.

    The only neuron inside the mask is the source itself, so a connection appears only if autapses
    are allowed.
    """

    nrns = build_network([(0.0, 0.0), (5.0, 0.0)], [1, 0], [1, 1], allow_autapses=allow_autapses)
    nest.EnableStructuralPlasticity(mask=nest.CreateMask("circular", {"radius": 1.0}))
    nest.Simulate(5.0)

    sources, targets = sp_connections()
    if allow_autapses:
        assert sources == [nrns[0].global_id]
        assert targets == [nrns[0].global_id]
    else:
        assert (sources, targets) == ([], [])


def test_multiple_vacant_elements_keep_their_multiplicity():
    """
    Several vacant elements on the same pair of neurons yield several connections.

    Matching removes one postsynaptic element per connection, not the whole neuron.
    """

    nrns = build_network([(0.0, 0.0), (1.0, 0.0)], [3, 0], [0, 3])
    nest.EnableStructuralPlasticity(spatial_kernel=gaussian_kernel(std=1.0))
    nest.Simulate(5.0)

    sources, targets = sp_connections()
    assert sources == [nrns[0].global_id] * 3
    assert targets == [nrns[1].global_id] * 3


def test_spatial_arguments_must_have_the_right_type():
    """The high-level API rejects anything that is not a Parameter or a Mask."""

    build_network([(0.0, 0.0), (1.0, 0.0)], [1, 0], [0, 1])

    with pytest.raises(TypeError, match="spatial_kernel"):
        nest.EnableStructuralPlasticity(spatial_kernel=1.0)

    with pytest.raises(TypeError, match="mask"):
        nest.EnableStructuralPlasticity(mask=1.0)


def test_spatial_matching_requires_positions():
    """Enabling a spatial kernel without spatially distributed neurons is an error."""

    nest.ResetKernel()
    nest.CopyModel("static_synapse", "sp_synapse")
    nest.structural_plasticity_synapses = {
        "sp_syn": {
            "synapse_model": "sp_synapse",
            "pre_synaptic_element": "Axon_ex",
            "post_synaptic_element": "Den_ex",
        }
    }
    nest.Create(
        "iaf_psc_alpha",
        10,
        {"synaptic_elements": {"Axon_ex": {"z": 1.0, "growth_rate": 0.0}, "Den_ex": {"z": 1.0, "growth_rate": 0.0}}},
    )

    with pytest.raises(RuntimeError, match="No neurons with valid positions found"):
        nest.EnableStructuralPlasticity(spatial_kernel=gaussian_kernel(std=1.0))


def test_uniform_matching_without_positions():
    """Without a kernel and without a mask, structural plasticity does not need positions."""

    nest.ResetKernel()
    nest.structural_plasticity_update_interval = 1.0
    nest.CopyModel("static_synapse", "sp_synapse")
    nest.structural_plasticity_synapses = {
        "sp_syn": {
            "synapse_model": "sp_synapse",
            "pre_synaptic_element": "Axon_ex",
            "post_synaptic_element": "Den_ex",
        }
    }
    nest.Create(
        "iaf_psc_alpha",
        10,
        {"synaptic_elements": {"Axon_ex": {"z": 1.0, "growth_rate": 0.0}, "Den_ex": {"z": 1.0, "growth_rate": 0.0}}},
    )

    nest.EnableStructuralPlasticity()
    nest.Simulate(5.0)

    assert len(sp_connections()[0]) == 10
