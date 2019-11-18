# -*- coding: utf-8 -*-
#
# pulsepacket.py
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
Pulse packet example
--------------------

This script compares the average and individual membrane potential excursions
in response to a single pulse packet with an analytically acquired voltage
trace (see: Diesmann [1]_)
A pulse packet is a transient spike volley with a Gaussian rate profile.
The user can specify the neural parameters, the parameters of the
pulse-packet and the number of trials.


References
~~~~~~~~~~~~

.. [1] Diesmann M. 2002. Dissertation. Conditions for stable propagation of
       synchronous spiking in cortical neural networks: Single neuron dynamics
       and network properties.
       http://d-nb.info/968772781/34.

"""


###############################################################################
# First, we import all necessary modules for simulation, analysis and
# plotting.

import nest
import numpy
import pylab

# Properties of pulse packet:

a = 100            # number of spikes in one pulse packet
sdev = 10.         # width of pulse packet (ms)
weight = 0.1       # PSP amplitude (mV)
pulsetime = 500.   # occurrence time (center) of pulse-packet (ms)


# Network and neuron characteristics:

n_neurons = 100    # number of neurons
cm = 200.          # membrane capacitance (pF)
tau_s = 0.5        # synaptic time constant (ms)
tau_m = 20.        # membrane time constant (ms)
V0 = 0.0           # resting potential (mV)
Vth = numpy.inf    # firing threshold, high value to avoid spiking


# Simulation and analysis parameters:

simtime = 1000.               # how long we simulate (ms)
simulation_resolution = 0.1  # (ms)
sampling_resolution = 1.   # for voltmeter (ms)
convolution_resolution = 1.   # for the analytics (ms)


# Some parameters in base units.

Cm = cm * 1e-12            # convert to Farad
Weight = weight * 1e-12    # convert to Ampere
Tau_s = tau_s * 1e-3       # convert to sec
Tau_m = tau_m * 1e-3       # convert to sec
Sdev = sdev * 1e-3         # convert to sec
Convolution_resolution = convolution_resolution * 1e-3  # convert to sec


###############################################################################
# This function calculates the membrane potential excursion in response
# to a single input spike (the equation is given for example in Diesmann [1]_,
# eq.2.3).
# It expects:
#
# * ``Time``: a time array or a single time point (in sec)
# * ``Tau_s`` and ``Tau_m``: the synaptic and the membrane time constant (in sec)
# * ``Cm``: the membrane capacity (in Farad)
# * ``Weight``: the synaptic weight (in Ampere)
#
# It returns the provoked membrane potential (in mV)

def make_psp(Time, Tau_s, Tau_m, Cm, Weight):
    term1 = (1 / Tau_s - 1 / Tau_m)
    term2 = numpy.exp(-Time / Tau_s)
    term3 = numpy.exp(-Time / Tau_m)
    PSP = (Weight / Cm * numpy.exp(1) / Tau_s *
           (((-Time * term2) / term1) + (term3 - term2) / term1 ** 2))
    return PSP * 1e3


###############################################################################
# This function finds the exact location of the maximum of the PSP caused by a
# single input spike. The location is obtained by setting the first derivative
# of the equation for the PSP (see ``make_psp()``) to zero. The resulting
# equation can be expressed in terms of a `LambertW function`. This function is
# implemented in nest as a .sli file. In order to access this function in
# PyNEST we called the function ``nest.sli_func()``.
# This function expects:
#
# * ``Tau_s`` and ``Tau_m``: the synaptic and membrane time constant (in sec)
#
# It returns the location of the maximum (in sec)

def find_loc_pspmax(tau_s, tau_m):
    var = tau_m / tau_s
    lam = nest.ll_api.sli_func('LambertWm1', -numpy.exp(-1 / var) / var)
    t_maxpsp = (-var * lam - 1) / var / (1 / tau_s - 1 / tau_m) * 1e-3
    return t_maxpsp


###############################################################################
# First, we construct a Gaussian kernel for a given standard derivation
# (``sig``) and mean value (``mu``). In this case the standard derivation is
# the width of the pulse packet (see [1]_).

sig = Sdev
mu = 0.0
x = numpy.arange(-4 * sig, 4 * sig, Convolution_resolution)
term1 = 1 / (sig * numpy.sqrt(2 * numpy.pi))
term2 = numpy.exp(-(x - mu) ** 2 / (sig ** 2 * 2))
gauss = term1 * term2 * Convolution_resolution


###############################################################################
# Second, we calculate the PSP of a neuron due to a single spiking input.
# (see Diesmann 2002, eq. 2.3).
# Since we do that in discrete time steps, we first construct an array
# (``t_psp``) that contains the time points we want to consider. Then, the
# function ``make_psp()`` (that creates the PSP) takes the time array as its
# first argument.

t_psp = numpy.arange(0, 10 * (Tau_m + Tau_s), Convolution_resolution)
psp = make_psp(t_psp, Tau_s, Tau_m, Cm, Weight)


###############################################################################
# Now, we want to normalize the PSP amplitude to one. We therefore have to
# divide the PSP by its maximum ([1]_ sec 6.1). The function
# ``find_loc_pspmax()`` returns the exact time point (``t_pspmax``) when we
# expect the maximum to occur. The function ``make_psp()`` calculates the
# corresponding PSP value, which is our PSP amplitude (``psp_amp``).

t_pspmax = find_loc_pspmax(Tau_s, Tau_m)
psp_amp = make_psp(t_pspmax, Tau_s, Tau_m, Cm, Weight)
psp_norm = psp / psp_amp


###############################################################################
# Now we have all ingredients to compute the membrane potential excursion
# (`U`). This calculation implies a convolution of the Gaussian with the
# normalized PSP (see [1]_, eq. 6.9). In order to avoid an offset in the
# convolution, we need to add a pad of zeros on the left side of the
# normalized PSP. Later on we want to compare our analytical results with the
# simulation outcome. Therefore we need a time vector (`t_U`) with the correct
# temporal resolution, which places the excursion of the potential at the
# correct time.
psp_norm = numpy.pad(psp_norm, [len(psp_norm) - 1, 1])
U = a * psp_amp * pylab.convolve(gauss, psp_norm)
l = len(U)
t_U = (convolution_resolution * numpy.linspace(-l / 2., l / 2., l) +
       pulsetime + 1.)


###############################################################################
# In this section we simulate a network of multiple neurons.
# All these neurons receive an individual pulse packet that is drawn from a
# Gaussian distribution.
#
# We reset the Kernel, define the simulation resolution and set the
# verbosity using ``set_verbosity`` to suppress info messages.

nest.ResetKernel()
nest.SetKernelStatus({'resolution': simulation_resolution})
nest.set_verbosity("M_WARNING")


###############################################################################
# Afterwards we create several neurons, the same amount of
# pulse-packet-generators and a voltmeter. All these nodes/devices
# have specific properties that are specified in device specific
# dictionaries (here: `neuron_pars` for the neurons, `ppg_pars`
# for the and pulse-packet-generators and `vm_pars` for the voltmeter).

neuron_pars = {
    'V_th': Vth,
    'tau_m': tau_m,
    'tau_syn_ex': tau_s,
    'C_m': cm,
    'E_L': V0,
    'V_reset': V0,
    'V_m': V0
}
neurons = nest.Create('iaf_psc_alpha', n_neurons, neuron_pars)
ppg_pars = {
    'pulse_times': [pulsetime],
    'activity': a,
    'sdev': sdev
}
ppgs = nest.Create('pulsepacket_generator', n_neurons, ppg_pars)
vm_pars = {'interval': sampling_resolution}
vm = nest.Create('voltmeter', 1, vm_pars)


###############################################################################
# Now, we connect each pulse generator to one neuron via static synapses.
# We want to keep all properties of the static synapse constant except the
# synaptic weight. Therefore we change the weight with  the help of the command
# ``SetDefaults``.
# The command ``Connect`` connects all kinds of nodes/devices. Since multiple
# nodes/devices can be connected in different ways e.g., each source connects
# to all targets, each source connects to a subset of targets or each source
# connects to exactly one target, we have to specify the connection. In our
# case we use the ``one_to_one`` connection routine since we connect one pulse
# generator (source) to one neuron (target).
# In addition we also connect the `voltmeter` to the `neurons`.

nest.SetDefaults('static_synapse', {'weight': weight})
nest.Connect(ppgs, neurons, 'one_to_one')
nest.Connect(vm, neurons)


###############################################################################
# In the next step we run the simulation for a given duration in ms.

nest.Simulate(simtime)


###############################################################################
# Finally, we record the membrane potential, when it occurred and to which
# neuron it belongs. We obtain this information using the command
# ``nest.GetStatus(vm, 'events')[0]``. The sender and the time point of a voltage
# data point at position x in the voltage array (``V_m``), can be found at the
# same position x in the sender (`senders`) and the time array (`times`).

Vm = vm.get('events', 'V_m')
times = vm.get('events', 'times')
senders = vm.get('events', 'senders')


###############################################################################
# Here we plot the membrane potential derived from the theory and from the
# simulation. Since we simulate multiple neurons that received slightly
# different pulse packets, we plot the individual and the averaged membrane
# potentials.
#
# We plot the analytical solution U (the resting potential V0 shifts the
# membrane potential up or downwards).

pylab.plot(t_U, U + V0, 'r', lw=2, zorder=3, label='analytical solution')


###############################################################################
# Then we plot all individual membrane potentials.
# The time axes is the range of the simulation time in steps of ms.

Vm_single = [Vm[senders == n.get('global_id')] for n in neurons]
simtimes = numpy.arange(1, simtime)
for idn in range(n_neurons):
    if idn == 0:
        pylab.plot(simtimes, Vm_single[idn], 'gray',
                   zorder=1, label='single potentials')
    else:
        pylab.plot(simtimes, Vm_single[idn], 'gray', zorder=1)


###############################################################################
# Finally, we plot the averaged membrane potential.

Vm_average = numpy.mean(Vm_single, axis=0)
pylab.plot(simtimes, Vm_average, 'b', lw=4,
           zorder=2, label='averaged potential')
pylab.legend()
pylab.xlabel('time (ms)')
pylab.ylabel('membrane potential (mV)')
pylab.xlim((-5 * (tau_m + tau_s) + pulsetime,
            10 * (tau_m + tau_s) + pulsetime))
pylab.show()
