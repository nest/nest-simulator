# -*- coding: utf-8 -*-
#
# test_iaf_psp_normalized.py
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
Name: testsuite::test_iaf_psp_normalized - check if PSP can be normalized

Synopsis: (test_iaf_psp_normalized) run -> compare response with desired outcome

Description:

The script computes the peak location of the PSP analytically for a
neuron model with an alpha-shaped postsynaptic current (PSC) [1]. In case
the GNU Scientific Library (GSL) is not present the peak location is
found by searching for the root of the derivative of the PSP. We then
compute the peak value for a PSC with unit amplitude and show how the
synaptic weight can be adjusted to cause a PSP of a specific
amplitude. Finally, we check whether the simulation indeed generates
a PSP of the desired amplitude.

In application code the test for the availability of the GSL is not
necessary because NEST has a built in version of the LambertWm1 which
automatically replaces the GSL function if required. This removes the
need to specify the derivative of the function of interest, here the
PSP, in application code.  A further alternative is used in
test_lambertw where knowledge of the range of values of the
non-principal branch of the Lambert-W function [-1,-\\infty) is
exploited to find the inverse of x*exp(x) by bisectioning.

Test ported from SLI unittest.

References:
  [1] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
      systems with applications to neuronal modeling. Biologial Cybernetics
      81:381-402.
  [2] Galassi, M., Davies, J., Theiler, J., Gough, B., Jungman, G., Booth, M.,
      & Rossi, F. (2006). GNU Scientific Library Reference Manual (2nd Ed.).
      Network Theory Limited.

Author:  July 2009, Diesmann
SeeAlso: testsuite::test_iaf_psp_peak, testsuite::test_iaf_psp, testsuite::test_lambertw, LambertWm1
"""

import nest
import numpy as np
import pytest
from scipy.optimize import brentq
from scipy.special import lambertw


def test_iaf_psp_normalized():
    """
    Test that PSP can be normalized to a desired amplitude by adjusting synaptic weight.
    """
    delay = 1.0  # in ms
    dt = 0.001  # resolution in ms
    u = 1.0  # requested PSP size in mV

    # Parameters of the Brunel network examples
    P = {
        "tau_m": 20.0,  # membrane time constant in ms
        "tau_syn": 0.5,  # synaptic time constant in ms
        "C_m": 1.0,  # membrane capacity in pF
        "E_L": 0.0,
        "V_reset": 0.0,
        "V_th": 15.0,
        "V_m": 0.0,
    }

    # E is the PSC amplitude - for unit amplitude, E=1.0
    # In SLI, E is used but not explicitly defined, suggesting it's a global constant
    # We set it to 1.0 for unit PSC amplitude
    E = 1.0  # unit amplitude (pA)

    def psp(t):
        """
        Postsynaptic potential function for alpha-shaped PSC.

        For an alpha-shaped PSC with amplitude E and time constant tau_syn,
        the PSP response of an LIF neuron is computed.
        This matches the SLI formula exactly.
        """
        tau_m = P["tau_m"]
        tau_syn = P["tau_syn"]
        C_m = P["C_m"]
        # PSP formula from SLI: E/tau_syn * 1/C_m * ( (exp(-t/tau_m)-exp(-t/tau_syn))/(1/tau_syn
        # - 1/tau_m)^2 - t*exp(-t/tau_syn)/(1/tau_syn - 1/tau_m) )
        denom = 1.0 / tau_syn - 1.0 / tau_m
        return (
            E
            / tau_syn
            * 1.0
            / C_m
            * ((np.exp(-t / tau_m) - np.exp(-t / tau_syn)) / denom**2 - t * np.exp(-t / tau_syn) / denom)
        )

    def dpsp(t):
        """Derivative of the postsynaptic potential."""
        tau_m = P["tau_m"]
        tau_syn = P["tau_syn"]
        C_m = P["C_m"]
        return (
            E
            / tau_syn
            * 1.0
            / C_m
            * (
                (-1.0 / tau_m * np.exp(-t / tau_m) + 1.0 / tau_syn * np.exp(-t / tau_syn))
                / (1.0 / tau_syn - 1.0 / tau_m) ** 2
                - (np.exp(-t / tau_syn) - 1.0 / tau_syn * t * np.exp(-t / tau_syn)) / (1.0 / tau_syn - 1.0 / tau_m)
            )
        )

    # Find peak time using numerical root finding (more reliable than Lambert W)
    # The SLI code uses Lambert W if GSL is available, otherwise uses FindRoot
    # We'll use numerical root finding to match the SLI behavior more closely
    tau_m = P["tau_m"]
    tau_syn = P["tau_syn"]

    # Numerical solution: find root of derivative
    # Note: dpsp(0.0) = 0.0 mathematically (derivative is zero at t=0)
    # So we use a small positive value for the lower bound
    # Check that dpsp has opposite signs at the bounds
    t_lower = 1e-6  # Small positive value to avoid t=0 where derivative is exactly 0
    assert dpsp(t_lower) > 0, f"dpsp({t_lower})={dpsp(t_lower)} should be positive"
    assert dpsp(5.0) < 0, f"dpsp(5.0)={dpsp(5.0)} should be negative"

    t_peak = brentq(dpsp, t_lower, 5.0, xtol=1e-11)

    # Verify that dpsp(t_peak) is close to zero
    assert abs(dpsp(t_peak)) < 1e-6, f"dpsp(t_peak)={dpsp(t_peak)} should be close to zero"

    # Compute peak value for unit PSC amplitude (E=1.0)
    # Debug: check PSP values at different times
    psp_at_01 = psp(0.1)
    psp_at_1 = psp(1.0)
    psp_peak_unit = psp(t_peak)  # PSP peak for unit PSC amplitude

    # Debug: check if psp_peak_unit is valid
    # The PSP peak should be positive for a positive PSC amplitude
    # If psp_peak_unit is zero or negative, there's an issue with the formula
    if psp_peak_unit <= 0:
        raise AssertionError(
            f"psp_peak_unit={psp_peak_unit} should be positive, "
            f"t_peak={t_peak}, psp(0.1)={psp_at_01}, psp(1.0)={psp_at_1}, "
            f"E={E}, tau_m={P['tau_m']}, tau_syn={P['tau_syn']}, C_m={P['C_m']}"
        )

    # f is the weight required for a PSP with unit amplitude (1.0 mV)
    # Since PSP scales linearly with weight, we need: w * psp_peak_unit = u
    # Therefore: w = u / psp_peak_unit
    # This matches SLI: t psp inv -> f, then u f mul -> w
    # Note: In SLI, E is used in the PSP formula but not explicitly defined.
    # The PSP formula assumes E=1.0 for unit PSC amplitude.
    # However, there might be a scaling factor between the analytical PSP and NEST's simulation.
    f = 1.0 / psp_peak_unit  # weight per unit PSP amplitude
    w = u * f  # weight for desired PSP amplitude u

    # Debug: print intermediate values
    print(f"DEBUG: t_peak={t_peak}, psp_peak_unit={psp_peak_unit}, f={f}, w={w}, E={E}")

    # Ensure weight is positive and reasonable
    assert (
        w > 0 and w < 1e10
    ), f"Weight w={w} should be positive and reasonable, psp_peak_unit={psp_peak_unit}, t_peak={t_peak}, f={f}"

    # Now simulate and verify
    nest.ResetKernel()
    nest.set(resolution=dt, local_num_threads=1)

    sg = nest.Create("spike_generator")
    sg.set(
        {
            "precise_times": False,
            "origin": 0.0,  # in ms
            "spike_times": [2.0],  # in ms
            "start": 1.0,  # in ms
            "stop": 3.0,  # in ms
        }
    )

    # Verify spike generator is set up correctly
    assert sg.get("spike_times") == [2.0]

    # Create neuron with parameters
    neuron_params = P.copy()
    neuron_params["tau_syn_ex"] = P["tau_syn"]
    neuron_params["tau_syn_in"] = P["tau_syn"]
    del neuron_params["tau_syn"]

    neuron = nest.Create("iaf_psc_alpha")
    neuron.set(neuron_params)

    vm = nest.Create("voltmeter", params={"time_in_steps": True, "interval": dt})

    # Connect spike generator to neuron with weight and delay
    nest.Connect(sg, neuron, syn_spec={"synapse_model": "static_synapse", "weight": w, "delay": delay})
    # Connect voltmeter to neuron - match SLI code which connects without syn_spec
    nest.Connect(vm, neuron)

    nest.Simulate(7.0)

    # Get maximum voltage from voltmeter events
    # Use the same pattern as other tests
    V_m_values = vm.get("events", "V_m")
    assert len(V_m_values) > 0, "Voltmeter should have recorded some data"
    V_max = np.max(V_m_values)

    # Debug: print values to understand the discrepancy
    # If V_max is approximately e (2.718...), there might be a scaling issue
    print(f"DEBUG: V_max={V_max}, expected={u}, psp_peak_unit={psp_peak_unit}, w={w}, ratio={V_max/u}")

    # Check that maximum voltage is close to desired amplitude
    # The SLI test uses 1e-6 tolerance
    # Note: V_max is measured from E_L (which is 0.0), so it should equal the PSP peak
    # If V_max/u is approximately e, we might need to adjust the weight calculation
    assert (
        abs(V_max - u) < 1e-6
    ), f"V_max={V_max}, expected={u}, difference={abs(V_max - u)}, psp_peak_unit={psp_peak_unit}, w={w}"
