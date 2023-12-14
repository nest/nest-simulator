# -*- coding: utf-8 -*-
#
# network.py
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

"""PyNEST EI-clustered network: Network Class
----------------------------------------

Main file of the EI-clustered network defining the ``ClusteredNetwork`` class with functions to
build and simulate the network.

"""

import pickle

import numpy as np

import general_helper
import helper
import nest


class ClusteredNetwork(object):
    """Provides functions to create neuron populations, stimulation devices and recording devices for an
    EI-clustered network and setups the simulation in NEST (v3.x).

    Creates an object with functions to create neuron populations,
    stimulation devices and recording devices for an EI-clustered network.
    Initializes the object. Creates the attributes Populations, RecordingDevices and
    Currentsources to be filled during network construction.
    Attribute params contains all parameters used to construct network.

    Parameters:
        defaultValues (module): A Module which contains the default configuration
        parameters (dict):      Dictionary with parameters which should be modified from their default values
    """

    def __init__(self, defaultValues, parameters):
        self.params = general_helper.merge_params(parameters, defaultValues)
        self.Populations = []
        self.RecordingDevices = []
        self.Currentsources = []
        self.ModelBuildPipeline = [
            self.setup_nest,
            self.create_populations,
            self.create_stimulation,
            self.create_recording_devices,
            self.connect,
        ]

    @classmethod
    def Clustered_Network_from_Rj_Rep(cls, Rj, Rep, defaultValues, parameters):
        """Creates a ClusteredNetwork object with parameters Rj and Rep

        Rj and Rep are used to calculate the parameters jplus (weight clustering) or pplus (probability clustering)

        Parameters:
            Rj (float):           Ratio excitatory to inhibitory clustering strength
            Rep (float):          Excitatory clustering strength
            defaultValues (module): A Module which contains the default configuration
            parameters (dict):      Dictionary with parameters which should be modified from their default values

        Returns:
            object: ClusteredNetwork object with parameters
        """
        params = general_helper.merge_params(parameters, defaultValues)
        if params["clustering"] == "weight":
            jep = Rep
            jip = 1.0 + (jep - 1) * Rj
            params["jplus"] = np.array([[jep, jip], [jip, jip]])
        elif params["clustering"] == "probabilities":
            pep = Rep
            pip = 1.0 + (pep - 1) * Rj
            params["pplus"] = np.array([[pep, pip], [pip, pip]])
        else:
            raise ValueError("Clustering type not recognized")
        return cls(defaultValues, params)

    def clean_network(self):
        """Creates clean network

        Creates empty attributes of a network.
        """
        self.Populations = []
        self.RecordingDevices = []
        self.Currentsources = []

    def setup_nest(self):
        """Initializes the NEST kernel.

        Reset the NEST kernel and pass parameters to it.
        Updates randseed of parameters to the actual used one if none is supplied.
        """
        nest.ResetKernel()
        nest.set_verbosity("M_WARNING")
        nest.local_num_threads = self.params.get("n_jobs", 1)
        nest.resolution = self.params.get("dt")
        self.params["randseed"] = self.params.get(
            "randseed", np.random.randint(1000000)
        )
        nest.rng_seed = self.params.get("randseed")
        # nest.print_time = True

    def create_populations(self):
        """Create all neuron populations.

        Q excitatory and inhibitory neuron populations with the parameters of the network are created.

        """
        # make sure number of clusters and units are compatible
        assert (
            self.params["N_E"] % self.params["Q"] == 0
        ), "N_E needs to be evenly divisible by Q"
        assert (
            self.params["N_I"] % self.params["Q"] == 0
        ), "N_I needs to be evenly divisible by Q"

        N = self.params["N_E"] + self.params["N_I"]  # total units
        try:
            DistParams = self.params["DistParams"]
        except AttributeError:
            DistParams = {"distribution": "normal", "sigma": 0.0, "fraction": False}

        if self.params["I_th_E"] is None:
            I_xE = self.params["I_xE"]
        else:
            I_xE = (
                self.params["I_th_E"]
                * (self.params["V_th_E"] - self.params["E_L"])
                / self.params["tau_E"]
                * self.params["C_m"]
            )

        if self.params["I_th_I"] is None:
            I_xI = self.params["I_xI"]
        else:
            I_xI = (
                self.params["I_th_I"]
                * (self.params["V_th_I"] - self.params["E_L"])
                / self.params["tau_I"]
                * self.params["C_m"]
            )

        E_neuron_params = {
            "E_L": self.params["E_L"],
            "C_m": self.params["C_m"],
            "tau_m": self.params["tau_E"],
            "t_ref": self.params["t_ref"],
            "V_th": self.params["V_th_E"],
            "V_reset": self.params["V_r"],
            "I_e": I_xE,
        }
        I_neuron_params = {
            "E_L": self.params["E_L"],
            "C_m": self.params["C_m"],
            "tau_m": self.params["tau_I"],
            "t_ref": self.params["t_ref"],
            "V_th": self.params["V_th_I"],
            "V_reset": self.params["V_r"],
            "I_e": I_xI,
        }
        if "iaf_psc_exp" in self.params["neuron_type"]:
            E_neuron_params["tau_syn_ex"] = self.params["tau_syn_ex"]
            E_neuron_params["tau_syn_in"] = self.params["tau_syn_in"]
            I_neuron_params["tau_syn_in"] = self.params["tau_syn_in"]
            I_neuron_params["tau_syn_ex"] = self.params["tau_syn_ex"]

            # iaf_psc_exp allows stochasticity, if not used - ignore
            try:
                if self.params["delta_"] is not None:
                    E_neuron_params["delta"] = self.params["delta_"]
                    I_neuron_params["delta"] = self.params["delta_"]
                if self.params["rho"] is not None:
                    E_neuron_params["rho"] = self.params["rho"]
                    I_neuron_params["rho"] = self.params["rho"]
            except KeyError:
                pass
        else:
            assert (
                "iaf_psc_exp" in self.params["neuron_type"]
            ), "iaf_psc_exp neuron model is the only implemented model"

            # create the neuron populations
        E_pops = []
        I_pops = []
        for q in range(self.params["Q"]):
            E_pops.append(
                nest.Create(
                    self.params["neuron_type"],
                    int(self.params["N_E"] / self.params["Q"]),
                )
            )
            nest.SetStatus(E_pops[-1], E_neuron_params)
        for q in range(self.params["Q"]):
            I_pops.append(
                nest.Create(
                    self.params["neuron_type"],
                    int(self.params["N_I"] / self.params["Q"]),
                )
            )
            nest.SetStatus(I_pops[-1], I_neuron_params)

        if self.params["delta_I_xE"] > 0:
            for E_pop in E_pops:
                I_xEs = nest.GetStatus(E_pop, "I_e")
                nest.SetStatus(
                    E_pop,
                    [
                        {
                            "I_e": (
                                1
                                - 0.5 * self.params["delta_I_xE"]
                                + np.random.rand() * self.params["delta_I_xE"]
                            )
                            * ixe
                        }
                        for ixe in I_xEs
                    ],
                )

        if self.params["delta_I_xI"] > 0:
            for I_pop in I_pops:
                I_xIs = nest.GetStatus(I_pop, "I_e")
                nest.SetStatus(
                    I_pop,
                    [
                        {
                            "I_e": (
                                1
                                - 0.5 * self.params["delta_I_xI"]
                                + np.random.rand() * self.params["delta_I_xI"]
                            )
                            * ixi
                        }
                        for ixi in I_xIs
                    ],
                )
        if self.params["V_m"] == "rand":
            T_0_E = self.params["t_ref"] + helper.fpt(
                self.params["tau_E"],
                self.params["E_L"],
                I_xE,
                self.params["C_m"],
                self.params["V_th_E"],
                self.params["V_r"],
            )
            if np.isnan(T_0_E):
                T_0_E = 10.0
            for E_pop in E_pops:
                nest.SetStatus(
                    E_pop,
                    [
                        {
                            "V_m": helper.v_fpt(
                                self.params["tau_E"],
                                self.params["E_L"],
                                I_xE,
                                self.params["C_m"],
                                T_0_E * np.random.rand(),
                                self.params["V_th_E"],
                                self.params["t_ref"],
                            )
                        }
                        for i in range(len(E_pop))
                    ],
                )

            T_0_I = self.params["t_ref"] + helper.fpt(
                self.params["tau_I"],
                self.params["E_L"],
                I_xI,
                self.params["C_m"],
                self.params["V_th_I"],
                self.params["V_r"],
            )
            if np.isnan(T_0_I):
                T_0_I = 10.0
            for I_pop in I_pops:
                nest.SetStatus(
                    I_pop,
                    [
                        {
                            "V_m": helper.v_fpt(
                                self.params["tau_I"],
                                self.params["E_L"],
                                I_xI,
                                self.params["C_m"],
                                T_0_I * np.random.rand(),
                                self.params["V_th_E"],
                                self.params["t_ref"],
                            )
                        }
                        for i in range(len(I_pop))
                    ],
                )
        else:
            nest.SetStatus(
                nest.NodeCollection([x for x in range(1, N + 1)]),
                [{"V_m": self.params["V_m"]} for i in range(N)],
            )
        self.Populations = [E_pops, I_pops]

    def connect(self):
        """
        Connects the excitatory and inhibitory populations with each other in the EI-clustered scheme
        """
        # if CLustering is set to "Weight" or is not set at all or is not in dictionary, use the connect_weight function
        try:
            if self.params["clustering"] == "weight":
                self.connect_weight()
            elif self.params["clustering"] == "probabilities":
                self.connect_probabilities()
            else:
                raise ValueError(
                    "Clustering method %s not implemented" % self.params["clustering"]
                )
        except KeyError:
            self.connect_weight()

    def connect_probabilities(self):
        """Connects the excitatory and inhibitory populations with each other in the EI-clustered scheme
        by increasing the probabilies of the connections within the clusters and decreasing the probabilites of the
        connections between the clusters. The weights are calculated so that the total input to a neuron
        is balanced.
        """
        #  self.Populations[0] -> Excitatory population
        #  self.Populations[1] -> Inhibitory population
        # connectivity parameters
        js = self.params["js"]  # connection weights
        N = self.params["N_E"] + self.params["N_I"]  # total units
        if np.isnan(js).any():
            js = helper.calc_js(self.params)
        js *= self.params["s"]

        if self.params["Q"] > 1:
            pminus = (self.params["Q"] - self.params["pplus"]) / float(
                self.params["Q"] - 1
            )
        else:
            self.params["pplus"] = np.ones((2, 2))
            pminus = np.ones((2, 2))

        p_plus = self.params["pplus"] * self.params["ps"]
        p_minus = pminus * self.params["ps"]

        iterations = np.ones((2, 2), dtype=int)
        # test if any of the probabilities is larger than 1
        if np.any(p_plus > 1):
            print("The probability of connection is larger than 1")
            print("p_plus: ", p_plus)
            print("p_minus: ", p_minus)
            for i in range(2):
                for j in range(2):
                    if p_plus[i, j] > 1:
                        iterations[i, j] = int(np.ceil(p_plus[i, j]))
                        p_plus[i, j] /= iterations[i, j]
            print("p_plus: ", p_plus)
            print(iterations)

        # define the synapses and connect the populations
        # EE
        j_ee = js[0, 0] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse", "EE", {"weight": j_ee, "delay": self.params["delay"]}
        )

        if self.params["fixed_indegree"]:
            K_EE_plus = int(p_plus[0, 0] * self.params["N_E"] / self.params["Q"])
            print("K_EE+: ", K_EE_plus)
            K_EE_minus = int(p_minus[0, 0] * self.params["N_E"] / self.params["Q"])
            print("K_EE-: ", K_EE_minus)
            conn_params_EE_plus = {
                "rule": "fixed_indegree",
                "indegree": K_EE_plus,
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_EE_minus = {
                "rule": "fixed_indegree",
                "indegree": K_EE_minus,
                "allow_autapses": False,
                "allow_multapses": True,
            }

        else:
            conn_params_EE_plus = {
                "rule": "pairwise_bernoulli",
                "p": p_plus[0, 0],
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_EE_minus = {
                "rule": "pairwise_bernoulli",
                "p": p_minus[0, 0],
                "allow_autapses": False,
                "allow_multapses": True,
            }
        for i, pre in enumerate(self.Populations[0]):
            for j, post in enumerate(self.Populations[0]):
                if i == j:
                    # same cluster
                    for n in range(iterations[0, 0]):
                        nest.Connect(pre, post, conn_params_EE_plus, "EE")
                else:
                    nest.Connect(pre, post, conn_params_EE_minus, "EE")

        # EI
        j_ei = js[0, 1] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse", "EI", {"weight": j_ei, "delay": self.params["delay"]}
        )

        if self.params["fixed_indegree"]:
            K_EI_plus = int(p_plus[0, 1] * self.params["N_I"] / self.params["Q"])
            print("K_EI+: ", K_EI_plus)
            K_EI_minus = int(p_minus[0, 1] * self.params["N_I"] / self.params["Q"])
            print("K_EI-: ", K_EI_minus)
            conn_params_EI_plus = {
                "rule": "fixed_indegree",
                "indegree": K_EI_plus,
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_EI_minus = {
                "rule": "fixed_indegree",
                "indegree": K_EI_minus,
                "allow_autapses": False,
                "allow_multapses": True,
            }

        else:
            conn_params_EI_plus = {
                "rule": "pairwise_bernoulli",
                "p": p_plus[0, 1],
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_EI_minus = {
                "rule": "pairwise_bernoulli",
                "p": p_minus[0, 1],
                "allow_autapses": False,
                "allow_multapses": True,
            }
        for i, pre in enumerate(self.Populations[1]):
            for j, post in enumerate(self.Populations[0]):
                if i == j:
                    # same cluster
                    for n in range(iterations[0, 1]):
                        nest.Connect(pre, post, conn_params_EI_plus, "EI")
                else:
                    nest.Connect(pre, post, conn_params_EI_minus, "EI")

        # IE
        j_ie = js[1, 0] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse", "IE", {"weight": j_ie, "delay": self.params["delay"]}
        )

        if self.params["fixed_indegree"]:
            K_IE_plus = int(p_plus[1, 0] * self.params["N_E"] / self.params["Q"])
            print("K_IE+: ", K_IE_plus)
            K_IE_minus = int(p_minus[1, 0] * self.params["N_E"] / self.params["Q"])
            print("K_IE-: ", K_IE_minus)
            conn_params_IE_plus = {
                "rule": "fixed_indegree",
                "indegree": K_IE_plus,
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_IE_minus = {
                "rule": "fixed_indegree",
                "indegree": K_IE_minus,
                "allow_autapses": False,
                "allow_multapses": True,
            }

        else:
            conn_params_IE_plus = {
                "rule": "pairwise_bernoulli",
                "p": p_plus[1, 0],
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_IE_minus = {
                "rule": "pairwise_bernoulli",
                "p": p_minus[1, 0],
                "allow_autapses": False,
                "allow_multapses": True,
            }
        for i, pre in enumerate(self.Populations[0]):
            for j, post in enumerate(self.Populations[1]):
                if i == j:
                    # same cluster
                    for n in range(iterations[1, 0]):
                        nest.Connect(pre, post, conn_params_IE_plus, "IE")
                else:
                    nest.Connect(pre, post, conn_params_IE_minus, "IE")

        # II
        j_ii = js[1, 1] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse", "II", {"weight": j_ii, "delay": self.params["delay"]}
        )

        if self.params["fixed_indegree"]:
            K_II_plus = int(p_plus[1, 1] * self.params["N_I"] / self.params["Q"])
            print("K_II+: ", K_II_plus)
            K_II_minus = int(p_minus[1, 1] * self.params["N_I"] / self.params["Q"])
            print("K_II-: ", K_II_minus)
            conn_params_II_plus = {
                "rule": "fixed_indegree",
                "indegree": K_II_plus,
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_II_minus = {
                "rule": "fixed_indegree",
                "indegree": K_II_minus,
                "allow_autapses": False,
                "allow_multapses": True,
            }

        else:
            conn_params_II_plus = {
                "rule": "pairwise_bernoulli",
                "p": p_plus[1, 1],
                "allow_autapses": False,
                "allow_multapses": True,
            }
            conn_params_II_minus = {
                "rule": "pairwise_bernoulli",
                "p": p_minus[1, 1],
                "allow_autapses": False,
                "allow_multapses": True,
            }
        for i, pre in enumerate(self.Populations[1]):
            for j, post in enumerate(self.Populations[1]):
                if i == j:
                    # same cluster
                    for n in range(iterations[1, 1]):
                        nest.Connect(pre, post, conn_params_II_plus, "II")
                else:
                    nest.Connect(pre, post, conn_params_II_minus, "II")

    def connect_weight(self):
        """Connects the excitatory and inhibitory populations with each other in the EI-clustered scheme
        by increasing the weights of the connections within the clusters and decreasing the weights of the
        connections between the clusters. The weights are calculated so that the total input to a neuron
        is balanced.
        """
        #  self.Populations[0] -> Excitatory population
        #  self.Populations[1] -> Inhibitory population
        # connectivity parameters
        js = self.params["js"]  # connection weights
        N = self.params["N_E"] + self.params["N_I"]  # total units

        # if js are not given compute them so that sqrt(K) spikes equal v_thr-E_L and rows are balanced
        if np.isnan(js).any():
            js = helper.calc_js(self.params)
        js *= self.params["s"]

        # jminus is calculated so that row sums remain constant
        if self.params["Q"] > 1:
            jminus = (self.params["Q"] - self.params["jplus"]) / float(
                self.params["Q"] - 1
            )
        else:
            self.params["jplus"] = np.ones((2, 2))
            jminus = np.ones((2, 2))

        # define the synapses and connect the populations
        # EE
        j_ee = js[0, 0] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "EE_plus",
            {
                "weight": self.params["jplus"][0, 0] * j_ee,
                "delay": self.params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "EE_minus",
            {"weight": jminus[0, 0] * j_ee, "delay": self.params["delay"]},
        )
        if self.params["fixed_indegree"]:
            K_EE = int(self.params["ps"][0, 0] * self.params["N_E"] / self.params["Q"])
            print("K_EE: ", K_EE)
            conn_params_EE = {
                "rule": "fixed_indegree",
                "indegree": K_EE,
                "allow_autapses": False,
                "allow_multapses": False,
            }

        else:
            conn_params_EE = {
                "rule": "pairwise_bernoulli",
                "p": self.params["ps"][0, 0],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self.Populations[0]):
            for j, post in enumerate(self.Populations[0]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_EE, "EE_plus")
                else:
                    nest.Connect(pre, post, conn_params_EE, "EE_minus")

        # EI
        j_ei = js[0, 1] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "EI_plus",
            {
                "weight": j_ei * self.params["jplus"][0, 1],
                "delay": self.params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "EI_minus",
            {"weight": j_ei * jminus[0, 1], "delay": self.params["delay"]},
        )
        if self.params["fixed_indegree"]:
            K_EI = int(self.params["ps"][0, 1] * self.params["N_I"] / self.params["Q"])
            print("K_EI: ", K_EI)
            conn_params_EI = {
                "rule": "fixed_indegree",
                "indegree": K_EI,
                "allow_autapses": False,
                "allow_multapses": False,
            }
        else:
            conn_params_EI = {
                "rule": "pairwise_bernoulli",
                "p": self.params["ps"][0, 1],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self.Populations[1]):
            for j, post in enumerate(self.Populations[0]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_EI, "EI_plus")
                else:
                    nest.Connect(pre, post, conn_params_EI, "EI_minus")
        # IE
        j_ie = js[1, 0] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "IE_plus",
            {
                "weight": j_ie * self.params["jplus"][1, 0],
                "delay": self.params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "IE_minus",
            {"weight": j_ie * jminus[1, 0], "delay": self.params["delay"]},
        )

        if self.params["fixed_indegree"]:
            K_IE = int(self.params["ps"][1, 0] * self.params["N_E"] / self.params["Q"])
            print("K_IE: ", K_IE)
            conn_params_IE = {
                "rule": "fixed_indegree",
                "indegree": K_IE,
                "allow_autapses": False,
                "allow_multapses": False,
            }
        else:
            conn_params_IE = {
                "rule": "pairwise_bernoulli",
                "p": self.params["ps"][1, 0],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self.Populations[0]):
            for j, post in enumerate(self.Populations[1]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_IE, "IE_plus")
                else:
                    nest.Connect(pre, post, conn_params_IE, "IE_minus")

        # II
        j_ii = js[1, 1] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "II_plus",
            {
                "weight": j_ii * self.params["jplus"][1, 1],
                "delay": self.params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "II_minus",
            {"weight": j_ii * jminus[1, 1], "delay": self.params["delay"]},
        )
        if self.params["fixed_indegree"]:
            K_II = int(self.params["ps"][1, 1] * self.params["N_I"] / self.params["Q"])
            print("K_II: ", K_II)
            conn_params_II = {
                "rule": "fixed_indegree",
                "indegree": K_II,
                "allow_autapses": False,
                "allow_multapses": False,
            }
        else:
            conn_params_II = {
                "rule": "pairwise_bernoulli",
                "p": self.params["ps"][1, 1],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self.Populations[1]):
            for j, post in enumerate(self.Populations[1]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_II, "II_plus")
                else:
                    nest.Connect(pre, post, conn_params_II, "II_minus")
        # print('Js: ', js / np.sqrt(N))

    def create_stimulation(self):
        """
        Creates a current source and connects it to the specified cluster/s.
        """
        if self.params["stim_clusters"] is not None:
            stim_amp = self.params[
                "stim_amp"
            ]  # amplitude of the stimulation current in pA
            stim_starts = self.params["stim_starts"]  # list of stimulation start times
            stim_ends = self.params["stim_ends"]  # list of stimulation end times
            amplitude_values = []
            amplitude_times = []
            for start, end in zip(stim_starts, stim_ends):
                amplitude_times.append(start + self.params["warmup"])
                amplitude_values.append(stim_amp)
                amplitude_times.append(end + self.params["warmup"])
                amplitude_values.append(0.0)
            self.Currentsources = [nest.Create("step_current_generator")]
            for stim_cluster in self.params["stim_clusters"]:
                nest.Connect(self.Currentsources[0], self.Populations[0][stim_cluster])
            nest.SetStatus(
                self.Currentsources[0],
                {
                    "amplitude_times": amplitude_times,
                    "amplitude_values": amplitude_values,
                },
            )

    def create_recording_devices(self):
        """
        Creates a spike recorder connected to all neuron populations created by create_populations
        """
        self.RecordingDevices = [nest.Create("spike_recorder")]
        self.RecordingDevices[0].record_to = "memory"

        all_units = self.Populations[0][0]
        for E_pop in self.Populations[0][1:]:
            all_units += E_pop
        for I_pop in self.Populations[1]:
            all_units += I_pop
        nest.Connect(all_units, self.RecordingDevices[0], "all_to_all")  # Spikerecorder

    def set_model_build_pipeline(self, Pipeline):
        """
        Sets the ModelBuildPipeline.
        Parameters:
            Pipeline (list of functions):   ordered list of functions executed to build the network model
        """
        self.ModelBuildPipeline = Pipeline

    def setup_network(self):
        """
        Initializes NEST and creates the network in NEST, ready to be simulated.
        Functions saved in ModelBuildPipeline are executed.
        nest.Prepare is executed in this function.
        """
        for func in self.ModelBuildPipeline:
            func()
        nest.Prepare()

    def simulate(self):
        """
        Simulates network for a period of warmup+simtime
        """
        if self.params["warmup"] + self.params["simtime"] <= 0.1:
            pass
        else:
            nest.Run(self.params["warmup"] + self.params["simtime"])

    def get_recordings(self):
        """
        Extracts spikes form the Spikerecorder connected to all populations created in create_populations.
        Cuts the warmup period away and sets time relative to end of warmup.
        Ids 1:N_E correspond to excitatory neurons, N_E+1:N_E+N_I correspond to inhibitory neurons.

        Returns:
            spiketimes (np.array): Row 0: spiketimes, Row 1: neuron ID.
        """
        events = nest.GetStatus(self.RecordingDevices[0], "events")[0]
        # convert them to the format accepted by spiketools
        spiketimes = np.append(
            events["times"][None, :], events["senders"][None, :], axis=0
        )
        spiketimes[1] -= 1
        # remove the pre warmup spikes
        spiketimes = spiketimes[:, spiketimes[0] >= self.params["warmup"]]
        spiketimes[0] -= self.params["warmup"]
        return spiketimes

    def get_parameter(self):
        """
        Returns all parameters used to create the network.
        Return:
            parameters (dict): Dictionary with all parameters for the simulation / network creation.
        """
        return self.params

    def create_and_simulate(self):
        """
        Creates the EI-clustered network and simulates it with the parameters supplied in the object creation.

        Returns:
            spiketimes (np.array):  Row 0: spiketimes, Row 1: neuron ID.
                                    Ids 1:N_E correspond to excitatory neurons,
                                    N_E+1:N_E+N_I correspond to inhibitory neurons.
        """
        self.setup_network()
        self.simulate()
        return self.get_recordings()

    def get_firing_rates(self, spiketimes=None):
        """
        Calculates the firing rates of all excitatory neurons and the firing rates of all inhibitory neurons
        created by self.create_populations. If spiketimes are not supplied, they get extracted.
        Parameters:
            spiketimes: (optional, np.array 2xT)   spiketimes of simulation
        Returns:
            (e_rate, i_rate) average firing rate of excitatory/inhibitory neurons (spikes/s)
        """
        if spiketimes is None:
            spiketimes = self.get_recordings()
        e_count = spiketimes[:, spiketimes[1] < self.params["N_E"]].shape[1]
        i_count = spiketimes[:, spiketimes[1] >= self.params["N_E"]].shape[1]
        e_rate = (
            e_count / float(self.params["N_E"]) / float(self.params["simtime"]) * 1000.0
        )
        i_rate = (
            i_count / float(self.params["N_I"]) / float(self.params["simtime"]) * 1000.0
        )
        return e_rate, i_rate

    def set_I_x(self, I_XE, I_XI):
        """
        Adds DC currents for the excitatory and inhibitory neurons. Allows also the usage of a baseline defined by the
        model setup.
        """
        for E_pop in self.Populations[0]:
            I_e_loc = E_pop.get("I_e")
            E_pop.set({"I_e": I_e_loc + I_XE})
        for I_pop in self.Populations[1]:
            I_e_loc = I_pop.get("I_e")
            I_pop.set({"I_e": I_e_loc + I_XI})

    def get_simulation(self, PathSpikes=None):
        """
        Creates the network, simulates it and extracts the firing rates. If PathSpikes is supplied the spikes get saved
        to a pickle file. If a timeout is supplied, a timeout handler is created which stops the execution.
        Parameters:
            PathSpikes: (optional) Path of file for spiketimes
            timeout: (optional) Time of timeout in seconds
        Returns:
            Dictionary with firing rates, timing information (dict) and parameters (dict)
        """

        self.setup_network()
        self.simulate()
        spiketimes = self.get_recordings()
        e_rate, i_rate = self.get_firing_rates(spiketimes)

        if PathSpikes is not None:
            with open(PathSpikes, "wb") as outfile:
                pickle.dump(spiketimes, outfile)

        nest.Cleanup()
        return {
            "e_rate": e_rate,
            "i_rate": i_rate,
            "params": self.get_parameter(),
            "spiketimes": spiketimes,
        }
