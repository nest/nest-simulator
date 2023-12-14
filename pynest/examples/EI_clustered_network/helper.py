# -*- coding: utf-8 -*-
#
# helper.py
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

"""PyNEST EI-clustered network: Helper Functions
-------------------------------------------

Helper functions to calculate synaptic weights and first passage times for exponential synapses to construct
random balanced networks.
"""

import numpy as np

small = 1e-10


def max_psp_exp(tau_m, tau_syn, c_m=1., e_l=0.):
    """
    Calculates the maximum psp amplitude for exponential synapses and unit J.
    :param tau_m: Membrane time constant [ms]
    :param tau_syn: Synapse time constant  [ms]
    :param c_m: Membrane capacity [pF]
    :param e_l: Resting potential [mV]
    :return: maximum psp amplitude
    """
    tmax = np.log(tau_syn / tau_m) / (1 / tau_m - 1 / tau_syn)
    B = tau_m * tau_syn / c_m / (tau_syn - tau_m)
    return (e_l - B) * np.exp(-tmax / tau_m) + B * np.exp(-tmax / tau_syn)


def calc_js(params):
    """
    calculates the synaptic weights for exponential synapses before clustering
    :param params: dictionary of network parameters
    :return: synaptic weights 2x2 matrix
    """
    N_E = params.get('N_E')  # excitatory units
    N_I = params.get('N_I')  # inhibitory units
    N = N_E + N_I  # total units
    ps = params.get('ps')  # connection probs
    ge = params.get('ge')
    gi = params.get('gi')
    gie = params.get('gie')
    V_th_E = params.get('V_th_E')  # threshold voltage
    V_th_I = params.get('V_th_I')
    tau_E = params.get('tau_E')
    tau_I = params.get('tau_I')
    E_L = params.get('E_L')
    neuron_type = params.get('neuron_type')
    if ('iaf_psc_exp' in neuron_type) or ('gif_psc_exp' in neuron_type):
        tau_syn_ex = params.get('tau_syn_ex')
        tau_syn_in = params.get('tau_syn_in')
        amp_EE = max_psp_exp(tau_E, tau_syn_ex)
        amp_EI = max_psp_exp(tau_E, tau_syn_in)
        amp_IE = max_psp_exp(tau_I, tau_syn_ex)
        amp_II = max_psp_exp(tau_I, tau_syn_in)
    else:
        amp_EE = 1.
        amp_EI = 1.
        amp_IE = 1.
        amp_II = 1.

    js = np.zeros((2, 2))
    K_EE = N_E * ps[0, 0]
    js[0, 0] = (V_th_E - E_L) * (K_EE ** -0.5) * N ** 0.5 / amp_EE
    js[0, 1] = -ge * js[0, 0] * ps[0, 0] * N_E * amp_EE / (ps[0, 1] * N_I * amp_EI)
    K_IE = N_E * ps[1, 0]
    js[1, 0] = gie * (V_th_I - E_L) * (K_IE ** -0.5) * N ** 0.5 / amp_IE
    js[1, 1] = -gi * js[1, 0] * ps[1, 0] * N_E * amp_IE / (ps[1, 1] * N_I * amp_II)
    return js


def fpt(tau_m, e_l, i_e, c_m, vtarget, vstart):
    """
    calculate first pasage time between Vstart and Vtarget.
    :param tau_m: Membrane time constant [ms]
    :param e_l: Resting potential [mV]
    :param i_e: external current [pA]
    :param c_m: Membrane capacity [pF]
    :param vtarget: target voltage [mV]
    :param vstart: start voltage [mV]
    :return: first passage time [ms]
    """
    inner = (vtarget - e_l - tau_m * i_e / c_m) / (vstart - e_l - tau_m * i_e / c_m + small)
    if inner < 0:
        return np.nan
    else:
        return -tau_m * np.log(inner)

def v_fpt(tau_m, e_l, i_e, c_m, ttarget, vtarget, t_ref):
    """
    Calculate the initial voltage required to obtain a certain first passage time.
    :param tau_m: Membrane time constant [ms]
    :param e_l: Resting potential [mV]
    :param i_e: external current [pA]
    :param c_m: Membrane capacity [pF]
    :param ttarget: target first passage time [ms]
    :param vtarget: target voltage [mV]
    :param t_ref: refractory period [ms]
    :return: initial voltage [mV]
    """
    return (vtarget - e_l - tau_m * i_e / c_m) * np.exp(ttarget / tau_m) + e_l + tau_m * i_e / c_m
