# -*- coding: utf-8 -*-
#
# intrinsic_currents_subthreshold.py
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
Intrinsic currents subthreshold
-------------------------------

This example illustrates how to record from a model with multiple
intrinsic currents and visualize the results. This is illustrated
using the ``ht_neuron`` which has four intrinsic currents: ``I_NaP``,
``I_KNa``, ``I_T``, and ``I_h``. It is a slightly simplified implementation of
neuron model proposed in [1]_.

The neuron is driven by DC current, which is alternated
between depolarizing and hyperpolarizing. Hyperpolarization
intervals become increasingly longer.

References
~~~~~~~~~~

.. [1] Hill and Tononi (2005) Modeling Sleep and Wakefulness in the
       Thalamocortical System J Neurophysiol 93:1671
       http://dx.doi.org/10.1152/jn.00915.2004.

See Also
~~~~~~~~

:doc:`intrinsic_currents_spiking`

"""

###############################################################################
# We imported all necessary modules for simulation, analysis and plotting.

import nest
import matplotlib.pyplot as plt

###############################################################################
# Additionally, we set the verbosity using ``set_verbosity`` to suppress info
# messages. We also reset the kernel to be sure to start with a clean NEST.

nest.set_verbosity("M_WARNING")
nest.ResetKernel()

###############################################################################
# We define simulation parameters:
#
# - The length of depolarization intervals
# - The length of hyperpolarization intervals
# - The amplitude for de- and hyperpolarizing currents
# - The end of the time window to plot

n_blocks = 5
t_block = 20.
t_dep = [t_block] * n_blocks
t_hyp = [t_block * 2 ** n for n in range(n_blocks)]
I_dep = 10.
I_hyp = -5.

t_end = 500.

###############################################################################
# We create the one neuron instance and the DC current generator and store
# the returned handles.

nrn = nest.Create('ht_neuron')
dc = nest.Create('dc_generator')

###############################################################################
# We create a multimeter to record
#
# - membrane potential ``V_m``
# - threshold value ``theta``
# - intrinsic currents ``I_NaP``, ``I_KNa``, ``I_T``, ``I_h``
#
# by passing these names in the ``record_from`` list.
#
# To find out which quantities can be recorded from a given neuron,
# run::
#
#   nest.GetDefaults('ht_neuron')['recordables']
#
# The result will contain an entry like::
#
#   <SLILiteral: V_m>
#
# for each recordable quantity. You need to pass the value of the
# ``SLILiteral``, in this case ``V_m`` in the ``record_from`` list.
#
# We want to record values with 0.1 ms resolution, so we set the
# recording interval as well; the default recording resolution is 1 ms.

# create multimeter and configure it to record all information
# we want at 0.1 ms resolution
mm = nest.Create('multimeter',
                 params={'interval': 0.1,
                         'record_from': ['V_m', 'theta',
                                         'I_NaP', 'I_KNa', 'I_T', 'I_h']}
                 )

###############################################################################
# We connect the DC generator and the multimeter to the neuron. Note that
# the multimeter, just like the voltmeter is connected to the neuron,
# not the neuron to the multimeter.

nest.Connect(dc, nrn)
nest.Connect(mm, nrn)

###############################################################################
# We are ready to simulate. We alternate between driving the neuron with
# depolarizing and hyperpolarizing currents. Before each simulation
# interval, we set the amplitude of the DC generator to the correct value.

for t_sim_dep, t_sim_hyp in zip(t_dep, t_hyp):

    dc.amplitude = I_dep
    nest.Simulate(t_sim_dep)

    dc.amplitude = I_hyp
    nest.Simulate(t_sim_hyp)

###############################################################################
# We now fetch the data recorded by the multimeter. The data are returned as
# a dictionary with entry ``times`` containing timestamps for all recorded
# data, plus one entry per recorded quantity.
#
# All data is contained in the ``events`` entry of the status dictionary
# returned by the multimeter.

data = mm.events
t = data['times']

###############################################################################
# The next step is to plot the results. We create a new figure, add a single
# subplot and plot at first membrane potential and threshold.

fig = plt.figure()
Vax = fig.add_subplot(111)
Vax.plot(t, data['V_m'], 'b-', lw=2, label=r'$V_m$')
Vax.plot(t, data['theta'], 'g-', lw=2, label=r'$\Theta$')
Vax.set_ylim(-80., 0.)
Vax.set_ylabel('Voltageinf [mV]')
Vax.set_xlabel('Time [ms]')

###############################################################################
# To plot the input current, we need to create an input current trace. We
# construct it from the durations of the de- and hyperpolarizing inputs and
# add the delay in the connection between DC generator and neuron:
#
# * We find the delay by checking the status of the dc->nrn connection.
# * We find the resolution of the simulation from the kernel status.
# * Each current interval begins one time step after the previous interval,
#   is delayed by the delay and effective for the given duration.
# * We build the time axis incrementally. We only add the delay when adding
#   the first time point after t=0. All subsequent points are then
#   automatically shifted by the delay.

conns = nest.GetConnections(dc, nrn)
delay = conns.delay
dt = nest.GetKernelStatus('resolution')

t_dc, I_dc = [0], [0]

for td, th in zip(t_dep, t_hyp):
    t_prev = t_dc[-1]
    t_start_dep = t_prev + dt
    if t_prev == 0:
        t_start_dep += delay
    t_end_dep = t_start_dep + td
    t_start_hyp = t_end_dep + dt
    t_end_hyp = t_start_hyp + th

    t_dc.extend([t_start_dep, t_end_dep, t_start_hyp, t_end_hyp])
    I_dc.extend([I_dep, I_dep, I_hyp, I_hyp])

###############################################################################
# The following function turns a name such as ``I_NaP`` into proper TeX code
# :math:`I_{\mathrm{NaP}}` for a pretty label.


def texify_name(name):
    return r'${}_{{\mathrm{{{}}}}}$'.format(*name.split('_'))

###############################################################################
# Next, we add a right vertical axis and plot the currents with respect to
# that axis.


Iax = Vax.twinx()
Iax.plot(t_dc, I_dc, 'k-', lw=2, label=texify_name('I_DC'))

for iname, color in (('I_h', 'maroon'), ('I_T', 'orange'),
                     ('I_NaP', 'crimson'), ('I_KNa', 'aqua')):
    Iax.plot(t, data[iname], color=color, lw=2, label=texify_name(iname))

Iax.set_xlim(0, t_end)
Iax.set_ylim(-10., 15.)
Iax.set_ylabel('Current [pA]')
Iax.set_title('ht_neuron driven by DC current')

###############################################################################
# We need to make a little extra effort to combine lines from the two axis
# into one legend.

lines_V, labels_V = Vax.get_legend_handles_labels()
lines_I, labels_I = Iax.get_legend_handles_labels()
try:
    Iax.legend(lines_V + lines_I, labels_V + labels_I, fontsize='small')
except TypeError:
    # work-around for older Matplotlib versions
    Iax.legend(lines_V + lines_I, labels_V + labels_I)

###############################################################################
# Note that ``I_KNa`` is not activated in this example because the neuron does
# not spike. ``I_T`` has only a very small amplitude.
