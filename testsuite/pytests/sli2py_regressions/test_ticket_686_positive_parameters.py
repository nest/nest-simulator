# -*- coding: utf-8 -*-
#
# test_ticket_686_positive_parameters.py
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

import math
from collections.abc import Sequence

try:
    import numpy as np
except ImportError:  # pragma: no cover - numpy is available in the test environment
    np = None

import nest

"""
Regression test for Ticket #686.

Test ported from SLI regression test.
Ensure models with positive-only parameters reject non-positive values and accept larger positive ones.

Author: Hans Ekkehard Plesser, 2013-04-18
"""


SKIPPED_MODELS = {
    "correlation_detector",
    "correlomatrix_detector",
    "correlospinmatrix_detector",
    "siegert_neuron",
}

POSITIVE_KEYS_CACHE: dict[str, tuple[str, ...]] = {}
# Mapping of parameters to their companion parameters that must be updated together.
#
# Some neuron model parameters have companion parameters that must maintain matching
# dimensions. For example, tau_sfa (spike-frequency adaptation time constant) and
# q_sfa (adaptation amplitude) must have the same vector length. Similarly for
# tau_stc and q_stc (spike-triggered current parameters).
#
# When updating these parameters via SetStatus, the model implementations validate
# that companion parameters have matching dimensions. If only one parameter is
# updated without its companion, a dimension mismatch error can occur, which would
# mask the actual positive-value validation we're testing.
#
# The original SLI test did not handle companion keys because it tested models
# before these dimension constraints were strictly enforced, or because the SLI
# implementation handled partial updates differently. The Python port includes
# companion keys to ensure the test focuses on positive-value validation rather
# than dimension mismatches.
#
# When testing a parameter that has companions, we include the companion parameters
# in the update dictionary with their current values to maintain dimension
# consistency and avoid spurious errors.
COMPANION_KEYS = {
    "tau_sfa": ("q_sfa",),
    "tau_stc": ("q_stc",),
}


def _models_to_check():
    return [model for model in nest.node_models if model not in SKIPPED_MODELS]


def _positive_keys(model: str) -> tuple[str, ...]:
    if model not in POSITIVE_KEYS_CACHE:
        defaults = nest.GetDefaults(model)
        keys = tuple(key for key in defaults if key == "C_m" or key.startswith("tau_"))
        POSITIVE_KEYS_CACHE[model] = keys
    return POSITIVE_KEYS_CACHE[model]


def _as_numeric_tuple(value) -> tuple[float, ...]:
    if np is not None and isinstance(value, np.ndarray):
        return tuple(float(v) for v in value.tolist())
    if isinstance(value, Sequence) and not isinstance(value, (str, bytes)):
        return tuple(float(v) for v in value)
    return (float(value),)


def _cast_like(reference, numeric_values: Sequence[float]):
    if np is not None and isinstance(reference, np.ndarray):
        return np.array(numeric_values, dtype=reference.dtype)
    if isinstance(reference, tuple):
        return tuple(numeric_values)
    if isinstance(reference, list):
        return list(numeric_values)
    return numeric_values[0]


def _allclose(values_a: Sequence[float], values_b: Sequence[float], *, abs_tol: float = 1e-12) -> bool:
    return len(values_a) == len(values_b) and all(
        math.isclose(a, b, rel_tol=0.0, abs_tol=abs_tol) for a, b in zip(values_a, values_b)
    )


def _get_companion_updates(neuron, key):
    """Get companion parameter values for a given key to include in updates."""
    return {companion: neuron.get(companion) for companion in COMPANION_KEYS.get(key, ())}


def _test_parameter_update(neuron, key, raw_value, new_values):
    """
    Test updating a parameter with new values, including companion parameters.

    Args:
        neuron: The neuron object
        key: Parameter key to update
        raw_value: Original raw value (used to preserve type)
        new_values: New numeric values as tuple[float, ...]

    Returns: (success: bool, error_message: str | None)
    """
    # Include companion parameters to avoid dimension mismatch errors
    companion_updates = _get_companion_updates(neuron, key)

    try:
        update = {key: _cast_like(raw_value, new_values)}
        update.update(companion_updates)
        nest.SetStatus(neuron, update)
        return True, None
    except nest.kernel.NESTError as err:
        return False, str(err)


def test_ticket_686_rejects_non_positive_values():
    """
    Ensure parameters that must remain positive reject zero and negative assignments.
    """

    models = _models_to_check()
    assert models, "No models available to test (all models may be skipped)"
    failing_cases = []

    for model in models:
        positive_keys = _positive_keys(model)
        if not positive_keys:
            continue

        nest.ResetKernel()
        nest.SetKernelStatus({"dict_miss_is_error": False})

        for key in positive_keys:
            for candidate in (0.0, -1.0):
                if model == "iaf_tum_2000" and candidate == 0.0:
                    continue
                neuron = nest.Create(model)
                raw_value = neuron.get(key)
                # Normalize to tuple[float, ...] to handle scalars, arrays, lists uniformly
                original_values = _as_numeric_tuple(raw_value)
                if not original_values:
                    continue
                # Create invalid values matching the dimension (scalar -> 1 element, vector -> N elements)
                invalid_values = tuple(float(candidate) for _ in original_values)

                success, _ = _test_parameter_update(neuron, key, raw_value, invalid_values)
                if success:
                    # Update succeeded but shouldn't have - this is always a failure
                    # because the user might think they've set the parameter when they haven't
                    current_values = _as_numeric_tuple(neuron.get(key))
                    if not _allclose(current_values, original_values):
                        # Value actually changed - neuron accepted the invalid value
                        failing_cases.append((model, key, f"accepted_non_positive_{candidate}"))
                    else:
                        # Value didn't change - neuron silently ignored the invalid value
                        failing_cases.append((model, key, f"silently_ignored_non_positive_{candidate}"))
                else:
                    # Exception raised as expected - verify value unchanged
                    current_values = _as_numeric_tuple(neuron.get(key))
                    if not _allclose(current_values, original_values):
                        failing_cases.append((model, key, "value_changed_after_exception"))

    assert not failing_cases, f"Models not rejecting non-positive values: {failing_cases}"


def test_ticket_686_accepts_positive_assignments():
    """
    Ensure the same parameters accept updated positive values.
    """

    models = _models_to_check()
    assert models, "No models available to test (all models may be skipped)"
    failing_cases = []

    for model in models:
        if model == "iaf_psc_exp_ps_lossless":
            continue

        positive_keys = _positive_keys(model)
        if not positive_keys:
            continue

        nest.ResetKernel()
        nest.SetKernelStatus({"dict_miss_is_error": False})

        for key in positive_keys:
            neuron = nest.Create(model)
            raw_value = neuron.get(key)
            # Normalize to tuple[float, ...] to handle scalars, arrays, lists uniformly
            original_values = _as_numeric_tuple(raw_value)
            if not original_values:
                continue
            # Create new positive values matching the dimension (scalar -> 1 element, vector -> N elements)
            new_values = tuple(v + 1.0 for v in original_values)

            success, error_msg = _test_parameter_update(neuron, key, raw_value, new_values)
            if success:
                # Verify the value was actually updated
                updated_values = _as_numeric_tuple(neuron.get(key))
                if not _allclose(updated_values, new_values):
                    failing_cases.append((model, key, "value_not_updated"))
            else:
                failing_cases.append((model, key, f"raised_exception:{error_msg}"))

    assert not failing_cases, f"Models not accepting positive updates: {failing_cases}"
