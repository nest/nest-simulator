# -*- coding: utf-8 -*-
#
# test_aeif_lsodar.py
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

import numpy as np
import unittest
import nest

"""
Comparing the new implementations the aeif models to the reference solution
obrained using the LSODAR solver (see
``doc/model_details/aeif_models_implementation.ipynb``).

The reference solution is stored in ``test_aeif_psc_lsodar.dat`` and was
generated using the same dictionary of parameters, the data is then downsampled
to keep one value every 0.01 ms and compare with the NEST simulation.

The new implementation binds V_m to be smaller than 0.

Rationale of the test: all models should be close to the reference LSODAR when
submitted to the same excitatory current.

Details:
  The models are compared and we assess that the difference is smaller than a
  given tolerance.
"""

HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")


# --------------------------------------------------------------------------- #
#  Tolerances
# -------------------------
#

# for the state variables (compare LSODAR and NEST implementations)
tol_compare_V = 2e-3  # higher for V because of the divergence at spike time
tol_compare_w = 5e-5  # better for w


# --------------------------------------------------------------------------- #
#  Individual dynamics
# -------------------------
#

models = [
  "aeif_cond_alpha",
  # "aeif_cond_alpha_RK5",
  "aeif_cond_exp",
  # "aeif_cond_alpha_multisynapse"
]

num_models = len(models)

# parameters with which the LSODAR reference solution was generated
aeif_param = {
    'V_reset': -58.,
    'V_peak': 0.0,
    'V_th': -50.,
    'I_e': 420.,
    'g_L': 11.,
    'tau_w': 300.,
    'E_L': -70.,
    'Delta_T': 2.,
    'a': 3.,
    'b': 0.,
    'C_m': 200.,
    'V_m': -70.,
    'w': 5.
}


# --------------------------------------------------------------------------- #
#  Comparison function
# -------------------------
#

def _find_idx_nearest(array, values):
    '''
    Find the indices of the nearest elements of `values` in `array`.
    Both ``array`` and ``values`` should be ``numpy.array``s and `array` MUST
    be sorted in increasing order.

    Parameters
    ----------
    array : reference list or np.ndarray
    values : double, list or array of values to find in `array`

    Returns
    -------
    idx : int or array representing the index of the closest value in `array`
    '''
    idx = np.searchsorted(array, values, side="left")  # get the interval
    # return the index of the closest
    if isinstance(values, float) or isinstance(values, int):
        if idx == len(array):
            return idx - 1
        else:
            return idx - (np.abs(values - array[idx - 1]) <
                          np.abs(values - array[idx]))
    else:
        # find where it is idx_max+1
        overflow = (idx == len(array))
        idx[overflow] -= 1
        # for the others, find the nearest
        tmp = idx[~overflow]
        idx[~overflow] = tmp - (np.abs(values[~overflow] - array[tmp - 1]) <
                                np.abs(values[~overflow] - array[tmp]))
        return idx


def _interpolate_lsodar(nest_times, lsodar):
    '''
    Interpolate the LSODAR data to get the value at the times computed by NEST.

    Parameters
    ----------
    nest_times : array of doubles
        Times on NEST grid, at which the state vaiables were measured.
    lsodar : array of doubles
        Loaded LSODAR data.

    Returns
    -------
    nest_indices : array of ints
        Indices of the NEST data to compare with the LSODAR data.
    lsodar_V : array of doubles
        Interpolated data for the potential.
    lsodar_w : array of doubles
        Interpolated data for the adaptation variable.
    '''
    lsodar_t = lsodar[0, :]  # times
    Vs = lsodar[1, :]  # V_m
    ws = lsodar[2, :]  # w
    indices = _find_idx_nearest(nest_times, lsodar_t)
    # step of lsodar can be too small, keep only the closest to the nest_times
    # when same value over a whole range in indices
    idx_change = np.nonzero(np.diff(indices))[0] + 1
    lsodar_V, lsodar_w, nest_indices = [], [], []
    idx_tmp = 0
    for idx in idx_change:
        # interpolate the value at the exact nest_time from closest lsodar_t
        t_int, V_int, w_int = 0., 0., 0.
        if idx - idx_tmp > 1:
            t_int = nest_times[indices[idx]]
            closest = idx_tmp + np.argmin(np.abs(t_int -
                                                 lsodar_t[idx_tmp:idx]))
            if lsodar_t[closest] < t_int:
                Dt = lsodar_t[closest + 1] - lsodar_t[closest]
                dt = t_int - lsodar_t[closest]
                V_int = Vs[closest] + dt * (Vs[closest + 1] - Vs[closest]) / Dt
                w_int = ws[closest] + dt * (ws[closest + 1] - ws[closest]) / Dt
            else:
                Dt = lsodar_t[closest] - lsodar_t[closest - 1]
                dt = lsodar_t[closest] - t_int
                V_int = (Vs[closest - 1] +
                         dt * (Vs[closest] - Vs[closest - 1]) / Dt)
                w_int = (ws[closest - 1] +
                         dt * (ws[closest] - ws[closest - 1]) / Dt)
        else:
            t_int = nest_times[indices[idx]]
            if lsodar_t[idx] < t_int and idx + 1 < len(lsodar_t):
                Dt = lsodar_t[idx + 1] - lsodar_t[idx]
                dt = t_int - lsodar_t[idx]
                V_int = Vs[idx] + dt * (Vs[idx + 1] - Vs[idx]) / Dt
                w_int = ws[idx] + dt * (ws[idx + 1] - ws[idx]) / Dt
            elif idx < len(lsodar_t):
                Dt = lsodar_t[idx] - lsodar_t[idx - 1]
                dt = t_int - lsodar_t[idx - 1]
                V_int = Vs[idx - 1] + dt * (Vs[idx] - Vs[idx - 1]) / Dt
                w_int = ws[idx - 1] + dt * (ws[idx] - ws[idx - 1]) / Dt
            else:
                break
        # ignore the values of V at spike times
        if V_int < (aeif_param["V_th"] - aeif_param["V_peak"]) / 2.:
            lsodar_V.append(V_int)
            lsodar_w.append(w_int)
            nest_indices.append(indices[idx])
        idx_tmp = idx
    return nest_indices, lsodar_V, lsodar_w


def compare_nest_lsodar():
    '''
    Compare models to the LSODAR implementation.
    '''
    simtime = 100.1
    resol = 0.01
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": resol})

    # get lsodar reference
    lsodar = np.loadtxt('test_aeif_data_lsodar.dat').T

    # create the neurons and devices
    lst_neurons = [nest.Create(model, params=aeif_param) for model in models]
    multimeters = [nest.Create("multimeter") for _ in range(num_models)]
    # connect them and simulate
    for i, mm in enumerate(multimeters):
        nest.SetStatus(mm, {"interval": resol, "record_from": ["V_m", "w"]})
        nest.Connect(mm, lst_neurons[i])
    nest.Simulate(simtime)

    # compute the relative differences: interpolate LSODAR to match NEST times
    nest_times = nest.GetStatus(multimeters[0], "events")[0]["times"]
    nest_indices, lsodar_V, lsodar_w = _interpolate_lsodar(nest_times, lsodar)
    rds = []
    for i, mm in enumerate(multimeters):
        Vs = nest.GetStatus(mm, "events")[0]["V_m"][nest_indices]
        rds.append(np.average(np.sqrt(
          np.square((Vs - lsodar_V))) / np.abs(Vs)))
        ws = nest.GetStatus(mm, "events")[0]["w"][nest_indices]
        rds.append(np.average(np.sqrt(np.square((ws - lsodar_w))) / ws))
    for rel_diff_V, rel_diff_w in zip(rds[::2], rds[1::2]):
        assert(rel_diff_V < tol_compare_V)
        assert(rel_diff_w < tol_compare_w)


# --------------------------------------------------------------------------- #
#  Run the comparisons
# ------------------------
#

@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
def run_tests():
    compare_nest_lsodar()

if __name__ == '__main__':
    run_tests()
