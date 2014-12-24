# -*- coding: utf-8 -*-
#
# brunel2000_classes.py
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
import nest.raster_plot
import pylab
import os

nest.ResetKernel()

class Brunel2000:
    """
    Implementation of the sparsely connected random network, described
    by Brunel (2000) J. Comp. Neurosci.  Parameters are chosen for the
    asynchronous irregular state (AI).
    """
    g     = 5.0    # Ratio of IPSP to EPSP amplitude: J_I/J_E
    eta   = 2.0    # rate of external population in multiples of threshold rate
    delay = 1.5    # synaptic delay in ms
    tau_m = 20.0   # Membrane time constant in mV
    V_th  = 20.0   # Spike threshold in mV
    N_E   = 8000
    N_I   = 2000
    J_E   = 0.1
    threads=2
    N_rec   = 50    # Number of neurons to record from
    built=False
    connected=False

    def __init__(self):
        """
        Initialize an object of this class.
        """
        self.name=self.__class__.__name__
        self.data_path=self.name+"/"
        if not os.path.exists(self.data_path):
            os.makedirs(self.data_path)
        print("Writing data to: {0}".format(self.data_path))
        nest.ResetKernel()
        nest.SetKernelStatus({"data_path": self.data_path})

    def calibrate(self):
        """
        Compute all parameter dependent variables of the
        model.
        """
        self.N_neurons = self.N_E+self.N_I
        self.C_E    = self.N_E/10
        self.C_I    = self.N_I/10
        self.J_I    = -self.g*self.J_E
        self.nu_ex  = self.eta* self.V_th/(self.J_E*self.C_E*self.tau_m)
        self.p_rate = 1000.0*self.nu_ex*self.C_E
        nest.SetKernelStatus({"print_time": True,
                              "local_num_threads":self.threads})
        nest.SetDefaults("iaf_psc_delta", 
                         {"C_m": 1.0,
                          "tau_m": self.tau_m,
                          "t_ref": 2.0,
                          "E_L": 0.0,
                          "V_th": self.V_th,
                          "V_reset": 10.0})

    def build(self):
        """
        Create all nodes, used in the model.
        """
        if self.built==True: return
        self.calibrate()
        self.nodes   = nest.Create("iaf_psc_delta",self.N_neurons)
        self.noise=nest.Create("poisson_generator",1,{"rate": self.p_rate})
        self.spikes=nest.Create("spike_detector",2, 
                                [{"label": "brunel-py-ex"},
                                 {"label": "brunel-py-in"}])
        self.nodes_E= self.nodes[:self.N_E]
        self.nodes_I= self.nodes[self.N_E:]
        self.spikes_E=self.spikes[:1]
        self.spikes_I=self.spikes[1:]
        self.built=True

    def connect(self):
        """
        Connect all nodes in the model.
        """
        if self.connected: return
        if not self.built:
            self.build()

        nest.CopyModel("static_synapse_hom_wd",
                       "excitatory",
                       {"weight":self.J_E, 
                        "delay":self.delay})
        nest.CopyModel("static_synapse_hom_wd",
                       "inhibitory",
                       {"weight":self.J_I, 
                        "delay":self.delay})

        nest.Connect(self.nodes_E, self.nodes,
                     {"rule": 'fixed_indegree', "indegree": self.C_E},
                     "excitatory")
        nest.Connect(self.nodes_I, self.nodes,
                     {"rule": 'fixed_indegree', "indegree": self.C_I},
                     "inhibitory")
        nest.Connect(self.noise, self.nodes, syn_spec="excitatory")

        nest.Connect(self.nodes_E[:self.N_rec], self.spikes_E)
        nest.Connect(self.nodes_I[:self.N_rec], self.spikes_I)

        self.connected=True

    def run(self, simtime=300.):
        """
        Simulate the model for simtime milliseconds and print the
        firing rates of the network during htis period.  
        """
        if not self.connected:
            self.connect()
        nest.Simulate(simtime)
        events = nest.GetStatus(self.spikes,"n_events")
        self.rate_ex= events[0]/simtime*1000.0/self.N_rec
        print("Excitatory rate   : %.2f Hz" % self.rate_ex)
        self.rate_in= events[1]/simtime*1000.0/self.N_rec
        print("Inhibitory rate   : %.2f Hz" % self.rate_in)
        nest.raster_plot.from_device(self.spikes_E, hist=True)
        #pylab.show()


class Brunel_randomized(Brunel2000):
    """
    Like Brunel2000, but with randomized connection weights.
    """
    def connect(self):
        """
        Connect nodes with randomized weights.
        """
        # Code for randomized connections follows

class Brunel_balanced(Brunel2000):
    """
    Exact balance of excitation and inhibition
    """
    g=4
