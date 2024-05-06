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
---------------------------------------------

``ClusteredNetwork`` class with functions to build and simulate
the EI-clustered network.
"""

import pickle

import helper
import nest
import numpy as np


class ClusteredNetwork:
    """EI-clustered network objeect to build and simulate the network.

    Provides functions to create neuron populations,
    stimulation devices and recording devices for an
    EI-clustered network and setups the simulation in
    NEST (v3.x).

    Attributes
    ----------
    _params: dict
        Dictionary with parameters used to construct network.
    _populations: list
        List of neuron population groups.
    _recording_devices: list
        List of recording devices.
    _currentsources: list
        List of current sources.
    _model_build_pipeline: list
        List of functions to build the network.
    """

    def __init__(self, sim_dict, net_dict, stim_dict):
        """Initialize the ClusteredNetwork object.

        Parameters are given and explained in the files network_params.py,
        sim_params.py and stimulus_params.py.

        Parameters
        ----------
        sim_dict: dict
            Dictionary with simulation parameters.
        net_dict: dict
            Dictionary with network parameters.
        stim_dict: dict
            Dictionary with stimulus parameters.
        """

        # merge dictionaries of simulation, network and stimulus parameters
        self._params = {**sim_dict, **net_dict, **stim_dict}
        # list of neuron population groups [E_pops, I_pops]
        self._populations = []
        self._recording_devices = []
        self._currentsources = []
        self._model_build_pipeline = [
            self.setup_nest,
            self.create_populations,
            self.create_stimulation,
            self.create_recording_devices,
            self.connect,
        ]

        if self._params["clustering"] == "weight":
            jep = self._params["rep"]
            jip = 1.0 + (jep - 1) * self._params["rj"]
            self._params["jplus"] = np.array([[jep, jip], [jip, jip]])
        elif self._params["clustering"] == "probabilities":
            pep = self._params["rep"]
            pip = 1.0 + (pep - 1) ** self._params["rj"]
            self._params["pplus"] = np.array([[pep, pip], [pip, pip]])
        else:
            raise ValueError("Clustering type not recognized")

    def setup_nest(self):
        """Initializes the NEST kernel.

        Reset the NEST kernel and pass parameters to it.
        Updates randseed of parameters to the actual
        used one if none is supplied.
        """

        nest.ResetKernel()
        nest.set_verbosity("M_WARNING")
        nest.local_num_threads = self._params.get("n_vp", 4)
        nest.resolution = self._params.get("dt")
        self._params["randseed"] = self._params.get("randseed")
        nest.rng_seed = self._params.get("randseed")

    def create_populations(self):
        """Create all neuron populations.

        n_clusters excitatory and inhibitory neuron populations
        with the parameters of the network are created.
        """

        # make sure number of clusters and units are compatible
        if self._params["N_E"] % self._params["n_clusters"] != 0:
            raise ValueError("N_E must be a multiple of Q")
        if self._params["N_I"] % self._params["n_clusters"] != 0:
            raise ValueError("N_E must be a multiple of Q")
        if self._params["neuron_type"] != "iaf_psc_exp":
            raise ValueError("Model only implemented for iaf_psc_exp neuron model")

        if self._params["I_th_E"] is None:
            I_xE = self._params["I_xE"]  # I_xE is the feed forward excitatory input in pA
        else:
            I_xE = self._params["I_th_E"] * helper.rheobase_current(
                self._params["tau_E"], self._params["E_L"], self._params["V_th_E"], self._params["C_m"]
            )

        if self._params["I_th_I"] is None:
            I_xI = self._params["I_xI"]
        else:
            I_xI = self._params["I_th_I"] * helper.rheobase_current(
                self._params["tau_I"], self._params["E_L"], self._params["V_th_I"], self._params["C_m"]
            )

        E_neuron_params = {
            "E_L": self._params["E_L"],
            "C_m": self._params["C_m"],
            "tau_m": self._params["tau_E"],
            "t_ref": self._params["t_ref"],
            "V_th": self._params["V_th_E"],
            "V_reset": self._params["V_r"],
            "I_e": I_xE
            if self._params["delta_I_xE"] == 0
            else I_xE * nest.random.uniform(1 - self._params["delta_I_xE"] / 2, 1 + self._params["delta_I_xE"] / 2),
            "tau_syn_ex": self._params["tau_syn_ex"],
            "tau_syn_in": self._params["tau_syn_in"],
            "V_m": self._params["V_m"]
            if not self._params["V_m"] == "rand"
            else self._params["V_th_E"] - 20 * nest.random.lognormal(0, 1),
        }
        I_neuron_params = {
            "E_L": self._params["E_L"],
            "C_m": self._params["C_m"],
            "tau_m": self._params["tau_I"],
            "t_ref": self._params["t_ref"],
            "V_th": self._params["V_th_I"],
            "V_reset": self._params["V_r"],
            "I_e": I_xI
            if self._params["delta_I_xE"] == 0
            else I_xI * nest.random.uniform(1 - self._params["delta_I_xE"] / 2, 1 + self._params["delta_I_xE"] / 2),
            "tau_syn_ex": self._params["tau_syn_ex"],
            "tau_syn_in": self._params["tau_syn_in"],
            "V_m": self._params["V_m"]
            if not self._params["V_m"] == "rand"
            else self._params["V_th_I"] - 20 * nest.random.lognormal(0, 1),
        }

        # iaf_psc_exp allows stochasticity, if not used - don't supply the parameters and use
        # iaf_psc_exp as deterministic model
        if (self._params.get("delta") is not None) and (self._params.get("rho") is not None):
            E_neuron_params["delta"] = self._params["delta"]
            I_neuron_params["delta"] = self._params["delta"]
            E_neuron_params["rho"] = self._params["rho"]
            I_neuron_params["rho"] = self._params["rho"]

        # create the neuron populations
        pop_size_E = self._params["N_E"] // self._params["n_clusters"]
        pop_size_I = self._params["N_I"] // self._params["n_clusters"]
        E_pops = [
            nest.Create(self._params["neuron_type"], n=pop_size_E, params=E_neuron_params)
            for _ in range(self._params["n_clusters"])
        ]
        I_pops = [
            nest.Create(self._params["neuron_type"], n=pop_size_I, params=I_neuron_params)
            for _ in range(self._params["n_clusters"])
        ]

        self._populations = [E_pops, I_pops]

    def connect(self):
        """Connect the excitatory and inhibitory populations with each other
        in the EI-clustered scheme

        Raises
        ------
        ValueError
            If the clustering method is not recognized
        """

        if "clustering" not in self._params or self._params["clustering"] == "weight":
            self.connect_weight()
        elif self._params["clustering"] == "probabilities":
            self.connect_probabilities()
        else:
            raise ValueError("Clustering method %s not implemented" % self._params["clustering"])

    def connect_probabilities(self):
        """Connect the clusters with a probability EI-cluster scheme

        Connects the excitatory and inhibitory populations with each other
        in the EI-clustered scheme by increasing the probabilities of the
        connections within the clusters and decreasing the probabilities of the
        connections between the clusters. The weights are calculated so that
        the total input to a neuron is balanced.
        """

        #  self._populations[0] -> Excitatory population
        #  self._populations[1] -> Inhibitory population

        N = self._params["N_E"] + self._params["N_I"]  # total units
        # if js are not given compute them so that sqrt(K) spikes equal v_thr-E_L and rows are balanced
        # if any of the js is nan or not given
        if self._params.get("js") is None or np.isnan(self._params.get("js")).any():
            js = helper.calculate_RBN_weights(self._params)
        js *= self._params["s"]

        if self._params["n_clusters"] > 1:
            pminus = (self._params["n_clusters"] - self._params["pplus"]) / float(self._params["n_clusters"] - 1)
        else:
            self._params["pplus"] = np.ones((2, 2))
            pminus = np.ones((2, 2))

        p_plus = self._params["pplus"] * self._params["baseline_conn_prob"]
        p_minus = pminus * self._params["baseline_conn_prob"]

        # Connection probabilities within clusters can exceed 1. In this case, we iteratively split
        # the connections in multiple synapse populations with probabilities < 1.
        iterations = np.ones((2, 2), dtype=int)
        # test if any of the probabilities is larger than 1
        if np.any(p_plus > 1):
            print("The probability of some connections is larger than 1.")
            print("Pre-splitting the connections in multiple synapse populations:")
            printoptions = np.get_printoptions()
            np.set_printoptions(precision=2, floatmode="fixed")
            print("p_plus:\n", p_plus)
            print("p_minus:\n", p_minus)
            for i in range(2):
                for j in range(2):
                    if p_plus[i, j] > 1:
                        iterations[i, j] = int(np.ceil(p_plus[i, j]))
                        p_plus[i, j] /= iterations[i, j]
            print("\nPost-splitting the connections in multiple synapse populations:")
            print("p_plus:\n", p_plus)
            print("Number of synapse populations:\n", iterations)
            np.set_printoptions(**printoptions)

        # define the synapses and connect the populations

        # Excitatory to excitatory neuron connections
        j_ee = js[0, 0] / np.sqrt(N)
        nest.CopyModel("static_synapse", "EE", {"weight": j_ee, "delay": self._params["delay"]})

        if self._params["fixed_indegree"]:
            K_EE_plus = int(p_plus[0, 0] * self._params["N_E"] / self._params["n_clusters"])
            print("K_EE+: ", K_EE_plus)
            K_EE_minus = int(p_minus[0, 0] * self._params["N_E"] / self._params["n_clusters"])
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
        for i, pre in enumerate(self._populations[0]):
            for j, post in enumerate(self._populations[0]):
                if i == j:
                    # same cluster
                    for n in range(iterations[0, 0]):
                        nest.Connect(pre, post, conn_params_EE_plus, "EE")
                else:
                    nest.Connect(pre, post, conn_params_EE_minus, "EE")

        # Inhibitory to excitatory neuron connections
        j_ei = js[0, 1] / np.sqrt(N)
        nest.CopyModel("static_synapse", "EI", {"weight": j_ei, "delay": self._params["delay"]})

        if self._params["fixed_indegree"]:
            K_EI_plus = int(p_plus[0, 1] * self._params["N_I"] / self._params["n_clusters"])
            print("K_EI+: ", K_EI_plus)
            K_EI_minus = int(p_minus[0, 1] * self._params["N_I"] / self._params["n_clusters"])
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
        for i, pre in enumerate(self._populations[1]):
            for j, post in enumerate(self._populations[0]):
                if i == j:
                    # same cluster
                    for n in range(iterations[0, 1]):
                        nest.Connect(pre, post, conn_params_EI_plus, "EI")
                else:
                    nest.Connect(pre, post, conn_params_EI_minus, "EI")

        # Excitatory to inhibitory neuron connections
        j_ie = js[1, 0] / np.sqrt(N)
        nest.CopyModel("static_synapse", "IE", {"weight": j_ie, "delay": self._params["delay"]})

        if self._params["fixed_indegree"]:
            K_IE_plus = int(p_plus[1, 0] * self._params["N_E"] / self._params["n_clusters"])
            print("K_IE+: ", K_IE_plus)
            K_IE_minus = int(p_minus[1, 0] * self._params["N_E"] / self._params["n_clusters"])
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
        for i, pre in enumerate(self._populations[0]):
            for j, post in enumerate(self._populations[1]):
                if i == j:
                    # same cluster
                    for n in range(iterations[1, 0]):
                        nest.Connect(pre, post, conn_params_IE_plus, "IE")
                else:
                    nest.Connect(pre, post, conn_params_IE_minus, "IE")

        # Inhibitory to inhibitory neuron connections
        j_ii = js[1, 1] / np.sqrt(N)
        nest.CopyModel("static_synapse", "II", {"weight": j_ii, "delay": self._params["delay"]})

        if self._params["fixed_indegree"]:
            K_II_plus = int(p_plus[1, 1] * self._params["N_I"] / self._params["n_clusters"])
            print("K_II+: ", K_II_plus)
            K_II_minus = int(p_minus[1, 1] * self._params["N_I"] / self._params["n_clusters"])
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
        for i, pre in enumerate(self._populations[1]):
            for j, post in enumerate(self._populations[1]):
                if i == j:
                    # same cluster
                    for n in range(iterations[1, 1]):
                        nest.Connect(pre, post, conn_params_II_plus, "II")
                else:
                    nest.Connect(pre, post, conn_params_II_minus, "II")

    def connect_weight(self):
        """Connect the clusters with a weight EI-cluster scheme

        Connects the excitatory and inhibitory populations with
        each other in the EI-clustered scheme by increasing the weights
        of the connections within the clusters and decreasing the weights
        of the connections between the clusters. The weights are calculated
        so that the total input to a neuron is balanced.
        """

        #  self._populations[0] -> Excitatory population
        #  self._populations[1] -> Inhibitory population

        N = self._params["N_E"] + self._params["N_I"]  # total units

        # if js are not given compute them so that sqrt(K) spikes equal v_thr-E_L and rows are balanced
        # if any of the js is nan or not given
        if self._params.get("js") is None or np.isnan(self._params.get("js")).any():
            js = helper.calculate_RBN_weights(self._params)
        js *= self._params["s"]

        # jminus is calculated so that row sums remain constant
        if self._params["n_clusters"] > 1:
            jminus = (self._params["n_clusters"] - self._params["jplus"]) / float(self._params["n_clusters"] - 1)
        else:
            self._params["jplus"] = np.ones((2, 2))
            jminus = np.ones((2, 2))

        # define the synapses and connect the populations

        # Excitatory to excitatory neuron connections
        j_ee = js[0, 0] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "EE_plus",
            {
                "weight": self._params["jplus"][0, 0] * j_ee,
                "delay": self._params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "EE_minus",
            {"weight": jminus[0, 0] * j_ee, "delay": self._params["delay"]},
        )
        if self._params["fixed_indegree"]:
            K_EE = int(self._params["baseline_conn_prob"][0, 0] * self._params["N_E"] / self._params["n_clusters"])
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
                "p": self._params["baseline_conn_prob"][0, 0],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self._populations[0]):
            for j, post in enumerate(self._populations[0]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_EE, "EE_plus")
                else:
                    nest.Connect(pre, post, conn_params_EE, "EE_minus")

        # Inhibitory to excitatory neuron connections
        j_ei = js[0, 1] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "EI_plus",
            {
                "weight": j_ei * self._params["jplus"][0, 1],
                "delay": self._params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "EI_minus",
            {"weight": j_ei * jminus[0, 1], "delay": self._params["delay"]},
        )
        if self._params["fixed_indegree"]:
            K_EI = int(self._params["baseline_conn_prob"][0, 1] * self._params["N_I"] / self._params["n_clusters"])
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
                "p": self._params["baseline_conn_prob"][0, 1],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self._populations[1]):
            for j, post in enumerate(self._populations[0]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_EI, "EI_plus")
                else:
                    nest.Connect(pre, post, conn_params_EI, "EI_minus")

        # Excitatory to inhibitory neuron connections
        j_ie = js[1, 0] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "IE_plus",
            {
                "weight": j_ie * self._params["jplus"][1, 0],
                "delay": self._params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "IE_minus",
            {"weight": j_ie * jminus[1, 0], "delay": self._params["delay"]},
        )

        if self._params["fixed_indegree"]:
            K_IE = int(self._params["baseline_conn_prob"][1, 0] * self._params["N_E"] / self._params["n_clusters"])
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
                "p": self._params["baseline_conn_prob"][1, 0],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self._populations[0]):
            for j, post in enumerate(self._populations[1]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_IE, "IE_plus")
                else:
                    nest.Connect(pre, post, conn_params_IE, "IE_minus")

        # Inhibitory to inhibitory neuron connections
        j_ii = js[1, 1] / np.sqrt(N)
        nest.CopyModel(
            "static_synapse",
            "II_plus",
            {
                "weight": j_ii * self._params["jplus"][1, 1],
                "delay": self._params["delay"],
            },
        )
        nest.CopyModel(
            "static_synapse",
            "II_minus",
            {"weight": j_ii * jminus[1, 1], "delay": self._params["delay"]},
        )
        if self._params["fixed_indegree"]:
            K_II = int(self._params["baseline_conn_prob"][1, 1] * self._params["N_I"] / self._params["n_clusters"])
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
                "p": self._params["baseline_conn_prob"][1, 1],
                "allow_autapses": False,
                "allow_multapses": False,
            }
        for i, pre in enumerate(self._populations[1]):
            for j, post in enumerate(self._populations[1]):
                if i == j:
                    # same cluster
                    nest.Connect(pre, post, conn_params_II, "II_plus")
                else:
                    nest.Connect(pre, post, conn_params_II, "II_minus")

    def create_stimulation(self):
        """Create a current source and connect it to clusters."""
        if self._params["stim_clusters"] is not None:
            stim_amp = self._params["stim_amp"]  # amplitude of the stimulation current in pA
            stim_starts = self._params["stim_starts"]  # list of stimulation start times
            stim_ends = self._params["stim_ends"]  # list of stimulation end times
            amplitude_values = []
            amplitude_times = []
            for start, end in zip(stim_starts, stim_ends):
                amplitude_times.append(start + self._params["warmup"])
                amplitude_values.append(stim_amp)
                amplitude_times.append(end + self._params["warmup"])
                amplitude_values.append(0.0)
            self._currentsources = [nest.Create("step_current_generator")]
            for stim_cluster in self._params["stim_clusters"]:
                nest.Connect(self._currentsources[0], self._populations[0][stim_cluster])
            nest.SetStatus(
                self._currentsources[0],
                {
                    "amplitude_times": amplitude_times,
                    "amplitude_values": amplitude_values,
                },
            )

    def create_recording_devices(self):
        """Creates a spike recorder

        Create and connect a spike recorder to all neuron populations
        in self._populations.
        """
        self._recording_devices = [nest.Create("spike_recorder")]
        self._recording_devices[0].record_to = "memory"

        all_units = self._populations[0][0]
        for E_pop in self._populations[0][1:]:
            all_units += E_pop
        for I_pop in self._populations[1]:
            all_units += I_pop
        nest.Connect(all_units, self._recording_devices[0], "all_to_all")  # Spikerecorder

    def set_model_build_pipeline(self, pipeline):
        """Set _model_build_pipeline

        Parameters
        ----------
        pipeline: list
            ordered list of functions executed to build the network model
        """
        self._model_build_pipeline = pipeline

    def setup_network(self):
        """Setup network in NEST

        Initializes NEST and creates
        the network in NEST, ready to be simulated.
        Functions saved in _model_build_pipeline are executed.
        """
        for func in self._model_build_pipeline:
            func()

    def simulate(self):
        """Simulates network for a period of warmup+simtime"""
        nest.Simulate(self._params["warmup"] + self._params["simtime"])

    def get_recordings(self):
        """Extract spikes from Spikerecorder

        Extract spikes form the Spikerecorder connected
        to all populations created in create_populations.
        Cuts the warmup period away and sets time relative to end of warmup.
        Ids 1:N_E correspond to excitatory neurons,
        N_E+1:N_E+N_I correspond to inhibitory neurons.

        Returns
        -------
        spiketimes: ndarray
            2D array [2xN_Spikes]
            of spiketimes with spiketimes in row 0 and neuron IDs in row 1.
        """
        events = nest.GetStatus(self._recording_devices[0], "events")[0]
        # convert them to the format accepted by spiketools
        spiketimes = np.append(events["times"][None, :], events["senders"][None, :], axis=0)
        spiketimes[1] -= 1
        # remove the pre warmup spikes
        spiketimes = spiketimes[:, spiketimes[0] >= self._params["warmup"]]
        spiketimes[0] -= self._params["warmup"]
        return spiketimes

    def get_parameter(self):
        """Get all parameters used to create the network.
        Returns
        -------
        dict
            Dictionary with all parameters of the network and the simulation.
        """
        return self._params

    def create_and_simulate(self):
        """Create and simulate the EI-clustered network.

        Returns
        -------
        spiketimes: ndarray
            2D array [2xN_Spikes]
            of spiketimes with spiketimes in row 0 and neuron IDs in row 1.
        """
        self.setup_network()
        self.simulate()
        return self.get_recordings()

    def get_firing_rates(self, spiketimes=None):
        """Calculates the average firing rates of
        all excitatory and inhibitory neurons.

        Calculates the firing rates of all excitatory neurons
        and the firing rates of all inhibitory neurons
        created by self.create_populations.
        If spiketimes are not supplied, they get extracted.

        Parameters
        ----------
        spiketimes: ndarray
            2D array [2xN_Spikes] of spiketimes
            with spiketimes in row 0 and neuron IDs in row 1.

        Returns
        -------
        tuple[float, float]
            average firing rates of excitatory (0)
            and inhibitory (1) neurons (spikes/s)
        """
        if spiketimes is None:
            spiketimes = self.get_recordings()
        e_count = spiketimes[:, spiketimes[1] < self._params["N_E"]].shape[1]
        i_count = spiketimes[:, spiketimes[1] >= self._params["N_E"]].shape[1]
        e_rate = e_count / float(self._params["N_E"]) / float(self._params["simtime"]) * 1000.0
        i_rate = i_count / float(self._params["N_I"]) / float(self._params["simtime"]) * 1000.0
        return e_rate, i_rate

    def set_I_x(self, I_XE, I_XI):
        """Set DC currents for excitatory and inhibitory neurons
        Adds DC currents for the excitatory and inhibitory neurons.
        The DC currents are added to the currents already
        present in the populations.

        Parameters
        ----------
        I_XE: float
            extra DC current for excitatory neurons [pA]
        I_XI: float
            extra DC current for inhibitory neurons [pA]
        """
        for E_pop in self._populations[0]:
            I_e_loc = E_pop.get("I_e")
            E_pop.set({"I_e": I_e_loc + I_XE})
        for I_pop in self._populations[1]:
            I_e_loc = I_pop.get("I_e")
            I_pop.set({"I_e": I_e_loc + I_XI})

    def get_simulation(self, PathSpikes=None):
        """Create network, simulate and return results

        Creates the EI-clustered network and simulates it with
        the parameters supplied in the object creation.
        Returns a dictionary with firing rates,
        timing information (dict) and parameters (dict).
        If PathSpikes is supplied the spikes get saved to a pickle file.

        Parameters
        ----------
        PathSpikes: str (optional)
            Path of file for spiketimes, if None, no file is saved

        Returns
        -------
        dict
         Dictionary with firing rates,
         spiketimes (ndarray) and parameters (dict)
        """

        self.setup_network()
        self.simulate()
        spiketimes = self.get_recordings()
        e_rate, i_rate = self.get_firing_rates(spiketimes)

        if PathSpikes is not None:
            with open(PathSpikes, "wb") as outfile:
                pickle.dump(spiketimes, outfile)
        return {
            "e_rate": e_rate,
            "i_rate": i_rate,
            "_params": self.get_parameter(),
            "spiketimes": spiketimes,
        }
