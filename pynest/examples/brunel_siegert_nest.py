# -*- coding: utf-8 -*-
#
# brunel_siegert_nest.py
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

"""Mean-field theory for random balanced network
---------------------------------------------------

This script performs a mean-field analysis of the spiking network of
excitatory and an inhibitory population of leaky-integrate-and-fire neurons
simulated in ``brunel_delta_nest.py``. We refer to this spiking network of LIF
neurons with 'SLIFN'.

The self-consistent equation for the population-averaged firing rates
(eq.27 in [1]_, [2]_) is solved by integrating a pseudo-time dynamics
(eq.30 in [1]_). The latter constitutes a network of rate neurons, which is
simulated here. The asymptotic rates, i.e., the fixed points of the
dynamics (eq.30), are the prediction for the population and
time-averaged from the spiking simulation.

References
~~~~~~~~~~~~~~

.. [1] Hahne J, Dahmen D, Schuecker J, Frommer A, Bolten M,
       Helias M and Diesmann M. (2017).  Integration of continuous-time
       dynamics in a spiking neural network simulator. Front. Neuroinform.
       11:34. doi: 10.3389/fninf.2017.00034

.. [2] Schuecker J, Schmidt M, van Albada SJ, Diesmann M.
       and Helias, M. (2017). Fundamental activity constraints lead
       to specific interpretations of the connectome.
       PLOS Computational Biology 13(2): e1005179.
       https://doi.org/10.1371/journal.pcbi.1005179

"""

import nest
import numpy

nest.ResetKernel()

###############################################################################
# Assigning the simulation parameters to variables.


dt = 0.1  # the resolution in ms
simtime = 50.0  # Simulation time in ms

###############################################################################
# Definition of the network parameters in the SLIFN

g = 5.0  # ratio inhibitory weight/excitatory weight
eta = 2.0  # external rate relative to threshold rate
epsilon = 0.1  # connection probability

###############################################################################
# Definition of the number of neurons and connections in the SLIFN, needed
# for the connection strength in the Siegert neuron network

order = 2500
NE = 4 * order  # number of excitatory neurons
NI = 1 * order  # number of inhibitory neurons
CE = int(epsilon * NE)  # number of excitatory synapses per neuron
CI = int(epsilon * NI)  # number of inhibitory synapses per neuron
C_tot = int(CI + CE)  # total number of synapses per neuron

###############################################################################
# Initialization of the parameters of the Siegert neuron and the connection
# strength. The parameter are equivalent to the LIF-neurons in the SLIFN.

tauMem = 20.0  # time constant of membrane potential in ms
theta = 20.0  # membrane threshold potential in mV
neuron_params = {'tau_m': tauMem,
                 't_ref': 2.0,
                 'theta': theta,
                 'V_reset': 0.0,
                 }

J = 0.1  # postsynaptic amplitude in mV in the SLIFN
J_ex = J  # amplitude of excitatory postsynaptic potential
J_in = -g * J_ex  # amplitude of inhibitory postsynaptic potential
# drift_factor in diffusion connections (see [1], eq. 28) for external
# drive, excitatory and inhibitory neurons
drift_factor_ext = tauMem * 1e-3 * J_ex
drift_factor_ex = tauMem * 1e-3 * CE * J_ex
drift_factor_in = tauMem * 1e-3 * CI * J_in
# diffusion_factor for diffusion connections (see [1], eq. 29)
diffusion_factor_ext = tauMem * 1e-3 * J_ex ** 2
diffusion_factor_ex = tauMem * 1e-3 * CE * J_ex ** 2
diffusion_factor_in = tauMem * 1e-3 * CI * J_in ** 2

###############################################################################
# External drive, this is equivalent to the drive in the SLIFN

nu_th = theta / (J * CE * tauMem)
nu_ex = eta * nu_th
p_rate = 1000.0 * nu_ex * CE

###############################################################################
# Configuration of the simulation kernel by the previously defined time
# resolution used in the simulation. Setting ``print_time`` to `True` prints the
# already processed simulation time as well as its percentage of the total
# simulation time.

nest.SetKernelStatus({"resolution": dt, "print_time": True,
                      "overwrite_files": True})

print("Building network")

###############################################################################
# Configuration of the model ``siegert_neuron`` using ``SetDefaults``.

nest.SetDefaults("siegert_neuron", neuron_params)

###############################################################################
# Creation of the nodes using ``Create``. One rate neuron represents the
# excitatory population of LIF-neurons in the SLIFN and one the inhibitory
# population assuming homogeneity of the populations.

siegert_ex = nest.Create("siegert_neuron", 1)
siegert_in = nest.Create("siegert_neuron", 1)

###############################################################################
# The Poisson drive in the SLIFN is replaced by a driving rate neuron,
# which does not receive input from other neurons. The activity of the rate
# neuron is controlled by setting ``mean`` to the rate of the corresponding
# poisson generator in the SLIFN.

siegert_drive = nest.Create('siegert_neuron', 1, params={'mean': p_rate})

###############################################################################
# To record from the rate neurons a multimeter is created and the parameter
# ``record_from`` is set to `rate` as well as the recording interval to `dt`

multimeter = nest.Create(
    'multimeter', params={'record_from': ['rate'], 'interval': dt})

###############################################################################
# Connections between ``Siegert neurons`` are realized with the synapse model
# ``diffusion_connection``. These two parameters reflect the prefactors in
# front of the rate variable in eq. 27-29 in [1].

###############################################################################
# Connections originating from the driving neuron


syn_dict = {'drift_factor': drift_factor_ext,
            'diffusion_factor': diffusion_factor_ext,
            'synapse_model': 'diffusion_connection'}

nest.Connect(
    siegert_drive, siegert_ex + siegert_in, 'all_to_all', syn_dict)
nest.Connect(multimeter, siegert_ex + siegert_in)

###############################################################################
# Connections originating from the excitatory neuron


syn_dict = {'drift_factor': drift_factor_ex, 'diffusion_factor':
            diffusion_factor_ex, 'synapse_model': 'diffusion_connection'}
nest.Connect(siegert_ex, siegert_ex + siegert_in, 'all_to_all', syn_dict)

###############################################################################
# Connections originating from the inhibitory neuron

syn_dict = {'drift_factor': drift_factor_in, 'diffusion_factor':
            diffusion_factor_in, 'synapse_model': 'diffusion_connection'}
nest.Connect(siegert_in, siegert_ex + siegert_in, 'all_to_all', syn_dict)

###############################################################################
# Simulate the network

nest.Simulate(simtime)

###################################################################################
# Analyze the activity data. The asymptotic rate of the Siegert neuron
# corresponds to the population- and time-averaged activity in the SLIFN.
# For the symmetric network setup used here, the excitatory and inhibitory
# rates are identical. For comparison execute the example ``brunel_delta_nest.py``.

data = multimeter.events
rates_ex = data['rate'][numpy.where(data['senders'] == siegert_ex.global_id)]
rates_in = data['rate'][numpy.where(data['senders'] == siegert_in.global_id)]
times = data['times'][numpy.where(data['senders'] == siegert_in.global_id)]
print("Excitatory rate   : %.2f Hz" % rates_ex[-1])
print("Inhibitory rate   : %.2f Hz" % rates_in[-1])
