# -*- coding: utf-8 -*-
#
# test_tsodyks_synapse.py
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
import numpy as np
import pytest


@pytest.mark.parametrize("synapse_model", ["tsodyks_synapse", "tsodyks_synapse_hom"])
class TestTsodyksSynapse:
    """Test of Tsodyks synapse model

    Test Tsodyks short term plasticity depressing synapses according to:
    'Neural Networks with Dynamic Synapses', Misha Tsodyks, Klaus Pawelzik, Henry Markram
    Neural computation 10, 821--853 (1998)
    """

    def test_tsodyks_depressing(self, synapse_model):
        """Test depression dynamics. Reproduces figure 1 A"""
        h = 0.1

        # Model parameters
        Tau = 40.0  # membrane time constant
        Theta = 15.0  # threshold
        E_L = 0.0  # reset potential of Vm
        R = 1.0
        C = Tau / R  # Tau [ms] / 1.0 GOhm in NEST units
        TauR = 2.0  # refractory time
        Tau_psc = 3.0  # time constant of PSC (=Tau_inact)
        Tau_rec = 800.0  # recovery time
        Tau_fac = 0.0  # facilitation time: 0 == no facilitation
        U = 0.5  # facilitation parameter U
        A = 250.0  # PSC weight in pA

        input_train = np.arange(98.0, 1048.1, 50.0)  # first spike [ms]  # last spike [ms]  # interspike interval [ms]

        T_sim = 1200.0  # simulation time [ms]

        nest.ResetKernel()
        nest.resolution = h

        sg = nest.Create("spike_generator", {"spike_times": input_train})
        pn = nest.Create("parrot_neuron")

        neuron = nest.Create(
            "iaf_psc_exp_htum",
            params={
                "tau_m": Tau,
                "t_ref_tot": TauR,
                "t_ref_abs": TauR,
                "tau_syn_ex": Tau_psc,
                "tau_syn_in": Tau_psc,
                "C_m": C,
                "V_th": Theta,
                "V_reset": E_L,
                "E_L": E_L,
                "V_m": E_L,
            },
        )

        vm = nest.Create("voltmeter")
        vm.interval = 25.0

        nest.SetDefaults(
            synapse_model,
            {
                "tau_psc": Tau_psc,
                "tau_rec": Tau_rec,
                "tau_fac": Tau_fac,
                "U": U,
                "delay": 0.1,
                "weight": A,
                "u": 0.0,
                "x": 1.0,
            },
        )

        nest.Connect(sg, pn)  # pn required, devices cannot project to plastic synapses
        nest.Connect(pn, neuron, syn_spec={"synapse_model": synapse_model})
        nest.Connect(vm, neuron)

        nest.Simulate(T_sim)

        # compare results to expected voltage trace
        times_vm_expected = np.array(
            [
                [25, 0],
                [50, 0],
                [75, 0],
                [100, 2.401350000000000e00],
                [125, 5.302440000000000e00],
                [150, 4.108330000000000e00],
                [175, 4.322170000000000e00],
                [200, 3.053390000000000e00],
                [225, 2.871240000000000e00],
                [250, 2.028640000000000e00],
                [275, 1.908020000000000e00],
                [300, 1.396960000000000e00],
                [325, 1.375850000000000e00],
                [350, 1.057780000000000e00],
                [375, 1.103490000000000e00],
                [400, 8.865690000000001e-01],
                [425, 9.693530000000000e-01],
                [450, 8.028760000000000e-01],
                [475, 9.046710000000000e-01],
                [500, 7.626880000000000e-01],
                [525, 8.738550000000000e-01],
                [550, 7.435880000000000e-01],
                [575, 8.592780000000000e-01],
                [600, 7.345670000000000e-01],
                [625, 8.524119999999999e-01],
                [650, 7.303210000000000e-01],
                [675, 8.491860000000000e-01],
                [700, 7.283280000000000e-01],
                [725, 8.476730000000000e-01],
                [750, 7.273930000000000e-01],
                [775, 8.469640000000001e-01],
                [800, 7.269550000000000e-01],
                [825, 8.466320000000001e-01],
                [850, 7.267500000000000e-01],
                [875, 8.464760000000000e-01],
                [900, 7.266540000000000e-01],
                [925, 8.464030000000000e-01],
                [950, 7.266089999999999e-01],
                [975, 8.463690000000000e-01],
                [1000, 7.265880000000000e-01],
                [1025, 8.463530000000000e-01],
                [1050, 7.265779999999999e-01],
                [1075, 8.463460000000000e-01],
                [1100, 4.531260000000000e-01],
                [1125, 2.425410000000000e-01],
                [1150, 1.298230000000000e-01],
                [1175, 6.948920000000000e-02],
            ]
        )

        times_vm_sim = np.vstack([vm.get("events")["times"], vm.get("events")["V_m"]]).T

        # test uses large atol due to finite precision of reference timeseries
        np.testing.assert_allclose(times_vm_sim, times_vm_expected, atol=1e-5)

    def test_tsodyks_facilitating(self, synapse_model):
        """Test depression dynamics. Reproduces figure 1 B, (C)"""
        # Model parameters

        h = 0.1

        Tau = 60.0  # membrane time constant
        Theta = 15.0  # threshold
        E_L = 0.0  # reset potential of Vm
        R = 1.0
        C = Tau / R  # Tau [ms] / 1.0 GOhm in NEST units
        TauR = 2.0  # refractory time
        Tau_psc = 1.5  # time constant of PSC (=Tau_inact)
        Tau_rec = 130.0  # recovery time
        Tau_fac = 530.0  # facilitation time
        U = 0.03  # facilitation parameter U
        A = 1540.0  # PSC weight in pA

        input_train = np.arange(98.1, 1051.0, 50.1)  # first spike [ms]  # last spike [ms]  # interspike interval [ms]

        T_sim = 1200.0  # simulation time [ms]

        nest.ResetKernel()

        # set resolution and limits on delays
        # limits must be set BEFORE connecting any elements
        nest.SetKernelStatus({"resolution": h})

        sg = nest.Create("spike_generator", {"spike_times": input_train})
        pn = nest.Create("parrot_neuron")

        neuron = nest.Create("iaf_psc_exp_htum")
        neuron.tau_m = Tau
        neuron.t_ref_tot = TauR
        neuron.t_ref_abs = TauR
        neuron.tau_syn_ex = Tau_psc
        neuron.tau_syn_in = Tau_psc
        neuron.C_m = C
        neuron.V_th = Theta
        neuron.V_reset = E_L
        neuron.E_L = E_L
        neuron.V_m = E_L

        vm = nest.Create("voltmeter")
        vm.interval = 25.0

        nest.SetDefaults(
            synapse_model,
            {
                "tau_psc": Tau_psc,
                "tau_rec": Tau_rec,
                "tau_fac": Tau_fac,
                "U": U,
                "delay": 0.1,
                "weight": A,
                "u": 0.0,
                "x": 1.0,
            },
        )

        nest.Connect(sg, pn)  # pn required, devices cannot project to plastic synapses
        nest.Connect(pn, neuron, syn_spec={"synapse_model": synapse_model})
        nest.Connect(vm, neuron)

        nest.Simulate(T_sim)

        # compare results to expected voltage trace
        times_vm_expected = np.array(
            [
                [25, 0],
                [50, 0],
                [75, 0],
                [100, 4.739750000000000e-01],
                [125, 7.706030000000000e-01],
                [150, 1.297120000000000e00],
                [175, 1.757990000000000e00],
                [200, 2.114420000000000e00],
                [225, 2.714490000000000e00],
                [250, 2.785530000000000e00],
                [275, 3.546150000000000e00],
                [300, 3.272710000000000e00],
                [325, 4.233240000000000e00],
                [350, 3.583070000000000e00],
                [375, 4.788130000000000e00],
                [400, 3.739560000000000e00],
                [425, 5.233850000000000e00],
                [450, 3.767230000000000e00],
                [475, 5.593890000000000e00],
                [500, 3.687720000000000e00],
                [525, 5.888270000000000e00],
                [550, 3.881790000000000e00],
                [575, 6.132580000000000e00],
                [600, 4.042850000000000e00],
                [625, 6.338430000000000e00],
                [650, 4.178550000000000e00],
                [675, 6.514250000000000e00],
                [700, 4.294460000000000e00],
                [725, 6.666160000000000e00],
                [750, 4.394600000000000e00],
                [775, 6.798620000000000e00],
                [800, 4.481930000000000e00],
                [825, 6.914990000000000e00],
                [850, 4.558640000000000e00],
                [875, 7.017830000000000e00],
                [900, 4.626440000000000e00],
                [925, 7.109160000000000e00],
                [950, 4.686650000000000e00],
                [975, 7.190620000000000e00],
                [1000, 4.740350000000000e00],
                [1025, 7.263560000000000e00],
                [1050, 4.788430000000000e00],
                [1075, 7.329110000000000e00],
                [1100, 4.831650000000000e00],
                [1125, 3.185220000000000e00],
                [1150, 2.099830000000000e00],
                [1175, 1.384290000000000e00],
            ]
        )

        times_vm_sim = np.vstack([vm.get("events")["times"], vm.get("events")["V_m"]]).T

        # test uses large atol due to finite precision of reference timeseries
        np.testing.assert_allclose(times_vm_sim, times_vm_expected, atol=1e-5)
