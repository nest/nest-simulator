# -*- coding: utf-8 -*-
#
# intrinsic_currents_spiking.py
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
#

"""Intrinsic currents spiking
-------------------------------

This example illustrates a neuron receiving spiking input through
several different receptors (AMPA, NMDA, GABA_A, GABA_B), provoking
spike output. The model, ``ht_neuron``, also has intrinsic currents
(``I_NaP``, ``I_KNa``, ``I_T``, and ``I_h``). It is a slightly simplified implementation of
neuron model proposed in [1]_.

The neuron is bombarded with spike trains from four Poisson generators,
which are connected to the AMPA, NMDA, GABA_A, and GABA_B receptors,
respectively.

References
~~~~~~~~~~~

.. [1] Hill and Tononi (2005) Modeling sleep and wakefulness in the
       thalamocortical system. J Neurophysiol 93:1671
       http://dx.doi.org/10.1152/jn.00915.2004.

See Also
~~~~~~~~~~

:doc:`intrinsic_currents_subthreshold`

"""

###############################################################################
# We imported all necessary modules for simulation, analysis and plotting.

import nest
import numpy as np
import matplotlib.pyplot as plt

###############################################################################
# Additionally, we set the verbosity using ``set_verbosity`` to suppress info
# messages. We also reset the kernel to be sure to start with a clean NEST.

nest.set_verbosity("M_WARNING")
nest.ResetKernel()

###############################################################################
# We define the simulation parameters:
#
# - The rate of the input spike trains
# - The weights of the different receptors (names must match receptor types)
# - The time to simulate
#
# Note that all parameter values should be doubles, since NEST expects doubles.

rate_in = 100.
w_recep = {'AMPA': 30., 'NMDA': 30., 'GABA_A': 5., 'GABA_B': 10.}
t_sim = 250.

num_recep = len(w_recep)

###############################################################################
# We create
#
# - one neuron instance
# - one Poisson generator instance for each synapse type
# - one multimeter to record from the neuron:

#   - membrane potential
#   - threshold potential
#   - synaptic conductances
#   - intrinsic currents
#
# See :doc:`intrinsic_currents_subthreshold` for more details on ``multimeter``
# configuration.

nrn = nest.Create('ht_neuron')
p_gens = nest.Create('poisson_generator', 4, params={'rate': rate_in})
mm = nest.Create('multimeter',
                 params={'interval': 0.1,
                         'record_from': ['V_m', 'theta',
                                         'g_AMPA', 'g_NMDA',
                                         'g_GABA_A', 'g_GABA_B',
                                         'I_NaP', 'I_KNa', 'I_T', 'I_h']})

###############################################################################
# We now connect each Poisson generator with the neuron through a different
# receptor type.
#
# First, we need to obtain the numerical codes for the receptor types from
# the model. The ``receptor_types`` entry of the default dictionary for the
# ``ht_neuron`` model is a dictionary mapping receptor names to codes.
#
# In the loop, we use Python's tuple unpacking mechanism to unpack
# dictionary entries from our `w_recep` dictionary.
#
# Note that we need to pack the `pg` variable into a list before
# passing it to ``Connect``, because iterating over the `p_gens` list
# makes `pg` a "naked" node ID.

receptors = nest.GetDefaults('ht_neuron')['receptor_types']
for index, (rec_name, rec_wgt) in enumerate(w_recep.items()):
    nest.Connect(p_gens[index], nrn, syn_spec={'receptor_type': receptors[rec_name], 'weight': rec_wgt})

###############################################################################
# We then connnect the ``multimeter``. Note that the multimeter is connected to
# the neuron, not the other way around.

nest.Connect(mm, nrn)

###############################################################################
# We are now ready to simulate.

nest.Simulate(t_sim)

###############################################################################
# We now fetch the data recorded by the multimeter. The data are returned as
# a dictionary with entry ``times`` containing timestamps for all
# recorded data, plus one entry per recorded quantity.
# All data is contained in the ``events`` entry of the status dictionary
# returned by the multimeter. Because all NEST function return arrays,
# we need to pick out element `0` from the result of ``GetStatus``.

data = nest.GetStatus(mm)[0]['events']
t = data['times']

###############################################################################
# The following function turns a name such as ``I_NaP`` into proper TeX code
# :math:`I_{\mathrm{NaP}}` for a pretty label.


def texify_name(name):
    return r'${}_{{\mathrm{{{}}}}}$'.format(*name.split('_'))

###############################################################################
# The next step is to plot the results. We create a new figure, and add one
# subplot each for membrane and threshold potential, synaptic conductances,
# and intrinsic currents.


fig = plt.figure()

Vax = fig.add_subplot(311)
Vax.plot(t, data['V_m'], 'b', lw=2, label=r'$V_m$')
Vax.plot(t, data['theta'], 'g', lw=2, label=r'$\Theta$')
Vax.set_ylabel('Potential [mV]')

try:
    Vax.legend(fontsize='small')
except TypeError:
    Vax.legend()  # work-around for older Matplotlib versions
Vax.set_title('ht_neuron driven by Poisson processes')

Gax = fig.add_subplot(312)
for gname in ('g_AMPA', 'g_NMDA', 'g_GABA_A', 'g_GABA_B'):
    Gax.plot(t, data[gname], lw=2, label=texify_name(gname))

try:
    Gax.legend(fontsize='small')
except TypeError:
    Gax.legend()  # work-around for older Matplotlib versions
Gax.set_ylabel('Conductance [nS]')

Iax = fig.add_subplot(313)
for iname, color in (('I_h', 'maroon'), ('I_T', 'orange'),
                     ('I_NaP', 'crimson'), ('I_KNa', 'aqua')):
    Iax.plot(t, data[iname], color=color, lw=2, label=texify_name(iname))

try:
    Iax.legend(fontsize='small')
except TypeError:
    Iax.legend()  # work-around for older Matplotlib versions
Iax.set_ylabel('Current [pA]')
Iax.set_xlabel('Time [ms]')
