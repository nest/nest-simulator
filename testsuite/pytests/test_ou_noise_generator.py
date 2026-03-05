# -*- coding: utf-8 -*-
#
# test_ou_noise_generator.py
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
Tests parameter setting and statistical correctness for one application.
"""

import math

import nest
import numpy as np
import pytest
from scipy.signal import fftconvolve


@pytest.fixture
def prepare_kernel():
    nest.ResetKernel()
    nest.resolution = 0.1


def burn_in_start(dt, tau, k=10):
    """Return number of steps to discard for a k*tau burn-in."""
    return int(round((k * tau) / dt))


def test_ou_noise_generator_set_parameters(prepare_kernel):
    params = {"mean": 210.0, "std": 60.0, "dt": 0.1}

    oung1 = nest.Create("ou_noise_generator")
    oung1.set(params)

    nest.SetDefaults("ou_noise_generator", params)

    oung2 = nest.Create("ou_noise_generator")
    assert oung1.get(params) == oung2.get(params)


def test_ou_noise_generator_incorrect_noise_dt(prepare_kernel):
    with pytest.raises(nest.kernel.NESTError, match="StepMultipleRequired"):
        nest.Create("ou_noise_generator", {"dt": 0.25})


def test_ou_noise_mean_and_variance(prepare_kernel):
    # run for resolution dt=0.1 project to iaf_psc_alpha.
    # create 100 repetitions of 1000ms simulations

    oung = nest.Create("ou_noise_generator", {"mean": 0.0, "std": 60.0, "tau": 1.0, "dt": 0.1})
    neuron = nest.Create("iaf_psc_alpha")

    # we need to connect to a neuron otherwise the generator does not generate
    nest.Connect(oung, neuron)
    mm = nest.Create("multimeter", 1, {"record_from": ["I"], "interval": 0.1})
    nest.Connect(mm, oung, syn_spec={"weight": 1})

    # Simulate for 100 times
    n_sims = 100
    ou_current = np.empty(n_sims)
    for i in range(n_sims):
        nest.Simulate(1000.0)
        ou_current[i] = mm.get("events")["I"][-1]

    curr_mean = np.mean(ou_current)
    curr_var = np.var(ou_current)
    expected_curr_mean = oung.mean
    expected_curr_var = oung.std**2

    # require mean within 3 std dev, std dev within 0.2 std dev
    assert np.abs(curr_mean - expected_curr_mean) < 3 * oung.std
    assert np.abs(curr_var - expected_curr_var) < 0.2 * curr_var


def test_ou_noise_generator_autocorrelation(prepare_kernel):
    # verify lag-1 autocorr = exp(-dt/tau)
    dt = 0.1
    tau = 1.0

    oung = nest.Create(
        "ou_noise_generator",
        {"mean": 0.0, "std": 60.0, "tau": tau, "dt": nest.resolution},
    )
    neuron = nest.Create("iaf_psc_alpha")

    # we need to connect to a neuron otherwise the generator does not generate
    nest.Connect(oung, neuron)
    mm = nest.Create("multimeter", 1, {"record_from": ["I"], "interval": dt})
    nest.Connect(mm, oung, syn_spec={"weight": 1})
    nest.Simulate(10000.0)
    ou_current = mm.get("events")["I"]

    # drop first k*tau
    cutoff = burn_in_start(dt=dt, tau=tau)
    x = ou_current[cutoff:]
    x -= x.mean()

    # empirical lag-1 autocorrelation
    emp_ac1 = np.dot(x[:-1], x[1:]) / ((len(x) - 1) * x.var())
    # theoretical lag-1: exp(-dt/tau)
    theor_ac1 = np.exp(-dt / tau)

    assert abs(emp_ac1 - theor_ac1) < 0.05, f"autocorr {emp_ac1:.3f} vs theoretical {theor_ac1:.3f}"


def test_cross_correlation(prepare_kernel):
    # two neurons driven by the same OU noise should remain uncorrelated
    dt, tau = 0.1, 1.0

    ou = nest.Create("ou_noise_generator", {"mean": 0.0, "std": 50.0, "tau": tau, "dt": dt})
    neurons = nest.Create("iaf_psc_alpha", 2, {"E_L": 0.0, "V_th": 1e9, "C_m": 1.0, "tau_m": tau})
    nest.Connect(ou, neurons)

    mm = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": dt})
    nest.Connect(mm, neurons)

    simtime = 50000.0
    nest.Simulate(simtime)

    ev = mm.get("events")
    senders = np.asarray(ev["senders"], int)
    times = np.asarray(ev["times"])
    vms = np.asarray(ev["V_m"])

    # build time array
    n_steps = int(round(simtime / dt)) + 1
    V = np.zeros((n_steps, 2))
    for t, s, v in zip(times, senders, vms):
        idx = int(round(t / dt))
        col = neurons.index(s)
        V[idx, col] = v

    # discard burn-in
    start = burn_in_start(dt, tau)
    X1 = V[start:, 0] - V[start:, 0].mean()
    X2 = V[start:, 1] - V[start:, 1].mean()

    # cross-corr via FFT
    corr = fftconvolve(X1, X2[::-1], mode="full")
    corr /= np.sqrt(np.dot(X1, X1) * np.dot(X2, X2))
    mid = corr.size // 2

    # maximum absolute cross-corr should be near zero
    assert np.max(np.abs(corr[mid:])) < 0.05


def _run_ou_with_on_off(
    mean=0.0,
    std=10.0,
    tau=100.0,
    dt=1.0,
    start_on=200.0,
    stop_on=500.0,
    section_ms=1000.0,
    n_sections=40,
):
    """
    Run an ou_noise_generator that is active only in [start_on, stop_on)
    of each section of length section_ms, for n_sections sections.

    Returns
    -------
    times   : np.ndarray
    currents: np.ndarray
    """
    nest.resolution = dt

    neuron = nest.Create("iaf_psc_alpha")
    ou = nest.Create(
        "ou_noise_generator",
        params={
            "mean": mean,
            "std": std,
            "tau": tau,
            "dt": dt,
            "start": start_on,
            "stop": stop_on,
        },
    )
    nest.Connect(ou, neuron)

    mm = nest.Create(
        "multimeter",
        params={
            "record_from": ["I"],
            "interval": dt,
        },
    )
    nest.Connect(mm, ou, syn_spec={"weight": 1.0})

    for k in range(n_sections):
        nest.SetStatus(
            ou,
            {
                "origin": float(k * section_ms),
                "mean": mean,
                "std": std,
                "tau": tau,
                "dt": dt,
            },
        )
        nest.Simulate(section_ms)

    ev = mm.get("events")
    times = np.asarray(ev["times"])
    currents = np.asarray(ev["I"])

    assert times.size > 0, "No samples recorded by multimeter."
    return times, currents


def test_ou_noise_generator_zero_output_when_inactive(prepare_kernel):
    """
    When the generator is outside [start, stop), the recorded currents
    must be zero (up to numerical tolerance), while inside the
    active window the OU process must exhibit non-zero variance.
    """
    mean, std, tau, dt = 0.0, 10.0, 100.0, 1.0
    start_on, stop_on = 200.0, 500.0
    section_ms = 1000.0
    n_sections = 40

    times, currents = _run_ou_with_on_off(
        mean=mean,
        std=std,
        tau=tau,
        dt=dt,
        start_on=start_on,
        stop_on=stop_on,
        section_ms=section_ms,
        n_sections=n_sections,
    )

    pos = times % section_ms
    on_mask = (pos >= start_on) & (pos < stop_on)
    off_mask = ~on_mask

    # We need both ON and OFF samples for this test
    assert np.any(on_mask), "No ON-window samples recorded."
    assert np.any(off_mask), "No OFF-window samples recorded."

    # OFF samples should be exactly zero
    off_vals = currents[off_mask]
    assert np.allclose(
        off_vals, 0.0, atol=1e-12
    ), f"Inactive samples are not zero-gated (max |I_off| = {np.max(np.abs(off_vals))})."

    # ON samples must show variance.
    on_vals = currents[on_mask]
    assert np.std(on_vals) > 0.0, "Active-window output has zero variance;"


def test_ou_noise_generator_jump_continuity_across_gaps(prepare_kernel):
    """
    Across inactive gaps, the OU state should evolve as an OU process
    with time constant tau. We test this by comparing the empirical
    correlation between the last ON sample in section s and the first
    ON sample in section s+1 to the theoretical value exp(-gap/tau).
    """
    mean, std, tau, dt = 0.0, 10.0, 100.0, 1.0
    start_on, stop_on = 100.0, 200.0
    section_ms = 400.0
    n_sections = 60

    times, currents = _run_ou_with_on_off(
        mean=mean,
        std=std,
        tau=tau,
        dt=dt,
        start_on=start_on,
        stop_on=stop_on,
        section_ms=section_ms,
        n_sections=n_sections,
    )

    pos = times % section_ms
    on_mask = (pos >= start_on) & (pos < stop_on)
    sections = (times // section_ms).astype(int)

    first_vals = []
    last_vals = []

    for s in range(sections.min(), sections.max() + 1):
        m = (sections == s) & on_mask
        Is = currents[m]
        if Is.size == 0:
            continue
        first_vals.append(Is[0])
        last_vals.append(Is[-1])

    first_vals = np.asarray(first_vals)
    last_vals = np.asarray(last_vals)

    assert first_vals.size >= 2, "Not enough ON-window sections."

    last_before = last_vals[:-1]
    first_after = first_vals[1:]

    assert np.std(last_before) > 0.0
    assert np.std(first_after) > 0.0

    # Time between the last ON sample in section s and first ON sample in s+1
    gap_ms = section_ms - (stop_on - dt) + start_on
    theor_corr = math.exp(-gap_ms / tau)

    emp_corr = np.corrcoef(last_before, first_after)[0, 1]

    # Allow for stochastic variability;
    assert abs(emp_corr - theor_corr) < 0.2, (
        f"OU continuity mismatch across gaps: "
        f"empirical corr={emp_corr:.3f}, theoretical={theor_corr:.3f}, gap={gap_ms:.1f} ms"
    )
