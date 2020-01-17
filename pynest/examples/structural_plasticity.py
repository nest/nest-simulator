# -*- coding: utf-8 -*-
#
# structural_plasticity.py
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
Structural Plasticity example
----------------------------------
This example shows a simple network of two populations where structural
plasticity is used. The network has 1000 neurons, 80% excitatory and
20% inhibitory. The simulation starts without any connectivity. A set of
homeostatic rules are defined, according to which structural plasticity will
create and delete synapses dynamically during the simulation until a desired
level of electrical activity is reached. The model of structural plasticity
used here corresponds to the formulation presented in [1]_.

At the end of the simulation, a plot of the evolution of the connectivity
in the network and the average calcium concentration in the neurons is created.

References
~~~~~~~~~~~
.. [1] Butz, M., and van Ooyen, A. (2013). A simple rule for dendritic spine and axonal bouton formation can
       account for cortical reorganization after focal retinal lesions. PLoS Comput. Biol. 9 (10), e1003259.

"""

####################################################################################
# First, we have import all necessary modules.

import nest
import numpy
import matplotlib.pyplot as pl
import sys


####################################################################################
# We define general simulation parameters

class StructralPlasticityExample:

    def __init__(self):
        # simulated time (ms)
        self.t_sim = 200000.0
        # simulation step (ms).
        self.dt = 0.1
        self.number_excitatory_neurons = 800
        self.number_inhibitory_neurons = 200

        # Structural_plasticity properties
        self.update_interval = 10000.0
        self.record_interval = 1000.0
        # rate of background Poisson input
        self.bg_rate = 10000.0
        self.neuron_model = 'iaf_psc_exp'

####################################################################################
# In this implementation of structural plasticity, neurons grow
# connection points called synaptic elements. Synapses can be created
# between compatible synaptic elements. The growth of these elements is
# guided by homeostatic rules, defined as growth curves.
# Here we specify the growth curves for synaptic elements of excitatory
# and inhibitory neurons.

        # Excitatory synaptic elements of excitatory neurons
        self.growth_curve_e_e = {
            'growth_curve': "gaussian",
            'growth_rate': 0.0001,  # (elements/ms)
            'continuous': False,
            'eta': 0.0,  # Ca2+
            'eps': 0.05,  # Ca2+
        }

        # Inhibitory synaptic elements of excitatory neurons
        self.growth_curve_e_i = {
            'growth_curve': "gaussian",
            'growth_rate': 0.0001,  # (elements/ms)
            'continuous': False,
            'eta': 0.0,  # Ca2+
            'eps': self.growth_curve_e_e['eps'],  # Ca2+
        }

        # Excitatory synaptic elements of inhibitory neurons
        self.growth_curve_i_e = {
            'growth_curve': "gaussian",
            'growth_rate': 0.0004,  # (elements/ms)
            'continuous': False,
            'eta': 0.0,  # Ca2+
            'eps': 0.2,  # Ca2+
        }

        # Inhibitory synaptic elements of inhibitory neurons
        self.growth_curve_i_i = {
            'growth_curve': "gaussian",
            'growth_rate': 0.0001,  # (elements/ms)
            'continuous': False,
            'eta': 0.0,  # Ca2+
            'eps': self.growth_curve_i_e['eps']  # Ca2+
        }

        # Now we specify the neuron model.

        self.model_params = {'tau_m': 10.0,  # membrane time constant (ms)
                             # excitatory synaptic time constant (ms)
                             'tau_syn_ex': 0.5,
                             # inhibitory synaptic time constant (ms)
                             'tau_syn_in': 0.5,
                             't_ref': 2.0,  # absolute refractory period (ms)
                             'E_L': -65.0,  # resting membrane potential (mV)
                             'V_th': -50.0,  # spike threshold (mV)
                             'C_m': 250.0,  # membrane capacitance (pF)
                             'V_reset': -65.0  # reset potential (mV)
                             }

        self.nodes_e = None
        self.nodes_i = None
        self.mean_ca_e = []
        self.mean_ca_i = []
        self.total_connections_e = []
        self.total_connections_i = []


####################################################################################
# We initialize variables for the post-synaptic currents of the
# excitatory, inhibitory, and external synapses. These values were
# calculated from a PSP amplitude of 1 for excitatory synapses,
# -1 for inhibitory synapses and 0.11 for external synapses.

        self.psc_e = 585.0
        self.psc_i = -585.0
        self.psc_ext = 6.2

    def prepare_simulation(self):
        nest.ResetKernel()
        nest.set_verbosity('M_ERROR')

####################################################################################
# We set global kernel parameters. Here we define the resolution
# for the simulation, which is also the time resolution for the update
# of the synaptic elements.

        nest.SetKernelStatus(
            {
                'resolution': self.dt
            }
        )


####################################################################################
# Set Structural Plasticity synaptic update interval which is how often
# the connectivity will be updated inside the network. It is important
# to notice that synaptic elements and connections change on different
# time scales.

        nest.SetStructuralPlasticityStatus({
            'structural_plasticity_update_interval': self.update_interval,
        })


####################################################################################
# Now we define Structural Plasticity synapses. In this example we create
# two synapse models, one for excitatory and one for inhibitory synapses.
# Then we define that excitatory synapses can only be created between a
# pre-synaptic element called `Axon_ex` and a post synaptic element
# called `Den_ex`. In a similar manner, synaptic elements for inhibitory
# synapses are defined.

        nest.CopyModel('static_synapse', 'synapse_ex')
        nest.SetDefaults('synapse_ex', {'weight': self.psc_e, 'delay': 1.0})
        nest.CopyModel('static_synapse', 'synapse_in')
        nest.SetDefaults('synapse_in', {'weight': self.psc_i, 'delay': 1.0})
        nest.SetStructuralPlasticityStatus({
            'structural_plasticity_synapses': {
                'synapse_ex': {
                    'synapse_model': 'synapse_ex',
                    'post_synaptic_element': 'Den_ex',
                    'pre_synaptic_element': 'Axon_ex',
                },
                'synapse_in': {
                    'synapse_model': 'synapse_in',
                    'post_synaptic_element': 'Den_in',
                    'pre_synaptic_element': 'Axon_in',
                },
            }
        })

    def create_nodes(self):

####################################################################################
# Now we assign the growth curves to the corresponding synaptic elements

        synaptic_elements = {
            'Den_ex': self.growth_curve_e_e,
            'Den_in': self.growth_curve_e_i,
            'Axon_ex': self.growth_curve_e_e,
        }

        synaptic_elements_i = {
            'Den_ex': self.growth_curve_i_e,
            'Den_in': self.growth_curve_i_i,
            'Axon_in': self.growth_curve_i_i,
        }


####################################################################################
# Then it is time to create a population with 80% of the total network
# size excitatory neurons and another one with 20% of the total network
# size of inhibitory neurons.

        self.nodes_e = nest.Create('iaf_psc_alpha',
                                   self.number_excitatory_neurons,
                                   {'synaptic_elements': synaptic_elements})

        self.nodes_i = nest.Create('iaf_psc_alpha',
                                   self.number_inhibitory_neurons,
                                   {'synaptic_elements': synaptic_elements_i})
        nest.SetStatus(self.nodes_e, 'synaptic_elements', synaptic_elements)
        nest.SetStatus(self.nodes_i, 'synaptic_elements', synaptic_elements_i)

    def connect_external_input(self):
        """
        We create and connect the Poisson generator for external input
        """
        noise = nest.Create('poisson_generator')
        nest.SetStatus(noise, {"rate": self.bg_rate})
        nest.Connect(noise, self.nodes_e, 'all_to_all',
                     {'weight': self.psc_ext, 'delay': 1.0})
        nest.Connect(noise, self.nodes_i, 'all_to_all',
                     {'weight': self.psc_ext, 'delay': 1.0})


####################################################################################
# In order to save the amount of average calcium concentration in each
# population through time we create the function ``record_ca``. Here we use the
# ``GetStatus`` function to retrieve the value of `Ca` for every neuron in the
# network and then store the average.

    def record_ca(self):
        ca_e = nest.GetStatus(self.nodes_e, 'Ca'),  # Calcium concentration
        self.mean_ca_e.append(numpy.mean(ca_e))

        ca_i = nest.GetStatus(self.nodes_i, 'Ca'),  # Calcium concentration
        self.mean_ca_i.append(numpy.mean(ca_i))


####################################################################################
# In order to save the state of the connectivity in the network through time
# we create the function ``record_connectivity``. Here we use the ``GetStatus``
# function to retrieve the number of connected pre-synaptic elements of each
# neuron. The total amount of excitatory connections is equal to the total
# amount of connected excitatory pre-synaptic elements. The same applies for
# inhibitory connections.

    def record_connectivity(self):
        syn_elems_e = nest.GetStatus(self.nodes_e, 'synaptic_elements')
        syn_elems_i = nest.GetStatus(self.nodes_i, 'synaptic_elements')
        self.total_connections_e.append(sum(neuron['Axon_ex']['z_connected']
                                            for neuron in syn_elems_e))
        self.total_connections_i.append(sum(neuron['Axon_in']['z_connected']
                                            for neuron in syn_elems_i))


####################################################################################
# We define a function to plot the recorded values
# at the end of the simulation.

    def plot_data(self):
        fig, ax1 = pl.subplots()
        ax1.axhline(self.growth_curve_e_e['eps'],
                    linewidth=4.0, color='#9999FF')
        ax1.plot(self.mean_ca_e, 'b',
                 label='Ca Concentration Excitatory Neurons', linewidth=2.0)
        ax1.axhline(self.growth_curve_i_e['eps'],
                    linewidth=4.0, color='#FF9999')
        ax1.plot(self.mean_ca_i, 'r',
                 label='Ca Concentration Inhibitory Neurons', linewidth=2.0)
        ax1.set_ylim([0, 0.275])
        ax1.set_xlabel("Time in [s]")
        ax1.set_ylabel("Ca concentration")
        ax2 = ax1.twinx()
        ax2.plot(self.total_connections_e, 'm',
                 label='Excitatory connections', linewidth=2.0, linestyle='--')
        ax2.plot(self.total_connections_i, 'k',
                 label='Inhibitory connections', linewidth=2.0, linestyle='--')
        ax2.set_ylim([0, 2500])
        ax2.set_ylabel("Connections")
        ax1.legend(loc=1)
        ax2.legend(loc=4)
        pl.savefig('StructuralPlasticityExample.eps', format='eps')


####################################################################################
# It is time to specify how we want to perform the simulation. In this
# function we first enable structural plasticity in the network and then we
# simulate in steps. On each step we record the calcium concentration and the
# connectivity. At the end of the simulation, the plot of connections and
# calcium concentration through time is generated.

    def simulate(self):
        if nest.NumProcesses() > 1:
            sys.exit("For simplicity, this example only works " +
                     "for a single process.")
        nest.EnableStructuralPlasticity()
        print("Starting simulation")
        sim_steps = numpy.arange(0, self.t_sim, self.record_interval)
        for i, step in enumerate(sim_steps):
            nest.Simulate(self.record_interval)
            self.record_ca()
            self.record_connectivity()
            if i % 20 == 0:
                print("Progress: " + str(i / 2) + "%")
        print("Simulation finished successfully")


####################################################################################
# Finally we take all the functions that we have defined and create the sequence
# for our example. We prepare the simulation, create the nodes for the network,
# connect the external input and then simulate. Please note that as we are
# simulating 200 biological seconds in this example, it will take a few minutes
# to complete.

if __name__ == '__main__':
    example = StructralPlasticityExample()
    # Prepare simulation
    example.prepare_simulation()
    example.create_nodes()
    example.connect_external_input()
    # Start simulation
    example.simulate()
    example.plot_data()
